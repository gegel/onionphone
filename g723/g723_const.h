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

#if defined(__BORLANDC__) || defined (__WATCOMC__) || defined(_MSC_VER) || defined(__ZTC__) || defined(__HIGHC__)
typedef  long  int   Word32   ;
typedef  short int   Word16   ;
typedef  short int   Flag  ;
#elif defined( __sun)
typedef short  Word16;
typedef long  Word32;
typedef int   Flag;
#elif defined(__alpha)
typedef short Word16;
typedef int   Word32;
typedef int   Flag;
#elif defined(VMS) || defined(__VMS) || defined(VAX)
typedef short  Word16;
typedef long  Word32;
typedef int   Flag;
#else
typedef short Word16;
typedef int   Word32;
typedef int   Flag;
#endif

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
	static int norm_l(Word32 _x) {
   return _x==0 ? 0 : __builtin_bfin_norm_fr1x32(_x);
}

__inline__
__attribute__ ((always_inline))
static Word32 L_deposit_h(Word16 _x) {
unsigned int u = (unsigned int)_x;
u <<= 16;
return (Word32)u;
}

__inline__
__attribute__ ((always_inline))
static Word32 L_deposit_l(Word16 _x) { return ((int)((Word16)(_x))); }

__inline__
__attribute__ ((always_inline))
static Word16 extract_l(Word32 _x) { return (Word16)(_x); }

__inline__
__attribute__ ((always_inline))
static Word16 extract_h(Word32 _x) {
	  unsigned int u = (unsigned int)_x;
	  u >>= 16;
	  return (Word16)u;
}
*/
/* If _x>0x00007fff, returns 0x7fff. If _x<0xffff8000, returns 0x8000.
   Otherwise returns the lower 16 bits of _x. */
/*
__inline__
__attribute__ ((always_inline))
static Word16 sature(Word32 _x)
{
*/
	   /* cast conversions due to MISRA */
/*
   unsigned int val;
    val = (unsigned int)__builtin_bfin_shl_fr1x32(_x,16)>>16;
   return (Word16)val;
  }


__inline__
__attribute__ ((always_inline))
static Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2)
{
    Word32 L_var_out;
    Word32 L_produit;

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
static Word16 round_(Word32 _x)
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
enum  Wmode { Both, Cod, Dec } ;

/* Coder rate */
enum  Crate    { Rate63, Rate53 } ;

/* Coder global constants */
#define  Frame       240
#define  LpcFrame    180
#define  SubFrames   4
#define  SubFrLen    (Frame/SubFrames)

/* LPC constants */
#define  LpcOrder          10
#define  RidgeFact         10
#define  CosineTableSize   512
#define  PreCoef           (Word16) 0xc000            /* -0.25*2 */

#define  LspPrd0           12288
#define  LspPrd1           23552

#define  LspQntBands       3
#define  LspCbSize         256
#define  LspCbBits         8

/* LTP constants */
#define  PitchMin          18
#define  PitchMax          (PitchMin+127)
#define  PwConst           (Word16) 0x2800
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
#define threshold          16384  /* 0.5 = 16384 in Q15 */
#define max_time           120

/* Gain constant */
#define  NumOfGainLev      24

/* FER constant */
#define  ErrMaxNum         3

/* CNG constants  */
#define NbAvAcf            3  /* Nb of frames for Acf average               */
#define NbAvGain           3  /* Nb of frames for gain average              */
#define ThreshGain         3  /* Theshold for quantized gains               */
#define FracThresh         7000   /* Itakura dist threshold: frac. part     */
#define NbPulsBlk          11 /* Nb of pulses in 2-subframes blocks         */

#define InvNbPulsBlk       2979 /* 32768/NbPulsBlk                          */
#define NbFilt             50 /* number of filters for CNG exc generation   */
#define LpcOrderP1         (LpcOrder+1)
#define SizAcf             ((NbAvAcf+1)*LpcOrderP1) /* size of array Acf    */
#define SubFrLenD          (2*SubFrLen)
#define Gexc_Max           5000  /* Maximum gain for fixed CNG excitation   */

