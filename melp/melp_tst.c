/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).


*/

/*                                                                  */
/*  melp.c: Mixed Excitation LPC mf_speech coder                       */
/*                                                                  */

/*  compiler include files  */
#include	<stdio.h>
#include "melp.h"
#include "spbstd.h"
#include "mat.h"

/*  compiler constants */
#define ANA_SYN 0
#define ANALYSIS 1
#define SYNTHESIS 2

/* note: CHSIZE is shortest integer number of words in channel packet */
#define CHSIZE 9
#define NUM_CH_BITS 54

/*  external memory */
int melpmode = ANA_SYN;
char in_name[80], out_name[80];
 
void main(int argc, char **argv)

{
    void parse(int argc, char **argv);

    int length, frame, eof_reached;
    int num_frames = 0;
    float mf_speech_in[mf_FRAME];
    float mf_speech_out[mf_FRAME];
    static struct mf_melp_param mf_melp_par;      /* melp parameters */
    unsigned int chbuf[CHSIZE];
    FILE *fp_in, *fp_out;

    /* Print user message */
    printf("\n2.4 kb/s Proposed Federal Standard MELP mf_speech coder\n");
    printf("  C simulation, version 1.2\n\n");

    /* Get input parameters from command line */
    parse(argc, argv);

    /* Open input, output, and parameter files */
    if (( fp_in = fopen(in_name,"rb")) == NULL ) {
	printf("  ERROR: cannot read file %s.\n",in_name);
	exit(1);
    }
    if (( fp_out = fopen(out_name,"wb")) == NULL ) {
	printf("  ERROR: cannot write file %s.\n",out_name);
	exit(1);
    }

    /* Check length of channel input if needed */
    if (melpmode == SYNTHESIS) {
	fseek(fp_in,0L,2);
	length = ftell(fp_in);
	rewind(fp_in);

	num_frames = 0.5 + length * (8.0 / NUM_CH_BITS) * (6.0/32);
	
    }

    /* Initialize MELP analysis and synthesis */
    if (melpmode != SYNTHESIS)
      mf_mf_melp_ana_init();
    if (melpmode != ANALYSIS)
      mf_mf_melp_syn_init();

    /* Run MELP coder on input signal */
    frame = 0;
    mf_melp_par.chptr = chbuf;
    mf_melp_par.chbit = 0;
    eof_reached = 0;
    while (eof_reached == 0) {

	/* Perform MELP analysis */
	if (melpmode != SYNTHESIS) {
	    /* read input mf_speech */
	    length = mf_readbl(mf_speech_in,fp_in,mf_FRAME);
	    if (length < mf_FRAME) {
		mf_v_zap(&mf_speech_in[length],mf_FRAME-length);
		eof_reached = 1;
	    }

	    /* Run MELP analyzer */
	    if (melpmode == ANA_SYN) {
		/* reset pointers to short channel buffer */
		mf_melp_par.chptr = chbuf;
		mf_melp_par.chbit = 0;
	    }

	    mf_melp_ana(mf_speech_in,&mf_melp_par);
 
	    /* Write channel output if needed */
	    if (melpmode == ANALYSIS && mf_melp_par.chbit == 0) {
		fwrite((void *) chbuf,sizeof(int),mf_melp_par.chptr-chbuf,fp_out);

		/* reset pointer to short channel buffer */
		mf_melp_par.chptr = chbuf;
	    }

	    if (mf_melp_par.chptr >= &chbuf[CHSIZE] && mf_melp_par.chbit > 0) {
		printf("\nERROR: Ran out of channel buffer memory.\n");
		exit(1);
	    }

	}          

	/* Perform MELP synthesis (skip first frame) */
	if (melpmode != ANALYSIS) {

	    if (melpmode == ANA_SYN) {
		/* reset pointers to short channel buffer */
		mf_melp_par.chptr = chbuf;
		mf_melp_par.chbit = 0;
	    }

	    /* Read channel input if needed */
	    if (melpmode == SYNTHESIS && mf_melp_par.chbit == 0) {
		fread((void *) chbuf,sizeof(int),CHSIZE,fp_in);

		/* reset pointer to short channel buffer */
		mf_melp_par.chptr = chbuf;
	    }

	    mf_melp_syn(&mf_melp_par,mf_speech_out);
	    if (frame > 0)
	      mf_writebl(mf_speech_out,fp_out,mf_FRAME);
	}

	frame++; 
	if (melpmode == SYNTHESIS) {
	    if (frame >= num_frames)
	      eof_reached = 1;
	}
    }
 
    /* Write channel output if needed */
    if (melpmode == ANALYSIS) {
	if (mf_melp_par.chbit > 0)
	  fwrite((void *) chbuf,sizeof(int),mf_melp_par.chptr-chbuf+1,fp_out);
	else
	  fwrite((void *) chbuf,sizeof(int),mf_melp_par.chptr-chbuf,fp_out);
    }

    fclose(fp_in);
    fclose(fp_out);

}

void parse(int argc,char **argv){

    int error_flag;

    error_flag = 0;
    if (argc < 2)
      error_flag = 1;
    melpmode = ANA_SYN;
    while ((--argc>0) && ((*++argv)[0] == '-')){
	switch ((*argv)[1]){
	  case 'a':
	    melpmode=ANALYSIS; break;
	  case 's':
	    melpmode=SYNTHESIS; break;
	  case 'i':
	    sscanf(*++argv,"%s",in_name); --argc; break;
	  case 'o':
	    sscanf(*++argv,"%s",out_name); --argc; break;
	  default:
	    error_flag = 1;
	    break;
	}
    }

    if (error_flag == 1) {
	fprintf(stderr,"Usage:\n\n"); 
	fprintf(stderr,"Analysis/synthesis: melp -i infile -o outfile\n"); 
	fprintf(stderr,"Analysis only:  melp -a -i infile  -o bitfile\n"); 
	fprintf(stderr,"Synthesis only: melp -s -i bitfile -o outfile\n"); 
        exit(1);
    }
    if (melpmode == ANA_SYN)
      printf(" MELP analysis and synthesis \n");
    else if (melpmode == ANALYSIS)
      printf(" MELP analysis \n");
    else if (melpmode == SYNTHESIS)
      printf(" MELP synthesis \n");

    printf("   input from %s\n   output to %s.\n",in_name, out_name);

}


