/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*---------------------------------------------------------------------------*\
                                                                           
  FILE........: lpc.c                                                              
  AUTHOR......: David Rowe                                                      
  DATE CREATED: 30 Sep 1990 (!)                                                 
                                                                          
  Linear Prediction functions written in C.                                
                                                                          
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009-2012 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#define LPC_MAX_N 512		/* maximum no. of samples in frame */
#define PI 3.141592654		/* mathematical constant */

#define ALPHA 1.0
#define BETA  0.94

#include <assert.h>
#include <math.h>
#include <ophtools.h>
#include "defines.h"
#include "lpc.h"

/*---------------------------------------------------------------------------*\
                                                                           
  autocorrelate()                                                          
                                                                          
  Finds the first P autocorrelation values of an array of windowed speech 
  samples Sn[].                                                            
                                                                          
\*---------------------------------------------------------------------------*/

void autocorrelate(float Sn[],	/* frame of Nsam windowed speech samples */
		   float Rn[],	/* array of P+1 autocorrelation coefficients */
		   int Nsam,	/* number of windowed samples to use */
		   int order	/* order of LPC analysis */
    )
{
	int i, j;		/* loop variables */

	for (j = 0; j < order + 1; j++) {
		Rn[j] = 0.0;
		for (i = 0; i < Nsam - j; i++)
			Rn[j] += Sn[i] * Sn[i + j];
	}
}

/*---------------------------------------------------------------------------*\
                                                                            
  levinson_durbin()                                                        
                                                                           
  Given P+1 autocorrelation coefficients, finds P Linear Prediction Coeff. 
  (LPCs) where P is the order of the LPC all-pole model. The Levinson-Durbin
  algorithm is used, and is described in:                                   
                                                                           
    J. Makhoul                                                               
    "Linear prediction, a tutorial review"                                   
    Proceedings of the IEEE                                                
    Vol-63, No. 4, April 1975                                               
                                                                             
\*---------------------------------------------------------------------------*/

void levinson_durbin(float R[],	/* order+1 autocorrelation coeff */
		     float lpcs[],	/* order+1 LPC's */
		     int order	/* order of the LPC analysis */
    )
{
	float a[order + 1][order + 1];
	float sum, e, k;
	int i, j;		/* loop variables */

	e = R[0];		/* Equation 38a, Makhoul */

	for (i = 1; i <= order; i++) {
		sum = 0.0;
		for (j = 1; j <= i - 1; j++)
			sum += a[i - 1][j] * R[i - j];
		k = -1.0 * (R[i] + sum) / e;	/* Equation 38b, Makhoul */
		if (fabsf(k) > 1.0)
			k = 0.0;

		a[i][i] = k;

		for (j = 1; j <= i - 1; j++)
			a[i][j] = a[i - 1][j] + k * a[i - 1][i - j];	/* Equation 38c, Makhoul */

		e *= (1 - k * k);	/* Equation 38d, Makhoul */
	}

	for (i = 1; i <= order; i++)
		lpcs[i] = a[order][i];
	lpcs[0] = 1.0;
}

/*---------------------------------------------------------------------------*\
                                                                            
  weight()                                                                  
                                                                          
  Weights a vector of LPCs.						   
                                                                          
\*---------------------------------------------------------------------------*/

void weight(float ak[],		/* vector of order+1 LPCs */
	    float gamma,	/* weighting factor */
	    int order,		/* num LPCs (excluding leading 1.0) */
	    float akw[]		/* weighted vector of order+1 LPCs */
    )
{
	int i;

	for (i = 1; i <= order; i++)
		akw[i] = ak[i] * powf(gamma, (float)i);
}
