/*************************************************************************
 *   FUNCTION:  w_Q_plsf_5()
 *
 *   PURPOSE:  Quantization of 2 sets of LSF parameters using 1st order MA
 *             w_prediction and split by 5 matrix quantization (split-MQ)
 *
 *   DESCRIPTION:
 *
 *        p[i] = w_pred_factor*past_r2q[i];   i=0,...,m-1
 *        r1[i]= lsf1[i] - p[i];      i=0,...,m-1
 *        r2[i]= lsf2[i] - p[i];      i=0,...,m-1
 *   where:
 *        lsf1[i]           1st mean-removed LSF vector.
 *        lsf2[i]           2nd mean-removed LSF vector.
 *        r1[i]             1st residual w_prediction vector.
 *        r2[i]             2nd residual w_prediction vector.
 *        past_r2q[i]       Past quantized residual (2nd vector).
 *
 *   The residual vectors r1[i] and r2[i] are jointly quantized using
 *   split-MQ with 5 codebooks. Each 4th dimension w_submatrix contains 2
 *   elements from each residual vector. The 5 w_submatrices are as follows:
 *     {r1[0], r1[1], r2[0], r2[1]};  {r1[2], r1[3], r2[2], r2[3]};
 *     {r1[4], r1[5], r2[4], r2[5]};  {r1[6], r1[7], r2[6], r2[7]};
 *                    {r1[8], r1[9], r2[8], r2[9]};
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "sig_proc.h"

#include "cnst.h"
#include "dtx.h"

/* Locals functions */

void w_Lsf_wt (
    Word16 *lsf,       /* input : LSF vector                    */
     Word16 *wf2       /* output: square of weighting factors   */
);

Word16 Vq_w_subvec (     /* output: return quantization index     */
    Word16 *lsf_r1,    /* input : 1st LSF residual vector       */
    Word16 *lsf_r2,    /* input : and LSF residual vector       */
    const Word16 *dico,/* input : quantization codebook         */
    Word16 *wf1,       /* input : 1st LSF weighting factors     */
    Word16 *wf2,       /* input : 2nd LSF weighting factors     */
    Word16 dico_size   /* input : size of quantization codebook */
);
Word16 Vq_w_subvec_s (   /* output: return quantization index     */
    Word16 *lsf_r1,    /* input : 1st LSF residual vector       */
    Word16 *lsf_r2,    /* input : and LSF residual vector       */
    const Word16 *dico,/* input : quantization codebook         */
    Word16 *wf1,       /* input : 1st LSF weighting factors     */
    Word16 *wf2,       /* input : 2nd LSF weighting factors     */
    Word16 dico_size   /* input : size of quantization codebook */
);
/* M  ->order of linear w_prediction filter                      */
/* LSF_GAP  -> Minimum distance between LSF after quantization */
/*             50 Hz = 205                                     */
/* PRED_FAC -> Predcition factor                               */

#define M         10
#define LSF_GAP   205
#define PRED_FAC  21299

#include "q_plsf_5.tab"         /* Codebooks of LSF w_prediction residual */

 /* Past quantized w_prediction w_error */

Word16 w_past_r2_q[M];

extern Word16 w_lsf_old_tx[DTX_HANGOVER][M];

