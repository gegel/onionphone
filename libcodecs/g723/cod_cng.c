/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "g723_const.h"

extern int16_t Durbin(int16_t * Lpc, int16_t * Corr, int16_t Err,
		      int16_t * Pk2);
extern int16_t Qua_SidGain(int16_t * Ener, int16_t * shEner, int16_t nq);
extern int16_t g723_abs_s(int16_t var1);	/* Short abs,           1 */
extern int16_t g723_sub(int16_t var1, int16_t var2);	/* Short sub,           1 */

extern VADSTATDEF VadStat;
extern CODSTATDEF CodStat;

extern int16_t g723_shr(int16_t var1, int16_t var2);	/* Short shift right,   1 */

#define NbPulsBlk          11	/* Nb of pulses in 2-subframes blocks         */

extern void AtoLsp(int16_t * LspVect, int16_t * Lpc, int16_t * PrevLsp);
extern int32_t Lsp_Qnt(int16_t * CurrLsp, int16_t * PrevLsp);
extern void Lsp_Inq(int16_t * Lsp, int16_t * PrevLsp, int32_t LspId,
		    int16_t Crc);
extern int16_t Dec_SidGain(int16_t i_gain);
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */
extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern void Calc_Exc_Rand(int16_t cur_gain, int16_t * PrevExc,
			  int16_t * DataExc,
			  int16_t * nRandom, LINEDEF * Line);
extern void Lsp_Int(int16_t * QntLpc, int16_t * CurrLsp, int16_t * PrevLsp);
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
extern int32_t g723_L_deposit_l(int16_t var1);	/* 16 bit var1 -> LSB,     2 */
extern int32_t L_g723_shl(int32_t L_var1, int16_t var2);	/* Long shift left,     2 */
extern int16_t g723_norm_l(int32_t L_var1);	/* Long norm,            30 */
extern int16_t g723_extract_l(int32_t L_var1);	/* Extract low,         1 */
extern int32_t L_g723_shr(int32_t L_var1, int16_t var2);	/* Long shift right,    2 */
extern int32_t g723_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,    1 */
extern int16_t round_(int32_t L_var1);	/* Round,               1 */
extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */

