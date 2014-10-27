/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)

    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
   Types definitions
*/
#ifndef G723_CONST_H
#define G723_CONST_H

#include <stdint.h>

//begain -----------------------------------add by haiping 2009-06-19
//#define TEST_MIPS
//#define TESTMIPSNUM 25
//end    -----------------------------------add by haiping 2009-06-19

/*
#define L_mult __builtin_bfin_mult_fr1x32
#define L_add __builtin_bfin_add_fr1x32
#define add __builtin_bfin_add_fr1x16
#define abs_s __builtin_bfin_abs_fr1x16
#define sub __builtin_bfin_sub_fr1x16

//use cost most
#define mult __builtin_bfin_mult_fr1x16
#define mult_r __builtin_bfin_multr_fr1x16
#define shl __builtin_bfin_shl_fr1x16

//#define negate __builtin_bfin_negate_fr1x16

//use cost little
#define L_sub __builtin_bfin_sub_fr1x32
#define L_shl __builtin_bfin_shl_fr1x32

#define shr(a,b) __builtin_bfin_shl_fr1x16(a,-(b))
#define L_shr(a,b) __builtin_bfin_shl_fr1x32(a,-(b))
#define L_abs __builtin_bfin_abs_fr1x32

//basop.c:652: error: incompatible type for argument 1 of ‘__builtin_bfin_extract_hi’
//example:
//  fract2x16 a, b, d;
//  d = __builtin_bfin_add_fr2x16 (a, b);
//  t1 = __builtin_bfin_extract_lo (d);
//  t2 = __builtin_bfin_extract_hi (d);

//so extract_h !=__builtin_bfin_extract_hi

//#define extract_h __builtin_bfin_extract_hi
//#define extract_l __builtin_bfin_extract_lo

#define norm_s __builtin_bfin_norm_fr1x16

__inline__
__attribute__ ((always_inline))
	static int norm_l(int32_t _x) {
   return _x==0 ? 0 : __builtin_bfin_norm_fr1x32(_x);
}

__inline__
__attribute__ ((always_inline))
static int32_t L_deposit_h(int16_t _x) {
unsigned int u = (unsigned int)_x;
u <<= 16;
return (int32_t)u;
}

__inline__
__attribute__ ((always_inline))
static int32_t L_deposit_l(int16_t _x) { return ((int)((int16_t)(_x))); }

__inline__
__attribute__ ((always_inline))
static int16_t extract_l(int32_t _x) { return (int16_t)(_x); }

__inline__
__attribute__ ((always_inline))
static int16_t extract_h(int32_t _x) {
	  unsigned int u = (unsigned int)_x;
	  u >>= 16;
	  return (int16_t)u;
}
*/
/* If _x>0x00007fff, returns 0x7fff. If _x<0xffff8000, returns 0x8000.
   Otherwise returns the lower 16 bits of _x. */
/*
__inline__
__attribute__ ((always_inline))
static int16_t sature(int32_t _x)
{
*/
	   /* cast conversions due to MISRA */
/*
   unsigned int val;
    val = (unsigned int)__builtin_bfin_shl_fr1x32(_x,16)>>16;
   return (int16_t)val;
  }

__inline__
__attribute__ ((always_inline))
static int32_t L_msu(int32_t L_var3, int16_t var1, int16_t var2)
{
    int32_t L_var_out;
    int32_t L_produit;

    L_produit = __builtin_bfin_mult_fr1x32(var1, var2);
    L_var_out = __builtin_bfin_sub_fr1x32(L_var3, L_produit);
    return(L_var_out);
}
*/
/* This function rounds the 32-bit fract to a 16-bit fract using biased
	 * rounding.  */
/*
__inline__
__attribute__ ((always_inline))
static int16_t round_(int32_t _x)
{ return __builtin_bfin_round_fr1x32(_x); }
*/
/*
**
** File:        "cst_lbc.h"
**
** Description:  This file contains global definition of the SG15
**    LBC Coder for 6.3/5.3 kbps.
**
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#define  False 0
#define  True  1

/* Definition of the working mode */
enum Wmode { Both, Cod, Dec };

/* Coder rate */
enum Crate { Rate63, Rate53 };

/* Coder global constants */
#define  Frame       240
#define  LpcFrame    180
#define  SubFrames   4
#define  SubFrLen    (Frame/SubFrames)

/* LPC constants */
#define  LpcOrder          10
#define  RidgeFact         10
#define  CosineTableSize   512
#define  PreCoef           (int16_t) 0xc000	/* -0.25*2 */

#define  LspPrd0           12288
#define  LspPrd1           23552

#define  LspQntBands       3
#define  LspCbSize         256
#define  LspCbBits         8

/* LTP constants */
#define  PitchMin          18
#define  PitchMax          (PitchMin+127)
#define  PwConst           (int16_t) 0x2800
#define  PwRange           3
#define  ClPitchOrd        5
#define  Pstep             1
#define NbFilt085          85
#define NbFilt170          170

/* MP-MLQ constants */
#define  Sgrid             2
#define  MaxPulseNum       6
#define  MlqSteps          2

