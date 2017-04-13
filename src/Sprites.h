/**
 * @file Sprites.h
 * \brief
 * A class for drawing animated sprites from image and mask bitmaps.
 */

#ifndef Sprites_h
#define Sprites_h

#include "Arduboy2.h"

#define SPRITE_MASKED 1
#define SPRITE_UNMASKED 2
#define SPRITE_OVERWRITE 2
#define SPRITE_PLUS_MASK 3
#define SPRITE_IS_MASK 250
#define SPRITE_IS_MASK_ERASE 251
#define SPRITE_AUTO_MODE 255

/** \brief
 * A class for drawing animated sprites from image and mask bitmaps.
 *
 * \details
 * The functions in this class will draw to the screen buffer an image
 * contained in an array located in program memory. A mask can also be
 * specified or implied, which dictates how existing pixels in the buffer,
 * within the image boundaries, will be affected.
 *
 * A sprite or mask array contains one or more "frames". Each frame is intended
 * to show whatever the sprite represents in a different position, such as the
 * various poses for a running or jumping character. By specifying a different
 * frame each time the sprite is drawn, it can be animated.
 *
 * Each image array begins with values for the width and height of the sprite,
 * in pixels. The width can be any value. The height must be a multiple of
 * 8 pixels, but with proper masking, a sprite of any height can be created.
 *
 * For a separate mask array, as is used with `drawExternalMask()`, the width
 * and height are not included but must contain data of the same dimensions
 * as the corresponding image array.
 *
 * Following the width and height values for an image array, or the from the
 * beginning of a separate mask array, the array contains the image and/or
 * mask data for each frame. Each byte represents a vertical column of 8 pixels
 * with the least significant bit (bit 0) at the top. The bytes are drawn as
 * 8 pixel high rows from left to right, top to bottom. When the end of a row
 * is reached, as specified by the width value, the next byte in the array will
 * be the start of the next row.
 *
 * Data for each frame after the first one immediately follows the previous
 * frame. Frame numbers start at 0.
 */
class Sprites
{
  public:
    /** \brief
     * Draw a sprite using a separate image and mask array.
     *
     * \param x,y The coordinates of the top left pixel location.
     * \param bitmap A pointer to the array containing the image frames.
     * \param mask A pointer to the array containing the mask frames.
     * \param frame The frame number of the image to draw.
     * \param mask_frame The frame number for the mask to use (can be different
     * from the image frame number).
     *
     * \details
     * An array containing the image frames, and another array containing
     * corresponding mask frames, are used to draw a sprite.
     *
     * Bits set to 1 in the mask indicate that the pixel will be set to the
     * value of the corresponding image bit. Bits set to 0 in the mask will be
     * left unchanged.
     *
     *     image  mask   before  after
     *
     *     .....  .OOO.  .....   .....
     *     ..O..  OOOOO  .....   ..O..
     *     OO.OO  OO.OO  .....   OO.OO
     *     ..O..  OOOOO  .....   ..O..
     *     .....  .OOO.  .....   .....
     *
     *     image  mask   before  after
     *
     *     .....  .OOO.  OOOOO   O...O
     *     ..O..  OOOOO  OOOOO   ..O..
     *     OO.OO  OOOOO  OOOOO   OO.OO
     *     ..O..  OOOOO  OOOOO   ..O..
     *     .....  .OOO.  OOOOO   O...O
     */
    static void drawExternalMask(int16_t x, int16_t y, const uint8_t *bitmap,
                                 const uint8_t *mask, uint8_t frame, uint8_t mask_frame);

    /** \brief
     * Draw a sprite using an array containing both image and mask values.
     *
     * \param x,y The coordinates of the top left pixel location.
     * \param bitmap A pointer to the array containing the image/mask frames.
     * \param frame The frame number of the image to draw.
     *
     * \details
     * An array containing combined image and mask data is used to draw a
     * sprite. Bytes are given in pairs with the first byte representing the
     * image pixels and the second byte specifying the corresponding mask.
     * The width given in the array still specifies the image width, so each
     * row of image and mask bytes will be twice the width value.
     *
     * Bits set to 1 in the mask indicate that the pixel will be set to the
     * value of the corresponding image bit. Bits set to 0 in the mask will be
     * left unchanged.
     *
     *     image  mask   before  after
     *
     *     .....  .OOO.  .....   .....
     *     ..O..  OOOOO  .....   ..O..
     *     OO.OO  OO.OO  .....   OO.OO
     *     ..O..  OOOOO  .....   ..O..
     *     .....  .OOO.  .....   .....
     *
     *     image  mask   before  after
     *
     *     .....  .OOO.  OOOOO   O...O
     *     ..O..  OOOOO  OOOOO   ..O..
     *     OO.OO  OOOOO  OOOOO   OO.OO
     *     ..O..  OOOOO  OOOOO   ..O..
     *     .....  .OOO.  OOOOO   O...O
     */
    static void drawPlusMask(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame);

