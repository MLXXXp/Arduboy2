/**
 * @file Arduboy2.h
 * \brief
 * The Arduboy2Base and Arduboy2 classes and support objects and definitions.
 */

#ifndef ARDUBOY2_H
#define ARDUBOY2_H

#include <Arduino.h>
#include "Arduboy2Core.h"
#include "Sprites.h"
#include <Print.h>
#include <limits.h>

/** \brief
 * Library version
 *
 * \details
 * For a version number in the form of x.y.z the value of the define will be
 * ((x * 10000) + (y * 100) + (z)) as a decimal number.
 * So, it will read as xxxyyzz, with no leading zeros on x.
 *
 * A user program can test this value to conditionally compile based on the
 * library version. For example:
 *
 * \code
 * // If the library is version 2.1.0 or higher
 * #if ARDUBOY_LIB_VER >= 20100
 *   // ... code that make use of a new feature added to V2.1.0
 * #endif
 * \endcode
 */
#define ARDUBOY_LIB_VER 30000

// EEPROM settings
#define EEPROM_VERSION 0
#define EEPROM_BRIGHTNESS 1
#define EEPROM_AUDIO_ON_OFF 2

/** \brief
 * Start of EEPROM storage space for sketches.
 *
 * \details
 * An area at the start of EEPROM is reserved for system use.
 * This define specifies the first EEPROM location past the system area.
 * Sketches can use locations from here to the end of EEPROM space.
 */
#define EEPROM_STORAGE_SPACE_START 16

// eeprom settings above are neded for audio
#include "Arduboy2Audio.h"

// If defined, it is safe to draw outside of the screen boundaries.
// Pixels that would exceed the display limits will be ignored.
#define PIXEL_SAFE_MODE

// pixel colors
#define BLACK 0  /**< Color value for an unlit pixel for draw functions. */
#define WHITE 1  /**< Color value for a lit pixel for draw functions. */
/** \brief
 * Color value to indicate pixels are to be inverted.
 *
 * \details
 * BLACK pixels will become WHITE and WHITE will become BLACK.
 *
 * \note
 * Only function Arduboy2Base::drawBitmap() currently supports this value.
 */
#define INVERT 2

#define CLEAR_BUFFER true /**< Value to be passed to `display()` to clear the screen buffer. */

// compare Vcc to 1.1 bandgap
#define ADC_VOLTAGE (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1))
// compare temperature to 2.5 internal reference and _BV(MUX5)
#define ADC_TEMP (_BV(REFS0) | _BV(REFS1) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0))


/** \brief
 * A rectangle object for collision functions.
 *
 * \details
 * The X and Y coordinates specify the top left corner of a rectangle with the
 * given width and height.
 *
 * \see Arduboy2Base::collide(Point, Rect) Arduboy2Base::collide(Rect, Rect)
 */
struct Rect
{
  int16_t x;      /**< The X coordinate of the top left corner */
  int16_t y;      /**< The Y coordinate of the top left corner */
  uint8_t width;  /**< The width of the rectangle */
  uint8_t height; /**< The height of the rectangle */
};

/** \brief
 * An object to define a single point for collision functions.
 *
 * \details
 * The location of the point is given by X and Y coordinates.
 *
 * \see Arduboy2Base::collide(Point, Rect)
 */
struct Point
{
  int16_t x; /**< The X coordinate of the point */
  int16_t y; /**< The Y coordinate of the point */
};

//==================================
//========== Arduboy2Base ==========
//==================================

