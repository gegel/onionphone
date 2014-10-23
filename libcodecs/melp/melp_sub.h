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

  melp_sub.h: include file for MELP subroutines

*/

void mf_dc_rmv(float sigin[], float sigout[], float mf_dcdel[], int frame);
void mf_bpvc_ana(float mf_speech[], float mf_fpitch[], float bpvc[],
		 float sub_pitch[]);
void mf_mf_bpvc_ana_init(int fr, int pmin, int pmax, int nbands, int num_p,
			 int lmin);
float mf_gain_ana(float sigin[], float pitch, int minlength, int maxlength);
void mf_q_gain(float *gain, int *gain_index, float qlow, float qup, float qlev);
void mf_mf_q_gain_dec(float *gain, int *gain_index, float qlow, float qup,
		      float qlev);
int mf_q_bpvc(float *bpvc, int *bpvc_index, float bpthresh, int num_bands);
void mf_mf_q_bpvc_dec(float *bpvc, int *bpvc_index, int uv_flag, int num_bands);
void mf_scale_adj(float *mf_speech, float gain, float *mf_premf_v_scale,
		  int length, int sc_over);
float mf_lin_int_bnd(float x, float xmin, float xmax, float ymin, float ymax);
void mf_noise_est(float gain, float *mf_noise_gain, float up, float down,
		  float min, float max);
void mf_noise_sup(float *gain, float mf_noise_gain, float max_noise,
		  float max_atten, float nfact);
