/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:        "util_cng.c"
**
** Description:     General Comfort Noise Generation functions
**
**
** Functions:       Calc_Exc_Rand() Computes random excitation
**                                  used both by coder & decoder
**                  Qua_SidGain()   Quantization of SID gain
**                                  used by coder
**                  Dec_SidGain()   Decoding of SID gain
**                                  used both by coder & decoder
**
** Local functions :
**                  distG()
**                  random_number()

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

extern int16_t base[3];
extern int16_t nfact[4];
extern int16_t Nb_puls[4];

extern int32_t L_bseg[3];

extern int16_t g723_shr(int16_t var1, int16_t var2);	/* Short shift right,   1 */
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
extern int16_t g723_shl(int16_t var1, int16_t var2);	/* Short shift left,    1 */
extern int16_t g723_sub(int16_t var1, int16_t var2);	/* Short sub,           1 */

extern void Decod_Acbk(int16_t * Tv, int16_t * PrevExc, int16_t Olp,
		       int16_t Lid,
		       int16_t Gid);

extern int16_t g723_abs_s(int16_t var1);	/* Short abs,           1 */
extern int16_t g723_norm_s(int16_t var1);	/* Short norm,           15 */
extern int32_t g723_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,    1 */
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int32_t L_g723_shl(int32_t L_var1, int16_t var2);	/* Long shift left,     2 */
extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern int32_t L_g723_shr(int32_t L_var1, int16_t var2);	/* Long shift right,    2 */
extern int16_t g723_extract_l(int32_t L_var1);	/* Extract low,         1 */
extern int32_t L_g723_sub(int32_t L_var1, int32_t L_var2);	/* Long sub,        2 */
extern int32_t L_mls(int32_t, int16_t);	/* Wght ?? */
extern int32_t g723_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,    1 */
extern int32_t L_g723_negate(int32_t L_var1);	/* Long negate,     2 */
extern int16_t g723_negate(int16_t var1);	/* Short negate,        1 */
extern int16_t Sqrt_lbc(int32_t Num);
extern int16_t g723_mult(int16_t var1, int16_t var2);	/* Short mult,          1 */
extern int16_t Rand_lbc(int16_t * p);
extern int32_t g723_L_deposit_l(int16_t var1);	/* 16 bit var1 -> LSB,     2 */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */
extern int16_t g723_mult_r(int16_t var1, int16_t var2);

/* Declaration of local functions */
static int16_t random_number(int16_t number_max_p1, int16_t * nRandom);

