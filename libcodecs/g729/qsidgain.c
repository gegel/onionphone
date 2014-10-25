/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : QSIDGAIN.C
*/

/* Quantize SID gain                                      */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ld8k.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"

/* Local function */
static int quant_Energy(float ener,	/* (i)  : Energy                 */
			float * enerq	/* (o)  : quantized energy in dB */
    );

/*-------------------------------------------------------------------*
* Function  Qua_Sidgain                                             *
*           ~~~~~~~~~~~                                             *
*-------------------------------------------------------------------*/
void qua_Sidgain(float * ener,	/* (i)   array of energies                   */
		 int nb_ener,	/* (i)   number of energies or               */
		 float * enerq,	/* (o)   decoded energies in dB              */
		 int *idx	/* (o)   SID gain quantization index         */
    )
{
	int i;
	float avr_ener;

	if (nb_ener == 0) {
		/* Quantize energy saved for frame erasure case                */
		avr_ener = (*ener) * fact[0];
	}

	else {

		/*
		 * Compute weighted average of energies
		 * avr_ener = fact[nb_ener] x SUM(i=0->nb_ener-1) ener[i]
		 * with fact[nb_ener] =  fact_ener / nb_ener x L_FRAME x nbAcf
		 */
		avr_ener = (float) 0.;
		for (i = 0; i < nb_ener; i++) {
			avr_ener += ener[i];
		}
		avr_ener *= fact[nb_ener];
	}

	*idx = quant_Energy(avr_ener, enerq);

	return;
}

/* Local function */
static int quant_Energy(float ener,	/* (i)  : Energy                 */
			float * enerq	/* (o)  : quantized energy in dB */
    )
{
	float ener_dB;
	int index;

	if (ener <= MIN_ENER) {	/* MIN_ENER <=> -8dB */
		*enerq = (float) - 12.;
		return (0);
	}

	ener_dB = (float) 10. *(float) log10(ener);

	if (ener_dB <= (float) - 8.) {
		*enerq = (float) - 12.;
		return (0);
	}

	if (ener_dB >= (float) 65.) {
		*enerq = (float) 66.;
		return (31);
	}

	if (ener_dB <= (float) 14.) {
		index = (int)((ener_dB + (float) 10.) * 0.25);
		if (index < 1)
			index = 1;
		*enerq = (float) 4. *(float) index - (float) 8.;
		return (index);
	}

	index = (int)((ener_dB - (float) 3.) * 0.5);
	if (index < 6)
		index = 6;
	*enerq = (float) 2. *(float) index + (float) 4.;
	return (index);
}
