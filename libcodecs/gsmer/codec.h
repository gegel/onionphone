/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

void Init_w_Coder_12k2(void);

void w_Coder_12k2(Word16 ana[],	/* output  : Analysis parameters */
		  Word16 w_w_synth[]	/* output  : Local w_w_synthesis     */
    );

void Init_w_Decoder_12k2(void);

void w_Decoder_12k2(Word16 w_parm[],	/* input : vector of w_w_synthesis parameters
					   w_parm[0] = bad frame indicator (bfi) */
		    Word16 w_w_synth[],	/* output: w_w_synthesis w_speech                    */
		    Word16 A_t[],	/* output: decoded LP filter in 4 w_subframes    */
		    Word16 TAF, Word16 SID_flag);

void w_Init_w_Post_Filter(void);

void w_Post_Filter(Word16 * w_syn,	/* in/out: w_w_synthesis w_speech (postfiltered is output) */
		   Word16 * Az_4	/* input : interpolated LPC parameters in all w_subfr. */
    );

void w_code_10i40_35bits(Word16 x[],	/* (i)   : target vector                             */
			 Word16 cn[],	/* (i)   : residual after long term w_prediction       */
			 Word16 h[],	/* (i)   : impulse response of weighted w_w_synthesis
					   filter                                    */
			 Word16 cod[],	/* (o)   : algebraic (fixed) codebook w_excitation     */
			 Word16 y[],	/* (o)   : filtered fixed codebook w_excitation        */
			 Word16 indx[]	/* (o)   : index of 10 pulses (sign + position)      */
    );
void w_dec_10i40_35bits(Word16 index[],	/* (i)   : index of 10 pulses (sign+position)        */
			Word16 cod[]	/* (o)   : algebraic (fixed) codebook w_excitation     */
    );
Word16 w_Dec_lag6(		/* output: return integer pitch lag                  */
			 Word16 index,	/* input : received pitch index                      */
			 Word16 pit_min,	/* input : minimum pitch lag                         */
			 Word16 pit_max,	/* input : maximum pitch lag                         */
			 Word16 i_w_subfr,	/* input : w_subframe flag                             */
			 Word16 L_frame_by2,	/* input : w_speech frame size divided by 2            */
			 Word16 * T0_frac,	/* output: fractional part of pitch lag              */
			 Word16 bfi	/* input : bad frame indicator                       */
    );
Word16 w_d_gain_pitch(		/* out      : quantized pitch gain                   */
			     Word16 index,	/* in       : index of quantization                  */
			     Word16 bfi,	/* in       : bad frame indicator (good = 0)         */
			     Word16 w_state,	/* in       : w_state of the w_state machine             */
			     Word16 w_prev_bf,	/* Previous bf                                       */
			     Word16 w_rxdtx_ctrl);
void w_d_gain_code(Word16 index,	/* input : received quantization index               */
		   Word16 code[],	/* input : innovation codevector                     */
		   Word16 lcode,	/* input : codevector length                         */
		   Word16 * gain_code,	/* output: decoded innovation gain                   */
		   Word16 bfi,	/* input : bad frame indicator                       */
		   Word16 w_state,	/* in    : w_state of the w_state machine                */
		   Word16 w_prev_bf,	/* Previous bf                                       */
		   Word16 w_rxdtx_ctrl,
		   Word16 i_w_subfr, Word16 w_w_rx_dtx_w_state);
void w_D_plsf_5(Word16 * indice,	/* input : quantization indices of 5 w_submatrices     */
		Word16 * lsp1_q,	/* output: quantized 1st LSP vector                  */
		Word16 * lsp2_q,	/* output: quantized 2nd LSP vector                  */
		Word16 bfi,	/* input : bad frame indicator (set to 1 if a bad
				   frame is received)                        */
		Word16 w_rxdtx_ctrl, Word16 w_w_rx_dtx_w_state);
Word16 w_Enc_lag6(		/* output: Return index of encoding                  */
			 Word16 T0,	/* input : Pitch delay                               */
			 Word16 * T0_frac,	/* in/out: Fractional pitch delay                    */
			 Word16 * T0_min,	/* in/out: Minimum search delay                      */
			 Word16 * T0_max,	/* in/out: Maximum search delay                      */
			 Word16 pit_min,	/* input : Minimum pitch delay                       */
			 Word16 pit_max,	/* input : Maximum pitch delay                       */
			 Word16 pit_flag	/* input : Flag for 1st or 3rd w_subframe              */
    );

