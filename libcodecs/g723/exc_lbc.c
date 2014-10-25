/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:    exc_lbc.c
**
** Description: Functions that implement adaptive and fixed codebook
**       operations.
**
** Functions:
**
**  Computing Open loop Pitch lag:
**
**      Estim_Pitch()
**
**  Harmonic noise weighting:
**
**      Comp_Pw()
**      Filt_Pw()
**
**  Fixed Cobebook computation:
**
**      Find_Fcbk()
**      Gen_Trn()
**      Find_Best()
**      Find_Pack()
**      Find_Unpk()
**      ACELP_LBC_code()
**      Cor_h()
**      Cor_h_X()
**      reset_max_time()
**      D4i64_LBC()
**      G_code()
**      search_T0()
**
**  Adaptive Cobebook computation:
**
**      Find_Acbk()
**      Get_Rez()
**      Decod_Acbk()
**
**  Pitch postfilter:
**      Comp_Lpf()
**      Find_B()
**      Find_F()
**      Filt_Lpf()
**
**  Residual interpolation:
**
**      Comp_Info()
**      Regen()
**
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ophtools.h>

#include "g723_const.h"
#include "lbccodec.h"
extern Word32 g723_L_mac(Word32 L_var3, Word16 var1, Word16 var2);	/* Mac,    1 */
extern Word16 g723_sub(Word16 var1, Word16 var2);	/* Short sub,           1 */
extern Word32 g723_L_msu(Word32 L_var3, Word16 var1, Word16 var2);	/* Msu,    1 */
extern Word16 g723_norm_l(Word32 L_var1);	/* Long norm,            30 */
extern Word32 L_g723_shl(Word32 L_var1, Word16 var2);	/* Long shift left,     2 */
extern Word16 g723_shl(Word16 var1, Word16 var2);	/* Short shift left,    1 */
//extern Word32 L_g723_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
extern Word16 round_(Word32 L_var1);	/* Round,               1 */
extern Word16 g723_mult_r(Word16 var1, Word16 var2);	/* Mult with round,     2 */
//extern Word16 shr_r(Word16 var1, Word16 var2);/* Shift right with round, 2 */
//extern Word16 g723_mac_r(Word32 L_var3, Word16 var1, Word16 var2);
//extern Word16 g723_msu_r(Word32 L_var3, Word16 var1, Word16 var2);
extern Word32 L_g723_mult(Word16 var1, Word16 var2);	/* Long mult,           1 */
extern Word16 g723_add(Word16 var1, Word16 var2);	/* Short add,           1 */
extern Word32 L_g723_add(Word32 L_var1, Word32 L_var2);	/* Long add,        2 */
extern Word16 g723_extract_h(Word32 L_var1);	/* Extract high,        1 */
extern Word16 g723_shr(Word16 var1, Word16 var2);	/* Short shift right,   1 */
//extern Word16 shr_r(Word16 var1, Word16 var2);/* Shift right with round, 2 */
extern Word32 L_g723_negate(Word32 L_var1);	/* Long negate,     2 */
extern Word32 L_g723_shr(Word32 L_var1, Word16 var2);	/* Long shift right,    2 */
extern Word32 g723_L_abs(Word32 L_var1);	/* Long abs,              3 */
extern Word16 g723_mult_r(Word16 var1, Word16 var2);	/* Mult with round,     2 */
extern Word32 L_g723_sub(Word32 L_var1, Word32 L_var2);	/* Long sub,        2 */
extern Word16 div_s(Word16 var1, Word16 var2);	/* Short division,       18 */
extern Word32 g723_L_deposit_h(Word16 var1);	/* 16 bit var1 -> MSB,     2 */
extern Word16 i_g723_mult(Word16 a, Word16 b);

extern Word16 g723_mult(Word16 var1, Word16 var2);	/* Short mult,          1 */
extern Word16 Test_Err(Word16 Lag1, Word16 Lag2);

//extern void  Gen_Trn( Word16 *Dst, Word16 *Src, Word16 Olp );
//extern Word16 search_T0 ( Word16 T0, Word16 Gid, Word16 *gain_T0);
extern Word16 g723_extract_l(Word32 L_var1);	/* Extract low,         1 */
extern Word16 g723_negate(Word16 var1);	/* Short negate,        1 */
extern Word16 g723_abs_s(Word16 var1);	/* Short abs,           1 */

extern Word32 CombinatorialTable[MaxPulseNum][SubFrLen / Sgrid];
extern Word16 Nb_puls[4];
extern Word32 MaxPosTable[4];
extern Word16 LpfConstTable[2];
extern Word16 FcbkGainTable[NumOfGainLev];
extern Word16 epsi170[170];
extern Word16 gain170[170];
extern Word16 tabgain170[170];

extern Word16 div_l(Word32, Word16);
extern Word16 Sqrt_lbc(Word32 Num);

extern Word16 Rand_lbc(Word16 * p);
extern Word16 Vec_Norm(Word16 * Vect, Word16 Len);
extern Word16 *AcbkGainTablePtr[2];
extern enum Crate WrkRate;

Word16 Estim_Pitch(Word16 * Dpnt, Word16 Start);
PWDEF Comp_Pw(Word16 * Dpnt, Word16 Start, Word16 Olp);
void Filt_Pw(Word16 * DataBuff, Word16 * Dpnt, Word16 Start, PWDEF Pw);
void Find_Fcbk(Word16 * Dpnt, Word16 * ImpResp, LINEDEF * Line, Word16 Sfc);
void Gen_Trn(Word16 * Dst, Word16 * Src, Word16 Olp);
void Find_Best(BESTDEF * Best, Word16 * Tv, Word16 * ImpResp, Word16 Np,
	       Word16 Olp);
void Fcbk_Pack(Word16 * Dpnt, SFSDEF * Sfs, BESTDEF * Best, Word16 Np);
void Fcbk_Unpk(Word16 * Tv, SFSDEF Sfs, Word16 Olp, Word16 Sfc);
void Find_Acbk(Word16 * Tv, Word16 * ImpResp, Word16 * PrevExc, LINEDEF
	       * Line, Word16 Sfc);
void Get_Rez(Word16 * Tv, Word16 * PrevExc, Word16 Lag);
void Decod_Acbk(Word16 * Tv, Word16 * PrevExc, Word16 Olp, Word16 Lid,
		Word16 Gid);
Word16 Comp_Info(Word16 * Buff, Word16 Olp, Word16 * Gain, Word16 * ShGain);
void Regen(Word16 * DataBuff, Word16 * Buff, Word16 Lag, Word16 Gain,
	   Word16 Ecount, Word16 * Sd);
PFDEF Comp_Lpf(Word16 * Buff, Word16 Olp, Word16 Sfc);
Word16 Find_B(Word16 * Buff, Word16 Olp, Word16 Sfc);
Word16 Find_F(Word16 * Buff, Word16 Olp, Word16 Sfc);
PFDEF Get_Ind(Word16 Ind, Word16 Ten, Word16 Ccr, Word16 Enr);
void Filt_Lpf(Word16 * Tv, Word16 * Buff, PFDEF Pf, Word16 Sfc);
void reset_max_time(void);
Word16 search_T0(Word16 T0, Word16 Gid, Word16 * gain_T0);
Word16 ACELP_LBC_code(Word16 X[], Word16 h[], Word16 T0, Word16 code[],
		      Word16 * gain, Word16 * shift, Word16 * sign,
		      Word16 gain_T0);
void Cor_h(Word16 * H, Word16 * rr);
void Cor_h_X(Word16 h[], Word16 X[], Word16 D[]);
Word16 D4i64_LBC(Word16 Dn[], Word16 rr[], Word16 h[], Word16 cod[],
		 Word16 y[], Word16 * code_shift, Word16 * sign);
Word16 G_code(Word16 X[], Word16 Y[], Word16 * gain_q);

/*
**
** Function:        Estim_Pitch()
**
** Description: Open loop pitch estimation made twice per frame (one for
**              the first two subframes and one for the last two).
**              The method is based on the maximization of the
**              crosscorrelation of the speech.
**
** Links to text:   Section 2.9
**
** Arguments:
**
**  Word16 *Dpnt    Perceptually weighted speech
**  Word16 Start    Starting index defining the subframes under study
**
** Outputs:
**
** Return value:
**
**  Word16      Open loop pitch period
**
*/
Word16 Estim_Pitch(Word16 * Dpnt, Word16 Start)
{
	int i, j;

	Word32 Acc0, Acc1;

	Word16 Exp, Tmp;
	Word16 Ccr, Enr;

	Word16 Indx = (Word16) PitchMin;

	Word16 Mxp = (Word16) 30;
	Word16 Mcr = (Word16) 0x4000;
	Word16 Mnr = (Word16) 0x7fff;

	Word16 Pr;

	/* Init the energy estimate */
	Pr = Start - (Word16) PitchMin + (Word16) 1;
	Acc1 = (Word32) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc1 = g723_L_mac(Acc1, Dpnt[Pr + j], Dpnt[Pr + j]);

	/* Main Olp search loop */
	for (i = PitchMin; i <= PitchMax - 3; i++) {

		Pr = g723_sub(Pr, (Word16) 1);

		/* Energy update */
		Acc1 =
		    g723_L_msu(Acc1, Dpnt[Pr + 2 * SubFrLen],
			       Dpnt[Pr + 2 * SubFrLen]);
		Acc1 = g723_L_mac(Acc1, Dpnt[Pr], Dpnt[Pr]);

		/*  Compute the cross */
		Acc0 = (Word32) 0;
		for (j = 0; j < 2 * SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, Dpnt[Start + j], Dpnt[Pr + j]);

		if (Acc0 > (Word32) 0) {

			/* Compute Exp and mant of the cross */
			Exp = g723_norm_l(Acc0);
			Acc0 = L_g723_shl(Acc0, Exp);
			Exp = g723_shl(Exp, (Word16) 1);
			Ccr = round_(Acc0);
			Acc0 = L_g723_mult(Ccr, Ccr);
			Ccr = g723_norm_l(Acc0);
			Acc0 = L_g723_shl(Acc0, Ccr);
			Exp = g723_add(Exp, Ccr);
			Ccr = g723_extract_h(Acc0);

			/* Do the same with energy */
			Acc0 = Acc1;
			Enr = g723_norm_l(Acc0);
			Acc0 = L_g723_shl(Acc0, Enr);
			Exp = g723_sub(Exp, Enr);
			Enr = round_(Acc0);

			if (Ccr >= Enr) {
				Exp = g723_sub(Exp, (Word16) 1);
				Ccr = g723_shr(Ccr, (Word16) 1);
			}

			if (Exp <= Mxp) {

				if ((Exp + 1) < Mxp) {
					Indx = (Word16) i;
					Mxp = Exp;
					Mcr = Ccr;
					Mnr = Enr;
					continue;
				}

				if ((Exp + 1) == Mxp)
					Tmp = g723_shr(Mcr, (Word16) 1);
				else
					Tmp = Mcr;

				/* Compare with equal exponents */
				Acc0 = L_g723_mult(Ccr, Mnr);
				Acc0 = g723_L_msu(Acc0, Enr, Tmp);
				if (Acc0 > (Word32) 0) {

					if (((Word16) i - Indx) <
					    (Word16) PitchMin) {
						Indx = (Word16) i;
						Mxp = Exp;
						Mcr = Ccr;
						Mnr = Enr;
					}

					else {
						Acc0 = L_g723_mult(Ccr, Mnr);
						Acc0 =
						    L_g723_negate(L_g723_shr
								  (Acc0,
								   (Word16) 2));
						Acc0 =
						    g723_L_mac(Acc0, Ccr, Mnr);
						Acc0 =
						    g723_L_msu(Acc0, Enr, Tmp);
						if (Acc0 > (Word32) 0) {
							Indx = (Word16) i;
							Mxp = Exp;
							Mcr = Ccr;
							Mnr = Enr;
						}
					}
				}
			}
		}
	}

	return Indx;
}

