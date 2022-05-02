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
 * @brief   Emulates the handbrake as a joystick over USB.
 * @example HandbrakeJoystick.ino
 */

// This example requires the Arduino Joystick Library
// Download Here: https://github.com/MHeironimus/ArduinoJoystickLibrary

#include <SimRacing.h>
#include <Joystick.h>

const int Pin_Handbrake = A2;

SimRacing::Handbrake handbrake(Pin_Handbrake);

Joystick_ Joystick(
	JOYSTICK_DEFAULT_REPORT_ID,          // default report (no additional pages)
	JOYSTICK_TYPE_JOYSTICK,              // so that this shows up in Windows joystick manager
	0,                                   // number of buttons (none)
	0,                                   // number of hat switches (none)
	false, false,                        // no X and Y axes
	true,                                // include Z axis for the handbrake
	false, false, false, false, false, false, false, false);  // no other axes

const int ADC_Max = 1023;  // max value of the analog inputs, 10-bit on AVR boards


void setup() {
	handbrake.begin();  // initialize handbrake pins

	// if you have one, your calibration line should go here
	
	Joystick.begin(false);  // 'false' to disable auto-send

	Joystick.setZAxisRange(0, ADC_Max);
}

void loop() {
	handbrake.update();

	int pos = handbrake.getPosition(0, ADC_Max);
	Joystick.setZAxis(pos);

	Joystick.sendState();
}
