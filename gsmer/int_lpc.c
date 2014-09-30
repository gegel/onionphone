/*************************************************************************
 *
 *  FUNCTION:  w_Int_lpc()
 *
 *  PURPOSE:  Interpolates the LSPs and converts to LPC parameters to get
 *            a different LP filter in each w_subframe.
 *
 *  DESCRIPTION:
 *     The 20 ms w_speech frame is divided into 4 w_subframes.
 *     The LSPs are quantized and transmitted at the 2nd and 4th w_subframes
 *     (twice per frame) and interpolated at the 1st and 3rd w_subframe.
 *
 *          |------|------|------|------|
 *             sf1    sf2    sf3    sf4
 *       F0            Fm            F1
 *
 *     sf1:   1/2 Fm + 1/2 F0         sf3:   1/2 F1 + 1/2 Fm
 *     sf2:       Fm                  sf4:       F1
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "sig_proc.h"

#define M   10                  /* LP order */
#define MP1 11                  /* M+1 */

void w_Int_lpc (
    Word16 w_lsp_old[],   /* input : LSP vector at the 4th w_subframe
                           of past frame    */
    Word16 lsp_mid[],   /* input : LSP vector at the 2nd w_subframe
                           of present frame */
    Word16 lsp_new[],   /* input : LSP vector at the 4th w_subframe of
                           present frame */
    Word16 Az[]         /* output: interpolated LP parameters in
                           all w_subframes */
)
{
    Word16 i;
    Word16 lsp[M];

    /*  lsp[i] = lsp_mid[i] * 0.5 + w_lsp_old[i] * 0.5 */

    for (i = 0; i < M; i++)
    {
        lsp[i] = w_add (w_shr (lsp_mid[i], 1), w_shr (w_lsp_old[i], 1));
                                  
    }

    w_Lsp_Az (lsp, Az);           /* Subframe 1 */
    Az += MP1;                    

    w_Lsp_Az (lsp_mid, Az);       /* Subframe 2 */
    Az += MP1;                    

    for (i = 0; i < M; i++)
    {
        lsp[i] = w_add (w_shr (lsp_mid[i], 1), w_shr (lsp_new[i], 1));
                                  
    }

    w_Lsp_Az (lsp, Az);           /* Subframe 3 */
    Az += MP1;                    

    w_Lsp_Az (lsp_new, Az);       /* Subframe 4 */

    return;
}

/*----------------------------------------------------------------------*
 * Function w_w_Int_lpc2()                                                  *
 * ~~~~~~~~~~~~~~~~~~                                                   *
 * Interpolation of the LPC parameters.                                 *
 * Same as the previous function but we do not recompute Az() for       *
 * w_subframe 2 and 4 because it is already available.                    *
 *----------------------------------------------------------------------*/

void w_w_Int_lpc2 (
             Word16 w_lsp_old[],  /* input : LSP vector at the 4th w_subframe
                                 of past frame    */
             Word16 lsp_mid[],  /* input : LSP vector at the 2nd w_subframe
                                 of present frame */
             Word16 lsp_new[],  /* input : LSP vector at the 4th w_subframe of
                                 present frame */
             Word16 Az[]        /* output: interpolated LP parameters
                                 in w_subframes 1 and 3 */
)
{
    Word16 i;
    Word16 lsp[M];

    /*  lsp[i] = lsp_mid[i] * 0.5 + w_lsp_old[i] * 0.5 */

    for (i = 0; i < M; i++)
    {
        lsp[i] = w_add (w_shr (lsp_mid[i], 1), w_shr (w_lsp_old[i], 1));
                                  
    }
    w_Lsp_Az (lsp, Az);           /* Subframe 1 */
    Az += MP1 * 2;                

    for (i = 0; i < M; i++)
    {
        lsp[i] = w_add (w_shr (lsp_mid[i], 1), w_shr (lsp_new[i], 1));
                                  
    }
    w_Lsp_Az (lsp, Az);           /* Subframe 3 */

    return;
}
