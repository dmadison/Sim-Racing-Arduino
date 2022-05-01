# Logitech Pedals {#logitech_pedals}

The 3-pedal peripheral included with the [G923 "TRUEFORCE"](https://www.logitechg.com/en-us/products/driving/g923-trueforce-sim-racing-wheel.html), [G29/G920 "Driving Force"](https://www.logitechg.com/en-us/products/driving/driving-force-racing-wheel.html), and [G27](https://en.wikipedia.org/wiki/Logitech_G27)/[G25](https://en.wikipedia.org/wiki/Logitech_G25) wheels is implemented using the SimRacing::LogitechPedals class.

The 2-pedal peripheral included with the [Logitech Driving Force GT wheel](https://en.wikipedia.org/wiki/Logitech_Driving_Force_GT) is implemented using the SimRacing::LogitechDrivingForceGT_Pedals class.

See the PedalsPrint.ino and PedalsJoystick.ino examples for reference.

## Connector

| ![DE9](DE9_Male.svg) | ![DE9_Female](DE9_Female.svg) |
| :-----------------------: | :---------------------------: |
| DE9 Male Connector        | DE9 Female connector          |

<sup>DE9 graphic from [Aeroid](https://commons.wikimedia.org/wiki/User:Aeroid) @ [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:DE9_Diagram.svg#/media/File:DE-9_Female.svg), modified for scale, colors, and creation of a complementary male version. These graphics are licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).</sup>

The Logitech pedals connect to the wheel base units using [a male DE9 connector](https://en.wikipedia.org/wiki/D-subminiature). Note that most jumper wires with [DuPont headers](https://en.wikipedia.org/wiki/Jump_wire) will not fit snugly into a DE9 connector. For reliability and ease of use it's recommended to use a mating female DE9 connector when interfacing with the pedals.

Note that the DE9 connector is often erroneously referred to as DB9. These are the same thing.

## Gas, Brake, and Clutch Pedals Pinout

Most Logitech wheels come with a combined pedal unit with a gas, brake, and clutch pedal. This connects to the wheel using a male DE9 connector.

| Function           | DE9 Pin | Wire Color | Necessary     | Recommended Pin |
|--------------------|---------|------------|---------------|-----------------|
| Ground             | 1       | Black      | X             | GND             |
| Gas Pedal Wiper    | 2       | Orange     | X             | A2              |
| Brake Pedal Wiper  | 3       | White      | X             | A1              |
| Clutch Pedal Wiper | 4       | Green      | X             | A0              |
| No Connection      | 5       | -          | -             | -               |
| Power              | 6       | Red        | X<sup>1</sup> | -               |
| No Connection      | 7       | -          | -             | -               |
| No Connection      | 8       | -          | -             | -               |
| Power              | 9       | Red        | X<sup>1</sup> | VCC<sup>2</sup> |

<sup>1. Both power pins are identical. Only one needs to be connected (9 is recommended). The other can be used for a detection circuit in combination with a pull-down resistor.</sup>  
<sup>2. VCC is the logic level voltage of your microcontroller. On most Arduinos this is the 5V pin.</sup>  

The pedal electronics are compatible up to at least 5V. Higher voltages may be possible but have not been tested.

## Gas + Brake Pedals Pinout

Inexpensive Logitech wheels such as the older "Driving Force GT" wheel come with a combined pedal unit with a gas and brake pedal. This connects to the wheel using a male DE9 connector with pins 1 and 5 removed.

| Function          | DE9 Pin | Wire Color | Necessary     | Recommended Pin |
|-------------------|---------|------------|---------------|-----------------|
| No Connection     | 1       | -          | -             | -               |
| Gas Pedal Wiper   | 2       | White      | X<sup>3</sup> | A2              |
| Brake Pedal Wiper | 3       | Green      | X<sup>3</sup> | A1              |
| Power             | 4       | Red        | X<sup>1</sup> | -               |
| No Connection     | 5       | -          | -             | -               |
| Ground            | 6       | Black      | X             | GND             |
| Gas Pedal Wiper   | 7       | White      | X<sup>3</sup> | -               |
| Brake Pedal Wiper | 8       | Green      | X<sup>3</sup> | -               |
| Power             | 9       | Red        | X<sup>1</sup> | VCC<sup>2</sup> |

<sup>1. Both power pins are identical. Only one needs to be connected (9 is recommended). The other can be used for a detection circuit in combination with a pull-down resistor.</sup>  
<sup>2. VCC is the logic level voltage of your microcontroller. On most Arduinos this is the 5V pin.</sup>  
<sup>3. The wiper connections are duplicated across two sets of pins. Only one group of these pins needs to be connected.</sup>  

The pedal electronics are compatible up to at least 5V. Higher voltages may be possible but have not been tested.
