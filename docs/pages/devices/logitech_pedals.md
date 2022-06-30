# Logitech Pedals {#logitech_pedals}

The 3-pedal peripheral included with the [G923 "TRUEFORCE"](https://www.logitechg.com/en-us/products/driving/g923-trueforce-sim-racing-wheel.html), [G29/G920 "Driving Force"](https://www.logitechg.com/en-us/products/driving/driving-force-racing-wheel.html), and [G27](https://en.wikipedia.org/wiki/Logitech_G27)/[G25](https://en.wikipedia.org/wiki/Logitech_G25) wheels is implemented using the SimRacing::LogitechPedals class.

The 2-pedal peripheral included with the [Logitech Driving Force GT wheel](https://en.wikipedia.org/wiki/Logitech_Driving_Force_GT) is implemented using the SimRacing::LogitechDrivingForceGT_Pedals class.

See the PedalsPrint.ino and PedalsJoystick.ino examples for reference.

## Adapters

@youtube_embed{https://www.youtube.com/embed/6Pu1qknGjy4}

The best way to connect to the pedals is to build your own DIY adapter using a female DE-9 connector. This is simple to make and does not require any modifications to the pedal base. The above video walks you through the process of wiring to an Arduino Leonardo. Be aware that the wiring is different between the 3-pedal and 2-pedal versions; the video does *not* apply to the 2-pedal version.

If you want something more robust, an open source shield is available to connect the three pedal peripheral to a [SparkFun Pro Micro](https://github.com/sparkfun/Pro_Micro). The design comes with a 3D printable case and custom board files so that the device appears as "Sim Racing Pedals" over USB. You can use this shield to build an inexpensive USB HID adapter.

You can find all of the necessary files in [the project repository](https://github.com/dmadison/Sim-Racing-Shields). Please note that this shield is also *not* compatible with the two pedal peripheral.

## Connector

| ![DE-9](DE9_Male.svg) | ![DE-9_Female](DE9_Female.svg) |
| :-----------------------: | :---------------------------: |
| DE-9 Male Connector        | DE-9 Female connector          |

<sup>DE-9 graphic from [Aeroid](https://commons.wikimedia.org/wiki/User:Aeroid) @ [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:DE9_Diagram.svg#/media/File:DE-9_Female.svg), modified for scale, colors, and creation of a complementary male version. These graphics are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).</sup>

The Logitech pedals connect to the wheel base units using [a male DE-9 connector](https://en.wikipedia.org/wiki/D-subminiature). Note that most jumper wires with [DuPont headers](https://en.wikipedia.org/wiki/Jump_wire) will not fit snugly into a DE-9 connector. For reliability and ease of use it's recommended to use a mating female DE-9 connector when interfacing with the pedals.

Note that the DE-9 connector is often erroneously referred to as DB-9. These are the same thing.

## Gas, Brake, and Clutch Pedals Pinout

Most Logitech wheels come with a combined pedal unit with a gas, brake, and clutch pedal. This connects to the wheel using a male DE-9 connector.

| Function           | DE-9 Pin | Wire Color | Necessary     | Recommended Pin |
|--------------------|----------|------------|---------------|-----------------|
| Ground             | 1        | Black      | X             | GND             |
| Gas Pedal Wiper    | 2        | Orange     | X             | A2              |
| Brake Pedal Wiper  | 3        | White      | X             | A1              |
| Clutch Pedal Wiper | 4        | Green      | X             | A0              |
| No Connection      | 5        | -          | -             | -               |
| Power              | 6        | Red        | X<sup>1</sup> | -               |
| No Connection      | 7        | -          | -             | -               |
| No Connection      | 8        | -          | -             | -               |
| Power              | 9        | Red        | X<sup>1</sup> | VCC<sup>2</sup> |

<sup>1. Both power pins are identical. Only one needs to be connected (9 is recommended). The other can be used for a detection circuit in combination with a pull-down resistor.</sup>  
<sup>2. VCC is the logic level voltage of your microcontroller. On most Arduinos this is the 5V pin.</sup>  

The pedal electronics are compatible up to at least 5V. Higher voltages may be possible but have not been tested.

## Gas + Brake Pedals Pinout

Inexpensive Logitech wheels such as the older "Driving Force GT" wheel come with a combined pedal unit with a gas and brake pedal. This connects to the wheel using a male DE-9 connector with pins 1 and 5 removed.

| Function          | DE-9 Pin | Wire Color | Necessary     | Recommended Pin |
|-------------------|----------|------------|---------------|-----------------|
| No Connection     | 1        | -          | -             | -               |
| Gas Pedal Wiper   | 2        | White      | X<sup>3</sup> | A2              |
| Brake Pedal Wiper | 3        | Green      | X<sup>3</sup> | A1              |
| Power             | 4        | Red        | X<sup>1</sup> | -               |
| No Connection     | 5        | -          | -             | -               |
| Ground            | 6        | Black      | X             | GND             |
| Gas Pedal Wiper   | 7        | White      | X<sup>3</sup> | -               |
| Brake Pedal Wiper | 8        | Green      | X<sup>3</sup> | -               |
| Power             | 9        | Red        | X<sup>1</sup> | VCC<sup>2</sup> |

<sup>1. Both power pins are identical. Only one needs to be connected (9 is recommended). The other can be used for a detection circuit in combination with a pull-down resistor.</sup>  
<sup>2. VCC is the logic level voltage of your microcontroller. On most Arduinos this is the 5V pin.</sup>  
<sup>3. The wiper connections are duplicated across two sets of pins. Only one group of these pins needs to be connected.</sup>  

The pedal electronics are compatible up to at least 5V. Higher voltages may be possible but have not been tested.
