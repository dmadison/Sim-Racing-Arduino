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

#ifndef SIM_RACING_LIB_H
#define SIM_RACING_LIB_H

#include <Arduino.h>

/**
* @file SimRacing.h
* @brief Header file for the Sim Racing Library
*/

namespace SimRacing {
	/**
	* Type alias for pin numbers, using Arduino numbering
	*/
	using PinNum = int16_t;

	/**
	* Dummy pin number signaling that a pin is unused
	* and can be safely ignored
	*/
	const PinNum UnusedPin = -1;


	/**
	* Enumeration for analog axis names, mapped to integers
	*/
	enum Axis : uint8_t {
		X = 0,  ///< Cartesian X axis
		Y = 1,  ///< Cartesian Y axis
	};


	/**
	* @brief Used for tracking whether a device is connected to
	* a specific pin and stable.
	*/
	class DeviceConnection {
	public:
		/**
		* The state of the connection, whether it is connected, disconnected, and
		* everywhere in-between. This is the type returned by the main 'getState()'
		* method.
		*
		* @see getState()
		*/
		enum ConnectionState {
			Disconnected,  ///< No connection present
			PlugIn,        ///< Device was just plugged in (connection starts), unstable
			Connected,     ///< Connection present and stable
			Unplug,        ///< Device was just removed (connection ends)
		};

		/**
		* Class constructor
		*
		* @param pin        the pin number being read. Can be 'UnusedPin' to disable.
		* @param activeLow  whether the device is detected on a high signal (false, 
		*                   default) or a low signal (true)
		* @param detectTime the amount of time, in ms, the input must be stable for
		*                   before it's interpreted as 'detected'
		*/
		DeviceConnection(PinNum pin, bool activeLow = false, unsigned long detectTime = 250);

		/**
		* Checks if the pin detects a connection. This polls the input and checks
		* for whether it has been connected sufficiently long enough to be
		* deemed "stable".
		*/
		void poll();

		/**
		* Retrieves the current ConnectionState from the instance. This
		* allows functions to check the same connection state multiple times
		* without having to re-poll.
		*/
		ConnectionState getState() const;

		/**
		* Check if the device is physically connected to the board. That means
		* it is both present and detected long enough to be considered 'stable'.
		*
		* @return 'true' if the device is connected, 'false' otherwise
		*/
		bool isConnected() const;

		/**
		* Set how long the detection pin must be stable for before the device
		* is considered to be 'connected'
		*
		* @param t the amount of time, in ms, the input must be stable for
		*          (no changes) before it's interpreted as 'detected'
		*/
		void setStablePeriod(unsigned long t);

	private:
		/**
		* Reads the state of the pin, compensating for inversion
		*
		* @return 'true' if the pin is detected, 'false' otherwise
		*/
		bool readPin() const;

		PinNum pin;                  ///< The pin number being read from. Can be 'UnusedPin' to disable
		bool inverted;               ///< Whether the input is inverted, so 'LOW' is detected instead of 'HIGH'
		unsigned long stablePeriod;  ///< The amount of time the input must be stable for (ms)

		ConnectionState state;       ///< The current state of the connection
		bool pinState;               ///< Buffered state of the input pin, accounting for inversion
		unsigned long lastChange;    ///< Timestamp of the last pin change, in ms (using millis())
	};


	/**
	* @brief Handle I/O for analog (ADC) inputs
	*/
	class AnalogInput {
	public:
		static const int Min = 0;     ///< Minimum value of the analog to digital (ADC) converter
		static const int Max = 1023;  ///< Maximum value of the analog to digital (ADC) converter. 10-bit by default.

		/**
		* Class constructor
		*
		* @param pin the I/O pin for this input (Arduino numbering)
		*/
		AnalogInput(PinNum pin);

		/**
		* Updates the current value of the axis by polling the ADC
		*
		* @return 'true' if the value changed, 'false' otherwise
		*/
		virtual bool read();

		/**
		* Retrieves the buffered position for the analog axis, rescaled to a
		* nominal range using the calibration values.
		*
		* By default this is rescaled to a 10-bit value, matching the range
		* used by the AVR analog to digital converter (ADC).
		*
		* @param rMin the minimum output value for the rescaling function
		* @param rMax the maximum output value for the rescaling function
		*
		* @return the axis position, buffered and rescaled
		*/
		long getPosition(long rMin = Min, long rMax = Max) const;

		/**
		* Retrieves the buffered position for the analog axis.
		*
		* @return the axis position, buffered
		*/
		int getPositionRaw() const;

		/**
		* Retrieves the calibrated minimum position.
		*
		* @return the minimum position for the axis, per the calibration
		*/
		int getMin() const { return this->cal.min; }

		/**
		* Retrieves the calibrated maximum position.
		*
		* @return the maximum position for the axis, per the calibration
		*/
		int getMax() const { return this->cal.max; }

		/**
		* Check whether the axis is inverted or not.
		*
		* @return 'true' if the axis is inverted, 'false' otherwise
		*/
		bool isInverted() const;

