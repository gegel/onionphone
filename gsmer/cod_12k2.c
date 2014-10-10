/***************************************************************************
 *
 *  FILE NAME:    cod_12k2.c
 *
 *  FUNCTIONS DEFINED IN THIS FILE:
 *                   w_Coder_12k2  and  Init_w_Coder_12k2
 *
 *
 *  Init_w_Coder_12k2(void):
 *      Initialization of variables for the coder section.
 *
 *  w_Coder_12k2(Word16 ana[], Word16 w_w_synth[]):
 *      Speech encoder routine operating on a frame basis.
 *

***************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "sig_proc.h"
#include "count.h"
#include "codec.h"
#include "cnst.h"

#include "window2.tab"

#include "vad.h"
#include "dtx.h"

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "cnst.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size                  *
 *   L_FRAME     : Frame size                                *
 *   L_FRAME_BY2 : Half the frame size                       *
 *   L_SUBFR     : Sub-frame size                            *
 *   M           : LPC order                                 *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of w_speech buffer               *
 *   PIT_MIN     : Minimum pitch lag                         *
 *   PIT_MAX     : Maximum pitch lag                         *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

 /* Speech vector */

static Word16 w_old_w_speech[L_TOTAL];
static Word16 *w_speech, *w_p_window, *w_w_p_window_mid;
Word16 *w_new_w_speech;             /* Global variable */

 /* Weight w_speech vector */

static Word16 w_old_w_wsp[L_FRAME + PIT_MAX];
static Word16 *w_wsp;

 /* Excitation vector */

static Word16 old_w_exc[L_FRAME + PIT_MAX + L_INTERPOL];
static Word16 *w_exc;

 /* Zero vector */

static Word16 w_ai_w_zero[L_SUBFR + MP1];
static Word16 *w_zero;

 /* Impulse response vector */

static Word16 *w_h1;
static Word16 w_hvec[L_SUBFR * 2];

 /* Spectral expansion factors */

static const Word16 w_F_gamma1[M] =
{
    29491, 26542, 23888, 21499, 19349,
    17414, 15672, 14105, 12694, 11425
};
static const Word16 w_F_gamma2[M] =
{
    19661, 11797, 7078, 4247, 2548,
    1529, 917, 550, 330, 198
};

 /* Lsp (Line spectral pairs) */

static Word16 w_lsp_old[M];
static Word16 w_w_lsp_old_q[M];

 /* Filter's memory */

static Word16 w_mem_w_syn[M], w_w_mem_w0[M], w_mem_w[M];
static Word16 w_mem_err[M + L_SUBFR], *w_error;

/***************************************************************************
 *  FUNCTION:   Init_w_Coder_12k2
 *
 *  PURPOSE:   Initialization of variables for the coder section.
 *
 *  DESCRIPTION:
 *       - initilize pointers to w_speech buffer
 *       - initialize static  pointers
 *       - set static vectors to w_zero
 *
 ***************************************************************************/

void Init_w_Coder_12k2 (void)
{

/*--------------------------------------------------------------------------*
 *          Initialize pointers to w_speech vector.                           *
 *--------------------------------------------------------------------------*/

    w_new_w_speech = w_old_w_speech + L_TOTAL - L_FRAME;/* New w_speech     */
    w_speech = w_new_w_speech;                        /* Present frame  */
    w_p_window = w_old_w_speech + L_TOTAL - L_WINDOW; /* For LPC window */
    w_w_p_window_mid = w_p_window;                    /* For LPC window */

    /* Initialize static pointers */

    w_wsp = w_old_w_wsp + PIT_MAX;
    w_exc = old_w_exc + PIT_MAX + L_INTERPOL;
    w_zero = w_ai_w_zero + MP1;
    w_error = w_mem_err + M;
    w_h1 = &w_hvec[L_SUBFR];

    /* Static vectors to w_zero */

    w_Set_w_zero (w_old_w_speech, L_TOTAL);
    w_Set_w_zero (old_w_exc, PIT_MAX + L_INTERPOL);
    w_Set_w_zero (w_old_w_wsp, PIT_MAX);
    w_Set_w_zero (w_mem_w_syn, M);
    w_Set_w_zero (w_mem_w, M);
    w_Set_w_zero (w_w_mem_w0, M);
    w_Set_w_zero (w_mem_err, M);
    w_Set_w_zero (w_zero, L_SUBFR);
    w_Set_w_zero (w_hvec, L_SUBFR);   /* set to w_zero "w_h1[-L_SUBFR..-1]" */

    /* Initialize w_lsp_old [] */

    w_lsp_old[0] = 30000;
    w_lsp_old[1] = 26000;
    w_lsp_old[2] = 21000;
    w_lsp_old[3] = 15000;
    w_lsp_old[4] = 8000;
    w_lsp_old[5] = 0;
    w_lsp_old[6] = -8000;
    w_lsp_old[7] = -15000;
    w_lsp_old[8] = -21000;
    w_lsp_old[9] = -26000;

    /* Initialize w_w_lsp_old_q[] */

    w_Copy (w_lsp_old, w_w_lsp_old_q, M);

    return;
}

