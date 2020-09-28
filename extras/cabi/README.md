# Cabi - Compress Arduboy Image

A command line program to read a PNG (Portable Network Graphics) file
containing a bitmap image, compress it using RLE encoding and convert it to
C/C++ code suitable for use with the Team A.R.G. *drawCompressed()* function.
This function is included in the Arduboy2 library.

Written by zep

https://www.lexaloffle.com/bbs/?uid=1

https://twitter.com/lexaloffle

Contributed to Team A.R.G.

This program uses code from the LodePNG project by Lode Vandevenne to read
and decode PNG files.

https://github.com/lvandeve/lodepng

This version of Cabi is maintained as part of the Arduboy2 library so that it
remains available since the demise of Team A.R.G.

## Building the program

Pre-built executable code is not provided due to the difficulty of maintaining
versions for all the many operating systems that it could be run on.

The code is written in C and should compile properly using any ANSI C99
compatible compiler, such as (but not limited to) gcc or clang.

### Build examples

To build from a copy of the cabi directory tree provided, while in the base
directory containing cabi.c use:

`gcc cabi.c lodepng/lodepng.c -o cabi`

or

`clang cabi.c lodepng/lodepng.c -o cabi`


For Windows, it may be more desirable to name the program `CABI.EXE` by using:

`-o CABI.EXE`

Compiler options for optimization, etc. (such as -O2 or -Os) can be added if
desired but likely won't make much difference for most uses.

## Usage

The binary executable file (cabi or CABI.EXE) should be placed somewhere in the
path for executables on the operating system used, or else include the path as
part of the command given.

Running Cabi without any parameters will just output a brief program
description and the usage syntax:

```text
cabi - Compress Arduboy Image
Convert a PNG file into RLE encoded C/C++ source
for use with Arduboy2 drawCompressed()

usage: cabi in.png [array_name_prefix]
```

For `in.png` substitute the name of the PNG file to be converted. If the file
isn't in the current directory, the full path and name can be specified.

For `[array_name_prefix]` an optional prefix for the names of the arrays created
can be given. If this parameter isn't provided, `compressed_image` will be used
for the prefix.

If the program is unable to produce proper output, an error message will be
given and a non-zero exit code will be returned.

## Input file decoding

The input file should be a PNG file containing the image to be converted.
The height of the image must be a multiple of 8 pixels (8, 16, 24, 32, ...).
The width can be any size.

The image will be translated to a raw array of 32 bit RGBA (Red, Green, Blue,
Alpha) pixels internally before being processed to output. Ideally, pixels that
are to be drawn (represented as a 1 in the image output) should be fully white.
Non-drawn (0) pixels should be fully black. Pixels intended to be masked out of
the image (represented as a 0 in both the image and mask output), should be
fully transparent and their color doesn't matter.

However, after translation to RGBA, any pixel with an alpha (opaqueness) value
of 127 or less will be set as non-drawn (0) for both the image and the mask.
For the image, after the alpha value is first taken into account, pixels with a
red color value greater than 127 will be set as drawn (1) and others will be
set as non-drawn (0). For the mask, only the alpha value is used and red is
ignored. Green and blue color values are ignored for both image and mask.

### To summarize:

For the image:

Green and blue are ignored.

| Alpha  |   Red   | Output |
|:------:|:-------:|:------:|
| <= 127 | <= 127  |   0    |
| <= 127 |  > 127  |   0    |
|  > 127 | <= 127  |   0    |
|  > 127 |  > 127  |   1    |

For the mask:

Red, green and blue are ignored.

| Alpha  | Output |
|:------:|:------:|
| <= 127 |   0    |
|  > 127 |   1    |

## Output

Cabi will send all output to `stdout`, which is usually the console unless
redirected. To save the output, you may be able to copy and paste it into your
editor, or you can redirect `stdout` to a file for importing. For example:

`cabi PlayerSprite.png PlayerSprite > PlayerSprite.out`

If conversion is successful, the output will be text representing C/C++ code
for two arrays, an image and a mask, that can be included in a sketch for use
by the *drawCompressed()* function. The image array will be named the same as
the prefix. The mask name will be the prefix with `_mask` appended to it.

