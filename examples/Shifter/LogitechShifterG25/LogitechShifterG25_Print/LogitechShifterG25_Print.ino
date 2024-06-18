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
 * @details Reads from the Logitech G25 shifter and prints the data over serial.
 * @example LogitechShifterG25_Print.ino
 */

#include <SimRacing.h>

const int Pin_ShifterX      = A0;  // DE-9 pin 4
const int Pin_ShifterY      = A2;  // DE-9 pin 8

const int Pin_ShifterLatch  = 5;   // DE-9 pin 3
const int Pin_ShifterClock  = 6;   // DE-9 pin 7
const int Pin_ShifterData   = 7;   // DE-9 pin 2

// These pins require extra resistors! If you have made the proper
// connections, change the pin numbers to the ones you're using
const int Pin_ShifterDetect = SimRacing::UnusedPin;  // DE-9 pin 1, requires pull-down resistor
const int Pin_ShifterLED    = SimRacing::UnusedPin;  // DE-9 pin 5, requires 100-120 Ohm series resistor

SimRacing::LogitechShifterG25 shifter(
	Pin_ShifterX, Pin_ShifterY,
	Pin_ShifterLatch, Pin_ShifterClock, Pin_ShifterData,
	Pin_ShifterDetect, Pin_ShifterLED
);

// alias so we don't need to type so much
using ShifterButton = SimRacing::LogitechShifterG25::Button;

// forward-declared functions for non-Arduino environments
void printConditional(bool state, char pressed);
void printButton(ShifterButton button, char pressed);
void printShifter();

const unsigned long PrintSpeed = 1500;  // ms
unsigned long lastPrint = 0;


void setup() {
	shifter.begin();

	// if you have one, your calibration line should go here

	Serial.begin(115200);
	while (!Serial);  // wait for connection to open

	Serial.println("Logitech G25 Starting...");
}

void loop() {
	// send some serial data to run conversational calibration
	if (Serial.read() != -1) {
		shifter.serialCalibration();
		shifter.serialCalibrationSequential();
		delay(2000);
	}

	bool dataChanged = shifter.update();

	// if data has changed, print immediately
	if (dataChanged) {
		Serial.print("! ");
		printShifter();
	}

	// otherwise, print if we've been idle for awhile
	if (millis() - lastPrint >= PrintSpeed) {
		Serial.print("  ");
		printShifter();
	}
}

void printConditional(bool state, char pressed) {
	if (state == true) {
		Serial.print(pressed);
	}
	else {
		Serial.print('_');
	}
}

void printButton(ShifterButton button, char pressed) {
	bool state = shifter.getButton(button);
	printConditional(state, pressed);
}

void printShifter() {
	// if in sequential mode, print up/down
	if (shifter.inSequentialMode()) {
		Serial.print("S:[");
		printConditional(shifter.getShiftUp(),   '+');
		printConditional(shifter.getShiftDown(), '-');
		Serial.print(']');
	}
	// otherwise in H-pattern mode, print the gear
	else {
		Serial.print("H: [");
		Serial.print(shifter.getGearChar());
		Serial.print("]");
	}

	// print X/Y position of shifter
	Serial.print(" - XY: (");
	Serial.print(shifter.getPositionRaw(SimRacing::X));
	Serial.print(", ");
	Serial.print(shifter.getPositionRaw(SimRacing::Y));
	Serial.print(") ");

	// print directional pad
	printButton(ShifterButton::DPAD_LEFT,    '<');
	printButton(ShifterButton::DPAD_UP,      '^');
	printButton(ShifterButton::DPAD_DOWN,    'v');
	printButton(ShifterButton::DPAD_RIGHT,   '>');
	Serial.print(' ');

	// print black buttons
	printButton(ShifterButton::BUTTON_NORTH, 'N');
	printButton(ShifterButton::BUTTON_SOUTH, 'S');
	printButton(ShifterButton::BUTTON_EAST,  'E');
	printButton(ShifterButton::BUTTON_WEST,  'W');
	Serial.print(' ');

	// print red buttons
	printButton(ShifterButton::BUTTON_1,     '1');
	printButton(ShifterButton::BUTTON_2,     '2');
	printButton(ShifterButton::BUTTON_3,     '3');
	printButton(ShifterButton::BUTTON_4,     '4');

	Serial.println();

	lastPrint = millis();
}
