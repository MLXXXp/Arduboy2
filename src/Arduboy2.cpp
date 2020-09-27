/**
 * @file Arduboy2.cpp
 * \brief
 * The Arduboy2Base and Arduboy2 classes and support objects and definitions.
 */

#include "Arduboy2.h"

//========================================
//========== class Arduboy2Base ==========
//========================================

uint8_t Arduboy2Base::sBuffer[];

uint16_t Arduboy2Base::frameCount = 0;

uint8_t Arduboy2Base::currentButtonState = 0;
uint8_t Arduboy2Base::previousButtonState = 0;

uint8_t Arduboy2Base::eachFrameMillis = 16;
uint8_t Arduboy2Base::thisFrameStart;
uint8_t Arduboy2Base::lastFrameDurationMs;
bool Arduboy2Base::justRendered = false;

// functions called here should be public so users can create their
// own init functions if they need different behavior than `begin`
// provides by default.
//
// This code and it's documentation should be kept in sync with
// Aruduboy2::begin()
void Arduboy2Base::begin()
{
  beginDoFirst();

  bootLogo();
  // alternative logo functions. Work the same as bootLogo() but may reduce
  // memory size if the sketch uses the same bitmap drawing function or
  // `Sprites`/`SpritesB` class
//  bootLogoCompressed();
//  bootLogoSpritesSelfMasked();
//  bootLogoSpritesOverwrite();
//  bootLogoSpritesBSelfMasked();
//  bootLogoSpritesBOverwrite();

  waitNoButtons(); // wait for all buttons to be released
}

void Arduboy2Base::beginDoFirst()
{
  boot(); // raw hardware

  display(); // blank the display (sBuffer is global, so cleared automatically)

  flashlight(); // light the RGB LED and screen if UP button is being held.

  // check for and handle buttons held during start up for system control
  systemButtons();

  audio.begin();
}

void Arduboy2Base::flashlight()
{
  if (!pressed(UP_BUTTON)) {
    return;
  }

  sendLCDCommand(OLED_ALL_PIXELS_ON); // smaller than allPixelsOn()
  digitalWriteRGB(RGB_ON, RGB_ON, RGB_ON);

#ifndef ARDUBOY_CORE // for Arduboy core timer 0 should remain enabled
  // prevent the bootloader magic number from being overwritten by timer 0
  // when a timer variable overlaps the magic number location, for when
  // flashlight mode is used for upload problem recovery
  power_timer0_disable();
#endif

  while (true) {
    idle();
  }
}

void Arduboy2Base::systemButtons()
{
  while (pressed(B_BUTTON)) {
    digitalWriteRGB(BLUE_LED, RGB_ON); // turn on blue LED
    sysCtrlSound(UP_BUTTON + B_BUTTON, GREEN_LED, 0xff);
    sysCtrlSound(DOWN_BUTTON + B_BUTTON, RED_LED, 0);
    delayShort(200);
  }

  digitalWriteRGB(BLUE_LED, RGB_OFF); // turn off blue LED
}

void Arduboy2Base::sysCtrlSound(uint8_t buttons, uint8_t led, uint8_t eeVal)
{
  if (pressed(buttons)) {
    digitalWriteRGB(BLUE_LED, RGB_OFF); // turn off blue LED
    delayShort(200);
    digitalWriteRGB(led, RGB_ON); // turn on "acknowledge" LED
    EEPROM.update(eepromAudioOnOff, eeVal);
    delayShort(500);
    digitalWriteRGB(led, RGB_OFF); // turn off "acknowledge" LED

    while (pressed(buttons)) { } // Wait for button release
  }
}

void Arduboy2Base::bootLogo()
{
  bootLogoShell(drawLogoBitmap);
}

void Arduboy2Base::drawLogoBitmap(int16_t y)
{
  drawBitmap(20, y, arduboy_logo, 88, 16);
}

void Arduboy2Base::bootLogoCompressed()
{
  bootLogoShell(drawLogoCompressed);
}

void Arduboy2Base::drawLogoCompressed(int16_t y)
{
  drawCompressed(20, y, arduboy_logo_compressed);
}

void Arduboy2Base::bootLogoSpritesSelfMasked()
{
  bootLogoShell(drawLogoSpritesSelfMasked);
}

void Arduboy2Base::drawLogoSpritesSelfMasked(int16_t y)
{
  Sprites::drawSelfMasked(20, y, arduboy_logo_sprite, 0);
}

