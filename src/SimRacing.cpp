/*
 *  Project     Sim Racing Library for Arduino
 *  @author     David Madison
 *  @link       github.com/dmadison/Sim-Racing-Arduino
 *  @license    LGPLv3 - Copyright (c) 2022 David Madison
 *
 *  This file is part of the Sim Racing Library for Arduino.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SimRacing.h"

/**
* @file SimRacing.cpp
* @brief Source file for the Sim Racing Library
*/

namespace SimRacing {

/**
* Take a pin number as an input and sanitize it to a known working value
* 
* In an ideal world this would check against the available pins on the micro,
* but as far as I know the Arduino API does not have a "valid pin" function. 
* Instead, we'll just accept any positive number as a pin and reject any
* negative number as invalid ("Unused").
* 
* @param pin the pin number to sanitize
* @returns the pin number, or UnusedPin
*/
static constexpr PinNum sanitizePin(PinNum pin) {
	return pin < 0 ? UnusedPin : pin;
}


/**
* Invert an input value so it's at the same relative position
* at the other side of an input range.
* 
* @param value the value to invert
* @param min the minimum value of the range
* @param max the maximum value of the range
* 
* @return the input value, mapped to the other end of the axis
*/
static constexpr long invertAxis(long value, long min, long max) {
	return max - value + min;  // flip to other side of the scale
}


/**
* Wraps the existing Arduino "map" function to include range checks, so the
* output is never outside the min/max range.
* 
* If inMin/inMax are flipped (max less than min), this will adjust the input value
* so its position is relative to the min/max axis. For example, if the min is
* 0 and the max is 100 with an input value of 5, the input value with be set to
* 95 (5 off of max) before being rescaled to the output.
* 
* @param value the value to remap to a new range
* @param inMin the minimum range of the input value
* @param inMax the maximum range of the input value
* @param outMin the minimum range of the output value
* @param outMax the maximum range of the output value
* 
* @return the remapped value
*/
static long remap(long value, long inMin, long inMax, long outMin, long outMax) {
	// if inverted, swap min/max and adjust position of value
	if (inMin > inMax) {
		const long temp = inMin;
		inMin = inMax;
		inMax = temp;

		value = invertAxis(value, inMin, inMax);
	}

	if (value <= inMin) return outMin;
	if (value >= inMax) return outMax;
	return map(value, inMin, inMax, outMin, outMax);
}


/**
* Filters a floating point value to a valid percentile range (0-1)
* 
* @param pct the input value
* @return the input value limited to 0-1
*/
static float floatPercent(float pct) {
	if (pct < 0.0) pct = 0.0;
	else if (pct > 1.0) pct = 1.0;
	return pct;
}

/**
* Flushes a Stream of input data until no data is remaining.
* 
* This includes a delay() statement so that platforms that require a yield()
* call for the watchdog timer don't freeze up.
*
* @param client the Stream client to flush
*/
static void flushClient(Stream& client) {
	while (client.read() != -1) { delay(2); }  // 9600 baud = ~1 ms per byte
}

/**
* Waits until new data is avaiable on a given Stream interface.
* 
* @param client the Stream client to read from
*/
static void waitClient(Stream& client) {
	flushClient(client);
	while (client.peek() == -1) { delay(1); }  // wait for a new byte (using delay to avoid watchdog)
}

/**
* Read a floating point percentage value from a given Stream interface.
* 
* The user can skip setting a floating point value by sending the character
* 'n' when prompted. If 'n' is received the value is left unchanged.
* 
* Note that this does *not* handle non-numeric strings. If a non-numeric string
* is sent the parseFloat() function will time out and default to "0.0".
* 
* @param value the floating point input, passed by reference
* @param client the Stream client to read from and write messages to
*/
static void readFloat(float& value, Stream& client) {
	client.print("(to skip this step and go with the default value of '");
	client.print(value);
	client.print("', send 'n')");
	client.println();

	waitClient(client);
	if (client.peek() == 'n') return;  // skip this step

	float input;

	while (true) {
		client.setTimeout(200);
		input = client.parseFloat();

		if (input >= 0.0 && input <= 1.0) {
			client.print(F("Set the new value to '"));
			client.print(input);
			client.println("'");
			break;
		}
		client.print(F("Input '"));
		client.print(input);
		client.print(F("' not within acceptable range (0.0 - 1.0). Please try again."));
		client.println();

		waitClient(client);
	}

	value = input;
}


//#########################################################
//                  DeviceConnection                      #
//#########################################################

DeviceConnection::DeviceConnection(PinNum pin, bool invert, unsigned long detectTime)
	:
	Pin(sanitizePin(pin)), Inverted(invert), stablePeriod(detectTime),  // constants(ish)