		/**
		* Override the current position with a custom value.
		* 
		* This is useful for whenever a device has disconnected and we want to
		* set a new 'default' value for position requests.
		*
		* @param newPos the new position value to set
		*/
		void setPosition(int newPos);

		/**
		* Set the 'inverted' state of the axis. This will return a flipped
		* number when getPosition() is called (e.g. the axis at its maximum
		* will return a minimum value).
		*
		* @param invert whether the axis is inverted
		*/
		void setInverted(bool invert = true);

		/**
		* @brief Simple struct containing min/max values for axis calibration
		*
		* Note that this does not set default values for these members, even
		* though it would make sense to set them to the ADC min and max,
		* because doing so inhibits aggregate initialization under C++11. That
		* means users could not call the calibration functions with a simple
		* brace-enclosed initializer list without creating a variable first.
		*
		* @see https://stackoverflow.com/a/18184210
		*/
		struct Calibration {
			int min;  ///< Minimum value of the analog axis
			int max;  ///< Maximum value of the analog axis
		};

		/**
		* Calibrate the axis' min/max values for rescaling.
		*
		* @param newCal the calibration data struct to pass
		*/
		void setCalibration(Calibration newCal);

	private:
		PinNum pin;              ///< the digital pin number for this input
		int position;            ///< the axis' position in its range, buffered
		Calibration cal;         ///< the calibration values for the axis
	};


	/**
	* @brief Abstract class for all peripherals
	*/
	class Peripheral {
	public:
		/**
		* Class destructor
		*/
		virtual ~Peripheral() {}

		/**
		* Initialize the hardware (if necessary)
		*/
		virtual void begin() {};

		/**
		* Perform a poll of the hardware to refresh the class state
		*
		* @returns 'true' if device state changed, 'false' otherwise
		*/
		bool update();

		/**
		* Check if the device is physically connected to the board. That means
		* it is both present and detected long enough to be considered 'stable'.
		*
		* @returns 'true' if the device is connected, 'false' otherwise
		*/
		bool isConnected() const;

		/** @copydoc DeviceConnection::setStablePeriod(unsigned long) */
		void setStablePeriod(unsigned long t);

	protected:
		/**
		* Perform an internal poll of the hardware to refresh the class state
		* 
		* This function is called from within the public update() in order to
		* refresh the cached state of the peripheral. It needs to be defined
		* in every derived class. This function is the *only* place where the
		* cached device state should be changed.
		* 
		* @param connected the state of the device connection
		* 
		* @returns 'true' if device state changed, 'false' otherwise
		*/
		virtual bool updateState(bool connected) = 0;

		/**
		* Sets the pointer to the detector object
		*
		* The detector object is used to check if the peripheral is connected
		* to the microcontroller. The object is polled on every update.
		*
		* Although the detector instance is accessed via the Peripheral class,
		* it is the responsibility of the dervied class to store the
		* DeviceConnection object and manage its lifetime.
		*
		* @param d pointer to the detector object
		*/
		void setDetectPtr(DeviceConnection* d);

	private:
		DeviceConnection* detector;  ///< Pointer to a device connection instance
	};


	/**
	* @defgroup Pedals Pedals
	* @brief Classes that interface with pedal devices.
	* @{
	*/

	/**
	* @brief Pedal ID names
	*/
	enum Pedal {
		Gas = 0,
		Accelerator = Gas,
		Throttle = Gas,
		Brake = 1,
		Clutch = 2,
	};

	/**
	* @brief Base class for all pedals instances
	*/
	class Pedals : public Peripheral {
	public:
		/** Scoped alias for SimRacing::Pedal */
		using PedalID = SimRacing::Pedal;

		/**
		* Class constructor
		*
		* @param dataPtr  pointer to the analog input data managed by the class,
		*                 stored elsewhere
		* @param nPedals  the number of pedals stored in said data pointer
		*/
		Pedals(
			AnalogInput* dataPtr, uint8_t nPedals
		);

		/** @copydoc Peripheral::begin() */
		virtual void begin();

		/**
		* Retrieves the buffered position for the pedal, rescaled to a
		* nominal range using the calibration values.
		*
		* By default this is rescaled to an integer percentage.
		*
		* @param pedal the pedal to retrieve position for
		* @param rMin the minimum output value for the rescaling function
		* @param rMax the maximum output value for the rescaling function
		*
		* @return the pedal position, buffered and rescaled
		*/
		long getPosition(PedalID pedal, long rMin = 0, long rMax = 100) const;

		/**
		* Retrieves the buffered position for the pedal, ignoring the
		* calibration data.
		*
		* @param pedal the pedal to retrieve position for
		* @return the axis position, buffered
		*/
		int getPositionRaw(PedalID pedal) const;

		/**
		* Checks if a given pedal is present in the class.
		* 
		* @param pedal the pedal to check
		* @return 'true' if there is data for the pedal, 'false' otherwise
		*/
		bool hasPedal(PedalID pedal) const;

		/**
		* Retrieves the number of pedals handled by the class.
		* 
		* @return the number of pedals handled by the class
		*/
		int getNumPedals() const { return this->NumPedals; }

