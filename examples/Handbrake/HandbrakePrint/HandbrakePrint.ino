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
 * @brief   Prints handbrake position percentage over Serial.
 * @example HandbrakePrint.ino
 */

#include <SimRacing.h>

const int Pin_Handbrake = A2;

SimRacing::Handbrake handbrake(Pin_Handbrake);


void setup() {
	handbrake.begin();  // initialize handbrake pins

	// if you have one, your calibration line should go here

	Serial.begin(115200);
	while (!Serial);  // wait for connection to open

	Serial.println("Starting...");
}

void loop() {
	// send some serial data to run conversational calibration
	if (Serial.read() != -1) {
		handbrake.serialCalibration();
		delay(2000);
	}

	handbrake.update();

	Serial.print("Handbrake: ");

	int pos = handbrake.getPosition();
	Serial.print(pos);
	Serial.print("%");
	Serial.println();

	delay(100);
}