/** \brief
 * The main functions provided for writing sketches for the Arduboy,
 * _minus_ text output.
 *
 * This class in inherited by Arduboy2, so if text output functions are
 * required Arduboy2 should be used instead.
 *
 * \note
 * \parblock
 * An Arduboy2Audio class object named `audio` will be created by the
 * Arduboy2Base class, so there is no need for a sketch itself to create an
 * Arduboy2Audio object. Arduboy2Audio functions can be called using the
 * Arduboy2 or Arduboy2Base `audio` object.
 *
 * Example:
 *
 * \code
 * #include <Arduboy2.h>
 *
 * Arduboy2 arduboy;
 *
 * // Arduboy2Audio functions can be called as follows:
 *   arduboy.audio.on();
 *   arduboy.audio.off();
 * \endcode
 * \endparblock
 *
 * \note
 * \parblock
 * A friend class named _Arduboy2Ex_ is declared by this class. The intention
 * is to allow a sketch to create an _Arduboy2Ex_ class which would have access
 * to the private and protected members of the Arduboy2Base class. It is hoped
 * that this may eliminate the need to create an entire local copy of the
 * library, in order to extend the functionality, in most circumstances.
 * \endparblock
 *
 * \see Arduboy2
 */
class Arduboy2Base : public Arduboy2Core
{
 friend class Arduboy2Ex;
 friend class Sprites;

 public:
  Arduboy2Base();

  /** \brief
   * An object created to provide audio control functions within this class.
   *
   * \details
   * This object is created to eliminate the need for a sketch to create an
   * Arduboy2Audio class object itself.
   *
   * \see Arduboy2Audio
   */
  Arduboy2Audio audio;

  /** \brief
   * Initialize the hardware, display the boot logo, provide boot utilities, etc.
   *
   * \details
   * This function should be called once near the start of the sketch,
   * usually in `setup()`, before using any other functions in this class.
   * It initializes the display, displays the boot logo, provides "flashlight"
   * and system control features and initializes audio control.
   *
   * \note
   * To free up some code space for use by the sketch, `boot()` can be used
   * instead of `begin()` allow the elimination of some of the things that
   * aren't really required, such as displaying the boot logo.
   *
   * \see boot()
   */
  void begin();

  /** \brief
   * Flashlight mode turns the RGB LED and display fully on.
   *
   * \details
   * Checks if the UP button is pressed and if so turns the RGB LED and all
   * display pixels fully on. Pressing the DOWN button will exit flashlight mode.
   *
   * This function is called by `begin()` and can be called by a sketch
   * after `boot()`.
   *
   * \see begin() boot()
   */
  void flashlight();

  /** \brief
   * Handle buttons held on startup for system control.
   *
   * \details
   * This function is called by `begin()` and can be called by a sketch
   * after `boot()`.
   *
   * Hold the B button when booting to enter system control mode.
   * The B button must be held continuously to remain in this mode.
   * Then, pressing other buttons will perform system control functions:
   *
   * - UP: Set "sound enabled" in EEPROM
   * - DOWN: Set "sound disabled" (mute) in EEPROM
   *
   * \see begin() boot()
   */
  void systemButtons();

  /** \brief
   * Display the boot logo sequence.
   *
   * \details
   * This function is called by `begin()` and can be called by a sketch
   * after `boot()`.
   *
   * The Arduboy logo scrolls down from the top of the screen to the center
   * while the RGB LEDs light in sequence.
   *
   * \see begin() boot()
   */
  void bootLogo();

  /** \brief
   * Clear the display buffer.
   *
   * \details
   * The entire contents of the screen buffer are cleared to BLACK.
   *
   * \see display(bool)
   */
  void clear();

  /** \brief
   * Copy the contents of the display buffer to the display.
   *
   * \details
   * The contents of the display buffer in RAM are copied to the display and
   * will appear on the screen.
   *
   * \see display(bool)
   */
  void display();

  /** \brief
   * Copy the contents of the display buffer to the display. The display buffer
   * can optionally be cleared.
   *
   * \param clear If `true` the display buffer will be cleared to zero.
   * The defined value `CLEAR_BUFFER` should be used instead of `true` to make
   * it more meaningful.
   *
   * \details
   * Operation is the same as calling `display()` without parameters except
   * additionally the display buffer will be cleared if the parameter evaluates
   * to `true`. (The defined value `CLEAR_BUFFER` can be used for this)
   *
   * Using `display(CLEAR_BUFFER)` is faster and produces less code than
   * calling `display()` followed by `clear()`.
   *
   * \see display() clear()
   */
  void display(bool clear);

