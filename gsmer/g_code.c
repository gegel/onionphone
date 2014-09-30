/*************************************************************************
 *
 *  FUNCTION:   w_G_code
 *
 *  PURPOSE:  Compute the innovative codebook gain.
 *
 *  DESCRIPTION:
 *      The innovative codebook gain is given by
 *
 *              g = <x[], y[]> / <y[], y[]>
 *
 *      where x[] is the target vector, y[] is the filtered innovative
 *      codevector, and <> denotes dot product.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "cnst.h"

Word16 w_G_code (         /* out   : Gain of innovation code         */
    Word16 xn2[],       /* in    : target vector                   */
    Word16 y2[]         /* in    : filtered innovation vector      */
)
{
    Word16 i;
    Word16 xy, yy, exp_xy, exp_yy, gain;
    Word16 scal_y2[L_SUBFR];
    Word32 s;

    /* Scale down Y[] by 2 to avoid overflow */

    for (i = 0; i < L_SUBFR; i++)
    {
        scal_y2[i] = w_shr (y2[i], 1);    
    }

    /* Compute scalar product <X[],Y[]> */

    s = 1L;                             /* Avoid case of all w_zeros */
    for (i = 0; i < L_SUBFR; i++)
    {
        s = w_L_mac (s, xn2[i], scal_y2[i]);
    }
    exp_xy = w_norm_l (s);
    xy = w_extract_h (w_L_w_shl (s, exp_xy));

    /* If (xy < 0) gain = 0  */

      
    if (xy <= 0)
        return ((Word16) 0);

    /* Compute scalar product <Y[],Y[]> */

    s = 0L;                             
    for (i = 0; i < L_SUBFR; i++)
    {
        s = w_L_mac (s, scal_y2[i], scal_y2[i]);
    }
    exp_yy = w_norm_l (s);
    yy = w_extract_h (w_L_w_shl (s, exp_yy));

    /* compute gain = xy/yy */

    xy = w_shr (xy, 1);                 /* Be sure xy < yy */
    gain = w_div_s (xy, yy);

    /* Denormalization of division */
    i = w_add (exp_xy, 5);              /* 15-1+9-18 = 5 */
    i = w_sub (i, exp_yy);

    gain = w_shr (gain, i);

    return (gain);
}