/*
**
** Function:        Comp_Pw()
**
** Description:     Computes harmonic noise filter coefficients.
**                  For each subframe, the optimal lag is searched around the
**                  open loop pitch lag based on only positive correlation
**                  maximization.
**
** Links to text:   Section 2.11
**
** Arguments:
**
**  Word16 *Dpnt    Formant perceptually weighted speech
**  Word16 Start
**  Word16 Olp      Open loop pitch lag
**
** Outputs:         None
**
** Return value:
**
**  PWDEF   Word16  Indx  lag of the harmonic noise shaping filter
**          Word16  Gain  gain of the harmonic noise shaping filter
**
*/
PWDEF Comp_Pw(Word16 * Dpnt, Word16 Start, Word16 Olp)
{

	int i, j;

	Word32 Lcr[15];
	Word16 Scr[15];
	PWDEF Pw;

	Word32 Acc0, Acc1;
	Word16 Exp;

	Word16 Ccr, Enr;
	Word16 Mcr, Mnr;

	/* Compute and save target energy */
	Lcr[0] = (Word32) 0;
	for (i = 0; i < SubFrLen; i++)
		Lcr[0] = g723_L_mac(Lcr[0], Dpnt[Start + i], Dpnt[Start + i]);

	/* Compute all Crosses and energys */
	for (i = 0; i <= 2 * PwRange; i++) {

		Acc1 = Acc0 = (Word32) 0;
		for (j = 0; j < SubFrLen; j++) {
			Acc0 = g723_L_mac(Acc0, Dpnt[Start + j],
					  Dpnt[Start - (Olp - PwRange + i) +
					       j]);
			Acc1 =
			    g723_L_mac(Acc1,
				       Dpnt[Start - (Olp - PwRange + i) + j],
				       Dpnt[Start - (Olp - PwRange + i) + j]);
		}

		/* Save both */
		Lcr[2 * i + 1] = Acc1;
		Lcr[2 * i + 2] = Acc0;
	}

	/* Normalize to maximum */
	Acc1 = (Word32) 0;
	for (i = 0; i < 15; i++) {
		Acc0 = Lcr[i];
		Acc0 = g723_L_abs(Acc0);
		if (Acc0 > Acc1)
			Acc1 = Acc0;
	}

	Exp = g723_norm_l(Acc1);
	for (i = 0; i < 15; i++) {
		Acc0 = L_g723_shl(Lcr[i], Exp);
		Scr[i] = round_(Acc0);

	}

	/* Find the best pair */
	Pw.Indx = (Word16) - 1;
	Pw.Gain = (Word16) 0;

	Mcr = (Word16) 1;
	Mnr = (Word16) 0x7fff;

	for (i = 0; i <= 2 * PwRange; i++) {

		Enr = Scr[2 * i + 1];
		Ccr = Scr[2 * i + 2];

		if (Ccr <= (Word16) 0)
			continue;

		Exp = g723_mult_r(Ccr, Ccr);

		/* Compute the cross */
		Acc0 = L_g723_mult(Exp, Mnr);
		Acc0 = g723_L_msu(Acc0, Enr, Mcr);

		if (Acc0 > (Word32) 0) {
			Mcr = Exp;
			Mnr = Enr;
			Pw.Indx = (Word16) i;
		}
	}

	if (Pw.Indx == -1) {
		Pw.Indx = Olp;
		return Pw;
	}

	/* Check the db limit */
	Acc0 = L_g723_mult(Scr[0], Mnr);
	Acc1 = Acc0;
	Acc0 = L_g723_shr(Acc0, (Word16) 2);
	Acc1 = L_g723_shr(Acc1, (Word16) 3);
	Acc0 = L_g723_add(Acc0, Acc1);
	Acc1 = L_g723_mult(Scr[2 * Pw.Indx + 2], Scr[2 * Pw.Indx + 2]);
	Acc0 = L_g723_sub(Acc0, Acc1);

	if (Acc0 < (Word32) 0) {

		Exp = Scr[2 * Pw.Indx + 2];

		if (Exp >= Mnr)
			Pw.Gain = PwConst;
		else {
			Pw.Gain = div_s(Exp, Mnr);
			Pw.Gain = g723_mult_r(Pw.Gain, PwConst);
		}
	}

	Pw.Indx = Olp - PwRange + Pw.Indx;

	return Pw;

}

/*
**
** Function:        Filt_Pw()
**
** Description:     Applies harmonic noise shaping filter.
**                  Lth order FIR filter on each subframe (L: lag of the filter).
**
** Links to text:   Section 2.11
**
** Arguments:
**
**  Word16 *DataBuff    Target vector
**  Word16 *Dpnt        Formant perceptually weighted speech
**  Word16 Start
**  PWDEF   Pw          Parameters of the harmonic noise shaping filter
**
** Outputs:
**
**  Word16 *DataBuff    Target vector
**
** Return value:        None
**
*/
void Filt_Pw(Word16 * DataBuff, Word16 * Dpnt, Word16 Start, PWDEF Pw)
{
	int i;

	Word32 Acc0;

	/* Perform the harmonic weighting */
	for (i = 0; i < SubFrLen; i++) {
		Acc0 = g723_L_deposit_h(Dpnt[PitchMax + Start + i]);
		Acc0 =
		    g723_L_msu(Acc0, Pw.Gain,
			       Dpnt[PitchMax + Start - Pw.Indx + i]);
		DataBuff[Start + (Word16) i] = round_(Acc0);
	}

	return;
}

/*
**
** Function:        Find_Fcbk()
**
** Description:     Fixed codebook excitation computation.
**
**
** Links to text:   Sections 2.15 & 2.16
**
** Arguments:
**
**  Word16 *Dpnt    Target vector
**  Word16 *ImpResp Impulse response of the synthesis filter
**  LineDef *Line   Excitation parameters for one subframe
**  Word16 Sfc      Subframe index
**
** Outputs:
**
**  Word16 *Dpnt    Excitation vector
**  LINEDEF *Line   Fixed codebook parameters for one subframe
**
** Return value:        None
**
*/
void Find_Fcbk(Word16 * Dpnt, Word16 * ImpResp, LINEDEF * Line, Word16 Sfc)
{
	int i;
	Word16 T0_acelp, gain_T0;
	Word16 Srate;

	BESTDEF Best;

	switch (WrkRate) {

	case Rate63:{

			Srate = Nb_puls[(int)Sfc];
			Best.MaxErr = (Word32) 0xc0000000L;
			Find_Best(&Best, Dpnt, ImpResp, Srate,
				  (Word16) SubFrLen);
			if ((*Line).Olp[Sfc >> 1] < (Word16) (SubFrLen - 2)) {
				Find_Best(&Best, Dpnt, ImpResp, Srate,
					  (*Line).Olp[Sfc >> 1]);
			}

			/* Reconstruct the excitation */
			for (i = 0; i < SubFrLen; i++)
				Dpnt[i] = (Word16) 0;
			for (i = 0; i < Srate; i++)
				Dpnt[Best.Ploc[i]] = Best.Pamp[i];

			/* Code the excitation */
			Fcbk_Pack(Dpnt, &((*Line).Sfs[Sfc]), &Best, Srate);

			if (Best.UseTrn == (Word16) 1)
				Gen_Trn(Dpnt, Dpnt, (*Line).Olp[Sfc >> 1]);

			break;
		}

	case Rate53:{

			T0_acelp = search_T0((Word16)
					     ((*Line).Olp[Sfc >> 1] - 1 +
					      (*Line).Sfs[Sfc].AcLg),
					     (*Line).Sfs[Sfc].AcGn, &gain_T0);

			(*Line).Sfs[Sfc].Ppos =
			    ACELP_LBC_code(Dpnt, ImpResp, T0_acelp, Dpnt,
					   &(*Line).Sfs[Sfc].Mamp,
					   &(*Line).Sfs[Sfc].Grid,
					   &(*Line).Sfs[Sfc].Pamp, gain_T0);

			(*Line).Sfs[Sfc].Tran = 0;

			break;
		}
	}

	return;
}

/*
**
** Function:        Gen_Trn()
**
** Description:     Generation of a train of Dirac functions with the period
**                  Olp.
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  Word16 *Dst     Fixed codebook excitation vector with  train of Dirac
**  Word16 *Src     Fixed codebook excitation vector without train of Dirac
**  Word16 Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  Word16 *Dst     excitation vector
**
** Return value:    None
**
*/
void Gen_Trn(Word16 * Dst, Word16 * Src, Word16 Olp)
{
	int i;

	Word16 Tmp0, Tmp1;
	Word16 Tmp[SubFrLen];

	Tmp0 = Olp;

	for (i = 0; i < SubFrLen; i++) {
		Tmp[i] = Src[i];
		Dst[i] = Src[i];
	}

	while (Tmp0 < SubFrLen) {
		for (i = (int)Tmp0; i < SubFrLen; i++) {
			Tmp1 = g723_add(Dst[i], Tmp[i - (int)Tmp0]);
			Dst[i] = Tmp1;
		}
		Tmp0 = g723_add(Tmp0, Olp);
	}

	return;
}