	/* Assume we're connected on first call
	*/
	state(ConnectionState::Connected),

	/* Init state to "not inverted", which is the connected state. For example
	* if we're looking for 'HIGH' then inverted is false, which means the
	* initial state is 'true' and thus connected.
	*
	* We're assuming we're connected on first call here because it allows
	* the device to be read as connected as soon as the board turns on, without
	* having to wait an arbitrary amount.
	*/
	pinState(!Inverted),

	/* Set the last pin change to right now minus the stable period so it's
	* read as being already stable. Again, this will make the class return
	* 'present' as soon as the board starts up
	*/
	lastChange(millis() - detectTime)

{
	if (pin != UnusedPin) {
		pinMode(Pin, INPUT);  // set pin as input, *no* pull-up
	}
}

void DeviceConnection::poll() {
	const bool newState = readPin();

	if (newState == HIGH && state == ConnectionState::Connected) return;  // short circuit, already connected

	// check if the pin changed. if it did, record the time
	if (pinState != newState) {
		pinState = newState;
		lastChange = millis();

		// rising, we just connected
		if (pinState == HIGH) {
			state = ConnectionState::PlugIn;
		}
		// falling, we just disconnected
		else {
			state = ConnectionState::Unplug;
		}
	}

	// if pin hasn't changed, compare stable times
	else {
		// check stable connection (over time)
		if (pinState == HIGH) {
			const unsigned long now = millis();
			if (now - lastChange >= stablePeriod) {
				state = ConnectionState::Connected;
			}
		}
		// if we were previously unplugged and are still low, now we're disconnected
		else if (state == ConnectionState::Unplug) {
			state = ConnectionState::Disconnected;
		}
	}
}

DeviceConnection::ConnectionState DeviceConnection::getState() const {
	return state;
}

bool DeviceConnection::isConnected() const {
	return this->getState() == ConnectionState::Connected;
}

void DeviceConnection::setStablePeriod(unsigned long t) {
	stablePeriod = t;

	if (state == ConnectionState::Connected) {
		const unsigned long now = millis();

		// if we were previously considered connected, adjust the timestamps
		// accordingly so that we still are
		if (now - lastChange < stablePeriod) {
			lastChange = now - stablePeriod;
		}
	}
}

bool DeviceConnection::readPin() const {
	if (Pin == UnusedPin) return HIGH;  // if no pin is set, we're always connected
	const bool state = digitalRead(Pin);
	return Inverted ? !state : state;
}

//#########################################################
//                     AnalogInput                        #
//#########################################################


AnalogInput::AnalogInput(PinNum p)
	: Pin(sanitizePin(p)), position(AnalogInput::Min), cal({AnalogInput::Min, AnalogInput::Max})
{
	if (Pin != UnusedPin) {
		pinMode(Pin, INPUT);
	}
}

bool AnalogInput::read() {
	bool changed = false;

	if (Pin != UnusedPin) {
		const int previous = this->position;
		this->position = analogRead(Pin);

		// check if value is different for 'changed' flag
		if (previous != this->position) {

			const int rMin = isInverted() ? getMax() : getMin();
			const int rMax = isInverted() ? getMin() : getMax();

			if (
				// if the previous value was under the minimum range
				// and the current value is as well, no change
				!(previous < rMin && this->position < rMin) && 

				// if the previous value was over the maximum range
				// and the current value is as well, no change
				!(previous > rMax && this->position > rMax)
			)
			{
				// otherwise, the current value is either within the
				// range limits *or* it has changed from one extreme
				// to the other. Either way, mark it changed!
				changed = true;
			}
		}
	}
	return changed;
}

long AnalogInput::getPosition(long rMin, long rMax) const {
	// inversion is handled within the remap function
	return remap(getPositionRaw(), getMin(), getMax(), rMin, rMax);
}

int AnalogInput::getPositionRaw() const {
	return this->position;
}

bool AnalogInput::isInverted() const {
	return (this->cal.min > this->cal.max);  // inverted if min is greater than max
}

void AnalogInput::setPosition(int newPos) {
	this->position = newPos;
}

void AnalogInput::setInverted(bool invert) {
	if (isInverted() == invert) return;  // inversion already set

	// to change inversion, swap max and min of the current calibration
	AnalogInput::Calibration inverted = { this->cal.max, this->cal.min };
	setCalibration(inverted);
}

void AnalogInput::setCalibration(AnalogInput::Calibration newCal) {
	this->cal = newCal;
}


//#########################################################
//                       Pedals                           #
//#########################################################

Pedals::Pedals(AnalogInput* dataPtr, uint8_t nPedals, PinNum detectPin)
	: 
	pedalData(dataPtr),
	NumPedals(nPedals),
	detector(detectPin),
	changed(false)
{}

void Pedals::begin() {
	update();  // set initial pedal position
}

bool Pedals::update() {
	changed = false;

	detector.poll();
	if (detector.getState() == DeviceConnection::Connected) {
		// if connected, read all pedal positions
		for (int i = 0; i < getNumPedals(); ++i) {
			changed |= pedalData[i].read();
		}
	}
	else if (detector.getState() == DeviceConnection::Unplug) {
		// on unplug event, zero all pedals
		for (int i = 0; i < getNumPedals(); ++i) {
			const int min = pedalData[i].getMin();
			pedalData[i].setPosition(min);
		}
		changed = true;  // set flag so we know everything moved to 0
	}

	return changed;
}

long Pedals::getPosition(PedalID pedal, long rMin, long rMax) const {
	if (!hasPedal(pedal)) return rMin;  // not a pedal
	return pedalData[pedal].getPosition(rMin, rMax);
}

int Pedals::getPositionRaw(PedalID pedal) const {
	if (!hasPedal(pedal)) return AnalogInput::Min;  // not a pedal
	return pedalData[pedal].getPositionRaw();
}

bool Pedals::hasPedal(PedalID pedal) const {
	return (pedal < getNumPedals());
}

void Pedals::setCalibration(PedalID pedal, AnalogInput::Calibration cal) {
	if (!hasPedal(pedal)) return;
	pedalData[pedal].setCalibration(cal);
	pedalData[pedal].setPosition(pedalData[pedal].getMin());  // reset to min position
}

String Pedals::getPedalName(PedalID pedal) {
	String name;

	switch (pedal) {
	case(PedalID::Gas):
		name = F("gas");
		break;
	case(PedalID::Brake):
		name = F("brake");
		break;
	case(PedalID::Clutch):
		name = F("clutch");
		break;
	default:
		name = F("???");
		break;
	}
	return name;
}

void Pedals::serialCalibration(Stream& iface) {
	const char* separator = "------------------------------------";

	iface.println();
	iface.println(F("Sim Racing Library Pedal Calibration"));
	iface.println(separator);
	iface.println();

	// read minimums
	iface.println(F("Take your feet off of the pedals so they move to their resting position."));
	iface.println(F("Send any character to continue."));
	waitClient(iface);

	const int MaxPedals = 3;  // hard-coded at 3 pedals

	AnalogInput::Calibration pedalCal[MaxPedals];

	// read minimums
	for (int i = 0; (i < getNumPedals()) && (i < MaxPedals); i++) {
		pedalData[i].read();  // read position
		pedalCal[i].min = pedalData[i].getPositionRaw();  // set min to the recorded position
	}
	iface.println(F("\nMinimum values for all pedals successfully recorded!\n"));
	iface.println(separator);

	// read maximums
	iface.println(F("\nOne at a time, let's measure the maximum range of each pedal.\n"));
	for (int i = 0; (i < getNumPedals()) && (i < MaxPedals); i++) {

		iface.print(F("Push the "));
		String name = getPedalName(static_cast<PedalID>(i));
		name.toLowerCase();
		iface.print(name);
		iface.print(F(" pedal to the floor. "));

		iface.println(F("Send any character to continue."));
		waitClient(iface);

		pedalData[i].read();  // read position
		pedalCal[i].max = pedalData[i].getPositionRaw();  // set max to the recorded position
	}

	// deadzone options
	iface.println(separator);
	iface.println();

	float DeadzoneMin = 0.01;  // by default, 1% (trying to keep things responsive)
	float DeadzoneMax = 0.025;  // by default, 2.5%

	iface.println(F("These settings are optional. Send 'y' to customize. Send any other character to continue with the default values."));

	iface.print(F("  * Pedal Travel Deadzone, Start: \t"));
	iface.print(DeadzoneMin);
	iface.println(F("  (Used to avoid the pedal always being slightly pressed)"));

	iface.print(F("  * Pedal Travel Deadzone, End:   \t"));
	iface.print(DeadzoneMax);
	iface.println(F("  (Used to guarantee that the pedal can be fully pressed)"));

	iface.println();

	waitClient(iface);

	if (iface.read() == 'y') {
		iface.println(F("Set the pedal travel starting deadzone as a floating point percentage."));
		readFloat(DeadzoneMin, iface);
		iface.println();

		iface.println(F("Set the pedal travel ending deadzone as a floating point percentage."));
		readFloat(DeadzoneMax, iface);
		iface.println();
	}

	flushClient(iface);

	// calculate deadzone offsets
	for (int i = 0; (i < getNumPedals()) && (i < MaxPedals); i++) {
		auto &cMin = pedalCal[i].min;
		auto &cMax = pedalCal[i].max;

		const int range = abs(cMax - cMin);
		const int dzMin = DeadzoneMin * (float)range;
		const int dzMax = DeadzoneMax * (float)range;

		// non-inverted
		if (cMax >= cMin) {
			cMax -= dzMax;  // 'cut' into the range so it limits sooner
			cMin += dzMin;
		}
		// inverted
		else {
			cMax += dzMax;
			cMin -= dzMin;
		}
	}

	// print finished calibration
	iface.println(F("Here is your calibration:"));
	iface.println(separator);
	iface.println();

	iface.print(F("pedals.setCalibration("));

	for (int i = 0; (i < getNumPedals()) && (i < MaxPedals); i++) {
		if(i > 0) iface.print(F(", "));
		iface.print('{');

		iface.print(pedalCal[i].min);
		iface.print(F(", "));
		iface.print(pedalCal[i].max);

		iface.print('}');

		this->setCalibration(static_cast<PedalID>(i), pedalCal[i]);  // and set it ourselves, too
	}
	iface.print(");");
	iface.println();

	iface.println();
	iface.println(separator);
	iface.println();

	iface.print(F("Paste this line into the setup() function. The "));
	iface.print(F("pedals"));
	iface.print(F(" will be calibrated with these values on startup."));
	iface.println(F("\nCalibration complete! :)\n\n"));

	flushClient(iface);
}


TwoPedals::TwoPedals(PinNum gasPin, PinNum brakePin, PinNum detectPin)
	: Pedals(pedalData, NumPedals, detectPin),
	pedalData{ AnalogInput(gasPin), AnalogInput(brakePin) }
{}

void TwoPedals::setCalibration(AnalogInput::Calibration gasCal, AnalogInput::Calibration brakeCal) {
	this->Pedals::setCalibration(PedalID::Gas, gasCal);
	this->Pedals::setCalibration(PedalID::Brake, brakeCal);
}


ThreePedals::ThreePedals(PinNum gasPin, PinNum brakePin, PinNum clutchPin, PinNum detectPin)
	: Pedals(pedalData, NumPedals, detectPin),
	pedalData{ AnalogInput(gasPin), AnalogInput(brakePin), AnalogInput(clutchPin) }
{}

void ThreePedals::setCalibration(AnalogInput::Calibration gasCal, AnalogInput::Calibration brakeCal, AnalogInput::Calibration clutchCal) {
	this->Pedals::setCalibration(PedalID::Gas, gasCal);
	this->Pedals::setCalibration(PedalID::Brake, brakeCal);
	this->Pedals::setCalibration(PedalID::Clutch, clutchCal);
}



LogitechPedals::LogitechPedals(PinNum gasPin, PinNum brakePin, PinNum clutchPin, PinNum detectPin)
	: ThreePedals(gasPin, brakePin, clutchPin, detectPin)
{
	// taken from calibrating my own pedals. the springs are pretty stiff so while
	// this covers the whole travel range, users may want to back it down for casual
	// use (esp. for the brake travel)
	this->setCalibration({ 904, 48 }, { 944, 286 }, { 881, 59 });
}

LogitechDrivingForceGT_Pedals::LogitechDrivingForceGT_Pedals(PinNum gasPin, PinNum brakePin, PinNum detectPin)
	: TwoPedals(gasPin, brakePin, detectPin)
{
	this->setCalibration({ 646, 0 }, { 473, 1023 });  // taken from calibrating my own pedals
}


//#########################################################
//                       Shifter                          #
//#########################################################

Shifter::Shifter(int8_t min, int8_t max)
	: MinGear(min), MaxGear(max)
{}

char Shifter::getGearChar(int gear) {
	char c = '?';

	switch (gear) {
	case(-1):
		c = 'r';
		break;
	case(0):
		c = 'n';
		break;
	default:
		if (gear > 0 && gear <= 9)
			c = '0' + gear;
		break;
	}
	return c;
}

char Shifter::getGearChar() const {
	return getGearChar(getGear());
}

String Shifter::getGearString(int gear) {
	String name;

	switch (gear) {
	case(-1):
		name = F("reverse");
		break;
	case(0):
		name = F("neutral");
		break;
	default: {
		if (gear < 0 || gear > 9) {
			name = F("???");
			break;  // out of range
		}
		name = gear;  // set string to current gear

		switch (gear) {
		case(1):
			name += F("st");
			break;
		case(2):
			name += F("nd");
			break;
		case(3):
			name += F("rd");
			break;
		default:
			name += F("th");
			break;
		}
		break;
	}
	}
	return name;
}

String Shifter::getGearString() const {
	return getGearString(getGear());
}


/* Static calibration constants
* These values are arbitrary - just what worked well with my own shifter.
*/
const float AnalogShifter::CalEngagementPoint = 0.70;
const float AnalogShifter::CalReleasePoint = 0.50;
const float AnalogShifter::CalEdgeOffset = 0.60;

AnalogShifter::AnalogShifter(PinNum pinX, PinNum pinY, PinNum pinRev, PinNum detectPin)
	: 
	/* In initializing the Shifter, the lowest gear is going to be '-1' if a pin
	* exists for reverse, otherwise it's going to be '0' (neutral).
	*/
	Shifter(sanitizePin(pinRev) != UnusedPin ? -1 : 0, 6),

