# USB Adapter FAQ {#usb_adapter_faq}

This page constains answers to frequently asked questions (FAQ) about building USB adapters using the [Sim Racing Library for Arduino](https://github.com/dmadison/Sim-Racing-Arduino).

You can find my tutorial videos for building your own USB adapters [on YouTube](https://www.youtube.com/playlist?list=PLTboGmshZ5EIWQSYEjdrIFqgc2J6sLCSa).


### I have a G923 / G920 / G29 shifter. Why do you call it the "Driving Force" shifter?

[That's what Logitech calls it](https://www.logitechg.com/en-us/products/driving/driving-force-shifter.html).


### What shifters are compatible with these DIY USB adapters?

I have made DIY USB adapter tutorial videos for the [Logitech Driving Force shifter](@ref logitech_shifter), the [Logitech G27 shifter](@ref logitech_shifter_g27), and the [Logitech G25 shifter](@ref logitech_shifter_g25).

All of these adapters are **only** compatible with the advertised shifter (i.e. if you made an adapter for the G25, it's only compatible with the G25). The exception to this is the G27 adapter, which is forwards-compatible with the Driving Force shifter. Do not connect an incompatible shifter to your adapter, you may damage the shifter or the Arduino!


### Can I make an adapter that supports all three shifters?

Yes, but it requires some additional resistors. The [Sim Racing Shifter Shield](https://github.com/dmadison/Sim-Racing-Shields/) I designed supports all three shifters, as does its included firmware. Take a look at the schematic for reference.


### Can I make a USB adapter with both a shifter and pedals?

Yes! Although you'll need to get your hands a little dirty with the code.

You'll need to change the wiring to the Arduino board so that you can fit both peripherals. None of the devices are particularly picky about which pins you use, so there's a lot of flexibility there. The only restrictions are that the power pins (5V/GND) need to stay the same, and the analog outputs need to connect to the analog pins (A0-A5). I would recommend changing the pins on the existing examples and testing them out before continuing further.

You will also need to combine the two example codes together. Believe me, this is easier than it sounds. I'd recommend opening two code windows side-by-side and copying between them. Add the new lines, remove duplicate lines, and if two lines are different do your best to reason out why that is. The most complicated bit is the `Joystick` object definition, which needs to change based on what you're outputting to USB. Hopefully the comments (the `//` bits) are clear enough.

The library itself does not need to be modified, only the example code. If you run into any trouble there are a number of great learning resources online, including [the Arduino forums](https://forum.arduino.cc/) and [the Arduino subreddit](https://www.reddit.com/r/arduino). Good luck!


### Can I use an Arduino Uno / Arduino Nano / Arduino Mega instead of an Arduino Leonardo?

Short answer: **No**.

Long answer: *Maybe*. But it requires a specific variant of the board and it's significantly more effort.

The tutorials suggest using an [Arduino Leonardo](https://docs.arduino.cc/hardware/leonardo/) because the onboard microcontroller (the ATmega32U4) has a hardware USB controller and female headers that don't require soldering. This means that the library code can set the USB descriptors and tell the microcontroller to behave as a USB human interface device (HID), then control the output based on the sim racing peripheral.

In contrast, the microcontrollers on the [Arduino Uno (ATmega328P)](https://docs.arduino.cc/hardware/uno-rev3/), [Arduino Nano (ATmega328P)](https://docs.arduino.cc/hardware/nano/), and [Arduino Mega (ATmega2560)](https://docs.arduino.cc/hardware/mega-2560/) do **not** have a USB controller. Instead, they use a secondary integrated circuit (IC) to convert the serial data (UART) into USB. This means that the library code *cannot* set the USB descriptors to tell the microcontroller to behave as a USB HID device.

On the Arduino Nano and on most low cost clones of the Arduino Uno and Arduino Mega, that IC is the [FT232](https://ftdichip.com/products/ft232rl/) or a knockoff like the CH340. These chips are unable to act as a USB adapter.

On genuine and more expensive Arduino Unos and Arduino Megas, that IC is the ATmega16U2 - another microcontroller, and the brother of the ATmega32U4 onboard the Leonardo. With these boards it *is* technically possible to use them as a USB adapter, although it's not easy.

To convert the ATmega16U2 into an HID device you can install the [UnoJoy](https://github.com/AlanChatham/UnoJoy) firmware. This is tricky to do over USB (using "Device Firmware Update" / DFU mode), but can be simplified using a hardware ICSP programmer. There are further instructions in that repository's documentation.

You will also need to modify the library examples to use the UnoJoy library in place of the Joystick library to send serial commands to the ATmega16U2. This should be relatively straightforward, but you will need to understand some basic C/C++ programming.

To recap, the process is:

    1. Inspect the board to verify that you have a compatible model (Arduino Uno or Arduino Mega with an ATmega16U2 as the USB to serial interface)
    2. Reset the ATmega16U2 into DFU mode by shorting the reset pins on the ICSP header (or) attach an external hardware programmer
    3. Flash UnoJoy firmware to the ATmega16U2
    4. Rewrite the library examples to use the UnoJoy library in place of the Joystick library for USB output
    5. Upload the code to the Uno (ATmega328P)

Fair warning that you may have some difficulty getting the ATmega16U2 into DFU mode, and you may have some difficulty getting it back to functioning as a "regular" Arduino board afterwards. It is also possible to "brick" the board and render it in an unusable state. This is often recoverable but requires a hardware ICSP programmer.


### Can I use ______ microcontroller instead?

Maybe! The library itself should be compatible with most development boards that support the Arduino framework. Try to compile one of the library examples for your microcontroller. If it compiles, it will probably work!

The [Joystick library](https://github.com/MHeironimus/ArduinoJoystickLibrary/) supports a more limited subset of boards, and you may need to rewrite the Sim Racing Library example code to use a different USB library.


### The Arduino Leonardo is really big. Is there a smaller board I can use?

Yes! The [SparkFun Pro Micro (ATmega32U4)](https://www.sparkfun.com/sparkfun-qwiic-pro-micro-usb-c-atmega32u4.html) has the same microcontroller and the same pinout. It's also the microcontroller that's compatible with the [Sim Racing Shields](https://github.com/dmadison/Sim-Racing-Shields/). But because it doesn't have female headers, you must solder to the pins.


### The SparkFun Pro Micro doesn't have a 5V pin. Which pin should I use?

You should use the VCC pin for power in place of the 5V pin.


### I don't know how to solder. How do I connect multiple wires to one pin?

I would recommend using [Wago 221 Series](https://www.wago.com/us/f/222-series-lever-nuts) or [Wago 222 Series Lever-Nuts](https://www.wago.com/us/f/221-series-levernuts). You can find them at many hardware stores, they're easy to use and make a solid connection.

You can also use a more traditional solderless breadboard. These are common prototyping tools, but they aren't a great long term solution as the wires can come loose.


### Why can't I just use a DE-9 to USB adapter?

The [DE-9 connector](https://en.wikipedia.org/wiki/D-subminiature) (also commonly called the "DB9" connector) has historically been used for serial devices using the [RS-232 standard](https://en.wikipedia.org/wiki/RS-232). Most common "DE-9 to USB" adapters you will find are, in fact, RS-232 to USB adapters.

Although they use the DE-9 connector, none of the Logitech sim racing peripherals in this library are RS-232 devices. At best, the adapter will not work. At worst, you may damage the adapter or your sim racing peripheral.
