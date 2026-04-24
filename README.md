# NEC-IR-Controller
NEC protocol IR transmitter and receiver using Arduino Nano with PWM brightness Control using two arduino nano boards, one for the transmitter that acts as a DIY remote and one for the receiver where the led
that which brigthness is control is connected to Pin 5 of the arduino Nano.

n NEC IR transmitter and receiver built with two Arduino Nano boards.
The transmitter sends a custom NEC protocol signal when a button is pressed.
The receiver decodes the signal and controls LED brightness using PWM.

## Hardware

### Transmitter
- Arduino Nano
- IR transmitter module (DAT → pin 11, VCC → 5V, GND → GND)
- Tactile switch (one leg → pin 2, other leg → GND)

### Receiver
- Arduino Nano
- IR receiver module (OUT → pin 2, VCC → 5V, GND → GND)
- LED with 220Ω resistor (pin 5 → resistor → LED → GND)
- 15pF decoupling capacitor across  OUTPUT or VCC and GND of receiver module(not necessary but to remove noise)

## Wiring Diagram
See the diagrams folder for the full pictorial wiring diagram.

## How It Works
The transmitter generates a 38kHz carrier using Timer2 hardware
registers on the ATmega328P and encodes data using the NEC protocol.
Each button press sends address 0x00 and command 0x45.

The receiver decodes the NEC signal and increases LED brightness
by a PWM step of 17 on each valid press from 0 to 255 over 15 steps.
At maximum brightness the LED blinks 4 times as confirmation.
The next press turns the LED off and the cycle repeats.

## Files
- transmitter/transmitter.ino — IR transmitter code
- receiver/receiver.ino — IR receiver with PWM brightness control
- nec_signal_tester/nec_signal_tester.ino — The codes verifies transmitter is sending NEC

## Library Required
IRremote by Shirriff, z3t0, ArminJo — version 3.x or higher
Install via Arduino IDE: Sketch → Include Library → Manage Libraries

## License
MIT License


 
