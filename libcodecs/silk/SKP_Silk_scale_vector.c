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

#include "SKP_Silk_SigProc_FIX.h"

/* Multiply a vector by a constant */
void SKP_Silk_scale_vector16_Q14( 
    int16_t           *data1, 
    int             gain_Q14,       /* Gain in Q14 */ 
    int             dataSize
)
{
    int   i;
    int32_t data32, gain_Q16;

    SKP_assert( gain_Q14 <   32768 );
    SKP_assert( gain_Q14 >= -32768 );

    gain_Q16 = SKP_LSHIFT( gain_Q14, 2 );

    if( (int32_t)( (int_ptr_size)data1 & 3 ) != 0 ) {
        /* Input is not 4-byte aligned */
        data1[ 0 ] = SKP_SMULWB( gain_Q16, data1[ 0 ] );
        i = 1;
    } else {
        i = 0;
    }
    dataSize--;
    for( ; i < dataSize; i += 2 ) {
        data32 = *( (int32_t *)&data1[ i ] );                     /* load two values at once */
        data1[ i     ] = SKP_SMULWB( gain_Q16, data32 );
        data1[ i + 1 ] = SKP_SMULWT( gain_Q16, data32 );
    }
    if( i == dataSize ) {
        /* One sample left to process */
        data1[ i ] = SKP_SMULWB( gain_Q16, data1[ i ] );
    }
}

/* Multiply a vector by a constant */
void SKP_Silk_scale_vector32_Q26_lshift_18( 
    int32_t           *data1,                     /* (I/O): Q0/Q18        */
    int32_t           gain_Q26,                   /* (I):   Q26           */
    int             dataSize                    /* (I):   length        */
)
{
    int  i;

    for( i = 0; i < dataSize; i++ ) {
        data1[ i ] = (int32_t)SKP_CHECK_FIT32( SKP_RSHIFT64( SKP_SMULL( data1[ i ], gain_Q26 ), 8 ) );// OUTPUT: Q18
    }
}

/* Multiply a vector by a constant */
void SKP_Silk_scale_vector32_16_Q14( 
    int32_t           *data1,                     /* (I/O): Q0/Q0         */
    int             gain_Q14,                   /* (I):   Q14           */
    int             dataSize                    /* (I):   length        */
)
{
    int  i, gain_Q16;

    if( gain_Q14 < ( int16_t_MAX >> 2 ) ) {
        gain_Q16 = SKP_LSHIFT( gain_Q14, 2 );
        for( i = 0; i < dataSize; i++ ) {
            data1[ i ] = SKP_SMULWB( data1[ i ], gain_Q16 );
        }
    } else {
        SKP_assert( gain_Q14 >= int16_t_MIN );
        for( i = 0; i < dataSize; i++ ) {
            data1[ i ] = SKP_LSHIFT( SKP_SMULWB( data1[ i ], gain_Q14 ), 2 );
        }
    }
}

/* Multiply a vector by a constant, does not saturate output data */
void SKP_Silk_scale_vector32_Q16( 
    int32_t           *data1,                     /* (I/O): Q0/Q0         */
    int32_t           gain_Q16,                   /* (I):   gain in Q16 ( int16_t_MIN <= gain_Q16 <= int16_t_MAX + 65536 ) */
    const int       dataSize                    /* (I):   length        */
)
{
    int     i;

    SKP_assert( gain_Q16 <= int16_t_MAX + 65536 );
    SKP_assert( gain_Q16 >= int16_t_MIN );

    if( gain_Q16 > int16_t_MAX ) {
        gain_Q16 -= 65536;
        for( i = 0; i < dataSize; i++ ) {
            data1[ i ] = SKP_SMLAWB( data1[ i ], data1[ i ], gain_Q16 );
        }
    } else {
        for( i = 0; i < dataSize; i++ ) {
            data1[ i ] = SKP_SMULWB( data1[ i ], gain_Q16 );
        }
    }
}
