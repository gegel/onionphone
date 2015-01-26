/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef KISS_FTR_H
#define KISS_FTR_H

#include "kiss_fft.h"
#ifdef __cplusplus
extern "C" {
#endif

/* 
 
 Real optimized version can save about 45% cpu time vs. complex fft of a real seq.

 
 
 */

	typedef struct kiss_fftr_state *kiss_fftr_cfg;

	kiss_fftr_cfg kiss_fftr_alloc(int nfft, int inverse_fft, void *mem,
				      size_t * lenmem);
/*
 nfft must be even

 If you don't care to allocate space, use mem = lenmem = NULL 
*/

	void kiss_fftr2(kiss_fftr_cfg st, const kiss_fft_scalar * timedata,
			kiss_fft_scalar * freqdata);

	void kiss_fftri2(kiss_fftr_cfg st, const kiss_fft_scalar * freqdata,
			 kiss_fft_scalar * timedata);

/*
 input freqdata has  nfft/2+1 complex points
 output timedata has nfft scalar points
*/

#define kiss_fftr_free free

#ifdef __cplusplus
}
#endif
#endif
