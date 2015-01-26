/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:    util_lbc.c
**
** Description: utility functions for the lbc codec
**
** Functions:
**
**  I/O functions:
**
**      Write_lbc()
**
**  High-pass filtering:
**
**      Rem_Dc()
**
**  Miscellaneous signal processing functions:
**
**      Vec_Norm()
**      Mem_Shift()
**      Comp_En()
**      Scale()
**
**  Bit stream packing/unpacking:
**
**      Line_Pack()
**      Line_Unpk()
**
**  Mathematical functions:
**
**      Sqrt_lbc()
**      Rand_lbc()
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include <stdlib.h>
#include <stdio.h>

#include "g723_const.h"
extern int UseHp;
extern CODSTATDEF CodStat;
extern DECSTATDEF DecStat;
extern enum Crate WrkRate;

extern void Line_Pack(LINEDEF * Line, char *Vout, int16_t Ftyp);
extern int32_t Ser2Par(int16_t ** Pnt, int Count);
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */

extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern int32_t g723_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,    1 */
extern int32_t L_mls(int32_t, int16_t);	/* Wght ?? */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */
extern int16_t round_(int32_t L_var1);	/* Round,               1 */
extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */
extern int16_t g723_shr(int16_t var1, int16_t var2);	/* Short shift right,   1 */
extern int32_t L_g723_shr(int32_t L_var1, int16_t var2);	/* Long shift right,    2 */
extern int16_t g723_abs_s(int16_t var1);	/* Short abs,           1 */
extern int16_t g723_norm_s(int16_t var1);	/* Short norm,           15 */
//extern int32_t L_g723_shr(int32_t L_var1, int16_t var2); /* Long shift right,    2 */
extern int16_t g723_extract_l(int32_t L_var1);	/* Extract low,         1 */
extern int16_t g723_sub(int16_t var1, int16_t var2);	/* Short sub,           1 */
extern int16_t *Par2Ser(int32_t Inp, int16_t * Pnt, int BitNum);
extern int16_t g723_norm_l(int32_t L_var1);	/* Long norm,            30 */
extern int32_t L_g723_shl(int32_t L_var1, int16_t var2);	/* Long shift left,     2 */
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int16_t div_l(int32_t, int16_t);
extern int32_t g723_L_deposit_h(int16_t var1);	/* 16 bit var1 -> MSB,     2 */
extern int32_t g723_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,    1 */
extern int32_t g723_L_deposit_l(int16_t var1);	/* 16 bit var1 -> LSB,     2 */
/*
**
** Function:        Rem_Dc()
**
** Description:     High-pass filtering
**
** Links to text:   Section 2.3
**
** Arguments:
**
**  int16_t *Dpnt
**
** Inputs:
**
**  CodStat.HpfZdl  FIR filter memory from previous frame (1 word)
**  CodStat.HpfPdl  IIR filter memory from previous frame (1 word)
**
** Outputs:
**
**  int16_t *Dpnt
**
** Return value:    None
**
*/
void Rem_Dc(int16_t * Dpnt)
{
	int i;

	int32_t Acc0, Acc1;

	if (UseHp) {
		for (i = 0; i < Frame; i++) {

			/* Do the Fir and scale by 2 */
			Acc0 = L_g723_mult(Dpnt[i], (int16_t) 0x4000);
			Acc0 =
			    g723_L_mac(Acc0, CodStat.HpfZdl, (int16_t) 0xc000);
			CodStat.HpfZdl = Dpnt[i];

			/* Do the Iir part */
			Acc1 = L_mls(CodStat.HpfPdl, (int16_t) 0x7f00);
			Acc0 = L_g723_add(Acc0, Acc1);
			CodStat.HpfPdl = Acc0;
			Dpnt[i] = round_(Acc0);
		}
	} else {
		for (i = 0; i < Frame; i++)
			Dpnt[i] = g723_shr(Dpnt[i], (int16_t) 1);
	}

	return;
}

