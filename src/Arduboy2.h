#ifndef Arduboy2_h
#define Arduboy2_h

#include "core/core.h"
#include <Print.h>
#include <limits.h>

// Library version.
// For a version number in the form of x.y.z the value of the define will be
// ((x * 10000) + (y * 100) + (z)) as a decimal number.
// So, it will read as xxxyyzz, with no leading zeros on x.
#define ARDUBOY_LIB_VER 20000

// EEPROM settings
#define EEPROM_VERSION 0
#define EEPROM_BRIGHTNESS 1
#define EEPROM_AUDIO_ON_OFF 2

// we reserve the first 16 byte of EEPROM for system use
#define EEPROM_STORAGE_SPACE_START 16 // and onward

// eeprom settings above are neded for audio
#include "audio/audio.h"

#define PIXEL_SAFE_MODE

// pixel colors
#define INVERT 2 //< lit/unlit pixel
#define WHITE 1 //< lit pixel
#define BLACK 0 //< unlit pixel

// compare Vcc to 1.1 bandgap
#define ADC_VOLTAGE (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
// compare temperature to 2.5 internal reference and _BV(MUX5)
#define ADC_TEMP (_BV(REFS0) | _BV(REFS1) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0))


//==================================
//========== Arduboy2Base ==========
//==================================

class Arduboy2Base : public ArduboyCore
{
public:
  Arduboy2Base();

  ArduboyAudio audio;

  /// Returns true if the button mask passed in is pressed.
  /**
   * if (pressed(LEFT_BUTTON + A_BUTTON))
   */
  bool pressed(uint8_t buttons);

  /// Returns true if the button mask passed in not pressed.
  /**
   * if (notPressed(LEFT_BUTTON))
   */
  bool notPressed(uint8_t buttons);

  /// Initialize hardware, boot logo, boot utilities, etc.
  /**
   * To free up some code space for use by the sketch, you can use "boot()"
   * instead of "begin()" to eliminate some of the some of the things that
   * aren't really required, such as displaying the boot logo.
   * 
   * Look at the source for "begin()" and after calling "boot()" call
   * whatever functions "begin()" does that you still want to keep.
   * If your sketch uses the speaker, it's probably a good idea to at least
   * call "audio.begin()".
   */
  void begin();

  /// Flashlight mode
  /**
   * Turn the RGB LED and screen fully on. Called in the begin() function.
   *
   * Hold the UP button when booting to enable. Press the DOWN button to exit
   * or simply turn off your Arduboy. It goes into a tight loop until the
   * DOWN button is pressed.
   */
  void flashlight();

  /// Handle buttons held on startup for system control
  /**
   * Hold the B button when booting to enter system control mode.
   * The B button must be held continuously to remain in this mode.
   * Pressing other buttons will perform system control functions:
   *
   * UP: Set "sound enabled" in EEPROM
   * DOWN: Set "sound disabled" (mute) in EEPROM
   */
  void systemButtons();

  /// Scrolls in the Arduboy logo
  void bootLogo();

  /// Clears display.
  void clear();

  /// Copies the contents of the screen buffer to the screen.
  /**
   * X and Y positions on the display are from the top left corner, thus a Y of 64
   * is the bottom of the screen and an X of 128 is the right side of the screen.
   * "Color" or "value" means choosing whether a pixel is lit or not - if color is
   * 0, the pixel is off (black), if color is 1, the pixel is on (white).
   */
  void display();

  /// Sets a single pixel on the screen buffer to white or black.
  void drawPixel(int x, int y, uint8_t color = WHITE);

  /// Returns the state of the given pixel in the screen buffer.
  /**
   * Return value will be 1 if the pixel is on (white) or 0 if the pixel is off (black).
   */
  uint8_t getPixel(uint8_t x, uint8_t y);