void Arduboy2Base::bootLogoSpritesOverwrite()
{
  bootLogoShell(drawLogoSpritesOverwrite);
}

void Arduboy2Base::drawLogoSpritesOverwrite(int16_t y)
{
  Sprites::drawOverwrite(20, y, arduboy_logo_sprite, 0);
}

void Arduboy2Base::bootLogoSpritesBSelfMasked()
{
  bootLogoShell(drawLogoSpritesBSelfMasked);
}

void Arduboy2Base::drawLogoSpritesBSelfMasked(int16_t y)
{
  SpritesB::drawSelfMasked(20, y, arduboy_logo_sprite, 0);
}

void Arduboy2Base::bootLogoSpritesBOverwrite()
{
  bootLogoShell(drawLogoSpritesBOverwrite);
}

void Arduboy2Base::drawLogoSpritesBOverwrite(int16_t y)
{
  SpritesB::drawOverwrite(20, y, arduboy_logo_sprite, 0);
}

// bootLogoText() should be kept in sync with bootLogoShell()
// if changes are made to one, equivalent changes should be made to the other
bool Arduboy2Base::bootLogoShell(void (&drawLogo)(int16_t))
{
  bool showLEDs = readShowBootLogoLEDsFlag();

  if (!readShowBootLogoFlag()) {
    return false;
  }

  if (showLEDs) {
    digitalWriteRGB(RED_LED, RGB_ON);
  }

  for (int16_t y = -15; y <= 24; y++) {
    if (pressed(RIGHT_BUTTON)) {
      digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); // all LEDs off
      return false;
    }

    if (showLEDs && y == 4) {
      digitalWriteRGB(RED_LED, RGB_OFF);    // red LED off
      digitalWriteRGB(GREEN_LED, RGB_ON);   // green LED on
    }

    // Using display(CLEAR_BUFFER) instead of clear() may save code space.
    // The extra time it takes to repaint the previous logo isn't an issue.
    display(CLEAR_BUFFER);
    (*drawLogo)(y); // call the function that actually draws the logo
    display();
    delayShort(15);
  }

  if (showLEDs) {
    digitalWriteRGB(GREEN_LED, RGB_OFF);  // green LED off
    digitalWriteRGB(BLUE_LED, RGB_ON);    // blue LED on
  }
  delayShort(400);
  digitalWriteRGB(BLUE_LED, RGB_OFF);

  return true;
}

// wait for all buttons to be released
void Arduboy2Base::waitNoButtons()
{
  do {
    delayShort(50); // simple button debounce
  } while (buttonsState());
}

/* Frame management */

void Arduboy2Base::setFrameRate(uint8_t rate)
{
  eachFrameMillis = 1000 / rate;
}

void Arduboy2Base::setFrameDuration(uint8_t duration)
{
  eachFrameMillis = duration;
}

bool Arduboy2Base::everyXFrames(uint8_t frames)
{
  return frameCount % frames == 0;
}

bool Arduboy2Base::nextFrame()
{
  uint8_t now = (uint8_t) millis();
  uint8_t frameDurationMs = now - thisFrameStart;

  if (justRendered) {
    lastFrameDurationMs = frameDurationMs;
    justRendered = false;
    return false;
  }
  else if (frameDurationMs < eachFrameMillis) {
    // Only idle if at least a full millisecond remains, since idle() may
    // sleep the processor until the next millisecond timer interrupt.
    if (++frameDurationMs < eachFrameMillis) {
      idle();
    }

    return false;
  }

  // pre-render
  justRendered = true;
  thisFrameStart = now;
  frameCount++;

  return true;
}

bool Arduboy2Base::nextFrameDEV()
{
  bool ret = nextFrame();

  if (ret) {
    if (lastFrameDurationMs > eachFrameMillis)
      TXLED1;
    else
      TXLED0;
  }
  return ret;
}

int Arduboy2Base::cpuLoad()
{
  return lastFrameDurationMs*100 / eachFrameMillis;
}

void Arduboy2Base::initRandomSeed()
{
  randomSeed(generateRandomSeed());
}

/* Graphics */

void Arduboy2Base::clear()
{
  fillScreen(BLACK);
}

