/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		smoothcbgain
*
* FUNCTION
*
*		smooth cbgain values when two errors detected in
*		Hamming block
*
* SYNOPSIS
*		smoothcbgain(cbgain,twoerror,syndavg, gains, subframe)
*
*   formal
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	cbgain		float	i/o	input cbgain
*	twoerror	int	i	two error flag
*	syndavg 	float	i	error rate estimation parameter
*	gains		float	i	vector of gains to calculate variance
*	subframe	int	i	subframe number
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	frame		int	i
*
***************************************************************************
*
* DESCRIPTION
*
*	Smoothing routine to smooth cbgain when errors are detected:
*
*	If the variance of past cbgain values is within the range VARLIMIT,
*	the validity of the current cbgain value is tested.  If the current
*	value of cbgain is within the range CBGAINLIMIT, cbgain is passed.
*	If CBGAIN is not within  the range CBGAINLIMIT it is reset to the 
*	average value of the surrounding cbgain values.
*
*	The array OLDCBGAIN contains past values of cbgain.  The array
*	GAINS contains current and future values of cbgain.  The array 
*	VECTOR is constructed from the arrays OLDCBGAIN and GAINS
*	depending on the current subframe.  CBGAIN is smoothed based on 
*	the statistics of VECTOR, which contains the nearest four
*	surrounding cbgain values, both past and future values, except
*	where future values are not available (subframes 3 and 4).
*
*	NOTE:  The smoothing parameters should be capable of adapting
*	to various bit error rate estimates.  For example, different
*	values of SYNDAVG should select different levels of CBGAINLIMIT,
*	VARLIMIT, and SYNDAVG.
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*	variance
*
**************************************************************************/
#define CBGAINHISTORY	4
#define CBGAINLIMIT	300.0
#define VARLIMIT	30000.0
#define SGAINLIMIT	9.0
#define SVARLIMIT	10.0
#define AVGLIMIT	6.0
#define SYNDLIMIT	0.04

static void smoothcbgain(float *cbgain, int twoerror, float syndavg,
			 float gains[], int subframe)
{
	int i, sign;
	static int enable;
	float avg, var, abscbgain, vector[4];
	static float oldcbgain[CBGAINHISTORY];

	for (i = 0; i < 4; i++)
		vector[i] = 0;

	abscbgain = fabs(*cbgain);
	if (subframe != 4)
		enable = TRUE;
	if ((twoerror || syndavg > SYNDLIMIT) && enable) {
		if (subframe == 1) {
			vector[0] = oldcbgain[0];
			vector[1] = oldcbgain[1];
			vector[2] = fabs(gains[1]);
			vector[3] = fabs(gains[2]);
		} else if (subframe == 2) {
			vector[0] = oldcbgain[0];
			vector[1] = oldcbgain[1];
			vector[2] = fabs(gains[2]);
			vector[3] = fabs(gains[3]);
		} else if (subframe == 3) {
			vector[0] = oldcbgain[0];
			vector[1] = oldcbgain[1];
			vector[2] = oldcbgain[2];
			vector[3] = fabs(gains[3]);
		} else if (subframe == 4) {
			vector[0] = oldcbgain[0];
			vector[1] = oldcbgain[1];
			vector[2] = oldcbgain[2];
			vector[3] = oldcbgain[3];
		} else {
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothcbgain: Error in subframe number\n");
#endif
		}

		variance(vector, 4, &var, &avg);
		sign = round(*cbgain / fabs(*cbgain));
		if (var < VARLIMIT &&
		    (abscbgain > avg + CBGAINLIMIT
		     || abscbgain < avg - CBGAINLIMIT)) {
			abscbgain = avg;
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothcbgain:  cbgain value reset to avg cbgains at frame %d subframe %d\n",
				frame, subframe);
#endif
			*cbgain = sign * abscbgain;
			if (subframe == 3) {
				enable = FALSE;
#ifdef CELPDIAG
				fprintf(stderr,
					"smoothcbgain:  smoothing disabled for subframe 4\n");
#endif
			}
		}

		if (var < SVARLIMIT && abscbgain > SGAINLIMIT &&
		    avg < AVGLIMIT && enable) {
			abscbgain = avg;
			*cbgain = sign * abscbgain;
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothcbgain:  %s at frame %d subframe %d\n",
				"cbgain value reset to avg cbgains (silence?)",
				frame, subframe);
#endif
			if (subframe == 3) {
				enable = FALSE;
#ifdef CELPDIAG
				fprintf(stderr,
					"smoothcbgain:  smoothing disabled for subframe 4\n");
#endif
			}
		}
	}
	for (i = CBGAINHISTORY - 1; i > 0; i--)
		oldcbgain[i] = oldcbgain[i - 1];
	oldcbgain[0] = abscbgain;
}

#undef CBGAINHISTORY
#undef CBGAINLIMIT
#undef VARLIMIT
#undef SGAINLIMIT
#undef SVARLIMIT
#undef AVGLIMIT
#undef SYNDLIMIT
