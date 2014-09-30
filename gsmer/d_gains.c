/*************************************************************************
 *
 *  FILE NAME:   D_GAINS.C
 *
 *  FUNCTIONS DEFINED IN THIS FILE:
 *
 *        w_d_gain_pitch(), w_d_gain_code() and w_gmed5()
 *
 * MA v_prediction is performed on the innovation energy
 * ( in dB/(20*log10(2)) ) with mean removed.
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

#include "gains_tb.h"

#include "cnst.h"
#include "dtx.h"

extern Word16 w_gain_code_old_rx[4 * DTX_HANGOVER];

/* Static variables for bad frame handling */

/* Variables used by w_d_gain_pitch: */
Word16 w_pbuf[5], w_w_past_gain_pit, w_prev_gp;

/* Variables used by w_d_gain_code: */
Word16 w_gbuf[5], w_w_past_gain_code, w_prev_gc;

/* Static variables for CNI (used by w_d_gain_code) */

Word16 w_gcode0_CN, w_gain_code_old_CN, w_gain_code_new_CN, w_gain_code_muting_CN;

/* Memories of gain dequantization: */

/* past quantized energies.      */
/* initialized to -14.0/constant, constant = 20*Log10(2) */

Word16 v_past_qua_en[4];

/* MA v_prediction coeff   */
Word16 v_pred[4];

/*************************************************************************
 *
 *  FUNCTION:   w_gmed5
 *
 *  PURPOSE:    calculates 5-point median.
 *
 *  DESCRIPTION:
 *             
 *************************************************************************/

Word16 w_gmed5 (        /* out      : index of the median value (0...4) */
    Word16 ind[]      /* in       : Past gain values                  */
)
{
    Word16 i, j, ix = 0, tmp[5];
    Word16 max, tmp2[5];

    for (i = 0; i < 5; i++)
    {
        tmp2[i] = ind[i];                                        
    }

    for (i = 0; i < 5; i++)
    {
        max = -8192;                                             
        for (j = 0; j < 5; j++)
        {
              
            if (w_sub (tmp2[j], max) >= 0)
            {
                max = tmp2[j];                                   
                ix = j;                                          
            }
        }
        tmp2[ix] = -16384;                                       
        tmp[i] = ix;                                             
    }

    return (ind[tmp[2]]);
}

/*************************************************************************
 *
 *  FUNCTION:   w_d_gain_pitch
 *
 *  PURPOSE:  decodes the pitch gain using the received index.
 *
 *  DESCRIPTION:
 *       In case of no frame erasure, the gain is obtained from the
 *       quantization w_table at the given index; otherwise, a downscaled
 *       past gain is used.
 *
 *************************************************************************/

Word16 w_d_gain_pitch ( /* out      : quantized pitch gain           */
    Word16 index,     /* in       : index of quantization          */
    Word16 bfi,       /* in       : bad frame indicator (good = 0) */
    Word16 w_state,
    Word16 w_prev_bf,
    Word16 w_rxdtx_ctrl
)
{
    static const Word16 pdown[7] =
    {
        32767, 32112, 32112, 26214,
        9830, 6553, 6553
    };

    Word16 gain, tmp, i;

      
    if (bfi == 0)
    {
            
        if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0)
        {
            gain = w_shr (w_qua_gain_pitch[index], 2);               

              
            if (w_prev_bf != 0)
            {
                  
                if (w_sub (gain, w_prev_gp) > 0)
                {
                    gain = w_prev_gp;
                }
            }
        }
        else
        {
            gain = 0;                                            
        }
        w_prev_gp = gain;                                          
    }
    else
    {
            
        if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0)
        {
            tmp = w_gmed5 (w_pbuf);                                  

              
            if (w_sub (tmp, w_w_past_gain_pit) < 0)
            {
                w_w_past_gain_pit = tmp;                             
            }
            gain = w_mult (pdown[w_state], w_w_past_gain_pit);
        }
        else
        {
            gain = 0;                                            
        }
    }

    w_w_past_gain_pit = gain;                                        

      
    if (w_sub (w_w_past_gain_pit, 4096) > 0)  /* if (w_w_past_gain_pit > 1.0) */
    {
        w_w_past_gain_pit = 4096;                                    
    }
    for (i = 1; i < 5; i++)
    {
        w_pbuf[i - 1] = w_pbuf[i];                                   
    }

    w_pbuf[4] = w_w_past_gain_pit;                                     

    return gain;
}

/*************************************************************************
 *
 *  FUNCTION:  w_d_gain_code
 *
 *  PURPOSE:  decode the fixed codebook gain using the received index.
 *
 *  DESCRIPTION:
 *      The received index gives the gain correction factor gamma.
 *      The quantized gain is given by   g_q = g0 * gamma
 *      where g0 is the v_predicted gain.
 *      To find g0, 4th order MA v_prediction is applied to the mean-removed
 *      innovation energy in dB.
 *      In case of frame erasure, downscaled past gain is used.
 *
 *************************************************************************/