/*
**
** Function:        Find_Best()
**
** Description:     Fixed codebook search for the high rate encoder.
**                  It performs the quantization of the residual signal.
**                  The excitation made of Np positive or negative pulses
**                  multiplied by a gain and whose positions on the grid are
**                  either all odd or all even, should approximate as best as
**                  possible the residual signal (perceptual criterion).
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  BESTDEF *Best   Parameters of the best excitation model
**  Word16 *Tv      Target vector
**  Word16 *ImpResp Impulse response of the combined filter
**  Word16 Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**  Word16 Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  BESTDEF *Best
**
** Return value:    None
**
*/
void Find_Best(BESTDEF * Best, Word16 * Tv, Word16 * ImpResp, Word16 Np,
	       Word16 Olp)
{

	int i, j, k, l;
	BESTDEF Temp;

	Word16 Exp;
	Word16 MaxAmpId;
	Word16 MaxAmp;
	Word32 Acc0, Acc1, Acc2;

	Word16 Imr[SubFrLen];
	Word16 OccPos[SubFrLen];
	Word16 ImrCorr[SubFrLen];
	Word32 ErrBlk[SubFrLen];
	Word32 WrkBlk[SubFrLen];

	/* Update Impulse response */
	if (Olp < (Word16) (SubFrLen - 2)) {
		Temp.UseTrn = (Word16) 1;
		Gen_Trn(Imr, ImpResp, Olp);
	} else {
		Temp.UseTrn = (Word16) 0;
		for (i = 0; i < SubFrLen; i++)
			Imr[i] = ImpResp[i];
	}

	/* Scale Imr to avoid overflow */
	for (i = 0; i < SubFrLen; i++)
		OccPos[i] = g723_shr(Imr[i], (Word16) 1);

	/* Compute Imr AutoCorr function */
	Acc0 = (Word32) 0;
	for (i = 0; i < SubFrLen; i++)
		Acc0 = g723_L_mac(Acc0, OccPos[i], OccPos[i]);

	Exp = g723_norm_l(Acc0);
	Acc0 = L_g723_shl(Acc0, Exp);
	ImrCorr[0] = round_(Acc0);

	/* Compute all the other */
	for (i = 1; i < SubFrLen; i++) {
		Acc0 = (Word32) 0;
		for (j = i; j < SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, OccPos[j], OccPos[j - i]);
		Acc0 = L_g723_shl(Acc0, Exp);
		ImrCorr[i] = round_(Acc0);
	}

	/* Cross correlation with the signal */
	Exp = g723_sub(Exp, 4);
	for (i = 0; i < SubFrLen; i++) {
		Acc0 = (Word32) 0;
		for (j = i; j < SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, Tv[j], Imr[j - i]);
		ErrBlk[i] = L_g723_shl(Acc0, Exp);
	}

	/* Search for the best sequence */
	for (k = 0; k < Sgrid; k++) {

		Temp.GridId = (Word16) k;

		/* Find maximum amplitude */
		Acc1 = (Word32) 0;
		for (i = k; i < SubFrLen; i += Sgrid) {
			Acc0 = g723_L_abs(ErrBlk[i]);
			if (Acc0 >= Acc1) {
				Acc1 = Acc0;
				Temp.Ploc[0] = (Word16) i;
			}
		}

		/* Quantize the maximum amplitude */
		Acc2 = Acc1;
		Acc1 = (Word32) 0x40000000L;
		MaxAmpId = (Word16) (NumOfGainLev - MlqSteps);

		for (i = MaxAmpId; i >= MlqSteps; i--) {
			Acc0 = L_g723_mult(FcbkGainTable[i], ImrCorr[0]);
			Acc0 = L_g723_sub(Acc0, Acc2);
			Acc0 = g723_L_abs(Acc0);
			if (Acc0 < Acc1) {
				Acc1 = Acc0;
				MaxAmpId = (Word16) i;
			}
		}
		MaxAmpId--;

		for (i = 1; i <= 2 * MlqSteps; i++) {

			for (j = k; j < SubFrLen; j += Sgrid) {
				WrkBlk[j] = ErrBlk[j];
				OccPos[j] = (Word16) 0;
			}
			Temp.MampId = MaxAmpId - (Word16) MlqSteps + (Word16) i;

			MaxAmp = FcbkGainTable[Temp.MampId];

			if (WrkBlk[Temp.Ploc[0]] >= (Word32) 0)
				Temp.Pamp[0] = MaxAmp;
			else
				Temp.Pamp[0] = g723_negate(MaxAmp);

			OccPos[Temp.Ploc[0]] = (Word16) 1;

			for (j = 1; j < Np; j++) {

				Acc1 = (Word32) 0xc0000000L;

				for (l = k; l < SubFrLen; l += Sgrid) {

					if (OccPos[l] != (Word16) 0)
						continue;

					Acc0 = WrkBlk[l];
					Acc0 =
					    g723_L_msu(Acc0, Temp.Pamp[j - 1],
						       ImrCorr[g723_abs_s
							       ((Word16)
								(l -
								 Temp.Ploc[j -
									   1]))]);
					WrkBlk[l] = Acc0;
					Acc0 = g723_L_abs(Acc0);
					if (Acc0 > Acc1) {
						Acc1 = Acc0;
						Temp.Ploc[j] = (Word16) l;
					}
				}

				if (WrkBlk[Temp.Ploc[j]] >= (Word32) 0)
					Temp.Pamp[j] = MaxAmp;
				else
					Temp.Pamp[j] = g723_negate(MaxAmp);

				OccPos[Temp.Ploc[j]] = (Word16) 1;
			}

			/* Compute error vector */
			for (j = 0; j < SubFrLen; j++)
				OccPos[j] = (Word16) 0;

			for (j = 0; j < Np; j++)
				OccPos[Temp.Ploc[j]] = Temp.Pamp[j];

			for (l = SubFrLen - 1; l >= 0; l--) {
				Acc0 = (Word32) 0;
				for (j = 0; j <= l; j++)
					Acc0 =
					    g723_L_mac(Acc0, OccPos[j],
						       Imr[l - j]);
				Acc0 = L_g723_shl(Acc0, (Word16) 2);
				OccPos[l] = g723_extract_h(Acc0);
			}

			/* Evaluate error */
			Acc1 = (Word32) 0;
			for (j = 0; j < SubFrLen; j++) {
				Acc1 = g723_L_mac(Acc1, Tv[j], OccPos[j]);
				Acc0 = L_g723_mult(OccPos[j], OccPos[j]);
				Acc1 =
				    L_g723_sub(Acc1,
					       L_g723_shr(Acc0, (Word16) 1));
			}

			if (Acc1 > (*Best).MaxErr) {
				(*Best).MaxErr = Acc1;
				(*Best).GridId = Temp.GridId;
				(*Best).MampId = Temp.MampId;
				(*Best).UseTrn = Temp.UseTrn;
				for (j = 0; j < Np; j++) {
					(*Best).Pamp[j] = Temp.Pamp[j];
					(*Best).Ploc[j] = Temp.Ploc[j];
				}
			}
		}
	}
	return;
}

/*
**
** Function:        Fcbk_Pack()
**
** Description:     Encoding of the pulse positions and gains for the high
**                  rate case.
**                  Combinatorial encoding is used to transmit the optimal
**                  combination of pulse locations.
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  Word16 *Dpnt    Excitation vector
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**  BESTDEF *Best   Parameters of the best excitation model
**  Word16 Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**
** Outputs:
**
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**
** Return value:    None
**
*/
void Fcbk_Pack(Word16 * Dpnt, SFSDEF * Sfs, BESTDEF * Best, Word16 Np)
{
	int i, j;

	/* Code the amplitudes and positions */
	j = MaxPulseNum - (int)Np;

	(*Sfs).Pamp = (Word16) 0;
	(*Sfs).Ppos = (Word32) 0;

	for (i = 0; i < SubFrLen / Sgrid; i++) {

		if (Dpnt[(int)(*Best).GridId + Sgrid * i] == (Word16) 0)
			(*Sfs).Ppos =
			    L_g723_add((*Sfs).Ppos, CombinatorialTable[j][i]);
		else {
			(*Sfs).Pamp = g723_shl((*Sfs).Pamp, (Word16) 1);
			if (Dpnt[(int)(*Best).GridId + Sgrid * i] < (Word16) 0)
				(*Sfs).Pamp = g723_add((*Sfs).Pamp, (Word16) 1);

			j++;
			/* Check for end */
			if (j == MaxPulseNum)
				break;
		}
	}

	(*Sfs).Mamp = (*Best).MampId;
	(*Sfs).Grid = (*Best).GridId;
	(*Sfs).Tran = (*Best).UseTrn;

	return;
}

/*
**
** Function:        Fcbk_Unpk()
**
** Description:     Decoding of the fixed codebook excitation for both rates.
**                  Gains, pulse positions, grid position (odd or even), signs
**                  are decoded and used to reconstruct the excitation.
**
** Links to text:   Section 2.17 & 3.5
**
** Arguments:
**
**  Word16 *Tv      Decoded excitation vector
**  SFSDEF Sfs      Encoded parameters of the excitation (for one subframe)
**  Word16 Olp      Closed loop adaptive pitch lag
**  Word16 Sfc      Subframe index
**
** Outputs:
**
**  Word16 *Tv      Decoded excitation vector
**
** Return value:    None
**
*/
void Fcbk_Unpk(Word16 * Tv, SFSDEF Sfs, Word16 Olp, Word16 Sfc)
{
	int i, j;

	Word32 Acc0;
	Word16 Np;
	Word16 Tv_tmp[SubFrLen + 4];
	Word16 acelp_gain, acelp_sign, acelp_shift, acelp_pos;
	Word16 offset, ipos, T0_acelp, gain_T0;

	switch (WrkRate) {
	case Rate63:{

			Np = Nb_puls[(int)Sfc];

			for (i = 0; i < SubFrLen; i++)
				Tv[i] = (Word16) 0;

			if (Sfs.Ppos >= MaxPosTable[Sfc])
				return;

			/* Decode the amplitudes and positions */
			j = MaxPulseNum - (int)Np;

			Acc0 = Sfs.Ppos;

			for (i = 0; i < SubFrLen / Sgrid; i++) {

				Acc0 =
				    L_g723_sub(Acc0, CombinatorialTable[j][i]);

				if (Acc0 < (Word32) 0) {
					Acc0 =
					    L_g723_add(Acc0,
						       CombinatorialTable[j]
						       [i]);
					j++;
					if ((Sfs.
					     Pamp & (1 << (MaxPulseNum - j))) !=
					    (Word16) 0)
						Tv[(int)Sfs.Grid + Sgrid * i] =
						    -FcbkGainTable[Sfs.Mamp];
					else
						Tv[(int)Sfs.Grid + Sgrid * i] =
						    FcbkGainTable[Sfs.Mamp];

					if (j == MaxPulseNum)
						break;
				}
			}

			if (Sfs.Tran == (Word16) 1)
				Gen_Trn(Tv, Tv, Olp);
			break;
		}

	case Rate53:{
			for (i = 0; i < SubFrLen + 4; i++)
				Tv_tmp[i] = (Word16) 0;

			/* decoding gain */
			acelp_gain = FcbkGainTable[Sfs.Mamp];
			/* decoding grid */
			acelp_shift = Sfs.Grid;
			/* decoding Sign */
			acelp_sign = Sfs.Pamp;
			/* decoding Pos */
			acelp_pos = (short)Sfs.Ppos;

			offset = 0;
			for (i = 0; i < 4; i++) {
				ipos = (acelp_pos & (Word16) 0x0007);
				ipos = g723_shl(ipos, 3) + acelp_shift + offset;
				if ((acelp_sign & 1) == 1) {
					Tv_tmp[ipos] = acelp_gain;
				} else {
					Tv_tmp[ipos] = -acelp_gain;
				}
				offset = g723_add(offset, 2);
				acelp_pos = g723_shr(acelp_pos, 3);
				acelp_sign = g723_shr(acelp_sign, 1);
			}
			for (i = 0; i < SubFrLen; i++)
				Tv[i] = Tv_tmp[i];
			T0_acelp =
			    search_T0((Word16) (Olp - 1 + Sfs.AcLg), Sfs.AcGn,
				      &gain_T0);
			if (T0_acelp < SubFrLen - 2) {
				/* code[i] += 0.8 * code[i-Olp] */
				for (i = T0_acelp; i < SubFrLen; i++)
					Tv[i] =
					    g723_add(Tv[i],
						     g723_mult(Tv[i - T0_acelp],
							       gain_T0));
			}

			break;
		}
	}
	return;
}

