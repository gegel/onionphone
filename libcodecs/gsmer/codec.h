/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

void Init_w_Coder_12k2(void);

void w_Coder_12k2(int16_t ana[],	/* output  : Analysis parameters */
		  int16_t w_w_synth[]	/* output  : Local w_w_synthesis     */
    );

void Init_w_Decoder_12k2(void);

void w_Decoder_12k2(int16_t w_parm[],	/* input : vector of w_w_synthesis parameters
					   w_parm[0] = bad frame indicator (bfi) */
		    int16_t w_w_synth[],	/* output: w_w_synthesis w_speech                    */
		    int16_t A_t[],	/* output: decoded LP filter in 4 w_subframes    */
		    int16_t TAF, int16_t SID_flag);

void w_Init_w_Post_Filter(void);

void w_Post_Filter(int16_t * w_syn,	/* in/out: w_w_synthesis w_speech (postfiltered is output) */
		   int16_t * Az_4	/* input : interpolated LPC parameters in all w_subfr. */
    );

void w_code_10i40_35bits(int16_t x[],	/* (i)   : target vector                             */
			 int16_t cn[],	/* (i)   : residual after long term w_prediction       */
			 int16_t h[],	/* (i)   : impulse response of weighted w_w_synthesis
					   filter                                    */
			 int16_t cod[],	/* (o)   : algebraic (fixed) codebook w_excitation     */
			 int16_t y[],	/* (o)   : filtered fixed codebook w_excitation        */
			 int16_t indx[]	/* (o)   : index of 10 pulses (sign + position)      */
    );
void w_dec_10i40_35bits(int16_t index[],	/* (i)   : index of 10 pulses (sign+position)        */
			int16_t cod[]	/* (o)   : algebraic (fixed) codebook w_excitation     */
    );
int16_t w_Dec_lag6(int16_t index,	/* input : received pitch index                      */
			 int16_t pit_min,	/* input : minimum pitch lag                         */
			 int16_t pit_max,	/* input : maximum pitch lag                         */
			 int16_t i_w_subfr,	/* input : w_subframe flag                             */
			 int16_t L_frame_by2,	/* input : w_speech frame size divided by 2            */
			 int16_t * T0_frac,	/* output: fractional part of pitch lag              */
			 int16_t bfi	/* input : bad frame indicator                       */
    );
int16_t w_d_gain_pitch(int16_t index,	/* in       : index of quantization                  */
			     int16_t bfi,	/* in       : bad frame indicator (good = 0)         */
			     int16_t w_state,	/* in       : w_state of the w_state machine             */
			     int16_t w_prev_bf,	/* Previous bf                                       */
			     int16_t w_rxdtx_ctrl);
void w_d_gain_code(int16_t index,	/* input : received quantization index               */
		   int16_t code[],	/* input : innovation codevector                     */
		   int16_t lcode,	/* input : codevector length                         */
		   int16_t * gain_code,	/* output: decoded innovation gain                   */
		   int16_t bfi,	/* input : bad frame indicator                       */
		   int16_t w_state,	/* in    : w_state of the w_state machine                */
		   int16_t w_prev_bf,	/* Previous bf                                       */
		   int16_t w_rxdtx_ctrl,
		   int16_t i_w_subfr, int16_t w_w_rx_dtx_w_state);
void w_D_plsf_5(int16_t * indice,	/* input : quantization indices of 5 w_submatrices     */
		int16_t * lsp1_q,	/* output: quantized 1st LSP vector                  */
		int16_t * lsp2_q,	/* output: quantized 2nd LSP vector                  */
		int16_t bfi,	/* input : bad frame indicator (set to 1 if a bad
				   frame is received)                        */
		int16_t w_rxdtx_ctrl, int16_t w_w_rx_dtx_w_state);
int16_t w_Enc_lag6(int16_t T0,	/* input : Pitch delay                               */
			 int16_t * T0_frac,	/* in/out: Fractional pitch delay                    */
			 int16_t * T0_min,	/* in/out: Minimum search delay                      */
			 int16_t * T0_max,	/* in/out: Maximum search delay                      */
			 int16_t pit_min,	/* input : Minimum pitch delay                       */
			 int16_t pit_max,	/* input : Maximum pitch delay                       */
			 int16_t pit_flag	/* input : Flag for 1st or 3rd w_subframe              */
    );