/***************************************************************************
 *   FUNCTION:   w_Coder_12k2
 *
 *   PURPOSE:  Principle encoder routine.
 *
 *   DESCRIPTION: This function is called every 20 ms w_speech frame,
 *       operating on the newly read 160 w_speech samples. It performs the
 *       principle encoding functions to produce the set of encoded parameters
 *       which include the LSP, adaptive codebook, and fixed codebook
 *       quantization indices (w_addresses and gains).
 *
 *   INPUTS:
 *       No input arguments are passed to this function. However, before
 *       calling this function, 160 new w_speech data samples should be copied to
 *       the vector w_new_w_speech[]. This is a global pointer which is declared in
 *       this file (it points to the end of w_speech buffer minus 160).
 *
 *   OUTPUTS:
 *
 *       ana[]:     vector of analysis parameters.
 *       w_w_synth[]:   Local w_w_synthesis w_speech (for debugging purposes)
 *
 ***************************************************************************/

void w_Coder_12k2 (
    Word16 ana[],    /* output  : Analysis parameters */
    Word16 w_w_synth[]   /* output  : Local w_w_synthesis     */
)
{
    /* LPC coefficients */

    Word16 r_l[MP1], r_h[MP1];      /* w_Autocorrelations lo and hi           */
    Word16 A_t[(MP1) * 4];          /* A(z) unquantized for the 4 w_subframes */
    Word16 Aq_t[(MP1) * 4];         /* A(z)   quantized for the 4 w_subframes */
    Word16 Ap1[MP1];                /* A(z) with spectral expansion         */
    Word16 Ap2[MP1];                /* A(z) with spectral expansion         */
    Word16 *A, *Aq;                 /* Pointer on A_t and Aq_t              */
    Word16 lsp_new[M], lsp_new_q[M];/* LSPs at 4th w_subframe                 */
    Word16 lsp_mid[M], lsp_mid_q[M];/* LSPs at 2nd w_subframe                 */

    /* Other vectors */

    Word16 xn[L_SUBFR];            /* Target vector for pitch search        */
    Word16 xn2[L_SUBFR];           /* Target vector for codebook search     */
    Word16 w_res2[L_SUBFR];          /* Long term w_prediction residual         */
    Word16 code[L_SUBFR];          /* Fixed codebook w_excitation             */
    Word16 y1[L_SUBFR];            /* Filtered adaptive w_excitation          */
    Word16 y2[L_SUBFR];            /* Filtered fixed codebook w_excitation    */

    /* Scalars */

    Word16 i, j, k, i_w_subfr;
    Word16 T_op, T0 = 0, T0_min = 0, T0_max = 0, T0_frac = 0;
    Word16 gain_pit, gain_code, pit_flag, pit_sharp = 0;
    Word16 temp;
    Word32 L_temp;

    Word16 scal_acf, VAD_flag, lags[2], rc[4];

    extern Word16 w_ptch;
    extern Word16 w_txdtx_ctrl, w_CN_w_excitation_gain;
    extern Word32 w_L_pn_seed_tx;
    extern Word16 w_dtx_mode;

    /*----------------------------------------------------------------------*
     *  - Perform LPC analysis: (twice per frame)                           *
     *       * autocorrelation + lag windowing                              *
     *       * w_er_Levinson-Durbin algorithm to find a[]                        *
     *       * convert a[] to lsp[]                                         *
     *       * quantize and code the LSPs                                   *
     *       * find the interpolated LSPs and convert to a[] for all        *
     *         w_subframes (both quantized and unquantized)                   *
     *----------------------------------------------------------------------*/

    /* LP analysis centered at 2nd w_subframe */


    scal_acf = w_Autocorr (w_w_p_window_mid, M, r_h, r_l, w_window_160_80);
                                /* w_Autocorrelations */

#if (WMOPS)
                          /* function worst case */
#endif

    w_Lag_window (M, r_h, r_l);   /* Lag windowing    */

#if (WMOPS)
                          /* function worst case */
#endif

    w_er_Levinson (r_h, r_l, &A_t[MP1], rc); /* w_er_Levinson-Durbin  */

#if (WMOPS)
                          /* function worst case */
#endif

    w_Az_lsp (&A_t[MP1], lsp_mid, w_lsp_old); /* From A(z) to lsp */

#if (WMOPS)
                          /* function worst case */
#endif

    /* LP analysis centered at 4th w_subframe */

    /* w_Autocorrelations */
    scal_acf = w_Autocorr (w_p_window, M, r_h, r_l, w_window_232_8);

#if (WMOPS)
                          /* function worst case */
#endif

    w_Lag_window (M, r_h, r_l);   /* Lag windowing    */

#if (WMOPS)
                          /* function worst case */
#endif

    w_er_Levinson (r_h, r_l, &A_t[MP1 * 3], rc); /* w_er_Levinson-Durbin  */

#if (WMOPS)
                          /* function worst case */
#endif

    w_Az_lsp (&A_t[MP1 * 3], lsp_new, lsp_mid); /* From A(z) to lsp */

#if (WMOPS)
                          /* function worst case */
#endif

    if (w_dtx_mode == 1)
    {
        /* DTX enabled, make voice activity decision */
        VAD_flag = w_vad_computation (r_h, r_l, scal_acf, rc, w_ptch);
                                                                  

        w_tx_dtx (VAD_flag, &w_txdtx_ctrl); /* TX DTX handler */
    }
    else
    {
        /* DTX disabled, active w_speech in every frame */
        VAD_flag = 1;
        w_txdtx_ctrl = TX_VAD_FLAG | TX_SP_FLAG;
    }

    /* LSP quantization (lsp_mid[] and lsp_new[] jointly quantized) */

    w_Q_plsf_5 (lsp_mid, lsp_new, lsp_mid_q, lsp_new_q, ana, w_txdtx_ctrl);

#if (WMOPS)
                          /* function worst case */
#endif
    ana += 5;                                                     

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all w_subframes (both quantized  *
     * and unquantized).                                                  *
     * The interpolated parameters are in array A_t[] of size (M+1)*4     *
     * and the quantized interpolated parameters are in array Aq_t[]      *
     *--------------------------------------------------------------------*/

    w_w_Int_lpc2 (w_lsp_old, lsp_mid, lsp_new, A_t);

#if (WMOPS)
                          /* function worst case */
#endif

        
    if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
    {
        w_Int_lpc (w_w_lsp_old_q, lsp_mid_q, lsp_new_q, Aq_t);

        /* update the LSPs for the next frame */
        for (i = 0; i < M; i++)
        {
            w_lsp_old[i] = lsp_new[i];                              
            w_w_lsp_old_q[i] = lsp_new_q[i];                          
        }
    }
    else
    {
        /* Use unquantized LPC parameters in case of no w_speech activity */
        for (i = 0; i < MP1; i++)
        {
            Aq_t[i] = A_t[i];                                     
            Aq_t[i + MP1] = A_t[i + MP1];                         
            Aq_t[i + MP1 * 2] = A_t[i + MP1 * 2];                 
            Aq_t[i + MP1 * 3] = A_t[i + MP1 * 3];                 
        }

        /* update the LSPs for the next frame */
        for (i = 0; i < M; i++)
        {
            w_lsp_old[i] = lsp_new[i];                              
            w_w_lsp_old_q[i] = lsp_new[i];                            
        }
    }

#if (WMOPS)
                          /* function worst case */
#endif

    /*----------------------------------------------------------------------*
     * - Find the weighted input w_speech w_wsp[] for the whole w_speech frame    *
     * - Find the open-loop pitch delay for first 2 w_subframes               *
     * - Set the range for searching closed-loop pitch in 1st w_subframe      *
     * - Find the open-loop pitch delay for last 2 w_subframes                *
     *----------------------------------------------------------------------*/

    A = A_t;                                                      
    for (i = 0; i < L_FRAME; i += L_SUBFR)
    {
        w_Weight_Ai (A, w_F_gamma1, Ap1);

#if (WMOPS)
                          /* function worst case */
#endif

        w_Weight_Ai (A, w_F_gamma2, Ap2);

#if (WMOPS)
                          /* function worst case */
#endif

        w_Residu (Ap1, &w_speech[i], &w_wsp[i], L_SUBFR);

#if (WMOPS)
                          /* function worst case */
#endif

        w_Syn_filt (Ap2, &w_wsp[i], &w_wsp[i], L_SUBFR, w_mem_w, 1);

#if (WMOPS)
                          /* function worst case */
#endif

        A += MP1;                                                 
    }

    /* Find open loop pitch lag for first two w_subframes */

    T_op = w_Pitch_ol (w_wsp, PIT_MIN, PIT_MAX, L_FRAME_BY2);         

#if (WMOPS)
                          /* function worst case */
#endif

    lags[0] = T_op;                                               

        
    if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
    {
        /* Range for closed loop pitch search in 1st w_subframe */

        T0_min = w_sub (T_op, 3);
          
        if (w_sub (T0_min, PIT_MIN) < 0)
        {
            T0_min = PIT_MIN;                                     
        }
        T0_max = w_add (T0_min, 6);
          
        if (w_sub (T0_max, PIT_MAX) > 0)
        {
            T0_max = PIT_MAX;                                     
            T0_min = w_sub (T0_max, 6);
        }
#if (WMOPS)
                          /* function worst case */
#endif
    }
    /* Find open loop pitch lag for last two w_subframes */

    T_op = w_Pitch_ol (&w_wsp[L_FRAME_BY2], PIT_MIN, PIT_MAX, L_FRAME_BY2);
                                                                  

#if (WMOPS)
                          /* function worst case */
#endif

    if (w_dtx_mode == 1)
    {
        lags[1] = T_op;                                           
        w_er_periodicity_update (lags, &w_ptch);
    }
    /*----------------------------------------------------------------------*
     *          Loop for every w_subframe in the analysis frame               *
     *----------------------------------------------------------------------*
     *  To find the pitch and innovation parameters. The w_subframe size is   *
     *  L_SUBFR and the loop is repeated L_FRAME/L_SUBFR times.             *
     *     - find the weighted LPC coefficients                             *
     *     - find the LPC residual signal res[]                             *
     *     - compute the target signal for pitch search                     *
     *     - compute impulse response of weighted w_w_synthesis filter (w_h1[])   *
     *     - find the closed-loop pitch parameters                          *
     *     - encode the pitch delay                                         *
     *     - update the impulse response w_h1[] by including pitch            *
     *     - find target vector for codebook search                         *
     *     - codebook search                                                *
     *     - encode codebook w_address                                        *
     *     - VQ of pitch and codebook gains                                 *
     *     - find w_w_synthesis w_speech                                          *
     *     - update w_states of weighting filter                              *
     *----------------------------------------------------------------------*/

    /* pointer to interpolated LPC parameters          */
    A = A_t;                                                     
    /* pointer to interpolated quantized LPC parameters */    
    Aq = Aq_t;                                                    

    for (i_w_subfr = 0; i_w_subfr < L_FRAME; i_w_subfr += L_SUBFR)
    {

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {

            /*---------------------------------------------------------------*
             * Find the weighted LPC coefficients for the weighting filter.  *
             *---------------------------------------------------------------*/

            w_Weight_Ai (A, w_F_gamma1, Ap1);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Weight_Ai (A, w_F_gamma2, Ap2);

#if (WMOPS)
                          /* function worst case */
#endif

            /*---------------------------------------------------------------*
             * Compute impulse response, w_h1[], of weighted w_w_synthesis filter  *
             *---------------------------------------------------------------*/

            for (i = 0; i <= M; i++)
            {
                w_ai_w_zero[i] = Ap1[i];                              
            }

            w_Syn_filt (Aq, w_ai_w_zero, w_h1, L_SUBFR, w_zero, 0);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Syn_filt (Ap2, w_h1, w_h1, L_SUBFR, w_zero, 0);

#if (WMOPS)
                          /* function worst case */
#endif

        }
        /*---------------------------------------------------------------*
         *          Find the target vector for pitch search:             *
         *---------------------------------------------------------------*/

        w_Residu (Aq, &w_speech[i_w_subfr], w_res2, L_SUBFR);   /* LPC residual */

#if (WMOPS)
                          /* function worst case */
#endif

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) == 0)
        {
            /* Compute comfort noise w_excitation gain based on
            LP residual energy */

            w_CN_w_excitation_gain = compute_w_CN_w_excitation_gain (w_res2);
              
        }
        else
        {
            w_Copy (w_res2, &w_exc[i_w_subfr], L_SUBFR);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Syn_filt (Aq, &w_exc[i_w_subfr], w_error, L_SUBFR, w_mem_err, 0);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Residu (Ap1, w_error, xn, L_SUBFR);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Syn_filt (Ap2, xn, xn, L_SUBFR, w_w_mem_w0, 0); /* target signal xn[]*/

#if (WMOPS)
                          /* function worst case */
#endif

            /*--------------------------------------------------------------*
             *                 Closed-loop fractional pitch search          *
             *--------------------------------------------------------------*/

            /* flag for first and 3th w_subframe */            
            pit_flag = i_w_subfr;                                   
              
            /* set t0_min and t0_max for 3th w_subf.*/
            if (w_sub (i_w_subfr, L_FRAME_BY2) == 0)
            {
                T0_min = w_sub (T_op, 3);

                  
                if (w_sub (T0_min, PIT_MIN) < 0)
                {
                    T0_min = PIT_MIN;                             
                }
                T0_max = w_add (T0_min, 6);
                  
                if (w_sub (T0_max, PIT_MAX) > 0)
                {
                    T0_max = PIT_MAX;                             
                    T0_min = w_sub (T0_max, 6);
                }
                pit_flag = 0;                                     
            }
#if (WMOPS)
                          /* function worst case */
#endif

            T0 = w_Pitch_fr6 (&w_exc[i_w_subfr], xn, w_h1, L_SUBFR, T0_min, T0_max,
                            pit_flag, &T0_frac);                  

#if (WMOPS)
                          /* function worst case */
#endif

            *ana = w_Enc_lag6 (T0, &T0_frac, &T0_min, &T0_max, PIT_MIN,
                             PIT_MAX, pit_flag);
              

#if (WMOPS)
                          /* function worst case */
#endif
        }
        ana++;
        /* Incrementation of ana is done here to work also
        when no w_speech activity is present */

            

        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {

            /*---------------------------------------------------------------*
             * - find unity gain pitch w_excitation (adaptive codebook entry)  *
             *   with fractional interpolation.                              *
             * - find filtered pitch w_exc. y1[]=w_exc[] convolved with w_h1[]     *
             * - compute pitch gain and limit between 0 and 1.2              *
             * - update target vector for codebook search                    *
             * - find LTP residual.                                          *
             *---------------------------------------------------------------*/

            w_Pred_lt_6 (&w_exc[i_w_subfr], T0, T0_frac, L_SUBFR);

#if (WMOPS)
                          /* function worst case */
#endif

            w_Convolve (&w_exc[i_w_subfr], w_h1, y1, L_SUBFR);

#if (WMOPS)
                          /* function worst case */
#endif

            gain_pit = w_G_pitch (xn, y1, L_SUBFR);        

#if (WMOPS)
                          /* function worst case */
#endif

            *ana = w_q_gain_pitch (&gain_pit);                      

#if (WMOPS)
                          /* function worst case */
#endif

        }
        else
        {
            gain_pit = 0;                                         
        }

        ana++;                  /* Incrementation of ana is done here to work
                                   also when no w_speech activity is present */

            

        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {
            /* xn2[i]   = xn[i] - y1[i] * gain_pit  */
            /* w_res2[i] -= w_exc[i+i_w_subfr] * gain_pit */

            for (i = 0; i < L_SUBFR; i++)
            {
                L_temp = w_L_w_mult (y1[i], gain_pit);
                L_temp = w_L_w_shl (L_temp, 3);
                xn2[i] = w_sub (xn[i], w_extract_h (L_temp));         

                L_temp = w_L_w_mult (w_exc[i + i_w_subfr], gain_pit);
                L_temp = w_L_w_shl (L_temp, 3);
                w_res2[i] = w_sub (w_res2[i], w_extract_h (L_temp));      
            }

#if (WMOPS)
                          /* function worst case */
#endif

            /*-------------------------------------------------------------*
             * - include pitch contribution into impulse resp. w_h1[]        *
             *-------------------------------------------------------------*/

            /* pit_sharp = gain_pit;                   */
            /* if (pit_sharp > 1.0) pit_sharp = 1.0;   */

            pit_sharp = w_shl (gain_pit, 3);

            for (i = T0; i < L_SUBFR; i++)
            {
                temp = w_mult (w_h1[i - T0], pit_sharp);
                w_h1[i] = w_add (w_h1[i], temp);                        
            }

#if (WMOPS)
                          /* function worst case */
#endif

            /*--------------------------------------------------------------*
             * - Innovative codebook search (find index and gain)           *
             *--------------------------------------------------------------*/

            w_code_10i40_35bits (xn2, w_res2, w_h1, code, y2, ana);

#if (WMOPS)
                          /* function worst case */
#endif

        }
        else
        {
            w_build_CN_code (code, &w_L_pn_seed_tx);
        }
        ana += 10;                                                

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {

            /*-------------------------------------------------------*
             * - Add the pitch contribution to code[].               *
             *-------------------------------------------------------*/

            for (i = T0; i < L_SUBFR; i++)
            {
                temp = w_mult (code[i - T0], pit_sharp);
                code[i] = w_add (code[i], temp);                    
            }

#if (WMOPS)
                          /* function worst case */
#endif

            /*------------------------------------------------------*
             * - Quantization of fixed codebook gain.               *
             *------------------------------------------------------*/

            gain_code = w_G_code (xn2, y2);                         

#if (WMOPS)
                          /* function worst case */
#endif

        }
        *ana++ = w_q_gain_code (code, L_SUBFR, &gain_code, w_txdtx_ctrl, i_w_subfr);
          

