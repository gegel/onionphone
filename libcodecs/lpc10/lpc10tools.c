/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#include <math.h>

#include "lpc10.h"

void lpc10_build_bits(unsigned char *c, int32_t * bits)
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

void lpc10_extract_bits(int32_t * bits, unsigned char *c)
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

int32_t pow_ii(int32_t * ap, int32_t * bp)
{
	int32_t pow, x, n;
	unsigned long u;

	x = *ap;
	n = *bp;

	if (n <= 0) {
		if (n == 0 || x == 1)
			return 1;
		if (x != -1)
			return x == 0 ? 0 : 1 / x;
		n = -n;
	}
	u = n;
	for (pow = 1;;) {
		if (u & 01)
			pow *= x;
		if (u >>= 1)
			x *= x;
		else
			break;
	}
	return (pow);
}

double r_sign(float *a, float *b)
{
	double x;
	x = (*a >= 0 ? *a : -*a);
	return (*b >= 0 ? x : -x);
}

int32_t i_nint(float *x)
{
	return ((int32_t) ((*x) >= 0 ? floor(*x + .5) : -floor(.5 - *x)));
}