/*
**
** Function:        Vec_Norm()
**
** Description:     Vector normalization
**
** Links to text:
**
** Arguments:
**
**  int16_t *Vect
**  int16_t Len
**
** Outputs:
**
**  int16_t *Vect
**
** Return value:  The power of 2 by which the data vector multiplyed.
**
*/
int16_t Vec_Norm(int16_t * Vect, int16_t Len)
{
	int i;

	int16_t Acc0, Acc1;
	int16_t Exp;
	int16_t Rez;
	int32_t Temp;

	static short ShiftTable[16] = {
		0x0001,
		0x0002,
		0x0004,
		0x0008,
		0x0010,
		0x0020,
		0x0040,
		0x0080,
		0x0100,
		0x0200,
		0x0400,
		0x0800,
		0x1000,
		0x2000,
		0x4000,
		0x7fff
	};

	/* Find absolute maximum */
	Acc1 = (int16_t) 0;
	for (i = 0; i < Len; i++) {
		Acc0 = g723_abs_s(Vect[i]);
		if (Acc0 > Acc1)
			Acc1 = Acc0;
	}

	/* Get the shift count */
	Rez = g723_norm_s(Acc1);
	Exp = ShiftTable[Rez];

	/* Normalize all the vector */
	for (i = 0; i < Len; i++) {
		Temp = L_g723_mult(Exp, Vect[i]);
		Temp = L_g723_shr(Temp, 4);
		Vect[i] = g723_extract_l(Temp);
	}

	Rez = g723_sub(Rez, (int16_t) 3);
	return Rez;
}

/*
**
** Function:        Mem_Shift()
**
** Description:     Memory shift, update of the high-passed input speech signal
**
** Links to text:
**
** Arguments:
**
**  int16_t *PrevDat
**  int16_t *DataBuff
**
** Outputs:
**
**  int16_t *PrevDat
**  int16_t *DataBuff
**
** Return value:    None
**
*/
void Mem_Shift(int16_t * PrevDat, int16_t * DataBuff)
{
	int i;

	int16_t Dpnt[Frame + LpcFrame - SubFrLen];

	/*  Form Buffer  */
	for (i = 0; i < LpcFrame - SubFrLen; i++)
		Dpnt[i] = PrevDat[i];
	for (i = 0; i < Frame; i++)
		Dpnt[i + LpcFrame - SubFrLen] = DataBuff[i];

	/* Update PrevDat */
	for (i = 0; i < LpcFrame - SubFrLen; i++)
		PrevDat[i] = Dpnt[Frame + i];

	/* Update DataBuff */
	for (i = 0; i < Frame; i++)
		DataBuff[i] = Dpnt[(LpcFrame - SubFrLen) / 2 + i];

	return;
}

