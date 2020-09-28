/*
FontDemo.ino

This sketch draws all the characters of the library's font as a 16 x 16 block.
The bottom half of the block will be initially off screen. An information
overlay will be drawn on top of the block showing the X and Y cursor position
of the start of the block, the current text wrap state and the current text
raw mode state.

The block can be moved around and off the screen using the direction buttons,
to view all the characters and show how the wrap and raw modes behave.

Controls:
Direction Buttons: Move the block 1 pixel.
Hold A with Direction Buttons: Continuously move the block.
B Button: Toggle text raw mode on and off.
Hold A, press B: Toggle text wrap mode on and off.
*/

/*
Written in September 2020 by Scott Allen saydisp-git@yahoo.ca

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <Arduboy2.h>

Arduboy2 arduboy;

constexpr uint8_t charWidth = arduboy.getCharacterWidth() + arduboy.getCharacterSpacing();
constexpr uint8_t charHeight = arduboy.getCharacterHeight() + arduboy.getLineSpacing();
constexpr uint16_t infoX = charWidth * 8;
constexpr uint16_t infoY = charHeight * 2;
constexpr uint8_t infoWidth = charWidth * 5;
constexpr uint8_t infoBorder = 1;

int16_t xPos = 0;
int16_t yPos = 0;

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(20);

  arduboy.clear();
  arduboy.print(F("DISPLAY AND MOVE FONT\n\n"
                  "    Dpad: move 1\n"
                  "A + Dpad: repeat move\n"
                  "       B: toggle raw\n"
                  "   A + B: toggle wrap\n\n"
                  "  Press A to start"));
  arduboy.display();

  while (!arduboy.pressed(A_BUTTON)) {
    arduboy.idle();
  }
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();

  if (arduboy.justPressed(B_BUTTON)) {
    if (arduboy.pressed(A_BUTTON)) {
      arduboy.setTextWrap(!arduboy.getTextWrap());
    }
    else {
      arduboy.setTextRawMode(!arduboy.getTextRawMode());
    }
  }

  if (arduboy.justPressed(UP_BUTTON) ||
      arduboy.pressed(UP_BUTTON + A_BUTTON)) {
    --yPos;
  }
  if (arduboy.justPressed(DOWN_BUTTON) ||
      arduboy.pressed(DOWN_BUTTON + A_BUTTON)) {
    ++yPos;
  }
  if (arduboy.justPressed(LEFT_BUTTON) ||
      arduboy.pressed(LEFT_BUTTON | A_BUTTON)) {
    --xPos;
  }
  if (arduboy.justPressed(RIGHT_BUTTON) ||
      arduboy.pressed(RIGHT_BUTTON | A_BUTTON)) {
    ++xPos;
  }

  arduboy.clear();

  char code = 0;
  for (uint8_t i = 0; i < 16; ++i) {
    arduboy.setCursor(xPos, yPos + (charHeight * i));
    for (uint8_t j = 0; j < 16; ++j) {
      arduboy.print(code);
      ++code;
    }
  }

  arduboy.fillRect(infoX - infoBorder, infoY - infoBorder,
                   infoWidth + infoBorder, charHeight * 4 + infoBorder);
  arduboy.setTextColor(BLACK);
  arduboy.setTextBackground(WHITE);
  arduboy.setCursor(infoX, infoY);
  arduboy.print(F("X:"));
  arduboy.print(xPos);
  arduboy.setCursor(infoX, infoY + charHeight);
  arduboy.print(F("Y:"));
  arduboy.print(yPos);
  arduboy.setCursor(infoX, infoY + (charHeight * 2));
  arduboy.print(F("W:"));
  arduboy.print(arduboy.getTextWrap() ? F("Yes") : F("No"));
  arduboy.setCursor(infoX, infoY + (charHeight * 3));
  arduboy.print(F("R:"));
  arduboy.print(arduboy.getTextRawMode() ? F("Yes") : F("No"));
  arduboy.setTextColor(WHITE);
  arduboy.setTextBackground(BLACK);
    
  arduboy.display();
}