#if (WMOPS)
                          /* function worst case */
#endif

        /*------------------------------------------------------*
         * - Find the total w_excitation                          *
         * - find w_w_synthesis w_speech corresponding to w_exc[]       *
         * - update filter memories for finding the target      *
         *   vector in the next w_subframe                        *
         *   (update w_mem_err[] and w_w_mem_w0[])                    *
         *------------------------------------------------------*/

        for (i = 0; i < L_SUBFR; i++)
        {
            /* w_exc[i] = gain_pit*w_exc[i] + gain_code*code[i]; */

            L_temp = w_L_w_mult (w_exc[i + i_w_subfr], gain_pit);
            L_temp = w_L_mac (L_temp, code[i], gain_code);
            L_temp = w_L_w_shl (L_temp, 3);
            w_exc[i + i_w_subfr] = w_round (L_temp);                    
        }

#if (WMOPS)
                          /* function worst case */
#endif

        w_Syn_filt (Aq, &w_exc[i_w_subfr], &w_w_synth[i_w_subfr], L_SUBFR, w_mem_w_syn, 1);

#if (WMOPS)
                          /* function worst case */
#endif

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {

            for (i = L_SUBFR - M, j = 0; i < L_SUBFR; i++, j++)
            {
                w_mem_err[j] = w_sub (w_speech[i_w_subfr + i], w_w_synth[i_w_subfr + i]);
                                                                  
                temp = w_extract_h (w_L_w_shl (w_L_w_mult (y1[i], gain_pit), 3));
                k = w_extract_h (w_L_w_shl (w_L_w_mult (y2[i], gain_code), 5));
                w_w_mem_w0[j] = w_sub (xn[i], w_add (temp, k));           
            }
        }
        else
        {
            for (j = 0; j < M; j++)
            {
                w_mem_err[j] = 0;                                   
                w_w_mem_w0[j] = 0;                                    
            }
        }

#if (WMOPS)
                          /* function worst case */
#endif
        /* interpolated LPC parameters for next w_subframe */
        A += MP1;                                                 
        Aq += MP1;                                                
    }

    /*--------------------------------------------------*
     * Update signal for next frame.                    *
     * -> shift to the left by L_FRAME:                 *
     *     w_speech[], w_wsp[] and  w_exc[]                   *
     *--------------------------------------------------*/

    w_Copy (&w_old_w_speech[L_FRAME], &w_old_w_speech[0], L_TOTAL - L_FRAME);

#if (WMOPS)
                          /* function worst case */
#endif

    w_Copy (&w_old_w_wsp[L_FRAME], &w_old_w_wsp[0], PIT_MAX);

#if (WMOPS)
                          /* function worst case */
#endif

    w_Copy (&old_w_exc[L_FRAME], &old_w_exc[0], PIT_MAX + L_INTERPOL);

#if (WMOPS)
                          /* function worst case */
#endif

    return;
}
