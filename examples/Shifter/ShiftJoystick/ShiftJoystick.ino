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

 /**
 * @brief   Emulates the shifter as a joystick over USB.
 * @example ShiftJoystick.ino
 */

// This example requires the Arduino Joystick Library
// Download Here: https://github.com/MHeironimus/ArduinoJoystickLibrary

#include <SimRacing.h>
#include <Joystick.h>

// Set this option to 'true' to send the shifter's X/Y position
// as a joystick. This is not needed for most games.
const bool SendAnalogAxis = false;

// Set this option to 'true' to send the raw state of the reverse
// trigger as its own button. This is not needed for any racing
// games, but can be useful for custom controller purposes.
const bool SendReverseRaw = false;

const int Pin_ShifterX   = A0;
const int Pin_ShifterY   = A2;
const int Pin_ShifterRev = 2;

SimRacing::LogitechShifter shifter(Pin_ShifterX, Pin_ShifterY, Pin_ShifterRev);

const int Gears[] = { 1, 2, 3, 4, 5, 6, -1 };
const int NumGears = sizeof(Gears) / sizeof(Gears[0]);

const int ADC_Max = 1023;  // 10-bit on AVR

Joystick_ Joystick(
	JOYSTICK_DEFAULT_REPORT_ID,      // default report (no additional pages)
	JOYSTICK_TYPE_JOYSTICK,          // so that this shows up in Windows joystick manager
	NumGears + SendReverseRaw,       // number of buttons (7 gears: reverse and 1-6)
	0,                               // number of hat switches (none)
	SendAnalogAxis, SendAnalogAxis,  // include X and Y axes for analog output, if set above
	false, false, false, false, false, false, false, false, false);  // no other axes

void updateJoystick();  // forward-declared function for non-Arduino environments


void setup() {
	shifter.begin();

	// if you have one, your calibration line should go here
	
	Joystick.begin(false);  // 'false' to disable auto-send
	Joystick.setXAxisRange(0, ADC_Max);
	Joystick.setYAxisRange(ADC_Max, 0);  // invert axis so 'up' is up

	updateJoystick();  // send initial state
}

void loop() {
	shifter.update();

	if (SendAnalogAxis == true || shifter.gearChanged()) {
		updateJoystick();
	}
}

void updateJoystick() {
	// set the buttons corresponding to the gears
	for (int i = 0; i < NumGears; i++) {
		if (shifter.getGear() == Gears[i]) {
			Joystick.pressButton(i);
		}
		else {
			Joystick.releaseButton(i);
		}
	}

	// set the analog axes (if the option is set)
	if (SendAnalogAxis == true) {
		int x = shifter.getPosition(SimRacing::X, 0, ADC_Max);
		int y = shifter.getPosition(SimRacing::Y, 0, ADC_Max);
		Joystick.setXAxis(x);
		Joystick.setYAxis(y);
	}

	// set the reverse button (if the option is set)
	if (SendReverseRaw == true) {
		bool reverseState = shifter.getReverseButton();
		Joystick.setButton(NumGears, reverseState);  // "NumGears" is the 0-indexed max gear + 1
	}

	Joystick.sendState();
}