/*
**
** Function:        Line_Pack()
**
** Description:     Packing coded parameters in bitstream of 16-bit words
**
** Links to text:   Section 4
**
** Arguments:
**
**  LINEDEF *Line     Coded parameters for a frame
**  char    *Vout     bitstream chars
**  int16_t   VadBit   Voice Activity Indicator
**
** Outputs:
**
**  int16_t *Vout
**
** Return value:    None
**
*/
void Line_Pack(LINEDEF * Line, char *Vout, int16_t Ftyp)
{
	int i;
	int BitCount;

	int16_t BitStream[192];
	int16_t *Bsp = BitStream;
	int32_t Temp;

	/* Clear the output vector */
	for (i = 0; i < 24; i++)
		Vout[i] = 0;

	/*
	 * Add the coder rate info and frame type info to the 2 msb
	 * of the first word of the frame.
	 * The signaling is as follows:
	 *     Ftyp  WrkRate => X1X0
	 *       1     Rate63     00  :   High Rate
	 *       1     Rate53     01  :   Low  Rate
	 *       2       x        10  :   Silence Insertion Descriptor frame
	 *       0       x        11  :   Used only for simulation of
	 *                                 untransmitted silence frames
	 */
	switch (Ftyp) {

	case 0:{
			Temp = 0x00000003L;
			break;
		}

	case 2:{
			Temp = 0x00000002L;
			break;
		}

	default:{
			if (WrkRate == Rate63)
				Temp = 0x00000000L;
			else
				Temp = 0x00000001L;
			break;
		}
	}

	/* Serialize Control info */
	Bsp = Par2Ser(Temp, Bsp, 2);

	/* Check for Speech/NonSpeech case */
	if (Ftyp == 1) {

		/* 24 bit LspId */
		Temp = (*Line).LspId;
		Bsp = Par2Ser(Temp, Bsp, 24);

		/*
		 * Do the part common to both rates
		 */

		/* Adaptive code book lags */
		Temp = (int32_t) (*Line).Olp[0] - (int32_t) PitchMin;
		Bsp = Par2Ser(Temp, Bsp, 7);

		Temp = (int32_t) (*Line).Sfs[1].AcLg;
		Bsp = Par2Ser(Temp, Bsp, 2);

		Temp = (int32_t) (*Line).Olp[1] - (int32_t) PitchMin;
		Bsp = Par2Ser(Temp, Bsp, 7);

		Temp = (int32_t) (*Line).Sfs[3].AcLg;
		Bsp = Par2Ser(Temp, Bsp, 2);

		/* Write combined 12 bit index of all the gains */
		for (i = 0; i < SubFrames; i++) {
			Temp =
			    (*Line).Sfs[i].AcGn * NumOfGainLev +
			    (*Line).Sfs[i].Mamp;
			if (WrkRate == Rate63)
				Temp += (int32_t) (*Line).Sfs[i].Tran << 11;
			Bsp = Par2Ser(Temp, Bsp, 12);
		}

		/* Write all the Grid indices */
		for (i = 0; i < SubFrames; i++)
			*Bsp++ = (*Line).Sfs[i].Grid;

		/* High rate only part */
		if (WrkRate == Rate63) {

			/* Write the reserved bit as 0 */
			*Bsp++ = (int16_t) 0;

			/* Write 13 bit combined position index */
			Temp = (*Line).Sfs[0].Ppos >> 16;
			Temp = Temp * 9 + ((*Line).Sfs[1].Ppos >> 14);
			Temp *= 90;
			Temp +=
			    ((*Line).Sfs[2].Ppos >> 16) * 9 +
			    ((*Line).Sfs[3].Ppos >> 14);
			Bsp = Par2Ser(Temp, Bsp, 13);

			/* Write all the pulse positions */
			Temp = (*Line).Sfs[0].Ppos & 0x0000ffffL;
			Bsp = Par2Ser(Temp, Bsp, 16);

			Temp = (*Line).Sfs[1].Ppos & 0x00003fffL;
			Bsp = Par2Ser(Temp, Bsp, 14);

			Temp = (*Line).Sfs[2].Ppos & 0x0000ffffL;
			Bsp = Par2Ser(Temp, Bsp, 16);

			Temp = (*Line).Sfs[3].Ppos & 0x00003fffL;
			Bsp = Par2Ser(Temp, Bsp, 14);

			/* Write pulse amplitudes */
			Temp = (int32_t) (*Line).Sfs[0].Pamp;
			Bsp = Par2Ser(Temp, Bsp, 6);

			Temp = (int32_t) (*Line).Sfs[1].Pamp;
			Bsp = Par2Ser(Temp, Bsp, 5);

			Temp = (int32_t) (*Line).Sfs[2].Pamp;
			Bsp = Par2Ser(Temp, Bsp, 6);

			Temp = (int32_t) (*Line).Sfs[3].Pamp;
			Par2Ser(Temp, Bsp, 5);
		}

		/* Low rate only part */
		else {

			/* Write 12 bits of positions */
			for (i = 0; i < SubFrames; i++) {
				Temp = (*Line).Sfs[i].Ppos;
				Bsp = Par2Ser(Temp, Bsp, 12);
			}

			/* Write 4 bit Pamps */
			for (i = 0; i < SubFrames; i++) {
				Temp = (*Line).Sfs[i].Pamp;
				Bsp = Par2Ser(Temp, Bsp, 4);
			}
		}

	}

	else if (Ftyp == 2) {	/* SID frame */

		/* 24 bit LspId */
		Temp = (*Line).LspId;
		Bsp = Par2Ser(Temp, Bsp, 24);

		/* Do Sid frame gain */
		Temp = (int32_t) (*Line).Sfs[0].Mamp;
		Par2Ser(Temp, Bsp, 6);
	}

	/* Write out active frames */
	if (Ftyp == 1) {
		if (WrkRate == Rate63)
			BitCount = 192;
		else
			BitCount = 160;
	}
	/* Non active frames */
	else if (Ftyp == 2)
		BitCount = 32;
	else
		BitCount = 2;

	for (i = 0; i < BitCount; i++)
		Vout[i >> 3] ^= BitStream[i] << (i & 0x0007);

	return;
}

