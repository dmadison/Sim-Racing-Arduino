/*
 *  Project     Sim Racing Library for Arduino
 *  @author     David Madison
 *  @link       github.com/dmadison/Sim-Racing-Arduino
 *  @license    LGPLv3 - Copyright (c) 2024 David Madison
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
 * @details Emulates the Logitech G27 shifter as a joystick over USB.
 * @example LogitechShifterG27_Joystick.ino
 */

// This example requires the Arduino Joystick Library
// Download Here: https://github.com/MHeironimus/ArduinoJoystickLibrary

#include <SimRacing.h>
#include <Joystick.h>

//  Power (VCC): DE-9 pin 9
// Ground (GND): DE-9 pin 6
const int Pin_ShifterX      = A0;  // DE-9 pin 4
const int Pin_ShifterY      = A2;  // DE-9 pin 8

const int Pin_ShifterLatch  = 5;   // DE-9 pin 3
const int Pin_ShifterClock  = 6;   // DE-9 pin 1
const int Pin_ShifterData   = 7;   // DE-9 pin 2

// These pins require extra resistors! If you have made the proper
// connections, change the pin numbers to the ones you're using
const int Pin_ShifterDetect = SimRacing::UnusedPin;  // DE-9 pin 7, requires pull-down resistor
const int Pin_ShifterLED    = SimRacing::UnusedPin;  // DE-9 pin 5, requires 100-120 Ohm series resistor

SimRacing::LogitechShifterG27 shifter(
	Pin_ShifterX, Pin_ShifterY,
	Pin_ShifterLatch, Pin_ShifterClock, Pin_ShifterData,
	Pin_ShifterDetect, Pin_ShifterLED
);
//SimRacing::LogitechShifterG27 shifter = SimRacing::CreateShieldObject<SimRacing::LogitechShifterG27, 2>();

// Set this option to 'true' to send the shifter's X/Y position
// as a joystick. This is not needed for most games.
const bool SendAnalogAxis = false;

const int Gears[] = { 1, 2, 3, 4, 5, 6, -1 };
const int NumGears = sizeof(Gears) / sizeof(Gears[0]);

using ShifterButton = SimRacing::LogitechShifterG27::Button;
const ShifterButton Buttons[] = {
	ShifterButton::BUTTON_SOUTH,
	ShifterButton::BUTTON_EAST,
	ShifterButton::BUTTON_WEST,
	ShifterButton::BUTTON_NORTH,
	ShifterButton::BUTTON_1,
	ShifterButton::BUTTON_2,
	ShifterButton::BUTTON_3,
	ShifterButton::BUTTON_4,
};
const int NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);

const int ADC_Max = 1023;  // 10-bit on AVR

Joystick_ Joystick(
	JOYSTICK_DEFAULT_REPORT_ID,      // default report (no additional pages)
	JOYSTICK_TYPE_JOYSTICK,          // so that this shows up in Windows joystick manager
	NumGears + NumButtons,           // number of buttons (7 gears: reverse and 1-6, 8 buttons)
	1,                               // number of hat switches (1, the directional pad)
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
	bool dataChanged = shifter.update();

	if (dataChanged || SendAnalogAxis == true) {
		updateJoystick();
	}
}

void updateJoystick() {
	// keep track of which button we're updating
	// in the joystick output
	int currentButton = 0;

	// set the buttons corresponding to the gears
	for (int i = 0; i < NumGears; i++) {
		if (shifter.getGear() == Gears[i]) {
			Joystick.pressButton(currentButton);
		}
		else {
			Joystick.releaseButton(currentButton);
		}

		currentButton++;
	}

	// set the analog axes (if the option is set)
	if (SendAnalogAxis == true) {
		int x = shifter.getPosition(SimRacing::X, 0, ADC_Max);
		int y = shifter.getPosition(SimRacing::Y, 0, ADC_Max);
		Joystick.setXAxis(x);
		Joystick.setYAxis(y);
	}

	// set the buttons
	for (int i = 0; i < NumButtons; i++) {
		bool state = shifter.getButton(Buttons[i]);
		Joystick.setButton(currentButton, state);

		currentButton++;
	}

	// set the hatswitch (directional pad)
	int angle = shifter.getDpadAngle();
	Joystick.setHatSwitch(0, angle);

	// send the updated data via USB
	Joystick.sendState();
}
