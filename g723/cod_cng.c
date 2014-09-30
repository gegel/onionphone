/*
**
** File:            "cod_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the encoder part
**
** Functions:       Init_Cod_Cng()
**                  Cod_Cng()
**                  Update_Cng()
**
** Local functions:
**                  ComputePastAvFilter()
**                  CalcRC()
**                  LpcDiff()
**
**
*/
/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.2
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
	Last modified : March 2006
*/

#include <stdio.h>
#include <stdlib.h>

#include "g723_const.h"

extern  Word16  Durbin( Word16 *Lpc, Word16 *Corr, Word16 Err, Word16 *Pk2 );
extern  Word16 Qua_SidGain(Word16 *Ener, Word16 *shEner, Word16 nq);
extern  Word16 g723_abs_s(Word16 var1);                /* Short abs,           1 */
extern  Word16 g723_sub(Word16 var1, Word16 var2); /* Short sub,           1 */

extern VADSTATDEF  VadStat ;
extern  CODSTATDEF  CodStat  ;

extern Word16 g723_shr(Word16 var1, Word16 var2);     /* Short shift right,   1 */

#define NbPulsBlk          11 /* Nb of pulses in 2-subframes blocks         */

extern  void AtoLsp( Word16 *LspVect, Word16 *Lpc, Word16 *PrevLsp );
extern  Word32   Lsp_Qnt( Word16 *CurrLsp, Word16 *PrevLsp );
extern  void Lsp_Inq( Word16 *Lsp, Word16 *PrevLsp, Word32 LspId, Word16 Crc );
extern  Word16 Dec_SidGain(Word16 i_gain);
extern  Word16 g723_extract_h(Word32 L_var1);          /* Extract high,        1 */
extern  Word32 L_g723_add(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
extern  Word32 L_g723_mult(Word16 var1, Word16 var2);  /* Long mult,           1 */
extern  void Calc_Exc_Rand(Word16 cur_gain, Word16 *PrevExc, Word16 *DataExc,Word16 *nRandom, LINEDEF *Line);
extern  void  Lsp_Int( Word16 *QntLpc, Word16 *CurrLsp, Word16 *PrevLsp );
extern  Word16 g723_add(Word16 var1, Word16 var2);     /* Short add,           1 */
extern  Word32 g723_L_deposit_l(Word16 var1);       /* 16 bit var1 -> LSB,     2 */
extern  Word32 L_g723_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
extern  Word16 g723_norm_l(Word32 L_var1);           /* Long norm,            30 */
extern  Word16 g723_extract_l(Word32 L_var1);          /* Extract low,         1 */
extern  Word32 L_g723_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
extern  Word32 g723_L_mac(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
extern  Word16 round_(Word32 L_var1);              /* Round,               1 */
extern  Word16 g723_mult_r(Word16 var1, Word16 var2);  /* Mult with round,     2 */

/* Declaration of local functions */
static void ComputePastAvFilter(Word16 *Coeff);
static void CalcRC(Word16 *Coeff, Word16 *RC, Word16 *shRC);
static Flag LpcDiff(Word16 *RC, Word16 shRC, Word16 *Acf, Word16 alpha);

/* Global Variables */
CODCNGDEF CodCng;

/*
**
** Function:        Init_Cod_Cng()
**
** Description:     Initialize Cod_Cng static variables
**
** Links to text:
**
** Arguments:       None
**
** Outputs:         None
**
** Return value:    None
**
*/
void Init_Cod_Cng(void)
{
    int i;

    CodCng.CurGain = 0;

    for(i=0; i< SizAcf; i++) CodCng.Acf[i] = 0;

    for(i=0; i <= NbAvAcf; i++) CodCng.ShAcf[i] = 40;

    for(i=0; i < LpcOrder; i++) CodCng.SidLpc[i] = 0;

    CodCng.PastFtyp = 1;

    CodCng.RandSeed = 12345;

    return;
}


/*
**
** Function:           Cod_Cng()
**
** Description:        Computes Ftyp for inactive frames
**                              0  :  for untransmitted frames
**                              2  :  for SID frames
**                     Computes current frame excitation
**                     Computes current frame LSPs
**                     Computes the coded parameters of SID frames
**
** Links to text:
**
** Arguments:
**
**  Word16 *DataExc    Current frame synthetic excitation
**  Word16 *Ftyp     Characterizes the frame type for CNG
**  LINEDEF *Line      Quantized parameters (used for SID frames)
**  Word16 *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  Word16 *DataExc
**  Word16 *Ftyp
**  LINEDEF *Line
**  Word16 *QntLpc
**
** Return value:       None
**
*/
void Cod_Cng(Word16 *DataExc, Word16 *Ftyp, LINEDEF *Line, Word16 *QntLpc)
{


    Word16 curCoeff[LpcOrder];
    Word16 curQGain;
    Word16 temp;
    int i;

 /*
  * Update Ener
  */
    for(i=NbAvGain-1; i>=1; i--) {
        CodCng.Ener[i] = CodCng.Ener[i-1];
    }

 /*
  * Compute LPC filter of present frame
  */
    CodCng.Ener[0] = Durbin(curCoeff, &CodCng.Acf[1], CodCng.Acf[0], &temp);

 /*
  * if first frame of silence => SID frame
  */
    if(CodCng.PastFtyp == 1) {
        *Ftyp = 2;
        CodCng.NbEner = 1;
        curQGain = Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);
    }

    else {
        CodCng.NbEner++;
        if(CodCng.NbEner > NbAvGain) CodCng.NbEner = NbAvGain;
        curQGain = Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);

 /*
  * Compute stationarity of current filter
  * versus reference filter
  */
        if(LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *CodCng.Ener) == 0) {
            /* transmit SID frame */
            *Ftyp = 2;
        }
        else {
            temp = g723_abs_s(g723_sub(curQGain, CodCng.IRef));
            if(temp > ThreshGain) {
                *Ftyp = 2;
            }
            else {
                /* no transmission */
                *Ftyp = 0;
            }
        }
    }

 /*
  * If SID frame : Compute SID filter
  */
    if(*Ftyp == 2) {

 /*
  * Evaluates local stationnarity :
  * Computes difference between current filter and past average filter
  * if signal not locally stationary SID filter = current filter
  * else SID filter = past average filter
  */
        /* Compute past average filter */
        ComputePastAvFilter(CodCng.SidLpc) ;

        /* If adaptation enabled, fill noise filter */
        if ( !VadStat.Aen ) {
            for(i=0; i<LpcOrder; i++) VadStat.NLpc[i] = CodCng.SidLpc[i];
        }

        /* Compute autocorr. of past average filter coefficients */
        CalcRC(CodCng.SidLpc , CodCng.RC, &CodCng.ShRC);

        if(LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *CodCng.Ener) == 0){
            for(i=0; i<LpcOrder; i++) {
                CodCng.SidLpc[i] = curCoeff[i];
            }
            CalcRC(curCoeff, CodCng.RC, &CodCng.ShRC);
        }

 /*
  * Compute SID frame codes
  */
        /* Compute LspSid */
        AtoLsp(CodCng.LspSid, CodCng.SidLpc, CodStat.PrevLsp);
        Line->LspId = Lsp_Qnt(CodCng.LspSid, CodStat.PrevLsp);
        Lsp_Inq(CodCng.LspSid, CodStat.PrevLsp, Line->LspId, 0);

        Line->Sfs[0].Mamp = curQGain;
        CodCng.IRef = curQGain;
        CodCng.SidGain = Dec_SidGain(CodCng.IRef);

    } /* end of Ftyp=2 case (SID frame) */

 /*
  * Compute new excitation
  */
    if(CodCng.PastFtyp == 1) {
        CodCng.CurGain = CodCng.SidGain;
    }
    else {
          CodCng.CurGain = g723_extract_h(L_g723_add( L_g723_mult(CodCng.CurGain,0x7000),
                    L_g723_mult(CodCng.SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(CodCng.CurGain, CodStat.PrevExc, DataExc,
                                                &CodCng.RandSeed, Line);

 /*
  * Interpolate LSPs and update PrevLsp
  */
    Lsp_Int(QntLpc, CodCng.LspSid, CodStat.PrevLsp);
    for (i=0; i < LpcOrder ; i++) {
        CodStat.PrevLsp[i] = CodCng.LspSid[i];
    }

 /*
  * Output & save frame type info
  */
    CodCng.PastFtyp = *Ftyp;
    return;
}

/*
**
** Function:           Update_Acf()
**
** Description:        Computes & Stores sums of subframe-acfs
**
** Links to text:
**
** Arguments:
**
**  Word16 *Acf_sf     sets of subframes Acfs of current frame
**  Word16 *ShAcf_sf   corresponding scaling factors
**
** Output :            None
**
** Return value:       None
**
*/
void Update_Acf(Word16 *Acf_sf, Word16 *ShAcf_sf)
{

    int i, i_subfr;
    Word16 *ptr1, *ptr2;
    Word32 L_temp[LpcOrderP1];
    Word16 sh1, temp;
    Word32 L_acc0;

    /* Update Acf and ShAcf */
    ptr2 = CodCng.Acf + SizAcf;
    ptr1 = ptr2 - LpcOrderP1;
    for(i=LpcOrderP1; i<SizAcf; i++) *(--ptr2) = *(--ptr1);
    for(i=NbAvAcf; i>=1; i--) CodCng.ShAcf[i] = CodCng.ShAcf[i-1];

    /* Search ShAcf_sf min for current frame */
    sh1 = ShAcf_sf[0];
    for(i_subfr=1; i_subfr<SubFrames; i_subfr++) {
        if(ShAcf_sf[i_subfr] < sh1) sh1 = ShAcf_sf[i_subfr];
    }
    sh1 = g723_add(sh1, 14);  /* 2 bits of margin */

    /* Compute current sum of acfs */
    for(i=0; i<= LpcOrder; i++) L_temp[i] = 0;

    ptr2 = Acf_sf;
    for(i_subfr=0; i_subfr<SubFrames; i_subfr++) {
        temp = g723_sub(sh1, ShAcf_sf[i_subfr]);
        for(i=0; i <= LpcOrder; i++) {
            L_acc0 = g723_L_deposit_l(*ptr2++);
            L_acc0 = L_g723_shl(L_acc0, temp);  /* shift right if temp<0 */
            L_temp[i] = L_g723_add(L_temp[i], L_acc0);
        }
    }
    /* Normalize */
    temp = g723_norm_l(L_temp[0]);
    temp = g723_sub(16, temp);
    if(temp < 0) temp = 0;
    for(i=0; i <= LpcOrder; i++) {
        CodCng.Acf[i] = g723_extract_l(L_g723_shr(L_temp[i],temp));
    }

    CodCng.ShAcf[0] = g723_sub(sh1, temp);

    return;
}

/*
**
** Function:           ComputePastAvFilter()
**
** Description:        Computes past average filter
**
** Links to text:
**
** Argument:
**
**  Word16 *Coeff      set of LPC coefficients
**
** Output:
**
**  Word16 *Coeff
**
** Return value:       None
**
*/
void ComputePastAvFilter(Word16 *Coeff)
{
    int i, j;
    Word16 *ptr_Acf;
    Word32 L_sumAcf[LpcOrderP1];
    Word16 Corr[LpcOrder], Err;
    Word16 sh1, temp;
    Word32 L_acc0;

    /* Search ShAcf min */
    sh1 = CodCng.ShAcf[1];
    for(i=2; i <= NbAvAcf; i ++) {
        temp = CodCng.ShAcf[i];
        if(temp < sh1) sh1 = temp;
    }
    sh1 = g723_add(sh1, 14);     /* 2 bits of margin : NbAvAcf <= 4 */

    /* Compute sum of NbAvAcf frame-Acfs  */
    for(j=0; j <= LpcOrder; j++) L_sumAcf[j] = 0;

    ptr_Acf = CodCng.Acf + LpcOrderP1;
    for(i=1; i <= NbAvAcf; i ++) {
        temp = g723_sub(sh1, CodCng.ShAcf[i]);
        for(j=0; j <= LpcOrder; j++) {
            L_acc0 = g723_L_deposit_l(*ptr_Acf++);
            L_acc0 = L_g723_shl(L_acc0, temp); /* shift right if temp<0 */
            L_sumAcf[j] = L_g723_add(L_sumAcf[j], L_acc0);
        }
    }

    /* Normalize */
    temp = g723_norm_l(L_sumAcf[0]);
    temp = g723_sub(16, temp);
    if(temp < 0) temp = 0;
    Err = g723_extract_l(L_g723_shr(L_sumAcf[0],temp));
    for(i=1; i<LpcOrderP1; i++) {
        Corr[i-1] = g723_extract_l(L_g723_shr(L_sumAcf[i],temp));
    }

    Durbin(Coeff, Corr, Err, &temp);

    return;
}

/*
**
** Function:           CalcRC()
**
** Description:        Computes function derived from
**                     the autocorrelation of LPC coefficients
**                     used for Itakura distance
**
** Links to text:
**
** Arguments :
**
**  Word16 *Coeff      set of LPC coefficients
**  Word16 *RC         derived from LPC coefficients autocorrelation
**  Word16 *ShRC       corresponding scaling factor
**
** Outputs :
**
**  Word16 *RC
**  Word16 *ShRC
**
** Return value:       None
**
*/
void CalcRC(Word16 *Coeff, Word16 *RC, Word16 *ShRC)
{
    int i, j;
    Word16 sh1;
    Word32 L_acc;

    L_acc = 0L;
    for(j=0; j<LpcOrder; j++) {
        L_acc = g723_L_mac(L_acc, Coeff[j], Coeff[j]);
    }
    L_acc = L_g723_shr(L_acc, 1);
    L_acc = L_g723_add(L_acc, 0x04000000L);  /* 1 << 2 * Lpc_justif. */
    sh1 = g723_norm_l(L_acc) - (Word16)2;    /* 1 bit because of x2 in RC[i], i> 0*/
                                /* & 1 bit margin for Itakura distance */
    L_acc = L_g723_shl(L_acc, sh1); /* shift right if < 0 */
    RC[0] = round_(L_acc);

    for(i=1; i<=LpcOrder; i++) {
        L_acc = L_g723_mult( (Word16) 0xE000, Coeff[i-1]);   /* - (1 << Lpc_justif.) */
        for(j=0; j<LpcOrder-i; j++) {
            L_acc = g723_L_mac(L_acc, Coeff[j], Coeff[j+i]);
        }
        L_acc = L_g723_shl(L_acc, sh1);
        RC[i] = round_(L_acc);
    }
    *ShRC = sh1;
    return;
}

/*
**
** Function:           LpcDiff()
**
** Description:        Comparison of two filters
**                     using Itakura distance
**                     1st filter : defined by *ptrAcf
**                     2nd filter : defined by *RC
**                     the autocorrelation of LPC coefficients
**                     used for Itakura distance
**
** Links to text:
**
** Arguments :
**
**  Word16 *RC         derived from LPC coefficients autocorrelation
**  Word16 ShRC        corresponding scaling factor
**  Word16 *ptrAcf     pointer on signal autocorrelation function
**  Word16 alpha       residual energy in LPC analysis using *ptrAcf
**
** Output:             None
**
** Return value:       flag = 1 if similar filters
**                     flag = 0 if different filters
**
*/
Flag LpcDiff(Word16 *RC, Word16 ShRC, Word16 *ptrAcf, Word16 alpha)
{
    Word32 L_temp0, L_temp1;
    Word16 temp;
    int i;
    Flag diff;

    L_temp0 = 0L;
    for(i=0; i<=LpcOrder; i++) {
        temp = g723_shr(ptrAcf[i], 2);  /* + 2 margin bits */
        L_temp0 = g723_L_mac(L_temp0, RC[i], temp);
    }

    temp = g723_mult_r(alpha, FracThresh);
    L_temp1 = L_g723_add((Word32)temp, (Word32)alpha);
    temp = g723_add(ShRC, 9);  /* 9 = Lpc_justif. * 2 - 15 - 2 */
    L_temp1 = L_g723_shl(L_temp1, temp);

    if(L_temp0 <= L_temp1) diff = 1; 	/* G723.1 maintenance April 2006*/
    									/* Before : if(L_temp0 < L_temp1) diff = 1; */
    else diff = 0;
    return(diff);
}

