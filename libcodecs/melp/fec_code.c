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
    Name: mf_fec_code.c, mf_fec_decode.c
    Description: Encode/decode FEC (Hamming codes) on selected MELP parameters
    Inputs:
      MELP parameter structure
    Outputs: 
      updated MELP parameter structure
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

#include <stdio.h>
#include "melp.h"

/* Prototypes */
int mf_binprod_int(int *x, int *y, int n);
int *mf_vgetbits(int *y, int x, int p, int n);
int mf_vsetbits(int x, int p, int n, int *y);
void mf_sbc_enc(int x[], int n, int k, int *pmat);
int mf_sbc_dec(int x[], int n, int k, int *pmat, int syntab[]);
int mf_sbc_syn(int x[], int n, int k, int *pmat);

/* Macros */
#define null_function() exit(1)
/* Set vector to scalar value. */

#define V_SET(x,sc,n) if (1)\
{int u__i;\
     for(u__i=0; u__i < (n); u__i++)\
     *((x)+u__i) = (sc);\
} else null_function()

/* Compiler constants */
#define UV_PIND 0    /* Unvoiced pitch index */
#define INVAL_PIND 1 /* Invalid pitch index  */
#define BEP_CORR -1   /* "Correct" bit error position */
#define BEP_UNCORR -2 /* "Uncorrectable" bit error position */

extern int mf_pitch_enc[PIT_QLEV+1]; /* Pitch index encoding table */
extern int mf_pmat74[3][4];  /* (7,4) Hamming code parity matrix */
extern int mf_syntab74[8];   /* (7,4) Hamming code syndrome->bep table */
extern int mf_pmat84[4][4];  /* (8,4) Hamming code parity matrix */
extern int mf_syntab84[16];  /* (8,4) Hamming code syndrome->bep table */

static int mf_codewd74[7];
static int mf_codewd84[8];

void mf_fec_code(struct mf_melp_param *par)

{

    /* Increment pitch index to allow for unvoiced pitch code */
    par->pitch_index++; 

/*
** Unvoiced case - use spare parameter bits for error protection.
*/
    if (par->uv_flag)
    {
	/* Set pitch index to unvoiced value */
	par->pitch_index = UV_PIND;

/*
** Code 4 MSB of first vq stage index using (8,4) Hamming code; parity bits in
** bpvc index.
*/
	mf_vgetbits(mf_codewd84,par->msvq_index[0],6,4);
	mf_sbc_enc(mf_codewd84,8,4,&mf_pmat84[0][0]);
	par->bpvc_index=mf_vsetbits(par->bpvc_index,3,4,&mf_codewd84[4]);
/*
** Code 3 LSB of first vq stage index using (7,4) Hamming code; parity bits
** in 3 MSB of fsvq index.
*/
	mf_vgetbits(mf_codewd74,par->msvq_index[0],2,3);
	mf_codewd74[3] = 0;
	mf_sbc_enc(mf_codewd74,7,4,&mf_pmat74[0][0]);
	par->fsvq_index[0]=mf_vsetbits(par->fsvq_index[0],7,3,&mf_codewd74[4]);
/*
** Code 4 MSB of second gain index using (7,4) Hamming code; parity bits in
** next 3 MSB of fsvq index.
*/
	mf_vgetbits(mf_codewd74,par->gain_index[1],4,4);
	mf_sbc_enc(mf_codewd74,7,4,&mf_pmat74[0][0]);
	par->fsvq_index[0]=mf_vsetbits(par->fsvq_index[0],4,3,&mf_codewd74[4]);
/*
** Code LSB of second gain index, first gain index using (7,4) Hamming code;
** parity bits in 2 LSB of fsvq index, jitter index bit.
*/
	mf_vgetbits(mf_codewd74,par->gain_index[1],0,1);
	mf_vgetbits(&mf_codewd74[1],par->gain_index[0],2,3);
	mf_sbc_enc(mf_codewd74,7,4,&mf_pmat74[0][0]);
	par->fsvq_index[0]=mf_vsetbits(par->fsvq_index[0],1,2,&mf_codewd74[4]);
	par->jit_index=mf_vsetbits(par->jit_index,0,1,&mf_codewd74[6]);
    }

    /* Encode pitch index */
    par->pitch_index = mf_pitch_enc[par->pitch_index];

}

