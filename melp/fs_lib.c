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

/*

  fs_lib.c: Fourier series subroutines 

*/

/*  compiler include files  */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spbstd.h"
#include "mat.h"
#include "fs.h"

/*  compiler constants */
#define PRINT 1

/*								*/
/*	Subroutine FIND_HARM: find Fourier coefficients using	*/
/*	FFT of input signal divided into pitch dependent bins.	*/
/*								*/
#define	FFTLENGTH	512

/* Memory definition		*/
static float mf_find_hbuf[2*FFTLENGTH];
static float mf_mag[FFTLENGTH];

void mf_find_harm(float input[], float fsmf_mag[], float pitch, int num_harm, 
	       int length)

{
    int	i, j, k, iwidth, i2;
    float temp, avg, fwidth;

    for (i = 0; i < num_harm; i++)
      fsmf_mag[i] = 1.0;
    avg = 0.0;

    /* Perform peak-picking on FFT of input signal */

    /* Calculate FFT of complex signal in scratch buffer	*/
    mf_v_zap(mf_find_hbuf,2*FFTLENGTH);
    for (i = 0; i < 2*length; i+=2)
	mf_find_hbuf[i] = input[i/2];
    mf_fft(mf_find_hbuf,FFTLENGTH,-1);
	
    /* Calculate mf_magnitude squared of coefficients		*/
    for (i = 0; i < FFTLENGTH; i++ )
	mf_mag[i] = mf_find_hbuf[2*i]*mf_find_hbuf[2*i] +
	    mf_find_hbuf[(2*i)+1]*mf_find_hbuf[(2*i)+1];
	
    /* Implement pitch dependent staircase function		*/
    fwidth = FFTLENGTH / pitch;	/* Harmonic bin width	*/
    iwidth = (int) fwidth;
    if (iwidth < 2)
	iwidth = 2;
    i2 = iwidth/2;
    avg = 0.0;
    if (num_harm > 0.25*pitch)
	num_harm = 0.25*pitch;
    for (k = 0; k < num_harm; k++) {
	i = ((k+1)*fwidth) - i2 + 0.5; /* Start at peak-i2 */
	j = i + mf_findmax(&mf_mag[i],iwidth);
	fsmf_mag[k] = mf_mag[j];
	avg += mf_mag[j];
    }

    /* Normalize Fourier series values to average mf_magnitude */
    temp = num_harm/(avg+ .0001);
    for (i = 0; i < num_harm; i++) {
	fsmf_mag[i] = sqrt(temp*fsmf_mag[i]);
    }

}

/*	Subroutine FFT: Fast Fourier Transform 		*/
/**************************************************************
* Replaces data by its DFT, if isign is 1, or replaces data   *
* by inverse DFT times nn if isign is -1.  data is a complex  *
* array of length nn, input as a real array of length 2*nn.   *
* nn MUST be an integer power of two.  This is not checked    *
* The real part of the number should be in the zeroeth        *
* of data , and the imf_maginary part should be in the next      *
* element.  Hence all the real parts should have even indeces *
* and the imf_maginary parts, odd indeces.			      *

* Data is passed in an array starting in position 0, but the  *
* code is copied from Fortran so uses an internal pointer     *
* which accesses position 0 as position 1, etc.		      *

* This code uses e+jwt sign convention, so isign should be    *
* reversed for e-jwt.                                         *
***************************************************************/
#define	SWAP(a,b) tempr = (a);(a) = (b); (b) = tempr

void mf_fft(float *datam1,int nn,int isign)

{
	int	n,mmax,m,j,istep,i;
	double	wtemp,wr,wpr,wpi,wi,theta;
	register float tempr,tempi;
	float	*data;

	/*  Use pointer indexed from 1 instead of 0	*/
	data = &datam1[-1];

	n = nn << 1;
	j = 1;
	for( i = 1; i < n; i+=2 ) {
	  if ( j > i) {
		SWAP(data[j],data[i]);
		SWAP(data[j+1],data[i+1]);
	  }
	  m = n >> 1;
	  while ( m >= 2 && j > m ) {
		j -= m;
		m >>= 1;
	  }
	  j += m;
	}
	mmax = 2;
	while ( n > mmax) {
	  istep = 2 * mmax;
	  theta = 6.28318530717959/(isign*mmax);
	  wtemp = sin(0.5*theta);
	  wpr   = -2.0*wtemp*wtemp;
	  wpi   = sin(theta);
	  wr = 1.0;
	  wi = 0.0;
	  for ( m = 1; m < mmax;m+=2) {
	    for ( i = m; i <= n; i += istep) {
   	      	j = i + mmax;
		tempr = wr * data[j] - wi * data[j+1];
	      	tempi = wr * data[j+1] + wi * data[j];
	   	data[j] = data[i] - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp=wr)*wpr-wi*wpi+wr;
	    wi = wi*wpr+wtemp*wpi+wi;
	  }
	  mmax = istep;
	}
}

/*								*/
/*	Subroutine FINDMAX: find maximum value in an 		*/
/*	input array.						*/
/*								*/
int 	mf_findmax(float input[], int npts)

{
register int	i, maxloc;
register float  maxval, *p_in;

	p_in = &input[0];
	maxloc = 0;
	maxval = input[maxloc];
	for (i = 1; i < npts; i++ ) {
		if (*(++p_in) > maxval) {
			maxloc = i;
			maxval = *p_in;
		}
	}
	return(maxloc);
}

/*								*/
/*	Subroutine IDFT_REAL: take inverse discrete Fourier 	*/
/*	transform of real input coefficients.			*/
/*	Assume real time signal, so reduce computation		*/
/*	using symmetry between lower and upper DFT		*/
/*	coefficients.						*/
/*								*/
#define DFTMAX 160

/* Memory definition	*/
static float	mf_idftc[DFTMAX];

void	mf_idft_real(float real[], float signal[], int length)

{
    int	i, j, k, k_inc, length2;
    float	w;

#if (PRINT)
    if (length > DFTMAX) {
	printf("****ERROR: IDFT size too large **** \n");
	exit(1);
    }
#endif
    
    length2 = (length/2)+1;
    w = TWOPI / length;
    for (i = 0; i < length; i++ ) {
	mf_idftc[i] = cos(w*i);
    }
    real[0] *= (1.0/length);
    for (i = 1; i < length2-1; i++ ) {
	real[i] *= (2.0/length);
    }
    if ((i*2) == length)
	real[i] *= (1.0/length);
    else
	real[i] *= (2.0/length);

    for (i = 0; i < length; i++ ) {
	signal[i] = real[0];
	k_inc = i;
	k = k_inc;
	for (j = 1; j < length2; j++ ) {
	    signal[i] += real[j] * mf_idftc[k];
	    k += k_inc;
	    if (k >= length)
		k -= length;
	}
    }
}
