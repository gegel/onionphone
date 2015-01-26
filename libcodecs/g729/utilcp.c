/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : UTILCP.C
*/

/*****************************************************************************/
/* auxiliary functions                                                       */
/*****************************************************************************/
#include "ld8k.h"

/*-------------------------------------------------------------------*
* Function  set zero()                                              *
*           ~~~~~~~~~~                                              *
* Set vector x[] to zero                                            *
*-------------------------------------------------------------------*/
void set_zero(float x[],	/* (o)    : vector to clear     */
	      int L		/* (i)    : length of vector    */
    )
{
	int i;

	for (i = 0; i < L; i++)
		x[i] = (float) 0.0;

	return;
}

/*-------------------------------------------------------------------*
* Function  copy:                                                   *
*           ~~~~~                                                   *
* Copy vector x[] to y[]                                            *
*-------------------------------------------------------------------*/
void copy(float x[],		/* (i)   : input vector   */
	  float y[],		/* (o)   : output vector  */
	  int L			/* (i)   : vector length  */
    )
{
	int i;

	for (i = 0; i < L; i++)
		y[i] = x[i];

	return;
}

/* Random generator  */
int16_t random_g729c(int16_t * seed)
{

	*seed = (int16_t) ((*seed) * 31821L + 13849L);

	return (*seed);

}

/*****************************************************************************/
/* Functions used by VAD.C                                                   */
/*****************************************************************************/
void dvsub(float * in1, float * in2, float * out, int16_t npts)
{
	while (npts--)
		*(out++) = *(in1++) - *(in2++);
}

float dvdot(float * in1, float * in2, int16_t npts)
{
	float accum;

	accum = (float) 0.0;
	while (npts--)
		accum += *(in1++) * *(in2++);
	return (accum);
}

void dvwadd(float * in1, float scalar1, float * in2, float scalar2,
	    float * out, int16_t npts)
{
	while (npts--)
		*(out++) = *(in1++) * scalar1 + *(in2++) * scalar2;
}

void dvsmul(float * in, float scalar, float * out, int16_t npts)
{
	while (npts--)
		*(out++) = *(in++) * scalar;
}
