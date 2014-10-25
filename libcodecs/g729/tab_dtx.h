/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
 File : TAB_DTX.H
*/

/* VAD constants */
extern float lbf_corr[NP + 1];

/* SID LSF quantization */
extern float noise_fg[MODE][MA_NP][M];
extern float noise_fg_sum[MODE][M];
extern float noise_fg_sum_inv[MODE][M];
extern int PtrTab_1[32];
extern int PtrTab_2[2][16];
extern float Mp[MODE];

/* SID gain quantization */
extern float fact[NB_GAIN + 1];
extern float tab_Sidgain[32];
