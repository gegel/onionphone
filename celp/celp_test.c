/*

    	    CELP Codec test driver
	
	This program uses the CELP encoder to compress a
	speech file recorded in 16 bit two's complement
	signed PCM into sequence of CELP frames in an output
	file named "test,clp", then uses the CELP decoder
	to synthesise an output file "ofile.spd", also in
	16 bit two's complement PCM.  The PCM files are in
	big-endian order; the program will work regardless
	of byte order of the system on which it is run.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include "celp.h"

#define INFILE	"speech.spd"
#define CLPFILE "test.clp"
#define OUTFILE "ofile.spd"

#define CELP_FRAME  240     	/* PCM samples per CELP frame */

/*  READ_PCM  -- Read 16 bit PCM file in byte-order independent manner.  */

static int read_pcm(FILE *f, short pcmframe[CELP_FRAME])
{
    int i, ch, cl;
    
    for (i = 0; i < CELP_FRAME; i++) {
    	ch = getc(f);
	if (ch == EOF) {
	    return 0;
	}
    	cl = getc(f);
	if (cl == EOF) {
	    return 0;
	}
	pcmframe[i] = (ch << 8) | cl;
    }
    return 1;
}

/*  WRITE_PCM  -- Write 16 bit PCM file in byte-order independent manner.  */

static void write_pcm(FILE *f, short pcmframe[CELP_FRAME])
{
    int i;
    
    for (i = 0; i < CELP_FRAME; i++) {
    	putc((pcmframe[i] >> 8) & 0xFF, f);
	putc(pcmframe[i] & 0xFF, f);
    }
}

int main(void)
{
    FILE *celp = fopen(CLPFILE, "wb"),
    	 *synth = fopen(OUTFILE, "wb"),
	 *in = fopen(INFILE, "rb");
    short pcmframe[CELP_FRAME];
    char celpframe[18];
    
    if (in == NULL) {
    	fprintf(stderr, "Cannot open input file: %s\n", INFILE);
	return 2;
    }
    
    if (celp == NULL) {
    	fprintf(stderr, "Cannot create CELP-encoded file: %s\n", CLPFILE);
	return 2;
    }
    
    if (synth == NULL) {
    	fprintf(stderr, "Cannot create output PCM file: %s\n", OUTFILE);
	return 2;
    }

    celp_init(1);
    while (read_pcm(in, pcmframe)) {
    	celp_encode(pcmframe, celpframe);
	fwrite(celpframe, sizeof celpframe, 1, celp);
    }
    fclose(in);
    fclose(celp);

    celp = fopen(CLPFILE, "rb");
    
    if (celp == NULL) {
    	fprintf(stderr, "Cannot reopen CELP-encoded file for reading: %s\n", CLPFILE);
	return 2;
    }
    
    while (fread(celpframe, sizeof celpframe, 1, celp) == 1) {
    	celp_decode(celpframe, pcmframe);
	write_pcm(synth, pcmframe);
    }
    fclose(celp);
    fclose(synth);

    return 0;
}
