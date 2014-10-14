/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
   Version 2.2    Last modified: October 2006 
*/
/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C+ floating point ANSI C source code
   Copyright (C) 1999, AT&T, France Telecom, NTT, University of
   Sherbrooke, Conexant, Ericsson. All rights reserved.
----------------------------------------------------------------------
*/

/*
 File : LIBG729.C
 */

/*--------------------------------------------------------------------------------------*
 * Lib of the ITU-T G.729C+   11.8/8/6.4 kbit/s encoder.
 *
 * Van Gegel http://torfone.org   
 *--------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "ld8k.h"
#include "ld8cp.h"
#include "dtx.h"
#include "octet.h"
#if defined(__BORLANDC__)
extern unsigned _stklen = 48000U;
#endif


extern FLOAT *new_speech;           /* Pointer to new speech data   */
static int    g729_prm[PRM_SIZE_E+1];          /* Analysis parameters.                  */
static INT16  g729_serial[SERIAL_SIZE_E];  /* Output bitstream buffer */
static int    g729_frame;               /* frame counter for VAD*/
static int    g729_dtx_enable;
static int    g729_rate;

static FLOAT  synth_buf[L_ANA_BWD];   /* Synthesis */
static int    parm[PRM_SIZE_E+3];             /* Synthesis parameters */
static FLOAT  pst_out[L_FRAME];               /* Postfilter output */
static FLOAT *synth;
static FLOAT  Az_dec[M_BWDP1*2];              /* Decoded Az for post-filter  */
static int    T0_first;                       /* Pitch lag in 1st subframe   */
static int    bwd_dominant;
static int    m_pst;
static int    g729_voicing;                        /* voicing from previous frame */
static int    Vad;
static int    long_h_st;
static int    sf_voic;                        /* voicing for subframe        */
static FLOAT *ptr_Az;
static FLOAT  ga1_post;
static FLOAT  ga2_post;
static FLOAT  ga_harm;

const unsigned char g729_mo[8]={1,2,4,8,16,32,64,128};

//rate: G729D=0,G729=1,G729E=2, dtx: 0/1
void g729ini(int rate, int dtx)
{
 int i;
 g729_dtx_enable = dtx;
 g729_rate = rate;
 
 //Initialize the coder
 init_pre_process();
 init_coder_ld8c(g729_dtx_enable);           
 if(g729_dtx_enable == 1) init_cod_cng();
 for(i=0; i<PRM_SIZE_E; i++) g729_prm[i] = 0;
 g729_frame=0;
 
 //Initialization of decoder
 g729_voicing=60;
 for (i=0; i<L_ANA_BWD; i++) synth_buf[i] = (F)0.;
 synth = synth_buf + MEM_SYN_BWD;
 init_decod_ld8c();
 init_post_filter();
 init_post_process();
 ga1_post = GAMMA1_PST_E;
 ga2_post = GAMMA2_PST_E;
 ga_harm  = GAMMA_HARM_E;
 init_dec_cng();

}

//encode 80 short samples (10 ms frame) to 64/80/118 bits (8/10/15 bytes)
//or produce 15 bits (2 bytes) SID frame 
int g729enc(short *sp16, unsigned char *br)
{
 int i,l,k;
 
 if (g729_frame == 32767) g729_frame = 256; else g729_frame++;
 for (i = 0; i < L_FRAME; i++)  new_speech[i] = (FLOAT) sp16[i];        
 pre_process( new_speech, L_FRAME);
 coder_ld8c(g729_prm, g729_frame, g729_dtx_enable, g729_rate);
 prm2bits_ld8c(g729_prm, g729_serial);       
 l = (int)g729_serial[1]; //number of resulting bits
 
 k=l/8;
 if(l%8) k++; //number of resulting bytes
 memset(br,0,k); 
     
 for(i=0;i<l;i++) if(g729_serial[i+2]==BIT_1) br[i>>3]|=g729_mo[i&7]; 

 //memcpy(br, g729_serial, 2*SERIAL_SIZE_E);
 memset(g729_serial, 0, 2*SERIAL_SIZE_E);

 return k; //0-erasure, 2-SID, 8-G729D, 10-G729, 15-G729E

}