int16_t w_q_gain_pitch(int16_t * gain	/* (i)    :  Pitch gain to quantize                  */
    );

int16_t w_q_gain_code(int16_t code[],	/* (i)      : fixed codebook w_excitation              */
			    int16_t lcode,	/* (i)      : codevector size                        */
			    int16_t * gain,	/* (i/o)    : quantized fixed codebook gain          */
			    int16_t w_txdtx_ctrl, int16_t i_w_subfr);

int16_t w_G_pitch(int16_t xn[],	/* (i)     : Pitch target.                           */
			int16_t y1[],	/* (i)     : Filtered adaptive codebook.             */
			int16_t w_L_w_subfr	/*         : Length of w_subframe.                     */
    );
int16_t w_G_code(int16_t xn[],	/* in       : target vector                          */
		       int16_t y2[]	/* in       : filtered inovation vector              */
    );

int16_t w_Interpol_6(int16_t * x,	/* (i)  : input vector                               */
			   int16_t frac	/* (i)  : fraction                                   */
    );
void w_Int_lpc(int16_t w_lsp_old[],	/* input: LSP vector at the 4th w_subfr. of past frame */
	       int16_t lsp_mid[],	/* input: LSP vector at the 2nd w_subfr. of
					   present frame                                     */
	       int16_t lsp_new[],	/* input: LSP vector at the 4th w_subfr. of
					   present frame                                     */
	       int16_t Az[]	/* output: interpolated LP parameters in all w_subfr.  */
    );
void w_w_Int_lpc2(int16_t w_lsp_old[],	/* input: LSP vector at the 4th w_subfr. of past frame */
		  int16_t lsp_mid[],	/* input: LSP vector at the 2nd w_subframe of
					   present frame                                     */
		  int16_t lsp_new[],	/* input: LSP vector at the 4th w_subframe of
					   present frame                                     */
		  int16_t Az[]	/* output:interpolated LP parameters
				   in w_subframes 1 and 3                              */
    );
int16_t w_Pitch_fr6(int16_t w_exc[],	/* (i)     : w_excitation buffer                       */
			  int16_t xn[],	/* (i)     : target vector                           */
			  int16_t h[],	/* (i)     : impulse response of w_w_synthesis and
					   weighting filters                       */
			  int16_t w_L_w_subfr,	/* (i)     : Length of w_subframe                      */
			  int16_t t0_min,	/* (i)     : minimum value in the searched range.    */
			  int16_t t0_max,	/* (i)     : maximum value in the searched range.    */
			  int16_t i_w_subfr,	/* (i)     : indicator for first w_subframe.           */
			  int16_t * pit_frac	/* (o)     : chosen fraction.                        */
    );
int16_t w_Pitch_ol(int16_t signal[],	/* input: signal used to compute the open loop pitch */
			 /* signal[-pit_max] to signal[-1] should be known    */
			 int16_t pit_min,	/* input : minimum pitch lag                         */
			 int16_t pit_max,	/* input : maximum pitch lag                         */
			 int16_t L_frame	/* input : length of frame to compute pitch          */
    );
void w_Pred_lt_6(int16_t w_exc[],	/* in/out: w_excitation buffer                         */
		 int16_t T0,	/* input : integer pitch lag                         */
		 int16_t frac,	/* input : fraction of lag                           */
		 int16_t w_L_w_subfr	/* input : w_subframe size                             */
    );
void w_Q_plsf_5(int16_t * lsp1,	/* input : 1st LSP vector                            */
		int16_t * lsp2,	/* input : 2nd LSP vector                            */
		int16_t * lsp1_q,	/* output: quantized 1st LSP vector                  */
		int16_t * lsp2_q,	/* output: quantized 2nd LSP vector                  */
		int16_t * indice,	/* output: quantization indices of 5 matrices        */
		int16_t w_txdtx_ctrl	/* input : tx dtx control word                       */
    );
void w_Bits2w_prm_12k2(int16_t bits[],	/* input : w_serial bits                               */
		       int16_t w_prm[]	/* output: analysis parameters                       */
    );
void w_Prm2bits_12k2(int16_t w_prm[],	/* input : analysis parameters                       */
		     int16_t bits[]	/* output: w_serial bits                               */
    );