/*
**
** Function:           Calc_Exc_Rand()
**
** Description:        Computation of random excitation for inactive frames:
**                     Adaptive codebook entry selected randomly
**                     Higher rate innovation pattern selected randomly
**                     Computes innovation gain to match curGain
**
** Links to text:
**
** Arguments:
**
**  int16_t curGain     current average gain to match
**  int16_t *PrevExc    previous/current excitation (updated)
**  int16_t *DataEXc    current frame excitation
**  int16_t *nRandom    random generator status (input/output)
**
** Outputs:
**
**  int16_t *PrevExc
**  int16_t *DataExc
**  int16_t *nRandom
**
** Return value:       None
**
*/
void Calc_Exc_Rand(int16_t curGain, int16_t * PrevExc, int16_t * DataExc,
		   int16_t * nRandom, LINEDEF * Line)
{
	int i, i_subfr, iblk;
	int16_t temp, temp2;
	int16_t j;
	int16_t TabPos[2 * NbPulsBlk], TabSign[2 * NbPulsBlk];
	int16_t *ptr_TabPos, *ptr_TabSign;
	int16_t *ptr1, *curExc;
	int16_t sh1, x1, x2, inter_exc, delta, b0;
	int32_t L_acc, L_c, L_temp;
	int16_t tmp[SubFrLen / Sgrid];
	int16_t offset[SubFrames];
	int16_t tempExc[SubFrLenD];

	/*
	 * generate LTP codes
	 */
	Line->Olp[0] = random_number(21, nRandom) + (int16_t) 123;
	Line->Olp[1] = random_number(19, nRandom) + (int16_t) 123;	/* G723.1 maintenance April 2006 */
	/* Before : Line->Olp[1] = random_number(21, nRandom) + (int16_t)123; */
	for (i_subfr = 0; i_subfr < SubFrames; i_subfr++) {	/* in [1, NbFilt] */
		Line->Sfs[i_subfr].AcGn =
		    random_number(NbFilt, nRandom) + (int16_t) 1;
	}
	Line->Sfs[0].AcLg = 1;
	Line->Sfs[1].AcLg = 0;
	Line->Sfs[2].AcLg = 1;
	Line->Sfs[3].AcLg = 3;

	/*
	 * Random innovation :
	 * Selection of the grids, signs and pulse positions
	 */

	/* Signs and Grids */
	ptr_TabSign = TabSign;
	ptr1 = offset;
	for (iblk = 0; iblk < SubFrames / 2; iblk++) {
		temp = random_number((1 << (NbPulsBlk + 2)), nRandom);
		*ptr1++ = temp & (int16_t) 0x0001;
		temp = g723_shr(temp, 1);
		*ptr1++ = g723_add((int16_t) SubFrLen,
				   (int16_t) (temp & 0x0001));
		for (i = 0; i < NbPulsBlk; i++) {
			*ptr_TabSign++ =
			    g723_shl(g723_sub((temp & (int16_t) 0x0002), 1),
				     14);
			temp = g723_shr(temp, 1);
		}
	}

	/* Positions */
	ptr_TabPos = TabPos;
	for (i_subfr = 0; i_subfr < SubFrames; i_subfr++) {

		for (i = 0; i < (SubFrLen / Sgrid); i++)
			tmp[i] = (int16_t) i;
		temp = (SubFrLen / Sgrid);
		for (i = 0; i < Nb_puls[i_subfr]; i++) {
			j = random_number(temp, nRandom);
			*ptr_TabPos++ =
			    g723_add(g723_shl(tmp[(int)j], 1), offset[i_subfr]);
			temp = g723_sub(temp, 1);
			tmp[(int)j] = tmp[(int)temp];
		}
	}

	/*
	 * Compute fixed codebook gains
	 */

	ptr_TabPos = TabPos;
	ptr_TabSign = TabSign;
	curExc = DataExc;
	i_subfr = 0;
	for (iblk = 0; iblk < SubFrames / 2; iblk++) {

		/* decode LTP only */
		Decod_Acbk(curExc, &PrevExc[0], Line->Olp[iblk],
			   Line->Sfs[i_subfr].AcLg, Line->Sfs[i_subfr].AcGn);
		Decod_Acbk(&curExc[SubFrLen], &PrevExc[SubFrLen],
			   Line->Olp[iblk], Line->Sfs[i_subfr + 1].AcLg,
			   Line->Sfs[i_subfr + 1].AcGn);

		temp2 = 0;
		for (i = 0; i < SubFrLenD; i++) {
			temp = g723_abs_s(curExc[i]);
			if (temp > temp2)
				temp2 = temp;
		}
		if (temp2 == 0)
			sh1 = 0;
		else {
			sh1 = g723_sub(4, g723_norm_s(temp2));	/* 4 bits of margin  */
			if (sh1 < -2)
				sh1 = -2;
		}

		L_temp = 0L;
		for (i = 0; i < SubFrLenD; i++) {
			temp = g723_shr(curExc[i], sh1);	/* left if sh1 < 0 */
			L_temp = g723_L_mac(L_temp, temp, temp);
			tempExc[i] = temp;
		}		/* ener_ltp x 2**(-2sh1+1) */

		L_acc = 0L;
		for (i = 0; i < NbPulsBlk; i++) {
			L_acc =
			    g723_L_mac(L_acc, tempExc[(int)ptr_TabPos[i]],
				       ptr_TabSign[i]);
		}
		inter_exc = g723_extract_h(L_g723_shl(L_acc, 1));	/* inter_exc x 2-sh1 */

		/* compute SubFrLenD x curGain**2 x 2**(-2sh1+1)    */
		/* curGain input = 2**5 curGain                     */
		L_acc = L_g723_mult(curGain, SubFrLen);
		L_acc = L_g723_shr(L_acc, 6);
		temp = g723_extract_l(L_acc);	/* SubFrLen x curGain : avoids overflows */
		L_acc = L_g723_mult(temp, curGain);
		temp = g723_shl(sh1, 1);
		temp = g723_add(temp, 4);
		L_acc = L_g723_shr(L_acc, temp);	/* SubFrLenD x curGain**2 x 2**(-2sh1+1) */

		/* c = (ener_ltp - SubFrLenD x curGain**2)/nb_pulses_blk */
		/* compute L_c = c >> 2sh1-1                                */
		L_acc = L_g723_sub(L_temp, L_acc);
		/* x 1/nb_pulses_blk */
		L_c = L_mls(L_acc, InvNbPulsBlk);

/*
 * Solve EQ(X) = X**2 + 2 b0 X + c
 */
		/* delta = b0 x b0 - c */
		b0 = g723_mult_r(inter_exc, InvNbPulsBlk);	/* b0 >> sh1 */
		L_acc = g723_L_msu(L_c, b0, b0);	/* (c - b0**2) >> 2sh1-1 */
		L_acc = L_g723_negate(L_acc);	/* delta x 2**(-2sh1+1) */

		/* Case delta <= 0 */
		if (L_acc <= 0) {	/* delta <= 0 */
			x1 = g723_negate(b0);	/* sh1 */
		}

		/* Case delta > 0 */
		else {
			delta = Sqrt_lbc(L_acc);	/* >> sh1 */
			x1 = g723_sub(delta, b0);	/* x1 >> sh1 */
			x2 = g723_add(b0, delta);	/* (-x2) >> sh1 */
			if (g723_abs_s(x2) < g723_abs_s(x1)) {
				x1 = g723_negate(x2);
			}
		}

		/* Update DataExc */
		sh1 = g723_add(sh1, 1);
		temp = g723_shl(x1, sh1);
		if (temp > (2 * Gexc_Max))
			temp = (2 * Gexc_Max);
		if (temp < -(2 * Gexc_Max))
			temp = -(2 * Gexc_Max);
		for (i = 0; i < NbPulsBlk; i++) {
			j = *ptr_TabPos++;
			curExc[(int)j] =
			    g723_add(curExc[(int)j],
				     g723_mult(temp, (*ptr_TabSign++)));
		}

		/* update PrevExc */
		ptr1 = PrevExc;
		for (i = SubFrLenD; i < PitchMax; i++)
			*ptr1++ = PrevExc[i];
		for (i = 0; i < SubFrLenD; i++)
			*ptr1++ = curExc[i];

		curExc += SubFrLenD;
		i_subfr += 2;

	}			/* end of loop on LTP blocks */

	return;
}

