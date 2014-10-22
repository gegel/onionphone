/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*--------------------------------------------------------------------------*
 * Function w_q_gain_pitch(), w_q_gain_code()                                  *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                  *
 * Scalar quantization of the pitch gain and the innovative codebook gain.  *
 *                                                                          *
 * MA w_prediction is performed on the innovation energy                      *
 * (in dB/(20*log10(2))) with mean removed.                                 *
 *-------------------------------------------------------------------------*/

#include "ophint.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

#include "gains_tb.h"

#include "cnst.h"
#include "dtx.h"

/* past quantized energies.                               */
/* initialized to -14.0/constant, constant = 20*Log10(2)   */
Word16 w_past_qua_en[4];

/* MA w_prediction coeff */
Word16 w_pred[4];

extern Word16 w_CN_w_excitation_gain, w_gain_code_old_tx[4 * DTX_HANGOVER];

Word16 w_q_gain_pitch(		/* Return index of quantization */
			     Word16 * gain	/* (i)  :  Pitch gain to quantize  */
    )
{
	Word16 i, index, gain_q14, err, err_min;

	gain_q14 = w_shl(*gain, 2);

	err_min = w_abs_s(w_sub(gain_q14, w_qua_gain_pitch[0]));
	index = 0;

	for (i = 1; i < NB_QUA_PITCH; i++) {
		err = w_abs_s(w_sub(gain_q14, w_qua_gain_pitch[i]));

		if (w_sub(err, err_min) < 0) {
			err_min = err;
			index = i;
		}
	}

	*gain = w_shr(w_qua_gain_pitch[index], 2);

	return index;
}

/* average innovation energy.                             */
/* MEAN_ENER  = 36.0/constant, constant = 20*Log10(2)     */

#define MEAN_ENER  783741L	/* 36/(20*log10(2))       */

Word16 w_q_gain_code(		/* Return quantization index                  */
			    Word16 code[],	/* (i)      : fixed codebook w_excitation       */
			    Word16 lcode,	/* (i)      : codevector size                 */
			    Word16 * gain,	/* (i/o)    : quantized fixed codebook gain   */
			    Word16 w_txdtx_ctrl, Word16 i_w_subfr)
{
	Word16 i, index = 0;
	Word16 gcode0, err, err_min, exp, frac;
	Word32 ener, ener_code;
	Word16 aver_gain;
	static Word16 w_gcode0_CN;

	if ((w_txdtx_ctrl & TX_SP_FLAG) != 0) {

	/*-------------------------------------------------------------------*
         *  energy of code:                                                   *
         *  ~~~~~~~~~~~~~~~                                                   *
         *  ener_code(Q17) = 10 * Log10(energy/lcode) / constant              *
         *                 = 1/2 * w_Log2(energy/lcode)                         *
         *                                           constant = 20*Log10(2)   *
         *-------------------------------------------------------------------*/

		/* ener_code = log10(ener_code/lcode) / (20*log10(2))       */
		ener_code = 0;
		for (i = 0; i < lcode; i++) {
			ener_code = w_L_mac(ener_code, code[i], code[i]);
		}
		/* ener_code = ener_code / lcode */
		ener_code = w_L_w_mult(w_round(ener_code), 26214);

		/* ener_code = 1/2 * w_Log2(ener_code) */
		w_Log2(ener_code, &exp, &frac);
		ener_code = w_L_Comp(w_sub(exp, 30), frac);

		/* w_predicted energy */

		ener = MEAN_ENER;
		for (i = 0; i < 4; i++) {
			ener = w_L_mac(ener, w_past_qua_en[i], w_pred[i]);
		}

	/*-------------------------------------------------------------------*
         *  w_predicted codebook gain                                           *
         *  ~~~~~~~~~~~~~~~~~~~~~~~                                           *
         *  gcode0(Qx) = Pow10( (ener*constant - ener_code*constant) / 20 )   *
         *             = w_Pow2(ener-ener_code)                                 *
         *                                           constant = 20*Log10(2)   *
         *-------------------------------------------------------------------*/

		ener = w_L_w_shr(w_L_w_sub(ener, ener_code), 1);
		w_L_Extract(ener, &exp, &frac);

		gcode0 = w_extract_l(w_Pow2(exp, frac));	/* w_predicted gain */

		gcode0 = w_shl(gcode0, 4);

	/*-------------------------------------------------------------------*
         *                   Search for best quantizer                        *
         *-------------------------------------------------------------------*/

		err_min =
		    w_abs_s(w_sub(*gain, w_mult(gcode0, w_qua_gain_code[0])));
		index = 0;

		for (i = 1; i < NB_QUA_CODE; i++) {
			err =
			    w_abs_s(w_sub
				    (*gain,
				     w_mult(gcode0, w_qua_gain_code[i])));

			if (w_sub(err, err_min) < 0) {
				err_min = err;
				index = i;
			}
		}

		*gain = w_mult(gcode0, w_qua_gain_code[index]);

	/*------------------------------------------------------------------*
         *  update w_table of past quantized energies                         *
         *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                         *
         *  w_past_qua_en(Q12) = 20 * Log10(w_qua_gain_code) / constant         *
         *                   = w_Log2(w_qua_gain_code)                          *
         *                                           constant = 20*Log10(2) *
         *------------------------------------------------------------------*/

		for (i = 3; i > 0; i--) {
			w_past_qua_en[i] = w_past_qua_en[i - 1];
		}
		w_Log2(w_L_deposit_l(w_qua_gain_code[index]), &exp, &frac);

		w_past_qua_en[0] = w_shr(frac, 5);
		w_past_qua_en[0] =
		    w_add(w_past_qua_en[0], w_shl(w_sub(exp, 11), 10));

		w_update_gain_code_history_tx(*gain, w_gain_code_old_tx);
	} else {

		if ((w_txdtx_ctrl & TX_PREV_HANGOVER_ACTIVE) != 0
		    && (i_w_subfr == 0)) {
			w_gcode0_CN = update_w_gcode0_CN(w_gain_code_old_tx);
			w_gcode0_CN = w_shl(w_gcode0_CN, 4);
		}
		*gain = w_CN_w_excitation_gain;

		if ((w_txdtx_ctrl & TX_SID_UPDATE) != 0) {
			aver_gain =
			    w_aver_gain_code_history(w_CN_w_excitation_gain,
						     w_gain_code_old_tx);

	    /*---------------------------------------------------------------*
             *                   Search for best quantizer                    *
             *---------------------------------------------------------------*/

			err_min = w_abs_s(w_sub(aver_gain,
						w_mult(w_gcode0_CN,
						       w_qua_gain_code[0])));
			index = 0;

			for (i = 1; i < NB_QUA_CODE; i++) {
				err = w_abs_s(w_sub(aver_gain,
						    w_mult(w_gcode0_CN,
							   w_qua_gain_code
							   [i])));

				if (w_sub(err, err_min) < 0) {
					err_min = err;
					index = i;
				}
			}
		}
		w_update_gain_code_history_tx(*gain, w_gain_code_old_tx);

	/*-------------------------------------------------------------------*
         *  reset w_table of past quantized energies                            *
         *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                            *
         *-------------------------------------------------------------------*/

		for (i = 0; i < 4; i++) {
			w_past_qua_en[i] = -2381;
		}
	}

	return index;
}
