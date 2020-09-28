/*
cabi - Compress Arduboy Image

A command line program to read a PNG file containing a bitmap image, compress
it using RLE encoding and convert it to C/C++ code suitable for use with the
Team A.R.G. drawCompressed() function. This function is included in the
Arduboy2 library.

Written by zep
https://www.lexaloffle.com/bbs/?uid=1
https://twitter.com/lexaloffle
Contributed to Team A.R.G.

Modifications by Scott Allen - July 2020, September 2020

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

Usage:
cabi in.png [array_name_prefix]
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include "lodepng/lodepng.h"

// alternative pixel order mapping
//#define READING_ORDER 1

unsigned reading_order = 0;

// one byte encodes a 1x8 stick; low byte at top



// for testing
void draw_sprite_ascii(const uint8_t *dat, unsigned w, unsigned h)
{
	unsigned x, y;
	unsigned row, bit;

	for (y = 0; y < h; y ++)
	{
		row = y/8;
		bit = y&7;

		for (x = 0; x < w; x++)
		{
			if (dat[x + (row*w)] & (1 << bit))
				printf("#");
			else
				printf(".");
		}
		printf("\n");
	}
}



// ----------------------------------------------------------------------------
// :: Decompress
// ----------------------------------------------------------------------------

// compression / decompression session state

typedef struct CSESSION{
	unsigned byte;
	unsigned bit;
	const uint8_t *src;
	uint8_t *dest;
	unsigned src_pos;
	unsigned out_pos;
	unsigned w, h;
}CSESSION;
static CSESSION cs;

// get an n-bit number from the compressed data stream
static unsigned getval(unsigned bits)
{
	unsigned val = 0;
	unsigned i;
	for (i = 0; i < bits; i++)
	{
		if (cs.bit == 0x100)
		{
			cs.bit = 0x1;
			cs.byte = cs.src[cs.src_pos];
			cs.src_pos ++;
		}
		if (cs.byte & cs.bit)
			val += (1 << i);
		cs.bit <<= 1;
	}
	return val;
}


// decompress_rle
// if not NULL, w and h give back the size of the sprite.
void draw_compressed_sprite_ascii(const uint8_t *src)
{
	unsigned col;
	unsigned bl, len;
	unsigned i;
	unsigned w, h;
	unsigned x, y;
	unsigned total = 0;

	memset(&cs, 0, sizeof(cs));
	cs.src = src;
	cs.bit = 0x100;
	cs.src_pos = 0;

	// header

	w = getval(8) + 1;
	h = getval(8) + 1;
	col = getval(1); // starting colour

	x = y = 0;

	while (y < h)
	{
		bl = 1;
		while (!getval(1))
			bl += 2;

		len = getval(bl)+1; // span length

		for (i = 0; i < len; i++)
		{
			//if ((x%8) == 0) // every 8th bit (format test)
			printf("%s", col ? "#":".");

			if (col) total++;
			x++;
			if (x >= w)
			{
				printf("\n");
				y ++;
				x = 0;
			}

			//if ((x+y*w)%(w*8) == 0) printf("\n"); // print every 8th line (format test)

		}

		col = 1-col; // toggle
	}
	printf("\ntotal: %u\n", total);
}



// ----------------------------------------------------------------------------
// :: Compress
// ----------------------------------------------------------------------------

/*
	getcol

	pos is the index of the pixel:  0 .. w*h-1
*/
static unsigned getcol(unsigned pos)
{
	unsigned x, y;

	// display order

	if (reading_order == 0)
	{
		if (cs.src[pos/8] & (1 << (pos&7))) return 1;
		return 0;
	}

	// reading order (compresses slightly better but harder to optimize sprite blit)
	// or use this after loading png into display order (no need for extra conversion)

	x = (pos % cs.w);
	y = (pos / cs.w);
	if (cs.src[x + ((y/8)*cs.w)] & (1 << (y&7))) return 1;
	return 0;

}

static unsigned find_rlen(unsigned pos, unsigned plen)
{
	unsigned col;
	unsigned pos0;

	col = getcol(pos);
	pos0 = pos;

	while(getcol(pos) == col && pos < plen)
		pos ++;

	return pos-pos0;
}