  /** \brief
   * Set a single pixel in the display buffer to the specified color.
   *
   * \param x The X coordinate of the pixel.
   * \param y The Y coordinate of the pixel.
   * \param color The color of the pixel (optional; defaults to WHITE).
   *
   * \details
   * The single pixel specified location in the display buffer is set to the
   * specified color. The values WHITE or BLACK can be used for the color.
   * If the `color` parameter isn't included, the pixel will be set to WHITE.
   */
  void drawPixel(int16_t x, int16_t y, uint8_t color = WHITE);

  /** \brief
   * Returns the state of the given pixel in the screen buffer.
   *
   * \param x The X coordinate of the pixel.
   * \param y The Y coordinate of the pixel.
   *
   * \return WHITE if the pixel is on or BLACK if the pixel is off.
   */
  uint8_t getPixel(uint8_t x, uint8_t y);

  /** \brief
   * Draw a circle of a given radius.
   *
   * \param x0 The X coordinate of the circle's center.
   * \param y0 The Y coordinate of the circle's center.
   * \param r The radius of the circle in pixels.
   * \param color The circle's color (optional; defaults to WHITE).
   */
  void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color = WHITE);

  // Draw one or more "corners" of a circle.
  // (Not officially part of the API)
  void drawCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t corners, uint8_t color = WHITE);

  /** \brief
   * Draw a filled-in circle of a given radius.
   *
   * \param x0 The X coordinate of the circle's center.
   * \param y0 The Y coordinate of the circle's center.
   * \param r The radius of the circle in pixels.
   * \param color The circle's color (optional; defaults to WHITE).
   */
  void fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color = WHITE);

  // Draw one or both vertical halves of a filled-in circle or
  // rounded rectangle edge.
  // (Not officially part of the API)
  void fillCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t sides, int16_t delta, uint8_t color = WHITE);

  /** \brief
   * Draw a line between two specified points.
   *
   * \param x0,x1 The X coordinates of the line ends.
   * \param y0,y1 The Y coordinates of the line ends.
   * \param color The line's color (optional; defaults to WHITE).
   *
   * \details
   * Draw a line from the start point to the end point using
   * Bresenham's algorithm.
   * The start and end points can be at any location with respect to the other.
   */
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color = WHITE);

  /** \brief
   * Draw a rectangle of a specified width and height.
   *
   * \param x The X coordinate of the upper left corner.
   * \param y The Y coordinate of the upper left corner.
   * \param w The width of the rectangle.
   * \param h The height of the rectangle.
   * \param color The color of the pixel (optional; defaults to WHITE).
   */
  void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /** \brief
   * Draw a vertical line.
   *
   * \param x The X coordinate of the upper start point.
   * \param y The Y coordinate of the upper start point.
   * \param h The height of the line.
   * \param color The color of the line (optional; defaults to WHITE).
   */
  void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color = WHITE);

  /** \brief
   * Draw a horizontal line.
   *
   * \param x The X coordinate of the left start point.
   * \param y The Y coordinate of the left start point.
   * \param w The width of the line.
   * \param color The color of the line (optional; defaults to WHITE).
   */
  void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color = WHITE);

  /** \brief
   * Draw a filled-in rectangle of a specified width and height.
   *
   * \param x The X coordinate of the upper left corner.
   * \param y The Y coordinate of the upper left corner.
   * \param w The width of the rectangle.
   * \param h The height of the rectangle.
   * \param color The color of the pixel (optional; defaults to WHITE).
   */
  void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /** \brief
   * Fill the screen buffer with the specified color.
   *
   * \param color The fill color (optional; defaults to WHITE).
   */
  void fillScreen(uint8_t color = WHITE);

  /** \brief
   * Draw a rectangle with rounded corners.
   *
   * \param x The X coordinate of the left edge.
   * \param y The Y coordinate of the top edge.
   * \param w The width of the rectangle.
   * \param h The height of the rectangle.
   * \param r The radius of the semicircles forming the corners.
   * \param color The color of the rectangle (optional; defaults to WHITE).
   */
  void drawRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color = WHITE);

  /** \brief
   * Draw a filled-in rectangle with rounded corners.
   *
   * \param x The X coordinate of the left edge.
   * \param y The Y coordinate of the top edge.
   * \param w The width of the rectangle.
   * \param h The height of the rectangle.
   * \param r The radius of the semicircles forming the corners.
   * \param color The color of the rectangle (optional; defaults to WHITE).
   */
  void fillRoundRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t r, uint8_t color = WHITE);

  /** \brief
   * Draw a triangle given the coordinates of each corner.
   *
   * \param x0,x1,x2 The X coordinates of the corners.
   * \param y0,y1,y2 The Y coordinates of the corners.
   * \param color The triangle's color (optional; defaults to WHITE).
   *
   * \details
   * A triangle is drawn by specifying each of the three corner locations.
   * The corners can be at any position with respect to the others.
   */
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color = WHITE);

  /** \brief
   * Draw a filled-in triangle given the coordinates of each corner.
   *
   * \param x0,x1,x2 The X coordinates of the corners.
   * \param y0,y1,y2 The Y coordinates of the corners.
   * \param color The triangle's color (optional; defaults to WHITE).
   *
   * \details
   * A triangle is drawn by specifying each of the three corner locations.
   * The corners can be at any position with respect to the others.
   */
  void fillTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color = WHITE);

  /** \brief
   * Draw a bitmap from an array in program memory.
   *
   * \param x The X coordinate of the top left pixel affected by the bitmap.
   * \param y The Y coordinate of the top left pixel affected by the bitmap.
   * \param bitmap A pointer to the bitmap array in program memory.
   * \param w The width of the bitmap in pixels.
   * \param h The height of the bitmap in pixels.
   * \param color The color of pixels for bits set to 1 in the bitmap.
   *              If the value is INVERT, bits set to 1 will invert the
   *              corresponding pixel. (optional; defaults to WHITE).
   *
   * \details
   * Bits set to 1 in the provided bitmap array will have their corresponding
   * pixel set to the specified color. For bits set to 0 in the array, the
   * corresponding pixel will be left unchanged.
   *
   * Each byte in the array specifies a vertical column of 8 pixels, with the
   * least significant bit at the top.
   *
   * The array must be located in program memory by using the PROGMEM modifier.
   */
  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /** \brief
   * Draw a bitmap from a horizontally oriented array in program memory.
   *
   * \param x The X coordinate of the top left pixel affected by the bitmap.
   * \param y The Y coordinate of the top left pixel affected by the bitmap.
   * \param bitmap A pointer to the bitmap array in program memory.
   * \param w The width of the bitmap in pixels.
   * \param h The height of the bitmap in pixels.
   * \param color The color of pixels for bits set to 1 in the bitmap.
   *              (optional; defaults to WHITE).
   *
   * \details
   * Bits set to 1 in the provided bitmap array will have their corresponding
   * pixel set to the specified color. For bits set to 0 in the array, the
   * corresponding pixel will be left unchanged.
   *
   * Each byte in the array specifies a horizontal row of 8 pixels, with the
   * most significant bit at the left end of the row.
   *
   * The array must be located in program memory by using the PROGMEM modifier.
   *
   * \note
   * This function requires a lot of additional CPU power and will draw images
   * slower than `drawBitmap()`, which uses bitmaps that are stored in a format
   * that allows them to be directly written to the screen. It is recommended
   * you use `drawBitmap()` when possible.
   */
  void drawSlowXYBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color = WHITE);

  /** \brief
   * Draw a bitmap from an array of compressed data.
   *
   * \param sx The X coordinate of the top left pixel affected by the bitmap.
   * \param sy The Y coordinate of the top left pixel affected by the bitmap.
   * \param bitmap A pointer to the compressed bitmap array in program memory.
   * \param color The color of pixels for bits set to 1 in the bitmap.
   *
   * \details
   * Draw a bitmap starting at the given coordinates from an array that has
   * been compressed using an algorthm implemented by Team A.R.G.
   * For more information see:
   * https://github.com/TEAMarg/drawCompressed
   * https://github.com/TEAMarg/Cabi
   *
   * Bits set to 1 in the provided bitmap array will have their corresponding
   * pixel set to the specified color. For bits set to 0 in the array, the
   * corresponding pixel will be left unchanged.
   *
   * The array must be located in program memory by using the PROGMEM modifier.
   */
  void drawCompressed(int16_t sx, int16_t sy, const uint8_t *bitmap, uint8_t color = WHITE);

  /** \brief
   * Get a pointer to the display buffer in RAM.
   *
   * \return A pointer to the display buffer array in RAM
   *
   * \details
   * The location of the display buffer in RAM, which is displayed using
   * `display()`, can be gotten using this function. The buffer can then be
   *  read and directly manipulated.
   */
  uint8_t* getBuffer();

  /** \brief
   * Seed the random number generator with a random value.
   *
   * \details
   * The Arduino random number generator is seeded with a random value
   * derrived from entropy from the temperature, voltage reading, and
   * microseconds since boot.
   *
   * This method is still most effective when called after a semi-random time,
   * such as after a user hits a button to start a game or other semi-random
   * event.
   */
  void initRandomSeed();

  // Swap the values of two int16_t variables passed by reference.
  void swap(int16_t& a, int16_t& b);

  /** \brief
   * Set the frame rate used by the frame control functions.
   *
   * \param rate The desired frame rate in frames per second.
   *
   * Set the frame rate, in frames per second, used by `nextFrame()` to update
   * frames at a given rate. If this function isn't used, the default rate will
   * be 60.
   *
   * Normally, the frame rate would be set to the desired value once, at the
   * start of the game, but it can be changed at any time to alter the frame
   * update rate.
   *
   * \see nextFrame()
   */
  void setFrameRate(uint8_t rate);

  /** \brief
   * Indicate that it's time to render the next frame.
   *
   * \return `true` if it's time for the next frame.
   *
   * \details
   * When this function returns `true`, the amount of time has elapsed to
   * display the next frame, as specified by `setFrameRate()`.
   *
   * This function will normally be called at the start of the rendering loop
   * which would wait for `true` to be returned before rendering and
   * displaying the next frame.
   *
   * example:
   * \code
   * void loop() {
   *   if (!arduboy.nextFrame()) {
   *     return; // go back to the start of the loop
   *   }
   *   // render and display the next frame
   * }
   * \endcode
   *
   * \see setFrameRate()
   */
  bool nextFrame();

  /** \brief
   * Indicate if the specified number of frames has elapsed.
   *
   * \param frames The desired number of elapsed frames.
   *
   * \return `true` if the specified number of frames has elapsed.
   *
   * This function should be called with the same value each time for a given
   * event. It will return `true` if the given number of frames has elapsed
   * since the previous frame in which it returned `true`.
   *
   * For example, if you wanted to fire a shot every 5 frames while the A button
   * is being held down:
   *
   * \code
   * if (arduboy.everyXframes(5)) {
   *   if arduboy.pressed(A_BUTTON) {
   *     fireShot();
   *   }
   * }
   * \endcode
   *
   * \see setFrameRate() nextFrame()
   */
  bool everyXFrames(uint8_t frames);

  /** \brief
   * Return the load on the CPU as a percentage.
   *
   * \return The load on the CPU as a percentage of the total frame time.
   *
   * \details
   * The returned value gives the time spent processing a frame as a percentage
   * the total time allotted for a frame, as determined by the frame rate.
   *
   * This function normally wouldn't be used in the final program. It is
   * intended for use during program development as an aid in helping with
   * frame timing.
   *
   * \note
   * The percentage returned can be higher than 100 if more time is spent
   * processing a frame than the time allotted per frame. This would indicate
   * that the frame rate should be made slower or the frame processing code
   * should be optimized to run faster.
   *
   * \see setFrameRate() nextFrame()
   */
  int cpuLoad();

  // Useful for getting raw approximate voltage values.
  uint16_t rawADC(uint8_t adc_bits);

  /** \brief
   * Test if the specified buttons are pressed.
   *
   * \param buttons A bit mask indicating which buttons to test.
   * (Can be a single button)
   *
   * \return `true` if *all* buttons in the provided mask are currently pressed.
   *
   * \details
   * Read the state of the buttons and return `true` if all the buttons in the
   * specified mask are being pressed.
   *
   * Example: `if (pressed(LEFT_BUTTON + A_BUTTON))`
   *
   * \note
   * This function does not perform any button debouncing.
   */
  bool pressed(uint8_t buttons);

  /** \brief
   * Test if the specified buttons are not pressed.
   *
   * \param buttons A bit mask indicating which buttons to test.
   * (Can be a single button)
   *
   * \return `true` if *all* buttons in the provided mask are currently
   * released.
   *
   * \details
   * Read the state of the buttons and return `true` if all the buttons in the
   * specified mask are currently released.
   *
   * Example: `if (notPressed(UP_BUTTON))`
   *
   * \note
   * This function does not perform any button debouncing.
   */
  bool notPressed(uint8_t buttons);

  /** \brief
   * Poll the buttons and track their state over time.
   *
   * \details
   * Read and save the current state of the buttons and also keep track of the
   * button state when this function was previouly called. These states are
   * used by the `justPressed()` and `justReleased()` functions to determine
   * if a button has changed state between now and the previous call to
   * `pollButtons()`.
   *
   * This function should be called once at the start of each new frame.
   *
   * The `justPressed()` and `justReleased()` functions rely on this function.
   *
   * example:
   * \code
   * void loop() {
   *   if (!arduboy.nextFrame()) {
   *     return;
   *   }
   *   arduboy.pollButtons();
   *
   *   // use justPressed() as necessary to determine if a button was just pressed
   * \endcode
   *
   * \note
   * As long as the elapsed time between calls to this function is long
   * enough, buttons will be naturally debounced. Calling it once per frame at
   * a frame rate of 60 or lower (or possibly somewhat higher), should be
   * sufficient.
   *
   * \see justPressed() justReleased()
   */
  void pollButtons();

  /** \brief
   * Check if a button has just been pressed.
   *
   * \param button The button to test for. Only one button should be specified.
   *
   * \return `true` if the specified button has just been pressed.
   *
   * \details
   * Return `true` if the given button was pressed between the latest
   * call to `pollButtons()` and previous call to `pollButtons()`.
   * If the button has been held down over multiple polls, this function will
   * return `false`.
   *
   * There is no need to check for the release of the button since it must have
   * been released for this function to return `true` when pressed again.
   *
   * This function should only be used to test a single button.
   *
   * \see pollButtons() justReleased()
   */
  bool justPressed(uint8_t button);

  /** \brief
   * Check if a button has just been released.
   *
   * \param button The button to test for. Only one button should be specified.
   *
   * \return `true` if the specified button has just been released.
   *
   * \details
   * Return `true` if the given button, having previously been pressed,
   * was released between the latest call to `pollButtons()` and previous call
   * to `pollButtons()`. If the button has remained released over multiple
   * polls, this function will return `false`.
   *
   * There is no need to check for the button having been pressed since it must
   * have been previously pressed for this function to return `true` upon
   * release.
   *
   * This function should only be used to test a single button.
   *
   * \note
   * There aren't many cases where this function would be needed. Wanting to
   * know if a button has been released, without knowing when it was pressed,
   * is uncommon.
   *
   * \see pollButtons() justPressed()
   */
  bool justReleased(uint8_t button);

  /** \brief
   * Test if a point falls within a rectangle
   *
   * \param point A structure describing the location of the point.
   * \param rect A structure describing the location and size of the rectangle.
   *
   * \return `true` if the specified point is within the specified rectangle.
   *
   * This function is intended to detemine if an object, whose boundaries are
   * are defined by the given rectangle, is in contact with the given point.
   *
   * \see Point Rect
   */
  bool collide(Point point, Rect rect);

  /** \brief
   * Test if a rectangle is intersecting with another rectangle.
   *
   * \param rect1,rect2 Structures describing the size and locations of the
   * rectangles.
   *
   * \return `true1 if the first rectangle is intersecting the second.
   *
   * This function is intended to detemine if an object, whose boundaries are
   * are defined by the given rectangle, is in contact with another rectangular
   * object.
   *
   * \see Rect
   */
  bool collide(Rect rect1, Rect rect2);

 protected:
  // helper function for sound enable/disable system control
  void sysCtrlSound(uint8_t buttons, uint8_t led, uint8_t eeVal);

  // Screen buffer
  static uint8_t sBuffer[(HEIGHT*WIDTH)/8];

  // For button handling
  uint8_t currentButtonState;
  uint8_t previousButtonState;

  // For frame funcions
  uint16_t frameCount;
  uint8_t eachFrameMillis;
  unsigned long lastFrameStart;
  unsigned long nextFrameStart;
  bool post_render;
  uint8_t lastFrameDurationMs;
};


