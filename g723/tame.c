
/*
**
** File:    tame.c
**
** Description: Functions used to avoid possible explosion of the decoder
**              excitation in case of series of long term unstable filters
**              and when the encoder and the decoder are de-synchronized
**
** Functions:
**
**  Computing excitation error estimation :
**      Update_Err()
**  Test excitation error :
**      Test_Err()
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.0
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#include <stdlib.h>
#include <stdio.h>

#include "lbccodec.h"
#include "g723_const.h"

extern Flag  UseHp;
extern Flag  UsePf;
extern Flag  UseVx;
extern enum  Wmode    WrkMode;
extern enum  Crate    WrkRate;
extern Word16   tabgain85[85];
extern Word16   tabgain170[170];
extern  CODSTATDEF  CodStat  ;

extern Word32 L_mls( Word32, Word16 ) ;        /* Wght ?? */
extern Word32 L_g723_add(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
extern Word16 g723_mult(Word16 var1, Word16 var2);    /* Short mult,          1 */
extern Word16 g723_add(Word16 var1, Word16 var2);     /* Short add,           1 */
extern Word16 g723_sub(Word16 var1, Word16 var2);     /* Short sub,           1 */

#define NbPulsBlk          11 /* Nb of pulses in 2-subframes blocks         */

extern Word16 g723_shl(Word16 var1, Word16 var2);     /* Short shift left,    1 */
extern Word32 L_g723_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
extern Word32 L_g723_sub(Word32 L_var1, Word32 L_var2);   /* Long sub,        2 */
extern Word32 L_g723_negate(Word32 L_var1);               /* Long negate,     2 */
extern Word32 L_g723_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
extern Word16 g723_extract_l(Word32 L_var1);          /* Extract low,         1 */
/*
**
** Function:        Update_Err()
**
** Description:     Estimation of the excitation error associated
**                  to the excitation signal when it is disturbed at
**                  the decoder, the disturbing signal being filtered
**                  by the long term synthesis filters
**                  one value for (SubFrLen/2) samples
**                  Updates the table CodStat.Err
**
** Links to text:   Section
**
** Arguments:
**
**  Word16 Olp      Center value for pitch delay
**  Word16 AcLg     Offset value for pitch delay
**  Word16 AcGn     Index of Gain LT filter
**
** Outputs: None
**
** Return value:    None
**
*/

void Update_Err(
    Word16 Olp, Word16 AcLg, Word16 AcGn
)
{
    Word16 *ptr_tab;
    Word16 i, iz, temp1, temp2;
    Word16 Lag;
    Word32 Worst1, Worst0, L_temp;
    Word16 beta;

    Lag = Olp - (Word16)Pstep + AcLg;

    /* Select Quantization tables */
    i = 0 ;
    ptr_tab = tabgain85;
    if ( WrkRate == Rate63 ) {
        if ( Olp >= (Word16) (SubFrLen-2) ) ptr_tab = tabgain170;
    }
    else {
        ptr_tab = tabgain170;
    }
    beta = ptr_tab[(int)AcGn]; /* beta = gain * 8192 */


    if(Lag <= (SubFrLen/2)) {
        Worst0 = L_mls(CodStat.Err[0], beta);
        Worst0 = L_g723_shl(Worst0, 2);
        Worst0 = L_g723_add(Err0, Worst0);
        Worst1 = Worst0;
    }

    else {
        iz = g723_mult(Lag, 1092);   /* Lag / 30 */
        temp1 = g723_add(iz, 1);
        temp2 = g723_sub(g723_shl(temp1, 5), g723_shl(temp1, 1));      /* 30 (iz+1) */
        if(temp2 != Lag) {

            if(iz == 1) {
                Worst0 = L_mls(CodStat.Err[0], beta);
                Worst0 = L_g723_shl(Worst0, 2);
                Worst0 = L_g723_add(Err0, Worst0);
                Worst1 = L_mls(CodStat.Err[1], beta);
                Worst1 = L_g723_shl(Worst1, 2);
                Worst1 = L_g723_add(Err0, Worst1);
                if(Worst0 > Worst1) Worst1 = Worst0;
                else Worst0 = Worst1;
            }

            else {
                Worst0 = L_mls(CodStat.Err[iz-2], beta);
                Worst0 = L_g723_shl(Worst0, 2);
                Worst0 = L_g723_add(Err0, Worst0);
                L_temp = L_mls(CodStat.Err[iz-1], beta);
                L_temp = L_g723_shl(L_temp, 2);
                L_temp = L_g723_add(Err0, L_temp);
                if(L_temp > Worst0) Worst0 = L_temp;
                Worst1 = L_mls(CodStat.Err[iz], beta);
                Worst1 = L_g723_shl(Worst1, 2);
                Worst1 = L_g723_add(Err0, Worst1);
                if(L_temp > Worst1) Worst1 = L_temp;
            }
        }
        else {  /* Lag % SubFrLen = 0 */
            Worst0 = L_mls(CodStat.Err[iz-1], beta);
            Worst0 = L_g723_shl(Worst0, 2);
            Worst0 = L_g723_add(Err0, Worst0);
            Worst1 = L_mls(CodStat.Err[iz], beta);
            Worst1 = L_g723_shl(Worst1, 2);
            Worst1 = L_g723_add(Err0, Worst1);
        }
    }

    for(i=4; i>=2; i--) {
        CodStat.Err[i] = CodStat.Err[i-2];
    }
    CodStat.Err[0] = Worst0;
    CodStat.Err[1] = Worst1;

    return;
}

/*
**
** Function:        Test_Err()
**
** Description:     Check the error excitation maximum for
**                  the subframe and computes an index iTest used to
**                  calculate the maximum nb of filters (in Find_Acbk) :
**                  Bound = Min(Nmin + iTest x pas, Nmax) , with
**                  AcbkGainTable085 : pas = 2, Nmin = 51, Nmax = 85
**                  AcbkGainTable170 : pas = 4, Nmin = 93, Nmax = 170
**                  iTest depends on the relative difference between
**                  errmax and a fixed threshold
**
** Links to text:   Section
**
** Arguments:
**
**  Word16 Lag1      1st long term Lag of the tested zone
**  Word16 Lag2      2nd long term Lag of the tested zone
**
** Outputs: None
**
** Return value:
**  Word16          index iTest used to compute Acbk number of filters
*/

Word16 Test_Err(
    Word16 Lag1, Word16 Lag2
)
{

    int i, i1, i2;
    Word16 zone1, zone2;
    Word32 Acc, Err_max;
    Word16 iTest;

    i2 = Lag2 + ClPitchOrd/2;
    zone2 = g723_mult( (Word16) i2, (Word16) 1092);

    i1 = - SubFrLen + 1 + Lag1 - ClPitchOrd/2;
    if(i1 <= 0) i1 = 1;
    zone1 = g723_mult( (Word16) i1, (Word16) 1092);

    Err_max = -1L;
    for(i=zone2; i>=zone1; i--) {
        Acc = L_g723_sub(CodStat.Err[i], Err_max);
        if(Acc > 0L) {
                Err_max = CodStat.Err[i];
        }
    }
    Acc = L_g723_sub(Err_max, ThreshErr);
    if((Acc > 0L) || (CodStat.SinDet < 0 ) ) {
        iTest = 0;
    }
    else {
        Acc = L_g723_negate(Acc);
        Acc = L_g723_shr(Acc, DEC);
        iTest = g723_extract_l(Acc);
    }

    return(iTest);
}