int mf_fec_decode(struct mf_melp_param *par,int erase)

{

    extern int mf_pitch_dec[1<<PIT_BITS]; /* Pitch index decoding table */
    int berr_pos;

    /* Decode pitch index */
    par->pitch_index = mf_pitch_dec[par->pitch_index];

/*
** Set unvoiced flag for pitch index of UV_PIND; set erase flag for invalid
** pitch index INVAL_PIND.  Otherwise, convert pitch index into quantization
** level.
*/
    if (!(par->uv_flag = par->pitch_index == UV_PIND) &&
	!(erase |= par->pitch_index == INVAL_PIND))
	par->pitch_index-=2; /* Subtract to acct. for reserved pitch codes.*/

    if (par->uv_flag && !erase)
/*
** Unvoiced case - use spare parameter bits for error control coding.
*/
    {
/*
** Decode 4 MSB of first vq stage index using (8,4) Hamming code; parity bits
** in bpvc index.  Set bpvc index to zero.
*/
	mf_vgetbits(mf_codewd84,par->msvq_index[0],6,4);
	mf_vgetbits(&mf_codewd84[4],par->bpvc_index,3,4);
	berr_pos=mf_sbc_dec(mf_codewd84,8,4,&mf_pmat84[0][0],mf_syntab84);
	erase |= berr_pos == BEP_UNCORR;
	par->msvq_index[0]=mf_vsetbits(par->msvq_index[0],6,4,mf_codewd84);
	par->bpvc_index = 0;

	/* Perform remaining decoding only if no frame repeat flagged. */
	if (!erase)
	{
/*
** Decode 3 LSB of first vq stage index using (7,4) Hamming code; parity bits
** in 3 MSB of fsvq index.
*/
	    mf_vgetbits(mf_codewd74,par->msvq_index[0],2,3);
	    mf_codewd74[3] = 0;
	    mf_vgetbits(&mf_codewd74[4],par->fsvq_index[0],7,3);
	    mf_sbc_dec(mf_codewd74,7,4,&mf_pmat74[0][0],mf_syntab74);
	    par->msvq_index[0]=mf_vsetbits(par->msvq_index[0],2,3,mf_codewd74);
/*
** Decode 4 MSB of second gain index using (7,4) Hamming code; parity bits in
** next 3 MSB of fsvq index.
*/
	    mf_vgetbits(mf_codewd74,par->gain_index[1],4,4);
	    mf_vgetbits(&mf_codewd74[4],par->fsvq_index[0],4,3);
	    mf_sbc_dec(mf_codewd74,7,4,&mf_pmat74[0][0],mf_syntab74);
	    par->gain_index[1]=mf_vsetbits(par->gain_index[1],4,4,mf_codewd74);
/*
** Decode LSB of second gain index, first gain index using (7,4) Hamming code;
** parity bits in 2 LSB of fsvq index, jitter index bit.  Set
** jitter index bits to one.
*/
	    mf_vgetbits(mf_codewd74,par->gain_index[1],0,1);
	    mf_vgetbits(&mf_codewd74[1],par->gain_index[0],2,3);
	    mf_vgetbits(&mf_codewd74[4],par->fsvq_index[0],1,2);
	    mf_vgetbits(&mf_codewd74[6],par->jit_index,0,1);
	    mf_sbc_dec(mf_codewd74,7,4,&mf_pmat74[0][0],mf_syntab74);
	    par->gain_index[1]=mf_vsetbits(par->gain_index[1],0,1,mf_codewd74);
	    par->gain_index[0]=mf_vsetbits(par->gain_index[0],2,3,&mf_codewd74[1]);
	    par->jit_index = 1;
	}
    } /* if (par->uv_flag && !erase) */

    return(erase);

}
/*
     binprod returns a
     bitwise modulo-2 inner product between x and y.
*/