int16_t *Par2Ser(int32_t Inp, int16_t * Pnt, int BitNum)
{
	int i;
	int16_t Temp;

	for (i = 0; i < BitNum; i++) {
		Temp = (int16_t) Inp & (int16_t) 0x0001;
		Inp >>= 1;
		*Pnt++ = Temp;
	}

	return Pnt;
}

/*
**
** Function:        Line_Unpk()
**
** Description:     unpacking of bitstream, gets coding parameters for a frame
**
** Links to text:   Section 4
**
** Arguments:
**
**  char   *Vinp        bitstream chars
**  int16_t *VadType
**
** Outputs:
**
**  int16_t *VadType
**
** Return value:
**
**  LINEDEF             coded parameters
**     int16_t   Crc
**     int32_t   LspId
**     int16_t   Olp[SubFrames/2]
**     SFSDEF   Sfs[SubFrames]
**
*/
LINEDEF Line_Unpk(char *Vinp, int16_t * Ftyp, int16_t Crc)
{
	int i;
	int16_t BitStream[192];
	int16_t *Bsp = BitStream;
	LINEDEF Line;
	int32_t Temp;
	int16_t Info;
	int16_t Bound_AcGn;

	Line.Crc = Crc;
	if (Crc != 0) {
		Line.LspId = 0L;	/* Dummy : to avoid gcc warning */
		return Line;
	}

	/* Unpack the byte info to BitStream vector */
	for (i = 0; i < 192; i++)
		BitStream[i] =
		    (Vinp[i >> 3] >> (i & (int16_t) 0x0007)) & (int16_t) 1;

	/* Decode the frame type and rate info */
	Info = (int16_t) Ser2Par(&Bsp, 2);

	if (Info == 3) {
		*Ftyp = 0;
		Line.LspId = 0L;	/* Dummy : to avoid Borland C3.1 warning */
		return Line;
	}

	/* Decode the LspId */
	Line.LspId = Ser2Par(&Bsp, 24);

	if (Info == 2) {
		/* Decode the Noise Gain */
		Line.Sfs[0].Mamp = (int16_t) Ser2Par(&Bsp, 6);
		*Ftyp = 2;
		return Line;
	}

	/*
	 * Decode the common information to both rates
	 */
	*Ftyp = 1;

	/* Decode the bit-rate */
	WrkRate = (Info == 0) ? Rate63 : Rate53;

	/* Decode the adaptive codebook lags */
	Temp = Ser2Par(&Bsp, 7);
	/* Test if forbidden code */
	if (Temp <= 123) {
		Line.Olp[0] = (int16_t) Temp + (int16_t) PitchMin;
	} else {
		/* transmission error */
		Line.Crc = 1;
		return Line;
	}

	Line.Sfs[1].AcLg = (int16_t) Ser2Par(&Bsp, 2);

	Temp = Ser2Par(&Bsp, 7);
	/* Test if forbidden code */
	if (Temp <= 123) {
		Line.Olp[1] = (int16_t) Temp + (int16_t) PitchMin;
	}

	else {
		/* transmission error */
		Line.Crc = 1;
		return Line;
	}

	Line.Sfs[3].AcLg = (int16_t) Ser2Par(&Bsp, 2);

	Line.Sfs[0].AcLg = 1;
	Line.Sfs[2].AcLg = 1;

	/* Decode the combined gains accordingly to the rate */
	for (i = 0; i < SubFrames; i++) {

		Temp = Ser2Par(&Bsp, 12);

		Line.Sfs[i].Tran = 0;
		Bound_AcGn = NbFilt170;
		if ((WrkRate == Rate63) && (Line.Olp[i >> 1] < (SubFrLen - 2))) {
			Line.Sfs[i].Tran = (int16_t) (Temp >> 11);
			Temp &= 0x000007ffL;
			Bound_AcGn = NbFilt085;
		}
		Line.Sfs[i].AcGn = (int16_t) (Temp / (int16_t) NumOfGainLev);
		if (Line.Sfs[i].AcGn < Bound_AcGn) {
			Line.Sfs[i].Mamp =
			    (int16_t) (Temp % (int16_t) NumOfGainLev);
		} else {
			/* error detected */
			Line.Crc = 1;
			return Line;
		}
	}

	/* Decode the grids */
	for (i = 0; i < SubFrames; i++)
		Line.Sfs[i].Grid = *Bsp++;

	if (Info == 0) {

		/* Skip the reserved bit */
		Bsp++;

		/* Decode 13 bit combined position index */
		Temp = Ser2Par(&Bsp, 13);
		Line.Sfs[0].Ppos = (Temp / 90) / 9;
		Line.Sfs[1].Ppos = (Temp / 90) % 9;
		Line.Sfs[2].Ppos = (Temp % 90) / 9;
		Line.Sfs[3].Ppos = (Temp % 90) % 9;

		/* Decode all the pulse positions */
		Line.Sfs[0].Ppos = (Line.Sfs[0].Ppos << 16) + Ser2Par(&Bsp, 16);
		Line.Sfs[1].Ppos = (Line.Sfs[1].Ppos << 14) + Ser2Par(&Bsp, 14);
		Line.Sfs[2].Ppos = (Line.Sfs[2].Ppos << 16) + Ser2Par(&Bsp, 16);
		Line.Sfs[3].Ppos = (Line.Sfs[3].Ppos << 14) + Ser2Par(&Bsp, 14);

		/* Decode pulse amplitudes */
		Line.Sfs[0].Pamp = (int16_t) Ser2Par(&Bsp, 6);
		Line.Sfs[1].Pamp = (int16_t) Ser2Par(&Bsp, 5);
		Line.Sfs[2].Pamp = (int16_t) Ser2Par(&Bsp, 6);
		Line.Sfs[3].Pamp = (int16_t) Ser2Par(&Bsp, 5);
	}

	else {

		/* Decode the positions */
		for (i = 0; i < SubFrames; i++)
			Line.Sfs[i].Ppos = Ser2Par(&Bsp, 12);

		/* Decode the amplitudes */
		for (i = 0; i < SubFrames; i++)
			Line.Sfs[i].Pamp = (int16_t) Ser2Par(&Bsp, 4);
	}
	return Line;
}

