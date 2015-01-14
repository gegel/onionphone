/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _LPC10TOOLS_H_
#define _LPC10TOOLS_H_

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

void lpc10_build_bits(unsigned char *c, int32_t * bits);
void lpc10_extract_bits(int32_t * bits, unsigned char *c);
int32_t pow_ii(int32_t * ap, int32_t * bp);
double r_sign(float *a, float *b);
int32_t i_nint(float *x);

#endif				/* _LPC10TOOLS_H_ */