		/**
		* Checks whether the current pedal positions have changed since the last update.
		*
		* @return 'true' if position has changed, 'false' otherwise
		*/
		bool positionChanged() const { return changed; }

		/**
		* Calibrate a pedal's min/max values for rescaling.
		* 
		* @param pedal the pedal to set the calibration of
		* @param cal the calibration data to set
		*/
		void setCalibration(PedalID pedal, AnalogInput::Calibration cal);

		/**
		* Runs an interactive calibration tool using the serial interface.
		*
		* @param iface the serial interface to send and receive prompts.
		*        Defaults to Serial (CDC USB on most boards).
		*/
		void serialCalibration(Stream& iface = Serial);
		
		/**
		* Utility function to get the string name for each pedal.
		*
		* @param pedal the pedal to get the name of
		* @return the name of the pedal, as a String
		*/
		static String getPedalName(PedalID pedal);

	protected:
		/** @copydoc Peripheral::updateState(bool) */
		virtual bool updateState(bool connected);

	private:
		AnalogInput* pedalData;     ///< pointer to the pedal data
		const int NumPedals;        ///< number of pedals managed by this class
		bool changed;               ///< whether the pedal position has changed since the previous update
	};


	/**
	* @brief Pedal implementation for devices with only gas and brake
	*/
	class TwoPedals : public Pedals {
	public:
		/**
		* Class constructor
		*
		* @param pinGas   the analog pin for the gas pedal potentiometer
		* @param pinBrake the analog pin for the brake pedal potentiometer
		*/
		TwoPedals(
			PinNum pinGas, PinNum pinBrake
		);

		/**
		* Sets the calibration data (min/max) for the pedals
		*
		* @param gasCal the calibration data for the gas pedal
		* @param brakeCal the calibration data for the brake pedal
		*/
		void setCalibration(AnalogInput::Calibration gasCal, AnalogInput::Calibration brakeCal);

	private:
		static const uint8_t NumPedals = 2;  ///< number of pedals handled by this class
		AnalogInput pedalData[NumPedals];    ///< pedal data storage struct, passed to AxisManager
	};


	/**
	* @brief Pedal implementation for devices with gas, brake, and clutch
	*/
	class ThreePedals : public Pedals {
	public:
		/**
		* Class constructor
		* 
		* @param pinGas    the analog pin for the gas pedal potentiometer
		* @param pinBrake  the analog pin for the brake pedal potentiometer
		* @param pinClutch the analog pin for the clutch pedal potentiometer
		*/
		ThreePedals(
			PinNum pinGas, PinNum pinBrake, PinNum pinClutch
		);

		/**
		* Sets the calibration data (min/max) for the pedals
		* 
		* @param gasCal the calibration data for the gas pedal
		* @param brakeCal the calibration data for the brake pedal
		* @param clutchCal the calibration data for the clutch pedal
		*/
		void setCalibration(AnalogInput::Calibration gasCal, AnalogInput::Calibration brakeCal, AnalogInput::Calibration clutchCal);

	private:
		static const uint8_t NumPedals = 3;  ///< number of pedals handled by this class
		AnalogInput pedalData[NumPedals];    ///< pedal data storage struct, passed to AxisManager
	};

	/// @} Pedals


	/**
	* @defgroup Shifters Shifters
	* @brief Classes that interface with shifting devices.
	* @{
	*/

	/**
	* @brief Base class for all shifter instances
	*/
	class Shifter : public Peripheral {
	public:
		/**
		* Type alias for gear numbers
		*/
		using Gear = int8_t;

		/**
		* Class constructor
		* 
		* @param min the lowest gear possible
		* @param max the highest gear possible
		*/
		Shifter(Gear min, Gear max);
	
		/**
		* Returns the currently selected gear.
		*
		* Will either be reverse (-1), neutral (0), or the current gear
		* indexed at 1 (1st gear is 1, 2nd gear is 2, etc.).
		*
		* @return current gear index
		*/
		Gear getGear() const { return currentGear; }

		/**
		* Returns a character that represents the given gear.
		*
		* 'r' for reverse, 'n' for neutral, or the number of the current gear.
		*
		* @param gear the gear index to get the representation for
		* @return letter representing the current gear
		*/
		static char getGearChar(int gear);

		/**
		* Returns a character that represents the current gear.
		*
		* @return letter representing the current gear
		* @see getGearChar(int)
		*/
		char getGearChar() const;

		/**
		* Returns a String that represents the given gear.
		*
		* "reverse" for reverse, "neutral" for neutral, and then "1st", "2nd",
		* "3rd", and so on.
		*
		* @param gear the gear index to get the representation for
		* @return String representing the current gear
		*/
		static String getGearString(int gear);

		/**
		* Returns a String that represents the current gear.
		*
		* @return String representing the current gear
		* @see getGearString(int)
		*/
		String getGearString() const;

		/**
		* Checks whether the current gear has changed since the last update.
		*
		* @return 'true' if gear has changed, 'false' otherwise
		*/
		bool gearChanged() const { 
			return this->currentGear != this->previousGear;
		}

		/**
		* Retrieves the minimum possible gear index.
		*
		* @return the lowest gear index
		*/
		Gear getGearMin() { return MinGear; }