/* acelp constants */
#define SubFrLen2          (SubFrLen +4)
#define DIM_RR             416
#define NB_POS             8
#define STEP               8
#define MSIZE              64
#define threshold          16384	/* 0.5 = 16384 in Q15 */
#define max_time           120

/* Gain constant */
#define  NumOfGainLev      24

/* FER constant */
#define  ErrMaxNum         3

/* CNG constants  */
#define NbAvAcf            3	/* Nb of frames for Acf average               */
#define NbAvGain           3	/* Nb of frames for gain average              */
#define ThreshGain         3	/* Theshold for quantized gains               */
#define FracThresh         7000	/* Itakura dist threshold: frac. part     */
#define NbPulsBlk          11	/* Nb of pulses in 2-subframes blocks         */

#define InvNbPulsBlk       2979	/* 32768/NbPulsBlk                          */
#define NbFilt             50	/* number of filters for CNG exc generation   */
#define LpcOrderP1         (LpcOrder+1)
#define SizAcf             ((NbAvAcf+1)*LpcOrderP1)	/* size of array Acf    */
#define SubFrLenD          (2*SubFrLen)
#define Gexc_Max           5000	/* Maximum gain for fixed CNG excitation   */

/* Taming constants */
#define NbFilt085_min      51
#define NbFilt170_min      93
#define SizErr             5
#define Err0               (int32_t)4	/* scaling factor */
#define ThreshErr          0x40000000L
#define DEC                (30 - 7)

/*
   Used structures
*/
typedef struct {
	/* High pass variables */
	int16_t HpfZdl;
	int32_t HpfPdl;

	/* Sine wave detector */
	int16_t SinDet;

	/* Lsp previous vector */
	int16_t PrevLsp[LpcOrder];

	/* All pitch operation buffers */
	int16_t PrevWgt[PitchMax];
	int16_t PrevErr[PitchMax];
	int16_t PrevExc[PitchMax];

	/* Required memory for the delay */
	int16_t PrevDat[LpcFrame - SubFrLen];

	/* Used delay lines */
	int16_t WghtFirDl[LpcOrder];
	int16_t WghtIirDl[LpcOrder];
	int16_t RingFirDl[LpcOrder];
	int16_t RingIirDl[LpcOrder];

	/* Taming procedure errors */
	int32_t Err[SizErr];

} CODSTATDEF;

typedef struct {
	int16_t Ecount;
	int16_t InterGain;
	int16_t InterIndx;
	int16_t Rseed;
	int16_t Park;
	int16_t Gain;
	/* Lsp previous vector */
	int16_t PrevLsp[LpcOrder];

	/* All pitch operation buffers */
	int16_t PrevExc[PitchMax];

	/* Used delay lines */
	int16_t SyntIirDl[LpcOrder];
	int16_t PostFirDl[LpcOrder];
	int16_t PostIirDl[LpcOrder];

} DECSTATDEF;

   /* subframe coded parameters */
typedef struct {
	int16_t AcLg;
	int16_t AcGn;
	int16_t Mamp;
	int16_t Grid;
	int16_t Tran;
	int16_t Pamp;
	int32_t Ppos;
} SFSDEF;

   /* frame coded parameters */
typedef struct {
	int16_t Crc;
	int32_t LspId;
	int16_t Olp[SubFrames / 2];
	SFSDEF Sfs[SubFrames];
} LINEDEF;

   /* harmonic noise shaping filter parameters */
typedef struct {
	int16_t Indx;
	int16_t Gain;
} PWDEF;

    /* pitch postfilter parameters */
typedef struct {
	int16_t Indx;
	int16_t Gain;
	int16_t ScGn;
} PFDEF;

    /* best excitation vector parameters for the high rate */
typedef struct {
	int32_t MaxErr;
	int16_t GridId;
	int16_t MampId;
	int16_t UseTrn;
	int16_t Ploc[MaxPulseNum];
	int16_t Pamp[MaxPulseNum];
} BESTDEF;

    /* VAD static variables */
typedef struct {
	int16_t Hcnt;
	int16_t Vcnt;
	int32_t Penr;
	int32_t Nlev;
	int16_t Aen;
	int16_t Polp[4];
	int16_t NLpc[LpcOrder];
} VADSTATDEF;

/* CNG features */

/* Coder part */
typedef struct {
	int16_t CurGain;
	int16_t PastFtyp;
	int16_t Acf[SizAcf];
	int16_t ShAcf[NbAvAcf + 1];
	int16_t LspSid[LpcOrder];
	int16_t SidLpc[LpcOrder];
	int16_t RC[LpcOrderP1];
	int16_t ShRC;
	int16_t Ener[NbAvGain];
	int16_t NbEner;
	int16_t IRef;
	int16_t SidGain;
	int16_t RandSeed;
} CODCNGDEF;

/* Decoder part */
typedef struct {
	int16_t CurGain;
	int16_t PastFtyp;
	int16_t LspSid[LpcOrder];
	int16_t SidGain;
	int16_t RandSeed;
} DECCNGDEF;
#endif