Word16 w_q_gain_pitch(		/* Return index of quantization                      */
			     Word16 * gain	/* (i)    :  Pitch gain to quantize                  */
    );

Word16 w_q_gain_code(		/* Return quantization index                         */
			    Word16 code[],	/* (i)      : fixed codebook w_excitation              */
			    Word16 lcode,	/* (i)      : codevector size                        */
			    Word16 * gain,	/* (i/o)    : quantized fixed codebook gain          */
			    Word16 w_txdtx_ctrl, Word16 i_w_subfr);

Word16 w_G_pitch(		/* (o)     : Gain of pitch lag w_saturated to 1.2      */
			Word16 xn[],	/* (i)     : Pitch target.                           */
			Word16 y1[],	/* (i)     : Filtered adaptive codebook.             */
			Word16 w_L_w_subfr	/*         : Length of w_subframe.                     */
    );
Word16 w_G_code(		/* out      : Gain of innovation code.               */
		       Word16 xn[],	/* in       : target vector                          */
		       Word16 y2[]	/* in       : filtered inovation vector              */
    );

Word16 w_Interpol_6(		/* (o)  : interpolated value                         */
			   Word16 * x,	/* (i)  : input vector                               */
			   Word16 frac	/* (i)  : fraction                                   */
    );
void w_Int_lpc(Word16 w_lsp_old[],	/* input: LSP vector at the 4th w_subfr. of past frame */
	       Word16 lsp_mid[],	/* input: LSP vector at the 2nd w_subfr. of
					   present frame                                     */
	       Word16 lsp_new[],	/* input: LSP vector at the 4th w_subfr. of
					   present frame                                     */
	       Word16 Az[]	/* output: interpolated LP parameters in all w_subfr.  */
    );
void w_w_Int_lpc2(Word16 w_lsp_old[],	/* input: LSP vector at the 4th w_subfr. of past frame */
		  Word16 lsp_mid[],	/* input: LSP vector at the 2nd w_subframe of
					   present frame                                     */
		  Word16 lsp_new[],	/* input: LSP vector at the 4th w_subframe of
					   present frame                                     */
		  Word16 Az[]	/* output:interpolated LP parameters
				   in w_subframes 1 and 3                              */
    );
Word16 w_Pitch_fr6(		/* (o)     : pitch period.                           */
			  Word16 w_exc[],	/* (i)     : w_excitation buffer                       */
			  Word16 xn[],	/* (i)     : target vector                           */
			  Word16 h[],	/* (i)     : impulse response of w_w_synthesis and
					   weighting filters                       */
			  Word16 w_L_w_subfr,	/* (i)     : Length of w_subframe                      */
			  Word16 t0_min,	/* (i)     : minimum value in the searched range.    */
			  Word16 t0_max,	/* (i)     : maximum value in the searched range.    */
			  Word16 i_w_subfr,	/* (i)     : indicator for first w_subframe.           */
			  Word16 * pit_frac	/* (o)     : chosen fraction.                        */
    );
Word16 w_Pitch_ol(		/* output: open loop pitch lag                       */
			 Word16 signal[],	/* input: signal used to compute the open loop pitch */
			 /* signal[-pit_max] to signal[-1] should be known    */
			 Word16 pit_min,	/* input : minimum pitch lag                         */
			 Word16 pit_max,	/* input : maximum pitch lag                         */
			 Word16 L_frame	/* input : length of frame to compute pitch          */
    );
void w_Pred_lt_6(Word16 w_exc[],	/* in/out: w_excitation buffer                         */
		 Word16 T0,	/* input : integer pitch lag                         */
		 Word16 frac,	/* input : fraction of lag                           */
		 Word16 w_L_w_subfr	/* input : w_subframe size                             */
    );
void w_Q_plsf_5(Word16 * lsp1,	/* input : 1st LSP vector                            */
		Word16 * lsp2,	/* input : 2nd LSP vector                            */
		Word16 * lsp1_q,	/* output: quantized 1st LSP vector                  */
		Word16 * lsp2_q,	/* output: quantized 2nd LSP vector                  */
		Word16 * indice,	/* output: quantization indices of 5 matrices        */
		Word16 w_txdtx_ctrl	/* input : tx dtx control word                       */
    );
void w_Bits2w_prm_12k2(Word16 bits[],	/* input : w_serial bits                               */
		       Word16 w_prm[]	/* output: analysis parameters                       */
    );
void w_Prm2bits_12k2(Word16 w_prm[],	/* input : analysis parameters                       */
		     Word16 bits[]	/* output: w_serial bits                               */
    );
