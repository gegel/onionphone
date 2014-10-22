/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		pgain
*
* FUNCTION
*		Find pitch gain and error
*
* SYNOPSIS
*		function pgain(ex, l, first, m, len, match)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	ex[l]		float	i	excitation vector
*	l		int	i	size of ex
*	first		int	i	first call flag
*	m		int	i	pitch lag
*	len		int	i	length to truncate impulse response
*	match		float	o	negative partial squared error
*	pgain		float	fun	optimal gain for ex
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	e0[]		float	i
*	h[]		float	i
*
***************************************************************************
*
* DESCRIPTION
*
*	For each lag:
*	   a.  Filter first error signal (v0) through truncated
*	       impulse response of perceptual weighting filter
*	       (LPC filter with bandwidth broadening).
*	   b.  Correlate filtered result with actual first error
*	       signal (e0).
*	   c.  Compute first order pitch filter coefficient (pgain)
*	       and error (er) for each lag.
*
*	Note:  Proper selection of the convolution length (len) depends on
*              the perceptual weighting filter's expansion factor (gamma)
*	       which controls the damping of the impulse response.
*
*               This is one of CELP's most computationally intensive
*		routines.  Neglecting overhead, the approximate number of
*		DSP instructions (add, multiply, multiply accumulate, or
*		compare) per second (IPS) is:
*
*
*		C      : convolution (recursive truncated end-point correction)
*               C'     : convolution (recursive truncated end-point correction)
*		R  = E : full correlation & energy
*               R' = E': delta correlation & energy
*		G      : gain quantization
*               G'     : delat gain quantization
*
*		IPS = 2.34 M (for integer delays)
*
*               i.e.,  L = 60, N = 128 pitch lags, N'= 32 delta delays
*                      K = K'= 2 pitch updates/frame, and F=30 ms frame rate:
*
*                      C = 9450, C'= 3690, R = E = 7680, R'= E'= 1920
*
*		       IPS = 2.2 M
*
*	pitch search complexity for integer delays:
*
*#define j  10
*
*	DSP chip instructions/operations:
*	int MUL=1;		!multiply
*	int ADD=1;		!add
*	int SUB=1;		!subtract
*	int MAC=1;		!multiply & accumulate
*	int MAD=2;		!multiply & add
*	int CMP=1;		!compare
*
*	CELP algorithm parameters:
*	int L=60;		!subframe length
*	int len=30;		!length to truncate calculations (<= L)
*	int K=4;		!number of subframes/frame
*	int shift=2;		!shift between code words
*	int g_bits=5;		!cbgain bit allocation
*	float p=0.77;		!code book sparsity
*	float F=30.e-3; 	!time (seconds)/frame
*
*	int N[j] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
*
*	main()
*	{
*	  int i;
*	  float C, R, E, G, IPS;
*         printf("\n    N       C          R          E          G       MIPS\n");
*	  for (i = 0; i < j; i++)
*	  {
*	    C = (335)*MAD + (N[i]-1)*shift*(1.0-p)*len*ADD;
*	    R = N[i]*L*MAC;
*	    E = L*MAC + (N[i]-1)*((1.0-p*p)*L*MAC + (p*p)*2*MAD);
*	    G = N[i]*(g_bits*(CMP+MUL+ADD) + 3*MUL+1*SUB);
*	    IPS = (C+R+E+G)*K/F;
*           printf("  %4d  %f   %f   %f   %f  %f\n", N[i], C*K/1.e6/F,
*		      R*K/1.e6/F,E*K/1.e6/F,G*K/1.e6/F,IPS/1.e6);
*	  }
*	}
*
*     N       C 	 R	    E	       G       MIPS
*     1  0.089333   0.008000   0.008000   0.002533  0.107867
*     2  0.091173   0.016000   0.011573   0.005067  0.123813
*     4  0.094853   0.032000   0.018719   0.010133  0.155706
*     8  0.102213   0.064000   0.033011   0.020267  0.219491
*    16  0.116933   0.128000   0.061595   0.040533  0.347062
*    32  0.146373   0.256000   0.118763   0.081067  0.602203
*    64  0.205253   0.512000   0.233100   0.162133  1.112486
*   128  0.323013   1.024000   0.461773   0.324267  2.133053
*   256  0.558533   2.048000   0.919118   0.648533  4.174185
*   512  1.029573   4.096000   1.833810   1.297067  8.256450
*
*
***************************************************************************
*
* CALLED BY
*
*	psearch
*
* CALLS
*
*
*
***************************************************************************
*
* REFERENCES
*
*	Tremain, Thomas E., Joseph P. Campbell, Jr and Vanoy C. Welch,
*       "A 4.8 kbps Code Excited Linear Predictive Coder," Proceedings
*	of the Mobile Satellite Conference, 3-5 May 1988, pp. 491-496.
*
*	Campbell, Joseph P. Jr., Vanoy C. Welch and Thomas E. Tremain,
*       "An Expandable Error-Protected 4800 bps CELP Coder (U.S. Federal
*       Standard 4800 bps Voice Coder)," Proceedings of ICASSP, 1989.
*	(and Proceedings of Speech Tech, 1989.)
*
**************************************************************************/