		/**
		* Retrieves the maximum possible gear index.
		*
		* @return the highest gear index
		*/
		Gear getGearMax() { return MaxGear; }

	protected:
		/**
		* Changes the currently set gear, internally
		* 
		* This function sanitizes the newly selected gear with MinGear / MaxGear,
		* and handles caching the previous value for checking if the gear has
		* changed.
		* 
		* @param gear the new gear value to set
		*/
		void setGear(Gear gear);

	private:
		const Gear MinGear;  ///< the lowest selectable gear
		const Gear MaxGear;  ///< the highest selectable gear

		Gear currentGear;    ///< index of the current gear
		Gear previousGear;   ///< index of the last selected gear
	};


	/**
	* @brief Interface with shifters using two potentiometers for gear position
	*/
	class AnalogShifter : public Shifter {
	public:
		/**
		* Class constructor
		* 
		* @param gearMin the lowest gear possible
		* @param gearMax the highest gear possible
		* @param pinX    the analog input pin for the X axis
		* @param pinY    the analog input pin for the Y axis
		* @param pinRev  the digital input pin for the 'reverse' button
		* 
		* @note With the way the class is designed, the lowest possible gear is
		*       -1 (reverse), and the highest possible gear is 6. Setting the
		*       arguments lower/higher than this will have no effect. Setting
		*       the arguments within this range will limit to those gears,
		*       and selecting gears out of range will result in neutral.
		*/
		AnalogShifter(
			Gear gearMin, Gear gearMax,
			PinNum pinX, PinNum pinY,
			PinNum pinRev = UnusedPin
		);

		/**
		* Initializes the hardware pins for reading the gear states.
		* 
		* Should be called in `setup()` before reading from the shifter.
		*/
		virtual void begin();

		/** @copydoc AnalogInput::getPosition()
		*   @param ax the axis to get the position of
		*/
		long getPosition(Axis ax, long rMin = AnalogInput::Min, long rMax = AnalogInput::Max) const;

		/** @copydoc AnalogInput::getPositionRaw()
		*   @param ax the axis to get the position of
		*/
		int getPositionRaw(Axis ax) const;

		/**
		* Checks the current state of the "reverse" button at the bottom
		* of the shift column. This button is pressed (HIGH) when the shifter
		* is in reverse gear, LOW otherwise.
		*
		* @return current state of the "reverse" button
		*/
		bool getReverseButton() const;

		/**
		* @brief Simple struct to store X/Y coordinates for the calibration function
		*/
		struct GearPosition {
			int x;  ///< X coordinate of the gear position from the ADC
			int y;  ///< Y coordinate of the gear position from the ADC
		};

		/**
		* Calibrate the gear shifter for more accurate shifting.
		* 
		* Note that this uses a large number of GearPosition arguments rather
		* than an array because it allows for the use of aggregate initialization.
		* 
		* This way users can copy a single line to set calibration, rather than
		* declaring an array of GearPosition elements and then passing that by
		* pointer to this function.
		* 
		* @param neutral the X/Y position of the shifter in neutral
		* @param g1 the X/Y position of the shifter in 1st gear
		* @param g2 the X/Y position of the shifter in 2nd gear
		* @param g3 the X/Y position of the shifter in 3rd gear
		* @param g4 the X/Y position of the shifter in 4th gear
		* @param g5 the X/Y position of the shifter in 5th gear
		* @param g6 the X/Y position of the shifter in 6th gear
		* @param engagePoint  distance from neutral on Y to register a gear as
		*                     being engaged (as a percentage of distance from
		*                     neutral to Y max, 0-1)
		* @param releasePoint distance from neutral on Y to go back into neutral 
		*                     from an engaged gear (as a percentage of distance
		*                     from neutral to Y max, 0-1)
		* @param edgeOffset   distance from neutral on X to select the side gears
		*                     rather than the center gears (as a percentage of
		*                     distance from neutral to X max, 0-1)
		*/
		void setCalibration(
			GearPosition neutral,
			GearPosition g1, GearPosition g2, GearPosition g3, GearPosition g4, GearPosition g5, GearPosition g6,
			float engagePoint = CalEngagementPoint, float releasePoint = CalReleasePoint, float edgeOffset = CalEdgeOffset);

		/**
		* Runs an interactive calibration tool using the serial interface.
		* 
		* @param iface the serial interface to send and receive prompts.
		*        Defaults to Serial (CDC USB on most boards).
		*/
		void serialCalibration(Stream& iface = Serial);

	protected:
		/** @copydoc Peripheral::updateState(bool) */
		virtual bool updateState(bool connected);

	private:
		/**
		* Read the state of the reverse button
		* 
		* This function should *only* be called as part of updateState(bool),
		* to update the state of the device.
		* 
		* @returns the state of the reverse button, 'true' if pressed,
		*          'false' otherwise
		*/
		virtual bool readReverseButton();

		/**
		* Distance from neutral on Y to register a gear as
		* being engaged (as a percentage of distance from
		* neutral to Y max, 0-1). Used for calibration.
		*/
		static const float CalEngagementPoint; 

