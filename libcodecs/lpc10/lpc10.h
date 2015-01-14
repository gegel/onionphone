/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

$Log$
Revision 1.18  2004/08/31 13:32:11  markster
Merge NetBSD and Courtesty tone with modifications (bug #2329)

Revision 1.17  2003/10/26 18:50:49  markster
Make it build and run on MacOS X

Revision 1.3  2003/10/26 18:50:49  markster
Make it build and run on MacOS X

Revision 1.2  2003/04/23 19:13:35  markster
More OpenBSD patches

Revision 1.1.1.2  2003/03/16 22:37:30  matteo
dom mar 16 23:37:23 CET 2003

Revision 1.2  2003/03/16 16:09:48  markster
Mere James's cleanups for fewer build warnings

Revision 1.1  2000/01/05 00:20:06  markster
Add broken lpc10 code...  It's not too far from working I don't think...

 * Revision 1.1  1996/08/19  22:47:31  jaf
 * Initial revision
 *

*/

#ifndef __LPC10_H__
#define __LPC10_H__

#include <stdint.h>

#define LPC10_SAMPLES_PER_FRAME 180
#define LPC10_BITS_IN_COMPRESSED_FRAME 54

/* The initial values for every member of this structure is 0, except
   where noted in comments. */

typedef struct lpc10_contrl {
	int32_t order, lframe;
	int32_t corrp;
} lpc10_contrl_t;

struct lpc10_encoder_state {
	/* State used only by function hp100 */
	float z11;
	float z21;
	float z12;
	float z22;

	/* State used by function analys */
	float inbuf[540], pebuf[540];
	float lpbuf[696], ivbuf[312];
	float bias;
	int32_t osbuf[10];	/* no initial value necessary */
	int32_t osptr;		/* initial value 1 */
	int32_t obound[3];
	int32_t vwin[6] /* was [2][3] */ ;	/* initial value vwin[4] = 307; vwin[5] = 462; */
	int32_t awin[6] /* was [2][3] */ ;	/* initial value awin[4] = 307; awin[5] = 462; */
	int32_t voibuf[8] /* was [2][4] */ ;
	float rmsbuf[3];
	float rcbuf[30] /* was [10][3] */ ;
	float zpre;

	/* State used by function onset */
	float n;
	float d__;		/* initial value 1.f */
	float fpc;		/* no initial value necessary */
	float l2buf[16];
	float l2sum1;
	int32_t l2ptr1;		/* initial value 1 */
	int32_t l2ptr2;		/* initial value 9 */
	int32_t lasti;		/* no initial value necessary */
	int32_t hyst;		/* initial value 0 */

	/* State used by function voicin */
	float dither;		/* initial value 20.f */
	float snr;
	float maxmin;
	float voice[6] /* was [2][3] */ ;	/* initial value is probably unnecessary */
	int32_t lbve, lbue, fbve, fbue;
	int32_t ofbue, sfbue;
	int32_t olbue, slbue;
	/* Initial values:
	   lbve = 3000;
	   fbve = 3000;
	   fbue = 187;
	   ofbue = 187;
	   sfbue = 187;
	   lbue = 93;
	   olbue = 93;
	   slbue = 93;
	   snr = (float) (fbve / fbue << 6);
	 */

	/* State used by function dyptrk */
	float s[60];
	int32_t p[120] /* was [60][2] */ ;
	int32_t ipoint;
	float alphax;

	/* State used by function chanwr */
	int32_t isync;

};

struct lpc10_decoder_state {

	/* State used by function decode */
	int32_t iptold;		/* initial value 60 */
	int32_t first;		/* initial value 1 */
	int32_t ivp2h;
	int32_t iovoic;
	int32_t iavgp;		/* initial value 60 */
	int32_t erate;
	int32_t drc[30] /* was [3][10] */ ;
	int32_t dpit[3];
	int32_t drms[3];

	/* State used by function synths */
	float buf[360];
	int32_t buflen;		/* initial value 180 */

	/* State used by function pitsyn */
	int32_t ivoico;		/* no initial value necessary as long as first_pitsyn is initially 1 */
	int32_t ipito;		/* no initial value necessary as long as first_pitsyn is initially 1 */
	float rmso;		/* initial value 1.f */
	float rco[10];		/* no initial value necessary as long as first_pitsyn is initially 1 */
	int32_t jsamp;		/* no initial value necessary as long as first_pitsyn is initially 1 */
	int32_t first_pitsyn;	/* initial value 1 */

	/* State used by function bsynz */
	int32_t ipo;
	float exc[166];
	float exc2[166];
	float lpi1;
	float lpi2;
	float lpi3;
	float hpi1;
	float hpi2;
	float hpi3;
	float rmso_bsynz;

	/* State used by function random */
	int32_t j;		/* initial value 2 */
	int32_t k;		/* initial value 5 */
	int16_t y[5];		/* initial value { -21161,-8478,30892,-10216,16950 } */

	/* State used by function deemp */
	float dei1;
	float dei2;
	float deo1;
	float deo2;
	float deo3;

};

/*

  Calling sequence:

  Call create_lpc10_encoder_state(), which returns a pointer to an
  already initialized lpc10_encoder_state structure.

  lpc10_encode reads indices 0 through (LPC10_SAMPLES_PER_FRAME-1) of
  array speech[], and writes indices 0 through
  (LPC10_BITS_IN_COMPRESSED_FRAME-1) of array bits[], and both reads
  and writes the lpc10_encoder_state structure contents.  The
  lpc10_encoder_state structure should *not* be initialized for every
  frame of encoded speech.  Once at the beginning of execution, done
  automatically for you by create_lpc10_encoder_state(), is enough.

  init_lpc10_encoder_state() reinitializes the lpc10_encoder_state
  structure.  This might be useful if you are finished processing one
  sound sample, and want to reuse the same lpc10_encoder_state
  structure to process another sound sample.  There might be other
  uses as well.

  Note that the comments in the lpc10/lpcenc.c file imply that indices
  1 through 180 of array speech[] are read.  These comments were
  written for the Fortran version of the code, before it was
  automatically converted to C by the conversion program f2c.  f2c
  seems to use the convention that the pointers to arrays passed as
  function arguments point to the first index used in the Fortran
  code, whatever index that might be (usually 1), and then it modifies
  the pointer inside of the function, like so:

  if (speech) {
      --speech;
  }

  So that the code can access the first value at index 1 and the last
  at index 180.  This makes the translated C code "closer" to the
  original Fortran code.

  The calling sequence for the decoder is similar to the encoder.  The
  only significant difference is that the array bits[] is read
  (indices 0 through (LPC10_BITS_IN_COMPRESSED_FRAME-1)), and the
  array speech[] is written (indices 0 through
  (LPC10_SAMPLES_PER_FRAME-1)).
  
  */

struct lpc10_encoder_state *create_lpc10_encoder_state(void);
void init_lpc10_encoder_state(struct lpc10_encoder_state *st);
int lpc10_encode(float *speech, int32_t * bits, struct lpc10_encoder_state *st);

struct lpc10_decoder_state *create_lpc10_decoder_state(void);
void init_lpc10_decoder_state(struct lpc10_decoder_state *st);
int lpc10_decode(int32_t * bits, float *speech, struct lpc10_decoder_state *st);

#endif				/* __LPC10_H__ */
