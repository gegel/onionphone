/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Global w_counter variable for calculation of complexity weight */

typedef struct {
	Word32 w_add;		/* Complexity Weight of 1 */
	Word32 w_sub;
	Word32 w_abs_s;
	Word32 w_shl;
	Word32 w_shr;
	Word32 w_extract_h;
	Word32 w_extract_l;
	Word32 w_mult;
	Word32 w_L_w_mult;
	Word32 w_negate;
	Word32 w_round;
	Word32 w_L_mac;
	Word32 w_L_msu;
	Word32 w_w_L_macNs;
	Word32 w_w_L_msuNs;
	Word32 L_w_add;		/* Complexity Weight of 2 */
	Word32 w_L_w_sub;
	Word32 L_w_add_c;
	Word32 w_w_L_w_sub_c;
	Word32 w_L_w_negate;
	Word32 w_L_w_shl;
	Word32 w_L_w_shr;
	Word32 w_w_mult_r;
	Word32 w_w_shr_r;
	Word32 shift_r;
	Word32 w_mac_r;
	Word32 w_msu_r;
	Word32 w_L_deposit_h;
	Word32 w_L_deposit_l;
	Word32 w_w_L_w_w_shr_r;	/* Complexity Weight of 3 */
	Word32 L_shift_r;
	Word32 w_L_abs;
	Word32 w_L_sat;		/* Complexity Weight of 4 */
	Word32 w_norm_s;	/* Complexity Weight of 15 */
	Word32 w_div_s;		/* Complexity Weight of 18 */
	Word32 w_norm_l;	/* Complexity Weight of 30 */
	Word32 DataMove16;	/* Complexity Weight of 1 */
	Word32 DataMove32;	/* Complexity Weight of 2 */
	Word32 Logic16;		/* Complexity Weight of 1 */
	Word32 Logic32;		/* Complexity Weight of 2 */
	Word32 Test;		/* Complexity Weight of 2 */
} BASIC_OP;

Word32 w_TotalWeightedOperation(void);
Word32 w_DeltaWeightedOperation(void);

void Init_WMOPS_w_counter(void);
void Reset_WMOPS_w_counter(void);
void w_WMOPS_output(Word16 w_dtx_mode);
Word32 w_fw_wc(void);

void ww_move16(void);
void ww_move32(void);
void ww_logic16(void);
void ww_logic32(void);
void ww_test(void);