int mf_binprod_int(int *x, int *y, int n)
{
    int val=(int) 0;
    register int i;
    
    for (i=0; i<n; i++)
	val ^= *x++ & *y++;
    
    return(val);
}
/*

     mf_vgetbits extracts an n-bit pattern beginning at bit position p from an
     int, and returns a bit vector containing the pattern.  Conversely,
     mf_vsetbits takes a length n bit vector and sets the bit pattern in an
     integer.

*/

int *mf_vgetbits(int *y, int x, int p, int n)
{
    int lsb=0x1; /* least significant bit mask */
    int *retval=y;

    if (n < 0 || p < n-1)
	return(NULL);
    
    for (y+=n-1,x>>=p-n+1; y>=retval; y--,x>>=1)
	*y = x & lsb;

    return(retval);
}

int mf_vsetbits(int x, int p, int n, int *y)
{
    register int i,j;

    if (n < 0 || p < n-1)
	return(x);
    
    for (i=0,j=p; i<n; i++,j--)
    {
	x &= ~(0x1 << j);  /* mask out bit position j */
	x |= *(y++) << j;  /* set bit position j to array value */
    }
    return(x);
}
/*
   Name: code_blk - systematic block error control code functions.

   Description:

     These functions are designed to implement systematic block codes given an
     input vector and a parity matrix.  These codes are characterized by
     leaving the data bits of the protected n-bit block unaltered.  The parity
     matrix used in these functions is a (n-k) x k matrix which generated the
     parity bits for a given input block.

     mf_sbc_enc takes a length n bit vector x, applies parity matrix pmat, and
     writes the parity bits into the last n-k positions of x.

     mf_sbc_dec takes x (after processing by mf_sbc_enc) and corrects x for bit
     errors using a syndrome table lookup.  mf_sbc_dec returns the index of a
     detected bit error in x.  mf_sbc_dec returns -1 if no error is found.

     mf_sbc_syn takes x (after processing by mf_sbc_enc) and computes a syndrome
     index used to look up a bit error position in the syndrome table.

*/

void mf_sbc_enc(int x[], int n, int k, int *pmat)
{
    register int i;
    for (i=k; i<n; i++,pmat+=k)
	x[i] = mf_binprod_int(x,pmat,k);
}

int mf_sbc_dec(int x[], int n, int k, int *pmat, int syntab[])
{
    int bep=syntab[mf_sbc_syn(x,n,k,pmat)];
    if (bep > -1)
	x[bep] ^= 0x1;
    return(bep);
}

int mf_sbc_syn(int x[], int n, int k, int *pmat)
{
    int retval=0;
    register int i,j;
    for (i=k,j=n-k-1; i<n; i++,j--,pmat+=k)
	retval += (x[i] ^ mf_binprod_int(x,pmat,k)) << j;
    return(retval);
}


/*
** (7,4) Hamming code tables.
*/
/* Parity generator matrix. */
int mf_pmat74[3][4] = {{1,1,0,1},{1,0,1,1},{0,1,1,1}};

/* Syndrome table. */
int mf_syntab74[8] = {BEP_CORR,6,5,2,4,1,0,3};

/*
** (8,4) extended Hamming code tables.
*/

/* Parity generator matrix. */
int mf_pmat84[4][4] = {{1,1,0,1},{1,0,1,1},{0,1,1,1},{1,1,1,0}};

/* Syndrome->error position lookup table. */
int mf_syntab84[16] =
{
    BEP_CORR,    /* 0x0 */
    7,           /* 0x1 */
    6,           /* 0x2 */
    BEP_UNCORR,  /* 0x3 */
    5,           /* 0x4 */
    BEP_UNCORR,  /* 0x5 */
    BEP_UNCORR,  /* 0x6 */
    2,           /* 0x7 */
    4,           /* 0x8 */
    BEP_UNCORR,  /* 0x9 */
    BEP_UNCORR,  /* 0xA */
    1,           /* 0xB */
    BEP_UNCORR,  /* 0xC */
    0,           /* 0xD */
    3,           /* 0xE */
    BEP_UNCORR   /* 0xF */
};