Along with the actual array text, a comment will be included before each array
giving the input file name used and the dimensions of the image. A comment
included after each array will give the size of the array and the compression
ratio compared to the non-compressed equivalent (although the ratio is based
on the compressed array including two bytes for the bitmap dimensions compared
to a non-compressed array without bitmap dimensions).

Note that it's possible that the "compressed" array will actually end up
larger than the equivalent non-compressed one would. This is indicated by
a compression ratio greater than 1. The ratio should be noted and taken into
account when determining whether using Cabi compressed bitmaps is suitable for
the intended purpose.

If masking isn't required, the mask array can be ignored or deleted.

Note that the usage message or any error message will also be sent to `stdout`,
rather than `stderr`. Therefore, if you redirect the output to a file, in this
case the file will contain only that text.

## Using the output with *drawCompressed()*

The Arduboy2 *drawCompressed()* function doesn't natively handle a mask for
"transparent" pixels in an image. However, masking can be accomplished by
calling *drawCompressed()* twice with the same coordinates. The first call
specifies the mask array and the color BLACK. The second call specifies the
image array and the color WHITE.

An example PNG bitmap named `sample.png` is included with the program. Here is
an example Arduboy sketch that draws this bitmap with masking, using the Cabi
output imported into the sketch.

```cpp
#include <Arduboy2.h>

Arduboy2 arduboy;

// ===== Cabi output =====
// sample.png  width: 32 height: 32
const PROGMEM uint8_t sample[] = {
0x1f,0x1f,0x68,0x93,0xca,0x39,0xe5,0x9c,0x72,0xca,0xe9,0x74,0x4b,0x25,0x95,0xdc,
0x6e,0xb7,0xdb,0xed,0x56,0x49,0x65,0xb7,0x4a,0x3a,0xa9,0xac,0x92,0x4e,0x3a,0xa9,
0x74,0x94,0x8c,0x6a,0xbb,0xdd,0x6e,0xb7,0x8c,0x76,0xbb,0xdd,0x6e,0xb7,0xdb,0xed,
0x76,0xbb,0xdd,0xf2,0xf1,0xa6,0xb7,0x52,0x79,0xc5,0xa4,0xbc,0x92,0x76,0x1d,0x2f,
0x9f,0xdd,0x6e,0xb7,0xdb,0xed,0x76,0xbb,0xdd,0x6e,0xb7,0x8c,0xf4,0xd9,0x15,0x23,
0x65,0x5a,0x49,0x27,0x9d,0x54,0x56,0x49,0x27,0x95,0xdd,0x2a,0xa9,0xec,0x76,0xbb,
0xdd,0x6e,0x97,0x4a,0x2a,0xb9,0x54,0xce,0x39,0xe5,0x94,0x73,0xca,0x39,0x25,0xa3,
0x05
};
// bytes:113 ratio: 0.883

const PROGMEM uint8_t sample_mask[] = {
0x1f,0x1f,0x68,0x93,0xca,0x39,0x25,0x95,0xdc,0xa6,0xd3,0xa1,0x35,0x9d,0x4e,0x6f,
0x95,0x54,0xd2,0x39,0xa9,0x74,0x94,0xe8,0xb4,0xdb,0xed,0x76,0xbb,0xdd,0x6e,0xb7,
0xdb,0xed,0x16,0x8f,0x8a,0x49,0xe1,0xd1,0x6e,0xb7,0xdb,0xed,0x76,0xbb,0xdd,0x6e,
0xb7,0x5b,0x74,0x52,0xa6,0x95,0x74,0x4e,0x2a,0xa9,0xec,0x3a,0x9d,0x0e,0xad,0xe9,
0x74,0x76,0xa9,0xa4,0x72,0x4e,0xc9,0x68,0x01
};
// bytes:73 ratio: 0.570
// =======================

void setup() {
  arduboy.begin();
}

void loop() {
  arduboy.clear();

  arduboy.drawCompressed(20, 10, sample_mask, BLACK);
  arduboy.drawCompressed(20, 10, sample, WHITE);

  arduboy.display();
}
```