//==============================
//========== Arduboy2 ==========
//==============================

/** \brief
 * The main functions provided for writing sketches for the Arduboy,
 * _including_ text output.
 *
 * \details
 * This class is derived from Arduboy2Base. It provides text output functions
 * in addition to all the functions inherited from Arduboy2Base.
 *
 * \note
 * A friend class named _Arduboy2Ex_ is declared by this class. The intention
 * is to allow a sketch to create an _Arduboy2Ex_ class which would have access
 * to the private and protected members of the Arduboy2 class. It is hoped
 * that this may eliminate the need to create an entire local copy of the
 * library, in order to extend the functionality, in most circumstances.
 *
 * \see Arduboy2Base
 */
class Arduboy2 : public Print, public Arduboy2Base
{
 friend class Arduboy2Ex;

 public:
  Arduboy2();

  /** \class Print
   * \brief
   * The Arduino `Print` class is available for writing text to the screen
   * buffer.
   *
   * \details
   * For an `Arduboy2` class object, functions provided by the Arduino `Print`
   * class can be used to write text to the screen buffer, in the same manner
   * as the Arduino `Serial.print()`, etc., functions.
   *
   * Print will use the `write()` function to actually draw each character
   * in the screen buffer.
   *
   * See:
   * https://www.arduino.cc/en/Serial/Print
   *
   * Example:
   * \code
   * int value = 42;
   *
   * arduboy.println("Hello World"); // Prints "Hello World" and then moves the
   *                                 // text cursor to the start of the next line
   * arduboy.print(value);  // Prints "42"
   * arduboy.print('\n');   // Moves the text cursor to the start of the next line
   * arduboy.print(78, HEX) // Prints "4E" (78 in hexadecimal)
   * \endcode
   *
   * \see Arduboy2::write()
   */