/* Taming constants */
#define NbFilt085_min      51
#define NbFilt170_min      93
#define SizErr             5
#define Err0               (Word32)4  /* scaling factor */
#define ThreshErr          0x40000000L
#define DEC                (30 - 7)

/*
   Used structures
*/
typedef  struct   {
   /* High pass variables */
   Word16   HpfZdl   ;
   Word32   HpfPdl   ;

   /* Sine wave detector */
   Word16   SinDet   ;

   /* Lsp previous vector */
   Word16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Word16   PrevWgt[PitchMax] ;
   Word16   PrevErr[PitchMax] ;
   Word16   PrevExc[PitchMax] ;

   /* Required memory for the delay */
   Word16   PrevDat[LpcFrame-SubFrLen] ;

   /* Used delay lines */
   Word16   WghtFirDl[LpcOrder] ;
   Word16   WghtIirDl[LpcOrder] ;
   Word16   RingFirDl[LpcOrder] ;
   Word16   RingIirDl[LpcOrder] ;

   /* Taming procedure errors */
   Word32 Err[SizErr];

   } CODSTATDEF  ;

typedef  struct   {
   Word16   Ecount ;
   Word16   InterGain ;
   Word16   InterIndx ;
   Word16   Rseed ;
   Word16   Park  ;
   Word16   Gain  ;
   /* Lsp previous vector */
   Word16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Word16   PrevExc[PitchMax] ;

   /* Used delay lines */
   Word16   SyntIirDl[LpcOrder] ;
   Word16   PostFirDl[LpcOrder] ;
   Word16   PostIirDl[LpcOrder] ;

   } DECSTATDEF  ;

   /* subframe coded parameters */
typedef  struct   {
   Word16   AcLg  ;
   Word16   AcGn  ;
   Word16   Mamp  ;
   Word16   Grid  ;
   Word16   Tran  ;
   Word16   Pamp  ;
   Word32   Ppos  ;
   } SFSDEF ;

   /* frame coded parameters */
typedef  struct   {
   Word16   Crc   ;
   Word32   LspId ;
   Word16   Olp[SubFrames/2] ;
   SFSDEF   Sfs[SubFrames] ;
   } LINEDEF ;

   /* harmonic noise shaping filter parameters */
typedef  struct   {
   Word16   Indx  ;
   Word16   Gain  ;
   } PWDEF  ;

    /* pitch postfilter parameters */
typedef  struct   {
   Word16   Indx  ;
   Word16   Gain  ;
   Word16   ScGn  ;
   } PFDEF  ;

    /* best excitation vector parameters for the high rate */
typedef  struct {
   Word32   MaxErr   ;
   Word16   GridId   ;
   Word16   MampId   ;
   Word16   UseTrn   ;
   Word16   Ploc[MaxPulseNum] ;
   Word16   Pamp[MaxPulseNum] ;
   } BESTDEF ;

    /* VAD static variables */
typedef struct {
    Word16  Hcnt ;
    Word16  Vcnt ;
    Word32  Penr ;
    Word32  Nlev ;
    Word16  Aen ;
    Word16  Polp[4] ;
    Word16  NLpc[LpcOrder] ;
} VADSTATDEF ;


/* CNG features */

/* Coder part */
typedef struct {
    Word16 CurGain;
    Word16 PastFtyp;
    Word16 Acf[SizAcf];
    Word16 ShAcf[NbAvAcf+1];
    Word16 LspSid[LpcOrder] ;
    Word16 SidLpc[LpcOrder] ;
    Word16 RC[LpcOrderP1];
    Word16 ShRC;
    Word16 Ener[NbAvGain];
    Word16 NbEner;
    Word16 IRef;
    Word16 SidGain;
    Word16 RandSeed;
} CODCNGDEF;

/* Decoder part */
typedef struct {
    Word16 CurGain;
    Word16 PastFtyp;
    Word16 LspSid[LpcOrder] ;
    Word16 SidGain;
    Word16 RandSeed;
} DECCNGDEF;
#endif
