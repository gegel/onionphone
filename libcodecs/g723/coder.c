/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */


/*
**
** File:        "coder.c"
**
** Description:     Top-level source code for G.723.1 dual-rate coder
**
** Functions:       Init_Coder()
**                  Coder()
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
/*
  This file includes the coder main functions
*/

//begain -----------------------------------add by haiping 2009-06-19
#ifdef TEST_MIPS
extern unsigned int cycles();
extern unsigned int test_start;
extern float mips_test[TESTMIPSNUM];
#endif				//TEST_MIPS
//end    -----------------------------------add by haiping 2009-06-19

CODSTATDEF CodStat;
extern Word16 LspDcTable[LpcOrder];
extern VADSTATDEF VadStat;
extern CODCNGDEF CodCng;

extern Word16 g723_add(Word16 var1, Word16 var2);	/* Short add,           1 */
extern Word16 g723_shl(Word16 var1, Word16 var2);	/* Short shift left,    1 */
extern void Rem_Dc(Word16 * Dpnt);
extern void Comp_Lpc(Word16 * UnqLpc, Word16 * PrevDat, Word16 * DataBuff);
extern void Find_Fcbk(Word16 * Dpnt, Word16 * ImpResp, LINEDEF * Line,
		      Word16 Sfc);
extern PWDEF Comp_Pw(Word16 * Dpnt, Word16 Start, Word16 Olp);
extern void AtoLsp(Word16 * LspVect, Word16 * Lpc, Word16 * PrevLsp);
extern Flag Comp_Vad(Word16 * Dpnt);
extern Word32 Lsp_Qnt(Word16 * CurrLsp, Word16 * PrevLsp);
extern void Mem_Shift(Word16 * PrevDat, Word16 * DataBuff);
extern void Wght_Lpc(Word16 * PerLpc, Word16 * UnqLpc);
extern void Error_Wght(Word16 * Dpnt, Word16 * PerLpc);
extern Word16 Vec_Norm(Word16 * Vect, Word16 Len);
extern Word16 Estim_Pitch(Word16 * Dpnt, Word16 Start);

extern void Init_Cod_Cng(void);
extern void Cod_Cng(Word16 * DataExc, Word16 * Ftyp, LINEDEF * Line,
		    Word16 * QntLpc);
extern void Filt_Pw(Word16 * DataBuff, Word16 * Dpnt, Word16 Start, PWDEF Pw);
extern void Lsp_Inq(Word16 * Lsp, Word16 * PrevLsp, Word32 LspId, Word16 Crc);
extern void Lsp_Int(Word16 * QntLpc, Word16 * CurrLsp, Word16 * PrevLsp);
extern void Comp_Ir(Word16 * ImpResp, Word16 * QntLpc, Word16 * PerLpc,
		    PWDEF Pw);
extern void Update_Err(Word16 Olp, Word16 AcLg, Word16 AcGn);
extern void Upd_Ring(Word16 * Dpnt, Word16 * QntLpc, Word16 * PerLpc,
		     Word16 * PrevErr);
extern void Sub_Ring(Word16 * Dpnt, Word16 * QntLpc, Word16 * PerLpc,
		     Word16 * PrevErr, PWDEF Pw);
extern void Find_Acbk(Word16 * Tv, Word16 * ImpResp, Word16 * PrevExc,
		      LINEDEF * Line, Word16 Sfc);
extern void Line_Pack(LINEDEF * Line, char *Vout, Word16 Ftyp);
extern void Decod_Acbk(Word16 * Tv, Word16 * PrevExc, Word16 Olp, Word16 Lid,
		       Word16 Gid);

/*
**
** Function:        Init_Coder()
**
** Description:     Initializes non-zero state variables
**          for the coder.
**
** Links to text:   Section 2.21
** 
** Arguments:       None
**
** Outputs:     None
** 
** Return value:    None
**
*/
void Init_Coder(void)
{
	int i;

	/* Initialize encoder data structure with zeros */
	memzero(&CodStat, sizeof(CODSTATDEF));

	/* Initialize the previously decoded LSP vector to the DC vector */
	for (i = 0; i < LpcOrder; i++)
		CodStat.PrevLsp[i] = LspDcTable[i];

	/* Initialize the taming procedure */
	for (i = 0; i < SizErr; i++)
		CodStat.Err[i] = Err0;

	return;
}