/* average innovation energy.                             */
/* MEAN_ENER = 36.0/constant, constant = 20*Log10(2)      */
#define MEAN_ENER  783741L      /* 36/(20*log10(2))       */

void w_d_gain_code (
    Word16 index,      /* input : received quantization index */
    Word16 code[],     /* input : innovation codevector       */
    Word16 lcode,      /* input : codevector length           */
    Word16 *gain_code, /* output: decoded innovation gain     */
    Word16 bfi,        /* input : bad frame indicator         */
    Word16 w_state,
    Word16 w_prev_bf,
    Word16 w_rxdtx_ctrl,
    Word16 i_w_subfr,
    Word16 w_w_rx_dtx_w_state
)
{
    static const Word16 cdown[7] =
    {
        32767, 32112, 32112, 32112,
        32112, 32112, 22937
    };

    Word16 i, tmp;
    Word16 gcode0, exp, frac, av_v_pred_en;
    Word32 ener, ener_code;

          
    if (((w_rxdtx_ctrl & RX_UPD_SID_QUANT_MEM) != 0) && (i_w_subfr == 0))
    {
        w_gcode0_CN = update_w_gcode0_CN (w_gain_code_old_rx);         
        w_gcode0_CN = w_shl (w_gcode0_CN, 4);
    }

    /* Handle cases of comfort noise fixed codebook gain decoding in
       which past valid SID frames are repeated */

                
    if (((w_rxdtx_ctrl & RX_NO_TRANSMISSION) != 0)
        || ((w_rxdtx_ctrl & RX_INVALID_SID_FRAME) != 0)
        || ((w_rxdtx_ctrl & RX_LOST_SID_FRAME) != 0))
    {

            
        if ((w_rxdtx_ctrl & RX_NO_TRANSMISSION) != 0)
        {
            /* DTX active: no transmission. Interpolate gain values
            in memory */
              
            if (i_w_subfr == 0)
            {
                *gain_code = w_interpolate_CN_param (w_gain_code_old_CN,
                                            w_gain_code_new_CN, w_w_rx_dtx_w_state);
                                                                 
            }
            else
            {
                *gain_code = w_prev_gc;                            
            }
        }
        else
        {                       /* Invalid or lost SID frame:
            use gain values from last good SID frame */
            w_gain_code_old_CN = w_gain_code_new_CN;                 
            *gain_code = w_gain_code_new_CN;                       

            /* reset w_table of past quantized energies */
            for (i = 0; i < 4; i++)
            {
                v_past_qua_en[i] = -2381;                          
            }
        }

            
        if ((w_rxdtx_ctrl & RX_DTX_MUTING) != 0)
        {
            /* attenuate the gain value by 0.75 dB in each w_subframe */
            /* (total of 3 dB per frame) */
            w_gain_code_muting_CN = w_mult (w_gain_code_muting_CN, 30057);
            *gain_code = w_gain_code_muting_CN;                    
        }
        else
        {
            /* Prepare for DTX muting by storing last good gain value */
            w_gain_code_muting_CN = w_gain_code_new_CN;              
        }

        w_w_past_gain_code = *gain_code;                             

        for (i = 1; i < 5; i++)
        {
            w_gbuf[i - 1] = w_gbuf[i];                               
        }

        w_gbuf[4] = w_w_past_gain_code;                                
        w_prev_gc = w_w_past_gain_code;                                

        return;
    }

    /*----------------- Test erasure ---------------*/

      
    if (bfi != 0)
    {
        tmp = w_gmed5 (w_gbuf);                                      
          
        if (w_sub (tmp, w_w_past_gain_code) < 0)
        {
            w_w_past_gain_code = tmp;                                
        }
        w_w_past_gain_code = w_mult (w_w_past_gain_code, cdown[w_state]);
        *gain_code = w_w_past_gain_code;                             

        av_v_pred_en = 0;                                          
        for (i = 0; i < 4; i++)
        {
            av_v_pred_en = w_add (av_v_pred_en, v_past_qua_en[i]);
        }

        /* av_v_pred_en = 0.25*av_v_pred_en - 4/(20Log10(2)) */
        av_v_pred_en = w_mult (av_v_pred_en, 8192);   /*  *= 0.25  */

        /* if (av_v_pred_en < -14/(20Log10(2))) av_v_pred_en = .. */
          
        if (w_sub (av_v_pred_en, -2381) < 0)
        {
            av_v_pred_en = -2381;                                  
        }
        for (i = 3; i > 0; i--)
        {
            v_past_qua_en[i] = v_past_qua_en[i - 1];                 
        }
        v_past_qua_en[0] = av_v_pred_en;                             
        for (i = 1; i < 5; i++)
        {
            w_gbuf[i - 1] = w_gbuf[i];                               
        }
        w_gbuf[4] = w_w_past_gain_code;                                

        /* Use the most recent comfort noise fixed codebook gain value
           for updating the fixed codebook gain history */
         
        if (w_gain_code_new_CN == 0)
        {
            tmp = w_prev_gc;                                      
        }
        else
        {
            tmp = w_gain_code_new_CN;
        }

        w_update_gain_code_history_rx (tmp, w_gain_code_old_rx);

         
        if (w_sub (i_w_subfr, (3 * L_SUBFR)) == 0)
        {
            w_gain_code_old_CN = *gain_code;                       
        }
        return;
    }

        
    if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0)
    {

        /*-------------- Decode codebook gain ---------------*/

        /*-------------------------------------------------------------------*
         *  energy of code:                                                   *
         *  ~~~~~~~~~~~~~~~                                                   *
         *  ener_code = 10 * Log10(energy/lcode) / constant                   *
         *            = 1/2 * w_Log2(energy/lcode)                              *
         *                                           constant = 20*Log10(2)   *
         *-------------------------------------------------------------------*/

        /* ener_code = log10(ener_code/lcode) / (20*log10(2)) */
        ener_code = 0;                                           
        for (i = 0; i < lcode; i++)
        {
            ener_code = w_L_mac (ener_code, code[i], code[i]);
        }
        /* ener_code = ener_code / lcode */
        ener_code = w_L_w_mult (w_round (ener_code), 26214);

        /* ener_code = 1/2 * w_Log2(ener_code) */
        w_Log2 (ener_code, &exp, &frac);
        ener_code = w_L_Comp (w_sub (exp, 30), frac);

        /* v_predicted energy */

        ener = MEAN_ENER;                                        
        for (i = 0; i < 4; i++)
        {
            ener = w_L_mac (ener, v_past_qua_en[i], v_pred[i]);
        }

        /*-------------------------------------------------------------------*
         *  v_predicted codebook gain                                           *
         *  ~~~~~~~~~~~~~~~~~~~~~~~                                           *
         *  gcode0     = Pow10( (ener*constant - ener_code*constant) / 20 )   *
         *             = w_Pow2(ener-ener_code)                                 *
         *                                           constant = 20*Log10(2)   *
         *-------------------------------------------------------------------*/

        ener = w_L_w_shr (w_L_w_sub (ener, ener_code), 1);
        w_L_Extract (ener, &exp, &frac);

        gcode0 = w_extract_l (w_Pow2 (exp, frac));  /* v_predicted gain */

        gcode0 = w_shl (gcode0, 4);

        *gain_code = w_mult (w_qua_gain_code[index], gcode0);        

          
        if (w_prev_bf != 0)
        {
              
            if (w_sub (*gain_code, w_prev_gc) > 0)
            {
                *gain_code = w_prev_gc;       
            }
        }
        /*-------------------------------------------------------------------*
         *  update w_table of past quantized energies                           *
         *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                           *
         *  v_past_qua_en      = 20 * Log10(w_qua_gain_code) / constant           *
         *                   = w_Log2(w_qua_gain_code)                            *
         *                                           constant = 20*Log10(2)   *
         *-------------------------------------------------------------------*/

        for (i = 3; i > 0; i--)
        {
            v_past_qua_en[i] = v_past_qua_en[i - 1];                 
        }
        w_Log2 (w_L_deposit_l (w_qua_gain_code[index]), &exp, &frac);

        v_past_qua_en[0] = w_shr (frac, 5);                          
        v_past_qua_en[0] = w_add (v_past_qua_en[0], w_shl (w_sub (exp, 11), 10));
          

        w_update_gain_code_history_rx (*gain_code, w_gain_code_old_rx);

        if (w_sub (i_w_subfr, (3 * L_SUBFR)) == 0)
        {
            w_gain_code_old_CN = *gain_code;                       
        }
    }
    else
    {
              
        if (((w_rxdtx_ctrl & RX_FIRST_SID_UPDATE) != 0) && (i_w_subfr == 0))
        {
            w_gain_code_new_CN = w_mult (w_gcode0_CN, w_qua_gain_code[index]);

            /*---------------------------------------------------------------*
             *  reset w_table of past quantized energies                        *
             *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                        *
             *---------------------------------------------------------------*/

            for (i = 0; i < 4; i++)
            {
                v_past_qua_en[i] = -2381;                          
            }
        }
              
        if (((w_rxdtx_ctrl & RX_CONT_SID_UPDATE) != 0) && (i_w_subfr == 0))
        {
            w_gain_code_old_CN = w_gain_code_new_CN;                 
            w_gain_code_new_CN = w_mult (w_gcode0_CN, w_qua_gain_code[index]);
                                                                 
        }
          
        if (i_w_subfr == 0)
        {
            *gain_code = w_interpolate_CN_param (w_gain_code_old_CN,
                                               w_gain_code_new_CN,
                                               w_w_rx_dtx_w_state);    
        }
        else
        {
            *gain_code = w_prev_gc;                                
        }
    }

    w_w_past_gain_code = *gain_code;                                 

    for (i = 1; i < 5; i++)
    {
        w_gbuf[i - 1] = w_gbuf[i];                                   
    }
    w_gbuf[4] = w_w_past_gain_code;                                    
    w_prev_gc = w_w_past_gain_code;                                    

    return;
}
