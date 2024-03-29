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
 * @details Reads and prints the current gear over serial.
 * @example ShiftPrint.ino
 */

#include <SimRacing.h>

const int Pin_ShifterX   = A0;
const int Pin_ShifterY   = A2;
const int Pin_ShifterRev = 2;

SimRacing::LogitechShifter shifter(Pin_ShifterX, Pin_ShifterY, Pin_ShifterRev);
//SimRacing::LogitechShifter shifter(SHIFTER_SHIELD_V1_PINS);

const unsigned long PrintSpeed = 1500;  // ms
unsigned long lastPrint = 0;


void setup() {
	shifter.begin();

	// if you have one, your calibration line should go here

	Serial.begin(115200);
	while (!Serial);  // wait for connection to open

	Serial.println("Starting...");
}

void loop() {
	// send some serial data to run conversational calibration
	if (Serial.read() != -1) {
		shifter.serialCalibration();
		delay(2000);
	}

	shifter.update();

	if (shifter.gearChanged()) {
		Serial.print("Shifted into ");
		Serial.print(shifter.getGearString());
		Serial.print(" [");
		Serial.print(shifter.getGear());
		Serial.print("]");

		Serial.print(" - XY: (");
		Serial.print(shifter.getPositionRaw(SimRacing::X));
		Serial.print(", ");
		Serial.print(shifter.getPositionRaw(SimRacing::Y));
		Serial.print(")");
		Serial.println();

		lastPrint = millis();
	}
	else {
		if(millis() - lastPrint >= PrintSpeed) {
			Serial.print("Currently in ");
			Serial.print(shifter.getGearString());
			Serial.println();

			lastPrint = millis();
		}
	}
}