static float pgain(const float ex[], int l, int first, int m, int len,
		   float *match)
{
	float cor, eng;
	float y2[MAXLP], pgain;
	static float y[MAXLP];
	int i, j;

	if (first) {

		/* *Calculate and save convolution of truncated (to len)
		 *impulse response for first lag of t (=mmin) samples:

		 min(i, len-1)
		 y     =  SUM  h * ex       , where i = 0, ..., L-1 points
		 i, t    j=0   j    i-j

		 h |0 1...len-1 x x|
		 ex |L-1  . . .  1 0|               = y[0]
		 ex |L-1  . . .  1 0|       = y[1]
		 :            :
		 ex |L-1  . . .  1 0| = y[L-1]
		 */

		for (i = 0; i < l; i++) {
			y[i] = 0.0;
			for (j = 0; j <= i && j < len; j++) {
				y[i] += h[j] * ex[i - j];
			}
		}
	} else {

		/* *End correct the convolution sum on subsequent pitch lags:

		   y     =  0
		   0, t
		   y     =  y + ex  * h   where i = 1, ..., L points
		   i, m       i-1, m-1   -m    i  and   m = t+1, ..., tmax lags
		 */

		for (i = len - 1; i > 0; i--) {
			y[i - 1] += ex[0] * h[i];
		}

		for (i = l - 1; i > 0; i--) {
			y[i] = y[i - 1];
		}

		y[0] = ex[0] * h[0];
	}

	/* *For lags (m) shorter than frame size (l), replicate the short
	 *adaptive codeword to the full codeword length by
	 *overlapping and adding the convolutions:                             */

	for (i = 0; i < l; i++) {
		y2[i] = y[i];
	}

	if (m < l) {

		/* add in 2nd convolution                                              */

		for (i = m; i < l; i++) {
			y2[i] = y[i] + y[i - m];
		}

		if (m < l / 2) {

			/* add in 3rd convolution                                            */

			for (i = 2 * m; i < l; i++) {
				y2[i] = y2[i] + y[i - 2 * m];
			}
		}
	}

	/* *Calculate correlation and energy:
	   e0 = r[n]   = spectrum prediction residual
	   y2 = r[n-m] = error weighting filtered reconstructed
	   pitch prediction signal (m = correlation lag)       */

	cor = 0.0;
	eng = 0.0;
	for (i = 0; i < l; i++) {
		cor += y2[i] * e0[i];
		eng += y2[i] * y2[i];
	}

	/* *Compute gain and error:
	   NOTE: Actual MSPE = e0.e0 - pgain(2*cor-pgain*eng)
	   since e0.e0 is independent of the code word,
	   minimizing MSPE is equivalent to maximizing:
	   match = pgain(2*cor-pgain*eng)   (1)
	   If unquantized pgain is used, this simplifies:
	   match = cor*pgain

	   NOTE: Inferior results were obtained when quantized
	   pgain was used in equation (1)???

	   NOTE: When the delay is less than the frame length, "match"
	   is only an approximation to the actual error.              

	   Independent (open-loop) quantization of gain and match (index):     */

	if (eng <= 0.0) {
		eng = 1.0;
	}
	pgain = cor / eng;
	*match = cor * pgain;

	return pgain;
}
