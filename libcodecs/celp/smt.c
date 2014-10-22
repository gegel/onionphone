/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		smoothtau
*
* FUNCTION
*
*		smooth tau values when two errors detected in Hamming block
*
* SYNOPSIS
*		smoothtau(tau,twoerror,syndavg,tau3,subframe)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	tau		float	i/o	input tau
*	twoerror	int	i	two error flag
*	syndavg 	float	i	error rate estimation parameter
*	tau3		int	i	third tau value
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
*	Smoothing routine to smooth tau (pitch lag) when errors are detected:
*
*	If the variance of past tau values is within the range VARLIMIT
*	(indicating voiced speech) the validity of the current tau value
*	is tested.  If the current value of TAU is within the range TAULIMIT,
*	TAU is passed.	If TAU is not within that range TAULIMIT, TAU is reset 
*	to the average value of taus. 
*
*	The array OLDTAU contains past values of tau.  The array VECTOR
*	is constructed from the array OLDTAU and TAU3 for subframes 1
*	and 2 (TAU3 is a future absolute tau value).  For subframes 3
*	and 4 there are no valid future values (since delta taus in the 
*	future are not valid), therefore the array VECTOR is constructed 
*	entirely from the array OLDTAU.  Decisions concering smoothing of 
*	a particular tau are made on the variance of the array VECTOR and 
*	the tau in question (TAU).
*
*	If the value of tau is smoothed in subframe 3, smoothing is disabled
*	for subframe 4 of the same frame since the tau value in subframe 4
*	is a delta based on subframe 3.
*
*	Note:  The smoothing parameters should be capable of adapting
*	to various bit error rate estimates. For example, different
*	values of SYNDAVG should select different levels of TAULIMIT and
*	VARLIMIT.
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
#define TAUHISTORY	4
#define TAULIMIT	15
#define VARLIMIT	15
#define SYNDLIMIT	0.04

static void smoothtau(float *tau, int twoerror, float syndavg,
		      float tau3, int subframe)
{
	int i;
	static int enable;
	float avg, var, vector[4];
	static float oldtau[TAUHISTORY];

	for (i = 0; i < 4; i++)
		vector[i] = 0;

	if (subframe != 4)
		enable = TRUE;
	if ((twoerror || syndavg > SYNDLIMIT) && enable) {
		if (subframe == 1) {
			vector[0] = oldtau[0];
			vector[1] = oldtau[1];
			vector[2] = oldtau[2];
			vector[3] = tau3;
		} else if (subframe == 2) {
			vector[0] = oldtau[0];
			vector[1] = oldtau[1];
			vector[2] = oldtau[2];
			vector[3] = tau3;
		} else if (subframe == 3) {
			vector[0] = oldtau[0];
			vector[1] = oldtau[1];
			vector[2] = oldtau[2];
			vector[3] = oldtau[3];
		} else if (subframe == 4) {
			vector[0] = oldtau[0];
			vector[1] = oldtau[1];
			vector[2] = oldtau[2];
			vector[3] = oldtau[3];
		} else {
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothtau: Error in subframe number\n");
#endif
		}

		variance(vector, 4, &var, &avg);
		if (var < VARLIMIT
		    && (*tau >= avg + TAULIMIT || *tau <= avg - TAULIMIT)) {
			*tau = round(avg);
#ifdef CELPDIAG
			fprintf(stderr,
				"smoothtau: tau value reset to avg at frame %d subframe %d\n",
				frame, subframe);
#endif
			if (subframe == 3) {
				enable = FALSE;
#ifdef CELPDIAG
				fprintf(stderr,
					"smoothpgain: tau smoothing disabled for subframe 4\n");
#endif
			}
		}
	}

	for (i = TAUHISTORY - 1; i > 0; i--)
		oldtau[i] = oldtau[i - 1];
	oldtau[0] = *tau;
}

#undef TAUHISTORY
#undef TAULIMIT
#undef VARLIMIT
#undef SYNDLIMIT
