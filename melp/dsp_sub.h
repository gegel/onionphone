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

  dsp_sub.h: include file 
  
*/

/* External function definitions */
void mf_autocorr(float input[], float r[], int order, int npts);
void mf_envelope(float input[], float prev_in, float output[], int npts);
void mf_fill(float output[], float mf_fillval, int npts);
void mf_interp_array(float prev[],float curr[],float out[],float ifact,int size);
float mf_median(float input[], int npts);
void  mf_pack_code(int code,unsigned int **p_ch_beg,int *p_ch_bit,int numbits,int size);
float mf_peakiness(float input[], int npts);
void mf_polflt(float input[], float coeff[], float output[], int order,int npts);
void mf_quant_u(float *p_data, int *p_index, float qmin, float qmax, int nlev);
void mf_mf_quant_u_dec(int index, float *p_data,float qmin, float qmax, int nlev);
void mf_rand_num(float output[],float amplitude, int npts);
int unmf_pack_code(unsigned int **p_ch_beg,int *p_ch_bit,int *p_code,int numbits,int wsize,unsigned int erase_mask);
void mf_window(float input[], float mf_win_cof[], float output[], int npts);
void mf_zerflt(float input[], float coeff[], float output[], int order,int npts);
int mf_readbl(float input[], FILE *fp_in, int size);
void mf_writebl(float output[], FILE *fp_out, int size);