// write a bit to the stream. non-zero val means 1, otherwise 0.
static void putbit(unsigned val)
{
	if (val) cs.byte |= cs.bit;
	cs.bit <<= 1;
	if (cs.bit == 0x100)
	{
		//output byte
		if (cs.out_pos != 0) printf(",");
		if (cs.out_pos % 16 == 0) printf("\n");
		printf("0x%02x", cs.byte);

		cs.out_pos ++;
		cs.bit = 0x1;
		cs.byte = 0;

	}
}

// write an n-bit (bits) number (val) to the output steam
static void putval(unsigned val, unsigned bits)
{
	unsigned i;

	if (bits <= 0) return;
	for (i = 0; i < bits; i++)
		putbit(val & (1 << i));
}

// write a span length
// a string of bits encoding the number of bits needed to encode the length,
// and then the length.
static void putsplen(unsigned len)
{
	unsigned blen = 1; // how bits needed to encode length
	while ((unsigned)(1 << blen) <= len) {
		blen += 2;
	}
	// write number of bits (1-terminated string of zeroes)
	putval(0,(blen-1)/2);
	putval(1,1);          // terminator
	// write length
	putval(len, blen);
}

/*
	comp

	compress plen 1-bit pixels from src to dest

*/
unsigned compress_rle(const uint8_t *src, unsigned w, unsigned h, char *prefix, char *suffix)
{
	unsigned pos;
	unsigned rlen;

	printf("const PROGMEM uint8_t %s%s[] = {", prefix, suffix);
	fflush(stdout);

	memset(&cs, 0, sizeof(cs));
	cs.src = src;
	cs.bit = 1;
	cs.w = w;
	cs.h = h;

	// header
	putval(w-1, 8);
	putval(h-1, 8);
	putval(getcol(0), 1); // first colour

	pos = 0;

	// span data

	while (pos < w*h)
	{
		rlen = find_rlen(pos, w*h);
		pos += rlen;
		putsplen(rlen-1);
	}

	// pad with zeros and flush
	while (cs.bit != 0x1)
		putbit(0);

	printf("\n};\n");

	return cs.out_pos; // bytes
}



int main(int argc, char **argv)
{
	unsigned compressed_len;
	unsigned w, h;
	unsigned char *bmp = NULL;
	unsigned char *bmp0 = NULL;
	unsigned char *bmp1 = NULL;
	unsigned result;
	unsigned rawlen;
	unsigned x, y;
	unsigned row, bit;
	char default_prefix[] = "compressed_image";
	char *prefix = default_prefix;


	if (argc < 2)
	{
		printf("cabi - Compress Arduboy Image\n");
		printf("Convert a PNG file into RLE encoded C/C++ source\n");
		printf("for use with Arduboy2 drawCompressed()\n\n");

		printf("usage: cabi in.png [array_name_prefix]\n");
		exit(1);
	}

	if (argc >= 3) {
		prefix = argv[2];
	}

	result = lodepng_decode32_file(&bmp, &w, &h, argv[1]);

	if (result != 0) {
		printf("error %u: file %s: %s\n", result, argv[1], lodepng_error_text(result));
		free(bmp);
		exit(result);
	}

	if (h % 8 != 0) {
		printf("error 120: file %s: image height must be a multiple of 8 but is %u\n", argv[1], h);
		free(bmp);
		exit(120);
	}

	// generate sprite and mask

	rawlen = w * (h+7) / 8;

	bmp0 = malloc(rawlen); memset(bmp0, 0, rawlen);
	bmp1 = malloc(rawlen); memset(bmp1, 0, rawlen);

	printf("// %s  width: %u height: %u\n", argv[1], w, h);

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			row = y/8;
			bit = y&7;
			
			if (bmp[(x+y*w)*4 + 3] > 127) // need to be opaque to count
			if (bmp[(x+y*w)*4 + 0] > 127)
			{
				// set sprite
				bmp0[x + (row*w)] |= (1 << bit);
			}
			
			if (bmp[(x+y*w)*4 + 3] > 127)
			{
				// set mask
				bmp1[x + (row*w)] |= (1 << bit);
			}

		}
	}

	compressed_len = compress_rle(bmp0, w, h, prefix, "");
	printf("// bytes:%u ratio: %3.3f\n\n", compressed_len, (float)(compressed_len * 8)/ (float)(w*h));

	compressed_len = compress_rle(bmp1, w, h, prefix, "_mask");
	printf("// bytes:%u ratio: %3.3f\n\n", compressed_len, (float)(compressed_len * 8)/ (float)(w*h));


	free(bmp);
	free(bmp0);
	free(bmp1);

	return 0;
}
