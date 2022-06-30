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
 * @brief   Prints pedal position percentages over Serial.
 * @example PedalsPrint.ino
 */

#include <SimRacing.h>

const int Pin_Gas    = A2;
const int Pin_Brake  = A1;
const int Pin_Clutch = A0;

SimRacing::LogitechPedals pedals(Pin_Gas, Pin_Brake, Pin_Clutch);
//SimRacing::LogitechPedals pedals(PEDAL_SHIELD_V1_PINS);


void setup() {
	pedals.begin();  // initialize pedal pins

	// if you have one, your calibration line should go here

	Serial.begin(115200);
	while (!Serial);  // wait for connection to open

	Serial.println("Starting...");
}

void loop() {
	// send some serial data to run conversational calibration
	if (Serial.read() != -1) {
		pedals.serialCalibration();
		delay(2000);
	}

	pedals.update();

	Serial.print("Pedals:");

	if (pedals.hasPedal(SimRacing::Gas)) {
		int gasPedal = pedals.getPosition(SimRacing::Gas);
		Serial.print("\tGas: [ ");
		Serial.print(gasPedal);
		Serial.print("% ]");
	}

	if (pedals.hasPedal(SimRacing::Brake)) {
		int brakePedal = pedals.getPosition(SimRacing::Brake);
		Serial.print("\tBrake: [ ");
		Serial.print(brakePedal);
		Serial.print("% ]");
	}

	if (pedals.hasPedal(SimRacing::Clutch)) {
		int clutchPedal = pedals.getPosition(SimRacing::Clutch);
		Serial.print("\tClutch: [ ");
		Serial.print(clutchPedal);
		Serial.print("% ]");
	}

	Serial.println();
	delay(100);
}
