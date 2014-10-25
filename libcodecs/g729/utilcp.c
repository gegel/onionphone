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

/*-----------------------------------------------------------*
* fwrite16 - writes a float array as a Short to a a file    *
*-----------------------------------------------------------*/
void fwrite16(float * data,	/* input: inputdata */
	      int length,	/* input: length of data array */
	      FILE * fp		/* input: file pointer */
    )
{
	int i;
	int16_t sp16[L_FRAME];
	float temp;

	if (length > L_FRAME) {
		printf("error in fwrite16\n");
		exit(16);
	}

	for (i = 0; i < length; i++) {
		/* round and convert to int  */
		temp = data[i];
		if (temp >= (float) 0.0)
			temp += (float) 0.5;
		else
			temp -= (float) 0.5;
		if (temp > (float) 32767.0)
			temp = (float) 32767.0;
		if (temp < (float) - 32768.0)
			temp = (float) - 32768.0;
		sp16[i] = (int16_t) temp;
	}
	fwrite(sp16, sizeof(int16_t), length, fp);
	return;
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
