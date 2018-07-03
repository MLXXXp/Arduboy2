# watchX-Arduboy2

This is a modification of the [Arduboy2](https://github.com/MLXXXp/Arduboy2) library, made to work with the pin-mappings and PWM/Timer allocations of the [watchX](http://watchx.io/). Packaged together with the *ArduBreakout.ino* example sketch for demonstration purposes.

## Things to note:

- we only have 3 buttons, these are mapped to **UP_BUTTON / DOWN_BUTTON / B_BUTTON**
- we only have 2 LEDs, these are mapped to **RED_LED / GREEN_LED**
- we only have 1 speaker pin, this is mapped to **BeepPin1** used by **Arduboy2Beep**

For the time being, I have commented out the extra start-up functions (flashlight / systemButtons / bootLogo...) because in their current state they rely on having more than just our 3 buttons and 2 LEDs, and also because the state of the EEPROM may not necessarily follow Arduboy2 rules on a watchX!
