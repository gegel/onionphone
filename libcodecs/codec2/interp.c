/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*---------------------------------------------------------------------------*\

  FILE........: interp.c
  AUTHOR......: David Rowe
  DATE CREATED: 9/10/09

  Interpolation of 20ms frames to 10ms frames.

\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

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

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "defines.h"
#include "interp.h"
#include "lsp.h"
#include "quantise.h"

/*---------------------------------------------------------------------------*\

  FUNCTION....: interp_Wo()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22 May 2012
        
  Interpolates centre 10ms sample of Wo and L samples given two
  samples 20ms apart. Assumes voicing is available for centre
  (interpolated) frame.
  
\*---------------------------------------------------------------------------*/

void interp_Wo(MODEL * interp,	/* interpolated model params                     */
	       MODEL * prev,	/* previous frames model params                  */
	       MODEL * next	/* next frames model params                      */
    )
{
	interp_Wo2(interp, prev, next, 0.5);
}

/*---------------------------------------------------------------------------*\

  FUNCTION....: interp_Wo2()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22 May 2012
        
  Weighted interpolation of two Wo samples.
  
\*---------------------------------------------------------------------------*/

void interp_Wo2(MODEL * interp,	/* interpolated model params                     */
		MODEL * prev,	/* previous frames model params                  */
		MODEL * next,	/* next frames model params                      */
		float weight)
{
	/* trap corner case where voicing est is probably wrong */

	if (interp->voiced && !prev->voiced && !next->voiced) {
		interp->voiced = 0;
	}

	/* Wo depends on voicing of this and adjacent frames */

	if (interp->voiced) {
		if (prev->voiced && next->voiced)
			interp->Wo =
			    (1.0 - weight) * prev->Wo + weight * next->Wo;
		if (!prev->voiced && next->voiced)
			interp->Wo = next->Wo;
		if (prev->voiced && !next->voiced)
			interp->Wo = prev->Wo;
	} else {
		interp->Wo = TWO_PI / P_MAX;
	}
	interp->L = PI / interp->Wo;
}

/*---------------------------------------------------------------------------*\

  FUNCTION....: interp_energy()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22 May 2012
        
  Interpolates centre 10ms sample of energy given two samples 20ms
  apart.
  
\*---------------------------------------------------------------------------*/

float interp_energy(float prev_e, float next_e)
{
	return powf(10.0, (log10f(prev_e) + log10f(next_e)) / 2.0);

}

/*---------------------------------------------------------------------------*\

  FUNCTION....: interp_energy2()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22 May 2012
        
  Interpolates centre 10ms sample of energy given two samples 20ms
  apart.
  
\*---------------------------------------------------------------------------*/

float interp_energy2(float prev_e, float next_e, float weight)
{
	return powf(10.0,
		    (1.0 - weight) * log10f(prev_e) + weight * log10f(next_e));

}

/*---------------------------------------------------------------------------*\

  FUNCTION....: interpolate_lsp_ver2()	     
  AUTHOR......: David Rowe			      
  DATE CREATED: 22 May 2012
        
  Weighted interpolation of LSPs.
  
\*---------------------------------------------------------------------------*/

void interpolate_lsp_ver2(float interp[], float prev[], float next[],
			  float weight)
{
	int i;

	for (i = 0; i < LPC_ORD; i++)
		interp[i] = (1.0 - weight) * prev[i] + weight * next[i];
}