	/* Two axes, X and Y */
	analogAxis{ AnalogInput(pinX), AnalogInput(pinY) },

	PinReverse(sanitizePin(pinRev)),
	detector(detectPin, false)  // not inverted
{}

void AnalogShifter::begin() {
	if (this->PinReverse != UnusedPin) {
		pinMode(PinReverse, INPUT);
	}
	update();  // set initial gear position
}

bool AnalogShifter::update() {
	detector.poll();

	switch (detector.getState()) {

	// connected! poll the ADC for new analog axis data
	case(DeviceConnection::Connected):
		analogAxis[Axis::X].read();
		analogAxis[Axis::Y].read();
		break;

	// on an unplug event, we want to reset our position back to
	// neutral and then immediately return
	case(DeviceConnection::Unplug):
	{
		const int8_t previousGear = this->getGear();

		analogAxis[Axis::X].setPosition(calibration.neutralX);
		analogAxis[Axis::Y].setPosition(calibration.neutralY);

		if (previousGear != 0) changed = true;
		currentGear = 0;
		return changed;
		break;
	}

	// if the device is either disconnected or just plugged in and unstable, set gear
	// 'changed' to false and then immediately return false to save on processing
	case(DeviceConnection::PlugIn):
	case(DeviceConnection::Disconnected):
		changed = false;
		return changed;
		break;
	}

	const int8_t previousGear = this->getGear();
	const bool prevOdd = ((previousGear != -1) && (previousGear & 1));  // were we previously in an odd gear
	const bool prevEven = (!prevOdd && previousGear != 0);  // were we previously in an even gear
	
	const int x = analogAxis[Axis::X].getPosition();
	const int y = analogAxis[Axis::Y].getPosition();
	int8_t newGear = 0;

	// If we're below the 'release' thresholds, we must still be in the previous gear
	if ((prevOdd && y > calibration.oddRelease) || (prevEven && y < calibration.evenRelease)) {
		newGear = previousGear;
	}

	// If we're *not* below the release thresholds, we may be in a different gear
	else {
		// Check if we're in even or odd gears (Y axis)
		if (y > calibration.oddTrigger) {
			newGear = 1;  // we're in an odd gear
		}
		else if (y < calibration.evenTrigger) {
			newGear = 2;  // we're in an even gear
		}

		if (newGear != 0) {
			// Now check *which* gear we're in, if we're in one (X axis)
			     if (x > calibration.rightEdge) newGear += 4;  // 1-2 + 4 = 5-6
			else if (x >= calibration.leftEdge) newGear += 2;  // 1-2 + 2 = 3-4
			// (note the '>=', because it would normally be a '<' check for the lower range)
			// else gear = 1-2 (as set above)

			const bool reverse = getReverseButton();

			// If the reverse button is pressed and we're in 5th gear
			// something is wrong. Revert that and go back to neutral.
			if (reverse && newGear == 5) {
				newGear = 0;
			}

			// If the reverse button is pressed or we were previously
			// in reverse *and* we are currently in 6th gear, then we
			// should be in reverse.
			else if ((reverse || previousGear == -1) && newGear == 6) {
				newGear = -1;
			}
		}
	}

	changed = (newGear != previousGear) ? 1 : 0;
	currentGear = newGear;

	return changed;
}

long AnalogShifter::getPosition(Axis ax, long min, long max) const {
	if (ax != Axis::X && ax != Axis::Y) return min;  // not an axis
	return analogAxis[ax].getPosition(min, max);
}

int AnalogShifter::getPositionRaw(Axis ax) const {
	if (ax != Axis::X && ax != Axis::Y) return AnalogInput::Min;  // not an axis
	return analogAxis[ax].getPositionRaw();
}

bool AnalogShifter::getReverseButton() const {
	// if the reverse pin is not set *or* if the device is not currently
	// connected, avoid reading the floating input and just return 'false'
	if (PinReverse == UnusedPin || detector.getState() != DeviceConnection::Connected) {
		return false;
	}
	return digitalRead(PinReverse);
}

void AnalogShifter::setCalibration(
	GearPosition neutral,
	GearPosition g1, GearPosition g2, GearPosition g3, GearPosition g4, GearPosition g5, GearPosition g6,
	float engagePoint, float releasePoint, float edgeOffset) {

	// limit percentage thresholds
	engagePoint = floatPercent(engagePoint);
	releasePoint = floatPercent(releasePoint);
	edgeOffset = floatPercent(edgeOffset);

	const int xLeft = (g1.x + g2.x) / 2;  // find the minimum X position average
	const int xRight = (g5.x + g6.x) / 2;  // find the maximum X position average

	const int yOdd = (g1.y + g3.y + g5.y) / 3;  // find the maximum Y position average
	const int yEven = (g2.y + g4.y + g6.y) / 3;  // find the minimum Y position average

	// set X/Y calibration and inversion
	analogAxis[Axis::X].setCalibration({ xLeft, xRight });
	analogAxis[Axis::Y].setCalibration({ yEven, yOdd });

	// save neutral values (raw)
	calibration.neutralX = neutral.x;
	calibration.neutralY = neutral.y;

	// get normalized and inverted neutral values
	// this lets us take advantage of the AnalogInput normalization function
	// that handles inverted axes and automatic range rescaling, so the rest of
	// the calibration options can be in the normalized range
	const Axis axes[2] = { Axis::X, Axis::Y };
	int* const neutralAxis[2] = { &neutral.x, &neutral.y };
	for (int i = 0; i < 2; i++) {
		const int previous = analogAxis[axes[i]].getPositionRaw();  // save current value
		analogAxis[axes[i]].setPosition(*neutralAxis[i]);           // set new value to neutral calibration
		*neutralAxis[i] = analogAxis[axes[i]].getPosition();        // get normalized neutral value
		analogAxis[axes[i]].setPosition(previous);                  // reset axis position to previous
	}

	// calculate the distances between each neutral and the limits of each axis
	const int yOddDiff = AnalogInput::Max - neutral.y;
	const int yEvenDiff = neutral.y - AnalogInput::Min;

	const int leftDiff = neutral.x - AnalogInput::Min;
	const int rightDiff = AnalogInput::Max - neutral.x;

	// calculate and save the trigger and release points for each level
	calibration.oddTrigger = neutral.y + ((float)yOddDiff * engagePoint);
	calibration.oddRelease = neutral.y + ((float)yOddDiff * releasePoint);

	calibration.evenTrigger = neutral.y - ((float)yEvenDiff * engagePoint);
	calibration.evenRelease = neutral.y - ((float)yEvenDiff * releasePoint);

	calibration.leftEdge = neutral.x - ((float)leftDiff * edgeOffset);
	calibration.rightEdge = neutral.x + ((float)rightDiff * edgeOffset);

#if 0
	Serial.print("Odd Trigger: ");
	Serial.println(calibration.oddTrigger);
	Serial.print("Odd Release: ");
	Serial.println(calibration.oddRelease);
	Serial.print("Even Trigger: ");
	Serial.println(calibration.evenTrigger);
	Serial.print("Even Release: ");
	Serial.println(calibration.evenRelease);
	Serial.print("Left Edge: ");
	Serial.println(calibration.leftEdge);
	Serial.print("Right Edge: ");
	Serial.println(calibration.rightEdge);
	Serial.println();

	Serial.print("X Min: ");
	Serial.println(analogAxis[Axis::X].getMin());
	Serial.print("X Max: ");
	Serial.println(analogAxis[Axis::X].getMax());

	Serial.print("Y Min: ");
	Serial.println(analogAxis[Axis::Y].getMin());
	Serial.print("Y Max: ");
	Serial.println(analogAxis[Axis::Y].getMax());
#endif
}

void AnalogShifter::serialCalibration(Stream& iface) {
	if (isConnected() == false) {
		iface.print(F("Error! Cannot perform calibration, "));
		iface.print(F("shifter"));
		iface.println(F(" is not connected."));
		return;
	}

	const char* separator = "------------------------------------";

	iface.println();
	iface.println(F("Sim Racing Library Shifter Calibration"));
	iface.println(separator);
	iface.println();

	AnalogShifter::GearPosition gears[7];  // neutral, then 1-6
	float engagementPoint = CalEngagementPoint;
	float releasePoint = CalReleasePoint;
	float edgeOffset = CalEdgeOffset;

	for (int i = 0; i <= 6; i++) {
		const String gearName = this->getGearString(i);

		iface.print(F("Please move the gear shifter into "));
		iface.print(gearName);
		iface.println(F(". Send any character to continue."));

		waitClient(iface);

		this->update();
		gears[i] = { 
			this->analogAxis[Axis::X].getPositionRaw(), 
			this->analogAxis[Axis::Y].getPositionRaw()
		};

		iface.print("Gear '");
		iface.print(gearName);
		iface.print("' position recorded as { ");
		iface.print(gears[i].x);
		iface.print(", ");
		iface.print(gears[i].y);
		iface.println(" }");
		iface.println();
	}

	iface.println(separator);
	iface.println();

	iface.println(F("These settings are optional. Send 'y' to customize. Send any other character to continue with the default values."));

	iface.print(F("  * Gear Engagement Point: \t"));
	iface.println(engagementPoint);

	iface.print(F("  * Gear Release Point:   \t"));
	iface.println(releasePoint);

	iface.print(F("  * Horizontal Gate Offset:\t"));
	iface.println(edgeOffset);

	iface.println();

	waitClient(iface);

	if (iface.read() == 'y') {
		iface.println(F("Set the engagement point as a floating point percentage. This is the percentage away from the neutral axis on Y to start engaging gears."));
		readFloat(engagementPoint, iface);
		iface.println();

		iface.println(F("Set the release point as a floating point percentage. This is the percentage away from the neutral axis on Y to go back into neutral. It must be less than the engagement point."));
		readFloat(releasePoint, iface);
		iface.println();

		iface.println(F("Set the gate offset as a floating point percentage. This is the percentage away from the neutral axis on X to select the side gears."));
		readFloat(edgeOffset, iface);
		iface.println();
	}

	flushClient(iface);

	this->setCalibration(gears[0], gears[1], gears[2], gears[3], gears[4], gears[5], gears[6], engagementPoint, releasePoint, edgeOffset);

	iface.println(F("Here is your calibration:"));
	iface.println(separator);
	iface.println();

	iface.print(F("shifter.setCalibration( "));

	for (int i = 0; i < 7; i++) {
		iface.print('{');

		iface.print(gears[i].x);
		iface.print(", ");
		iface.print(gears[i].y);

		iface.print('}');
		iface.print(", ");
	}
	iface.print(engagementPoint);
	iface.print(", ");
	iface.print(releasePoint);
	iface.print(", ");
	iface.print(edgeOffset);
	iface.print(");");
	iface.println();

	iface.println();
	iface.println(separator);
	iface.println();

	iface.println(F("Paste this line into the setup() function to calibrate on startup."));
	iface.println(F("\n\nCalibration complete! :)\n"));
}

LogitechShifter::LogitechShifter(PinNum pinX, PinNum pinY, PinNum pinRev, PinNum detectPin)
	: AnalogShifter(pinX, pinY, pinRev, detectPin)
{
	this->setCalibration({ 490, 440 }, { 253, 799 }, { 262, 86 }, { 460, 826 }, { 470, 76 }, { 664, 841 }, { 677, 77 });
}

//#########################################################
//                      Handbrake                         #
//#########################################################

Handbrake::Handbrake(PinNum pinAx, PinNum detectPin)
	: 
	analogAxis(pinAx),
	detector(detectPin),
	changed(false)
{}

void Handbrake::begin() {
	update();  // set initial handbrake position
}

bool Handbrake::update() {
	changed = false;

	detector.poll();
	if (detector.getState() == DeviceConnection::Connected) {
		changed = analogAxis.read();
	}
	else if (detector.getState() == DeviceConnection::Unplug) {
		analogAxis.setPosition(analogAxis.getMin());
		changed = true;
	}

	return changed;
}

long Handbrake::getPosition(long rMin, long rMax) const {
	return analogAxis.getPosition(rMin, rMax);
}

int Handbrake::getPositionRaw() const {
	return analogAxis.getPositionRaw();
}

void Handbrake::setCalibration(AnalogInput::Calibration newCal) {
	analogAxis.setCalibration(newCal);
	analogAxis.setPosition(analogAxis.getMin());  // reset to min
}

void Handbrake::serialCalibration(Stream& iface) {
	if (isConnected() == false) {
		iface.print(F("Error! Cannot perform calibration, "));
		iface.print(F("handbrake"));
		iface.println(F(" is not connected."));
		return;
	}

	const char* separator = "------------------------------------";

	iface.println();
	iface.println(F("Sim Racing Library Handbrake Calibration"));
	iface.println(separator);
	iface.println();

	AnalogInput::Calibration newCal;

	// read minimum
	iface.println(F("Keep your hand off of the handbrake to record its resting position"));
	iface.println(F("Send any character to continue."));
	waitClient(iface);

	analogAxis.read();
	newCal.min = analogAxis.getPositionRaw();
	iface.println();

	// read maximum
	iface.println(F("Now pull on the handbrake and hold it at the end of its range"));
	iface.println(F("Send any character to continue."));
	waitClient(iface);

	analogAxis.read();
	newCal.max = analogAxis.getPositionRaw();
	iface.println();

	// set new calibration
	this->setCalibration(newCal);

	// print finished calibration
	iface.println(F("Here is your calibration:"));
	iface.println(separator);
	iface.println();

	iface.print(F("handbrake.setCalibration("));
	iface.print('{');
	iface.print(newCal.min);
	iface.print(F(", "));
	iface.print(newCal.max);
	iface.print("});");
	iface.println();

	iface.println();
	iface.println(separator);
	iface.println();

	iface.print(F("Paste this line into the setup() function. The "));
	iface.print(F("handbrake"));
	iface.print(F(" will be calibrated with these values on startup."));
	iface.println(F("\nCalibration complete! :)\n\n"));

	flushClient(iface);
}
	
};  // end SimRacing namespace
