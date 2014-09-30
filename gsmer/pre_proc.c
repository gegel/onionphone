/*************************************************************************
 *
 *  FUNCTION:  w_Pre_Process()
 *
 *  PURPOSE: Preprocessing of input w_speech.
 *
 *  DESCRIPTION:
 *     - 2nd order high pass filtering with cut off frequency at 80 Hz.
 *     - Divide input by two.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"

/*------------------------------------------------------------------------*
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2                    *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *                                                                        *
 *  Input is divided by two in the filtering process.                     *
 *------------------------------------------------------------------------*/

/* filter coefficients (fc = 80 Hz, coeff. b[] is divided by 2) */

static const Word16 b[3] = {1899, -3798, 1899};
static const Word16 a[3] = {4096, 7807, -3733};

/* Static values to be preserved between calls */
/* y[] values are kept in double precision     */

static Word16 w_y2_hi, w_y2_lo, w_y1_hi, w_y1_lo, x0, x1;

/* Initialization of static values */

void w_Init_w_Pre_Process (void)
{
    w_y2_hi = 0;
    w_y2_lo = 0;
    w_y1_hi = 0;
    w_y1_lo = 0;
    x0 = 0;
    x1 = 0;
}

void w_Pre_Process (
    Word16 signal[], /* input/output signal */
    Word16 lg)       /* lenght of signal    */
{
    Word16 i, x2;
    Word32 L_tmp;

    for (i = 0; i < lg; i++)
    {
        x2 = x1;                     
        x1 = x0;                     
        x0 = signal[i];              

        /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b140[2]*x[i-2]/2  */
        /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

        L_tmp = w_w_Mpy_32_16 (w_y1_hi, w_y1_lo, a[1]);
        L_tmp = L_w_add (L_tmp, w_w_Mpy_32_16 (w_y2_hi, w_y2_lo, a[2]));
        L_tmp = w_L_mac (L_tmp, x0, b[0]);
        L_tmp = w_L_mac (L_tmp, x1, b[1]);
        L_tmp = w_L_mac (L_tmp, x2, b[2]);
        L_tmp = w_L_w_shl (L_tmp, 3);
        signal[i] = w_round (L_tmp);   

        w_y2_hi = w_y1_hi;               
        w_y2_lo = w_y1_lo;               
        w_L_Extract (L_tmp, &w_y1_hi, &w_y1_lo);
    }
    return;
}
