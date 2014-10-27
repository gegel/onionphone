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
extern int16_t LspDcTable[LpcOrder];
extern VADSTATDEF VadStat;
extern CODCNGDEF CodCng;

extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
extern int16_t g723_shl(int16_t var1, int16_t var2);	/* Short shift left,    1 */
extern void Rem_Dc(int16_t * Dpnt);
extern void Comp_Lpc(int16_t * UnqLpc, int16_t * PrevDat, int16_t * DataBuff);
extern void Find_Fcbk(int16_t * Dpnt, int16_t * ImpResp, LINEDEF * Line,
		      int16_t Sfc);
extern PWDEF Comp_Pw(int16_t * Dpnt, int16_t Start, int16_t Olp);
extern void AtoLsp(int16_t * LspVect, int16_t * Lpc, int16_t * PrevLsp);
extern int Comp_Vad(int16_t * Dpnt);
extern int32_t Lsp_Qnt(int16_t * CurrLsp, int16_t * PrevLsp);
extern void Mem_Shift(int16_t * PrevDat, int16_t * DataBuff);
extern void Wght_Lpc(int16_t * PerLpc, int16_t * UnqLpc);
extern void Error_Wght(int16_t * Dpnt, int16_t * PerLpc);
extern int16_t Vec_Norm(int16_t * Vect, int16_t Len);
extern int16_t Estim_Pitch(int16_t * Dpnt, int16_t Start);

extern void Init_Cod_Cng(void);
extern void Cod_Cng(int16_t * DataExc, int16_t * Ftyp, LINEDEF * Line,
		    int16_t * QntLpc);
extern void Filt_Pw(int16_t * DataBuff, int16_t * Dpnt, int16_t Start,
		    PWDEF Pw);
extern void Lsp_Inq(int16_t * Lsp, int16_t * PrevLsp, int32_t LspId,
		    int16_t Crc);
extern void Lsp_Int(int16_t * QntLpc, int16_t * CurrLsp, int16_t * PrevLsp);
extern void Comp_Ir(int16_t * ImpResp, int16_t * QntLpc, int16_t * PerLpc,
		    PWDEF Pw);
extern void Update_Err(int16_t Olp, int16_t AcLg, int16_t AcGn);
extern void Upd_Ring(int16_t * Dpnt, int16_t * QntLpc, int16_t * PerLpc,
		     int16_t * PrevErr);
extern void Sub_Ring(int16_t * Dpnt, int16_t * QntLpc, int16_t * PerLpc,
		     int16_t * PrevErr, PWDEF Pw);
extern void Find_Acbk(int16_t * Tv, int16_t * ImpResp, int16_t * PrevExc,
		      LINEDEF * Line, int16_t Sfc);
extern void Line_Pack(LINEDEF * Line, char *Vout, int16_t Ftyp);
extern void Decod_Acbk(int16_t * Tv, int16_t * PrevExc, int16_t Olp,
		       int16_t Lid,
		       int16_t Gid);

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
**  int16_t DataBuff[]   frame (480 bytes)
**

** Outputs:
**
**  int16_t Vout[]       Encoded frame (20/24 bytes)
**
** Return value:
**
**  int            Always True
**
*/

#include <stdint.h>

int Coder(int16_t * DataBuff, char *Vout)
{
	int i, j;

#ifdef TEST_MIPS
	unsigned int test_temp;
#endif				//TEST_MIPS

	/*
	   Local variables
	 */
	int16_t UnqLpc[SubFrames * LpcOrder];
	int16_t QntLpc[SubFrames * LpcOrder];
	int16_t PerLpc[2 * SubFrames * LpcOrder];

	int16_t LspVect[LpcOrder];
	LINEDEF Line;
	PWDEF Pw[SubFrames];

	int16_t ImpResp[SubFrLen];

	int16_t *Dpnt;

	int16_t Ftyp;

	/*
	   Coder Start
	 */
	Line.Crc = (int16_t) 0;

	//High-pass filtering
	//------------------------------------------------------------(1)
#ifdef TEST_MIPS
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
	test_start = cycles();
#endif				//TEST_MIPS

	Ftyp = (int16_t) Comp_Vad(DataBuff);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[4])
		mips_test[4] = test_temp;
#endif				//TEST_MIPS

	/* VQ Lsp vector */
	//------------------------------------------------------------(5)
#ifdef TEST_MIPS
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
	test_start = cycles();
#endif				//TEST_MIPS

	Dpnt = (int16_t *) malloc(sizeof(int16_t) * (PitchMax + Frame));

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[9])
		mips_test[9] = test_temp;
#endif				//TEST_MIPS

	/* Construct the buffer */
	//------------------------------------------------------------(10)
#ifdef TEST_MIPS
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
	test_start = cycles();
#endif				//TEST_MIPS

	Vec_Norm(Dpnt, (int16_t) (PitchMax + Frame));

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[11])
		mips_test[11] = test_temp;
#endif				//TEST_MIPS

	j = PitchMax;

	//------------------------------------------------------------(12)
#ifdef TEST_MIPS
	test_start = cycles();
#endif				//TEST_MIPS

	for (i = 0; i < SubFrames / 2; i++) {
		Line.Olp[i] = Estim_Pitch(Dpnt, (int16_t) j);
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
			Pw[i] = Comp_Pw(Dpnt, (int16_t) j, Line.Olp[i >> 1]);
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
			Filt_Pw(DataBuff, Dpnt, (int16_t) j, Pw[i]);
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
			test_start = cycles();
#endif				//TEST_MIPS

			Find_Acbk(Dpnt, ImpResp, CodStat.PrevExc, &Line,
				  (int16_t) i);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[17])
				mips_test[17] = test_temp;
#endif				//TEST_MIPS

			/* Compute fixed code book contribution */
			//------------------------------------------------------------(18)
#ifdef TEST_MIPS
			test_start = cycles();
#endif				//TEST_MIPS

			Find_Fcbk(Dpnt, ImpResp, &Line, (int16_t) i);

#ifdef TEST_MIPS
			test_temp = cycles() - test_start;
			if (test_temp > mips_test[18])
				mips_test[18] = test_temp;
#endif				//TEST_MIPS

			/* Reconstruct the excitation */
			//------------------------------------------------------------(19)
#ifdef TEST_MIPS
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
			test_start = cycles();
#endif				//TEST_MIPS

			for (j = SubFrLen; j < PitchMax; j++)
				CodStat.PrevExc[j - SubFrLen] =
				    CodStat.PrevExc[j];

			for (j = 0; j < SubFrLen; j++) {
				Dpnt[j] = g723_shl(Dpnt[j], (int16_t) 1);
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
	test_start = cycles();
#endif				//TEST_MIPS

	Line_Pack(&Line, Vout, Ftyp);

#ifdef TEST_MIPS
	test_temp = cycles() - test_start;
	if (test_temp > mips_test[23])
		mips_test[23] = test_temp;
#endif				//TEST_MIPS

	return (int) True;
}