  /** \brief
   * Write a single ASCII character at the current text cursor location.
   *
   * \param c The ASCII value of the character to be written.
   *
   * \return The number of characters written (will always be 1).
   *
   * \details
   * This is the Arduboy implemetation of the Arduino virtual `write()`
   * function. The single ASCII character specified is written to the
   * the screen buffer at the current text cursor. The text cursor is then
   * moved to the next character position in the screen buffer. This new cursor
   * position will depend on the current text size and possibly the current
   * wrap mode.
   *
   * Two special characters are handled:
   *
   * - The newline character `\n`. This will move the text cursor to the start
   *   of the next line based on the current text size.
   * - The carriage return character `\r`. This character will be ignored.
   *
   * \note
   * This function is rather low level and, although it's available as a public
   * function, it wouldn't normally be used. In most cases the Arduino Print
   * class should be used for writing text.
   *
   * \see Print setTextSize() setTextWrap()
   */
  virtual size_t write(uint8_t);

  /** \brief
   * Draw a single ASCII character at the specified location in the screen
   * buffer.
   *
   * \param x The X coordinate, in pixels, for where to draw the character.
   * \param y The Y coordinate, in pixels, for where to draw the character.
   * \param c The ASCII value of the character to be drawn.
   * \param color the forground color of the character.
   * \param bg the background color of the character.
   * \param size The size of the character to draw.
   *
   * \details
   * The specified ASCII character is drawn starting at the provided
   * coordinate. The point specified by the X and Y coordinates will be the
   * top left corner of the character.
   *
   * \note
   * This is a low level function used by the `write()` function to draw a
   * character. Although it's available as a public function, it wouldn't
   * normally be used. In most cases the Arduino Print class should be used for
   * writing text.
   *
   * \see Print write() setTextColor() setTextBackground() setTextSize()
   */
  void drawChar(int16_t x, int16_t y, unsigned char c, uint8_t color, uint8_t bg, uint8_t size);

