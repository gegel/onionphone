/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name:  sp_enc.c
 *
 *   Purpose:  Contains speech encoder function.  Calls are made to the
 *      frame-based encoding functions (see sp_frm.c), and the subframe-
 *      based encoding function (see sp_sfrm.c)
 *
 *      Functions in this file (only 1)
 *      speechEncoder()
 *
 **************************************************************************/
/*_________________________________________________________________________
 |                                                                         |
 |                              Include Files                              |
 |_________________________________________________________________________|
*/

#include "mathhalf.h"
#include "mathdp31.h"
#include "sp_rom.h"
#include "sp_dec.h"
#include "sp_frm.h"
#include "sp_sfrm.h"
#include "sp_enc.h"
#include "host.h"
#include "vad.h"

/*_________________________________________________________________________
 |                                                                         |
 |                              Local Defines                              |
 |_________________________________________________________________________|
*/

#define CG_INT_MACS       6	/* Number of Multiply-Accumulates in */
				       /* one interpolation                 */
#define ASCALE       0x0800
#define LMAX            142	/* largest lag (integer sense) */
#define LSMAX           (LMAX+ CG_INT_MACS/2)	/* Lag Search Array Length */
#define NUM_CLOSED        3	/* Maximum number of lags searched */
				      /* in closed loop.                 */

#define LPCSTARTINDEX     25	/* Where the LPC analysis window
				 * starts */
#define INBUFFSZ          LPCSTARTINDEX + A_LEN	/* Input buffer size  */
#define NUMSTARTUPSMP     INBUFFSZ - F_LEN	/* Number of samples needed */
						/* at start up              */
#define NUMSTARTUPSMP_P1  INBUFFSZ - F_LEN + 1
#define HPFSHIFT          1	/* no right shifts high pass shifts
				 * speech */

/*_________________________________________________________________________
 |                                                                         |
 |                     State variables (globals)                           |
 |_________________________________________________________________________|
*/

int16_t swOldR0;
int16_t swOldR0Index;

struct NormSw psnsWSfrmEngSpace[2 * N_SUB];

int16_t pswHPFXState[4];
int16_t pswHPFYState[8];
int16_t pswOldFrmKs[NP];
int16_t pswOldFrmAs[NP];
int16_t pswOldFrmSNWCoefs[NP];
int16_t pswWgtSpeechSpace[F_LEN + LMAX + CG_INT_MACS / 2];

int16_t pswSpeech[INBUFFSZ];	/* input speech */

int16_t swPtch = 1;

/*_________________________________________________________________________
 |                                                                         |
 |                         Global DTX variables                            |
 |_________________________________________________________________________|
*/

int16_t swTxGsHistPtr = 0;

int16_t pswCNVSCode1[N_SUB],
    pswCNVSCode2[N_SUB], pswCNGsp0Code[N_SUB], pswCNLpc[3], swCNR0;

extern int32_t pL_GsHist[];
extern int32_t ppLr_gsTable[4][32];

/***************************************************************************
 *
 *   FUNCTION NAME: speechEncoder
 *
 *   PURPOSE:
 *
 *     Performs GSM half-rate speech encoding on frame basis (160 samples).
 *
 *   INPUTS:
 *
 *     pswSpeechIn[0:159] - input speech samples, 160 new samples per frame
 *
 *   OUTPUTS:
 *
 *     pswFrmCodes[0:19] - output parameters, 18 speech parameters plus
 *                         VAD and SP flags
 *
 *   RETURN VALUE:
 *
 *     None
 *
 *   IMPLEMENTATION:
 *
 *     n/a
 *
 *   REFERENCES: Sub-clause 4.1 of GSM Recomendation 06.20
 *
 *   KEYWORDS: speechcoder, analysis
 *
 *************************************************************************/

