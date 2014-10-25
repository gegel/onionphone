/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
/*
 File : DTX.H
*/

/*--------------------------------------------------------------------------*
 * Prototypes for DTX/CNG                                                   *
 *--------------------------------------------------------------------------*/

/* Encoder DTX/CNG functions */
void init_cod_cng(void);
void cod_cng(float * exc,	/* (i/o) : excitation array                     */
	     int pastVad,	/* (i)   : previous VAD decision                */
	     float * lsp_old_q,	/* (i/o) : previous quantized lsp               */
	     float * old_A,	/* (i/o) : last stable filter LPC coefficients  */
	     float * old_rc,	/* (i/o) : last stable filter Reflection coefficients. */
	     float * Aq,	/* (o)   : set of interpolated LPC coefficients */
	     int *ana,		/* (o)   : coded SID parameters                 */
	     float freq_prev[MA_NP][M],
	     /* (i/o) : previous LPS for quantization        */
	     int16_t * seed	/* (i/o) : random generator seed                */
    );
void update_cng(float * r,	/* (i) :   frame autocorrelation               */
		int Vad		/* (i) :   current Vad decision                */
    );

/* SID gain Quantization */
void qua_Sidgain(float * ener,	/* (i)   array of energies                   */
		 int nb_ener,	/* (i)   number of energies or               */
		 float * enerq,	/* (o)   decoded energies in dB              */
		 int *idx	/* (o)   SID gain quantization index         */
    );

/* CNG excitation generation */
void calc_exc_rand(float cur_gain,	/* (i)   :   target sample gain                 */
		   float * exc,	/* (i/o) :   excitation array                   */
		   int16_t * seed,	/* (i)   :   current Vad decision               */
		   int flag_cod	/* (i)   :   encoder/decoder flag               */
    );

/* SID LSP Quantization */
void get_freq_prev(float x[MA_NP][M]);
void update_freq_prev(float x[MA_NP][M]);
void get_decfreq_prev(float x[MA_NP][M]);
void update_decfreq_prev(float x[MA_NP][M]);

/* Decoder CNG generation */
void init_dec_cng(void);
void dec_cng(int past_ftyp,	/* (i)   : past frame type                      */
	     float sid_sav,	/* (i)   : energy to recover SID gain           */
	     int *parm,		/* (i)   : coded SID parameters                 */
	     float * exc,	/* (i/o) : excitation array                     */
	     float * lsp_old,	/* (i/o) : previous lsp                         */
	     float * A_t,	/* (o)   : set of interpolated LPC coefficients */
	     int16_t * seed,	/* (i/o) : random generator seed                */
	     float freq_prev[MA_NP][M]
	     /* (i/o) : previous LPS for quantization        */
    );
int read_frame(FILE * f_serial, int *parm);

/*--------------------------------------------------------------------------*
 * Constants for DTX/CNG                                                    *
 *--------------------------------------------------------------------------*/

/* DTX constants */
#define FLAG_COD        1
#define FLAG_DEC        0
#define INIT_SEED       (int16_t)11111
#define FR_SID_MIN      3
#define NB_SUMACF       3
#define NB_CURACF       2
#define NB_GAIN         2
#define THRESH1         (float)1.1481628
#define THRESH2         (float)1.0966466
#define A_GAIN0         (float)0.875

#define SIZ_SUMACF      (NB_SUMACF * MP1)
#define SIZ_ACF         (NB_CURACF * MP1)
#define A_GAIN1         ((float)1. - A_GAIN0)

#define MIN_ENER        (float)0.1588489319	/* <=> - 8 dB      */

/* CNG excitation generation constant */
					   /* alpha = 0.5 */
#define NORM_GAUSS      (float)3.16227766	/* sqrt(40)xalpha */
#define K0              (float)3.	/* 4 x (1 - alpha ** 2) */
#define G_MAX           (float)5000.
