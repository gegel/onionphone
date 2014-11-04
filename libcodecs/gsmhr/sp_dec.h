/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __SP_DEC
#define __SP_DEC

#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                            Function Prototypes                          |
 |_________________________________________________________________________|
*/

void speechDecoder(int16_t pswParameters[],
		   int16_t pswDecodedSpeechFrame[]);

void aFlatRcDp(int32_t * pL_R, int16_t * pswRc);

void b_con(int16_t swCodeWord, short siNumBits, int16_t pswVectOut[]);

void fp_ex(int16_t swOrigLagIn, int16_t pswLTPState[]);

int16_t g_corr1(int16_t * pswIn, int32_t * pL_out);

int16_t g_corr1s(int16_t pswIn[], int16_t swEngyRShft, int32_t * pL_out);

void getSfrmLpc(short int siSoftInterpolation,
		int16_t swPrevR0, int16_t swNewR0,
		int16_t pswPrevFrmKs[],
		int16_t pswPrevFrmAs[],
		int16_t pswPrevFrmPFNum[],
		int16_t pswPrevFrmPFDenom[],
		int16_t pswNewFrmKs[],
		int16_t pswNewFrmAs[],
		int16_t pswNewFrmPFNum[],
		int16_t pswNewFrmPFDenom[],
		struct NormSw *psnsSqrtRs,
		int16_t * ppswSynthAs[],
		int16_t * ppswPFNumAs[], int16_t * ppswPFDenomAs[]);

void get_ipjj(int16_t swLagIn, int16_t * pswIp, int16_t * pswJj);

short int interpolateCheck(int16_t pswRefKs[],
			   int16_t pswRefCoefsA[],
			   int16_t pswOldCoefsA[],
			   int16_t pswNewCoefsA[],
			   int16_t swOldPer,
			   int16_t swNewPer,
			   int16_t swRq,
			   struct NormSw *psnsSqrtRsOut,
			   int16_t pswCoefOutA[]);

void lpcFir(int16_t pswInput[], int16_t pswCoef[],
	    int16_t pswState[], int16_t pswFiltOut[]);

void lpcIir(int16_t pswInput[], int16_t pswCoef[],
	    int16_t pswState[], int16_t pswFiltOut[]);

void lpcIrZsIir(int16_t pswCoef[], int16_t pswFiltOut[]);

void lpcZiIir(int16_t pswCoef[], int16_t pswState[],
	      int16_t pswFiltOut[]);

void lpcZsFir(int16_t pswInput[], int16_t pswCoef[],
	      int16_t pswFiltOut[]);

void lpcZsIir(int16_t pswInput[], int16_t pswCoef[],
	      int16_t pswFiltOut[]);

void lpcZsIirP(int16_t pswCommonIO[], int16_t pswCoef[]);

int16_t r0BasedEnergyShft(int16_t swR0Index);

short rcToADp(int16_t swAscale, int16_t pswRc[], int16_t pswA[]);

void rcToCorrDpL(int16_t swAshift, int16_t swAscale,
		 int16_t pswRc[], int32_t pL_R[]);

void res_eng(int16_t pswReflecCoefIn[], int16_t swRq,
	     struct NormSw *psnsSqrtRsOut);

void rs_rr(int16_t pswExcitation[], struct NormSw snsSqrtRs,
	   struct NormSw *snsSqrtRsRr);

void rs_rrNs(int16_t pswExcitation[], struct NormSw snsSqrtRs,
	     struct NormSw *snsSqrtRsRr);

int16_t scaleExcite(int16_t pswVect[],
		      int16_t swErrTerm, struct NormSw snsRS,
		      int16_t pswScldVect[]);

int16_t sqroot(int32_t L_SqrtIn);

void v_con(int16_t pswBVects[], int16_t pswOutVect[],
	   int16_t pswBitArray[], short int siNumBVctrs);

#endif