void w_Q_plsf_5 (
    Word16 *lsp1,      /* input : 1st LSP vector                     */
    Word16 *lsp2,      /* input : 2nd LSP vector                     */
    Word16 *lsp1_q,    /* output: quantized 1st LSP vector           */
    Word16 *lsp2_q,    /* output: quantized 2nd LSP vector           */
    Word16 *indice,    /* output: quantization indices of 5 matrices */
    Word16 w_txdtx_ctrl  /* input : tx dtx control word                */
)
{
    Word16 i;
    Word16 lsf1[M], lsf2[M], wf1[M], wf2[M], lsf_p[M], lsf_r1[M], lsf_r2[M];
    Word16 lsf1_q[M], lsf2_q[M];
    Word16 lsf_aver[M];
    static Word16 w_lsf_p_CN[M];

    /* convert LSFs to normalize frequency domain 0..16384  */

    w_Lsp_lsf (lsp1, lsf1, M);
    w_Lsp_lsf (lsp2, lsf2, M);

    /* Update LSF CN quantizer "memory" */

            
    if ((w_txdtx_ctrl & TX_SP_FLAG) == 0
        && (w_txdtx_ctrl & TX_PREV_HANGOVER_ACTIVE) != 0)
    {
        update_w_lsf_p_CN (w_lsf_old_tx, w_lsf_p_CN);
    }
        
    if ((w_txdtx_ctrl & TX_SID_UPDATE) != 0)
    {
        /* New SID frame is to be sent:
        Compute average of the current LSFs and the LSFs in the history */

        w_aver_lsf_history (w_lsf_old_tx, lsf1, lsf2, lsf_aver);
    }
    /* Update LSF history with unquantized LSFs when no w_speech activity
    is present */

        
    if ((w_txdtx_ctrl & TX_SP_FLAG) == 0)
    {
        w_update_lsf_history (lsf1, lsf2, w_lsf_old_tx);
    }
        
    if ((w_txdtx_ctrl & TX_SID_UPDATE) != 0)
    {
        /* Compute LSF weighting factors for lsf2, using averaged LSFs */
        /* Set LSF weighting factors for lsf1 to w_zero */
        /* Replace lsf1 and lsf2 by the averaged LSFs */

        w_Lsf_wt (lsf_aver, wf2);
        for (i = 0; i < M; i++)
        {
            wf1[i] = 0;                                   
            lsf1[i] = lsf_aver[i];                        
            lsf2[i] = lsf_aver[i];                        
        }
    }
    else
    {
        /* Compute LSF weighting factors */

        w_Lsf_wt (lsf1, wf1);
        w_Lsf_wt (lsf2, wf2);
    }

    /* Compute w_predicted LSF and w_prediction w_error */

        
    if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
    {
        for (i = 0; i < M; i++)
        {
            lsf_p[i] = w_add (w_mean_lsf[i], w_mult (w_past_r2_q[i], PRED_FAC));
                                                          
            lsf_r1[i] = w_sub (lsf1[i], lsf_p[i]);          
            lsf_r2[i] = w_sub (lsf2[i], lsf_p[i]);          
        }
    }
    else
    {
        for (i = 0; i < M; i++)
        {
            lsf_r1[i] = w_sub (lsf1[i], w_lsf_p_CN[i]);       
            lsf_r2[i] = w_sub (lsf2[i], w_lsf_p_CN[i]);       
        }
    }

    /*---- Split-VQ of w_prediction w_error ----*/

    indice[0] = Vq_w_subvec (&lsf_r1[0], &lsf_r2[0], w_dico1_lsf,
                           &wf1[0], &wf2[0], DICO1_SIZE);
                                                          

    indice[1] = Vq_w_subvec (&lsf_r1[2], &lsf_r2[2], w_dico2_lsf,
                           &wf1[2], &wf2[2], DICO2_SIZE);
                                                          

    indice[2] = Vq_w_subvec_s (&lsf_r1[4], &lsf_r2[4], w_dico3_lsf,
                             &wf1[4], &wf2[4], DICO3_SIZE);
                                                          

    indice[3] = Vq_w_subvec (&lsf_r1[6], &lsf_r2[6], w_dico4_lsf,
                           &wf1[6], &wf2[6], DICO4_SIZE);
                                                          

    indice[4] = Vq_w_subvec (&lsf_r1[8], &lsf_r2[8], w_dico5_lsf,
                           &wf1[8], &wf2[8], DICO5_SIZE);
                                                          

    /* Compute quantized LSFs and update the past quantized residual */
    /* In case of no w_speech activity, skip computing the quantized LSFs,
       and set w_past_r2_q to w_zero (initial value) */

        
    if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
    {
        for (i = 0; i < M; i++)
        {
            lsf1_q[i] = w_add (lsf_r1[i], lsf_p[i]);        
            lsf2_q[i] = w_add (lsf_r2[i], lsf_p[i]);        
            w_past_r2_q[i] = lsf_r2[i];                     
        }

        /* verification that LSFs has minimum distance of LSF_GAP */

        w_Reorder_lsf (lsf1_q, LSF_GAP, M);
        w_Reorder_lsf (lsf2_q, LSF_GAP, M);

        /* Update LSF history with quantized LSFs
        when hangover period is active */

            
        if ((w_txdtx_ctrl & TX_HANGOVER_ACTIVE) != 0)
        {
            w_update_lsf_history (lsf1_q, lsf2_q, w_lsf_old_tx);
        }
        /*  convert LSFs to the cosine domain */

        w_Lsf_lsp (lsf1_q, lsp1_q, M);
        w_Lsf_lsp (lsf2_q, lsp2_q, M);
    }
    else
    {
        for (i = 0; i < M; i++)
        {
            w_past_r2_q[i] = 0;                             
        }
    }

    return;
}