void speechEncoder(int16_t pswSpeechIn[], int16_t pswFrmCodes[])
{

/*_________________________________________________________________________
 |                                                                         |
 |                            Static Variables                             |
 |_________________________________________________________________________|
*/

	/* 1st point in analysis window */
	static int16_t *pswLpcStart = &pswSpeech[LPCSTARTINDEX];

	/* 1st point of new frame other than 1st */
	static int16_t *pswNewSpeech = &pswSpeech[NUMSTARTUPSMP];

	/* sample 0 of weighted speech */
	static int16_t *pswWgtSpeech = &pswWgtSpeechSpace[LSMAX];

	static struct NormSw *psnsWSfrmEng = &psnsWSfrmEngSpace[N_SUB];

/*_________________________________________________________________________
 |                                                                         |
 |                            Automatic Variables                          |
 |_________________________________________________________________________|
*/

	int iVoicing,		/* bitAlloc */
	 iR0,			/* bitAlloc and aflat */
	 piVq[3],		/* bitAlloc */
	 iSi,			/* bitAlloc */
	 piLagCode[N_SUB],	/* bitAlloc */
	 piVSCode1[N_SUB],	/* bitAlloc */
	 piVSCode2[N_SUB],	/* bitAlloc */
	 piGsp0Code[N_SUB];	/* bitAlloc */

	short int siUVCode, siSi, i, j;

	int16_t swR0,
	    pswLagCode[N_SUB],
	    pswVSCode1[N_SUB],
	    pswVSCode2[N_SUB],
	    pswGsp0Code[N_SUB],
	    *pswLagListPtr,
	    pswFrmKs[NP],
	    pswFrmAs[NP],
	    pswFrmSNWCoefs[NP],
	    pswLagList[N_SUB * NUM_CLOSED],
	    pswNumLagList[N_SUB],
	    pswPitchBuf[N_SUB],
	    pswHNWCoefBuf[N_SUB],
	    ppswSNWCoefAs[N_SUB][NP], ppswSynthAs[N_SUB][NP];

	int16_t swSP, pswVadLags[4],	/* VAD Parameters */
	 swVadFlag;		/* flag indicating voice activity
				 * detector state.  1 = speech or
				 * speech/signal present */
	struct NormSw psnsSqrtRs[N_SUB];

/*_________________________________________________________________________
 |                                                                         |
 |                              Executable Code                            |
 |_________________________________________________________________________|
*/

	/* Speech frame processing     */
	/* High pass filter the speech */
	/* ---------------------------- */

	filt4_2nd(psrHPFCoefs, pswSpeechIn,
		  pswHPFXState, pswHPFYState, F_LEN, HPFSHIFT);

	/* copy high passed filtered speech into encoder's speech buff */
  /*-------------------------------------------------------------*/

	for (i = 0; i < F_LEN; i++)
		pswNewSpeech[i] = pswSpeechIn[i];

	/* Calculate and quantize LPC coefficients */
	/* --------------------------------------- */

	aflat(pswLpcStart, &iR0, pswFrmKs, piVq, swPtch, &swVadFlag, &swSP);

	/* Lookup frame energy r0 */
	/* ---------------------- */

	swR0 = psrR0DecTbl[iR0 * 2];	/* lookupR0 */

	/* Generate the direct form coefs */
	/* ------------------------------ */

	if (!rcToADp(ASCALE, pswFrmKs, pswFrmAs)) {

		getNWCoefs(pswFrmAs, pswFrmSNWCoefs);
	} else {

		for (i = 0; i < NP; i++) {
			pswFrmKs[i] = pswOldFrmKs[i];
			pswFrmAs[i] = pswOldFrmAs[i];
			pswFrmSNWCoefs[i] = pswOldFrmSNWCoefs[i];
		}
	}

	/* Interpolate, or otherwise get sfrm reflection coefs */
	/* --------------------------------------------------- */

	getSfrmLpcTx(swOldR0, swR0,
		     pswOldFrmKs, pswOldFrmAs,
		     pswOldFrmSNWCoefs,
		     pswFrmKs, pswFrmAs,
		     pswFrmSNWCoefs,
		     pswSpeech, &siSi, psnsSqrtRs, ppswSynthAs, ppswSNWCoefAs);

	/* loose once bitAlloc done */
	iSi = siSi;

	/* Weight the entire speech frame */
	/* ------------------------------ */

	weightSpeechFrame(pswSpeech, ppswSynthAs[0], ppswSNWCoefAs[0],
			  pswWgtSpeechSpace);

	/* Perform open-loop lag search, get harmonic-noise-weighting parameters */
	/* --------------------------------------------------------------------- */

	openLoopLagSearch(&pswWgtSpeechSpace[LSMAX],
			  swOldR0Index,
			  (int16_t) iR0,
			  &siUVCode,
			  pswLagList,
			  pswNumLagList,
			  pswPitchBuf,
			  pswHNWCoefBuf, &psnsWSfrmEng[0], pswVadLags, swSP);

	iVoicing = siUVCode;

	/* Using open loop LTP data to calculate swPtch *//* DTX mode */
	/* parameter                                    *//* DTX mode */
	/* -------------------------------------------- *//* DTX mode */

	periodicity_update(pswVadLags, &swPtch);	/* DTX mode */

	/* Subframe processing loop */
	/* ------------------------ */

	pswLagListPtr = pswLagList;

	for (giSfrmCnt = 0; giSfrmCnt < N_SUB; giSfrmCnt++) {

		if (swSP == 0) {	/* DTX mode *//* DTX mode */
			pswVSCode1[giSfrmCnt] = pswCNVSCode1[giSfrmCnt];	/* DTX mode */
			pswVSCode2[giSfrmCnt] = pswCNVSCode2[giSfrmCnt];	/* DTX mode */
			pswGsp0Code[giSfrmCnt] = pswCNGsp0Code[giSfrmCnt];	/* DTX mode */
		}
		/* DTX mode */
		sfrmAnalysis(&pswWgtSpeech[giSfrmCnt * S_LEN],
			     siUVCode,
			     psnsSqrtRs[giSfrmCnt],
			     ppswSNWCoefAs[giSfrmCnt],
			     pswLagListPtr,
			     pswNumLagList[giSfrmCnt],
			     pswPitchBuf[giSfrmCnt],
			     pswHNWCoefBuf[giSfrmCnt],
			     &pswLagCode[giSfrmCnt], &pswVSCode1[giSfrmCnt],
			     &pswVSCode2[giSfrmCnt], &pswGsp0Code[giSfrmCnt],
			     swSP);

		pswLagListPtr = &pswLagListPtr[pswNumLagList[giSfrmCnt]];

	}

	/* copy comfort noise parameters, *//* DTX mode */
	/* update GS history              *//* DTX mode */
	/* ------------------------------ *//* DTX mode */

	if (swSP == 0) {	/* DTX mode *//* DTX mode */

		/* copy comfort noise frame parameter *//* DTX mode */
		/* ---------------------------------- *//* DTX mode */

		iR0 = swCNR0;	/* quantized R0 index *//* DTX mode */
		for (i = 0; i < 3; i++)	/* DTX mode */
			piVq[i] = pswCNLpc[i];	/* DTX mode */

	} /* DTX mode */
	else {			/* DTX mode */
		/* DTX mode */

		/* if swSP != 0, then update the GS history *//* DTX mode */
		/* ----------------------------------------- *//* DTX mode */

		for (i = 0; i < N_SUB; i++) {	/* DTX mode */
			pL_GsHist[swTxGsHistPtr] =	/* DTX mode */
			    ppLr_gsTable[siUVCode][pswGsp0Code[i]];	/* DTX mode */
			swTxGsHistPtr++;	/* DTX mode */
			if (swTxGsHistPtr > ((OVERHANG - 1) * N_SUB) - 1)	/* DTX mode */
				swTxGsHistPtr = 0;	/* DTX mode */
		}		/* DTX mode */

	}			/* DTX mode */

	/* End of frame processing, update frame based parameters */
	/* ------------------------------------------------------ */

	for (i = 0; i < N_SUB; i++) {
		piLagCode[i] = pswLagCode[i];
		piVSCode1[i] = pswVSCode1[i];
		piVSCode2[i] = pswVSCode2[i];
		piGsp0Code[i] = pswGsp0Code[i];
	}

	swOldR0Index = (int16_t) iR0;
	swOldR0 = swR0;

	for (i = 0; i < NP; i++) {
		pswOldFrmKs[i] = pswFrmKs[i];
		pswOldFrmAs[i] = pswFrmAs[i];
		pswOldFrmSNWCoefs[i] = pswFrmSNWCoefs[i];
	}

	/* Insert SID Codeword *//* DTX mode */
	/* ------------------- *//* DTX mode */

	if (swSP == 0) {	/* DTX mode *//* DTX mode */
		iVoicing = 0x0003;	/* 2 bits *//* DTX mode */
		iSi = 0x0001;	/* 1 bit  *//* DTX mode */
		for (i = 0; i < N_SUB; i++) {	/* DTX mode *//* DTX mode */
			piVSCode1[i] = 0x01ff;	/* 9 bits *//* DTX mode */
			piGsp0Code[i] = 0x001f;	/* 5 bits *//* DTX mode */
		}
		piLagCode[0] = 0x00ff;	/* 8 bits *//* DTX mode */
		piLagCode[1] = 0x000f;	/* 4 bits *//* DTX mode */
		piLagCode[2] = 0x000f;	/* 4 bits *//* DTX mode */
		piLagCode[3] = 0x000f;	/* 4 bits *//* DTX mode */
	}

	/* DTX mode */
	/* Generate encoded parameter array */
	/* -------------------------------- */
	fillBitAlloc(iVoicing, iR0, piVq, iSi, piLagCode,
		     piVSCode1, piVSCode2,
		     piGsp0Code, swVadFlag, swSP, pswFrmCodes);

	/* delay the input speech by 1 frame */
  /*-----------------------------------*/

	for (i = 0, j = F_LEN; j < INBUFFSZ; i++, j++) {
		pswSpeech[i] = pswSpeech[j];
	}
}
