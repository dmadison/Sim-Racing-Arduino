# Logitech G25 Shifter {#logitech_shifter_g25}

The [Logitech G25](https://en.wikipedia.org/wiki/Logitech_G25) shifter is implemented using the SimRacing::LogitechShifterG25 class. See the LogitechShifterG25_Print.ino and LogitechShifterG25_Joystick.ino examples for reference.

The G25 shifter is near-identical to the [G27 shifter](@ref logitech_shifter_g27). It includes a "sequential" shifting mode, and pins 1 and 7 of the connector are swapped (respectively: power/clock for the G25, clock/power for the G27). These pin swaps are done in the wiring between the DE-9 and internal J11 connector; the circuit board appears to be identical.

These notes are based off of disassembling my own G25 shifter, with the internal PCB marked "202339-0000 REV. A1".

## Adapters

@youtube_embed{https://www.youtube.com/embed/BVbpuYmPmm0}

You can build your own DIY USB adapter using a male DE-9 connector. This is simple to make and does not require any modifications to the shifter. The above video walks you through the process of wiring to an Arduino Leonardo.

If you want something more robust, an open source shield is available to connect the shifter to a [SparkFun Pro Micro](https://github.com/sparkfun/Pro_Micro). The design comes with a 3D printable case and custom board files so that the device appears as a "Sim Racing Shifter" over USB. You can use this shield to build an inexpensive USB HID adapter.

You can find all of the necessary files in [the project repository](https://github.com/dmadison/Sim-Racing-Shields).

## Connector

| ![DE-9_Male](DE9_Male.svg) | ![DE-9_Female](DE9_Female.svg) |
| :-----------------------: | :---------------------------: |
| DE-9 Male Connector        | DE-9 Female connector          |

<sup>DE-9 graphic from [Aeroid](https://commons.wikimedia.org/wiki/User:Aeroid) @ [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:DE9_Diagram.svg#/media/File:DE-9_Female.svg), modified for scale, colors, and creation of a complementary male version. These graphics are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).</sup>

The Logitech G25 shifter connects to the wheel base unit using [a female DE-9 connector](https://en.wikipedia.org/wiki/D-subminiature). Note that most jumper wires with [DuPont headers](https://en.wikipedia.org/wiki/Jump_wire) will not fit snugly into a DE-9 connector. For reliability and ease of use it's recommended to use a mating male DE-9 connector when interfacing with the shifter.

Note that the DE-9 connector is often erroneously referred to as DB-9. These are the same thing.

## Pinout

| Function                  | DE-9 Pin | Internal J11 Pin | Data Direction | Wire Color       | Necessary | Recommended Pin |
|---------------------------|----------|------------------|----------------|------------------|-----------|-----------------|
| Power                     | 1        | 1                | -              | Black            |           |                 |
| Data Output (SDO)         | 2        | 7                | Out            | Gray             | X         | 7               |
| Latch / Chip Select       | 3        | 5                | In             | Yellow           | X         | 5               |
| X Axis Wiper              | 4        | 3                | Out            | Orange           | X         | A0              |
| Data In (SDI) / Power LED | 5        | 2                | In             | Red              |           |                 |
| Ground                    | 6        | 8                | -              | Black (Sheathed) | X         | GND             |
| Clock (SCLK)              | 7        | 6                | In             | Purple           | X         | 6               |
| Y Axis Wiper              | 8        | 4                | Out            | Green            | X         | A2              |
| Power                     | 9        | 1                | -              | Black            | X         | VCC             |

Pin #2 (Data Output) is connected directly to the output of the EEPROM, and connected to the output of the shift registers through a 1000 Ohm resistor.

Pin #3 (Latch / Chip Select) is shared between the onboard EEPROM and the shift registers. It is floating but should typically be held HIGH by the microcontroller. It must be pulsed once (HIGH / LOW / HIGH) to latch the data into the shift registers. Holding it LOW instructs the EEPROM to listen for commands.

Pin #5 (Data In) is used exclusively by the EEPROM. It is also connected to the "Power" LED through a 330 Ohm resistor. Driving this pin LOW will turn on the LED. As the "Sequential Mode" LED is connected using a 470 Ohm resistor, I would recommend using a 100-120 Ohm resistor in series so that the pair are closer in brightness.

Pin #7 (Clock) is connected to the clock inputs of both the EEPROM and the shift registers. It is floating but should typically be held LOW by the microcontroller. Pulsing the pin HIGH (LOW / HIGH / LOW) will shift one bit of data. Be wary of driving this pin to ground without protection, as this pin is also used as a joint power input for the [Logitech Driving Force Shifter](@ref logitech_shifter) and the [Logitech G27 Shifter](@ref logitech_shifter_g27).

The power pins (#1 / #9) are connected together within the DE-9 connector. Either one can be used, but it is recommended to use pin 9 for compatibility with the other shifters.

The shifter's electronics are theoretically compatibile with both 3.3V and 5V logic. Be sure to use the appropriate voltage for the logic level of your microcontroller.

## Buttons

The shifter includes 12 user-facing buttons:

* Four black buttons in a diamond pattern
* One directional pad (D-Pad)
* Four red buttons in a straight line

The shifter also contains two internal buttons: a button on the bottom of the shift column to indicate that it's in reverse, and a button on the sequential mode dial to indicate that it's in sequential mode.

### Shift Registers

These buttons are connected to the external DE-9 connector through a pair of NXP 74HC165D parallel-to-serial shift registers.

| Button                | Register | Bit | Offset | Enum                                             |
|-----------------------|----------|-----|--------|--------------------------------------------------|
| (Unused)              | Bottom   | D7  | 15     | SimRacing::LogitechShifterG27::BUTTON_UNUSED1    |
| Reverse               | Bottom   | D6  | 14     | SimRacing::LogitechShifterG27::BUTTON_REVERSE    |
| (Unused)              | Bottom   | D5  | 13     | SimRacing::LogitechShifterG27::BUTTON_UNUSED2    |
| Sequential Mode       | Bottom   | D4  | 12     | SimRacing::LogitechShifterG27::BUTTON_SEQUENTIAL |
| Red #3                | Bottom   | D3  | 11     | SimRacing::LogitechShifterG27::BUTTON_3          |
| Red #2                | Bottom   | D2  | 10     | SimRacing::LogitechShifterG27::BUTTON_2          |
| Red #4                | Bottom   | D1  | 9      | SimRacing::LogitechShifterG27::BUTTON_4          |
| Red #1                | Bottom   | D0  | 8      | SimRacing::LogitechShifterG27::BUTTON_1          |
| Black Up              | Top      | D7  | 7      | SimRacing::LogitechShifterG27::BUTTON_NORTH      |
| Black Right           | Top      | D6  | 6      | SimRacing::LogitechShifterG27::BUTTON_EAST       |
| Black Left            | Top      | D5  | 5      | SimRacing::LogitechShifterG27::BUTTON_WEST       |
| Black Down            | Top      | D4  | 4      | SimRacing::LogitechShifterG27::BUTTON_SOUTH      |
| Directional Pad Right | Top      | D3  | 3      | SimRacing::LogitechShifterG27::DPAD_RIGHT        |
| Directional Pad Left  | Top      | D2  | 2      | SimRacing::LogitechShifterG27::DPAD_LEFT         |
| Directional Pad Down  | Top      | D1  | 1      | SimRacing::LogitechShifterG27::DPAD_DOWN         |
| Directional Pad Up    | Top      | D0  | 0      | SimRacing::LogitechShifterG27::DPAD_UP           |

Data from the shift registers can be read using the Data Output (DE-9 #2), Latch (DE-9 #3), and Clock (DE-9 #7) pins. The latch must be pulsed LOW (HIGH / LOW / HIGH), then data read out via the data output pin while the clock is pulsed repeatedly from LOW to HIGH.

All buttons will report a '1' state if they are pressed, and a '0' state if they are unpressed. Internally, all of these buttons are held to ground with 10k pull-downs.

The red buttons are numbered from left to right, 1-4. The black buttons use cardinal directions.

## EEPROM Storage

The Logitech shifter has an internal EEPROM chip, presumably for storing settings and calibration data. In my shifter this is an [ST Microelectronics M95010-W](https://www.st.com/resource/en/datasheet/m95010-w.pdf) in an SO8 package. It has 1 Kbit of memory and can be read and written to via the DE-9 connector. The EERPOM does not need to be used in order to retrieve the control surface data from the shifter.

This library does not implement EEPROM support, either for reading from the EEPROM or utilizing its data.
