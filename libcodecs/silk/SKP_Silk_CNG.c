/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#include "SKP_Silk_main_FIX.h"

/* Generates excitation for CNG LPC synthesis */
SKP_INLINE void SKP_Silk_CNG_exc(
    int16_t                       residual[],         /* O    CNG residual signal Q0                      */
    int32_t                       exc_buf_Q10[],      /* I    Random samples buffer Q10                   */
    int32_t                       Gain_Q16,           /* I    Gain to apply                               */
    int                         length,             /* I    Length                                      */
    int32_t                       *rand_seed          /* I/O  Seed to random index generator              */
)
{
    int32_t seed;
    int   i, idx, exc_mask;

    exc_mask = CNG_BUF_MASK_MAX;
    while( exc_mask > length ) {
        exc_mask = SKP_RSHIFT( exc_mask, 1 );
    }

    seed = *rand_seed;
    for( i = 0; i < length; i++ ) {
        seed = SKP_RAND( seed );
        idx = ( int )( SKP_RSHIFT( seed, 24 ) & exc_mask );
        SKP_assert( idx >= 0 );
        SKP_assert( idx <= CNG_BUF_MASK_MAX );
        residual[ i ] = ( int16_t )SKP_SAT16( SKP_RSHIFT_ROUND( SKP_SMULWW( exc_buf_Q10[ idx ], Gain_Q16 ), 10 ) );
    }
    *rand_seed = seed;
}

void SKP_Silk_CNG_Reset(
    SKP_Silk_decoder_state      *psDec              /* I/O  Decoder state                               */
)
{
    int i, NLSF_step_Q15, NLSF_acc_Q15;

    NLSF_step_Q15 = SKP_DIV32_16( int16_t_MAX, psDec->LPC_order + 1 );
    NLSF_acc_Q15 = 0;
    for( i = 0; i < psDec->LPC_order; i++ ) {
        NLSF_acc_Q15 += NLSF_step_Q15;
        psDec->sCNG.CNG_smth_NLSF_Q15[ i ] = NLSF_acc_Q15;
    }
    psDec->sCNG.CNG_smth_Gain_Q16 = 0;
    psDec->sCNG.rand_seed = 3176576;
}

/* Updates CNG estimate, and applies the CNG when packet was lost   */
void SKP_Silk_CNG(
    SKP_Silk_decoder_state      *psDec,             /* I/O  Decoder state                               */
    SKP_Silk_decoder_control    *psDecCtrl,         /* I/O  Decoder control                             */
    int16_t                   signal[],           /* I/O  Signal                                      */
    int                     length              /* I    Length of residual                          */
)
{
    int   i, subfr;
    int32_t tmp_32, Gain_Q26, max_Gain_Q16;
    int16_t LPC_buf[ MAX_LPC_ORDER ];
    int16_t CNG_sig[ MAX_FRAME_LENGTH ];
    SKP_Silk_CNG_struct *psCNG;
    psCNG = &psDec->sCNG;

    if( psDec->fs_kHz != psCNG->fs_kHz ) {
        /* Reset state */
        SKP_Silk_CNG_Reset( psDec );

        psCNG->fs_kHz = psDec->fs_kHz;
    }
    if( psDec->lossCnt == 0 && psDec->vadFlag == NO_VOICE_ACTIVITY ) {
        /* Update CNG parameters */

        /* Smoothing of LSF's  */
        for( i = 0; i < psDec->LPC_order; i++ ) {
            psCNG->CNG_smth_NLSF_Q15[ i ] += SKP_SMULWB( psDec->prevNLSF_Q15[ i ] - psCNG->CNG_smth_NLSF_Q15[ i ], CNG_NLSF_SMTH_Q16 );
        }
        /* Find the subframe with the highest gain */
        max_Gain_Q16 = 0;
        subfr        = 0;
        for( i = 0; i < NB_SUBFR; i++ ) {
            if( psDecCtrl->Gains_Q16[ i ] > max_Gain_Q16 ) {
                max_Gain_Q16 = psDecCtrl->Gains_Q16[ i ];
                subfr        = i;
            }
        }
        /* Update CNG excitation buffer with excitation from this subframe */
        SKP_memmove( &psCNG->CNG_exc_buf_Q10[ psDec->subfr_length ], psCNG->CNG_exc_buf_Q10, ( NB_SUBFR - 1 ) * psDec->subfr_length * sizeof( int32_t ) );
        SKP_memcpy(   psCNG->CNG_exc_buf_Q10, &psDec->exc_Q10[ subfr * psDec->subfr_length ], psDec->subfr_length * sizeof( int32_t ) );

        /* Smooth gains */
        for( i = 0; i < NB_SUBFR; i++ ) {
            psCNG->CNG_smth_Gain_Q16 += SKP_SMULWB( psDecCtrl->Gains_Q16[ i ] - psCNG->CNG_smth_Gain_Q16, CNG_GAIN_SMTH_Q16 );
        }
    }

    /* Add CNG when packet is lost and / or when low speech activity */
    if( psDec->lossCnt ) {//|| psDec->vadFlag == NO_VOICE_ACTIVITY ) {

        /* Generate CNG excitation */
        SKP_Silk_CNG_exc( CNG_sig, psCNG->CNG_exc_buf_Q10, 
                psCNG->CNG_smth_Gain_Q16, length, &psCNG->rand_seed );

        /* Convert CNG NLSF to filter representation */
        SKP_Silk_NLSF2A_stable( LPC_buf, psCNG->CNG_smth_NLSF_Q15, psDec->LPC_order );

        Gain_Q26 = ( int32_t )1 << 26; /* 1.0 */
        
        /* Generate CNG signal, by synthesis filtering */
        if( psDec->LPC_order == 16 ) {
            SKP_Silk_LPC_synthesis_order16( CNG_sig, LPC_buf, 
                Gain_Q26, psCNG->CNG_synth_state, CNG_sig, length );
        } else {
            SKP_Silk_LPC_synthesis_filter( CNG_sig, LPC_buf, 
                Gain_Q26, psCNG->CNG_synth_state, CNG_sig, length, psDec->LPC_order );
        }
        /* Mix with signal */
        for( i = 0; i < length; i++ ) {
            tmp_32 = signal[ i ] + CNG_sig[ i ];
            signal[ i ] = SKP_SAT16( tmp_32 );
        }
    } else {
        SKP_memset( psCNG->CNG_synth_state, 0, psDec->LPC_order *  sizeof( int32_t ) );
    }
}