		/**
		* Distance from neutral on Y to go back into neutral 
		* from an engaged gear (as a percentage of distance
		* from neutral to Y max, 0-1). Used for calibration.
		*/
		static const float CalReleasePoint;

		/**
		* Distance from neutral on X to select the side gears
		* rather than the center gears (as a percentage of
		* distance from neutral to X max, 0-1). Used for calibration.
		*/
		static const float CalEdgeOffset;

		/*** Internal calibration struct */
		struct Calibration {
			int    neutralX;  ///< X-axis neutral position, for reset on disconnect
			int    neutralY;  ///< Y-axis neutral position, for reset on disconnect
			int  oddTrigger;  ///< Odd gear threshold to set the input 'on' if disengaged
			int  oddRelease;  ///< Odd gear threshold to set the input 'off' if engaged
			int evenTrigger;  ///< Even gear threshold to set the input 'on' if disengaged
			int evenRelease;  ///< Even gear threshold to set the input 'off' if engaged
			int    leftEdge;  ///< Threshold for the lower (left) gears, 1 + 2
			int   rightEdge;  ///< Threshold for the higher (right) gears, 5 + 6
		} calibration;

		AnalogInput analogAxis[2];  ///< Axis data for X and Y
		PinNum pinReverse;          ///< The pin for the reverse gear button
		bool reverseState;          ///< Buffered value for the state of the reverse gear button
	};

	/// @} Shifters


	/**
	* @brief Interface with analog handbrakes that use hall effect sensors
	*/
	class Handbrake : public Peripheral {
	public:
		/**
		* Class constructor
		*
		* @param pinAx analog pin number for the handbrake axis
		*/
		Handbrake(PinNum pinAx);

		/**
		* Initializes the pin for reading from the handbrake.
		*/
		virtual void begin();

		/**
		* Retrieves the buffered position for the handbrake axis, rescaled to a
		* nominal range using the calibration values.
		*
		* By default this is rescaled to an integer percentage (0 - 100)
		*
		* @param rMin the minimum output value
		* @param rMax the maximum output value
		*
		* @return the handbrake position, buffered and rescaled
		*/
		long getPosition(long rMin = 0, long rMax = 100) const;

		/**
		* Retrieves the buffered position for the handbrake, ignoring the
		* calibration data.
		*
		* @return the handbrake position, buffered
		*/
		int getPositionRaw() const;

		/**
		* Checks whether the handbrake's position has changed since the last update.
		*
		* @return 'true' if the position has changed, 'false' otherwise
		*/
		bool positionChanged() const { return this->changed; }

		/// @copydoc AnalogInput::setCalibration()
		void setCalibration(AnalogInput::Calibration newCal);

		/// @copydoc AnalogShifter::serialCalibration()
		void serialCalibration(Stream& iface = Serial);

	protected:
		/** @copydoc Peripheral::updateState(bool) */
		virtual bool updateState(bool connected);

	private:
		AnalogInput analogAxis;      ///< axis data for the handbrake's position
		bool changed;                ///< whether the handbrake position has changed since the previous update
	};


	/**
	* @brief Interface with the Logitech pedals (Gas, Brake, and Clutch)
	* @ingroup Pedals
	*
	* @see https://www.logitechg.com/en-us/products/driving/driving-force-racing-wheel.html
	*/
	class LogitechPedals : public ThreePedals {
	public:
		/**
		* Class constructor
		*
		* @param pinGas    the analog pin for the gas pedal potentiometer, DE-9 pin 2
		* @param pinBrake  the analog pin for the brake pedal potentiometer, DE-9 pin 3
		* @param pinClutch the analog pin for the clutch pedal potentiometer, DE-9 pin 4
		* @param pinDetect the digital pin for device detection, DE-9 pin 6. Requires a
		*                  pull-down resistor.
		*/
		LogitechPedals(PinNum pinGas, PinNum pinBrake, PinNum pinClutch, PinNum pinDetect = UnusedPin);

	private:
		DeviceConnection detectObj;  ///< detector instance for checking if the pedals are connected
	};

	/**
	* @brief Interface with the Logitech Driving Force GT pedals (Gas + Brake)
	* @ingroup Pedals
	*
	* Note that this is the older wheel made for the PS3. It is not the modern
	* "Driving Force" wheel.
	*
	* @see https://en.wikipedia.org/wiki/Logitech_Driving_Force_GT
	*/
	class LogitechDrivingForceGT_Pedals : public TwoPedals {
	public:
		/**
		* Class constructor
		*
		* @param pinGas    the analog pin for the gas pedal potentiometer, DE-9 pin 2
		* @param pinBrake  the analog pin for the brake pedal potentiometer, DE-9 pin 3
		* @param pinDetect the digital pin for device detection, DE-9 pin 4. Requires a
		*                  pull-down resistor.
		*/
		LogitechDrivingForceGT_Pedals(PinNum pinGas, PinNum pinBrake, PinNum pinDetect = UnusedPin);

	private:
		DeviceConnection detectObj;  ///< detector instance for checking if the pedals are connected
	};

