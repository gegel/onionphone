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
extern int32_t g723_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,    1 */
extern int16_t g723_sub(int16_t var1, int16_t var2);	/* Short sub,           1 */
extern int32_t g723_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,    1 */
extern int16_t g723_norm_l(int32_t L_var1);	/* Long norm,            30 */
extern int32_t L_g723_shl(int32_t L_var1, int16_t var2);	/* Long shift left,     2 */
extern int16_t g723_shl(int16_t var1, int16_t var2);	/* Short shift left,    1 */
//extern int32_t L_g723_shl(int32_t L_var1, int16_t var2); /* Long shift left,     2 */
extern int16_t round_(int32_t L_var1);	/* Round,               1 */
extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */
extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int16_t g723_shr(int16_t var1, int16_t var2);	/* Short shift right,   1 */
extern int32_t L_g723_negate(int32_t L_var1);	/* Long negate,     2 */
extern int32_t L_g723_shr(int32_t L_var1, int16_t var2);	/* Long shift right,    2 */
extern int32_t g723_L_abs(int32_t L_var1);	/* Long abs,              3 */
extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */
extern int32_t L_g723_sub(int32_t L_var1, int32_t L_var2);	/* Long sub,        2 */
extern int16_t div_s(int16_t var1, int16_t var2);	/* Short division,       18 */
extern int32_t g723_L_deposit_h(int16_t var1);	/* 16 bit var1 -> MSB,     2 */
extern int16_t i_g723_mult(int16_t a, int16_t b);

extern int16_t g723_mult(int16_t var1, int16_t var2);	/* Short mult,          1 */
extern int16_t Test_Err(int16_t Lag1, int16_t Lag2);

//extern void  Gen_Trn( int16_t *Dst, int16_t *Src, int16_t Olp );
//extern int16_t search_T0 ( int16_t T0, int16_t Gid, int16_t *gain_T0);
extern int16_t g723_extract_l(int32_t L_var1);	/* Extract low,         1 */
extern int16_t g723_negate(int16_t var1);	/* Short negate,        1 */
extern int16_t g723_abs_s(int16_t var1);	/* Short abs,           1 */

extern int32_t CombinatorialTable[MaxPulseNum][SubFrLen / Sgrid];
extern int16_t Nb_puls[4];
extern int32_t MaxPosTable[4];
extern int16_t LpfConstTable[2];
extern int16_t FcbkGainTable[NumOfGainLev];
extern int16_t epsi170[170];
extern int16_t gain170[170];
extern int16_t tabgain170[170];

extern int16_t div_l(int32_t, int16_t);
extern int16_t Sqrt_lbc(int32_t Num);

extern int16_t Rand_lbc(int16_t * p);
extern int16_t Vec_Norm(int16_t * Vect, int16_t Len);
extern int16_t *AcbkGainTablePtr[2];
extern enum Crate WrkRate;

int16_t Estim_Pitch(int16_t * Dpnt, int16_t Start);
PWDEF Comp_Pw(int16_t * Dpnt, int16_t Start, int16_t Olp);
void Filt_Pw(int16_t * DataBuff, int16_t * Dpnt, int16_t Start, PWDEF Pw);
void Find_Fcbk(int16_t * Dpnt, int16_t * ImpResp, LINEDEF * Line,
	       int16_t Sfc);
void Gen_Trn(int16_t * Dst, int16_t * Src, int16_t Olp);
void Find_Best(BESTDEF * Best, int16_t * Tv, int16_t * ImpResp, int16_t Np,
	       int16_t Olp);
void Fcbk_Pack(int16_t * Dpnt, SFSDEF * Sfs, BESTDEF * Best, int16_t Np);
void Fcbk_Unpk(int16_t * Tv, SFSDEF Sfs, int16_t Olp, int16_t Sfc);
void Find_Acbk(int16_t * Tv, int16_t * ImpResp, int16_t * PrevExc, LINEDEF
	       * Line, int16_t Sfc);
void Get_Rez(int16_t * Tv, int16_t * PrevExc, int16_t Lag);
void Decod_Acbk(int16_t * Tv, int16_t * PrevExc, int16_t Olp, int16_t Lid,
		int16_t Gid);
int16_t Comp_Info(int16_t * Buff, int16_t Olp, int16_t * Gain,
		  int16_t * ShGain);
void Regen(int16_t * DataBuff, int16_t * Buff, int16_t Lag, int16_t Gain,
	   int16_t Ecount, int16_t * Sd);
PFDEF Comp_Lpf(int16_t * Buff, int16_t Olp, int16_t Sfc);
int16_t Find_B(int16_t * Buff, int16_t Olp, int16_t Sfc);
int16_t Find_F(int16_t * Buff, int16_t Olp, int16_t Sfc);
PFDEF Get_Ind(int16_t Ind, int16_t Ten, int16_t Ccr, int16_t Enr);
void Filt_Lpf(int16_t * Tv, int16_t * Buff, PFDEF Pf, int16_t Sfc);
void reset_max_time(void);
int16_t search_T0(int16_t T0, int16_t Gid, int16_t * gain_T0);
int16_t ACELP_LBC_code(int16_t X[], int16_t h[], int16_t T0, int16_t code[],
		      int16_t * gain, int16_t * shift, int16_t * sign,
		      int16_t gain_T0);
void Cor_h(int16_t * H, int16_t * rr);
void Cor_h_X(int16_t h[], int16_t X[], int16_t D[]);
int16_t D4i64_LBC(int16_t Dn[], int16_t rr[], int16_t h[], int16_t cod[],
		 int16_t y[], int16_t * code_shift, int16_t * sign);
