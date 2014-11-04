/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __SP_FRM
#define __SP_FRM

#include "typedefs.h"
#include "sp_rom.h"

struct QuantList {
	/* structure which points to the beginning of a block of candidate vq
	 * vectors.  It also stores the residual error for each vector. */
	int iNum;		/* total number in list */
	int iRCIndex;		/* an index to the first vector of the
				 * block */
	int16_t pswPredErr[PREQ1_NUM_OF_ROWS];	/* PREQ1 is the biggest block */
};

/*_________________________________________________________________________
 |                                                                         |
 |                            Function Prototypes                          |
 |_________________________________________________________________________|
*/

void iir_d(int16_t pswCoeff[], int16_t pswIn[],
	   int16_t pswXstate[],
	   int16_t pswYstate[],
	   int npts, int shifts,
	   int16_t swPreFirDownSh, int16_t swFinalUpShift);

void filt4_2nd(int16_t pswCoeff[],
	       int16_t pswIn[],
	       int16_t pswXstate[],
	       int16_t pswYstate[], int npts, int shifts);

void initPBarVBarL(int32_t pL_PBarFull[],
		   int16_t pswPBar[], int16_t pswVBar[]);

void initPBarFullVBarFullL(int32_t pL_CorrelSeq[],
			   int32_t pL_PBarFull[], int32_t pL_VBarFull[]);

int16_t aflatRecursion(int16_t pswQntRc[],
			 int16_t pswPBar[],
			 int16_t pswVBar[],
			 int16_t * ppswPAddrs[],
			 int16_t * ppswVAddrs[], int16_t swSegmentOrder);

void aflatNewBarRecursionL(int16_t pswQntRc[],
			   int iSegment,
			   int32_t pL_PBar[],
			   int32_t pL_VBar[],
			   int16_t pswPBar[], int16_t pswVBar[]);

void setupPreQ(int iSeg, int iVector);

void setupQuant(int iSeg, int iVector);

void getNextVec(int16_t pswRc[]);

void aflat(int16_t pswSpeechToLPC[],
	   int piR0Index[],
	   int16_t pswFinalRc[],
	   int piVQCodewds[],
	   int16_t swPtch, int16_t * pswVadFlag, int16_t * pswSP);

int16_t fnExp2(int32_t L_Input);

int16_t fnLog2(int32_t L_Input);

void weightSpeechFrame(int16_t pswSpeechFrm[],
		       int16_t pswWNumSpace[],
		       int16_t pswWDenomSpace[],
		       int16_t pswWSpeechBuffBase[]);

void getSfrmLpcTx(int16_t swPrevR0, int16_t swNewR0,
		  int16_t pswPrevFrmKs[],
		  int16_t pswPrevFrmAs[],
		  int16_t pswPrevFrmSNWCoef[],
		  int16_t pswNewFrmKs[],
		  int16_t pswNewFrmAs[],
		  int16_t pswNewFrmSNWCoef[],
		  int16_t pswHPFSpeech[],
		  short *pswSoftInterp,
		  struct NormSw *psnsSqrtRs,
		  int16_t ppswSynthAs[][NP], int16_t ppswSNWCoefAs[][NP]);

short int fnBest_CG(int16_t pswCframe[],
		    int16_t pswGframe[],
		    int16_t * pswCmaxSqr,
		    int16_t * pswGmax, short int siNumPairs);

short compResidEnergy(int16_t pswSpeech[],
		      int16_t ppswInterpCoef[][NP],
		      int16_t pswPreviousCoef[],
		      int16_t pswCurrentCoef[], struct NormSw psnsSqrtRs[]);

int16_t r0Quant(int32_t L_UnqntzdR0);

int16_t cov32(int16_t pswIn[],
		int32_t pppL_B[NP][NP][2],
		int32_t pppL_F[NP][NP][2],
		int32_t pppL_C[NP][NP][2],
		int32_t * pL_R0,
		int32_t pL_VadAcf[], int16_t * pswVadScalAuto);

int32_t flat(int16_t pswSpeechIn[],
	      int16_t pswRc[],
	      int *piR0Inx, int32_t pL_VadAcf[], int16_t * pswVadScalAuto);

void openLoopLagSearch(int16_t pswWSpeech[],
		       int16_t swPrevR0Index,
		       int16_t swCurrR0Index,
		       int16_t * psiUVCode,
		       int16_t pswLagList[],
		       int16_t pswNumLagList[],
		       int16_t pswPitchBuf[],
		       int16_t pswHNWCoefBuf[],
		       struct NormSw psnsWSfrmEng[],
		       int16_t pswVadLags[], int16_t swSP);

int16_t getCCThreshold(int16_t swRp0, int16_t swCC, int16_t swG);

void pitchLags(int16_t swBestIntLag,
	       int16_t pswIntCs[],
	       int16_t pswIntGs[],
	       int16_t swCCThreshold,
	       int16_t pswLPeaksSorted[],
	       int16_t pswCPeaksSorted[],
	       int16_t pswGPeaksSorted[],
	       int16_t * psiNumSorted,
	       int16_t * pswPitch, int16_t * pswHNWCoef);

short CGInterpValid(int16_t swFullResLag,
		    int16_t pswCIn[],
		    int16_t pswGIn[],
		    int16_t pswLOut[],
		    int16_t pswCOut[], int16_t pswGOut[]);

void CGInterp(int16_t pswLIn[],
	      short siNum,
	      int16_t pswCIn[],
	      int16_t pswGIn[],
	      short siLoIntLag, int16_t pswCOut[], int16_t pswGOut[]);

int16_t quantLag(int16_t swRawLag, int16_t * psiCode);

void findBestInQuantList(struct QuantList psqlInList,
			 int iNumVectOut, struct QuantList psqlBestOutList[]);

int16_t findPeak(int16_t swSingleResLag,
		   int16_t pswCIn[], int16_t pswGIn[]);

void bestDelta(int16_t pswLagList[],
	       int16_t pswCSfrm[],
	       int16_t pswGSfrm[],
	       short int siNumLags,
	       short int siSfrmIndex,
	       int16_t pswLTraj[],
	       int16_t pswCCTraj[], int16_t pswGTraj[]);

int16_t
maxCCOverGWithSign(int16_t pswCIn[],
		   int16_t pswGIn[],
		   int16_t * pswCCMax, int16_t * pswGMax, int16_t swNum);

void getNWCoefs(int16_t pswACoefs[], int16_t pswHCoefs[]);

#endif
