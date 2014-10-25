/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name: vad.h
 *
 *   Purpose:   Contains the prototypes for all functions of voice activity
 *              detection. Also contains the type definition for the pseudo
 *              floating point data type.
 *
 **************************************************************************/

/* Struct for storing pseudo floating point exponent and mantissa */
struct _fp {
	int16_t e;		/* exponent */
	int16_t m;		/* mantissa */
};

typedef struct _fp Pfloat;

void w_er_vad_reset(void);

int16_t w_vad_computation(int16_t r_h[],
			 int16_t r_l[],
			 int16_t scal_acf, int16_t rc[], int16_t w_ptch);

void w_er_energy_computation(int16_t r_h[],
			     int16_t scal_acf,
			     int16_t w_rvad[],
			     int16_t scal_w_rvad, Pfloat * acf0,
			     Pfloat * pvad);

void w_acf_averaging(int16_t r_h[],
		     int16_t r_l[],
		     int16_t scal_acf, int32_t L_av0[], int32_t L_av1[]
    );

void w_er_w_predictor_values(int32_t L_av1[], int16_t rav1[],
			     int16_t * scal_rav1);

void w_er_schur_recursion(int32_t L_av1[], int16_t vpar[]
    );

void w_er_step_up(int16_t np, int16_t vpar[], int16_t aav1[]
    );

void w_er_compute_rav1(int16_t aav1[], int16_t rav1[], int16_t * scal_rav1);

int16_t w_er_spectral_comparison(int16_t rav1[], int16_t scal_rav1, int32_t L_av0[]
    );

void w_er_threshold_adaptation(int16_t stat,
			       int16_t w_ptch,
			       int16_t tone,
			       int16_t rav1[],
			       int16_t scal_rav1,
			       Pfloat pvad,
			       Pfloat acf0,
			       int16_t w_rvad[],
			       int16_t * scal_w_rvad, Pfloat * w_thvad);

void w_er_tone_detection(int16_t rc[], int16_t * tone);

int16_t w_er_vad_decision(Pfloat pvad, Pfloat w_thvad);

int16_t w_er_vad_hangover(int16_t vvad);

void w_er_periodicity_update(int16_t lags[], int16_t * w_ptch);
