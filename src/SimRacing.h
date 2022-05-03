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
		* @param pin the pin number being read. Can be 'NOT_A_PIN' to disable.
		* @param invert whether the input is inverted, so 'LOW' is detected instead of 'HIGH'
		* @param detectTime the amount of time, in ms, the input must be stable for
		*        before it's interpreted as 'detected'
		*/
		DeviceConnection(uint8_t pin, bool invert = false, unsigned long detectTime = 250);

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
		* Allows the user to change the stable period of the detector.
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

		const uint8_t Pin;           ///< The pin number being read from. Can be 'NOT_A_PIN' to disable
		const bool Inverted;         ///< Whether the input is inverted, so 'LOW' is detected instead of 'HIGH'
		unsigned long stablePeriod;  ///< The amount of time the input must be stable for (ms)

		ConnectionState state;     ///< The current state of the connection
		bool pinState;             ///< Buffered state of the input pin, accounting for inversion
		unsigned long lastChange;  ///< Timestamp of the last pin change, in ms (using millis())
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
		* @param p the I/O pin for this input (Arduino numbering)
		*/
		AnalogInput(uint8_t p);

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
		const uint8_t Pin = NOT_A_PIN;  ///< the digital pin number for this input
		int position;                   ///< the axis' position in its range, buffered
		Calibration cal;                ///< the calibration values for the axis
	};


	/**
	* @brief Abstract class for all peripherals
	*/
	class Peripheral {
	public:
		/**
		* Initialize the hardware (if necessary)
		*/
		virtual void begin() {};

		/**
		* Perform a poll of the hardware to refresh the class state
		*
		* @return 'true' if device state changed, 'false' otherwise
		*/
		virtual bool update() = 0;

		/** @copydoc DeviceConnection::isConnected() */
		virtual bool isConnected() const { return true; }
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
		* @param dataPtr pointer to the analog input data managed by the class, stored elsewhere
		* @param nPedals the number of pedals stored in said data pointer
		* @param detectPin the digital pin for device detection (high is detected)
		*/
		Pedals(AnalogInput* dataPtr, uint8_t nPedals, uint8_t detectPin);

		/** @copydoc Peripheral::begin() */
		virtual void begin();

		/** @copydoc Peripheral::update() */
		virtual bool update();

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
		
		/** @copydoc Peripheral::isConnected() */
		bool isConnected() const { return detector.isConnected(); }

		/**
		* Utility function to get the string name for each pedal.
		*
		* @param pedal the pedal to get the name of
		* @return the name of the pedal, as a String
		*/
		static String getPedalName(PedalID pedal);

	private:
		AnalogInput* pedalData;     ///< pointer to the pedal data
		const int NumPedals;        ///< number of pedals managed by this class
		DeviceConnection detector;  ///< detector instance for checking if the pedals are connected
	};


	/**
	* @brief Pedal implementation for devices with only gas and brake
	*/
	class TwoPedals : public Pedals {
	public:
		/**
		* Class constructor
		*
		* @param gasPin the analog pin for the gas pedal potentiometer
		* @param brakePin the analog pin for the brake pedal potentiometer
		* @param detectPin the digital pin for device detection (high is detected)
		*/
		TwoPedals(uint8_t gasPin, uint8_t brakePin, uint8_t detectPin = NOT_A_PIN);

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
		* @param gasPin the analog pin for the gas pedal potentiometer
		* @param brakePin the analog pin for the brake pedal potentiometer
		* @param clutchPin the analog pin for the clutch pedal potentiometer
		* @param detectPin the digital pin for device detection (high is detected)
		*/
		ThreePedals(uint8_t gasPin, uint8_t brakePin, uint8_t clutchPin, uint8_t detectPin = NOT_A_PIN);

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
		* Class constructor
		* 
		* @param min the lowest gear possible
		* @param max the highest gear possible
		*/
		Shifter(int8_t min, int8_t max);
	
		/**
		* Returns the currently selected gear.
		*
		* Will either be reverse (-1), neutral (0), or the current gear
		* indexed at 1 (1st gear is 1, 2nd gear is 2, etc.).
		*
		* @return current gear index
		*/
		int8_t getGear() const { return currentGear; }

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
		bool gearChanged() const { return changed; }

		/**
		* Retrieves the minimum possible gear index.
		*
		* @return the lowest gear index
		*/
		int8_t getGearMin() { return MinGear; }

		/**
		* Retrieves the maximum possible gear index.
		*
		* @return the highest gear index
		*/
		int8_t getGearMax() { return MaxGear; }

	protected:
		const int8_t MinGear;  ///< the lowest selectable gear
		const int8_t MaxGear;  ///< the highest selectable gear

		int8_t currentGear;    ///< index of the current gear
		bool changed;          ///< whether the gear has changed since the previous update
	};


	/**
	* @brief Interface with shifters using two potentiometers for gear position
	*/
	class AnalogShifter : public Shifter {
	public:
		/**
		* Class constructor
		* 
		* @param pinX the analog input pin for the X axis
		* @param pinY the analog input pin for the Y axis
		* @param pinRev the digital input pin for the 'reverse' button
		* @param detectPin the digital pin for device detection (high is detected)
		*/
		AnalogShifter(uint8_t pinX, uint8_t pinY, uint8_t pinRev = NOT_A_PIN, uint8_t detectPin = NOT_A_PIN);

		/**
		* Initializes the hardware pins for reading the gear states.
		* 
		* Should be called in `setup()` before reading from the shifter.
		*/
		virtual void begin();

		/**
		* Polls the hardware to update the current gear state.
		*
		* @return 'true' if the gear has changed, 'false' otherwise
		*/
		virtual bool update();

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

		/** @copydoc Peripheral::isConnected() */
		bool isConnected() const { return detector.isConnected(); }

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
		const uint8_t PinReverse;   ///< The pin for the reverse gear button
		DeviceConnection detector;  ///< detector instance for checking if the shifter is connected
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
		* @param detectPin the digital pin for device detection (high is detected)
		*/
		Handbrake(uint8_t pinAx, uint8_t detectPin = NOT_A_PIN);

		/**
		* Initializes the pin for reading from the handbrake.
		*/
		virtual void begin();

		/**
		* Polls the handbrake to update its position.
		*
		* @return 'true' if the gear has changed, 'false' otherwise
		*/
		virtual bool update();

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

		/// @copydoc AnalogInput::setCalibration()
		void setCalibration(AnalogInput::Calibration newCal);

		/// @copydoc AnalogShifter::serialCalibration()
		void serialCalibration(Stream& iface = Serial);

		/** @copydoc Peripheral::isConnected() */
		bool isConnected() const { return detector.isConnected(); }

	private:
		AnalogInput analogAxis;     ///< axis data for the handbrake's position
		DeviceConnection detector;  ///< detector instance for checking if the handbrake is connected
	};


	/**
	* @brief Interface with the Logitech pedals (Gas, Brake, and Clutch)
	* @ingroup Pedals
	*
	* @see https://www.logitechg.com/en-us/products/driving/driving-force-racing-wheel.html
	*/
	class LogitechPedals : public ThreePedals {
	public:
		/** @copydoc ThreePedals::ThreePedals */
		LogitechPedals(uint8_t gasPin, uint8_t brakePin, uint8_t clutchPin, uint8_t detectPin = NOT_A_PIN);
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
		/** @copydoc TwoPedals::TwoPedals */
		LogitechDrivingForceGT_Pedals(uint8_t gasPin, uint8_t brakePin, uint8_t detectPin = NOT_A_PIN);
	};

	/**
	* @brief Interface with the Logitech Driving Force shifter
	* @ingroup Shifters
	* 
	* @see https://www.logitechg.com/en-us/products/driving/driving-force-shifter.941-000119.html
	*/
	class LogitechShifter : public AnalogShifter {
	public:
		/** @copydoc AnalogShifter::AnalogShifter */
		LogitechShifter(uint8_t pinX, uint8_t pinY, uint8_t pinRev = NOT_A_PIN, uint8_t detectPin = NOT_A_PIN);
	};


