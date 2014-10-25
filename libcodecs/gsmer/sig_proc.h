/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*--------------------------------------------------------------*
* Function prototypes for general SIGnal PROCessing functions. *
*--------------------------------------------------------------*/

/* Mathematic functions  */

int32_t w_Inv_sqrt(int32_t L_x	/* (i) : input value    (range: 0<=val<=7fffffff)    */
    );
void w_Log2(int32_t L_x,		/* (i) : input value                                 */
	    int16_t * exponent,	/* (o) : Integer part of w_Log2.   (range: 0<=val<=30) */
	    int16_t * fraction	/* (o) : Fractional part of w_Log2. (range: 0<=val<1) */
    );
int32_t w_Pow2(int16_t exponent,	/* (i) : Integer part.      (range: 0<=val<=30)      */
		     int16_t fraction	/* (i) : Fractional part.  (range: 0.0<=val<1.0)     */
    );

/* General signal processing */

void w_Init_w_Pre_Process(void);
void w_Pre_Process(int16_t signal[],	/* Input/output signal                               */
		   int16_t lg	/* Lenght of signal                                  */
    );

int16_t w_Autocorr(int16_t x[],	/* (i)    : Input signal                             */
		  int16_t m,	/* (i)    : LPC order                                */
		  int16_t r_h[],	/* (o)    : w_Autocorrelations  (msb)                  */
		  int16_t r_l[],	/* (o)    : w_Autocorrelations  (lsb)                  */
		  const int16_t wind[]	/* (i)    : window for LPC analysis.                 */
    );
void w_Lag_window(int16_t m,	/* (i)    : LPC order                                */
		  int16_t r_h[],	/* (i/o)  : w_Autocorrelations  (msb)                  */
		  int16_t r_l[]	/* (i/o)  : w_Autocorrelations  (lsb)                  */
    );
void w_er_Levinson(int16_t Rh[],	/* (i)    : Rh[m+1] Vector of autocorrelations (msb) */
		   int16_t Rl[],	/* (i)    : Rl[m+1] Vector of autocorrelations (lsb) */
		   int16_t A[],	/* (o)    : A[m]    LPC coefficients  (m = 10)       */
		   int16_t rc[]	/* (o)    : rc[4]   First 4 reflection coefficients  */
    );
void w_Az_lsp(int16_t a[],	/* (i)    : w_predictor coefficients                   */
	      int16_t lsp[],	/* (o)    : line spectral pairs                      */
	      int16_t old_lsp[]	/* (i)    : old lsp[] (in case not found 10 roots)   */
    );
void w_Lsp_Az(int16_t lsp[],	/* (i)    : line spectral frequencies                */
	      int16_t a[]	/* (o)    : w_predictor coefficients (order = 10)      */
    );
void w_Lsf_lsp(int16_t lsf[],	/* (i)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
	       int16_t lsp[],	/* (o)    : lsp[m] (range: -1<=val<1)                */
	       int16_t m		/* (i)    : LPC order                                */
    );
void w_Lsp_lsf(int16_t lsp[],	/* (i)    : lsp[m] (range: -1<=val<1)                */
	       int16_t lsf[],	/* (o)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
	       int16_t m		/* (i)    : LPC order                                */
    );
void w_Reorder_lsf(int16_t * lsf,	/* (i/o)  : vector of LSFs   (range: 0<=val<=0.5)    */
		   int16_t min_dist,	/* (i)    : minimum required distance                */
		   int16_t n	/* (i)    : LPC order                                */
    );
void Weight_Fac(int16_t gamma,	/* (i)    : Spectral expansion.                      */
		int16_t fac[]	/* (i/o)  : Computed factors.                        */
    );
void w_Weight_Ai(int16_t a[],	/* (i)  : a[m+1]  LPC coefficients   (m=10)          */
		 const int16_t fac[],	/* (i)  : Spectral expansion factors.                */
		 int16_t a_exp[]	/* (o)  : Spectral expanded LPC coefficients         */
    );
void w_Residu(int16_t a[],	/* (i)  : w_prediction coefficients                    */
	      int16_t x[],	/* (i)  : w_speech signal                              */
	      int16_t y[],	/* (o)  : residual signal                            */
	      int16_t lg		/* (i)  : size of filtering                          */
    );
void w_Syn_filt(int16_t a[],	/* (i)  : a[m+1] w_prediction coefficients   (m=10)    */
		int16_t x[],	/* (i)  : input signal                               */
		int16_t y[],	/* (o)  : output signal                              */
		int16_t lg,	/* (i)  : size of filtering                          */
		int16_t mem[],	/* (i/o): memory associated with this filtering.     */
		int16_t update	/* (i)  : 0=no update, 1=update of memory.           */
    );
void w_Convolve(int16_t x[],	/* (i)  : input vector                               */
		int16_t h[],	/* (i)  : impulse response                           */
		int16_t y[],	/* (o)  : output vector                              */
		int16_t L	/* (i)  : vector size                                */
    );
void w_agc(int16_t * sig_in,	/* (i)  : postfilter input signal                    */
	   int16_t * sig_out,	/* (i/o): postfilter output signal                   */
	   int16_t w_agc_fac,	/* (i)  : AGC factor                                 */
	   int16_t l_trm		/* (i)  : w_subframe size                              */
    );
void w_w_agc2(int16_t * sig_in,	/* (i)  : postfilter input signal                    */
	      int16_t * sig_out,	/* (i/o): postfilter output signal                   */
	      int16_t l_trm	/* (i)  : w_subframe size                              */
    );
void w_preemphasis(int16_t * signal,	/* (i/o): input signal overwritten by the output     */
		   int16_t g,	/* (i)  : w_preemphasis coefficient                    */
		   int16_t L	/* (i)  : size of filtering                          */
    );

/* General */

void w_Copy(int16_t x[],		/* (i)  : input vector                               */
	    int16_t y[],		/* (o)  : output vector                              */
	    int16_t L		/* (i)  : vector length                              */
    );
void w_Set_w_zero(int16_t x[],	/* (o)  : vector to clear                            */
		  int16_t L	/* (i)  : length of vector                           */
    );