/*
** Pitch index encoding table.  Reserve Hamming weight 0,1 words for
** unvoiced pitch value.  Reserve Hamming weight 2 words for invalid (protect
** against single bit voiced pitch errors.  Assign voiced pitch codes to
** values having Hamming weight > 2.
*/

int mf_pitch_enc[PIT_QLEV+1] = 
{
0x0, /* UV_PIND */
0x7, /* 1 (first pitch QL - note offset) */
0xB, /* 2 */
0xD, /* 3 */
0xE, /* 4 */
0xF, /* 5 */
0x13, /* 6 */
0x15, /* 7 */
0x16, /* 8 */
0x17, /* 9 */
0x19, /* 10 */
0x1A, /* 11 */
0x1B, /* 12 */
0x1C, /* 13 */
0x1D, /* 14 */
0x1E, /* 15 */
0x1F, /* 16 */
0x23, /* 17 */
0x25, /* 18 */
0x26, /* 19 */
0x27, /* 20 */
0x29, /* 21 */
0x2A, /* 22 */
0x2B, /* 23 */
0x2C, /* 24 */
0x2D, /* 25 */
0x2E, /* 26 */
0x2F, /* 27 */
0x31, /* 28 */
0x32, /* 29 */
0x33, /* 30 */
0x34, /* 31 */
0x35, /* 32 */
0x36, /* 33 */
0x37, /* 34 */
0x38, /* 35 */
0x39, /* 36 */
0x3A, /* 37 */
0x3B, /* 38 */
0x3C, /* 39 */
0x3D, /* 40 */
0x3E, /* 41 */
0x3F, /* 42 */
0x43, /* 43 */
0x45, /* 44 */
0x46, /* 45 */
0x47, /* 46 */
0x49, /* 47 */
0x4A, /* 48 */
0x4B, /* 49 */
0x4C, /* 50 */
0x4D, /* 51 */
0x4E, /* 52 */
0x4F, /* 53 */
0x51, /* 54 */
0x52, /* 55 */
0x53, /* 56 */
0x54, /* 57 */
0x55, /* 58 */
0x56, /* 59 */
0x57, /* 60 */
0x58, /* 61 */
0x59, /* 62 */
0x5A, /* 63 */
0x5B, /* 64 */
0x5C, /* 65 */
0x5D, /* 66 */
0x5E, /* 67 */
0x5F, /* 68 */
0x61, /* 69 */
0x62, /* 70 */
0x63, /* 71 */
0x64, /* 72 */
0x65, /* 73 */
0x66, /* 74 */
0x67, /* 75 */
0x68, /* 76 */
0x69, /* 77 */
0x6A, /* 78 */
0x6B, /* 79 */
0x6C, /* 80 */
0x6D, /* 81 */
0x6E, /* 82 */
0x6F, /* 83 */
0x70, /* 84 */
0x71, /* 85 */
0x72, /* 86 */
0x73, /* 87 */
0x74, /* 88 */
0x75, /* 89 */
0x76, /* 90 */
0x77, /* 91 */
0x78, /* 92 */
0x79, /* 93 */
0x7A, /* 94 */
0x7B, /* 95 */
0x7C, /* 96 */
0x7D, /* 97 */
0x7E, /* 98 */
0x7F /* 99 */
};

/*
** Pitch index decoding table.  Hamming weight 1 codes map to UV_PIND,
** allowing for 1-bit error correction of unvoiced marker.  Hamming weight 2
** codes map to INVAL_PIND, protecting against 1-bit errors in voiced pitches
** creating false unvoiced condition.
*/