/*
**
** Function:        Find_Acbk()
**
** Description:     Computation of adaptive codebook contribution in
**                  closed-loop around the open-loop pitch lag (subframes 0 & 2)
**                  around the previous subframe closed-loop pitch lag
**                  (subframes 1 & 3).  For subframes 0 & 2, the pitch lag is
**                  encoded whereas for subframes 1 & 3, only the difference
**                  with the previous value is encoded (-1, 0, +1 or +2).
**                  The pitch predictor gains are quantized using one of the two
**                  codebooks (85 entries or 170 entries) depending on the
**                  rate and on the pitch lag value.
**                  Finally, the contribution of the pitch predictor is decoded
**                  and subtracted to obtain the residual signal.
**
** Links to text:   Section 2.14
**
** Arguments:
**
**  Word16 *Tv      Target vector
**  Word16 *ImpResp Impulse response of the combined filter
**  Word16 *PrevExc Previous excitation vector
**  LINEDEF *Line   Contains pitch related parameters (open/closed loop lag, gain)
**  Word16 Sfc      Subframe index
**
** Outputs:
**
**  Word16 *Tv     Residual vector
**  LINEDEF *Line  Contains pitch related parameters (closed loop lag, gain)
**
** Return value:    None
**
*/
void Find_Acbk(Word16 * Tv, Word16 * ImpResp, Word16 * PrevExc, LINEDEF
	       * Line, Word16 Sfc)
{
	int i, j, k, l;

	Word32 Acc0, Acc1;

	Word16 RezBuf[SubFrLen + ClPitchOrd - 1];
	Word16 FltBuf[ClPitchOrd][SubFrLen];
	Word32 CorBuf[4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2)];
	Word32 *lPnt;

	Word16 CorVct[4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2)];
	Word16 *sPnt;

	Word16 Olp;
	Word16 Lid;
	Word16 Gid;
	Word16 Hb;
	Word16 Exp;
	Word16 Bound[2];

	Word16 Lag1, Lag2;
	Word16 off_filt;

	memzero(CorBuf,
		4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2) * sizeof(Word32));

	/* Init constants */
	Olp = (*Line).Olp[g723_shr(Sfc, (Word16) 1)];
	Lid = (Word16) Pstep;
	Gid = (Word16) 0;
	Hb = (Word16) 3 + (Sfc & (Word16) 1);

	/* For even frames only */
	if ((Sfc & (Word16) 1) == (Word16) 0) {
		if (Olp == (Word16) PitchMin)
			Olp = g723_add(Olp, (Word16) 1);
		if (Olp > (Word16) (PitchMax - 5))
			Olp = (Word16) (PitchMax - 5);
	}

	lPnt = CorBuf;
	for (k = 0; k < (int)Hb; k++) {

		/* Get residual from the excitation buffer */
		Get_Rez(RezBuf, PrevExc, (Word16) (Olp - (Word16) Pstep + k));

		/* Filter the last one using the impulse response */
		for (i = 0; i < SubFrLen; i++) {
			Acc0 = (Word32) 0;
			for (j = 0; j <= i; j++)
				Acc0 =
				    g723_L_mac(Acc0, RezBuf[ClPitchOrd - 1 + j],
					       ImpResp[i - j]);
			FltBuf[ClPitchOrd - 1][i] = round_(Acc0);
		}

		/* Update all the others */
		for (i = ClPitchOrd - 2; i >= 0; i--) {
			FltBuf[i][0] = g723_mult_r(RezBuf[i], (Word16) 0x2000);
			for (j = 1; j < SubFrLen; j++) {
				Acc0 = g723_L_deposit_h(FltBuf[i + 1][j - 1]);
				Acc0 = g723_L_mac(Acc0, RezBuf[i], ImpResp[j]);
				FltBuf[i][j] = round_(Acc0);
			}
		}

		/* Compute the cross with the signal */
		for (i = 0; i < ClPitchOrd; i++) {
			Acc1 = (Word32) 0;
			for (j = 0; j < SubFrLen; j++) {
				Acc0 = L_g723_mult(Tv[j], FltBuf[i][j]);
				Acc1 =
				    L_g723_add(Acc1,
					       L_g723_shr(Acc0, (Word16) 1));
			}
			*lPnt++ = L_g723_shl(Acc1, (Word16) 1);
		}

		/* Compute the energies */
		for (i = 0; i < ClPitchOrd; i++) {
			Acc1 = (Word32) 0;
			for (j = 0; j < SubFrLen; j++)
				Acc1 =
				    g723_L_mac(Acc1, FltBuf[i][j],
					       FltBuf[i][j]);
			*lPnt++ = Acc1;
		}

		/* Compute the between crosses */
		for (i = 1; i < ClPitchOrd; i++) {
			for (j = 0; j < i; j++) {
				Acc1 = (Word32) 0;
				for (l = 0; l < SubFrLen; l++) {
					Acc0 =
					    L_g723_mult(FltBuf[i][l],
							FltBuf[j][l]);
					Acc1 =
					    L_g723_add(Acc1,
						       L_g723_shr(Acc0,
								  (Word16) 1));
				}
				*lPnt++ = L_g723_shl(Acc1, (Word16) 2);
			}
		}
	}

	/* Find Max and normalize */
	Acc1 = (Word32) 0;
	for (i = 0; i < Hb * 20; i++) {
		Acc0 = g723_L_abs(CorBuf[i]);
		if (Acc0 > Acc1)
			Acc1 = Acc0;
	}

	Exp = g723_norm_l(Acc1);
	/* Convert to shorts */
	for (i = 0; i < Hb * 20; i++) {
		Acc0 = L_g723_shl(CorBuf[i], Exp);
		CorVct[i] = round_(Acc0);
	}

	/* Test potential error */
	Lag1 = Olp - (Word16) Pstep;
	Lag2 = Olp - (Word16) Pstep + Hb - (Word16) 1;
	off_filt = Test_Err(Lag1, Lag2);
	Bound[0] = NbFilt085_min + g723_shl(off_filt, 2);
	if (Bound[0] > NbFilt085)
		Bound[0] = NbFilt085;
	Bound[1] = NbFilt170_min + g723_shl(off_filt, 3);
	if (Bound[1] > NbFilt170)
		Bound[1] = NbFilt170;

	/* Init the search loop */
	Acc1 = (Word32) 0;

	for (k = 0; k < (int)Hb; k++) {

		/* Select Quantization tables */
		l = 0;
		if (WrkRate == Rate63) {
			if ((Sfc & (Word16) 1) == (Word16) 0) {
				if ((int)Olp - Pstep + k >= SubFrLen - 2)
					l++;
			} else {
				if ((int)Olp >= SubFrLen - 2)
					l++;
			}
		} else {
			l = 1;
		}

		sPnt = AcbkGainTablePtr[l];

		for (i = 0; i < (int)Bound[l]; i++) {

			Acc0 = (Word32) 0;
			for (j = 0; j < 20; j++)
				Acc0 =
				    L_g723_add(Acc0,
					       L_g723_shr(L_g723_mult
							  (CorVct[k * 20 + j],
							   *sPnt++),
							  (Word16) 1));

			if (Acc0 > Acc1) {
				Acc1 = Acc0;
				Gid = (Word16) i;
				Lid = (Word16) k;
			}
		}
	}

	/* Modify Olp for even sub frames */
	if ((Sfc & (Word16) 1) == (Word16) 0) {
		Olp = Olp - (Word16) Pstep + Lid;
		Lid = (Word16) Pstep;
	}

	/* Save Gains and Olp */
	(*Line).Sfs[Sfc].AcLg = Lid;
	(*Line).Sfs[Sfc].AcGn = Gid;
	(*Line).Olp[g723_shr(Sfc, (Word16) 1)] = Olp;

	/* Decode the Acbk contribution and subtract it */
	Decod_Acbk(RezBuf, PrevExc, Olp, Lid, Gid);

	for (i = 0; i < SubFrLen; i++) {
		Acc0 = g723_L_deposit_h(Tv[i]);
		Acc0 = L_g723_shr(Acc0, (Word16) 1);

		for (j = 0; j <= i; j++)
			Acc0 = g723_L_msu(Acc0, RezBuf[j], ImpResp[i - j]);
		Acc0 = L_g723_shl(Acc0, (Word16) 1);
		Tv[i] = round_(Acc0);
	}

	return;
}

/*
**
** Function:        Get_Rez()
**
** Description:     Gets delayed contribution from the previous excitation
**                  vector.
**
** Links to text:   Sections 2.14, 2.18 & 3.4
**
** Arguments:
**
**  Word16 *Tv      delayed excitation
**  Word16 *PrevExc Previous excitation vector
**  Word16 Lag      Closed loop pitch lag
**
** Outputs:
**
**  Word16 *Tv      delayed excitation
**
** Return value:    None
**
*/
void Get_Rez(Word16 * Tv, Word16 * PrevExc, Word16 Lag)
{
	int i;

	for (i = 0; i < ClPitchOrd / 2; i++)
		Tv[i] = PrevExc[PitchMax - (int)Lag - ClPitchOrd / 2 + i];

	for (i = 0; i < SubFrLen + ClPitchOrd / 2; i++)
		Tv[ClPitchOrd / 2 + i] =
		    PrevExc[PitchMax - (int)Lag + i % (int)Lag];

	return;
}

/*
**
** Function:        Decod_Acbk()
**
** Description:     Computes the adaptive codebook contribution from the previous
**                  excitation vector.
**                  With the gain index, the closed loop pitch lag, the jitter
**                  which when added to this pitch lag gives the actual closed
**                  loop value, and after having selected the proper codebook,
**                  the pitch contribution is reconstructed using the previous
**                  excitation buffer.
**
** Links to text:   Sections 2.14, 2.18 & 3.4
**
** Arguments:
**
**  Word16 *Tv      Reconstructed excitation vector
**  Word16 *PrevExc Previous excitation vector
**  Word16 Olp      closed-loop pitch period
**  Word16 Lid      Jitter around pitch period
**  Word16 Gid      Gain vector index in 5- dimensional
**                      adaptive gain vector codebook
**
** Outputs:
**
**  Word16 *Tv      Reconstructed excitation vector
**
** Return value:    None
**
*/
void Decod_Acbk(Word16 * Tv, Word16 * PrevExc, Word16 Olp, Word16 Lid,
		Word16 Gid)
{
	int i, j;

	Word32 Acc0;
	Word16 RezBuf[SubFrLen + ClPitchOrd - 1];
	Word16 *sPnt;

	Get_Rez(RezBuf, PrevExc, (Word16) (Olp - (Word16) Pstep + Lid));

	/* Select Quantization tables */
	i = 0;
	if (WrkRate == Rate63) {
		if (Olp >= (Word16) (SubFrLen - 2))
			i++;
	} else {
		i = 1;
	}
	sPnt = AcbkGainTablePtr[i];

	sPnt += (int)Gid *20;

	for (i = 0; i < SubFrLen; i++) {
		Acc0 = (Word32) 0;
		for (j = 0; j < ClPitchOrd; j++)
			Acc0 = g723_L_mac(Acc0, RezBuf[i + j], sPnt[j]);
		Acc0 = L_g723_shl(Acc0, (Word16) 1);
		Tv[i] = round_(Acc0);
	}

	return;
}