  /// Draw a circle of a defined radius.
  /**
   * Draws a circle in white or black. X and Y are the center point of the circle.
   */
  void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color = WHITE);

  /// Draws one or more "corners" of a circle.
  void drawCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, uint8_t color = WHITE);

  /// Draws a filled-in circle.
  void fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color = WHITE);

   /// Draws one or both vertical halves of a filled-in circle.
  void fillCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t cornername, int16_t delta, uint8_t color = WHITE);

  /// Draws a line between two points.
  /**
   * Uses Bresenham's algorithm.
   */
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color = WHITE);

  /// Draws a rectangle of a width and height.
  void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /// Draws vertical line.
  void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color = WHITE);

  /// Draws a horizontal line.
  void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color = WHITE);

  /// Draws a filled-in rectangle.
  void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /// Fills the screen buffer with white or black.
  void fillScreen(uint8_t color = WHITE);

  /// Draws a rectangle with rounded edges.
  void drawRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color = WHITE);

  /// Draws a filled-in rectangle with rounded edges.
  void fillRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color = WHITE);

   /// Draws the outline of a triangle.
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color = WHITE);

  /// Draws a filled-in triangle.
  void fillTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color = WHITE);

  /// Draws a bitmap from program memory to a specific X/Y
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /// Draws images that are bit-oriented horizontally.
  /**
   * This requires a lot of additional CPU power and will draw images slower
   * than drawBitmap, where the images are stored in a format that
   * allows them to be directly written to the screen. It is
   * recommended you use drawBitmap when possible.
   */
  void drawSlowXYBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /// Get a pointer to the display buffer.
  unsigned char* getBuffer();

  /// Seeds the random number generator with entropy from the temperature, voltage reading, and microseconds since boot.
  /**
   * This method is still most effective when called semi-randomly such
   * as after a user hits a button to start a game or other semi-random
   * events
   */
  void initRandomSeed();

  /// Swap the references of two pointers.
  void swap(int16_t& a, int16_t& b);

  /// Sets the frame rate in frames per second.
  void setFrameRate(uint8_t rate);

  /// Returns "true" if it's time for the next frame.
  bool nextFrame();

  /// Returns "true" if the current frame number is evenly divisible by the specified number.
  /**
   * If called with the same value each time, will return "true" if the given
   * number of frames has elapsed since the last frame in which it was "true".
   */
  bool everyXFrames(uint8_t frames);

  /// Returns the load on the CPU as a percentage.
  /**
   * This is based on how much of the time your app is spending rendering
   * frames.  This number can be higher than 100 if your app is rendering
   * really slowly.
   */
  int cpuLoad();

  uint8_t frameRate;
  uint16_t frameCount;
  uint8_t eachFrameMillis;
  long lastFrameStart;
  long nextFrameStart;
  bool post_render;
  uint8_t lastFrameDurationMs;

  /// useful for getting raw approximate voltage values
  uint16_t rawADC(uint8_t adc_bits);

protected:
  // helper function for sound enable/disable system control
  void sysCtrlSound(uint8_t buttons, uint8_t led, uint8_t eeVal);

  // Screen buffer
  uint8_t sBuffer[(HEIGHT*WIDTH)/8];
};


//==============================
//========== Arduboy2 ==========
//==============================

class Arduboy2 : public Print, public Arduboy2Base
{
public:
  Arduboy2();

  /// Writes a single ASCII character to the screen.
  virtual size_t write(uint8_t);

  /// Draws an ASCII character at a point.
  void drawChar(int16_t x, int16_t y, unsigned char c, uint8_t color, uint8_t bg, uint8_t size);

  /// Sets the location of the text cursor.
  void setCursor(int16_t x, int16_t y);

  /// Get the text cursor X position
  uint16_t getCursorX();

  /// Get the text cursor Y position
  uint16_t getCursorY();

  /// Sets the text foreground color
  void setTextColor(uint8_t color);

  /// Sets the text background color
  void setTextBackground(uint8_t bg);

  /// Set the text size
  /**
   * Individual ASCII characters are 6x8 pixels
   * (5x7 with spacing on two edges). The size is a pixel multiplier,
   * so a size of 2 means each character will be 12x16, etc.
   */
  void setTextSize(uint8_t s);

  /// Sets whether text will wrap at screen edges.
  void setTextWrap(bool w);

  /// Clears the display and sets the cursor to 0, 0
  void clear();

protected:
  int16_t cursor_x;
  int16_t cursor_y;
  uint8_t textColor;
  uint8_t textBackground;
  uint8_t textSize;
  bool textWrap;
};

#endif

