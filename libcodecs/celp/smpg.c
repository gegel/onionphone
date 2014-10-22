/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		smoothpgain
*
* FUNCTION
*
*		smooth pgain values when two errors detected in
*		Hamming block
*
* SYNOPSIS
*		smoothpgain(pgain,twoerror,syndavg,pgains,subframe)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	pgain		float	i/o	input pgain
*	twoerror	int	i	two error flag
*	syndavg 	float	i	error rate estimation parameter
*	pgains		float	i	vector of pgains to calculate variance
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
*	Smoothing routine to smooth pgain (alpha) when errors are detected:
*
*	Due to the range of PGAIN, statistical variance is not appropriate.
*	Pseudovariance is used and calculated as: 
*		sum of delta oldpgains/# of deltas
*
*	If this variance of past pgain values is within the range VARLIMIT, 
*	the validity of the current pgain value is tested.  If the current 
*	value of pgain is within the range PGAINLIMIT, PGAIN is passed.  
*	If PGAIN is not within that range it is reset to the average
*	value of surrounding pgain values.
*
*	The array OLDPGAIN contains past values of pgain.  The array
*	PGAINS contains current and future values of pgain.  The array 
*	VECTOR is constructed from the arrays OLDPGAIN and PGAINS 
*	depending on the current subframe.  PGAIN is smoothed based on 
*	the statistics of VECTOR, which contains the nearest four 
*	surrounding pgain values, both past and future values, except 
*	where future values are not available (subframes 3 and 4).
*
*	Absolute values of pgain are used in averaging and reassigning
*	pgain.	All reassigned pgains are limited to the range 0.0-1.0.
*
*	Note:  The smoothing parameters should be capable of adapting
*	to various bit error rate estimates. For example, different
*	values of SYNDAVG should select different levels of PGAINLIMIT,
*	VARLIMIT, and SYNDLIMIT.
*
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*
*
**************************************************************************/
#define PGAINHISTORY	4
#define PGAINLIMIT	0.9
#define VARLIMIT	0.2
#define SYNDLIMIT	0.04

static void smoothpgain(float *pgain, int twoerror, float syndavg,
			float pgains[], int subframe)
{
	int i;
	static int enable;
	float abspgain, avg, var, sum1, sum2, vector[4];
	static float oldpgain[PGAINHISTORY];

	for (i = 0; i < 4; i++)
		vector[i] = 0;

	abspgain = fabs(*pgain);
	if (subframe != 4)
		enable = TRUE;
	if ((twoerror || syndavg > SYNDLIMIT) && enable) {
		if (subframe == 1) {
			vector[0] = oldpgain[0];
			vector[1] = oldpgain[1];
			vector[2] = fabs(pgains[1]);
			vector[3] = fabs(pgains[2]);
		} else if (subframe == 2) {
			vector[0] = oldpgain[0];
			vector[1] = oldpgain[1];
			vector[2] = fabs(pgains[2]);
			vector[3] = fabs(pgains[3]);
		} else if (subframe == 3) {
			vector[0] = oldpgain[0];
			vector[1] = oldpgain[1];
			vector[2] = oldpgain[2];
			vector[3] = fabs(pgains[3]);
		} else if (subframe == 4) {
			vector[0] = oldpgain[0];
			vector[1] = oldpgain[1];
			vector[2] = oldpgain[2];
			vector[3] = oldpgain[3];
		} else {
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothpgain: Error in subframe number\n");
#endif
		}

		for (sum1 = 0.0, i = 0; i < PGAINHISTORY; i++) {
			sum1 += vector[i];
		}
		avg = sum1 / PGAINHISTORY;
		for (sum2 = 0.0, i = 0; i < PGAINHISTORY - 1; i++)
			sum2 += fabs(vector[i] - vector[i + 1]);
		var = sum2 / (PGAINHISTORY - 1);
		if (var < VARLIMIT && enable &&
		    (abspgain > avg + PGAINLIMIT
		     || abspgain < avg - PGAINLIMIT)) {
			*pgain = avg;
			if (*pgain > 1.0)
				*pgain = 1.0;
			if (*pgain < -1.0)
				*pgain = -1.0;
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothpgain:  pgain value reset to avg at frame %d subframe %d\n",
				frame, subframe);
#endif
			if (subframe == 3) {
				enable = FALSE;
#ifdef CELPDIAG
				fprintf(stderr,
					"smoothpgain:  smoothing disabled for subframe 4\n");
#endif
			}
		}
	}

	for (i = PGAINHISTORY - 1; i > 0; i--)
		oldpgain[i] = oldpgain[i - 1];
	oldpgain[0] = abspgain;
}

#undef PGAINHISTORY
#undef PGAINLIMIT
#undef VARLIMIT
#undef SYNDLIMIT