/*
**
** Function:        Comp_Info()
**
** Description:     Voiced/unvoiced classifier.
**                  It is based on a cross correlation maximization over the
**                  last 120 samples of the frame and with an index varying
**                  around the decoded pitch lag (from L-3 to L+3). Then the
**                  prediction gain is tested to declare the frame voiced or
**                  unvoiced.
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Word16 *Buff  decoded excitation
**  Word16 Olp    Decoded pitch lag
**
** Outputs: None
**
** Return value:
**
**      Word16   Estimated pitch value
*/
Word16 Comp_Info(Word16 * Buff, Word16 Olp, Word16 * Gain, Word16 * ShGain)
{
	int i, j;

	Word32 Acc0, Acc1;

	Word16 Tenr;
	Word16 Ccr, Enr;
	Word16 Indx;

	/* Normalize the excitation */
	*ShGain = Vec_Norm(Buff, (Word16) (PitchMax + Frame));

	if (Olp > (Word16) (PitchMax - 3))
		Olp = (Word16) (PitchMax - 3);

	Indx = Olp;

	Acc1 = (Word32) 0;

	for (i = (int)Olp - 3; i <= (int)Olp + 3; i++) {

		Acc0 = (Word32) 0;
		for (j = 0; j < 2 * SubFrLen; j++)
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + Frame - 2 * SubFrLen +
					    j],
				       Buff[PitchMax + Frame - 2 * SubFrLen -
					    i + j]);

		if (Acc0 > Acc1) {
			Acc1 = Acc0;
			Indx = (Word16) i;
		}
	}

	/* Compute target energy */
	Acc0 = (Word32) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0, Buff[PitchMax + Frame - 2 * SubFrLen + j],
			       Buff[PitchMax + Frame - 2 * SubFrLen + j]);
	Tenr = round_(Acc0);
	*Gain = Tenr;

	/* Compute best energy */
	Acc0 = (Word32) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0,
			       Buff[PitchMax + Frame - 2 * SubFrLen -
				    (int)Indx + j],
			       Buff[PitchMax + Frame - 2 * SubFrLen -
				    (int)Indx + j]);

	Ccr = round_(Acc1);

	if (Ccr <= (Word16) 0)
		return (Word16) 0;

	Enr = round_(Acc0);

	Acc0 = L_g723_mult(Enr, Tenr);
	Acc0 = L_g723_shr(Acc0, (Word16) 3);

	Acc0 = g723_L_msu(Acc0, Ccr, Ccr);

	if (Acc0 < (Word32) 0)
		return Indx;
	else
		return (Word16) 0;
}

/*
**
** Function:        Regen()
**
** Description:     Performs residual interpolation depending of the frame
**                  classification.
**                  If the frame is previously declared unvoiced, the excitation
**                  is regenerated using a random number generator. Otherwise
**                  a periodic excitation is generated with the period
**                  previously found.
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Word16 *DataBuff  current subframe decoded excitation
**  Word16 *Buff     past decoded excitation
**  Word16 Lag       Decoded pitch lag from previous frame
**  Word16 Gain      Interpolated gain from previous frames
**  Word16 Ecount    Number of erased frames
**  Word16 *Sd       Random number used in unvoiced cases
**
** Outputs:
**
**  Word16 *DataBuff current subframe decoded excitation
**  Word16 *Buff     updated past excitation
**
** Return value:    None
**
*/
void Regen(Word16 * DataBuff, Word16 * Buff, Word16 Lag, Word16 Gain,
	   Word16 Ecount, Word16 * Sd)
{
	int i;

	/* Test for clearing */
	if (Ecount >= (Word16) ErrMaxNum) {
		for (i = 0; i < Frame; i++)
			DataBuff[i] = (Word16) 0;
		for (i = 0; i < Frame + PitchMax; i++)
			Buff[i] = (Word16) 0;
	} else {
		/* Interpolate accordingly to the voicing estimation */
		if (Lag != (Word16) 0) {
			/* Voiced case */
			for (i = 0; i < Frame; i++)
				Buff[PitchMax + i] =
				    Buff[PitchMax - (int)Lag + i];
			for (i = 0; i < Frame; i++)
				DataBuff[i] = Buff[PitchMax + i] =
				    g723_mult(Buff[PitchMax + i],
					      (Word16) 0x6000);
		} else {
			/* Unvoiced case */
			for (i = 0; i < Frame; i++)
				DataBuff[i] = g723_mult(Gain, Rand_lbc(Sd));
			/* Clear buffer to reset memory */
			for (i = 0; i < Frame + PitchMax; i++)
				Buff[i] = (Word16) 0;
		}
	}

	return;
}

/*
**
** Function:        Comp_Lpf()
**
** Description:     Computes pitch postfilter parameters.
**                  The pitch postfilter lag is first derived (Find_B
**                  and Find_F). Then, the one that gives the largest
**                  contribution is used to calculate the gains (Get_Ind).
**
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 *Buff    decoded excitation
**  Word16 Olp      Decoded pitch lag
**  Word16 Sfc      Subframe index
**
** Outputs:
**
**
** Return value:
**
**  PFDEF       Pitch postfilter parameters: PF.Gain    Pitch Postfilter gain
**                                           PF.ScGn    Pitch Postfilter scaling gain
**                                           PF.Indx    Pitch postfilter lag
*/
PFDEF Comp_Lpf(Word16 * Buff, Word16 Olp, Word16 Sfc)
{
	int i, j;

	PFDEF Pf;
	Word32 Lcr[5];
	Word16 Scr[5];
	Word16 Bindx, Findx;
	Word16 Exp;

	Word32 Acc0, Acc1;

	/* Initialize */
	Pf.Indx = (Word16) 0;
	Pf.Gain = (Word16) 0;
	Pf.ScGn = (Word16) 0x7fff;

	/* Find both indices */
	Bindx = Find_B(Buff, Olp, Sfc);
	Findx = Find_F(Buff, Olp, Sfc);

	/* Combine the results */
	if ((Bindx == (Word16) 0) && (Findx == (Word16) 0))
		return Pf;

	/* Compute target energy */
	Acc0 = (Word32) 0;
	for (j = 0; j < SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0, Buff[PitchMax + (int)Sfc * SubFrLen + j],
			       Buff[PitchMax + (int)Sfc * SubFrLen + j]);
	Lcr[0] = Acc0;

	if (Bindx != (Word16) 0) {
		Acc0 = (Word32) 0;
		Acc1 = (Word32) 0;
		for (j = 0; j < SubFrLen; j++) {
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + (int)Sfc * SubFrLen + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Bindx + j]);
			Acc1 =
			    g723_L_mac(Acc1,
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Bindx + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Bindx + j]);
		}
		Lcr[1] = Acc0;
		Lcr[2] = Acc1;
	} else {
		Lcr[1] = (Word32) 0;
		Lcr[2] = (Word32) 0;
	}

	if (Findx != (Word16) 0) {
		Acc0 = (Word32) 0;
		Acc1 = (Word32) 0;
		for (j = 0; j < SubFrLen; j++) {
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + (int)Sfc * SubFrLen + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Findx + j]);
			Acc1 =
			    g723_L_mac(Acc1,
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Findx + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen +
					    (int)Findx + j]);
		}
		Lcr[3] = Acc0;
		Lcr[4] = Acc1;
	} else {
		Lcr[3] = (Word32) 0;
		Lcr[4] = (Word32) 0;
	}

	/* Normalize and convert to shorts */
	Acc1 = 0L;
	for (i = 0; i < 5; i++) {
		Acc0 = Lcr[i];
		if (Acc0 > Acc1)
			Acc1 = Acc0;
	}

	Exp = g723_norm_l(Acc1);
	for (i = 0; i < 5; i++) {
		Acc0 = L_g723_shl(Lcr[i], Exp);
		Scr[i] = g723_extract_h(Acc0);
	}

	/* Select the best pair */
	if ((Bindx != (Word16) 0) && (Findx == (Word16) 0))
		Pf = Get_Ind(Bindx, Scr[0], Scr[1], Scr[2]);

	if ((Bindx == (Word16) 0) && (Findx != (Word16) 0))
		Pf = Get_Ind(Findx, Scr[0], Scr[3], Scr[4]);

	if ((Bindx != (Word16) 0) && (Findx != (Word16) 0)) {
		Exp = g723_mult_r(Scr[1], Scr[1]);
		Acc0 = L_g723_mult(Exp, Scr[4]);
		Exp = g723_mult_r(Scr[3], Scr[3]);
		Acc1 = L_g723_mult(Exp, Scr[2]);
		if (Acc0 > Acc1)
			Pf = Get_Ind(Bindx, Scr[0], Scr[1], Scr[2]);
		else
			Pf = Get_Ind(Findx, Scr[0], Scr[3], Scr[4]);
	}

	return Pf;
}

/*
**
** Function:        Find_B()
**
** Description:     Computes best pitch postfilter backward lag by
**                  backward cross correlation maximization around the
**                  decoded pitch lag
**                  of the subframe 0 (for subframes 0 & 1)
**                  of the subframe 2 (for subframes 2 & 3)
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 *Buff    decoded excitation
**  Word16 Olp      Decoded pitch lag
**  Word16 Sfc      Subframe index
**
** Outputs:     None
**
** Return value:
**
**  Word16   Pitch postfilter backward lag
*/
Word16 Find_B(Word16 * Buff, Word16 Olp, Word16 Sfc)
{
	int i, j;

	Word16 Indx = 0;

	Word32 Acc0, Acc1;

	if (Olp > (Word16) (PitchMax - 3))
		Olp = (Word16) (PitchMax - 3);

	Acc1 = (Word32) 0;

	for (i = (int)Olp - 3; i <= (int)Olp + 3; i++) {

		Acc0 = (Word32) 0;
		for (j = 0; j < SubFrLen; j++)
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + (int)Sfc * SubFrLen + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen - i +
					    j]);
		if (Acc0 > Acc1) {
			Acc1 = Acc0;
			Indx = -(Word16) i;
		}
	}
	return Indx;
}

/*
**
** Function:        Find_F()
**
** Description:     Computes best pitch postfilter forward lag by
**                  forward cross correlation maximization around the
**                  decoded pitch lag
**                  of the subframe 0 (for subframes 0 & 1)
**                  of the subframe 2 (for subframes 2 & 3)
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 *Buff    decoded excitation
**  Word16 Olp      Decoded pitch lag
**  Word16 Sfc      Subframe index
**
** Outputs:     None
**
** Return value:
**
**  Word16    Pitch postfilter forward lag
*/
Word16 Find_F(Word16 * Buff, Word16 Olp, Word16 Sfc)
{
	int i, j;

	Word16 Indx = 0;

	Word32 Acc0, Acc1;

	if (Olp > (Word16) (PitchMax - 3))
		Olp = (Word16) (PitchMax - 3);

	Acc1 = (Word32) 0;

	for (i = Olp - 3; i <= Olp + 3; i++) {

		Acc0 = (Word32) 0;
		if (((int)Sfc * SubFrLen + SubFrLen + i) <= Frame) {
			for (j = 0; j < SubFrLen; j++)
				Acc0 =
				    g723_L_mac(Acc0,
					       Buff[PitchMax +
						    (int)Sfc * SubFrLen + j],
					       Buff[PitchMax +
						    (int)Sfc * SubFrLen + i +
						    j]);
		}

		if (Acc0 > Acc1) {
			Acc1 = Acc0;
			Indx = (Word16) i;
		}
	}

	return Indx;
}