void Arduboy2Base::drawPixel(int16_t x, int16_t y, uint8_t color)
{
  if (x < 0 || x > (WIDTH-1) || y < 0 || y > (HEIGHT-1))
  {
    return;
  }

  uint16_t row_offset;
  uint8_t bit;

  asm volatile
  (
    // bit = 1 << (y & 7)
    "ldi  %[bit], 1                    \n" //bit = 1;
    "sbrc %[y], 1                      \n" //if (y & _BV(1)) bit = 4;
    "ldi  %[bit], 4                    \n"
    "sbrc %[y], 0                      \n" //if (y & _BV(0)) bit = bit << 1;
    "lsl  %[bit]                       \n"
    "sbrc %[y], 2                      \n" //if (y & _BV(2)) bit = (bit << 4) | (bit >> 4);
    "swap %[bit]                       \n"
    //row_offset = y / 8 * WIDTH + x;
    "andi %A[y], 0xf8                  \n" //row_offset = (y & 0xF8) * WIDTH / 8
    "mul  %[width_offset], %A[y]       \n"
    "movw %[row_offset], r0            \n"
    "clr  __zero_reg__                 \n"
    "add  %A[row_offset], %[x]         \n" //row_offset += x
#if WIDTH != 128
    "adc  %B[row_offset], __zero_reg__ \n" // only non 128 width can overflow
#endif
    : [row_offset]   "=&x" (row_offset),   // upper register (ANDI)
      [bit]          "=&d" (bit),          // upper register (LDI)
      [y]            "+d"  (y)             // upper register (ANDI), must be writable
    : [width_offset] "r"   ((uint8_t)(WIDTH/8)),
      [x]            "r"   ((uint8_t)x)
    :
  );
  uint8_t data = sBuffer[row_offset] | bit;
  if (!(color & _BV(0))) data ^= bit;
  sBuffer[row_offset] = data;
}
#if 0
// For reference, this is the C++ equivalent
void Arduboy2Base::drawPixel(int16_t x, int16_t y, uint8_t color)
{
  if (x < 0 || x > (WIDTH-1) || y < 0 || y > (HEIGHT-1))
  {
    return;
  }

  uint16_t row_offset;
  uint8_t bit;

  bit = 1 << (y & 7);
  row_offset = (y & 0xF8) * WIDTH / 8 + x;
  uint8_t data = sBuffer[row_offset] | bit;
  if (!color) data ^= bit;
  sBuffer[row_offset] = data;
}
#endif

uint8_t Arduboy2Base::getPixel(uint8_t x, uint8_t y)
{
  uint8_t row = y / 8;
  uint8_t bit_position = y % 8;
  return (sBuffer[(row*WIDTH) + x] & _BV(bit_position)) >> bit_position;
}