	/**
	* @brief Interface with the Logitech Driving Force shifter
	* @ingroup Shifters
	* 
	* @see https://www.logitechg.com/en-us/products/driving/driving-force-shifter.941-000119.html
	*/
	class LogitechShifter : public AnalogShifter {
	public:
		/**
		* Class constructor
		* 
		* @param pinX      the analog input pin for the X axis, DE-9 pin 4
		* @param pinY      the analog input pin for the Y axis, DE-9 pin 8
		* @param pinRev    the digital input pin for the 'reverse' button, DE-9 pin 2
		* @param pinDetect the digital pin for device detection, DE-9 pin 7. Requires
		*                  a pull-down resistor.
		* 
		* @note In order to get the 'reverse' signal from the shifter, the chip select
		*       pin (DE-9 pin 3) needs to be pulled up to VCC.
		*/
		LogitechShifter(PinNum pinX, PinNum pinY, PinNum pinRev = UnusedPin, PinNum pinDetect = UnusedPin);

	private:
		DeviceConnection detectObj;  ///< detector instance for checking if the shifter is connected
	};

	/**
	* @brief Interface with the Logitech G923 shifter
	* @ingroup Shifters
	*
	* @see https://www.logitechg.com/en-us/products/driving/g923-trueforce-sim-racing-wheel.html
	*/
	using LogitechShifterG923 = LogitechShifter;

	/**
	* @brief Interface with the Logitech G29 shifter
	* @ingroup Shifters
	*
	* @see https://en.wikipedia.org/wiki/Logitech_G29
	*/
	using LogitechShifterG29 = LogitechShifter;

	/**
	* @brief Interface with the Logitech G920 shifter
	* @ingroup Shifters
	*
	* @see https://en.wikipedia.org/wiki/Logitech_G29
	*/
	using LogitechShifterG920 = LogitechShifter;

	/**
	* @brief Interface with the Logitech G27 shifter
	* @ingroup Shifters
	*
	* The G27 shifter includes the same analog shifter as the Logitech Driving
	* Force shifter (implemented in the LogitechShifter class), as well as
	* a directional pad and eight buttons.
	*
	* @see https://en.wikipedia.org/wiki/Logitech_G27
	*/
	class LogitechShifterG27 : public LogitechShifter {
	public:
		/**
		* @brief Enumeration of button values
		* 
		* Buttons 1-4 are the red buttons, from left to right. The directional
		* pad is read as four separate buttons. The black buttons use cardinal
		* directions.
		* 
		* These values represent the bit offset from LSB. Data is read in
		* MSB first.
		*/
		enum Button : uint8_t {
			BUTTON_UNUSED1    = 15,  ///< Unused shift register pin
			BUTTON_REVERSE    = 14,  ///< Reverse button (press down on the shifter)
			BUTTON_UNUSED2    = 13,  ///< Unused shift register pin
			BUTTON_SEQUENTIAL = 12,  ///< Sequential mode button (turn the dial counter-clockwise)
			BUTTON_3          = 11,  ///< 3rd red button (mid right)
			BUTTON_2          = 10,  ///< 2nd red button (mid left)
			BUTTON_4          = 9,   ///< 4th red button (far right)
			BUTTON_1          = 8,   ///< 1st red button (far left)
			BUTTON_NORTH      = 7,   ///< The top black button
			BUTTON_EAST       = 6,   ///< The right black button
			BUTTON_WEST       = 5,   ///< The left black button
			BUTTON_SOUTH      = 4,   ///< The bottom black button
			DPAD_RIGHT        = 3,   ///< Right button of the directional pad
			DPAD_LEFT         = 2,   ///< Left button of the directional pad
			DPAD_DOWN         = 1,   ///< Down button of the directional pad
			DPAD_UP           = 0,   ///< Top button of the directional pad
		};

		/**
		* Class constructor
		*
		* @param pinX      analog input pin for the X axis, DE-9 pin 4
		* @param pinY      analog input pin for the Y axis, DE-9 pin 8
		* @param pinLatch  digital output pin to pulse to latch data, DE-9 pin 3
		* @param pinClock  digital output pin to pulse as a clock, DE-9 pin 1
		* @param pinData   digital input pin to use for reading data, DE-9 pin 2
		* @param pinLed    digital output pin to light the power LED on connection,
		*                  DE-9 pin 5
		* @param pinDetect digital input pin for device detection, DE-9 pin 7.
		*                  Requires a pull-down resistor.
		*/
		LogitechShifterG27(
			PinNum pinX, PinNum pinY,
			PinNum pinLatch, PinNum pinClock, PinNum pinData,
			PinNum pinLed    = UnusedPin,
			PinNum pinDetect = UnusedPin
		);

		/**
		* Initializes the hardware pins for reading the gear states and
		* the buttons.
		*/
		virtual void begin();

		/**
		* Retrieve the state of a single button
		*
		* @param button The button to retrieve
		* @returns The state of the button
		*/
		bool getButton(Button button) const;

		/**
		* Checks whether a button has changed between updates
		*
		* @param button The button to check
		* @returns 'true' if the button's state has changed, 'false' otherwise
		*/
		bool getButtonChanged(Button button) const;