/*
**
** Function:        Coder()
**
** Description:     Implements G.723.1 dual-rate coder for    a frame
**          of speech
**
** Links to text:   Section 2
**
** Arguments:
**
**  Word16 DataBuff[]   frame (480 bytes)
**

** Outputs:
**
**  Word16 Vout[]       Encoded frame (20/24 bytes)
**
** Return value:
**
**  Flag            Always True
**
*/
Flag Coder(Word16 * DataBuff, char *Vout)
{
	int i, j;

#ifdef TEST_MIPS
	unsigned int test_temp = 0;
#endif				//TEST_MIPS

	/*
	   Local variables
	 */
	Word16 UnqLpc[SubFrames * LpcOrder];
	Word16 QntLpc[SubFrames * LpcOrder];
	Word16 PerLpc[2 * SubFrames * LpcOrder];

	Word16 LspVect[LpcOrder];
	LINEDEF Line;
	PWDEF Pw[SubFrames];

	Word16 ImpResp[SubFrLen];

	Word16 *Dpnt;

	Word16 Ftyp = 1;

	/*
	   Coder Start
	 */
	Line.Crc = (Word16) 0;

	//High-pass filtering
	//------------------------------------------------------------(1)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Rem_Dc(DataBuff);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[1])
		mips_test[1] = test_temp;
#endif				//TEST_MIPS

	/* Compute the Unquantized Lpc set for whole frame */
	//------------------------------------------------------------(2)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Comp_Lpc(UnqLpc, CodStat.PrevDat, DataBuff);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[2])
		mips_test[2] = test_temp;
#endif				//TEST_MIPS

	/* Convert to Lsp */
	//------------------------------------------------------------(3)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	AtoLsp(LspVect, &UnqLpc[LpcOrder * (SubFrames - 1)], CodStat.PrevLsp);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[3])
		mips_test[3] = test_temp;
#endif				//TEST_MIPS

	/* Compute the Vad */
/* Convert to Lsp */
	//------------------------------------------------------------(4)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Ftyp = (Word16) Comp_Vad(DataBuff);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[4])
		mips_test[4] = test_temp;
#endif				//TEST_MIPS

	/* VQ Lsp vector */
	//------------------------------------------------------------(5)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Line.LspId = Lsp_Qnt(LspVect, CodStat.PrevLsp);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[5])
		mips_test[5] = test_temp;
#endif				//TEST_MIPS

	//------------------------------------------------------------(6)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Mem_Shift(CodStat.PrevDat, DataBuff);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[6])
		mips_test[6] = test_temp;
#endif				//TEST_MIPS

	/* Compute Perceptual filter Lpc coefficients */
	//------------------------------------------------------------(7)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Wght_Lpc(PerLpc, UnqLpc);
#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[7])
		mips_test[7] = test_temp;
#endif				//TEST_MIPS

	/* Apply the perceptual weighting filter */
	//------------------------------------------------------------(8)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Error_Wght(DataBuff, PerLpc);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[8])
		mips_test[8] = test_temp;
#endif				//TEST_MIPS

	/*
	   // Compute Open loop pitch estimates
	 */
	//------------------------------------------------------------(9)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Dpnt = (Word16 *) malloc(sizeof(Word16) * (PitchMax + Frame));

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[9])
		mips_test[9] = test_temp;
#endif				//TEST_MIPS

	/* Construct the buffer */
	//------------------------------------------------------------(10)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	for (i = 0; i < PitchMax; i++)
		Dpnt[i] = CodStat.PrevWgt[i];
	for (i = 0; i < Frame; i++)
		Dpnt[PitchMax + i] = DataBuff[i];

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[10])
		mips_test[10] = test_temp;
#endif				//TEST_MIPS

	//------------------------------------------------------------(11)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Vec_Norm(Dpnt, (Word16) (PitchMax + Frame));

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[11])
		mips_test[11] = test_temp;
#endif				//TEST_MIPS

	j = PitchMax;

	//------------------------------------------------------------(12)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	for (i = 0; i < SubFrames / 2; i++) {
		Line.Olp[i] = Estim_Pitch(Dpnt, (Word16) j);
		VadStat.Polp[i + 2] = Line.Olp[i];
		j += 2 * SubFrLen;
	}

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[12])
		mips_test[12] = test_temp;
