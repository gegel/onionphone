/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).

*/

/*

  pit.h: include file for pitch subroutines
  
*/

/* External function definitions */
float mf_double_chk(float sig_in[], float *pcorr, float pitch, float pdouble,
		    int pmin, int pmax, int lmin);
void mf_double_ver(float sig_in[], float *pcorr, float pitch, int pmin,
		   int pmax, int lmin);
float mf_find_pitch(float sig_in[], float *pcorr, int lower, int upper,
		    int length);
float mf_frac_pch(float sig_in[], float *corr, float pitch, int range, int pmin,
		  int pmax, int lmin);
float mf_pitch_ana(float mf_speech[], float resid[], float pitch_est,
		   float mf_pitch_avg, float *pcorr2);
void mf_mf_pitch_ana_init(int pmin, int pmax, int fr, int lpf_ord, int lmin);
float mf_p_avg_update(float pitch, float pcorr, float pthresh);
void mf_p_avg_init(float pdecay, float default_pitch, int num_good);
