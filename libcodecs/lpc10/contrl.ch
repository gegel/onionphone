/*   LPC Processing control variables:
*/


/*  Files for Speech, Parameter, and Bitstream Input & Output,
      and message and debug outputs.
*/
#if ANALYZER + SYNTHESIZER

#include <stdio.h>

extern FILE *fbi, *fbo, *fopen();
#endif
/*  Quantization, Listing control	*/
/*	int quant, listl, lincnt;*/

/*  Current frame, frames per block, current frame in block, bits per block
       Unstable frames, Output clip count, Max onset buffer
*/
extern int nframe, nfb, nbits, nunsfm, iclip, maxosp, NFBLK;
