/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
*
* ROUTINE
*		cgain
*
* FUNCTION
*		Find codeword gain and error (TERNARY CODE BOOK ASSUMED!)
*
* SYNOPSIS
*		function cgain(ex, l, first, len, match)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	ex[l]		float	i	excitation vector (ternary codeword)
*	l		int	i	size of ex (dimension of codeword)
*	first		int	i	first call flag
*	len		int	i	length to truncate impulse response
*	match		float	o	negative partial squared error
*	cgain		float	fun	optimal gain for ex
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	e0[]		float	i
*	h[];		float	i
*
**************************************************************************
*
* DESCRIPTION
*
*	Find the gain and error for each code word:
*	   a.  Filter code words through impulse response
*	       of perceptual weighting filter (LPC filter with
*	       bandwidth broadening).
*	   b.  Correlate filtered result with actual second error
*	       signal (e0).
*	   c.  Compute MSPE gain and error for code book vector.
*
*	Notes:	Code words may contain many zeros (i.e., ex(1)=0).  The
*		code book could be accessed by a pointer to nonzero samples.
*		Because the code book is static, it`s silly to test its
*		samples as in the code below.
*
*		Proper selection of the convolution length (len) depends on
*               the perceptual weighting filter's expansion factor (gamma)
*		which controls the damping of the inpulse response.
*
*               This is one of CELP's most computationally intensive
*		routines.  Neglecting overhead, the approximate number of 
*		DSP instructions (add, multiply, multiply accumulate, or
*		compare) per second (IPS) is:
*
*		       Code book size	MIPS
*		       --------------	----
*			 64		 1.1
*			128		 2.1
*			256		 4.2
*			512		 8.3
*
*		 C:  convolution (recursive truncated end-point correction)
*		 R:  correlation
*		 E:  energy (recursive end-point correction)
*		 G:  gain quantization
*
*
*       celp code book search complexity (doesn't fully exploit ternary values!):
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
**************************************************************************
*
* CALLED BY
*
*	cbsearch
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
*       "The New 4800 bps Voice Coding Standard," Military and 
*	Government Speech Tech, 1989, p. 64-70.
*
*       Lin, Daniel, "New Approaches to Stochastic Coding of Speech
*       Sources at Very Low Bit Rates," Signal Processing III:  Theories
*	and Applications (Proceedings of EUSIPCO-86), 1986, p.445.
*
*       Xydeas, C.S., M.A. Ireton and D.K. Baghbadrani, "Theory and
*	Real Time Implementation of a CELP Coder at 4.8 and 6.0 kbits/s
*       Using Ternary Code Excitation," Fifth International Conference on
*	Digital Processing of Signals in Communications, 1988, p. 167.
*
*       Ess, Mike, "Simple Convolution on the Cray X-MP,"
*	Supercomputer, March 1988, p. 35
*
*	Supercomputer, July 1988, p. 24
*
**************************************************************************/

static float cgain(const float ex[], int l, int first, int len, float *match)
{
	float cor;
	float cgain;
	static float y[MAXL], y59save, y60save, eng;
	int i, j;

	int ex0_0 = round(ex[0]) == 0,
	    ex0_1 = round(ex[0]) == 1,
	    ex1_0 = round(ex[1]) == 0, ex1_1 = round(ex[1]) == 1;

	if (first) {

    /**    For first code word, calculate and save convolution
	   of code word with truncated (to len) impulse response:

	   NOTES: A standard convolution of two L point sequences
		  produces 2L-1 points, however, this convolution
		  generates only the first L points.

                  A "scalar times vector accumulation" method is used
		  to exploit (skip) zero samples of the code words:

		  min(L-i, len-1)
	   y	   =  SUM  ex * h   , where i = 0, ..., L-1 points
	    i+j, t    j=0    i	 j

			ex |0 1 .  .  . L-1|
	   h |x x len-1...1 0|		     = y[0]
	     h |x x len-1...1 0|	     = y[1]
			      : 		:
			 h |x x len-1...1 0| = y[L-1]
	      -----------------------------------------------------	*/

		for (i = 0; i < l; i++) {
			y[i] = 0.0;
		}
		for (i = 0; i < l; i++) {
			if ((ex[i] >= 0.5) || (ex[i] < -0.5))
				for (j = 0; j < l - i && j < len; j++) {
					y[i + j] += ex[i] * h[j];
				}
		}
	} else {

    /**     End correct the convolution sum on subsequent code words:
	    (Do two end corrections for a shift by 2 code book)

	  y	=  0
	    0, 0
	   y	 =  y	     + ex * h	where i = 0, ..., L-1 points
	    i, m     i-1, m-1	-m   i	and   m = 0, ..., cbsize-1 code words

           NOTE:  The data movements in the 2 loops with "y[i] = y[i - 1]"
		  are performed many times and can be quite time consuming.
		  Therefore, special precautions should be taken when
		  implementing this.  Some implementation suggestions:
		  1.  Circular buffers with pointers to eliminate data moves.
                  2.  Fast "block move" operation as offered on some DSPs.
	      -----------------------------------------------------	 */

		/* *First shift */

		if (!ex1_0) {
			/* *ternary stochastic code book (-1, 0, +1)  */
			if (ex1_1) {
				for (i = len - 1; i > 0; i--) {
					y[i - 1] += h[i];
				}
			} else {
				for (i = len - 1; i > 0; i--) {
					y[i - 1] -= h[i];
				}
			}
		}
		for (i = l - 1; i > 0; i--) {
			y[i] = y[i - 1];
		}

		y[0] = ex[1] * h[0];

		/* *Second shift */

		if (!ex0_0) {
			/* *ternary stochastic code book (-1, 0, +1)  */
			if (ex0_1) {
				for (i = len - 1; i > 0; i--) {
					y[i - 1] += h[i];
				}
			} else {
				for (i = len - 1; i > 0; i--) {
					y[i - 1] -= h[i];
				}
			}
		}

		for (i = l - 1; i > 0; i--) {
			y[i] = y[i - 1];
		}
		y[0] = ex[0] * h[0];
	}

  /**	Calculate correlation and energy:
	e0 = spectrum & pitch prediction residual
	y  = error weighting filtered code words

        \/\/\/  CELP's computations are focused in this correlation \/\/\/
		- For a 512 code book this correlation takes 4 MIPS!
		- Decimation?, Down-sample & decimate?, FEC codes?	*/

	cor = 0.0;
	for (i = 0; i < l; i++) {
		cor += y[i] * e0[i];
	}

	/* *End correct energy on subsequent code words: */
	if (ex0_0 && ex1_0 && !first) {
		eng = eng - y59save * y59save - y60save * y60save;
	} else {
		eng = 0.0;
		for (i = 0; i < l; i++) {
			eng += y[i] * y[i];
		}
	}
	y59save = y[l - 2];
	y60save = y[l - 1];

	/* Compute gain and error:
	   NOTE: Actual MSPE = e0.e0 - cgain(2*cor-cgain*eng)
	   since e0.e0 is independent of the code word,
	   minimizing MSPE is equivalent to maximizing:
	   *match = cgain(2*cor-cgain*eng)
	   If unquantized cgain is used, this simplifies:
	   *match = cor*cgain                                  */

	/*    Independent (open-loop) quantization of gain and match (index):  */

	if (eng <= 0.0) {
		eng = 1.0;
	}
	cgain = cor / eng;
	*match = cor * cgain;

	return cgain;
}
