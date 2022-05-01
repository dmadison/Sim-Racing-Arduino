# Supported Devices

### Commercial Devices:

- @subpage logitech_pedals
- @subpage logitech_shifter

### Generic Devices

#### Pedals

The library supports generic pedal devices that connect via the microcontroller's analog to digital converter (ADC).

* Two pedal setups (gas + brake) use the `SimRacing::TwoPedals` class.
* Three pedal setups (gas, brake, clutch) use the `SimRacing::ThreePedals` class.

#### Shifters

The library supports generic shifting devices that record gear position using a pair of potentiometers. These are supported as part of the `SimRacing::AnalogShifter` class.
