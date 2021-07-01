/*
RGBled

This sketch demonstrates controlling the Arduboy's RGB LED,
in both analog and digital modes.
*/

/*
Written in 2018 by Scott Allen saydisp-git@yahoo.ca

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <Arduboy2.h>

// The frame rate determines the button auto-repeat rate
#define FRAME_RATE 25

// The increment/decrement amount when auto-repeating
#define REPEAT_AMOUNT 3

// Delay time before button auto-repeat starts, in milliseconds
#define REPEAT_DELAY 700

// Calculation of the number of frames to wait before button auto-repeat starts
#define DELAY_FRAMES (REPEAT_DELAY / (1000 / FRAME_RATE))

#define ANALOG false
#define DIGITAL true

#define ANALOG_MAX 255

// Color array index
enum class Color {
  RED,
  GREEN,
  BLUE,
  COUNT
};

// Map LED color index to LED name
const byte LEDpin[(byte)(Color::COUNT)] = {
  RED_LED,
  GREEN_LED,
  BLUE_LED
};

Arduboy2 arduboy;

// Analog LED values
byte analogValue[3] = { 0, 0, 0};
// Digital LED states
byte digitalState[3] = { RGB_OFF, RGB_OFF, RGB_OFF };

byte analogSelected = (byte)(Color::RED);
byte digitalSelected = (byte)(Color::RED);

boolean controlMode = ANALOG;

// Button repeat handling
unsigned int delayCount = 0;
boolean repeating = false;

// ============================= SETUP ===================================
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(FRAME_RATE);
  analogSet();
}
// =======================================================================


// =========================== MAIN LOOP =================================
void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();

  // Toggle analog/digital control mode
  if (arduboy.justPressed(A_BUTTON)) {
    if ((controlMode = !controlMode) == DIGITAL) {
      arduboy.freeRGBled();
      digitalSet();
    }
    else {
      analogSet();
    }
  }

  // Reset to Analog mode and all LEDs off
  if (arduboy.justPressed(B_BUTTON)) {
    reset();
  }

  // Handle D-pad buttons for current mode
  if (controlMode == ANALOG) {
    modeAnalog();
  }
  else {
    modeDigital();
  }

  // Handle delay before button auto-repeat starts
  if ((delayCount != 0) && (--delayCount == 0)) {
    repeating = true;
  }

  renderScreen(); // Render and display the entire screen
}
// =======================================================================


// Analog control
void modeAnalog() {
  if (arduboy.justPressed(RIGHT_BUTTON)) {
    valueInc(1);
    startButtonDelay();
  }
  else if (arduboy.justPressed(LEFT_BUTTON)) {
    valueDec(1);
    startButtonDelay();
  }
  else if (repeating && arduboy.pressed(RIGHT_BUTTON)) {
    valueInc(REPEAT_AMOUNT);
  }
  else if (repeating && arduboy.pressed(LEFT_BUTTON)) {
    valueDec(REPEAT_AMOUNT);
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    analogSelectInc();
  }
  else if (arduboy.justPressed(UP_BUTTON)) {
    analogSelectDec();
  }
  else if (repeating) {
    stopButtonRepeat();
  }
}

// Digital control
void modeDigital() {
  if (arduboy.justPressed(RIGHT_BUTTON) || arduboy.justPressed(LEFT_BUTTON)) {
    digitalState[digitalSelected] = (digitalState[digitalSelected] == RGB_ON) ?
                                     RGB_OFF : RGB_ON;
    arduboy.digitalWriteRGB(LEDpin[digitalSelected],
                            digitalState[digitalSelected]);
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    digitalSelectInc();
  }
  else if (arduboy.justPressed(UP_BUTTON)) {
    digitalSelectDec();
  }
}

// Reset to analog mode and turn all LEDs off
void reset() {
  digitalState[(byte)(Color::RED)] = RGB_OFF;
  digitalState[(byte)(Color::GREEN)] = RGB_OFF;
  digitalState[(byte)(Color::BLUE)] = RGB_OFF;
  digitalSet();

  analogValue[(byte)(Color::RED)] = 0;
  analogValue[(byte)(Color::GREEN)] = 0;
  analogValue[(byte)(Color::BLUE)] = 0;
  analogSet();

  digitalSelected = (byte)(Color::RED);
  analogSelected = (byte)(Color::RED);

  controlMode = ANALOG;
}

// Increment the selected analog LED value by the specified amount
// and update the LED
void valueInc(byte amount) {
  if ((ANALOG_MAX - analogValue[analogSelected]) <= amount) {
    analogValue[analogSelected] = ANALOG_MAX;
  }
  else {
    analogValue[analogSelected] += amount;
  }

  arduboy.setRGBled(LEDpin[analogSelected], analogValue[analogSelected]);
}

// Decrement the selected analog LED value by the specified amount
// and update the LED
void valueDec(byte amount) {
  if (analogValue[analogSelected] <= amount) {
    analogValue[analogSelected] = 0;
  }
  else {
    analogValue[analogSelected] -= amount;
  }

  arduboy.setRGBled(LEDpin[analogSelected], analogValue[analogSelected]);
}

// Select the next analog color index with wrap
void analogSelectInc() {
  selectInc(analogSelected);
}

// Select the previous analog color index with wrap
void analogSelectDec() {
  selectDec(analogSelected);
}

// Select the next digital color index with wrap
void digitalSelectInc() {
  selectInc(digitalSelected);
}

// Select the previous digital color index with wrap
void digitalSelectDec() {
  selectDec(digitalSelected);
}

// Select the next color index with wrap
void selectInc(byte &index) {
  if (++index == (byte)(Color::COUNT)) {
    index = 0;
  }
}

// Select the previous color index with wrap
void selectDec(byte &index) {
  if (index == 0) {
    index = ((byte)(Color::COUNT) - 1);
  }
  else {
    index--;
  }
}

// Update all LEDs in analog mode
void analogSet() {
  arduboy.setRGBled(analogValue[(byte)(Color::RED)],
                    analogValue[(byte)(Color::GREEN)],
                    analogValue[(byte)(Color::BLUE)]);
}

// Update all LEDs in digital mode
void digitalSet() {
  arduboy.digitalWriteRGB(digitalState[(byte)(Color::RED)],
                          digitalState[(byte)(Color::GREEN)],
                          digitalState[(byte)(Color::BLUE)]);
}

// Start the button auto-repeat delay
void startButtonDelay() {
  delayCount = DELAY_FRAMES;
  repeating = false;
}

// Stop the button auto-repeat or delay
void stopButtonRepeat() {
  delayCount = 0;
  repeating = false;
}

// Render and display the screen
void renderScreen() {
  arduboy.setCursor(12, 0);
  arduboy.print(F("RGB LED"));
  arduboy.setCursor(15, 56);
  arduboy.print(F("A:Mode   B:Reset"));
  arduboy.setCursor(74, 0);

  if (controlMode == ANALOG) {
    arduboy.print(F(" Analog"));
    drawAnalog(9, Color::RED, "Red:");
    drawAnalog(25, Color::GREEN, "Green:");
    drawAnalog(41, Color::BLUE, "Blue:");
  }
  else { // Digital
    arduboy.print(F("Digital"));
    drawDigital(9, Color::RED, "Red:");
    drawDigital(25, Color::GREEN, "Green:");
    drawDigital(41, Color::BLUE, "Blue:");
  }

  arduboy.display(CLEAR_BUFFER);
}

// Draw the information for one analog color
void drawAnalog(int y, Color color, const char* name) {
  byte value = analogValue[(byte)color];

  arduboy.setCursor(0, y);
  arduboy.print(name);
  arduboy.setCursor(42, y);
  printValue(value);
  if (analogSelected == (byte)color) {
    arduboy.print(F(" <--"));
  }
  drawBar(y + 8, color, value);
}

// Draw the value bar for an analog color
void drawBar(int y, Color color, byte value) {
  byte barLength = value / 2;

  if (barLength == 0) {
    return;
  }

  if (analogSelected == (byte)color) {
    arduboy.fillRect(0, y, barLength, 5);
  }
  else {
    arduboy.drawRect(0, y, barLength, 5);
  }
}

// Draw the information for one digital color
void drawDigital(int y, Color color, const char* name) {
  byte state = digitalState[(byte)color];

  arduboy.setCursor(34, y + 3);
  arduboy.print(name);
  arduboy.setCursor(76, y + 3);
  if (state == RGB_ON) {
    arduboy.print(F("ON "));
    arduboy.fillCircle(22, y + 6, 4);
  }
  else {
    arduboy.print(F("OFF"));
    arduboy.drawCircle(22, y + 6, 4);
  }

  if (digitalSelected == (byte)color) {
    arduboy.print(F(" <--"));
    arduboy.drawRect(16, y, 13, 13);
  }
}

// Print a byte in decimal and hex
void printValue(byte val) {
  if (val < 100) {
    arduboy.print(' ');
  }
  if (val < 10) {
    arduboy.print(' ');
  }
  arduboy.print(val);

  arduboy.print(F("  0x"));
  if (val < 0x10) {
    arduboy.print('0');
  }
  arduboy.print(val, HEX);
}