int mf_pitch_dec[1<<PIT_BITS] = 
{
UV_PIND, /* 0x0 */
UV_PIND, /* 0x1 */
UV_PIND, /* 0x2 */
INVAL_PIND, /* 0x3 */
UV_PIND, /* 0x4 */
INVAL_PIND, /* 0x5 */
INVAL_PIND, /* 0x6 */
2, /* 0x7 */
UV_PIND, /* 0x8 */
INVAL_PIND, /* 0x9 */
INVAL_PIND, /* 0xA */
3, /* 0xB */
INVAL_PIND, /* 0xC */
4, /* 0xD */
5, /* 0xE */
6, /* 0xF */
UV_PIND, /* 0x10 */
INVAL_PIND, /* 0x11 */
INVAL_PIND, /* 0x12 */
7, /* 0x13 */
INVAL_PIND, /* 0x14 */
8, /* 0x15 */
9, /* 0x16 */
10, /* 0x17 */
INVAL_PIND, /* 0x18 */
11, /* 0x19 */
12, /* 0x1A */
13, /* 0x1B */
14, /* 0x1C */
15, /* 0x1D */
16, /* 0x1E */
17, /* 0x1F */
UV_PIND, /* 0x20 */
INVAL_PIND, /* 0x21 */
INVAL_PIND, /* 0x22 */
18, /* 0x23 */
INVAL_PIND, /* 0x24 */
19, /* 0x25 */
20, /* 0x26 */
21, /* 0x27 */
INVAL_PIND, /* 0x28 */
22, /* 0x29 */
23, /* 0x2A */
24, /* 0x2B */
25, /* 0x2C */
26, /* 0x2D */
27, /* 0x2E */
28, /* 0x2F */
INVAL_PIND, /* 0x30 */
29, /* 0x31 */
30, /* 0x32 */
31, /* 0x33 */
32, /* 0x34 */
33, /* 0x35 */
34, /* 0x36 */
35, /* 0x37 */
36, /* 0x38 */
37, /* 0x39 */
38, /* 0x3A */
39, /* 0x3B */
40, /* 0x3C */
41, /* 0x3D */
42, /* 0x3E */
43, /* 0x3F */
UV_PIND, /* 0x40 */
INVAL_PIND, /* 0x41 */
INVAL_PIND, /* 0x42 */
44, /* 0x43 */
INVAL_PIND, /* 0x44 */
45, /* 0x45 */
46, /* 0x46 */
47, /* 0x47 */
INVAL_PIND, /* 0x48 */
48, /* 0x49 */
49, /* 0x4A */
50, /* 0x4B */
51, /* 0x4C */
52, /* 0x4D */
53, /* 0x4E */
54, /* 0x4F */
INVAL_PIND, /* 0x50 */
55, /* 0x51 */
56, /* 0x52 */
57, /* 0x53 */
58, /* 0x54 */
59, /* 0x55 */
60, /* 0x56 */
61, /* 0x57 */
62, /* 0x58 */
63, /* 0x59 */
64, /* 0x5A */
65, /* 0x5B */
66, /* 0x5C */
67, /* 0x5D */
68, /* 0x5E */
69, /* 0x5F */
INVAL_PIND, /* 0x60 */
70, /* 0x61 */
71, /* 0x62 */
72, /* 0x63 */
73, /* 0x64 */
74, /* 0x65 */
75, /* 0x66 */
76, /* 0x67 */
77, /* 0x68 */
78, /* 0x69 */
79, /* 0x6A */
80, /* 0x6B */
81, /* 0x6C */
82, /* 0x6D */
83, /* 0x6E */
84, /* 0x6F */
85, /* 0x70 */
86, /* 0x71 */
87, /* 0x72 */
88, /* 0x73 */
89, /* 0x74 */
90, /* 0x75 */
91, /* 0x76 */
92, /* 0x77 */
93, /* 0x78 */
94, /* 0x79 */
95, /* 0x7A */
96, /* 0x7B */
97, /* 0x7C */
98, /* 0x7D */
99, /* 0x7E */
100 /* 0x7F */
};