		/**
		* Get the directional pad angle in degrees
		*
		* This is useful for using the directional pad as a "hatswitch", in USB
		*
		* @returns the directional pad value in degrees (0-360), or '-1' if no
		*          directional pad buttons are pressed
		*/
		int getDpadAngle() const;

		/**
		* Checks if any of the buttons have changed since the last update
		*
		* @returns 'true' if any buttons have changed state, 'false' otherwise
		*/
		bool buttonsChanged() const;

		/**
		* Sets the state of the shifter's power LED
		*
		* If the shifter is currently connected, this function will turn the
		* power LED on and off. If the shifter is not connected, this will
		* buffer the commanded state and set the LED when the shifter is next
		* connected.
		*
		* @note The update() function must be called in order to push the
		*       commanded state to the shifter.
		*
		* @param state the state to set: 1 = on, 0 = off
		*/
		void setPowerLED(bool state);

		/**
		* Gets the commanded state of the shifter's power LED
		* 
		* @returns 'true' if the power LED is commanded to be on, 'false'
		*          if it's commanded to be off.
		*/
		bool getPowerLED() const { return this->ledState; }

	protected:
		/** @copydoc Peripheral::updateState(bool) */
		virtual bool updateState(bool connected);

	private:
		/**
		* Extracts a button value from a given data word
		* 
		* @param button The button to extract state for
		* @param data   Packed data word containing button states
		* 
		* @returns The state of the button
		*/
		static bool extractButton(Button button, uint16_t data) {
			// convert button to single bit with offset, and perform
			// a bitwise 'AND' to get the bit value
			return data & (1 << (uint8_t) button);
		}

		/**
		* Store the current button data for reference and replace it with
		* a new value
		* 
		* @param newStates The new button states to store
		*/
		void cacheButtons(uint16_t newStates);

		/**
		* Set the pin modes for all pins
		*
		* @param enabled 'true' to set the pins to their active configuration,
		*                'false' to set them to idle / safe
		*/
		void setPinModes(bool enabled);

		/**
		* Shift the button data out from the shift register
		* 
		* @returns the 16-bit data from the shift registers
		*/
		uint16_t readShiftRegisters();

		/** @copydoc AnalogShifter::readReverseButton() */
		virtual bool readReverseButton();

		// Pins for the shift register interface
		PinNum pinLatch;             ///< Pin to pulse to latch data, DE-9 pin 3
		PinNum pinClock;             ///< Pin to pulse as a clock, DE-9 pin 1
		PinNum pinData;              ///< Pin to use for reading data, DE-9 pin 2

		// Generic I/O pins
		PinNum pinLed;               ///< Pin to light the power LED, DE-9 pin 5

		// I/O state
		bool pinModesSet;            ///< Flag for whether the output pins are enabled / driven
		bool ledState;               ///< Commanded state of the power LED output, DE-9 pin 5

		// Button states
		uint16_t buttonStates;       ///< the state of the buttons, as a packed word (where 0 = unpressed and 1 = pressed)
		uint16_t previousButtons;    ///< the previous state of the buttons, for comparison
	};

	/**
	* @brief Interface with the Logitech G25 shifter
	* @ingroup Shifters
	*
	* The G25 shifter includes the same analog shifter as the Logitech Driving
	* Force shifter (implemented in the LogitechShifter class), the buttons
	* included with the G27 shifter (implemented in the LogitechShifterG27
	* class), and a mode switch between H-pattern and sequential shift modes.
	*
	* @see https://en.wikipedia.org/wiki/Logitech_G25
	*/
	class LogitechShifterG25 : public LogitechShifterG27 {
	public:
		/**
		* Class constructor
		*
		* @param pinX      analog input pin for the X axis, DE-9 pin 4
		* @param pinY      analog input pin for the Y axis, DE-9 pin 8
		* @param pinLatch  digital output pin to pulse to latch data, DE-9 pin 3
		* @param pinClock  digital output pin to pulse as a clock, DE-9 pin 7
		* @param pinData   digital input pin to use for reading data, DE-9 pin 2
		* @param pinLed    digital output pin to light the power LED on connection,
		*                  DE-9 pin 5
		* @param pinDetect digital input pin for device detection, DE-9 pin 1.
		*                  Requires a pull-down resistor.
		*/
		LogitechShifterG25(
			PinNum pinX, PinNum pinY,
			PinNum pinLatch, PinNum pinClock, PinNum pinData,
			PinNum pinLed    = UnusedPin,
			PinNum pinDetect = UnusedPin
		);

		/** @copydoc LogitechShifterG27::begin() */
		virtual void begin();

		/**
		* Check if the shifter is in sequential shifting mode
		* 
		* @returns 'true' if the shifter is in sequential shifting mode,
		*          'false' if the shifter is in H-pattern shifting mode.
		*/
		bool inSequentialMode() const;

		/**
		* Check if the sequential shifter is shifted up
		* 
		* @returns 'true' if the sequential shifter is shifted up,
		*          'false' otherwise
		*/
		bool getShiftUp() const;

