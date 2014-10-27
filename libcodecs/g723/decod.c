/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:        "decod.c"
**
** Description:     Top-level source code for G.723.1 dual-rate decoder
**
** Functions:       Init_Decod()
**                  Decod()
**
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
#include <string.h>
#include <ophtools.h>

#include "g723_const.h"
#include "lbccodec.h"

extern int UsePf;
extern int16_t LspDcTable[LpcOrder];

extern LINEDEF Line_Unpk(char *Vinp, int16_t * Ftyp, int16_t Crc);
extern void Dec_Cng(int16_t Ftyp, LINEDEF * Line, int16_t * DataExc,
		    int16_t * QntLpc);

extern DECCNGDEF DecCng;
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
//extern  int32_t L_g723_add(int32_t L_var1, int32_t L_var2);   /* Long add,        2 */
extern void Lsp_Inq(int16_t * Lsp, int16_t * PrevLsp, int32_t LspId,
		    int16_t Crc);
extern void Lsp_Int(int16_t * QntLpc, int16_t * CurrLsp, int16_t * PrevLsp);
extern int16_t g723_shr(int16_t var1, int16_t var2);	/* Short shift right,   1 */

extern int16_t FcbkGainTable[NumOfGainLev];

extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */
extern void Fcbk_Unpk(int16_t * Tv, SFSDEF Sfs, int16_t Olp, int16_t Sfc);
extern int16_t g723_shl(int16_t var1, int16_t var2);	/* Short shift left,    1 */
extern int16_t Comp_Info(int16_t * Buff, int16_t Olp, int16_t * Gain,
			int16_t * ShGain);
extern PFDEF Comp_Lpf(int16_t * Buff, int16_t Olp, int16_t Sfc);
extern void Filt_Lpf(int16_t * Tv, int16_t * Buff, PFDEF Pf, int16_t Sfc);
extern void Regen(int16_t * DataBuff, int16_t * Buff, int16_t Lag,
		  int16_t Gain,
		  int16_t Ecount, int16_t * Sd);
extern void Decod_Acbk(int16_t * Tv, int16_t * PrevExc, int16_t Olp,
		       int16_t Lid,
		       int16_t Gid);

extern int16_t SyntIirDl[LpcOrder];

extern void Synt(int16_t * Dpnt, int16_t * Lpc);
extern int32_t Spf(int16_t * Tv, int16_t * Lpc);
extern void Scale(int16_t * Tv, int32_t Sen);
/*
   The following structure contains all the static decoder
      variables.
*/

DECSTATDEF DecStat;

/*
**
** Function:        Init_Decod()
** 
** Description:     Initializes non-zero state variables
**          for the decoder.
**
** Links to text:   Section 3.11
** 
** Arguments:       None
** 
** Outputs:     None
** 
** Return value:    None
**
*/
void Init_Decod()
{
	int i;

	/* Initialize encoder data structure with zeros */
	memzero(&DecStat, sizeof(DECSTATDEF));

	/* Initialize the previously decoded LSP vector to the DC vector */
	for (i = 0; i < LpcOrder; i++)
		DecStat.PrevLsp[i] = LspDcTable[i];

	/* Initialize the gain scaling unit memory to a constant */
	DecStat.Gain = (int16_t) 0x1000;

	return;
}

/*
**
** Function:        Decod()
**
** Description:     Implements G.723.1 dual-rate decoder for  a frame
**          of speech
**
** Links to text:   Section 3
**
** Arguments:
**
**  int16_t *DataBuff    Empty buffer
**  int16_t Vinp[]       Encoded frame (22/26 bytes)
**

** Outputs:
**
**  int16_t DataBuff[]   Decoded frame (480 bytes)
**
** Return value:
**
**  int            Always True
**
*/