/*
**
** Function:           random_number()
**
** Description:        returns a number randomly taken in [0, n]
**                     with np1 = n+1 at input
**
** Links to text:
**
** Arguments:
**
**  int16_t np1
**  int16_t *nRandom    random generator status (input/output)
**
** Outputs:
**
**  int16_t *nRandom
**
** Return value:       random number in [0, (np1-1)]
**
*/
int16_t random_number(int16_t np1, int16_t * nRandom)
{
	int16_t temp;

	temp = Rand_lbc(nRandom) & (int16_t) 0x7FFF;
	temp = g723_mult(temp, np1);
	return (temp);
}

/*
**
** Function:           Qua_SidGain()
**
** Description:        Quantization of Sid gain
**                     Pseudo-log quantizer in 3 segments
**                     1st segment : length = 16, resolution = 2
**                     2nd segment : length = 16, resolution = 4
**                     3rd segment : length = 32, resolution = 8
**                     quantizes a sum of energies
**
** Links to text:
**
** Arguments:
**
**  int16_t *Ener        table of the energies
**  int16_t *shEner      corresponding scaling factors
**  int16_t nq           if nq >= 1 : quantization of nq energies
**                      for SID gain calculation in function Cod_Cng()
**                      if nq = 0 : in function Comp_Info(),
**                      quantization of saved estimated excitation energy
**
** Outputs:             None
**
**
** Return value:       index of quantized energy
**
*/
int16_t Qua_SidGain(int16_t * Ener, int16_t * shEner, int16_t nq)
{
	int16_t temp, iseg, iseg_p1;
	int16_t j, j2, k, exp;
	int32_t L_x, L_y;
	int16_t sh1;
	int32_t L_acc;
	int i;

	if (nq == 0) {
		/* Quantize energy saved for frame erasure case                */
		/* L_x = 2 x average_ener                                      */
		temp = g723_shl(*shEner, 1);
		temp = g723_sub(16, temp);
		L_acc = g723_L_deposit_l(*Ener);
		L_acc = L_g723_shl(L_acc, temp);	/* may overflow, and >> if temp < 0 */
		L_x = L_mls(L_acc, nfact[0]);
	}

	else {

		/*
		 * Compute weighted average of energies
		 * Ener[i] = enerR[i] x 2**(shEner[i]-14)
		 * L_x = k[nq] x SUM(i=0->nq-1) enerR[i]
		 * with k[nq] =  2 x fact_mul x fact_mul / nq x Frame
		 */
		sh1 = shEner[0];
		for (i = 1; i < nq; i++) {
			if (shEner[i] < sh1)
				sh1 = shEner[i];
		}
		for (i = 0, L_x = 0L; i < nq; i++) {
			temp = g723_sub(shEner[i], sh1);
			temp = g723_shr(Ener[i], temp);
			temp = g723_mult_r(nfact[nq], temp);
			L_x = L_g723_add(L_x, g723_L_deposit_l(temp));
		}
		temp = g723_sub(15, sh1);
		L_x = L_g723_shl(L_x, temp);
	}

	/* Quantize L_x */
	if (L_x >= L_bseg[2])
		return (63);

	/* Compute segment number iseg */
	if (L_x >= L_bseg[1]) {
		iseg = 2;
		exp = 4;
	} else {
		exp = 3;
		if (L_x >= L_bseg[0])
			iseg = 1;
		else
			iseg = 0;
	}

	iseg_p1 = g723_add(iseg, 1);
	j = g723_shl(1, exp);
	k = g723_shr(j, 1);

	/* Binary search in segment iseg */
	for (i = 0; i < exp; i++) {
		temp = g723_add(base[iseg], g723_shl(j, iseg_p1));
		L_y = L_g723_mult(temp, temp);
		if (L_x >= L_y)
			j = g723_add(j, k);
		else
			j = g723_sub(j, k);
		k = g723_shr(k, 1);
	}

	temp = g723_add(base[iseg], g723_shl(j, iseg_p1));
	L_y = L_g723_mult(temp, temp);
	L_y = L_g723_sub(L_y, L_x);
	if (L_y <= 0L) {
		j2 = g723_add(j, 1);
		temp = g723_add(base[iseg], g723_shl(j2, iseg_p1));
		L_acc = L_g723_mult(temp, temp);
		L_acc = L_g723_sub(L_x, L_acc);
		if (L_y > L_acc)
			temp = g723_add(g723_shl(iseg, 4), j);
		else
			temp = g723_add(g723_shl(iseg, 4), j2);
	} else {
		j2 = g723_sub(j, 1);
		temp = g723_add(base[iseg], g723_shl(j2, iseg_p1));
		L_acc = L_g723_mult(temp, temp);
		L_acc = L_g723_sub(L_x, L_acc);
		if (L_y < L_acc)
			temp = g723_add(g723_shl(iseg, 4), j);
		else
			temp = g723_add(g723_shl(iseg, 4), j2);
	}
	return (temp);
}

/*
**
** Function:           Dec_SidGain()
**
** Description:        Decoding of quantized Sid gain
**                     (corresponding to sqrt of average energy)
**
** Links to text:
**
** Arguments:
**
**  int16_t iGain        index of quantized Sid Gain
**
** Outputs:             None
**
** Return value:        decoded gain value << 5
**
*/
int16_t Dec_SidGain(int16_t iGain)
{
	int16_t i, iseg;
	int16_t temp;

	iseg = g723_shr(iGain, 4);
	if (iseg == 3)
		iseg = 2;
	i = g723_sub(iGain, g723_shl(iseg, 4));
	temp = g723_add(iseg, 1);
	temp = g723_shl(i, temp);
	temp = g723_add(temp, base[iseg]);	/* SidGain */
	temp = g723_shl(temp, 5);	/* << 5 */
	return (temp);
}