#if defined(__AVR_ATmega32U4__) || defined(SIM_RACING_DOXYGEN)
	/**
	* Pin definitions for the Parts Not Included Logitech Shifter Shield,
	* designed for the SparkFun Pro Micro:
	* 
	* * X Wiper: A1
	* * Y Wiper: A0
	* * Reverse Pin: 14
	* * Detect Pin: A2
	* 
	* This macro can be inserted directly into the constructor in place of the
	* normal pin definitions:
	* 
	* @code{.cpp}
	* SimRacing::LogitechShifter shifter(SHIFTER_SHIELD_V1_PINS);
	* @endcode
	*/
	#define SHIFTER_SHIELD_V1_PINS A1, A0, 14, A2

	/**
	* Pin definitions for the Parts Not Included Logitech Pedals Shield,
	* designed for the SparkFun Pro Micro:
	*
	* * Gas Wiper:    A2
	* * Brake Wiper:  A1
	* * Clutch Wiper: A0
	* * Detect Pin:   10
	*
	* This macro can be inserted directly into the constructor in place of the
	* normal pin definitions:
	*
	* @code{.cpp}
	* SimRacing::LogitechPedals pedals(PEDAL_SHIELD_V1_PINS);
	* @endcode
	*/
	#define PEDAL_SHIELD_V1_PINS A2, A1, A0, 10
#endif

}  // end SimRacing namespace

#endif