/*
**
** Function:        Get_Ind()
**
** Description:     Computes gains of the pitch postfilter.
**                  The gains are calculated using the cross correlation
**                  (forward or backward, the one with the greatest contribution)
**                  and the energy of the signal. Also, a test is performed on
**                  the prediction gain to see whether the pitch postfilter
**                  should be used or not.
**
**
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 Ind      Pitch postfilter lag
**  Word16 Ten      energy of the current subframe excitation vector
**  Word16 Ccr      Crosscorrelation of the excitation
**  Word16 Enr      Energy of the (backward or forward) "delayed" excitation
**
** Outputs:     None
**
** Return value:
**
**  PFDEF
**         Word16   Indx    Pitch postfilter lag
**         Word16   Gain    Pitch postfilter gain
**         Word16   ScGn    Pitch postfilter scaling gain
**
*/
PFDEF Get_Ind(Word16 Ind, Word16 Ten, Word16 Ccr, Word16 Enr)
{
	Word32 Acc0, Acc1;
	Word16 Exp;

	PFDEF Pf;

	Pf.Indx = Ind;

	/* Check valid gain */
	Acc0 = L_g723_mult(Ten, Enr);
	Acc0 = L_g723_shr(Acc0, (Word16) 2);
	Acc1 = L_g723_mult(Ccr, Ccr);

	if (Acc1 > Acc0) {

		if (Ccr >= Enr)
			Pf.Gain = LpfConstTable[(int)WrkRate];
		else {
			Pf.Gain = div_s(Ccr, Enr);
			Pf.Gain =
			    g723_mult(Pf.Gain, LpfConstTable[(int)WrkRate]);
		}
		/* Compute scaling gain */
		Acc0 = g723_L_deposit_h(Ten);
		Acc0 = L_g723_shr(Acc0, (Word16) 1);
		Acc0 = g723_L_mac(Acc0, Ccr, Pf.Gain);
		Exp = g723_mult(Pf.Gain, Pf.Gain);
		Acc1 = L_g723_mult(Enr, Exp);
		Acc1 = L_g723_shr(Acc1, (Word16) 1);
		Acc0 = L_g723_add(Acc0, Acc1);
		Exp = round_(Acc0);

		Acc1 = g723_L_deposit_h(Ten);
		Acc0 = g723_L_deposit_h(Exp);
		Acc1 = L_g723_shr(Acc1, (Word16) 1);

		if (Acc1 >= Acc0)
			Exp = (Word16) 0x7fff;
		else
			Exp = div_l(Acc1, Exp);

		Acc0 = g723_L_deposit_h(Exp);
		Pf.ScGn = Sqrt_lbc(Acc0);
	} else {
		Pf.Gain = (Word16) 0;
		Pf.ScGn = (Word16) 0x7fff;
	}

	Pf.Gain = g723_mult(Pf.Gain, Pf.ScGn);

	return Pf;
}

/*
**
** Function:        Filt_Lpf()
**
** Description:     Applies the pitch postfilter for each subframe.
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 *Tv      Pitch postfiltered excitation
**  Word16 *Buff    decoded excitation
**  PFDEF Pf        Pitch postfilter parameters
**  Word16 Sfc      Subframe index
**
** Outputs:
**
**  Word16 *Tv      Pitch postfiltered excitation
**
** Return value: None
**
*/
void Filt_Lpf(Word16 * Tv, Word16 * Buff, PFDEF Pf, Word16 Sfc)
{
	int i;

	Word32 Acc0;

	for (i = 0; i < SubFrLen; i++) {
		Acc0 =
		    L_g723_mult(Buff[PitchMax + (int)Sfc * SubFrLen + i],
				Pf.ScGn);
		Acc0 =
		    g723_L_mac(Acc0,
			       Buff[PitchMax + (int)Sfc * SubFrLen +
				    (int)Pf.Indx + i], Pf.Gain);
		Tv[(int)Sfc * SubFrLen + i] = round_(Acc0);
	}

	return;
}

/*
**
** Function:        ACELP_LBC_code()
**
** Description:     Find Algebraic codebook for low bit rate LBC encoder
**
** Links to text:   Section 2.16
**
** Arguments:
**
**   Word16 X[]              Target vector.     (in Q0)
**   Word16 h[]              Impulse response.  (in Q12)
**   Word16 T0               Pitch period.
**   Word16 code[]           Innovative vector.        (in Q12)
**   Word16 gain             Innovative vector gain.   (in Q0)
**   Word16 sign             Signs of the 4 pulses.
**   Word16 shift            Shift of the innovative vector
**   Word16 gain_T0          Gain for pitch synchronous fiter
**
** Inputs :
**
**   Word16 X[]              Target vector.     (in Q0)
**   Word16 h[]              Impulse response.  (in Q12)
**   Word16 T0               Pitch period.
**   Word16 gain_T0          Gain for pitch synchronous fiter
**
** Outputs:
**
**   Word16 code[]           Innovative vector.        (in Q12)
**   Word16 gain             Innovative vector gain.   (in Q0)
**   Word16 sign             Signs of the 4 pulses.
**   Word16 shift            Shift of the innovative vector.
**
** Return value:
**
**   Word16 index            Innovative codebook index
**
*/
Word16 ACELP_LBC_code(Word16 X[], Word16 h[], Word16 T0, Word16 code[],
		      Word16 * ind_gain, Word16 * shift, Word16 * sign,
		      Word16 gain_T0)
{
	Word16 i, index, gain_q;
	Word16 Dn[SubFrLen2], tmp_code[SubFrLen2];
	Word16 rr[DIM_RR];

	/*
	 * Include fixed-gain pitch contribution into impulse resp. h[]
	 * Find correlations of h[] needed for the codebook search.
	 */
	for (i = 0; i < SubFrLen; i++)	/* Q13 -->  Q12 */
		h[i] = g723_shr(h[i], 1);

	if (T0 < SubFrLen - 2) {
		for (i = T0; i < SubFrLen; i++)	/* h[i] += gain_T0*h[i-T0] */
			h[i] = g723_add(h[i], g723_mult(h[i - T0], gain_T0));
	}

	Cor_h(h, rr);

	/*
	 * Compute correlation of target vector with impulse response.
	 */

	Cor_h_X(h, X, Dn);

	/*
	 * Find innovative codebook.
	 * rr input matrix autocorrelation
	 *    output filtered codeword
	 */

	index = D4i64_LBC(Dn, rr, h, tmp_code, rr, shift, sign);

	/*
	 * Compute innovation vector gain.
	 * Include fixed-gain pitch contribution into code[].
	 */

	*ind_gain = G_code(X, rr, &gain_q);

	for (i = 0; i < SubFrLen; i++) {
		code[i] = i_g723_mult(tmp_code[i], gain_q);
	}

	if (T0 < SubFrLen - 2)
		for (i = T0; i < SubFrLen; i++)	/* code[i] += gain_T0*code[i-T0] */
			code[i] =
			    g723_add(code[i], g723_mult(code[i - T0], gain_T0));

	return index;
}

/*
**
** Function:        Cor_h()
**
** Description:     Compute correlations of h[] needed for the codebook search.
**
** Links to text:   Section 2.16
**
** Arguments:
**
**  Word16 h[]              Impulse response.
**  Word16 rr[]             Correlations.
**
**  Outputs:
**
**  Word16 rr[]             Correlations.
**
**  Return value :          None
*/
void Cor_h(Word16 * H, Word16 * rr)
{
	Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3;
	Word16 *rri0i1, *rri0i2, *rri0i3;
	Word16 *rri1i2, *rri1i3, *rri2i3;

	Word16 *p0, *p1, *p2, *p3;

	Word16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
	Word32 cor;
	Word16 i, k, ldec, l_fin_sup, l_fin_inf;
	Word16 h[SubFrLen2];

	/* Scaling for maximum precision */

	cor = 0;
	for (i = 0; i < SubFrLen; i++)
		cor = g723_L_mac(cor, H[i], H[i]);

	if (g723_extract_h(cor) > 32000) {
		for (i = 0; i < SubFrLen; i++)
			h[i + 4] = g723_shr(H[i], 1);
	} else {
		k = g723_norm_l(cor);
		k = g723_shr(k, 1);
		for (i = 0; i < SubFrLen; i++)
			h[i + 4] = g723_shl(H[i], k);
	}

	for (i = 0; i < 4; i++)
		h[i] = 0;

	/* Init pointers */

	rri0i0 = rr;
	rri1i1 = rri0i0 + NB_POS;
	rri2i2 = rri1i1 + NB_POS;
	rri3i3 = rri2i2 + NB_POS;

	rri0i1 = rri3i3 + NB_POS;
	rri0i2 = rri0i1 + MSIZE;
	rri0i3 = rri0i2 + MSIZE;
	rri1i2 = rri0i3 + MSIZE;
	rri1i3 = rri1i2 + MSIZE;
	rri2i3 = rri1i3 + MSIZE;

	/*
	 * Compute rri0i0[], rri1i1[], rri2i2[] and rri3i3[]
	 */

	p0 = rri0i0 + NB_POS - 1;	/* Init pointers to last position of rrixix[] */
	p1 = rri1i1 + NB_POS - 1;
	p2 = rri2i2 + NB_POS - 1;
	p3 = rri3i3 + NB_POS - 1;

	ptr_h1 = h;
	cor = 0;
	for (i = 0; i < NB_POS; i++) {
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		*p3-- = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		*p2-- = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		*p1-- = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h1);
		ptr_h1++;
		*p0-- = g723_extract_h(cor);
	}

	/*
	 * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
	 */

	l_fin_sup = MSIZE - 1;
	l_fin_inf = l_fin_sup - (Word16) 1;
	ldec = NB_POS + 1;

	ptr_hd = h;
	ptr_hf = ptr_hd + 2;

	for (k = 0; k < NB_POS; k++) {

		p3 = rri2i3 + l_fin_sup;
		p2 = rri1i2 + l_fin_sup;
		p1 = rri0i1 + l_fin_sup;
		p0 = rri0i3 + l_fin_inf;
		cor = 0;
		ptr_h1 = ptr_hd;
		ptr_h2 = ptr_hf;

		for (i = k + (Word16) 1; i < NB_POS; i++) {

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p3 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p2 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p1 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p0 = g723_extract_h(cor);

			p3 -= ldec;
			p2 -= ldec;
			p1 -= ldec;
			p0 -= ldec;
		}
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p3 = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p2 = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p1 = g723_extract_h(cor);

		l_fin_sup -= NB_POS;
		l_fin_inf--;
		ptr_hf += STEP;
	}

	/*
	 * Compute elements of: rri0i2[], rri1i3[]
	 */

	ptr_hd = h;
	ptr_hf = ptr_hd + 4;
	l_fin_sup = MSIZE - 1;
	l_fin_inf = l_fin_sup - (Word16) 1;
	for (k = 0; k < NB_POS; k++) {
		p3 = rri1i3 + l_fin_sup;
		p2 = rri0i2 + l_fin_sup;
		p1 = rri1i3 + l_fin_inf;
		p0 = rri0i2 + l_fin_inf;

		cor = 0;
		ptr_h1 = ptr_hd;
		ptr_h2 = ptr_hf;
		for (i = k + (Word16) 1; i < NB_POS; i++) {
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p3 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p2 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p1 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p0 = g723_extract_h(cor);

			p3 -= ldec;
			p2 -= ldec;
			p1 -= ldec;
			p0 -= ldec;
		}
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p3 = g723_extract_h(cor);

		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p2 = g723_extract_h(cor);

		l_fin_sup -= NB_POS;
		l_fin_inf--;
		ptr_hf += STEP;
	}

	/*
	 * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
	 */

	ptr_hd = h;
	ptr_hf = ptr_hd + 6;
	l_fin_sup = MSIZE - 1;
	l_fin_inf = l_fin_sup - (Word16) 1;
	for (k = 0; k < NB_POS; k++) {

		p3 = rri0i3 + l_fin_sup;
		p2 = rri2i3 + l_fin_inf;
		p1 = rri1i2 + l_fin_inf;
		p0 = rri0i1 + l_fin_inf;

		ptr_h1 = ptr_hd;
		ptr_h2 = ptr_hf;
		cor = 0;
		for (i = k + (Word16) 1; i < NB_POS; i++) {

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p3 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p2 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p1 = g723_extract_h(cor);

			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
			ptr_h1++;
			ptr_h2++;
			*p0 = g723_extract_h(cor);

			p3 -= ldec;
			p2 -= ldec;
			p1 -= ldec;
			p0 -= ldec;
		}
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		cor = g723_L_mac(cor, *ptr_h1, *ptr_h2);
		ptr_h1++;
		ptr_h2++;
		*p3 = g723_extract_h(cor);

		l_fin_sup -= NB_POS;
		l_fin_inf--;
		ptr_hf += STEP;
	}

	return;
}

