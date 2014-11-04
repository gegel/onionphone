/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __VAD
#define __VAD

#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                            Function Prototypes                          |
 |_________________________________________________________________________|
*/

void vad_reset(void);

void vad_algorithm
    (int32_t pL_acf[9],
     int16_t swScaleAcf,
     int16_t pswRc[4], int16_t swPtch, int16_t * pswVadFlag);

void energy_computation
    (int32_t pL_acf[],
     int16_t swScaleAcf,
     int16_t pswRvad[],
     int16_t swNormRvad,
     int16_t * pswM_pvad,
     int16_t * pswE_pvad, int16_t * pswM_acf0, int16_t * pswE_acf0);

void average_acf
    (int32_t pL_acf[],
     int16_t swScaleAcf, int32_t pL_av0[], int32_t pL_av1[]
    );

void predictor_values
    (int32_t pL_av1[], int16_t pswRav1[], int16_t * pswNormRav1);

void schur_recursion(int32_t pL_av1[], int16_t pswVpar[]
    );

void step_up(int16_t swNp, int16_t pswVpar[], int16_t pswAav1[]
    );

void compute_rav1
    (int16_t pswAav1[], int16_t pswRav1[], int16_t * pswNormRav1);

void spectral_comparison
    (int16_t pswRav1[],
     int16_t swNormRav1, int32_t pL_av0[], int16_t * pswStat);

void tone_detection(int16_t pswRc[4], int16_t * pswTone);

void threshold_adaptation
    (int16_t swStat,
     int16_t swPtch,
     int16_t swTone,
     int16_t pswRav1[],
     int16_t swNormRav1,
     int16_t swM_pvad,
     int16_t swE_pvad,
     int16_t swM_acf0,
     int16_t swE_acf0,
     int16_t pswRvad[],
     int16_t * pswNormRvad, int16_t * pswM_thvad, int16_t * pswE_thvad);

void vad_decision
    (int16_t swM_pvad,
     int16_t swE_pvad,
     int16_t swM_thvad, int16_t swE_thvad, int16_t * pswVvad);

void vad_hangover(int16_t swVvad, int16_t * pswVadFlag);

void periodicity_update(int16_t pswLags[4], int16_t * pswPtch);

#endif