int32_t Ser2Par(int16_t ** Pnt, int Count)
{
	int i;
	int32_t Rez = 0L;

	for (i = 0; i < Count; i++) {
		Rez += (int32_t) ** Pnt << i;
		(*Pnt)++;
	}
	return Rez;
}

/*
**
** Function:        Comp_En()
**
** Description:     Compute energy of a subframe vector
**
** Links to text:
**
** Arguments:
**
**  int16_t *Dpnt
**
** Outputs:         None
**
** Return value:
**
**      int32_t energy
**
*/
int32_t Comp_En(int16_t * Dpnt)
{
	int i;
	int32_t Rez;
	int16_t Temp[SubFrLen];

	for (i = 0; i < SubFrLen; i++)
		Temp[i] = g723_shr(Dpnt[i], (int16_t) 2);

	Rez = (int32_t) 0;
	for (i = 0; i < SubFrLen; i++)
		Rez = g723_L_mac(Rez, Temp[i], Temp[i]);

	return Rez;
}

/*
**
** Function:        Sqrt_lbc()
**
** Description:     Square root computation
**
** Links to text:
**
** Arguments:
**
**  int32_t Num
**
** Outputs:     None
**
** Return value:
**
**  int16_t square root of num
**
*/
int16_t Sqrt_lbc(int32_t Num)
{
	int i;

	int16_t Rez = (int16_t) 0;
	int16_t Exp = (int16_t) 0x4000;

	int32_t Acc;

	for (i = 0; i < 14; i++) {

		Acc = L_g723_mult(g723_add(Rez, Exp), g723_add(Rez, Exp));
		if (Num >= Acc)
			Rez = g723_add(Rez, Exp);

		Exp = g723_shr(Exp, (int16_t) 1);
	}
	return Rez;
}