/* Declaration of local functions */
static void ComputePastAvFilter(int16_t * Coeff);
static void CalcRC(int16_t * Coeff, int16_t * RC, int16_t * shRC);
static int LpcDiff(int16_t * RC, int16_t shRC, int16_t * Acf,
		    int16_t alpha);

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

	for (i = 0; i < SizAcf; i++)
		CodCng.Acf[i] = 0;

	for (i = 0; i <= NbAvAcf; i++)
		CodCng.ShAcf[i] = 40;

	for (i = 0; i < LpcOrder; i++)
		CodCng.SidLpc[i] = 0;

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
**  int16_t *DataExc    Current frame synthetic excitation
**  int16_t *Ftyp     Characterizes the frame type for CNG
**  LINEDEF *Line      Quantized parameters (used for SID frames)
**  int16_t *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  int16_t *DataExc
**  int16_t *Ftyp
**  LINEDEF *Line
**  int16_t *QntLpc
**
** Return value:       None
**
*/
void Cod_Cng(int16_t * DataExc, int16_t * Ftyp, LINEDEF * Line,
	     int16_t * QntLpc)
{

	int16_t curCoeff[LpcOrder];
	int16_t curQGain;
	int16_t temp;
	int i;

	/*
	 * Update Ener
	 */
	for (i = NbAvGain - 1; i >= 1; i--) {
		CodCng.Ener[i] = CodCng.Ener[i - 1];
	}

	/*
	 * Compute LPC filter of present frame
	 */
	CodCng.Ener[0] = Durbin(curCoeff, &CodCng.Acf[1], CodCng.Acf[0], &temp);

	/*
	 * if first frame of silence => SID frame
	 */
	if (CodCng.PastFtyp == 1) {
		*Ftyp = 2;
		CodCng.NbEner = 1;
		curQGain =
		    Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);
	}

	else {
		CodCng.NbEner++;
		if (CodCng.NbEner > NbAvGain)
			CodCng.NbEner = NbAvGain;
		curQGain =
		    Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);

		/*
		 * Compute stationarity of current filter
		 * versus reference filter
		 */
		if (LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *CodCng.Ener) ==
		    0) {
			/* transmit SID frame */
			*Ftyp = 2;
		} else {
			temp = g723_abs_s(g723_sub(curQGain, CodCng.IRef));
			if (temp > ThreshGain) {
				*Ftyp = 2;
			} else {
				/* no transmission */
				*Ftyp = 0;
			}
		}
	}

	/*
	 * If SID frame : Compute SID filter
	 */
	if (*Ftyp == 2) {

		/*
		 * Evaluates local stationnarity :
		 * Computes difference between current filter and past average filter
		 * if signal not locally stationary SID filter = current filter
		 * else SID filter = past average filter
		 */
		/* Compute past average filter */
		ComputePastAvFilter(CodCng.SidLpc);

		/* If adaptation enabled, fill noise filter */
		if (!VadStat.Aen) {
			for (i = 0; i < LpcOrder; i++)
				VadStat.NLpc[i] = CodCng.SidLpc[i];
		}

		/* Compute autocorr. of past average filter coefficients */
		CalcRC(CodCng.SidLpc, CodCng.RC, &CodCng.ShRC);

		if (LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *CodCng.Ener) ==
		    0) {
			for (i = 0; i < LpcOrder; i++) {
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

	}

	/* end of Ftyp=2 case (SID frame) */
	/*
	 * Compute new excitation
	 */
	if (CodCng.PastFtyp == 1) {
		CodCng.CurGain = CodCng.SidGain;
	} else {
		CodCng.CurGain =
		    g723_extract_h(L_g723_add
				   (L_g723_mult(CodCng.CurGain, 0x7000),
				    L_g723_mult(CodCng.SidGain, 0x1000)));
	}
	Calc_Exc_Rand(CodCng.CurGain, CodStat.PrevExc, DataExc,
		      &CodCng.RandSeed, Line);

	/*
	 * Interpolate LSPs and update PrevLsp
	 */
	Lsp_Int(QntLpc, CodCng.LspSid, CodStat.PrevLsp);
	for (i = 0; i < LpcOrder; i++) {
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
**  int16_t *Acf_sf     sets of subframes Acfs of current frame
**  int16_t *ShAcf_sf   corresponding scaling factors
**
** Output :            None
**
** Return value:       None
**
*/
void Update_Acf(int16_t * Acf_sf, int16_t * ShAcf_sf)
{

	int i, i_subfr;
	int16_t *ptr1, *ptr2;
	int32_t L_temp[LpcOrderP1];
	int16_t sh1, temp;
	int32_t L_acc0;

	/* Update Acf and ShAcf */
	ptr2 = CodCng.Acf + SizAcf;
	ptr1 = ptr2 - LpcOrderP1;
	for (i = LpcOrderP1; i < SizAcf; i++)
		*(--ptr2) = *(--ptr1);
	for (i = NbAvAcf; i >= 1; i--)
		CodCng.ShAcf[i] = CodCng.ShAcf[i - 1];

	/* Search ShAcf_sf min for current frame */
	sh1 = ShAcf_sf[0];
	for (i_subfr = 1; i_subfr < SubFrames; i_subfr++) {
		if (ShAcf_sf[i_subfr] < sh1)
			sh1 = ShAcf_sf[i_subfr];
	}
	sh1 = g723_add(sh1, 14);	/* 2 bits of margin */

	/* Compute current sum of acfs */
	for (i = 0; i <= LpcOrder; i++)
		L_temp[i] = 0;

	ptr2 = Acf_sf;
	for (i_subfr = 0; i_subfr < SubFrames; i_subfr++) {
		temp = g723_sub(sh1, ShAcf_sf[i_subfr]);
		for (i = 0; i <= LpcOrder; i++) {
			L_acc0 = g723_L_deposit_l(*ptr2++);
			L_acc0 = L_g723_shl(L_acc0, temp);	/* shift right if temp<0 */
			L_temp[i] = L_g723_add(L_temp[i], L_acc0);
		}
	}
	/* Normalize */
	temp = g723_norm_l(L_temp[0]);
	temp = g723_sub(16, temp);
	if (temp < 0)
		temp = 0;
	for (i = 0; i <= LpcOrder; i++) {
		CodCng.Acf[i] = g723_extract_l(L_g723_shr(L_temp[i], temp));
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
**  int16_t *Coeff      set of LPC coefficients
**
** Output:
**
**  int16_t *Coeff
**
** Return value:       None
**
*/
void ComputePastAvFilter(int16_t * Coeff)
{
	int i, j;
	int16_t *ptr_Acf;
	int32_t L_sumAcf[LpcOrderP1];
	int16_t Corr[LpcOrder], Err;
	int16_t sh1, temp;
	int32_t L_acc0;

	/* Search ShAcf min */
	sh1 = CodCng.ShAcf[1];
	for (i = 2; i <= NbAvAcf; i++) {
		temp = CodCng.ShAcf[i];
		if (temp < sh1)
			sh1 = temp;
	}
	sh1 = g723_add(sh1, 14);	/* 2 bits of margin : NbAvAcf <= 4 */

	/* Compute sum of NbAvAcf frame-Acfs  */
	for (j = 0; j <= LpcOrder; j++)
		L_sumAcf[j] = 0;

	ptr_Acf = CodCng.Acf + LpcOrderP1;
	for (i = 1; i <= NbAvAcf; i++) {
		temp = g723_sub(sh1, CodCng.ShAcf[i]);
		for (j = 0; j <= LpcOrder; j++) {
			L_acc0 = g723_L_deposit_l(*ptr_Acf++);
			L_acc0 = L_g723_shl(L_acc0, temp);	/* shift right if temp<0 */
			L_sumAcf[j] = L_g723_add(L_sumAcf[j], L_acc0);
		}
	}

	/* Normalize */
	temp = g723_norm_l(L_sumAcf[0]);
	temp = g723_sub(16, temp);
	if (temp < 0)
		temp = 0;
	Err = g723_extract_l(L_g723_shr(L_sumAcf[0], temp));
	for (i = 1; i < LpcOrderP1; i++) {
		Corr[i - 1] = g723_extract_l(L_g723_shr(L_sumAcf[i], temp));
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
**  int16_t *Coeff      set of LPC coefficients
**  int16_t *RC         derived from LPC coefficients autocorrelation
**  int16_t *ShRC       corresponding scaling factor
**
** Outputs :
**
**  int16_t *RC
**  int16_t *ShRC
**
** Return value:       None
**
*/
void CalcRC(int16_t * Coeff, int16_t * RC, int16_t * ShRC)
{
	int i, j;
	int16_t sh1;
	int32_t L_acc;

	L_acc = 0L;
	for (j = 0; j < LpcOrder; j++) {
		L_acc = g723_L_mac(L_acc, Coeff[j], Coeff[j]);
	}
	L_acc = L_g723_shr(L_acc, 1);
	L_acc = L_g723_add(L_acc, 0x04000000L);	/* 1 << 2 * Lpc_justif. */
	sh1 = g723_norm_l(L_acc) - (int16_t) 2;	/* 1 bit because of x2 in RC[i], i> 0 */
	/* & 1 bit margin for Itakura distance */
	L_acc = L_g723_shl(L_acc, sh1);	/* shift right if < 0 */
	RC[0] = round_(L_acc);

	for (i = 1; i <= LpcOrder; i++) {
		L_acc = L_g723_mult((int16_t) 0xE000, Coeff[i - 1]);	/* - (1 << Lpc_justif.) */
		for (j = 0; j < LpcOrder - i; j++) {
			L_acc = g723_L_mac(L_acc, Coeff[j], Coeff[j + i]);
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
**  int16_t *RC         derived from LPC coefficients autocorrelation
**  int16_t ShRC        corresponding scaling factor
**  int16_t *ptrAcf     pointer on signal autocorrelation function
**  int16_t alpha       residual energy in LPC analysis using *ptrAcf
**
** Output:             None
**
** Return value:       flag = 1 if similar filters
**                     flag = 0 if different filters
**
*/
int LpcDiff(int16_t * RC, int16_t ShRC, int16_t * ptrAcf, int16_t alpha)
{
	int32_t L_temp0, L_temp1;
	int16_t temp;
	int i;
	int diff;

	L_temp0 = 0L;
	for (i = 0; i <= LpcOrder; i++) {
		temp = g723_shr(ptrAcf[i], 2);	/* + 2 margin bits */
		L_temp0 = g723_L_mac(L_temp0, RC[i], temp);
	}

	temp = g723_mult_r(alpha, FracThresh);
	L_temp1 = L_g723_add((int32_t) temp, (int32_t) alpha);
	temp = g723_add(ShRC, 9);	/* 9 = Lpc_justif. * 2 - 15 - 2 */
	L_temp1 = L_g723_shl(L_temp1, temp);

	if (L_temp0 <= L_temp1)
		diff = 1;	/* G723.1 maintenance April 2006 */
	/* Before : if(L_temp0 < L_temp1) diff = 1; */
	else
		diff = 0;
	return (diff);
}