/*
**
**  Function:     Corr_h_X()
**
**  Description:    Compute  correlations of input response h[] with
**                  the target vector X[].
**
**  Links to the text: Section 2.16
**
** Arguments:
**
**      Word16 h[]              Impulse response.
**      Word16 X[]              Target vector.
**      Word16 D[]              Correlations.
**
**  Outputs:
**
**      Word16 D[]              Correlations.
**
**  Return value:           None
*/
void Cor_h_X(Word16 h[], Word16 X[], Word16 D[])
{
	Word16 i, j;
	Word32 s, max;
	Word32 y32[SubFrLen];

	/* first keep the result on 32 bits and find absolute maximum */

	max = 0;

	for (i = 0; i < SubFrLen; i++) {
		s = 0;
		for (j = i; j < SubFrLen; j++)
			s = g723_L_mac(s, X[j], h[j - i]);

		y32[i] = s;

		s = g723_L_abs(s);
		if (s > max)
			max = s;
	}

	/*
	 * Find the number of right shifts to do on y32[]
	 * so that maximum is on 13 bits
	 */

	j = g723_norm_l(max);
	if (g723_sub(j, 16) > 0)
		j = 16;

	j = g723_sub(18, j);

	for (i = 0; i < SubFrLen; i++)
		D[i] = g723_extract_l(L_g723_shr(y32[i], j));

	return;
}

/*
** Function:            Reset_max_time()
**
**  Description:        This function should be called at the beginning
**                      of each frame.
**
**  Links to the text:  Section 2.16
**
**  Arguments:          None
**
**  Inputs:             None
**
**  Outputs:
**
**      Word16          extra
**
**  Return value:           None
**
*/
static Word16 extra;
void reset_max_time(void)
{
	extra = 120;
	return;
}

/*
**
**  Function:       D4i64_LBC
**
**  Description:       Algebraic codebook for LBC.
**                     -> 17 bits; 4 pulses in a frame of 60 samples
**
**                     The code length is 60, containing 4 nonzero pulses
**                     i0, i1, i2, i3. Each pulse can have 8 possible
**                     positions (positive or negative):
**
**                     i0 (+-1) : 0, 8,  16, 24, 32, 40, 48, 56
**                     i1 (+-1) : 2, 10, 18, 26, 34, 42, 50, 58
**                     i2 (+-1) : 4, 12, 20, 28, 36, 44, 52, (60)
**                     i3 (+-1) : 6, 14, 22, 30, 38, 46, 54, (62)
**
**                     All the pulse can be shifted by one.
**                     The last position of the last 2 pulses falls outside the
**                     frame and signifies that the pulse is not present.
**                     The threshold controls if a section of the innovative
**                     codebook should be searched or not.
**
**  Links to the text: Section 2.16
**
**  Input arguments:
**
**      Word16 Dn[]       Correlation between target vector and impulse response h[]
**      Word16 rr[]       Correlations of impulse response h[]
**      Word16 h[]        Impulse response of filters
**
**  Output arguments:
**
**      Word16 cod[]      Selected algebraic codeword
**      Word16 y[]        Filtered codeword
**      Word16 code_shift Shift of the codeword
**      Word16 sign       Signs of the 4 pulses.
**
**  Return value:
**
**      Word16   Index of selected codevector
**
*/
Word16 D4i64_LBC(Word16 Dn[], Word16 rr[], Word16 h[], Word16 cod[],
		 Word16 y[], Word16 * code_shift, Word16 * sign)
{
	Word16 i0, i1, i2, i3, ip0, ip1, ip2, ip3;
	Word16 i, j, time;
	Word16 shif, shift;
	Word16 ps0, ps1, ps2, ps3, alp, alp0;
	Word32 alp1, alp2, alp3, L32;
	Word16 ps0a, ps1a, ps2a;
	Word16 ps3c, psc, alpha;
	Word16 means, max0, max1, max2, thres;

	Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3;
	Word16 *rri0i1, *rri0i2, *rri0i3;
	Word16 *rri1i2, *rri1i3, *rri2i3;

	Word16 *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3;
	Word16 *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3;
	Word16 *ptr_ri1i2, *ptr_ri1i3, *ptr_ri2i3;

	Word16 *ptr1_ri0i1, *ptr1_ri0i2, *ptr1_ri0i3;
	Word16 *ptr1_ri1i2, *ptr1_ri1i3, *ptr1_ri2i3;

	Word16 p_sign[SubFrLen2 / 2];

	/* Init pointers */

	rri0i0 = rr;
	rri1i1 = rri0i0 + NB_POS;
	rri2i2 = rri1i1 + NB_POS;
	rri3i3 = rri2i2 + NB_POS;

	rri0i1 = rri3i3 + NB_POS;
	rri0i2 = rri0i1 + MSIZE;
	rri0i3 = rri0i2 + MSIZE;
	rri1i2 = rri0i3 + MSIZE;
	rri1i3 = rri1i2 + MSIZE;
	rri2i3 = rri1i3 + MSIZE;

	/*
	 * Extend the backward filtered target vector by zeros
	 */

	for (i = SubFrLen; i < SubFrLen2; i++)
		Dn[i] = 0;

	/*
	 * Chose the sign of the impulse.
	 */

	for (i = 0; i < SubFrLen; i += 2) {
		if (g723_add(Dn[i], Dn[i + 1]) >= 0) {
			p_sign[i / 2] = 1;
		} else {
			p_sign[i / 2] = -1;
			Dn[i] = -Dn[i];
			Dn[i + 1] = -Dn[i + 1];
		}
	}
	p_sign[30] = p_sign[31] = 1;

	/*
	 *   Compute the search threshold after three pulses
	 */

	/* odd positions */
	/* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

	max0 = Dn[0];
	max1 = Dn[2];
	max2 = Dn[4];
	for (i = 8; i < SubFrLen; i += STEP) {
		if (Dn[i] > max0)
			max0 = Dn[i];
		if (Dn[i + 2] > max1)
			max1 = Dn[i + 2];
		if (Dn[i + 4] > max2)
			max2 = Dn[i + 4];
	}
	max0 = g723_add(max0, max1);
	max0 = g723_add(max0, max2);

	/* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

	L32 = 0;
	for (i = 0; i < SubFrLen; i += STEP) {
		L32 = g723_L_mac(L32, Dn[i], 1);
		L32 = g723_L_mac(L32, Dn[i + 2], 1);
		L32 = g723_L_mac(L32, Dn[i + 4], 1);
	}
	means = g723_extract_l(L_g723_shr(L32, 4));

	/* thres = means + (max0-means)*threshold; */

	thres = g723_sub(max0, means);
	thres = g723_mult(thres, threshold);
	thres = g723_add(thres, means);

	/* even positions */
	/* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

	max0 = Dn[1];
	max1 = Dn[3];
	max2 = Dn[5];
	for (i = 9; i < SubFrLen; i += STEP) {
		if (Dn[i] > max0)
			max0 = Dn[i];
		if (Dn[i + 2] > max1)
			max1 = Dn[i + 2];
		if (Dn[i + 4] > max2)
			max2 = Dn[i + 4];
	}
	max0 = g723_add(max0, max1);
	max0 = g723_add(max0, max2);

	/* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

	L32 = 0;
	for (i = 1; i < SubFrLen; i += STEP) {
		L32 = g723_L_mac(L32, Dn[i], 1);
		L32 = g723_L_mac(L32, Dn[i + 2], 1);
		L32 = g723_L_mac(L32, Dn[i + 4], 1);
	}
	means = g723_extract_l(L_g723_shr(L32, 4));

	/* max1 = means + (max0-means)*threshold */

	max1 = g723_sub(max0, means);
	max1 = g723_mult(max1, threshold);
	max1 = g723_add(max1, means);

	/* Keep maximum threshold between odd and even position */

	if (max1 > thres)
		thres = max1;

	/*
	 * Modification of rrixiy[] to take signs into account.
	 */

	ptr_ri0i1 = rri0i1;
	ptr_ri0i2 = rri0i2;
	ptr_ri0i3 = rri0i3;
	ptr1_ri0i1 = rri0i1;
	ptr1_ri0i2 = rri0i2;
	ptr1_ri0i3 = rri0i3;

	for (i0 = 0; i0 < SubFrLen / 2; i0 += STEP / 2) {
		for (i1 = 2 / 2; i1 < SubFrLen / 2; i1 += STEP / 2) {
			*ptr_ri0i1++ = i_g723_mult(*ptr1_ri0i1++,
						   i_g723_mult(p_sign[i0],
							       p_sign[i1]));
			*ptr_ri0i2++ =
			    i_g723_mult(*ptr1_ri0i2++,
					i_g723_mult(p_sign[i0],
						    p_sign[i1 + 1]));
			*ptr_ri0i3++ =
			    i_g723_mult(*ptr1_ri0i3++,
					i_g723_mult(p_sign[i0],
						    p_sign[i1 + 2]));
		}
	}

	ptr_ri1i2 = rri1i2;
	ptr_ri1i3 = rri1i3;
	ptr1_ri1i2 = rri1i2;
	ptr1_ri1i3 = rri1i3;
	for (i1 = 2 / 2; i1 < SubFrLen / 2; i1 += STEP / 2) {
		for (i2 = 4 / 2; i2 < SubFrLen2 / 2; i2 += STEP / 2) {
			*ptr_ri1i2++ = i_g723_mult(*ptr1_ri1i2++,
						   i_g723_mult(p_sign[i1],
							       p_sign[i2]));
			*ptr_ri1i3++ =
			    i_g723_mult(*ptr1_ri1i3++,
					i_g723_mult(p_sign[i1],
						    p_sign[i2 + 1]));

		}
	}

	ptr_ri2i3 = rri2i3;

	ptr1_ri2i3 = rri2i3;
	for (i2 = 4 / 2; i2 < SubFrLen2 / 2; i2 += STEP / 2) {
		for (i3 = 6 / 2; i3 < SubFrLen2 / 2; i3 += STEP / 2)
			*ptr_ri2i3++ =
			    i_g723_mult(*ptr1_ri2i3++,
					i_g723_mult(p_sign[i2], p_sign[i3]));
	}

	/*
	 * Search the optimum positions of the four  pulses which maximize
	 *     square(correlation) / energy
	 * The search is performed in four  nested loops. At each loop, one
	 * pulse contribution is added to the correlation and energy.
	 *
	 * The fourth loop is entered only if the correlation due to the
	 *  contribution of the first three pulses exceeds the preset
	 *  threshold.
	 */

	/* Default values */
	ip0 = 0;
	ip1 = 2;
	ip2 = 4;
	ip3 = 6;
	shif = 0;
	psc = 0;
	alpha = 32767;
	time = g723_add(max_time, extra);

	/* Four loops to search innovation code. */

	/* Init. pointers that depend on first loop */
	ptr_ri0i0 = rri0i0;
	ptr_ri0i1 = rri0i1;
	ptr_ri0i2 = rri0i2;
	ptr_ri0i3 = rri0i3;

	/* first pulse loop  */
	for (i0 = 0; i0 < SubFrLen; i0 += STEP) {

		ps0 = Dn[i0];
		ps0a = Dn[i0 + 1];
		alp0 = *ptr_ri0i0++;

		/* Init. pointers that depend on second loop */
		ptr_ri1i1 = rri1i1;
		ptr_ri1i2 = rri1i2;
		ptr_ri1i3 = rri1i3;

		/* second pulse loop */
		for (i1 = 2; i1 < SubFrLen; i1 += STEP) {

			ps1 = g723_add(ps0, Dn[i1]);
			ps1a = g723_add(ps0a, Dn[i1 + 1]);

			/* alp1 = alp0 + *ptr_ri1i1++ + 2.0 * ( *ptr_ri0i1++); */

			alp1 = L_g723_mult(alp0, 1);
			alp1 = g723_L_mac(alp1, *ptr_ri1i1++, 1);
			alp1 = g723_L_mac(alp1, *ptr_ri0i1++, 2);

			/* Init. pointers that depend on third loop */
			ptr_ri2i2 = rri2i2;
			ptr_ri2i3 = rri2i3;

			/* third pulse loop */
			for (i2 = 4; i2 < SubFrLen2; i2 += STEP) {

				ps2 = g723_add(ps1, Dn[i2]);
				ps2a = g723_add(ps1a, Dn[i2 + 1]);

				/* alp2 = alp1 + *ptr_ri2i2++
				   + 2.0 * (*ptr_ri0i2++ + *ptr_ri1i2++); */

				alp2 = g723_L_mac(alp1, *ptr_ri2i2++, 1);
				alp2 = g723_L_mac(alp2, *ptr_ri0i2++, 2);
				alp2 = g723_L_mac(alp2, *ptr_ri1i2++, 2);

				/* Decide the shift */

				shift = 0;
				if (ps2a > ps2) {
					shift = 1;
					ps2 = ps2a;
				}

				/* Test threshold */

				if (ps2 > thres) {

					/* Init. pointers that depend on 4th loop */
					ptr_ri3i3 = rri3i3;

					/* 4th pulse loop */
					for (i3 = 6; i3 < SubFrLen2; i3 += STEP) {

						ps3 =
						    g723_add(ps2,
							     Dn[i3 + shift]);

						/* alp3 = alp2 + (*ptr_ri3i3++) +
						   2 x ( (*ptr_ri0i3++) +
						   (*ptr_ri1i3++) +
						   (*ptr_ri2i3++) ) */

						alp3 =
						    g723_L_mac(alp2,
							       *ptr_ri3i3++, 1);
						alp3 =
						    g723_L_mac(alp3,
							       *ptr_ri0i3++, 2);
						alp3 =
						    g723_L_mac(alp3,
							       *ptr_ri1i3++, 2);
						alp3 =
						    g723_L_mac(alp3,
							       *ptr_ri2i3++, 2);
						alp =
						    g723_extract_l(L_g723_shr
								   (alp3, 5));

						ps3c = g723_mult(ps3, ps3);
						if (L_g723_mult(ps3c, alpha) >
						    L_g723_mult(psc, alp)) {
							psc = ps3c;
							alpha = alp;
							ip0 = i0;
							ip1 = i1;
							ip2 = i2;
							ip3 = i3;
							shif = shift;
						}
					}	/*  end of for i3 = */

					time--;
					if (time <= 0)
						goto end_search;	/* Max time finish */
					ptr_ri0i3 -= NB_POS;
					ptr_ri1i3 -= NB_POS;

				}
				/* end of if >thres */
				else {
					ptr_ri2i3 += NB_POS;
				}

			}	/* end of for i2 = */

			ptr_ri0i2 -= NB_POS;
			ptr_ri1i3 += NB_POS;

		}		/* end of for i1 = */

		ptr_ri0i2 += NB_POS;
		ptr_ri0i3 += NB_POS;

	}			/* end of for i0 = */

 end_search:

	extra = time;

	/* Set the sign of impulses */

	i0 = p_sign[g723_shr(ip0, 1)];
	i1 = p_sign[g723_shr(ip1, 1)];
	i2 = p_sign[g723_shr(ip2, 1)];
	i3 = p_sign[g723_shr(ip3, 1)];

	/* Find the codeword corresponding to the selected positions */

	for (i = 0; i < SubFrLen; i++)
		cod[i] = 0;

	if (shif > 0) {
		ip0 = g723_add(ip0, 1);
		ip1 = g723_add(ip1, 1);
		ip2 = g723_add(ip2, 1);
		ip3 = g723_add(ip3, 1);
	}

	cod[ip0] = i0;
	cod[ip1] = i1;
	if (ip2 < SubFrLen)
		cod[ip2] = i2;
	if (ip3 < SubFrLen)
		cod[ip3] = i3;

	/* find the filtered codeword */

	for (i = 0; i < SubFrLen; i++)
		y[i] = 0;

	if (i0 > 0)
		for (i = ip0, j = 0; i < SubFrLen; i++, j++)
			y[i] = g723_add(y[i], h[j]);
	else
		for (i = ip0, j = 0; i < SubFrLen; i++, j++)
			y[i] = g723_sub(y[i], h[j]);

	if (i1 > 0)
		for (i = ip1, j = 0; i < SubFrLen; i++, j++)
			y[i] = g723_add(y[i], h[j]);
	else
		for (i = ip1, j = 0; i < SubFrLen; i++, j++)
			y[i] = g723_sub(y[i], h[j]);

	if (ip2 < SubFrLen) {

		if (i2 > 0)
			for (i = ip2, j = 0; i < SubFrLen; i++, j++)
				y[i] = g723_add(y[i], h[j]);
		else
			for (i = ip2, j = 0; i < SubFrLen; i++, j++)
				y[i] = g723_sub(y[i], h[j]);
	}

	if (ip3 < SubFrLen) {

		if (i3 > 0)
			for (i = ip3, j = 0; i < SubFrLen; i++, j++)
				y[i] = g723_add(y[i], h[j]);
		else
			for (i = ip3, j = 0; i < SubFrLen; i++, j++)
				y[i] = g723_sub(y[i], h[j]);
	}

	/* find codebook index;  17-bit address */

	*code_shift = shif;

	*sign = 0;
	if (i0 > 0)
		*sign = g723_add(*sign, 1);
	if (i1 > 0)
		*sign = g723_add(*sign, 2);
	if (i2 > 0)
		*sign = g723_add(*sign, 4);
	if (i3 > 0)
		*sign = g723_add(*sign, 8);

	i = g723_shr(ip0, 3);
	i = g723_add(i, g723_shl(g723_shr(ip1, 3), 3));
	i = g723_add(i, g723_shl(g723_shr(ip2, 3), 6));
	i = g723_add(i, g723_shl(g723_shr(ip3, 3), 9));

	return i;
}

