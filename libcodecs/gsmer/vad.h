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
	Word16 e;		/* exponent */
	Word16 m;		/* mantissa */
};

typedef struct _fp Pfloat;

void w_er_vad_reset(void);

Word16 w_vad_computation(Word16 r_h[],
			 Word16 r_l[],
			 Word16 scal_acf, Word16 rc[], Word16 w_ptch);

void w_er_energy_computation(Word16 r_h[],
			     Word16 scal_acf,
			     Word16 w_rvad[],
			     Word16 scal_w_rvad, Pfloat * acf0, Pfloat * pvad);

void w_acf_averaging(Word16 r_h[],
		     Word16 r_l[],
		     Word16 scal_acf, Word32 L_av0[], Word32 L_av1[]
    );

void w_er_w_predictor_values(Word32 L_av1[], Word16 rav1[], Word16 * scal_rav1);

void w_er_schur_recursion(Word32 L_av1[], Word16 vpar[]
    );

void w_er_step_up(Word16 np, Word16 vpar[], Word16 aav1[]
    );

void w_er_compute_rav1(Word16 aav1[], Word16 rav1[], Word16 * scal_rav1);

Word16 w_er_spectral_comparison(Word16 rav1[], Word16 scal_rav1, Word32 L_av0[]
    );

void w_er_threshold_adaptation(Word16 stat,
			       Word16 w_ptch,
			       Word16 tone,
			       Word16 rav1[],
			       Word16 scal_rav1,
			       Pfloat pvad,
			       Pfloat acf0,
			       Word16 w_rvad[],
			       Word16 * scal_w_rvad, Pfloat * w_thvad);

void w_er_tone_detection(Word16 rc[], Word16 * tone);

Word16 w_er_vad_decision(Pfloat pvad, Pfloat w_thvad);

Word16 w_er_vad_hangover(Word16 vvad);

void w_er_periodicity_update(Word16 lags[], Word16 * w_ptch);
