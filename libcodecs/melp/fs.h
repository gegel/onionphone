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
 
  fs.h: Fourier series functions include file

*/

void mf_fft(float *datam1,int nn,int isign);
void mf_find_harm(float input[], float mf_mag[],float pitch,int num_harm,int length);
int mf_findmax(float input[], int npts);
void mf_idft_real(float real[], float signal[], int length);