void Arduboy2Base::drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  while (x<y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void Arduboy2Base::drawCircleHelper
(int16_t x0, int16_t y0, uint8_t r, uint8_t corners, uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    if (corners & 0x4) // lower right
    {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (corners & 0x2) // upper right
    {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (corners & 0x8) // lower left
    {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (corners & 0x1) // upper left
    {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void Arduboy2Base::fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color)
{
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Arduboy2Base::fillCircleHelper
(int16_t x0, int16_t y0, uint8_t r, uint8_t sides, int16_t delta,
 uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    if (sides & 0x1) // right side
    {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }

    if (sides & 0x2) // left side
    {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

void Arduboy2Base::drawLine
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
  // bresenham's algorithm - thx wikpedia
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swapInt16(x0, y0);
    swapInt16(x1, y1);
  }

  if (x0 > x1) {
    swapInt16(x0, x1);
    swapInt16(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      drawPixel(y0, x0, color);
    }
    else
    {
      drawPixel(x0, y0, color);
    }

    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void Arduboy2Base::drawRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void Arduboy2Base::drawFastVLine
(int16_t x, int16_t y, uint8_t h, uint8_t color)
{
  int end = y+h;
  for (int a = max(0,y); a < min(end,HEIGHT); a++)
  {
    drawPixel(x,a,color);
  }
}

void Arduboy2Base::drawFastHLine
(int16_t x, int16_t y, uint8_t w, uint8_t color)
{
  int16_t xEnd; // last x point + 1

  // Do y bounds checks
  if (y < 0 || y >= HEIGHT)
    return;

  xEnd = x + w;

  // Check if the entire line is not on the display
  if (xEnd <= 0 || x >= WIDTH)
    return;

  // Don't start before the left edge
  if (x < 0)
    x = 0;

  // Don't end past the right edge
  if (xEnd > WIDTH)
    xEnd = WIDTH;

  // calculate actual width (even if unchanged)
  w = xEnd - x;

  // buffer pointer plus row offset + x offset
  register uint8_t *pBuf = sBuffer + ((y / 8) * WIDTH) + x;

  // pixel mask
  register uint8_t mask = 1 << (y & 7);

  switch (color)
  {
    case WHITE:
      while (w--)
      {
        *pBuf++ |= mask;
      }
      break;

    case BLACK:
      mask = ~mask;
      while (w--)
      {
        *pBuf++ &= mask;
      }
      break;
  }
}

void Arduboy2Base::fillRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{
  // stupidest version - update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++)
  {
    drawFastVLine(i, y, h, color);
  }
}

void Arduboy2Base::fillScreen(uint8_t color)
{
  // C version:
  //
  // if (color != BLACK)
  // {
  //   color = 0xFF; // all pixels on
  // }
  // for (int16_t i = 0; i < WIDTH * HEIGTH / 8; i++)
  // {
  //    sBuffer[i] = color;
  // }

  // This asm version is hard coded for 1024 bytes. It doesn't use the defined
  // WIDTH and HEIGHT values. It will have to be modified for a different
  // screen buffer size.
  // It also assumes color value for BLACK is 0.

  // local variable for screen buffer pointer,
  // which can be declared a read-write operand
  uint8_t* bPtr = sBuffer;

  asm volatile
  (
    // if value is zero, skip assigning to 0xff
    "cpse %[color], __zero_reg__\n"
    "ldi %[color], 0xFF\n"
    // counter = 0
    "clr __tmp_reg__\n"
    "1:\n"
    // (4x) push zero into screen buffer,
    // then increment buffer position
    "st Z+, %[color]\n"
    "st Z+, %[color]\n"
    "st Z+, %[color]\n"
    "st Z+, %[color]\n"
    // increase counter
    "inc __tmp_reg__\n"
    // repeat for 256 loops
    // (until counter rolls over back to 0)
    "brne 1b\n"
    : [color] "+d" (color),
      "+z" (bPtr)
    :
    :
  );
}

void Arduboy2Base::drawRoundRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color)
{
  // smarter version
  drawFastHLine(x+r, y, w-2*r, color); // Top
  drawFastHLine(x+r, y+h-1, w-2*r, color); // Bottom
  drawFastVLine(x, y+r, h-2*r, color); // Left
  drawFastVLine(x+w-1, y+r, h-2*r, color); // Right
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}

void Arduboy2Base::fillRoundRect
(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color)
{
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

void Arduboy2Base::drawTriangle
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void Arduboy2Base::fillTriangle
(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{

  int16_t a, b, y, last;
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1)
  {
    swapInt16(y0, y1); swapInt16(x0, x1);
  }
  if (y1 > y2)
  {
    swapInt16(y2, y1); swapInt16(x2, x1);
  }
  if (y0 > y1)
  {
    swapInt16(y0, y1); swapInt16(x0, x1);
  }

  if(y0 == y2)
  { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)
    {
      a = x1;
    }
    else if(x1 > b)
    {
      b = x1;
    }
    if(x2 < a)
    {
      a = x2;
    }
    else if(x2 > b)
    {
      b = x2;
    }
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t dx01 = x1 - x0,
      dy01 = y1 - y0,
      dx02 = x2 - x0,
      dy02 = y2 - y0,
      dx12 = x2 - x1,
      dy12 = y2 - y1,
      sa = 0,
      sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
  {
    last = y1;   // Include y1 scanline
  }
  else
  {
    last = y1-1; // Skip it
  }


  for(y = y0; y <= last; y++)
  {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if(a > b)
    {
      swapInt16(a,b);
    }

    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);

  for(; y <= y2; y++)
  {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if(a > b)
    {
      swapInt16(a,b);
    }

    drawFastHLine(a, y, b-a+1, color);
  }
}

void Arduboy2Base::drawBitmap
(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h,
 uint8_t color)
{
  // no need to draw at all if we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  int yOffset = abs(y) % 8;
  int sRow = y / 8;
  if (y < 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }
  int rows = h/8;
  if (h%8!=0) rows++;
  for (int a = 0; a < rows; a++) {
    int bRow = sRow + a;
    if (bRow > (HEIGHT/8)-1) break;
    if (bRow > -2) {
      for (int iCol = 0; iCol<w; iCol++) {
        if (iCol + x > (WIDTH-1)) break;
        if (iCol + x >= 0) {
          if (bRow >= 0) {
            if (color == WHITE)
              sBuffer[(bRow*WIDTH) + x + iCol] |= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
            else if (color == BLACK)
              sBuffer[(bRow*WIDTH) + x + iCol] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) << yOffset);
            else
              sBuffer[(bRow*WIDTH) + x + iCol] ^= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
          }
          if (yOffset && bRow<(HEIGHT/8)-1 && bRow > -2) {
            if (color == WHITE)
              sBuffer[((bRow+1)*WIDTH) + x + iCol] |= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
            else if (color == BLACK)
              sBuffer[((bRow+1)*WIDTH) + x + iCol] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset));
            else
              sBuffer[((bRow+1)*WIDTH) + x + iCol] ^= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
          }
        }
      }
    }
  }
}


void Arduboy2Base::drawSlowXYBitmap
(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color)
{
  // no need to draw at all of we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  int16_t xi, yi, byteWidth = (w + 7) / 8;
  for(yi = 0; yi < h; yi++) {
    for(xi = 0; xi < w; xi++ ) {
      if(pgm_read_byte(bitmap + yi * byteWidth + xi / 8) & (128 >> (xi & 7))) {
        drawPixel(x + xi, y + yi, color);
      }
    }
  }
}

// Helper for drawCompressed()
class Arduboy2Base::BitStreamReader
{
 private:
  const uint8_t *source;
  uint16_t sourceIndex;
  uint8_t bitBuffer;
  uint8_t byteBuffer;

 public:
  BitStreamReader(const uint8_t *bitmap)
    : source(bitmap), sourceIndex(), bitBuffer(), byteBuffer()
  {
  }

  uint16_t readBits(uint16_t bitCount)
  {
    uint16_t result = 0;
    for (uint16_t i = 0; i < bitCount; i++)
    {
      if (bitBuffer == 0)
      {
        bitBuffer = 0x1;
        byteBuffer = pgm_read_byte(&source[sourceIndex]);
        ++sourceIndex;
      }

      if ((byteBuffer & bitBuffer) != 0)
        result |= (1 << i);

      bitBuffer <<= 1;
    }
    return result;
  }
};

void Arduboy2Base::drawCompressed(int16_t sx, int16_t sy, const uint8_t *bitmap, uint8_t color)
{
  // set up decompress state
  BitStreamReader cs(bitmap);

  // read header
  int width = (int)cs.readBits(8) + 1;
  int height = (int)cs.readBits(8) + 1;
  uint8_t spanColour = (uint8_t)cs.readBits(1); // starting colour

  // no need to draw at all if we're offscreen
  if ((sx + width <= 0) || (sx > WIDTH - 1) || (sy + height <= 0) || (sy > HEIGHT - 1))
    return;

  // sy = sy - (frame * height);
  int yOffset = abs(sy) % 8;
  int startRow = sy / 8;
  if (sy < 0) {
    startRow--;
    yOffset = 8 - yOffset;
  }
  int rows = height / 8;
  if ((height % 8) != 0)
    ++rows;

  int rowOffset = 0; // +(frame*rows);
  int columnOffset = 0;

  uint8_t byte = 0x00;
  uint8_t bit = 0x01;
  while (rowOffset < rows) // + (frame*rows))
  {
    uint16_t bitLength = 1;
    while (cs.readBits(1) == 0)
      bitLength += 2;

    uint16_t len = cs.readBits(bitLength) + 1; // span length

    // draw the span
    for (uint16_t i = 0; i < len; ++i)
    {
      if (spanColour != 0)
        byte |= bit;
      bit <<= 1;

      if (bit == 0) // reached end of byte
      {
        // draw
        int bRow = startRow + rowOffset;

        //if (byte) // possible optimisation
        if ((bRow <= (HEIGHT / 8) - 1) && (bRow > -2) &&
            (columnOffset + sx <= (WIDTH - 1)) && (columnOffset + sx >= 0))
        {
          int16_t offset = (bRow * WIDTH) + sx + columnOffset;
          if (bRow >= 0)
          {
            int16_t index = offset;
            uint8_t value = byte << yOffset;

            if (color != 0)
              sBuffer[index] |= value;
            else
              sBuffer[index] &= ~value;
          }
          if ((yOffset != 0) && (bRow < (HEIGHT / 8) - 1))
          {
            int16_t index = offset + WIDTH;
            uint8_t value = byte >> (8 - yOffset);

            if (color != 0)
              sBuffer[index] |= value;
            else
              sBuffer[index] &= ~value;
          }
        }

        // iterate
        ++columnOffset;
        if (columnOffset >= width)
        {
          columnOffset = 0;
          ++rowOffset;
        }

        // reset byte
        byte = 0x00;
        bit = 0x01;
      }
    }

    spanColour ^= 0x01; // toggle colour bit (bit 0) for next span
  }
}

void Arduboy2Base::display()
{
  paintScreen(sBuffer);
}

void Arduboy2Base::display(bool clear)
{
  paintScreen(sBuffer, clear);
}

uint8_t* Arduboy2Base::getBuffer()
{
  return sBuffer;
}

bool Arduboy2Base::pressed(uint8_t buttons)
{
  return (buttonsState() & buttons) == buttons;
}

bool Arduboy2Base::anyPressed(uint8_t buttons)
{
  return (buttonsState() & buttons) != 0;
}

bool Arduboy2Base::notPressed(uint8_t buttons)
{
  return (buttonsState() & buttons) == 0;
}

void Arduboy2Base::pollButtons()
{
  previousButtonState = currentButtonState;
  currentButtonState = buttonsState();
}

bool Arduboy2Base::justPressed(uint8_t button)
{
  return (!(previousButtonState & button) && (currentButtonState & button));
}

bool Arduboy2Base::justReleased(uint8_t button)
{
  return ((previousButtonState & button) && !(currentButtonState & button));
}

bool Arduboy2Base::collide(Point point, Rect rect)
{
  return ((point.x >= rect.x) && (point.x < rect.x + rect.width) &&
      (point.y >= rect.y) && (point.y < rect.y + rect.height));
}

bool Arduboy2Base::collide(Rect rect1, Rect rect2)
{
  return !(rect2.x                >= rect1.x + rect1.width  ||
           rect2.x + rect2.width  <= rect1.x                ||
           rect2.y                >= rect1.y + rect1.height ||
           rect2.y + rect2.height <= rect1.y);
}

uint16_t Arduboy2Base::readUnitID()
{
  return EEPROM.read(eepromUnitID) |
         (((uint16_t)(EEPROM.read(eepromUnitID + 1))) << 8);
}

void Arduboy2Base::writeUnitID(uint16_t id)
{
  EEPROM.update(eepromUnitID, (uint8_t)(id & 0xff));
  EEPROM.update(eepromUnitID + 1, (uint8_t)(id >> 8));
}

uint8_t Arduboy2Base::readUnitName(char* name)
{
  char val;
  uint8_t dest;
  uint8_t src = eepromUnitName;

  for (dest = 0; dest < ARDUBOY_UNIT_NAME_LEN; dest++)
  {
    val = EEPROM.read(src);
    name[dest] = val;
    src++;
    if (val == 0x00 || (byte)val == 0xFF) {
      break;
    }
  }

  name[dest] = 0x00;
  return dest;
}

void Arduboy2Base::writeUnitName(const char* name)
{
  bool done = false;
  uint8_t dest = eepromUnitName;

  for (uint8_t src = 0; src < ARDUBOY_UNIT_NAME_LEN; src++)
  {
    if (name[src] == 0x00) {
      done = true;
    }
    // write character or 0 pad if finished
    EEPROM.update(dest, done ? 0x00 : name[src]);
    dest++;
  }
}

bool Arduboy2Base::readShowBootLogoFlag()
{
  return (EEPROM.read(eepromSysFlags) & sysFlagShowLogoMask);
}

void Arduboy2Base::writeShowBootLogoFlag(bool val)
{
  uint8_t flags = EEPROM.read(eepromSysFlags);

  bitWrite(flags, sysFlagShowLogoBit, val);
  EEPROM.update(eepromSysFlags, flags);
}

bool Arduboy2Base::readShowUnitNameFlag()
{
  return (EEPROM.read(eepromSysFlags) & sysFlagUnameMask);
}

void Arduboy2Base::writeShowUnitNameFlag(bool val)
{
  uint8_t flags = EEPROM.read(eepromSysFlags);

  bitWrite(flags, sysFlagUnameBit, val);
  EEPROM.update(eepromSysFlags, flags);
}

bool Arduboy2Base::readShowBootLogoLEDsFlag()
{
  return (EEPROM.read(eepromSysFlags) & sysFlagShowLogoLEDsMask);
}

void Arduboy2Base::writeShowBootLogoLEDsFlag(bool val)
{
  uint8_t flags = EEPROM.read(eepromSysFlags);

  bitWrite(flags, sysFlagShowLogoLEDsBit, val);
  EEPROM.update(eepromSysFlags, flags);
}

void Arduboy2Base::swapInt16(int16_t& a, int16_t& b)
{
  int16_t temp = a;
  a = b;
  b = temp;
}


//====================================
//========== class Arduboy2 ==========
//====================================

int16_t Arduboy2::cursor_x = 0;
int16_t Arduboy2::cursor_y = 0;
uint8_t Arduboy2::textColor = WHITE;
uint8_t Arduboy2::textBackground = BLACK;
uint8_t Arduboy2::textSize = 1;
bool Arduboy2::textWrap = false;
bool Arduboy2::textRaw = false;

// functions called here should be public so users can create their
// own init functions if they need different behavior than `begin`
// provides by default.
//
// This code and it's documentation should be kept in sync with
// Aruduboy2Base::begin()
void Arduboy2::begin()
{
  beginDoFirst();

  bootLogo();
  // alternative logo functions. Work the same as bootLogo() but may reduce
  // memory size if the sketch uses the same bitmap drawing function or
  // `Sprites`/`SpritesB` class
//  bootLogoCompressed();
//  bootLogoSpritesSelfMasked();
//  bootLogoSpritesOverwrite();
//  bootLogoSpritesBSelfMasked();
//  bootLogoSpritesBOverwrite();

  waitNoButtons();
}

void Arduboy2::bootLogo()
{
  if (bootLogoShell(drawLogoBitmap))
  {
    bootLogoExtra();
  }
}

void Arduboy2::bootLogoCompressed()
{
  if (bootLogoShell(drawLogoCompressed))
  {
    bootLogoExtra();
  }
}

void Arduboy2::bootLogoSpritesSelfMasked()
{
  if (bootLogoShell(drawLogoSpritesSelfMasked))
  {
    bootLogoExtra();
  }
}

void Arduboy2::bootLogoSpritesOverwrite()
{
  if (bootLogoShell(drawLogoSpritesOverwrite))
  {
    bootLogoExtra();
  }
}

void Arduboy2::bootLogoSpritesBSelfMasked()
{
  if (bootLogoShell(drawLogoSpritesBSelfMasked))
  {
    bootLogoExtra();
  }
}

void Arduboy2::bootLogoSpritesBOverwrite()
{
  if (bootLogoShell(drawLogoSpritesBOverwrite))
  {
    bootLogoExtra();
  }
}

// bootLogoText() should be kept in sync with bootLogoShell()
// if changes are made to one, equivalent changes should be made to the other
void Arduboy2::bootLogoText()
{
  bool showLEDs = readShowBootLogoLEDsFlag();

  if (!readShowBootLogoFlag()) {
    return;
  }

  if (showLEDs) {
    digitalWriteRGB(RED_LED, RGB_ON);
  }

  for (int16_t y = -15; y <= 24; y++) {
    if (pressed(RIGHT_BUTTON)) {
      digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); // all LEDs off
      return;
    }

    if (showLEDs && y == 4) {
      digitalWriteRGB(RED_LED, RGB_OFF);    // red LED off
      digitalWriteRGB(GREEN_LED, RGB_ON);   // green LED on
    }

    // Using display(CLEAR_BUFFER) instead of clear() may save code space.
    // The extra time it takes to repaint the previous logo isn't an issue.
    display(CLEAR_BUFFER);
    cursor_x = 23;
    cursor_y = y;
    textSize = 2;
    print(F("ARDUBOY"));
    textSize = 1;
    display();
    delayShort(11);
  }

  if (showLEDs) {
    digitalWriteRGB(GREEN_LED, RGB_OFF);  // green LED off
    digitalWriteRGB(BLUE_LED, RGB_ON);    // blue LED on
  }
  delayShort(400);
  digitalWriteRGB(BLUE_LED, RGB_OFF);

  bootLogoExtra();
}

void Arduboy2::bootLogoExtra()
{
  uint8_t c;

  if (!readShowUnitNameFlag())
  {
    return;
  }

  c = EEPROM.read(eepromUnitName);

  if (c != 0xFF && c != 0x00)
  {
    uint8_t i = eepromUnitName;
    cursor_x = 50;
    cursor_y = 56;

    do
    {
      write(c);
      c = EEPROM.read(++i);
    }
    while (i < eepromUnitName + ARDUBOY_UNIT_NAME_LEN);

    display();
    delayShort(1000);
  }
}

size_t Arduboy2::write(uint8_t c)
{
  if ((c == '\r') && !textRaw)
  {
    return 1;
  }

  if (((c == '\n') && !textRaw) ||
      (textWrap && (cursor_x > (WIDTH - (characterWidth * textSize)))))
  {
    cursor_x = 0;
    cursor_y += fullCharacterHeight * textSize;
  }

  if ((c != '\n') || textRaw)
  {
    drawChar(cursor_x, cursor_y, c, textColor, textBackground, textSize);
    cursor_x += fullCharacterWidth * textSize;
  }

  return 1;
}

void Arduboy2::drawChar
  (int16_t x, int16_t y, uint8_t c, uint8_t color, uint8_t bg, uint8_t size)
{
// It is assumed that rendering characters fully off screen will be rare,
// so let drawPixel() handle off screen checks, to reduce code size at the
// expense of slower off screen character handling.
#if 0
  if ((x >= WIDTH) ||              // Clip right
      (y >= HEIGHT) ||             // Clip bottom
      ((x + characterWidth * size - 1) < 0) ||  // Clip left
      ((y + characterHeight * size - 1) < 0)    // Clip top
     )
  {
    return;
  }
#endif

  bool drawBackground = bg != color;
  const uint8_t* bitmap =
    &font5x7[c * characterWidth * ((characterHeight + 8 - 1) / 8)];

  for (uint8_t i = 0; i < fullCharacterWidth; i++)
  {
    uint8_t column;

    if (characterHeight <= 8)
    {
      column = (i < characterWidth) ? pgm_read_byte(bitmap++) : 0;
    }
    else
    {
      column = 0;
    }

    // draw the character by columns. Top to bottom, left to right
    // including character spacing on the right
    for (uint8_t j = 0; j < characterHeight; j++)
    {
      if (characterHeight > 8)
      {
        // at this point variable "column" will be 0, either from initialization
        // or by having eight 0 bits shifted in by the >>= operation below
        if ((j % 8 == 0) && (i < characterWidth))
        {
          column = pgm_read_byte(bitmap++);
        }
      }

      // pixelIsSet should be a bool but at the time of writing,
      // the GCC AVR compiler generates less code if it's a uint8_t
      uint8_t pixelIsSet = column & 0x01;

      if (pixelIsSet || drawBackground)
      {
        for (uint8_t a = 0; a < size; a++)
        {
          for (uint8_t b = 0; b < size; b++)
          {
            drawPixel(x + (i * size) + a, y + (j * size) + b,
                      pixelIsSet ? color : bg);
          }
        }
      }
      column >>= 1;
    }

    // draw the inter-line spacing pixels for this column if required
    if (drawBackground)
    {
      for (uint8_t j = characterHeight; j < fullCharacterHeight; j++)
      {
        for (uint8_t a = 0; a < size; a++)
        {
          for (uint8_t b = 0; b < size; b++)
          {
            drawPixel(x + (i * size) + a, y + (j * size) + b, bg);
          }
        }
      }
    }
  }
}

void Arduboy2::setCursor(int16_t x, int16_t y)
{
  cursor_x = x;
  cursor_y = y;
}

void Arduboy2::setCursorX(int16_t x)
{
  cursor_x = x;
}

void Arduboy2::setCursorY(int16_t y)
{
  cursor_y = y;
}

int16_t Arduboy2::getCursorX()
{
  return cursor_x;
}

int16_t Arduboy2::getCursorY()
{
  return cursor_y;
}

void Arduboy2::setTextColor(uint8_t color)
{
  textColor = color;
}

uint8_t Arduboy2::getTextColor()
{
  return textColor;
}

void Arduboy2::setTextBackground(uint8_t bg)
{
  textBackground = bg;
}

uint8_t Arduboy2::getTextBackground()
{
  return textBackground;
}

void Arduboy2::setTextSize(uint8_t s)
{
  // size must always be 1 or higher
  textSize = max(1, s);
}

uint8_t Arduboy2::getTextSize()
{
  return textSize;
}

void Arduboy2::setTextWrap(bool w)
{
  textWrap = w;
}

bool Arduboy2::getTextWrap()
{
  return textWrap;
}

void Arduboy2::setTextRawMode(bool raw)
{
  textRaw = raw;
}

bool Arduboy2::getTextRawMode()
{
  return textRaw;
}

void Arduboy2::clear()
{
    Arduboy2Base::clear();
    cursor_x = cursor_y = 0;
}

