/*************************************************************************
 *
 *  FILE NAME:   pstfilt2.c
 *
 * Performs adaptive postfiltering on the w_w_synthesis w_speech
 *
 *  FUNCTIONS INCLUDED:  w_Init_w_Post_Filter()  and w_Post_Filter()
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "sig_proc.h"
#include "count.h"
#include "codec.h"
#include "cnst.h"

/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "cnst.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   MU          : Factor for tilt compensation filter           *
 *   AGC_FAC     : Factor for automatic gain control             *
 *---------------------------------------------------------------*/

#define L_H 22  /* size of truncated impulse response of A(z/g1)/A(z/g2) */

/*------------------------------------------------------------*
 *   static vectors                                           *
 *------------------------------------------------------------*/

 /* inverse filtered w_w_synthesis */

static Word16 w_res2[L_SUBFR];

 /* memory of filter 1/A(z/0.75) */

static Word16 w_w_mem_w_syn_pst[M];

 /* Spectral expansion factors */

const Word16 w_F_gamma3[M] =
{
    22938, 16057, 11240, 7868, 5508,
    3856, 2699, 1889, 1322, 925
};
const Word16 w_F_gamma4[M] =
{
    24576, 18432, 13824, 10368, 7776,
    5832, 4374, 3281, 2461, 1846
};

/*************************************************************************
 *
 *  FUNCTION:   w_Init_w_Post_Filter
 *
 *  PURPOSE: Initializes the postfilter parameters.
 *
 *************************************************************************/

void w_Init_w_Post_Filter (void)
{
    w_Set_w_zero (w_w_mem_w_syn_pst, M);

    w_Set_w_zero (w_res2, L_SUBFR);

    return;
}

/*************************************************************************
 *  FUNCTION:  w_Post_Filter()
 *
 *  PURPOSE:  postfiltering of w_w_synthesis w_speech.
 *
 *  DESCRIPTION:
 *      The postfiltering process is described as follows:
 *
 *          - inverse filtering of w_syn[] through A(z/0.7) to get w_res2[]
 *          - tilt compensation filtering; 1 - MU*k*z^-1
 *          - w_w_synthesis filtering through 1/A(z/0.75)
 *          - adaptive gain control
 *
 *************************************************************************/

void w_Post_Filter (
    Word16 *w_syn,    /* in/out: w_w_synthesis w_speech (postfiltered is output)    */
    Word16 *Az_4    /* input: interpolated LPC parameters in all w_subframes  */
)
{
    /*-------------------------------------------------------------------*
     *           Declaration of parameters                               *
     *-------------------------------------------------------------------*/

    Word16 w_syn_pst[L_FRAME];    /* post filtered w_w_synthesis w_speech   */
    Word16 Ap3[MP1], Ap4[MP1];  /* bandwidth expanded LP parameters */
    Word16 *Az;                 /* pointer to Az_4:                 */
                                /*  LPC parameters in each w_subframe */
    Word16 i_w_subfr;             /* index for beginning of w_subframe  */
    Word16 h[L_H];

    Word16 i;
    Word16 temp1, temp2;
    Word32 L_tmp;

    /*-----------------------------------------------------*
     * Post filtering                                      *
     *-----------------------------------------------------*/

    Az = Az_4;

    for (i_w_subfr = 0; i_w_subfr < L_FRAME; i_w_subfr += L_SUBFR)
    {
        /* Find weighted filter coefficients Ap3[] and ap[4] */

        w_Weight_Ai (Az, w_F_gamma3, Ap3);
        w_Weight_Ai (Az, w_F_gamma4, Ap4);

        /* filtering of w_w_synthesis w_speech by A(z/0.7) to find w_res2[] */

        w_Residu (Ap3, &w_syn[i_w_subfr], w_res2, L_SUBFR);

        /* tilt compensation filter */

        /* impulse response of A(z/0.7)/A(z/0.75) */

        w_Copy (Ap3, h, M + 1);
        w_Set_w_zero (&h[M + 1], L_H - M - 1);
        w_Syn_filt (Ap4, h, h, L_H, &h[M + 1], 0);

        /* 1st correlation of h[] */

        L_tmp = w_L_w_mult (h[0], h[0]);
        for (i = 1; i < L_H; i++)
        {
            L_tmp = w_L_mac (L_tmp, h[i], h[i]);
        }
        temp1 = w_extract_h (L_tmp);

        L_tmp = w_L_w_mult (h[0], h[1]);
        for (i = 1; i < L_H - 1; i++)
        {
            L_tmp = w_L_mac (L_tmp, h[i], h[i + 1]);
        }
        temp2 = w_extract_h (L_tmp);

          
        if (temp2 <= 0)
        {
            temp2 = 0;            
        }
        else
        {
            temp2 = w_mult (temp2, MU);
            temp2 = w_div_s (temp2, temp1);
        }

        w_preemphasis (w_res2, temp2, L_SUBFR);

        /* filtering through  1/A(z/0.75) */

        w_Syn_filt (Ap4, w_res2, &w_syn_pst[i_w_subfr], L_SUBFR, w_w_mem_w_syn_pst, 1);

        /* scale output to input */

        w_agc (&w_syn[i_w_subfr], &w_syn_pst[i_w_subfr], AGC_FAC, L_SUBFR);

        Az += MP1;
    }

    /* update w_syn[] buffer */

    w_Copy (&w_syn[L_FRAME - M], &w_syn[-M], M);

    /* overwrite w_w_synthesis w_speech by postfiltered w_w_synthesis w_speech */

    w_Copy (w_syn_pst, w_syn, L_FRAME);

    return;
}
