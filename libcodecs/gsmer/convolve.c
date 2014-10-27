/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_Convolve
 *
 *  PURPOSE:
 *     Perform the convolution between two vectors x[] and h[] and
 *     write the result in the vector y[]. All vectors are of length L
 *     and only the first L samples of the convolution are computed.
 *
 *  DESCRIPTION:
 *     The convolution is given by
 *
 *          y[n] = sum_{i=0}^{n} x[i] h[n-i],        n=0,...,L-1
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

void w_Convolve(int16_t x[],	/* (i)     : input vector                           */
		int16_t h[],	/* (i)     : impulse response                       */
		int16_t y[],	/* (o)     : output vector                          */
		int16_t L	/* (i)     : vector size                            */
    )
{
	int16_t i, n;
	int32_t s;

	for (n = 0; n < L; n++) {
		s = 0;
		for (i = 0; i <= n; i++) {
			s = w_L_mac(s, x[i], h[n - i]);
		}
		s = w_L_w_shl(s, 3);
		y[n] = w_extract_h(s);
	}

	return;
}
