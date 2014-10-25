/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name: dtx.h
 *
 *   Purpose:   Contains the prototypes for all the functions of DTX.
 *              Also contains definitions of constants used in DTX functions.
 *
 **************************************************************************/

#define PN_INITIAL_SEED 0x70816958L	/* Pseudo noise generator seed value  */

#define CN_INT_PERIOD 24	/* Comfort noise interpolation period
				   (nbr of frames between successive
				   SID updates in the decoder) */

#define DTX_HANGOVER 7		/* Period when SP=1 although VAD=0.
				   Used for comfort noise averaging */

/* Frame classification constants */

#define VALID_SID_FRAME          1
#define INVALID_SID_FRAME        2
#define GOOD_SPEECH_FRAME        3
#define UNUSABLE_FRAME           4

/* Encoder DTX control flags */

#define TX_SP_FLAG               0x0001
#define TX_VAD_FLAG              0x0002
#define TX_HANGOVER_ACTIVE       0x0004
#define TX_PREV_HANGOVER_ACTIVE  0x0008
#define TX_SID_UPDATE            0x0010
#define TX_USE_OLD_SID           0x0020

/* Decoder DTX control flags */

#define RX_SP_FLAG               0x0001
#define RX_UPD_SID_QUANT_MEM     0x0002
#define RX_FIRST_SID_UPDATE      0x0004
#define RX_CONT_SID_UPDATE       0x0008
#define RX_LOST_SID_FRAME        0x0010
#define RX_INVALID_SID_FRAME     0x0020
#define RX_NO_TRANSMISSION       0x0040
#define RX_DTX_MUTING            0x0080
#define RX_PREV_DTX_MUTING       0x0100
#define RX_CNI_BFI               0x0200
#define RX_FIRST_SP_FLAG         0x0400

void w_reset_w_tx_dtx(void);	/* Reset tx dtx variables */
void w_reset_w_rx_dtx(void);	/* Reset rx dtx variables */

void w_tx_dtx(int16_t VAD_flag, int16_t * w_txdtx_ctrl);

void w_rx_dtx(int16_t * w_rxdtx_ctrl, int16_t TAF, int16_t bfi,
	      int16_t SID_flag);

void w_CN_encoding(int16_t params[], int16_t w_txdtx_ctrl);

void w_sid_codeword_encoding(int16_t ser2[]
    );

int16_t w_sid_frame_detection(int16_t ser2[]
    );

void w_update_lsf_history(int16_t lsf1[M],
			  int16_t lsf2[M], int16_t lsf_old[DTX_HANGOVER][M]
    );

void update_w_lsf_p_CN(int16_t lsf_old[DTX_HANGOVER][M], int16_t w_lsf_p_CN[M]
    );

void w_aver_lsf_history(int16_t lsf_old[DTX_HANGOVER][M],
			int16_t lsf1[M], int16_t lsf2[M], int16_t lsf_aver[M]
    );

void w_update_gain_code_history_tx(int16_t new_gain_code,
				   int16_t w_gain_code_old_tx[4 * DTX_HANGOVER]
    );

void w_update_gain_code_history_rx(int16_t new_gain_code,
				   int16_t w_gain_code_old_rx[4 * DTX_HANGOVER]
    );

int16_t compute_w_CN_w_excitation_gain(int16_t w_res2[L_SUBFR]
    );

int16_t update_w_gcode0_CN(int16_t w_gain_code_old_tx[4 * DTX_HANGOVER]
    );

int16_t w_aver_gain_code_history(int16_t w_CN_w_excitation_gain,
				int16_t gain_code_old[4 * DTX_HANGOVER]
    );

void w_build_CN_code(int16_t cod[], int32_t * seed);

int16_t w_pseudonoise(int32_t * shift_reg, int16_t no_bits);

int16_t w_interpolate_CN_param(int16_t old_param,
			      int16_t new_param, int16_t w_w_rx_dtx_w_state);

void w_interpolate_CN_lsf(int16_t w_lsf_old_CN[M],
			  int16_t w_lsf_new_CN[M],
			  int16_t lsf_interp_CN[M],
			  int16_t w_w_rx_dtx_w_state);