int16_t G_code(int16_t X[], int16_t Y[], int16_t * gain_q);

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
**  int16_t *Dpnt    Perceptually weighted speech
**  int16_t Start    Starting index defining the subframes under study
**
** Outputs:
**
** Return value:
**
**  int16_t      Open loop pitch period
**
*/
int16_t Estim_Pitch(int16_t * Dpnt, int16_t Start)
{
	int i, j;

	int32_t Acc0, Acc1;

	int16_t Exp, Tmp;
	int16_t Ccr, Enr;

	int16_t Indx = (int16_t) PitchMin;

	int16_t Mxp = (int16_t) 30;
	int16_t Mcr = (int16_t) 0x4000;
	int16_t Mnr = (int16_t) 0x7fff;

	int16_t Pr;

	/* Init the energy estimate */
	Pr = Start - (int16_t) PitchMin + (int16_t) 1;
	Acc1 = (int32_t) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc1 = g723_L_mac(Acc1, Dpnt[Pr + j], Dpnt[Pr + j]);

	/* Main Olp search loop */
	for (i = PitchMin; i <= PitchMax - 3; i++) {

		Pr = g723_sub(Pr, (int16_t) 1);

		/* Energy update */
		Acc1 =
		    g723_L_msu(Acc1, Dpnt[Pr + 2 * SubFrLen],
			       Dpnt[Pr + 2 * SubFrLen]);
		Acc1 = g723_L_mac(Acc1, Dpnt[Pr], Dpnt[Pr]);

		/*  Compute the cross */
		Acc0 = (int32_t) 0;
		for (j = 0; j < 2 * SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, Dpnt[Start + j], Dpnt[Pr + j]);

		if (Acc0 > (int32_t) 0) {

			/* Compute Exp and mant of the cross */
			Exp = g723_norm_l(Acc0);
			Acc0 = L_g723_shl(Acc0, Exp);
			Exp = g723_shl(Exp, (int16_t) 1);
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
				Exp = g723_sub(Exp, (int16_t) 1);
				Ccr = g723_shr(Ccr, (int16_t) 1);
			}

			if (Exp <= Mxp) {

				if ((Exp + 1) < Mxp) {
					Indx = (int16_t) i;
					Mxp = Exp;
					Mcr = Ccr;
					Mnr = Enr;
					continue;
				}

				if ((Exp + 1) == Mxp)
					Tmp = g723_shr(Mcr, (int16_t) 1);
				else
					Tmp = Mcr;

				/* Compare with equal exponents */
				Acc0 = L_g723_mult(Ccr, Mnr);
				Acc0 = g723_L_msu(Acc0, Enr, Tmp);
				if (Acc0 > (int32_t) 0) {

					if (((int16_t) i - Indx) <
					    (int16_t) PitchMin) {
						Indx = (int16_t) i;
						Mxp = Exp;
						Mcr = Ccr;
						Mnr = Enr;
					}

					else {
						Acc0 = L_g723_mult(Ccr, Mnr);
						Acc0 =
						    L_g723_negate(L_g723_shr
								  (Acc0,
								   (int16_t) 2));
						Acc0 =
						    g723_L_mac(Acc0, Ccr, Mnr);
						Acc0 =
						    g723_L_msu(Acc0, Enr, Tmp);
						if (Acc0 > (int32_t) 0) {
							Indx = (int16_t) i;
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
**  int16_t *Dpnt    Formant perceptually weighted speech
**  int16_t Start
**  int16_t Olp      Open loop pitch lag
**
** Outputs:         None
**
** Return value:
**
**  PWDEF   int16_t  Indx  lag of the harmonic noise shaping filter
**          int16_t  Gain  gain of the harmonic noise shaping filter
**
*/
PWDEF Comp_Pw(int16_t * Dpnt, int16_t Start, int16_t Olp)
{

	int i, j;

	int32_t Lcr[15];
	int16_t Scr[15];
	PWDEF Pw;

	int32_t Acc0, Acc1;
	int16_t Exp;

	int16_t Ccr, Enr;
	int16_t Mcr, Mnr;

	/* Compute and save target energy */
	Lcr[0] = (int32_t) 0;
	for (i = 0; i < SubFrLen; i++)
		Lcr[0] = g723_L_mac(Lcr[0], Dpnt[Start + i], Dpnt[Start + i]);

	/* Compute all Crosses and energys */
	for (i = 0; i <= 2 * PwRange; i++) {

		Acc1 = Acc0 = (int32_t) 0;
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
	Acc1 = (int32_t) 0;
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
	Pw.Indx = (int16_t) - 1;
	Pw.Gain = (int16_t) 0;

	Mcr = (int16_t) 1;
	Mnr = (int16_t) 0x7fff;

	for (i = 0; i <= 2 * PwRange; i++) {

		Enr = Scr[2 * i + 1];
		Ccr = Scr[2 * i + 2];

		if (Ccr <= (int16_t) 0)
			continue;

		Exp = g723_mult_r(Ccr, Ccr);

		/* Compute the cross */
		Acc0 = L_g723_mult(Exp, Mnr);
		Acc0 = g723_L_msu(Acc0, Enr, Mcr);

		if (Acc0 > (int32_t) 0) {
			Mcr = Exp;
			Mnr = Enr;
			Pw.Indx = (int16_t) i;
		}
	}

	if (Pw.Indx == -1) {
		Pw.Indx = Olp;
		return Pw;
	}

	/* Check the db limit */
	Acc0 = L_g723_mult(Scr[0], Mnr);
	Acc1 = Acc0;
	Acc0 = L_g723_shr(Acc0, (int16_t) 2);
	Acc1 = L_g723_shr(Acc1, (int16_t) 3);
	Acc0 = L_g723_add(Acc0, Acc1);
	Acc1 = L_g723_mult(Scr[2 * Pw.Indx + 2], Scr[2 * Pw.Indx + 2]);
	Acc0 = L_g723_sub(Acc0, Acc1);

	if (Acc0 < (int32_t) 0) {

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
**  int16_t *DataBuff    Target vector
**  int16_t *Dpnt        Formant perceptually weighted speech
**  int16_t Start
**  PWDEF   Pw          Parameters of the harmonic noise shaping filter
**
** Outputs:
**
**  int16_t *DataBuff    Target vector
**
** Return value:        None
**
*/
void Filt_Pw(int16_t * DataBuff, int16_t * Dpnt, int16_t Start, PWDEF Pw)
{
	int i;

	int32_t Acc0;

	/* Perform the harmonic weighting */
	for (i = 0; i < SubFrLen; i++) {
		Acc0 = g723_L_deposit_h(Dpnt[PitchMax + Start + i]);
		Acc0 =
		    g723_L_msu(Acc0, Pw.Gain,
			       Dpnt[PitchMax + Start - Pw.Indx + i]);
		DataBuff[Start + (int16_t) i] = round_(Acc0);
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
**  int16_t *Dpnt    Target vector
**  int16_t *ImpResp Impulse response of the synthesis filter
**  LineDef *Line   Excitation parameters for one subframe
**  int16_t Sfc      Subframe index
**
** Outputs:
**
**  int16_t *Dpnt    Excitation vector
**  LINEDEF *Line   Fixed codebook parameters for one subframe
**
** Return value:        None
**
*/
void Find_Fcbk(int16_t * Dpnt, int16_t * ImpResp, LINEDEF * Line,
	       int16_t Sfc)
{
	int i;
	int16_t T0_acelp, gain_T0;
	int16_t Srate;

	BESTDEF Best;

	switch (WrkRate) {

	case Rate63:{

			Srate = Nb_puls[(int)Sfc];
			Best.MaxErr = (int32_t) 0xc0000000L;
			Find_Best(&Best, Dpnt, ImpResp, Srate,
				  (int16_t) SubFrLen);
			if ((*Line).Olp[Sfc >> 1] < (int16_t) (SubFrLen - 2)) {
				Find_Best(&Best, Dpnt, ImpResp, Srate,
					  (*Line).Olp[Sfc >> 1]);
			}

			/* Reconstruct the excitation */
			for (i = 0; i < SubFrLen; i++)
				Dpnt[i] = (int16_t) 0;
			for (i = 0; i < Srate; i++)
				Dpnt[Best.Ploc[i]] = Best.Pamp[i];

			/* Code the excitation */
			Fcbk_Pack(Dpnt, &((*Line).Sfs[Sfc]), &Best, Srate);

			if (Best.UseTrn == (int16_t) 1)
				Gen_Trn(Dpnt, Dpnt, (*Line).Olp[Sfc >> 1]);

			break;
		}

	case Rate53:{

			T0_acelp = search_T0((int16_t)
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
**  int16_t *Dst     Fixed codebook excitation vector with  train of Dirac
**  int16_t *Src     Fixed codebook excitation vector without train of Dirac
**  int16_t Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  int16_t *Dst     excitation vector
**
** Return value:    None
**
*/
void Gen_Trn(int16_t * Dst, int16_t * Src, int16_t Olp)
{
	int i;

	int16_t Tmp0, Tmp1;
	int16_t Tmp[SubFrLen];

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
**  int16_t *Tv      Target vector
**  int16_t *ImpResp Impulse response of the combined filter
**  int16_t Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**  int16_t Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  BESTDEF *Best
**
** Return value:    None
**
*/
void Find_Best(BESTDEF * Best, int16_t * Tv, int16_t * ImpResp, int16_t Np,
	       int16_t Olp)
{

	int i, j, k, l;
	BESTDEF Temp;

	int16_t Exp;
	int16_t MaxAmpId;
	int16_t MaxAmp;
	int32_t Acc0, Acc1, Acc2;

	int16_t Imr[SubFrLen];
	int16_t OccPos[SubFrLen];
	int16_t ImrCorr[SubFrLen];
	int32_t ErrBlk[SubFrLen];
	int32_t WrkBlk[SubFrLen];

	/* Update Impulse response */
	if (Olp < (int16_t) (SubFrLen - 2)) {
		Temp.UseTrn = (int16_t) 1;
		Gen_Trn(Imr, ImpResp, Olp);
	} else {
		Temp.UseTrn = (int16_t) 0;
		for (i = 0; i < SubFrLen; i++)
			Imr[i] = ImpResp[i];
	}

	/* Scale Imr to avoid overflow */
	for (i = 0; i < SubFrLen; i++)
		OccPos[i] = g723_shr(Imr[i], (int16_t) 1);

	/* Compute Imr AutoCorr function */
	Acc0 = (int32_t) 0;
	for (i = 0; i < SubFrLen; i++)
		Acc0 = g723_L_mac(Acc0, OccPos[i], OccPos[i]);

	Exp = g723_norm_l(Acc0);
	Acc0 = L_g723_shl(Acc0, Exp);
	ImrCorr[0] = round_(Acc0);

	/* Compute all the other */
	for (i = 1; i < SubFrLen; i++) {
		Acc0 = (int32_t) 0;
		for (j = i; j < SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, OccPos[j], OccPos[j - i]);
		Acc0 = L_g723_shl(Acc0, Exp);
		ImrCorr[i] = round_(Acc0);
	}

	/* Cross correlation with the signal */
	Exp = g723_sub(Exp, 4);
	for (i = 0; i < SubFrLen; i++) {
		Acc0 = (int32_t) 0;
		for (j = i; j < SubFrLen; j++)
			Acc0 = g723_L_mac(Acc0, Tv[j], Imr[j - i]);
		ErrBlk[i] = L_g723_shl(Acc0, Exp);
	}

	/* Search for the best sequence */
	for (k = 0; k < Sgrid; k++) {

		Temp.GridId = (int16_t) k;

		/* Find maximum amplitude */
		Acc1 = (int32_t) 0;
		for (i = k; i < SubFrLen; i += Sgrid) {
			Acc0 = g723_L_abs(ErrBlk[i]);
			if (Acc0 >= Acc1) {
				Acc1 = Acc0;
				Temp.Ploc[0] = (int16_t) i;
			}
		}

		/* Quantize the maximum amplitude */
		Acc2 = Acc1;
		Acc1 = (int32_t) 0x40000000L;
		MaxAmpId = (int16_t) (NumOfGainLev - MlqSteps);

		for (i = MaxAmpId; i >= MlqSteps; i--) {
			Acc0 = L_g723_mult(FcbkGainTable[i], ImrCorr[0]);
			Acc0 = L_g723_sub(Acc0, Acc2);
			Acc0 = g723_L_abs(Acc0);
			if (Acc0 < Acc1) {
				Acc1 = Acc0;
				MaxAmpId = (int16_t) i;
			}
		}
		MaxAmpId--;

		for (i = 1; i <= 2 * MlqSteps; i++) {

			for (j = k; j < SubFrLen; j += Sgrid) {
				WrkBlk[j] = ErrBlk[j];
				OccPos[j] = (int16_t) 0;
			}
			Temp.MampId = MaxAmpId - (int16_t) MlqSteps + (int16_t) i;

			MaxAmp = FcbkGainTable[Temp.MampId];

			if (WrkBlk[Temp.Ploc[0]] >= (int32_t) 0)
				Temp.Pamp[0] = MaxAmp;
			else
				Temp.Pamp[0] = g723_negate(MaxAmp);

			OccPos[Temp.Ploc[0]] = (int16_t) 1;

			for (j = 1; j < Np; j++) {

				Acc1 = (int32_t) 0xc0000000L;

				for (l = k; l < SubFrLen; l += Sgrid) {

					if (OccPos[l] != (int16_t) 0)
						continue;

					Acc0 = WrkBlk[l];
					Acc0 =
					    g723_L_msu(Acc0, Temp.Pamp[j - 1],
						       ImrCorr[g723_abs_s
							       ((int16_t)
								(l -
								 Temp.Ploc[j -
									   1]))]);
					WrkBlk[l] = Acc0;
					Acc0 = g723_L_abs(Acc0);
					if (Acc0 > Acc1) {
						Acc1 = Acc0;
						Temp.Ploc[j] = (int16_t) l;
					}
				}

				if (WrkBlk[Temp.Ploc[j]] >= (int32_t) 0)
					Temp.Pamp[j] = MaxAmp;
				else
					Temp.Pamp[j] = g723_negate(MaxAmp);

				OccPos[Temp.Ploc[j]] = (int16_t) 1;
			}

			/* Compute error vector */
			for (j = 0; j < SubFrLen; j++)
				OccPos[j] = (int16_t) 0;

			for (j = 0; j < Np; j++)
				OccPos[Temp.Ploc[j]] = Temp.Pamp[j];

			for (l = SubFrLen - 1; l >= 0; l--) {
				Acc0 = (int32_t) 0;
				for (j = 0; j <= l; j++)
					Acc0 =
					    g723_L_mac(Acc0, OccPos[j],
						       Imr[l - j]);
				Acc0 = L_g723_shl(Acc0, (int16_t) 2);
				OccPos[l] = g723_extract_h(Acc0);
			}

			/* Evaluate error */
			Acc1 = (int32_t) 0;
			for (j = 0; j < SubFrLen; j++) {
				Acc1 = g723_L_mac(Acc1, Tv[j], OccPos[j]);
				Acc0 = L_g723_mult(OccPos[j], OccPos[j]);
				Acc1 =
				    L_g723_sub(Acc1,
					       L_g723_shr(Acc0, (int16_t) 1));
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
**  int16_t *Dpnt    Excitation vector
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**  BESTDEF *Best   Parameters of the best excitation model
**  int16_t Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**
** Outputs:
**
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**
** Return value:    None
**
*/
void Fcbk_Pack(int16_t * Dpnt, SFSDEF * Sfs, BESTDEF * Best, int16_t Np)
{
	int i, j;

	/* Code the amplitudes and positions */
	j = MaxPulseNum - (int)Np;

	(*Sfs).Pamp = (int16_t) 0;
	(*Sfs).Ppos = (int32_t) 0;

	for (i = 0; i < SubFrLen / Sgrid; i++) {

		if (Dpnt[(int)(*Best).GridId + Sgrid * i] == (int16_t) 0)
			(*Sfs).Ppos =
			    L_g723_add((*Sfs).Ppos, CombinatorialTable[j][i]);
		else {
			(*Sfs).Pamp = g723_shl((*Sfs).Pamp, (int16_t) 1);
			if (Dpnt[(int)(*Best).GridId + Sgrid * i] < (int16_t) 0)
				(*Sfs).Pamp = g723_add((*Sfs).Pamp,
						       (int16_t) 1);

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
**  int16_t *Tv      Decoded excitation vector
**  SFSDEF Sfs      Encoded parameters of the excitation (for one subframe)
**  int16_t Olp      Closed loop adaptive pitch lag
**  int16_t Sfc      Subframe index
**
** Outputs:
**
**  int16_t *Tv      Decoded excitation vector
**
** Return value:    None
**
*/
void Fcbk_Unpk(int16_t * Tv, SFSDEF Sfs, int16_t Olp, int16_t Sfc)
{
	int i, j;

	int32_t Acc0;
	int16_t Np;
	int16_t Tv_tmp[SubFrLen + 4];
	int16_t acelp_gain, acelp_sign, acelp_shift, acelp_pos;
	int16_t offset, ipos, T0_acelp, gain_T0;

	switch (WrkRate) {
	case Rate63:{

			Np = Nb_puls[(int)Sfc];

			for (i = 0; i < SubFrLen; i++)
				Tv[i] = (int16_t) 0;

			if (Sfs.Ppos >= MaxPosTable[Sfc])
				return;

			/* Decode the amplitudes and positions */
			j = MaxPulseNum - (int)Np;

			Acc0 = Sfs.Ppos;

			for (i = 0; i < SubFrLen / Sgrid; i++) {

				Acc0 =
				    L_g723_sub(Acc0, CombinatorialTable[j][i]);

				if (Acc0 < (int32_t) 0) {
					Acc0 =
					    L_g723_add(Acc0,
						       CombinatorialTable[j]
						       [i]);
					j++;
					if ((Sfs.
					     Pamp & (1 << (MaxPulseNum - j))) !=
					    (int16_t) 0)
						Tv[(int)Sfs.Grid + Sgrid * i] =
						    -FcbkGainTable[Sfs.Mamp];
					else
						Tv[(int)Sfs.Grid + Sgrid * i] =
						    FcbkGainTable[Sfs.Mamp];

					if (j == MaxPulseNum)
						break;
				}
			}

			if (Sfs.Tran == (int16_t) 1)
				Gen_Trn(Tv, Tv, Olp);
			break;
		}

	case Rate53:{
			for (i = 0; i < SubFrLen + 4; i++)
				Tv_tmp[i] = (int16_t) 0;

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
				ipos = (acelp_pos & (int16_t) 0x0007);
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
			    search_T0((int16_t) (Olp - 1 + Sfs.AcLg),
				      Sfs.AcGn,
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
**  int16_t *Tv      Target vector
**  int16_t *ImpResp Impulse response of the combined filter
**  int16_t *PrevExc Previous excitation vector
**  LINEDEF *Line   Contains pitch related parameters (open/closed loop lag, gain)
**  int16_t Sfc      Subframe index
**
** Outputs:
**
**  int16_t *Tv     Residual vector
**  LINEDEF *Line  Contains pitch related parameters (closed loop lag, gain)
**
** Return value:    None
**
*/
void Find_Acbk(int16_t * Tv, int16_t * ImpResp, int16_t * PrevExc, LINEDEF
	       * Line, int16_t Sfc)
{
	int i, j, k, l;

	int32_t Acc0, Acc1;

	int16_t RezBuf[SubFrLen + ClPitchOrd - 1];
	int16_t FltBuf[ClPitchOrd][SubFrLen];
	int32_t CorBuf[4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2)];
	int32_t *lPnt;

	int16_t CorVct[4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2)];
	int16_t *sPnt;

	int16_t Olp;
	int16_t Lid;
	int16_t Gid;
	int16_t Hb;
	int16_t Exp;
	int16_t Bound[2];

	int16_t Lag1, Lag2;
	int16_t off_filt;

	memzero(CorBuf,
		4 * (2 * ClPitchOrd + ClPitchOrd * (ClPitchOrd - 1) / 2) * sizeof(int32_t));

	/* Init constants */
	Olp = (*Line).Olp[g723_shr(Sfc, (int16_t) 1)];
	Lid = (int16_t) Pstep;
	Gid = (int16_t) 0;
	Hb = (int16_t) 3 + (Sfc & (int16_t) 1);

	/* For even frames only */
	if ((Sfc & (int16_t) 1) == (int16_t) 0) {
		if (Olp == (int16_t) PitchMin)
			Olp = g723_add(Olp, (int16_t) 1);
		if (Olp > (int16_t) (PitchMax - 5))
			Olp = (int16_t) (PitchMax - 5);
	}

	lPnt = CorBuf;
	for (k = 0; k < (int)Hb; k++) {

		/* Get residual from the excitation buffer */
		Get_Rez(RezBuf, PrevExc,
			(int16_t) (Olp - (int16_t) Pstep + k));

		/* Filter the last one using the impulse response */
		for (i = 0; i < SubFrLen; i++) {
			Acc0 = (int32_t) 0;
			for (j = 0; j <= i; j++)
				Acc0 =
				    g723_L_mac(Acc0, RezBuf[ClPitchOrd - 1 + j],
					       ImpResp[i - j]);
			FltBuf[ClPitchOrd - 1][i] = round_(Acc0);
		}

		/* Update all the others */
		for (i = ClPitchOrd - 2; i >= 0; i--) {
			FltBuf[i][0] = g723_mult_r(RezBuf[i],
						   (int16_t) 0x2000);
			for (j = 1; j < SubFrLen; j++) {
				Acc0 = g723_L_deposit_h(FltBuf[i + 1][j - 1]);
				Acc0 = g723_L_mac(Acc0, RezBuf[i], ImpResp[j]);
				FltBuf[i][j] = round_(Acc0);
			}
		}

		/* Compute the cross with the signal */
		for (i = 0; i < ClPitchOrd; i++) {
			Acc1 = (int32_t) 0;
			for (j = 0; j < SubFrLen; j++) {
				Acc0 = L_g723_mult(Tv[j], FltBuf[i][j]);
				Acc1 =
				    L_g723_add(Acc1,
					       L_g723_shr(Acc0, (int16_t) 1));
			}
			*lPnt++ = L_g723_shl(Acc1, (int16_t) 1);
		}

		/* Compute the energies */
		for (i = 0; i < ClPitchOrd; i++) {
			Acc1 = (int32_t) 0;
			for (j = 0; j < SubFrLen; j++)
				Acc1 =
				    g723_L_mac(Acc1, FltBuf[i][j],
					       FltBuf[i][j]);
			*lPnt++ = Acc1;
		}

		/* Compute the between crosses */
		for (i = 1; i < ClPitchOrd; i++) {
			for (j = 0; j < i; j++) {
				Acc1 = (int32_t) 0;
				for (l = 0; l < SubFrLen; l++) {
					Acc0 =
					    L_g723_mult(FltBuf[i][l],
							FltBuf[j][l]);
					Acc1 =
					    L_g723_add(Acc1,
						       L_g723_shr(Acc0,
								  (int16_t) 1));
				}
				*lPnt++ = L_g723_shl(Acc1, (int16_t) 2);
			}
		}
	}

	/* Find Max and normalize */
	Acc1 = (int32_t) 0;
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
	Lag1 = Olp - (int16_t) Pstep;
	Lag2 = Olp - (int16_t) Pstep + Hb - (int16_t) 1;
	off_filt = Test_Err(Lag1, Lag2);
	Bound[0] = NbFilt085_min + g723_shl(off_filt, 2);
	if (Bound[0] > NbFilt085)
		Bound[0] = NbFilt085;
	Bound[1] = NbFilt170_min + g723_shl(off_filt, 3);
	if (Bound[1] > NbFilt170)
		Bound[1] = NbFilt170;

	/* Init the search loop */
	Acc1 = (int32_t) 0;

	for (k = 0; k < (int)Hb; k++) {

		/* Select Quantization tables */
		l = 0;
		if (WrkRate == Rate63) {
			if ((Sfc & (int16_t) 1) == (int16_t) 0) {
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

			Acc0 = (int32_t) 0;
			for (j = 0; j < 20; j++)
				Acc0 =
				    L_g723_add(Acc0,
					       L_g723_shr(L_g723_mult
							  (CorVct[k * 20 + j],
							   *sPnt++),
							  (int16_t) 1));

			if (Acc0 > Acc1) {
				Acc1 = Acc0;
				Gid = (int16_t) i;
				Lid = (int16_t) k;
			}
		}
	}

	/* Modify Olp for even sub frames */
	if ((Sfc & (int16_t) 1) == (int16_t) 0) {
		Olp = Olp - (int16_t) Pstep + Lid;
		Lid = (int16_t) Pstep;
	}

	/* Save Gains and Olp */
	(*Line).Sfs[Sfc].AcLg = Lid;
	(*Line).Sfs[Sfc].AcGn = Gid;
	(*Line).Olp[g723_shr(Sfc, (int16_t) 1)] = Olp;

	/* Decode the Acbk contribution and subtract it */
	Decod_Acbk(RezBuf, PrevExc, Olp, Lid, Gid);

	for (i = 0; i < SubFrLen; i++) {
		Acc0 = g723_L_deposit_h(Tv[i]);
		Acc0 = L_g723_shr(Acc0, (int16_t) 1);

		for (j = 0; j <= i; j++)
			Acc0 = g723_L_msu(Acc0, RezBuf[j], ImpResp[i - j]);
		Acc0 = L_g723_shl(Acc0, (int16_t) 1);
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
**  int16_t *Tv      delayed excitation
**  int16_t *PrevExc Previous excitation vector
**  int16_t Lag      Closed loop pitch lag
**
** Outputs:
**
**  int16_t *Tv      delayed excitation
**
** Return value:    None
**
*/
void Get_Rez(int16_t * Tv, int16_t * PrevExc, int16_t Lag)
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
**  int16_t *Tv      Reconstructed excitation vector
**  int16_t *PrevExc Previous excitation vector
**  int16_t Olp      closed-loop pitch period
**  int16_t Lid      Jitter around pitch period
**  int16_t Gid      Gain vector index in 5- dimensional
**                      adaptive gain vector codebook
**
** Outputs:
**
**  int16_t *Tv      Reconstructed excitation vector
**
** Return value:    None
**
*/
void Decod_Acbk(int16_t * Tv, int16_t * PrevExc, int16_t Olp, int16_t Lid,
		int16_t Gid)
{
	int i, j;

	int32_t Acc0;
	int16_t RezBuf[SubFrLen + ClPitchOrd - 1];
	int16_t *sPnt;

	Get_Rez(RezBuf, PrevExc, (int16_t) (Olp - (int16_t) Pstep + Lid));

	/* Select Quantization tables */
	i = 0;
	if (WrkRate == Rate63) {
		if (Olp >= (int16_t) (SubFrLen - 2))
			i++;
	} else {
		i = 1;
	}
	sPnt = AcbkGainTablePtr[i];

	sPnt += (int)Gid *20;

	for (i = 0; i < SubFrLen; i++) {
		Acc0 = (int32_t) 0;
		for (j = 0; j < ClPitchOrd; j++)
			Acc0 = g723_L_mac(Acc0, RezBuf[i + j], sPnt[j]);
		Acc0 = L_g723_shl(Acc0, (int16_t) 1);
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
**  int16_t *Buff  decoded excitation
**  int16_t Olp    Decoded pitch lag
**
** Outputs: None
**
** Return value:
**
**      int16_t   Estimated pitch value
*/
int16_t Comp_Info(int16_t * Buff, int16_t Olp, int16_t * Gain,
		  int16_t * ShGain)
{
	int i, j;

	int32_t Acc0, Acc1;

	int16_t Tenr;
	int16_t Ccr, Enr;
	int16_t Indx;

	/* Normalize the excitation */
	*ShGain = Vec_Norm(Buff, (int16_t) (PitchMax + Frame));

	if (Olp > (int16_t) (PitchMax - 3))
		Olp = (int16_t) (PitchMax - 3);

	Indx = Olp;

	Acc1 = (int32_t) 0;

	for (i = (int)Olp - 3; i <= (int)Olp + 3; i++) {

		Acc0 = (int32_t) 0;
		for (j = 0; j < 2 * SubFrLen; j++)
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + Frame - 2 * SubFrLen +
					    j],
				       Buff[PitchMax + Frame - 2 * SubFrLen -
					    i + j]);

		if (Acc0 > Acc1) {
			Acc1 = Acc0;
			Indx = (int16_t) i;
		}
	}

	/* Compute target energy */
	Acc0 = (int32_t) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0, Buff[PitchMax + Frame - 2 * SubFrLen + j],
			       Buff[PitchMax + Frame - 2 * SubFrLen + j]);
	Tenr = round_(Acc0);
	*Gain = Tenr;

	/* Compute best energy */
	Acc0 = (int32_t) 0;
	for (j = 0; j < 2 * SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0,
			       Buff[PitchMax + Frame - 2 * SubFrLen -
				    (int)Indx + j],
			       Buff[PitchMax + Frame - 2 * SubFrLen -
				    (int)Indx + j]);

	Ccr = round_(Acc1);

	if (Ccr <= (int16_t) 0)
		return (int16_t) 0;

	Enr = round_(Acc0);

	Acc0 = L_g723_mult(Enr, Tenr);
	Acc0 = L_g723_shr(Acc0, (int16_t) 3);

	Acc0 = g723_L_msu(Acc0, Ccr, Ccr);

	if (Acc0 < (int32_t) 0)
		return Indx;
	else
		return (int16_t) 0;
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
**  int16_t *DataBuff  current subframe decoded excitation
**  int16_t *Buff     past decoded excitation
**  int16_t Lag       Decoded pitch lag from previous frame
**  int16_t Gain      Interpolated gain from previous frames
**  int16_t Ecount    Number of erased frames
**  int16_t *Sd       Random number used in unvoiced cases
**
** Outputs:
**
**  int16_t *DataBuff current subframe decoded excitation
**  int16_t *Buff     updated past excitation
**
** Return value:    None
**
*/
void Regen(int16_t * DataBuff, int16_t * Buff, int16_t Lag, int16_t Gain,
	   int16_t Ecount, int16_t * Sd)
{
	int i;

	/* Test for clearing */
	if (Ecount >= (int16_t) ErrMaxNum) {
		for (i = 0; i < Frame; i++)
			DataBuff[i] = (int16_t) 0;
		for (i = 0; i < Frame + PitchMax; i++)
			Buff[i] = (int16_t) 0;
	} else {
		/* Interpolate accordingly to the voicing estimation */
		if (Lag != (int16_t) 0) {
			/* Voiced case */
			for (i = 0; i < Frame; i++)
				Buff[PitchMax + i] =
				    Buff[PitchMax - (int)Lag + i];
			for (i = 0; i < Frame; i++)
				DataBuff[i] = Buff[PitchMax + i] =
				    g723_mult(Buff[PitchMax + i],
					      (int16_t) 0x6000);
		} else {
			/* Unvoiced case */
			for (i = 0; i < Frame; i++)
				DataBuff[i] = g723_mult(Gain, Rand_lbc(Sd));
			/* Clear buffer to reset memory */
			for (i = 0; i < Frame + PitchMax; i++)
				Buff[i] = (int16_t) 0;
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
**  int16_t *Buff    decoded excitation
**  int16_t Olp      Decoded pitch lag
**  int16_t Sfc      Subframe index
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
PFDEF Comp_Lpf(int16_t * Buff, int16_t Olp, int16_t Sfc)
{
	int i, j;

	PFDEF Pf;
	int32_t Lcr[5];
	int16_t Scr[5];
	int16_t Bindx, Findx;
	int16_t Exp;

	int32_t Acc0, Acc1;

	/* Initialize */
	Pf.Indx = (int16_t) 0;
	Pf.Gain = (int16_t) 0;
	Pf.ScGn = (int16_t) 0x7fff;

	/* Find both indices */
	Bindx = Find_B(Buff, Olp, Sfc);
	Findx = Find_F(Buff, Olp, Sfc);

	/* Combine the results */
	if ((Bindx == (int16_t) 0) && (Findx == (int16_t) 0))
		return Pf;

	/* Compute target energy */
	Acc0 = (int32_t) 0;
	for (j = 0; j < SubFrLen; j++)
		Acc0 =
		    g723_L_mac(Acc0, Buff[PitchMax + (int)Sfc * SubFrLen + j],
			       Buff[PitchMax + (int)Sfc * SubFrLen + j]);
	Lcr[0] = Acc0;

	if (Bindx != (int16_t) 0) {
		Acc0 = (int32_t) 0;
		Acc1 = (int32_t) 0;
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
		Lcr[1] = (int32_t) 0;
		Lcr[2] = (int32_t) 0;
	}

	if (Findx != (int16_t) 0) {
		Acc0 = (int32_t) 0;
		Acc1 = (int32_t) 0;
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
		Lcr[3] = (int32_t) 0;
		Lcr[4] = (int32_t) 0;
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
	if ((Bindx != (int16_t) 0) && (Findx == (int16_t) 0))
		Pf = Get_Ind(Bindx, Scr[0], Scr[1], Scr[2]);

	if ((Bindx == (int16_t) 0) && (Findx != (int16_t) 0))
		Pf = Get_Ind(Findx, Scr[0], Scr[3], Scr[4]);

	if ((Bindx != (int16_t) 0) && (Findx != (int16_t) 0)) {
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
**  int16_t *Buff    decoded excitation
**  int16_t Olp      Decoded pitch lag
**  int16_t Sfc      Subframe index
**
** Outputs:     None
**
** Return value:
**
**  int16_t   Pitch postfilter backward lag
*/
int16_t Find_B(int16_t * Buff, int16_t Olp, int16_t Sfc)
{
	int i, j;

	int16_t Indx = 0;

	int32_t Acc0, Acc1;

	if (Olp > (int16_t) (PitchMax - 3))
		Olp = (int16_t) (PitchMax - 3);

	Acc1 = (int32_t) 0;

	for (i = (int)Olp - 3; i <= (int)Olp + 3; i++) {

		Acc0 = (int32_t) 0;
		for (j = 0; j < SubFrLen; j++)
			Acc0 =
			    g723_L_mac(Acc0,
				       Buff[PitchMax + (int)Sfc * SubFrLen + j],
				       Buff[PitchMax + (int)Sfc * SubFrLen - i +
					    j]);
		if (Acc0 > Acc1) {
			Acc1 = Acc0;
			Indx = -(int16_t) i;
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
**  int16_t *Buff    decoded excitation
**  int16_t Olp      Decoded pitch lag
**  int16_t Sfc      Subframe index
**
** Outputs:     None
**
** Return value:
**
**  int16_t    Pitch postfilter forward lag
*/
int16_t Find_F(int16_t * Buff, int16_t Olp, int16_t Sfc)
{
	int i, j;

	int16_t Indx = 0;

	int32_t Acc0, Acc1;

	if (Olp > (int16_t) (PitchMax - 3))
		Olp = (int16_t) (PitchMax - 3);

	Acc1 = (int32_t) 0;

	for (i = Olp - 3; i <= Olp + 3; i++) {

		Acc0 = (int32_t) 0;
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
			Indx = (int16_t) i;
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
**  int16_t Ind      Pitch postfilter lag
**  int16_t Ten      energy of the current subframe excitation vector
**  int16_t Ccr      Crosscorrelation of the excitation
**  int16_t Enr      Energy of the (backward or forward) "delayed" excitation
**
** Outputs:     None
**
** Return value:
**
**  PFDEF
**         int16_t   Indx    Pitch postfilter lag
**         int16_t   Gain    Pitch postfilter gain
**         int16_t   ScGn    Pitch postfilter scaling gain
**
*/
PFDEF Get_Ind(int16_t Ind, int16_t Ten, int16_t Ccr, int16_t Enr)
{
	int32_t Acc0, Acc1;
	int16_t Exp;

	PFDEF Pf;

	Pf.Indx = Ind;

	/* Check valid gain */
	Acc0 = L_g723_mult(Ten, Enr);
	Acc0 = L_g723_shr(Acc0, (int16_t) 2);
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
		Acc0 = L_g723_shr(Acc0, (int16_t) 1);
		Acc0 = g723_L_mac(Acc0, Ccr, Pf.Gain);
		Exp = g723_mult(Pf.Gain, Pf.Gain);
		Acc1 = L_g723_mult(Enr, Exp);
		Acc1 = L_g723_shr(Acc1, (int16_t) 1);
		Acc0 = L_g723_add(Acc0, Acc1);
		Exp = round_(Acc0);

		Acc1 = g723_L_deposit_h(Ten);
		Acc0 = g723_L_deposit_h(Exp);
		Acc1 = L_g723_shr(Acc1, (int16_t) 1);

		if (Acc1 >= Acc0)
			Exp = (int16_t) 0x7fff;
		else
			Exp = div_l(Acc1, Exp);

		Acc0 = g723_L_deposit_h(Exp);
		Pf.ScGn = Sqrt_lbc(Acc0);
	} else {
		Pf.Gain = (int16_t) 0;
		Pf.ScGn = (int16_t) 0x7fff;
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
**  int16_t *Tv      Pitch postfiltered excitation
**  int16_t *Buff    decoded excitation
**  PFDEF Pf        Pitch postfilter parameters
**  int16_t Sfc      Subframe index
**
** Outputs:
**
**  int16_t *Tv      Pitch postfiltered excitation
**
** Return value: None
**
*/
void Filt_Lpf(int16_t * Tv, int16_t * Buff, PFDEF Pf, int16_t Sfc)
{
	int i;

	int32_t Acc0;

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
**   int16_t X[]              Target vector.     (in Q0)
**   int16_t h[]              Impulse response.  (in Q12)
**   int16_t T0               Pitch period.
**   int16_t code[]           Innovative vector.        (in Q12)
**   int16_t gain             Innovative vector gain.   (in Q0)
**   int16_t sign             Signs of the 4 pulses.
**   int16_t shift            Shift of the innovative vector
**   int16_t gain_T0          Gain for pitch synchronous fiter
**
** Inputs :
**
**   int16_t X[]              Target vector.     (in Q0)
**   int16_t h[]              Impulse response.  (in Q12)
**   int16_t T0               Pitch period.
**   int16_t gain_T0          Gain for pitch synchronous fiter
**
** Outputs:
**
**   int16_t code[]           Innovative vector.        (in Q12)
**   int16_t gain             Innovative vector gain.   (in Q0)
**   int16_t sign             Signs of the 4 pulses.
**   int16_t shift            Shift of the innovative vector.
**
** Return value:
**
**   int16_t index            Innovative codebook index
**
*/
int16_t ACELP_LBC_code(int16_t X[], int16_t h[], int16_t T0, int16_t code[],
		      int16_t * ind_gain, int16_t * shift, int16_t * sign,
		      int16_t gain_T0)
{
	int16_t i, index, gain_q;
	int16_t Dn[SubFrLen2], tmp_code[SubFrLen2];
	int16_t rr[DIM_RR];

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
**  int16_t h[]              Impulse response.
**  int16_t rr[]             Correlations.
**
**  Outputs:
**
**  int16_t rr[]             Correlations.
**
**  Return value :          None
*/
void Cor_h(int16_t * H, int16_t * rr)
{
	int16_t *rri0i0, *rri1i1, *rri2i2, *rri3i3;
	int16_t *rri0i1, *rri0i2, *rri0i3;
	int16_t *rri1i2, *rri1i3, *rri2i3;

	int16_t *p0, *p1, *p2, *p3;

	int16_t *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
	int32_t cor;
	int16_t i, k, ldec, l_fin_sup, l_fin_inf;
	int16_t h[SubFrLen2];

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
	l_fin_inf = l_fin_sup - (int16_t) 1;
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

		for (i = k + (int16_t) 1; i < NB_POS; i++) {

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
	l_fin_inf = l_fin_sup - (int16_t) 1;
	for (k = 0; k < NB_POS; k++) {
		p3 = rri1i3 + l_fin_sup;
		p2 = rri0i2 + l_fin_sup;
		p1 = rri1i3 + l_fin_inf;
		p0 = rri0i2 + l_fin_inf;

		cor = 0;
		ptr_h1 = ptr_hd;
		ptr_h2 = ptr_hf;
		for (i = k + (int16_t) 1; i < NB_POS; i++) {
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
	l_fin_inf = l_fin_sup - (int16_t) 1;
	for (k = 0; k < NB_POS; k++) {

		p3 = rri0i3 + l_fin_sup;
		p2 = rri2i3 + l_fin_inf;
		p1 = rri1i2 + l_fin_inf;
		p0 = rri0i1 + l_fin_inf;

		ptr_h1 = ptr_hd;
		ptr_h2 = ptr_hf;
		cor = 0;
		for (i = k + (int16_t) 1; i < NB_POS; i++) {

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
**      int16_t h[]              Impulse response.
**      int16_t X[]              Target vector.
**      int16_t D[]              Correlations.
**
**  Outputs:
**
**      int16_t D[]              Correlations.
**
**  Return value:           None
*/
void Cor_h_X(int16_t h[], int16_t X[], int16_t D[])
{
	int16_t i, j;
	int32_t s, max;
	int32_t y32[SubFrLen];

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
**      int16_t          extra
**
**  Return value:           None
**
*/
static int16_t extra;
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
**      int16_t Dn[]       Correlation between target vector and impulse response h[]
**      int16_t rr[]       Correlations of impulse response h[]
**      int16_t h[]        Impulse response of filters
**
**  Output arguments:
**
**      int16_t cod[]      Selected algebraic codeword
**      int16_t y[]        Filtered codeword
**      int16_t code_shift Shift of the codeword
**      int16_t sign       Signs of the 4 pulses.
**
**  Return value:
**
**      int16_t   Index of selected codevector
**
*/
int16_t D4i64_LBC(int16_t Dn[], int16_t rr[], int16_t h[], int16_t cod[],
		 int16_t y[], int16_t * code_shift, int16_t * sign)
{
	int16_t i0, i1, i2, i3, ip0, ip1, ip2, ip3;
	int16_t i, j, time;
	int16_t shif, shift;
	int16_t ps0, ps1, ps2, ps3, alp, alp0;
	int32_t alp1, alp2, alp3, L32;
	int16_t ps0a, ps1a, ps2a;
	int16_t ps3c, psc, alpha;
	int16_t means, max0, max1, max2, thres;

	int16_t *rri0i0, *rri1i1, *rri2i2, *rri3i3;
	int16_t *rri0i1, *rri0i2, *rri0i3;
	int16_t *rri1i2, *rri1i3, *rri2i3;

	int16_t *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3;
	int16_t *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3;
	int16_t *ptr_ri1i2, *ptr_ri1i3, *ptr_ri2i3;

	int16_t *ptr1_ri0i1, *ptr1_ri0i2, *ptr1_ri0i3;
	int16_t *ptr1_ri1i2, *ptr1_ri1i3, *ptr1_ri2i3;

	int16_t p_sign[SubFrLen2 / 2];

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
**      int16_t X[]        Code target.  (in Q0)
**      int16_t Y[]        Filtered innovation code. (in Q12)
**
** Output:
**
**      int16_t *gain_q    Gain of innovation code.  (in Q0)
**
**  Return value:
**
**      int16_t  index of innovation code gain
**
*/
int16_t G_code(int16_t X[], int16_t Y[], int16_t * gain_q)
{
	int16_t i;
	int16_t xy, yy, exp_xy, exp_yy, gain, gain_nq;
	int32_t L_xy, L_yy;
	int16_t dist, dist_min;

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

	gain = (int16_t) 0;
	dist_min = g723_sub(gain_nq, FcbkGainTable[0]);
	dist_min = g723_abs_s(dist_min);
	for (i = 1; i < NumOfGainLev; i++) {
		dist = g723_sub(gain_nq, FcbkGainTable[i]);
		dist = g723_abs_s(dist);
		if (dist < dist_min) {
			dist_min = dist;
			gain = (int16_t) i;
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
**      int16_t T0         Decoded pitch lag
**      int16_t Gid        Gain vector index in the adaptive gain vector codebook
**      int16_t *gain_T0   Pitch synchronous gain
**
**  Outputs:
**
**      int16_t *gain_T0   Pitch synchronous filter gain
**
**  Return Value:
**
**      int16_t T0_mod     Pitch synchronous filter lag
*/
int16_t search_T0(int16_t T0, int16_t Gid, int16_t * gain_T0)
{

	int16_t T0_mod;

	T0_mod = T0 + epsi170[Gid];
	*gain_T0 = gain170[Gid];

	return (T0_mod);
}