void g729dec(unsigned char *br, short *sp16)
{
 int serial_size=0;
 int i;
 

 //check frame type
 if( (g729_dtx_enable) && (!(*(unsigned int*)(br))) && (!(*(unsigned int*)(br+4))) ) serial_size=0;
 else if( (g729_dtx_enable) && (!(*(unsigned int*)(br+4))) ) serial_size=16;
 else if(g729_rate==0) serial_size=64;
 else if(g729_rate==1) serial_size=80;
 else if(g729_rate==2) serial_size=118;
 
 for(i=0;i<serial_size;i++) if(br[i>>3] & g729_mo[i&7])  g729_serial[i+2]=BIT_1;
 else g729_serial[i+2]=BIT_0;
 
 g729_serial[0]=SYNC_WORD;
 g729_serial[1]=(short)serial_size;
 
 bits2prm_ld8c(&g729_serial[1], parm);
 
 if( (serial_size ==80) ) {
            parm[5] = check_parity_pitch(parm[4], parm[5]);
        }
        else 
            if (serial_size == 118) {
                /* ------------------------------------------------------------------ */
                /* check parity and put 1 in parm[6] if parity error in Forward mode  */
                /*                  put 1 in parm[4] if parity error in Backward mode */
                /* ------------------------------------------------------------------ */
                if (parm[2] == 0) {
                    i = (parm[5] >> 1) & 1;
                    parm[6] += i;
                    parm[6] = check_parity_pitch(parm[5], parm[6]);
                }
                else {
                    i = (parm[3] >> 1) & 1;
                    parm[4] += i;
                    parm[4] = check_parity_pitch(parm[3], parm[4]);
                }
            }

        /* for speech and SID frames, the hardware detects frame erasures
        by checking if all bits are set to zero */
        /* for untransmitted frames, the hardware detects frame erasures
        by testing serial[0] */

        parm[0] = 0;           /* No frame erasure */
        if(g729_serial[1] != 0) {
            for (i=0; i < g729_serial[1]; i++)
                if (g729_serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */
        }
        else if(g729_serial[0] != SYNC_WORD) parm[0] = 1;


        if (parm[0] == 1) {
            if(serial_size < RATE_6400) {
                serial_size = 0;
            }
        }
        
        
          /* ---------- */
        /*  Decoding  */
        /* ---------- */
        decod_ld8c(parm, g729_voicing, synth_buf, Az_dec,
            &T0_first, &bwd_dominant, &m_pst, &Vad);

        /* ---------- */
        /* Postfilter */
        /* ---------- */
        ptr_Az = Az_dec;

        /* Adaptive parameters for postfiltering */
        /* ------------------------------------- */
        if (serial_size != 118) {
            long_h_st = LONG_H_ST;
            ga1_post = GAMMA1_PST;
            ga2_post = GAMMA2_PST;
            ga_harm = GAMMA_HARM;
        }
        else {
            long_h_st = LONG_H_ST_E;
            /* If backward mode is dominant => progressively reduce postfiltering */
            if ((parm[2] == 1) && (bwd_dominant == 1)) {
                ga_harm -= (F)0.0125;
                if (ga_harm < 0) ga_harm = 0;
                ga1_post -= (F)0.035;
                if (ga1_post < 0) ga1_post = 0;
                ga2_post -= (F)0.0325;
                if (ga2_post < 0) ga2_post = 0;
            }
            else {
                ga_harm += (F)0.0125;
                if (ga_harm > GAMMA_HARM_E) ga_harm = GAMMA_HARM_E;
                ga1_post += (F)0.035;
                if (ga1_post > GAMMA1_PST_E) ga1_post = GAMMA1_PST_E;
                ga2_post += (F)0.0325;
                if (ga2_post > GAMMA2_PST_E) ga2_post = GAMMA2_PST_E;
            }
        }
        
        for(i=0; i<L_FRAME; i++) pst_out[i] = synth[i];

        g729_voicing = 0;
        for(i=0; i<L_FRAME; i+=L_SUBFR) {
            poste(T0_first, &synth[i], ptr_Az, &pst_out[i], &sf_voic,
                ga1_post, ga2_post, ga_harm, long_h_st, m_pst, Vad);
            if (sf_voic != 0) g729_voicing = sf_voic;
            ptr_Az += m_pst+1;
        }

        post_process(pst_out, L_FRAME);
        
        for(i=0;i<L_FRAME;i++) sp16[i]=(short)pst_out[i];

}