#endif				//TEST_MIPS

	if (Ftyp != 1) {

		/*
		   // Case of inactive signal
		 */

		//------------------------------------------------------------(13)
#ifdef TEST_MIPS
		test_temp = 0;
		test_start = cycles();
#endif				//TEST_MIPS

		free((char *)Dpnt);

		/* Save PrevWgt */
		for (i = 0; i < PitchMax; i++)
			CodStat.PrevWgt[i] = DataBuff[i + Frame - PitchMax];

		/* CodCng => Ftyp = 0 (untransmitted) or 2 (SID) */
		Cod_Cng(DataBuff, &Ftyp, &Line, QntLpc);

		/* Update the ringing delays */
		Dpnt = DataBuff;
		for (i = 0; i < SubFrames; i++) {

			/* Update exc_err */
			Update_Err(Line.Olp[i >> 1], Line.Sfs[i].AcLg,
				   Line.Sfs[i].AcGn);

			Upd_Ring(Dpnt, &QntLpc[i * LpcOrder],
				 &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr);
			Dpnt += SubFrLen;
		}

#ifdef TEST_MIPS
		test_temp = cycles() - test_start;
		if (test_temp > mips_test[13])
			mips_test[13] = test_temp;
#endif				//TEST_MIPS

	}

	else {

		/*
		   // Case of Active signal  (Ftyp=1)
		 */
		/* Compute the Hmw */
		//------------------------------------------------------------(14)
#ifdef TEST_MIPS
		test_start = cycles();
#endif				//TEST_MIPS

		j = PitchMax;
		for (i = 0; i < SubFrames; i++) {
			Pw[i] = Comp_Pw(Dpnt, (Word16) j, Line.Olp[i >> 1]);
			j += SubFrLen;
		}

		/* Reload the buffer */
		for (i = 0; i < PitchMax; i++)
			Dpnt[i] = CodStat.PrevWgt[i];
		for (i = 0; i < Frame; i++)
			Dpnt[PitchMax + i] = DataBuff[i];

		/* Save PrevWgt */
		for (i = 0; i < PitchMax; i++)
			CodStat.PrevWgt[i] = Dpnt[Frame + i];

		/* Apply the Harmonic filter */
		j = 0;
		for (i = 0; i < SubFrames; i++) {
			Filt_Pw(DataBuff, Dpnt, (Word16) j, Pw[i]);
			j += SubFrLen;
		}
		free((char *)Dpnt);

		/* Inverse quantization of the LSP */
		Lsp_Inq(LspVect, CodStat.PrevLsp, Line.LspId, Line.Crc);

		/* Interpolate the Lsp vectors */
		Lsp_Int(QntLpc, LspVect, CodStat.PrevLsp);

		/* Copy the LSP vector for the next frame */
		for (i = 0; i < LpcOrder; i++)
			CodStat.PrevLsp[i] = LspVect[i];

#ifdef TEST_MIPS
		test_temp = cycles() - test_start;
		if (test_temp > mips_test[14])
			mips_test[14] = test_temp;
#endif				//TEST_MIPS

		/*
		   // Start the sub frame processing loop
		 */
		Dpnt = DataBuff;

		for (i = 0; i < SubFrames; i++) {

			/* Compute full impulse response */
			//------------------------------------------------------------(15)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Comp_Ir(ImpResp, &QntLpc[i * LpcOrder],
				&PerLpc[i * 2 * LpcOrder], Pw[i]);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[15])
				mips_test[15] = test_temp;
#endif				//TEST_MIPS

			/* Subtract the ringing of previous sub-frame */

			//------------------------------------------------------------(16)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Sub_Ring(Dpnt, &QntLpc[i * LpcOrder],
				 &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr,
				 Pw[i]);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[16])
				mips_test[16] = test_temp;
#endif				//TEST_MIPS

			/* Compute adaptive code book contribution */

			//------------------------------------------------------------(17)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Find_Acbk(Dpnt, ImpResp, CodStat.PrevExc, &Line,
				  (Word16) i);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[17])
				mips_test[17] = test_temp;
#endif				//TEST_MIPS

			/* Compute fixed code book contribution */
			//------------------------------------------------------------(18)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Find_Fcbk(Dpnt, ImpResp, &Line, (Word16) i);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[18])
				mips_test[18] = test_temp;
#endif				//TEST_MIPS

			/* Reconstruct the excitation */
			//------------------------------------------------------------(19)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Decod_Acbk(ImpResp, CodStat.PrevExc, Line.Olp[i >> 1],
				   Line.Sfs[i].AcLg, Line.Sfs[i].AcGn);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[19])
				mips_test[19] = test_temp;
#endif				//TEST_MIPS

			//------------------------------------------------------------(20)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			for (j = SubFrLen; j < PitchMax; j++)
				CodStat.PrevExc[j - SubFrLen] =
				    CodStat.PrevExc[j];

			for (j = 0; j < SubFrLen; j++) {
				Dpnt[j] = g723_shl(Dpnt[j], (Word16) 1);
				Dpnt[j] = g723_add(Dpnt[j], ImpResp[j]);
				CodStat.PrevExc[PitchMax - SubFrLen + j] =
				    Dpnt[j];
			}

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[20])
				mips_test[20] = test_temp;
#endif				//TEST_MIPS

			/* Update exc_err */
			//------------------------------------------------------------(21)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Update_Err(Line.Olp[i >> 1], Line.Sfs[i].AcLg,
				   Line.Sfs[i].AcGn);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[21])
				mips_test[21] = test_temp;
#endif				//TEST_MIPS

			/* Update the ringing delays */

			//------------------------------------------------------------(22)
#ifdef TEST_MIPS
			test_temp = 0;
			test_start = cycles();
#endif				//TEST_MIPS

			Upd_Ring(Dpnt, &QntLpc[i * LpcOrder],
				 &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[22])
				mips_test[22] = test_temp;
#endif				//TEST_MIPS

			Dpnt += SubFrLen;
		}		/* end of subframes loop */

		/*
		   // Save Vad information and reset CNG random generator
		 */
		CodCng.PastFtyp = 1;
		CodCng.RandSeed = 12345;

	}			/* End of active frame case */

	/* Pack the Line structure */
	//------------------------------------------------------------(23)
#ifdef TEST_MIPS
	test_temp = 0;
	test_start = cycles();
#endif				//TEST_MIPS

	Line_Pack(&Line, Vout, Ftyp);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[23])
		mips_test[23] = test_temp;
#endif				//TEST_MIPS

	return (Flag) True;
}