		/**
		* Check if the sequential shifter is shifted down
		*
		* @returns 'true' if the sequential shifter is shifted down,
		*          'false' otherwise
		*/
		bool getShiftDown() const;

		/**
		* Calibrate the sequential shifter for more accurate shifting.
		* 
		* @param neutral      the Y position of the shifter in neutral
		* @param up           the Y position of the shifter in sequential 'up'
		* @param down         the Y position of the shifter in sequential 'down'
		* @param engagePoint  distance from neutral on Y to register a gear as
		*                     being engaged (as a percentage of distance from
		*                     neutral to Y max, 0-1)
		* @param releasePoint distance from neutral on Y to go back into neutral 
		*                     from an engaged gear (as a percentage of distance
		*                     from neutral to Y max, 0-1)
		*/
		void setCalibrationSequential(int neutral, int up, int down,
			float engagePoint  = LogitechShifterG25::CalEngagementPoint,
			float releasePoint = LogitechShifterG25::CalReleasePoint
		);

		/** @copydoc AnalogShifter::serialCalibration(Stream&) */
		void serialCalibrationSequential(Stream& iface = Serial);

	protected:
		/** @copydoc Peripheral::updateState(bool) */
		virtual bool updateState(bool connected);

	private:
		/**
		* Distance from neutral on Y to register a gear as
		* being engaged (as a percentage of distance from
		* neutral to Y max, 0-1). Used for calibration.
		*/
		static const float CalEngagementPoint; 

		/**
		* Distance from neutral on Y to go back into neutral 
		* from an engaged gear (as a percentage of distance
		* from neutral to Y max, 0-1). Used for calibration.
		*/
		static const float CalReleasePoint;

		bool   sequentialProcess;  ///< Flag to indicate whether we are processing sequential shifts
		int8_t sequentialState;    ///< Tri-state flag for the shift direction. 1 (Up), 0 (Neutral), -1 (Down).

		/*** Internal calibration struct */
		struct SequentialCalibration {
			int   upTrigger;  ///< Threshold to set the sequential shift as 'up'
			int   upRelease;  ///< Threshold to clear the 'up' sequential shift
			int downTrigger;  ///< Threshold to set the sequential shift as 'down'
			int downRelease;  ///< Threshold to clear the 'down' sequential shift
		} seqCalibration;
	};


#if defined(__AVR_ATmega32U4__) || defined(SIM_RACING_DOXYGEN)
	/**
	* Create an object for use with one of the Sim Racing Shields, designed
	* for the SparkFun Pro Micro (32U4).
	*
	* This is a convenience function, so that users with a shield don't need to
	* look up or remember the pin assignments for their hardware.
	*
	* @code{.cpp}
	* // Generic Usage
	* auto myObject = SimRacing::CreateShieldObject<ObjectType, Version>();
	*
	* // Creating a LogitechShifter object for the v2 shifter shield
	* auto myShifter = SimRacing::CreateShieldObject<LogitechShifter, 2>();
	* @endcode
	*
	* The following classes are supported for the Pedals shield, v1:
	*     * SimRacing::LogitechPedals
	*
	* The following classes are supported for the Shifter shield, v1:
	*     * SimRacing::LogitechShifter (Driving Force)
	*     * SimRacing::LogitechShifterG923 (alias)
	*     * SimRacing::LogitechShifterG920 (alias)
	*     * SimRacing::LogitechShifterG29 (alias)
	*
	* Version 2 of the shifter shield includes support for all of the classes
	* from v1, as well as the following:
	*     * SimRacing::LogitechShifterG27
	*     * SimRacing::LogitechShifterG25
	*
	* @note The default version of this template is undefined, so trying to
	*       create a class that is unsupported by the shield will generate
	*       a linker error. This is intentional.
	*
	* @tparam T       The class to create
	* @tparam Version The major version number of the shield
	*
	* @returns class instance, using the hardware pins on the shield
	*
	* @see https://github.com/dmadison/Sim-Racing-Shields
	*/
	template<class T, uint8_t Version>
	T CreateShieldObject();

	/**
	* Create a LogitechPedals object for the Pedals Shield v1
	*/
	template<>
	LogitechPedals CreateShieldObject<LogitechPedals, 1>();

	/**
	* Create a LogitechPedals object for the Pedals Shield v2
	*/
	template<>
	LogitechPedals CreateShieldObject<LogitechPedals, 2>();

	/**
	* Create a LogitechShifter object for the Shifter Shield v1
	*/
	template<>
	LogitechShifter CreateShieldObject<LogitechShifter, 1>();

	/**
	* Create a LogitechShifter object for the Shifter Shield v2
	*/
	template<>
	LogitechShifter CreateShieldObject<LogitechShifter, 2>();

	/**
	* Create a LogitechShifterG27 object for the Shifter Shield v2
	*/
	template<>
	LogitechShifterG27 CreateShieldObject<LogitechShifterG27, 2>();

	/**
	* Create a LogitechShifterG25 object for the Shifter Shield v2
	*/
	template<>
	LogitechShifterG25 CreateShieldObject<LogitechShifterG25, 2>();
#endif

}  // end SimRacing namespace

#endif
