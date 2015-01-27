/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _SPEEX_TYPES_H_
#define _SPEEX_TYPES_H_

/** Speex echo cancellation state. */
typedef struct SpeexEchoState_ {
	int frame_size;	     /**< Number of samples processed each time */
	int window_size;
	spx_word16_t leak_estimate;

	spx_word16_t *y;	/* scratch */
	spx_word16_t *last_y;
	spx_word16_t *Y;	/* scratch */
	spx_word16_t *window;
	void *fft_table;
} SpeexEchoState;

#endif /* _SPEEX_TYPES_H_ */