/*
**
** Function:        Rand_lbc()
**
** Description:     Generator of random numbers
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  int16_t *p
**
** Outputs:
**
**  int16_t *p
**
** Return value:
**
**  int16_t random number
**
*/
int16_t Rand_lbc(int16_t * p)
{
	int32_t Temp;

	Temp = g723_L_deposit_l(*p);
	Temp &= (int32_t) 0x0000ffff;
	Temp = Temp * (int32_t) 521 + (int32_t) 259;
	*p = g723_extract_l(Temp);
	return g723_extract_l(Temp);
}

/*
**
** Function:        Scale()
**
** Description:     Postfilter gain scaling
**
** Links to text:   Section 3.9
**
** Arguments:
**
**  int16_t *Tv
**  int32_t Sen
**
**  Inputs:
**
**  int16_t DecStat.Gain
**
** Outputs:
**
**  int16_t *Tv
**
** Return value:    None
**
*/
void Scale(int16_t * Tv, int32_t Sen)
{
	int i;

	int32_t Acc0, Acc1;
	int16_t Exp, SfGain;

	Acc0 = Sen;
	Acc1 = Comp_En(Tv);

	/* Normalize both */
	if ((Acc1 != (int32_t) 0) && (Acc0 != (int32_t) 0)) {

		Exp = g723_norm_l(Acc1);
		Acc1 = L_g723_shl(Acc1, Exp);

		SfGain = g723_norm_l(Acc0);
		Acc0 = L_g723_shl(Acc0, SfGain);
		Acc0 = L_g723_shr(Acc0, (int16_t) 1);
		Exp = g723_sub(Exp, SfGain);
		Exp = g723_add(Exp, (int16_t) 1);
		Exp = g723_sub((int16_t) 6, Exp);
		if (Exp < (int16_t) 0)
			Exp = (int16_t) 0;

		SfGain = g723_extract_h(Acc1);

		SfGain = div_l(Acc0, SfGain);

		Acc0 = g723_L_deposit_h(SfGain);

		Acc0 = L_g723_shr(Acc0, Exp);

		SfGain = Sqrt_lbc(Acc0);
	} else
		SfGain = 0x1000;

	/* Filter the data */
	for (i = 0; i < SubFrLen; i++) {

		/* Update gain */
		Acc0 = g723_L_deposit_h(DecStat.Gain);
		Acc0 = g723_L_msu(Acc0, DecStat.Gain, (int16_t) 0x0800);
		Acc0 = g723_L_mac(Acc0, SfGain, (int16_t) 0x0800);
		DecStat.Gain = round_(Acc0);

		Exp =
		    g723_add(DecStat.Gain,
			     g723_shr(DecStat.Gain, (int16_t) 4));

		Acc0 = L_g723_mult(Tv[i], Exp);
		Acc0 = L_g723_shl(Acc0, (int16_t) 4);
		Tv[i] = round_(Acc0);
	}

	return;
}
