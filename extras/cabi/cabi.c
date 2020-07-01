/*
	abe.c

	arduboy encoder	
	monochrome rle encoding by zep
	License: CC-0

	cabi in.png [array_name_prefix]
	
*/

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "lodepng.h"

// alternative pixel order mapping
//#define READING_ORDER 1

int reading_order = 0;

typedef unsigned char uint8_t;

#ifndef WIDTH
	#define WIDTH 128
	#define HEIGHT 64
	#define PROGMEM
#endif

// one byte encodes a 1x8 stick; low byte at top




// for testing
void draw_sprite_ascii(const uint8_t *dat, int w, int h)
{
	int x, y;
	int row, bit;
	int rows;
	
	rows = (h+7)/8;
	
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
	int byte;
	int bit;
	const uint8_t *src;
	uint8_t *dest;
	int src_pos;
	int out_pos;
	int w, h;
}CSESSION;
static CSESSION cs;

// get an n-bit number from the compressed data stream
static int getval(int bits)
{
	int val = 0;
	int i;
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
	int col;
	int pos;
	int bl, len;
	int i;
	int w, h;
	int x, y;
	int total = 0;
	
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
	printf("\ntotal: %d\n", total);
}


#if 0


#endif

// ----------------------------------------------------------------------------
// :: Compress
// ----------------------------------------------------------------------------

/*
	getcol
	
	pos is the index of the pixel:  0 .. w*h-1
*/
static int getcol(int pos)
{
	int x, y;
	
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

static int find_rlen(int pos, int plen) 
{
	int col;
	int pos0;
	
	col = getcol(pos);
	pos0 = pos;

	while(getcol(pos) == col && pos < plen)
		pos ++;
	
	return pos-pos0;
}

// write a bit to the stream. non-zero val means 1, otherwise 0.
static void putbit(int val)
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
static void putval(int val, int bits)
{
	int i;
	
	if (bits <= 0) return;
	for (i = 0; i < bits; i++)
		putbit(val & (1 << i));
}

// write a span length
// a string of bits encoding the number of bits needed to encode the length,
// and then the length.
static int putsplen(int len)
{
	int blen = 1; // how bits needed to encode length
	while ((1 << blen) <= len)
		blen += 2;
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
int compress_rle(const uint8_t *src, int w, int h, char *prefix, char *suffix)
{
	int pos;
	int rlen;
	int len;
	
	printf("const uint8_t PROGMEM %s%s[] = {", prefix, suffix);
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
	int compressed_len;
	int pixels;
	int w,h;
	unsigned char *bmp = NULL;
	unsigned char *bmp0 = NULL;
	unsigned char *bmp1 = NULL;
	int result;
	int rawlen;
	int x, y;
	int row,bit;
	char prefix[256] = "out";
	
	
	if (argc < 2)
	{
		printf("usage: cabi in.png [array_name_prefix]\n");
	}
	
	if (argc >= 3)
		strcpy(prefix, argv[2]);
	
	result = lodepng_decode_file(&bmp, &w, &h, argv[1], 6, 8);
	
	if (result != 0)
	{
		printf("could not load %s\n", argv[1]);
		exit(0);
	}
	
	// generate sprite and mask
	
	rawlen = w * (h+7) / 8;
	
	bmp0 = malloc(rawlen); memset(bmp0, 0, rawlen);
	bmp1 = malloc(rawlen); memset(bmp1, 0, rawlen);
	
	printf("// %s  width: %d height: %d\n", argv[1], w, h);
	
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			row = y/8;
			bit = y&7;
			
			if (bmp[(x+y*w)*4 + 3] > 128) // need to be opaque to count
			if (bmp[(x+y*w)*4 + 0] > 128)
			{
				// set sprite
				bmp0[x + (row*w)] |= (1 << bit);
			}
			
			if (bmp[(x+y*w)*4 + 3] > 128)
			{
				// set mask
				bmp1[x + (row*w)] |= (1 << bit);
			}
			
		}
	}
	
	compressed_len = compress_rle(bmp0, w, h, prefix, "");
	printf("// bytes:%d ratio: %3.3f\n\n", compressed_len, (float)(compressed_len * 8)/ (float)(w*h));

	compressed_len = compress_rle(bmp1, w, h, prefix, "_mask");
	printf("// bytes:%d ratio: %3.3f\n\n", compressed_len, (float)(compressed_len * 8)/ (float)(w*h));
	

	free(bmp0);
	free(bmp1);
	
	return 0;
}
