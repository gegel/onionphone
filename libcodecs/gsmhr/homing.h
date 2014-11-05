/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __HOMING
#define __HOMING

#include "typedefs.h"

#define EHF_MASK 0x0008		/* Encoder Homing Frame pattern */

/*_________________________________________________________________________
 |                                                                         |
 |                           Function Prototypes                           |
 |_________________________________________________________________________|
*/

int decoderHomingFrameTest(int16_t pswSpeechPara[], int iLastPara);

void decoderReset(void);

int encoderHomingFrameTest(int16_t pswSpeech[]);

void encoderReset(void);

void resetDec(void);

void resetEnc(void);

void dtxResetTx(void);

void dtxResetRx(void);

#endif