  /** \brief
   * Set the location of the text cursor.
   *
   * \param x The X coordinate, in pixels, for the new location of the text cursor.
   * \param y The Y coordinate, in pixels, for the new location of the text cursor.
   *
   * \details
   * The location of the text cursor is set the the specified coordinates.
   * The coordinates are in pixels. Since the coordinates can specify any pixel
   * location, the text does not have to be placed on specific rows.
   * As with all drawing functions, location 0, 0 is the top left corner of
   * the display. The cursor location will be the top left corner of the next
   * character written.
   */
  void setCursor(int16_t x, int16_t y);

  /** \brief
   * Get the X coordinate of the current text cursor position.
   *
   * \return The X coordinate of the current text cursor position.
   *
   * \details
   * The X coordinate returned is a pixel location with 0 indicating the
   * leftmost column.
   */
  int16_t getCursorX();

  /** \brief
   * Get the Y coordinate of the current text cursor position.
   *
   * \return The Y coordinate of the current text cursor position.
   *
   * \details
   * The Y coordinate returned is a pixel location with 0 indicating the
   * topmost row.
   */
  int16_t getCursorY();

  /** \brief
   * Set the text foreground color.
   *
   * \param color The color to be used for following text.
   */
  void setTextColor(uint8_t color);

  /** \brief
   * Set the text background color.
   *
   * \param bg The background color to be used for following text.
   */
  void setTextBackground(uint8_t bg);

  /** \brief
   * Set the text character size.
   *
   * \param s The text size multiplier. Must be 1 or higher.
   *
   * \details
   * Setting a text size of 1 will result in standard size characters which
   * occupy 6x8 pixels (the result of 5x7 characters with spacing on the
   * right and bottom edges).
   *
   * The value specified is a multiplier. A value of 2 will double the
   * size so they will occupy 12x16 pixels. A value of 3 will result in
   * 18x24, etc.
   */
  void setTextSize(uint8_t s);

  /** \brief
   * Set or disable text wrap mode.
   *
   * \param w `true` enables text wrap mode. `false` disables it.
   *
   * \details
   * Text wrap mode is enabled by specifying `true`. In wrap mode, the text
   * cursor will be moved to the start of the next line (based on the current
   * text size) if the following character wouldn't fit entirely at the end of
   * the current line.

   * If wrap mode is disabled, characters will continue to be written to the
   * same line. A character at the right edge of the screen may only be
   * partially displayed and additional characters will be off screen.
   */
  void setTextWrap(bool w);

  /** \brief
   * Clear the display buffer and set the text cursor to location 0, 0
   */
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