    /** \brief
     * Draw a sprite by replacing the existing content completely.
     *
     * \param x,y The coordinates of the top left pixel location.
     * \param bitmap A pointer to the array containing the image frames.
     * \param frame The frame number of the image to draw.
     *
     * \details
     * A sprite is drawn by overwriting the pixels in the buffer with the data
     * from the specified frame in the array. No masking is done. A bit set
     * to 1 in the frame will set the pixel to 1 in the buffer, and a 0 in the
     * array will set a 0 in the buffer.
     *
     *     image  before  after
     *
     *     .....  .....   .....
     *     ..O..  .....   ..O..
     *     OO.OO  .....   OO.OO
     *     ..O..  .....   ..O..
     *     .....  .....   .....
     *
     *     image  before  after
     *
     *     .....  OOOOO   .....
     *     ..O..  OOOOO   ..O..
     *     OO.OO  OOOOO   OO.OO
     *     ..O..  OOOOO   ..O..
     *     .....  OOOOO   .....
     */
    static void drawOverwrite(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame);

    /** \brief
     * "Erase" a sprite.
     *
     * \param x,y The coordinates of the top left pixel location.
     * \param bitmap A pointer to the array containing the image frames.
     * \param frame The frame number of the image to erase.
     *
     * \details
     * The data from the specified frame in the array is used to erase a
     * sprite. To "erase" a sprite, bits set to 1 in the frame will set the
     * corresponding pixel in the buffer to 0. Frame bits set to 0 will remain
     * unchanged in the buffer.
     *
     *     image  before  after
     *
     *     .....  .....   .....
     *     ..O..  .....   .....
     *     OO.OO  .....   .....
     *     ..O..  .....   .....
     *     .....  .....   .....
     *
     *     image  before  after
     *
     *     .....  OOOOO   OOOOO
     *     ..O..  OOOOO   OO.OO
     *     OO.OO  OOOOO   ..O..
     *     ..O..  OOOOO   OO.OO
     *     .....  OOOOO   OOOOO
     */
    static void drawErase(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame);

    /** \brief
     * Draw a sprite using only the bits set to 1.
     *
     * \param x,y The coordinates of the top left pixel location.
     * \param bitmap A pointer to the array containing the image frames.
     * \param frame The frame number of the image to draw.
     *
     * \details
     * Bits set to 1 in the frame will be used to draw the sprite by setting
     * the corresponding pixel in the buffer to 1. Bits set to 0 in the frame
     * will remain unchanged in the buffer.
     *
     *     image  before  after
     *
     *     .....  .....   .....
     *     ..O..  .....   ..O..
     *     OO.OO  .....   OO.OO
     *     ..O..  .....   ..O..
     *     .....  .....   .....
     *
     *     image  before  after
     *
     *     .....  OOOOO   OOOOO  (no change because all pixels were
     *     ..O..  OOOOO   OOOOO  already white)
     *     OO.OO  OOOOO   OOOOO
     *     ..O..  OOOOO   OOOOO
     *     .....  OOOOO   OOOOO
     */
    static void drawSelfMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame);

    // Master function. Needs to be abstracted into separate function for
    // every render type.
    // (Not officially part of the API)
    static void draw(int16_t x, int16_t y,
                     const uint8_t *bitmap, uint8_t frame,
                     const uint8_t *mask, uint8_t sprite_frame,
                     uint8_t drawMode);

    // (Not officially part of the API)
    static void drawBitmap(int16_t x, int16_t y,
                           const uint8_t *bitmap, const uint8_t *mask,
                           uint8_t w, uint8_t h, uint8_t draw_mode);
};

#endif