int Decod(int16_t * DataBuff, char *Vinp, int16_t Crc)
{
	int i, j;

	int32_t Senr;
	int16_t QntLpc[SubFrames * LpcOrder];
	int16_t AcbkCont[SubFrLen];

	int16_t LspVect[LpcOrder];
	int16_t Temp[PitchMax + Frame];
	int16_t *Dpnt;

	LINEDEF Line;
	PFDEF Pf[SubFrames];

	int16_t Ftyp;

	/*
	 * Decode the packed bitstream for the frame.  (Text: Section 4;
	 * pars of sections 2.17, 2.18)
	 */
	Line = Line_Unpk(Vinp, &Ftyp, Crc);

	/*
	 * Update the frame erasure count (Text: Section 3.10)
	 */
	if (Line.Crc != (int16_t) 0) {
		if (DecCng.PastFtyp == 1)
			Ftyp = 1;	/* active */
		else
			Ftyp = 0;	/* untransmitted */
	}

	if (Ftyp != 1) {

		/* Silence frame : do noise generation */
		Dec_Cng(Ftyp, &Line, DataBuff, QntLpc);
	}

	else {

		/*
		 * Update the frame erasure count (Text: Section 3.10)
		 */
		if (Line.Crc != (int16_t) 0)
			DecStat.Ecount = g723_add(DecStat.Ecount, (int16_t) 1);
		else
			DecStat.Ecount = (int16_t) 0;

		if (DecStat.Ecount > (int16_t) ErrMaxNum)
			DecStat.Ecount = (int16_t) ErrMaxNum;

		/*
		 * Decode the LSP vector for subframe 3.  (Text: Section 3.2)
		 */
		Lsp_Inq(LspVect, DecStat.PrevLsp, Line.LspId, Line.Crc);

		/*
		 * Interpolate the LSP vectors for subframes 0--2.  Convert the
		 * LSP vectors to LPC coefficients.  (Text: Section 3.3)
		 */
		Lsp_Int(QntLpc, LspVect, DecStat.PrevLsp);

		/* Copy the LSP vector for the next frame */
		for (i = 0; i < LpcOrder; i++)
			DecStat.PrevLsp[i] = LspVect[i];

		/*
		 * In case of no erasure, update the interpolation gain memory.
		 * Otherwise compute the interpolation gain (Text: Section 3.10)
		 */
		if (DecStat.Ecount == (int16_t) 0) {
			DecStat.InterGain =
			    g723_add(Line.Sfs[SubFrames - 2].Mamp,
				     Line.Sfs[SubFrames - 1].Mamp);
			DecStat.InterGain =
			    g723_shr(DecStat.InterGain, (int16_t) 1);
			DecStat.InterGain = FcbkGainTable[DecStat.InterGain];
		} else
			DecStat.InterGain =
			    g723_mult_r(DecStat.InterGain, (int16_t) 0x6000);

		/*
		 * Generate the excitation for the frame
		 */
		for (i = 0; i < PitchMax; i++)
			Temp[i] = DecStat.PrevExc[i];

		Dpnt = &Temp[PitchMax];

		if (DecStat.Ecount == (int16_t) 0) {

			for (i = 0; i < SubFrames; i++) {

				/* Generate the fixed codebook excitation for a
				   subframe. (Text: Section 3.5) */
				Fcbk_Unpk(Dpnt, Line.Sfs[i], Line.Olp[i >> 1],
					  (int16_t) i);

				/* Generate the adaptive codebook excitation for a
				   subframe. (Text: Section 3.4) */
				Decod_Acbk(AcbkCont, &Temp[SubFrLen * i],
					   Line.Olp[i >> 1], Line.Sfs[i].AcLg,
					   Line.Sfs[i].AcGn);

				/* Add the adaptive and fixed codebook contributions to
				   generate the total excitation. */
				for (j = 0; j < SubFrLen; j++) {
					Dpnt[j] = g723_shl(Dpnt[j],
							   (int16_t) 1);
					Dpnt[j] =
					    g723_add(Dpnt[j], AcbkCont[j]);
				}

				Dpnt += SubFrLen;
			}

			/* Save the excitation */
			for (j = 0; j < Frame; j++)
				DataBuff[j] = Temp[PitchMax + j];

			/* Compute interpolation index. (Text: Section 3.10) */
			/* Use DecCng.SidGain and DecCng.CurGain to store    */
			/* excitation energy estimation                      */
			DecStat.InterIndx =
			    Comp_Info(Temp, Line.Olp[SubFrames / 2 - 1],
				      &DecCng.SidGain, &DecCng.CurGain);

			/* Compute pitch post filter coefficients.  (Text: Section 3.6) */
			if (UsePf)
				for (i = 0; i < SubFrames; i++)
					Pf[i] =
					    Comp_Lpf(Temp, Line.Olp[i >> 1],
						     (int16_t) i);

			/* Reload the original excitation */
			for (j = 0; j < PitchMax; j++)
				Temp[j] = DecStat.PrevExc[j];
			for (j = 0; j < Frame; j++)
				Temp[PitchMax + j] = DataBuff[j];

			/* Perform pitch post filtering for the frame.  (Text: Section
			   3.6) */
			if (UsePf)
				for (i = 0; i < SubFrames; i++)
					Filt_Lpf(DataBuff, Temp, Pf[i],
						 (int16_t) i);

			/* Save Lsps --> LspSid */
			for (i = 0; i < LpcOrder; i++)
				DecCng.LspSid[i] = DecStat.PrevLsp[i];
		}

		else {

			/* If a frame erasure has occurred, regenerate the
			   signal for the frame. (Text: Section 3.10) */
			Regen(DataBuff, Temp, DecStat.InterIndx,
			      DecStat.InterGain, DecStat.Ecount,
			      &DecStat.Rseed);
		}

		/* Update the previous excitation for the next frame */
		for (j = 0; j < PitchMax; j++)
			DecStat.PrevExc[j] = Temp[Frame + j];

		/* Resets random generator for CNG */
		DecCng.RandSeed = 12345;
	}

	/* Save Ftyp information for next frame */
	DecCng.PastFtyp = Ftyp;

	/*
	 * Synthesize the speech for the frame
	 */
	Dpnt = DataBuff;
	for (i = 0; i < SubFrames; i++) {

		/* Compute the synthesized speech signal for a subframe.
		 * (Text: Section 3.7)
		 */
		Synt(Dpnt, &QntLpc[i * LpcOrder]);

		if (UsePf) {

			/* Do the formant post filter. (Text: Section 3.8) */
			Senr = Spf(Dpnt, &QntLpc[i * LpcOrder]);

			/* Do the gain scaling unit.  (Text: Section 3.9) */
			Scale(Dpnt, Senr);
		}

		Dpnt += SubFrLen;
	}
	return (int) True;
}
