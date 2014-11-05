/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __ERR_CONC
#define __ERR_CONC

#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                            Function Prototypes                          |
 |_________________________________________________________________________|
*/

void para_conceal_speech_decoder(int16_t pswErrorFlag[],
				 int16_t pswSpeechPara[],
				 int16_t * pswMutePermit);

int16_t level_calc(int16_t swInd, int32_t * pl_en);

void level_estimator(int16_t update, int16_t * pswLevelMean,
		     int16_t * pswLevelMax,
		     int16_t pswDecodedSpeechFrame[]);

void signal_conceal_sub(int16_t pswPPFExcit[],
			int16_t ppswSynthAs[], int16_t pswSynthFiltState[],
			int16_t pswLtpStateOut[], int16_t pswPPreState[],
			int16_t swLevelMean, int16_t swLevelMax,
			int16_t swErrorFlag1, int16_t swMuteFlagOld,
			int16_t * pswMuteFlag, int16_t swMutePermit);

#endif
