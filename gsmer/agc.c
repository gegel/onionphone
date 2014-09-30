/*************************************************************************
 *
 *  FUNCTION:  w_agc
 *
 *  PURPOSE: Scales the postfilter output on a w_subframe basis by automatic
 *           control of the w_subframe gain.
 *
 *  DESCRIPTION:
 *   sig_out[n] = sig_out[n] * gain[n];
 *   where gain[n] is the gain at the nth sample given by
 *     gain[n] = w_agc_fac * gain[n-1] + (1 - w_agc_fac) g_in/g_out
 *   g_in/g_out is the square root of the ratio of energy at the input
 *   and output of the postfilter.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "sig_proc.h"
#include "cnst.h"

Word16 w_past_gain;               /* initial value of w_past_gain = 1.0  */

void w_agc (
    Word16 *sig_in,             /* (i)     : postfilter input signal  */
    Word16 *sig_out,            /* (i/o)   : postfilter output signal */
    Word16 w_agc_fac,             /* (i)     : AGC factor               */
    Word16 l_trm                /* (i)     : w_subframe size            */
)
{
    Word16 i, exp;
    Word16 gain_in, gain_out, g0, gain;
    Word32 s;

    Word16 temp;

    /* calculate gain_out with exponent */

    temp = w_shr (sig_out[0], 2);
    s = w_L_w_mult (temp, temp);

    for (i = 1; i < l_trm; i++)
    {
        temp = w_shr (sig_out[i], 2);
        s = w_L_mac (s, temp, temp);
    }

      
    if (s == 0)
    {
        w_past_gain = 0;            
        return;
    }
    exp = w_sub (w_norm_l (s), 1);
    gain_out = w_round (w_L_w_shl (s, exp));

    /* calculate gain_in with exponent */

    temp = w_shr (sig_in[0], 2);
    s = w_L_w_mult (temp, temp);

    for (i = 1; i < l_trm; i++)
    {
        temp = w_shr (sig_in[i], 2);
        s = w_L_mac (s, temp, temp);
    }

      
    if (s == 0)
    {
        g0 = 0;                   
    }
    else
    {
        i = w_norm_l (s);
        gain_in = w_round (w_L_w_shl (s, i));
        exp = w_sub (exp, i);

        /*---------------------------------------------------*
         *  g0 = (1-w_agc_fac) * sqrt(gain_in/gain_out);       *
         *---------------------------------------------------*/

        s = w_L_deposit_l (w_div_s (gain_out, gain_in));
        s = w_L_w_shl (s, 7);       /* s = gain_out / gain_in */
        s = w_L_w_shr (s, exp);     /* w_add exponent */

        s = w_Inv_sqrt (s);
        i = w_round (w_L_w_shl (s, 9));

        /* g0 = i * (1-w_agc_fac) */
        g0 = w_mult (i, w_sub (32767, w_agc_fac));
    }

    /* compute gain[n] = w_agc_fac * gain[n-1]
                        + (1-w_agc_fac) * sqrt(gain_in/gain_out) */
    /* sig_out[n] = gain[n] * sig_out[n]                        */

    gain = w_past_gain;             

    for (i = 0; i < l_trm; i++)
    {
        gain = w_mult (gain, w_agc_fac);
        gain = w_add (gain, g0);
        sig_out[i] = w_extract_h (w_L_w_shl (w_L_w_mult (sig_out[i], gain), 3));
                                  
    }

    w_past_gain = gain;             

    return;
}

void w_w_agc2 (
 Word16 *sig_in,        /* (i)     : postfilter input signal  */
 Word16 *sig_out,       /* (i/o)   : postfilter output signal */
 Word16 l_trm           /* (i)     : w_subframe size            */
)
{
    Word16 i, exp;
    Word16 gain_in, gain_out, g0;
    Word32 s;

    Word16 temp;

    /* calculate gain_out with exponent */

    temp = w_shr (sig_out[0], 2);
    s = w_L_w_mult (temp, temp);
    for (i = 1; i < l_trm; i++)
    {
        temp = w_shr (sig_out[i], 2);
        s = w_L_mac (s, temp, temp);
    }

      
    if (s == 0)
    {
        return;
    }
    exp = w_sub (w_norm_l (s), 1);
    gain_out = w_round (w_L_w_shl (s, exp));

    /* calculate gain_in with exponent */

    temp = w_shr (sig_in[0], 2);
    s = w_L_w_mult (temp, temp);
    for (i = 1; i < l_trm; i++)
    {
        temp = w_shr (sig_in[i], 2);
        s = w_L_mac (s, temp, temp);
    }

      
    if (s == 0)
    {
        g0 = 0;                   
    }
    else
    {
        i = w_norm_l (s);
        gain_in = w_round (w_L_w_shl (s, i));
        exp = w_sub (exp, i);

        /*---------------------------------------------------*
         *  g0 = sqrt(gain_in/gain_out);                     *
         *---------------------------------------------------*/

        s = w_L_deposit_l (w_div_s (gain_out, gain_in));
        s = w_L_w_shl (s, 7);       /* s = gain_out / gain_in */
        s = w_L_w_shr (s, exp);     /* w_add exponent */

        s = w_Inv_sqrt (s);
        g0 = w_round (w_L_w_shl (s, 9));
    }

    /* sig_out(n) = gain(n) sig_out(n) */

    for (i = 0; i < l_trm; i++)
    {
        sig_out[i] = w_extract_h (w_L_w_shl (w_L_w_mult (sig_out[i], g0), 3));
                                  
    }

    return;
}
