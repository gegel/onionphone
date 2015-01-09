/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#include "lpc10.h"

void lpc10_build_bits(unsigned char *c, INT32 * bits)
{
	unsigned char mask = 0x80;
	int x;

	*c = 0;
	for (x = 0; x < LPC10_BITS_IN_COMPRESSED_FRAME; x++) {
		if (bits[x])
			*c |= mask;
		mask = mask >> 1;
		if ((x % 8) == 7) {
			c++;
			*c = 0;
			mask = 0x80;
		}
	}
}

void lpc10_extract_bits(INT32 * bits, unsigned char *c)
{
	int x;
	for (x = 0; x < LPC10_BITS_IN_COMPRESSED_FRAME; x++) {
		if (*c & (0x80 >> (x & 7)))
			bits[x] = 1;
		else
			bits[x] = 0;
		if ((x & 7) == 7)
			c++;
	}
}
