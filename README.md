# Sim Racing Library for Arduino
[![arduino-library-badge](https://www.ardu-badge.com/badge/Sim%20Racing%20Library.svg?)](https://www.ardu-badge.com/Sim%20Racing%20Library) [![Build Status](https://github.com/dmadison/Sim-Racing-Arduino/workflows/build/badge.svg)](https://github.com/dmadison/Sim-Racing-Arduino/actions/workflows/ci.yml) [![Documentation](https://img.shields.io/badge/Docs-Doxygen-blue.svg)](http://dmadison.github.io/Sim-Racing-Arduino/docs/index.html)
[![LGPL license](https://img.shields.io/badge/License-LGPL-orange.svg)](https://github.com/dmadison/Sim-Racing-Arduino/blob/master/LICENSE)

The [Sim Racing Library](https://github.com/dmadison/Sim-Racing-Arduino/) is an Arduino library for interfacing sim racing equipment such as pedals and gear shifters with Arduino development boards.

## Getting Started

Install the library using either [the .zip file from the latest release](https://github.com/dmadison/Sim-Racing-Arduino/releases/latest/) or by searching for "Sim Racing" in the libraries manager of the Arduino IDE. [See the Arduino documentation on how to install libraries for more information.](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries)

[Read the documentation](http://dmadison.github.io/Sim-Racing-Arduino/docs/supported_devices.html) to check if your device is supported by the library. Follow the wiring instructions on the device page to connect your device to the Arduino.

Run one of the library examples in the Arduino IDE by going to `File -> Examples -> Sim Racing Library`. For all peripherals, call `begin()` to initialize the class and `update()` to refresh with new data.

## Supported Devices

### Generic Devices
* [Two pedal peripherals (gas + brake)](https://dmadison.github.io/Sim-Racing-Arduino/docs/class_sim_racing_1_1_two_pedals.html)
* [Three pedal peripherals (gas, brake, clutch)](https://dmadison.github.io/Sim-Racing-Arduino/docs/class_sim_racing_1_1_three_pedals.html)
* [Analog shifters](https://dmadison.github.io/Sim-Racing-Arduino/docs/class_sim_racing_1_1_analog_shifter.html)
* [Analog handbrakes](https://dmadison.github.io/Sim-Racing-Arduino/docs/class_sim_racing_1_1_handbrake.html)

### Commercial Devices
* [Logitech Two Pedal Peripheral (Gas, Brake)](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_pedals.html)
* [Logitech Three Pedal Peripheral (Gas, Brake, Clutch)](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_pedals.html)
* [Logitech Driving Force Shifter (G923 / G920 / G29)](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_shifter.html)
* [Logitech G25 / G27 Shifter](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_shifter_g25.html)

## Adapters

Open source shields are available to connect the [Logitech Three Pedal Peripheral](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_pedals.html) and the [Logitech Driving Force Shifter](http://dmadison.github.io/Sim-Racing-Arduino/docs/logitech_shifter.html) to a [SparkFun Pro Micro](https://github.com/sparkfun/Pro_Micro). The design comes with a 3D printable case and custom board files so that the adapter appears with a custom identity and "Sim Racing" name over USB. You can use these shields to build an inexpensive USB HID adapter.

You can find all of the necessary files in [the project repository](https://github.com/dmadison/Sim-Racing-Shields).

## License

This library is licensed under the terms of the [GNU Lesser General Public License (LGPL)](https://www.gnu.org/licenses/lgpl.html), either version 3 of the License, or (at your option) any later version. See the [LICENSE](https://github.com/dmadison/Sim-Racing-Arduino/blob/master/LICENSE) file for more information.