/*
**
**  Function:  G_code()
**
**  Description: Compute the gain of innovative code.
**
**
**  Links to the text: Section 2.16
**
** Input arguments:
**
**      Word16 X[]        Code target.  (in Q0)
**      Word16 Y[]        Filtered innovation code. (in Q12)
**
** Output:
**
**      Word16 *gain_q    Gain of innovation code.  (in Q0)
**
**  Return value:
**
**      Word16  index of innovation code gain
**
*/
Word16 G_code(Word16 X[], Word16 Y[], Word16 * gain_q)
{
	Word16 i;
	Word16 xy, yy, exp_xy, exp_yy, gain, gain_nq;
	Word32 L_xy, L_yy;
	Word16 dist, dist_min;

	/* Scale down Y[] by 8 to avoid overflow */
	for (i = 0; i < SubFrLen; i++)
		Y[i] = g723_shr(Y[i], 3);

	/* Compute scalar product <X[],Y[]> */
	L_xy = 0L;
	for (i = 0; i < SubFrLen; i++)
		L_xy = g723_L_mac(L_xy, X[i], Y[i]);

	exp_xy = g723_norm_l(L_xy);
	xy = g723_extract_h(L_g723_shl(L_xy, exp_xy));

	if (xy <= 0) {
		gain = 0;
		*gain_q = FcbkGainTable[gain];
		return (gain);
	}

	/* Compute scalar product <Y[],Y[]> */
	L_yy = 0L;
	for (i = 0; i < SubFrLen; i++)
		L_yy = g723_L_mac(L_yy, Y[i], Y[i]);

	exp_yy = g723_norm_l(L_yy);
	yy = g723_extract_h(L_g723_shl(L_yy, exp_yy));

	/* compute gain = xy/yy */
	xy = g723_shr(xy, 1);	/* Be sure xy < yy */
	gain_nq = div_s(xy, yy);

	i = g723_add(exp_xy, 5);	/* Denormalization of division */
	i = g723_sub(i, exp_yy);

	gain_nq = g723_shr(gain_nq, i);

	gain = (Word16) 0;
	dist_min = g723_sub(gain_nq, FcbkGainTable[0]);
	dist_min = g723_abs_s(dist_min);
	for (i = 1; i < NumOfGainLev; i++) {
		dist = g723_sub(gain_nq, FcbkGainTable[i]);
		dist = g723_abs_s(dist);
		if (dist < dist_min) {
			dist_min = dist;
			gain = (Word16) i;
		}
	}
	*gain_q = FcbkGainTable[gain];

	return (gain);
}

/*
**
**  Function:       search_T0()
**
**  Description:          Gets parameters of pitch synchronous filter
**
**  Links to the text:    Section 2.16
**
**  Arguments:
**
**      Word16 T0         Decoded pitch lag
**      Word16 Gid        Gain vector index in the adaptive gain vector codebook
**      Word16 *gain_T0   Pitch synchronous gain
**
**  Outputs:
**
**      Word16 *gain_T0   Pitch synchronous filter gain
**
**  Return Value:
**
**      Word16 T0_mod     Pitch synchronous filter lag
*/
Word16 search_T0(Word16 T0, Word16 Gid, Word16 * gain_T0)
{

	Word16 T0_mod;

	T0_mod = T0 + epsi170[Gid];
	*gain_T0 = gain170[Gid];

	return (T0_mod);
}