/* Quantization of a 4 dimensional w_subvector */

Word16 Vq_w_subvec (      /* output: return quantization index     */
    Word16 *lsf_r1,     /* input : 1st LSF residual vector       */
    Word16 *lsf_r2,     /* input : and LSF residual vector       */
    const Word16 *dico, /* input : quantization codebook         */
    Word16 *wf1,        /* input : 1st LSF weighting factors     */
    Word16 *wf2,        /* input : 2nd LSF weighting factors     */
    Word16 dico_size    /* input : size of quantization codebook */
)
{
    Word16 i, index = 0, temp = 0;
    const Word16 *p_dico;
    Word32 dist_min, dist;

    dist_min = MAX_32;                                    
    p_dico = dico;                                        

    for (i = 0; i < dico_size; i++)
    {
        temp = w_sub (lsf_r1[0], *p_dico++);
        temp = w_mult (wf1[0], temp);
        dist = w_L_w_mult (temp, temp);

        temp = w_sub (lsf_r1[1], *p_dico++);
        temp = w_mult (wf1[1], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_sub (lsf_r2[0], *p_dico++);
        temp = w_mult (wf2[0], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_sub (lsf_r2[1], *p_dico++);
        temp = w_mult (wf2[1], temp);
        dist = w_L_mac (dist, temp, temp);

          
        if (w_L_w_sub (dist, dist_min) < (Word32) 0)
        {
            dist_min = dist;                              
            index = i;                                    
        }
    }

    /* Reading the selected vector */

    p_dico = &dico[w_shl (index, 2)];                       
    lsf_r1[0] = *p_dico++;                                
    lsf_r1[1] = *p_dico++;                                
    lsf_r2[0] = *p_dico++;                                
    lsf_r2[1] = *p_dico++;                                

    return index;

}

/* Quantization of a 4 dimensional w_subvector with a signed codebook */

Word16 Vq_w_subvec_s (    /* output: return quantization index     */
    Word16 *lsf_r1,     /* input : 1st LSF residual vector       */
    Word16 *lsf_r2,     /* input : and LSF residual vector       */
    const Word16 *dico, /* input : quantization codebook         */
    Word16 *wf1,        /* input : 1st LSF weighting factors     */
    Word16 *wf2,        /* input : 2nd LSF weighting factors     */
    Word16 dico_size)   /* input : size of quantization codebook */
{
    Word16 i, index = 0, sign = 0, temp = 0;
    const Word16 *p_dico;
    Word32 dist_min, dist;

    dist_min = MAX_32;                                    
    p_dico = dico;                                        

    for (i = 0; i < dico_size; i++)
    {
        /* w_test positive */

        temp = w_sub (lsf_r1[0], *p_dico++);
        temp = w_mult (wf1[0], temp);
        dist = w_L_w_mult (temp, temp);

        temp = w_sub (lsf_r1[1], *p_dico++);
        temp = w_mult (wf1[1], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_sub (lsf_r2[0], *p_dico++);
        temp = w_mult (wf2[0], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_sub (lsf_r2[1], *p_dico++);
        temp = w_mult (wf2[1], temp);
        dist = w_L_mac (dist, temp, temp);

          
        if (w_L_w_sub (dist, dist_min) < (Word32) 0)
        {
            dist_min = dist;                              
            index = i;                                    
            sign = 0;                                     
        }
        /* w_test negative */

        p_dico -= 4;                                      
        temp = w_add (lsf_r1[0], *p_dico++);
        temp = w_mult (wf1[0], temp);
        dist = w_L_w_mult (temp, temp);

        temp = w_add (lsf_r1[1], *p_dico++);
        temp = w_mult (wf1[1], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_add (lsf_r2[0], *p_dico++);
        temp = w_mult (wf2[0], temp);
        dist = w_L_mac (dist, temp, temp);

        temp = w_add (lsf_r2[1], *p_dico++);
        temp = w_mult (wf2[1], temp);
        dist = w_L_mac (dist, temp, temp);

          
        if (w_L_w_sub (dist, dist_min) < (Word32) 0)
        {
            dist_min = dist;                              
            index = i;                                    
            sign = 1;                                     
        }
    }

    /* Reading the selected vector */

    p_dico = &dico[w_shl (index, 2)];                       
      
    if (sign == 0)
    {
        lsf_r1[0] = *p_dico++;                            
        lsf_r1[1] = *p_dico++;                            
        lsf_r2[0] = *p_dico++;                            
        lsf_r2[1] = *p_dico++;                            
    }
    else
    {
        lsf_r1[0] = w_negate (*p_dico++);                   
        lsf_r1[1] = w_negate (*p_dico++);                   
        lsf_r2[0] = w_negate (*p_dico++);                   
        lsf_r2[1] = w_negate (*p_dico++);                   
    }

    index = w_shl (index, 1);
    index = w_add (index, sign);

    return index;

}

/****************************************************
 * FUNCTION  w_Lsf_wt                                                         *
 *                                                                          *
 ****************************************************
 * Compute LSF weighting factors                                            *
 *                                                                          *
 *  d[i] = lsf[i+1] - lsf[i-1]                                              *
 *                                                                          *
 *  The weighting factors are approximated by two line segment.             *
 *                                                                          *
 *  First segment passes by the following 2 points:                         *
 *                                                                          *
 *     d[i] = 0Hz     wf[i] = 3.347                                         *
 *     d[i] = 450Hz   wf[i] = 1.8                                           *
 *                                                                          *
 *  Second segment passes by the following 2 points:                        *
 *                                                                          *
 *     d[i] = 450Hz   wf[i] = 1.8                                           *
 *     d[i] = 1500Hz  wf[i] = 1.0                                           *
 *                                                                          *
 *  if( d[i] < 450Hz )                                                      *
 *    wf[i] = 3.347 - ( (3.347-1.8) / (450-0)) *  d[i]                      *
 *  else                                                                    *
 *    wf[i] = 1.8 - ( (1.8-1.0) / (1500-450)) *  (d[i] - 450)               *
 *                                                                          *
 *                                                                          *
 *  if( d[i] < 1843)                                                        *
 *    wf[i] = 3427 - (28160*d[i])>>15                                       *
 *  else                                                                    *
 *    wf[i] = 1843 - (6242*(d[i]-1843))>>15                                 *
 *                                                                          *
 *--------------------------------------------------------------------------*/

void w_Lsf_wt (
    Word16 *lsf,         /* input : LSF vector                  */
    Word16 *wf)          /* output: square of weighting factors */
{
    Word16 temp;
    Word16 i;
    /* wf[0] = lsf[1] - 0  */
    wf[0] = lsf[1];                                       
    for (i = 1; i < 9; i++)
    {
        wf[i] = w_sub (lsf[i + 1], lsf[i - 1]);             
    }
    /* wf[9] = 0.5 - lsf[8] */    
    wf[9] = w_sub (16384, lsf[8]);       

    for (i = 0; i < 10; i++)
    {
        temp = w_sub (wf[i], 1843);
          
        if (temp < 0)
        {
            wf[i] = w_sub (3427, w_mult (wf[i], 28160));      
        }
        else
        {
            wf[i] = w_sub (1843, w_mult (temp, 6242));        
        }

        wf[i] = w_shl (wf[i], 3);   
    }
    return;
}
