/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Global w_counter variable for calculation of complexity weight */

typedef struct {
	int32_t w_add;		/* Complexity Weight of 1 */
	int32_t w_sub;
	int32_t w_abs_s;
	int32_t w_shl;
	int32_t w_shr;
	int32_t w_extract_h;
	int32_t w_extract_l;
	int32_t w_mult;
	int32_t w_L_w_mult;
	int32_t w_negate;
	int32_t w_round;
	int32_t w_L_mac;
	int32_t w_L_msu;
	int32_t w_w_L_macNs;
	int32_t w_w_L_msuNs;
	int32_t L_w_add;		/* Complexity Weight of 2 */
	int32_t w_L_w_sub;
	int32_t L_w_add_c;
	int32_t w_w_L_w_sub_c;
	int32_t w_L_w_negate;
	int32_t w_L_w_shl;
	int32_t w_L_w_shr;
	int32_t w_w_mult_r;
	int32_t w_w_shr_r;
	int32_t shift_r;
	int32_t w_mac_r;
	int32_t w_msu_r;
	int32_t w_L_deposit_h;
	int32_t w_L_deposit_l;
	int32_t w_w_L_w_w_shr_r;	/* Complexity Weight of 3 */
	int32_t L_shift_r;
	int32_t w_L_abs;
	int32_t w_L_sat;		/* Complexity Weight of 4 */
	int32_t w_norm_s;	/* Complexity Weight of 15 */
	int32_t w_div_s;		/* Complexity Weight of 18 */
	int32_t w_norm_l;	/* Complexity Weight of 30 */
	int32_t DataMove16;	/* Complexity Weight of 1 */
	int32_t DataMove32;	/* Complexity Weight of 2 */
	int32_t Logic16;		/* Complexity Weight of 1 */
	int32_t Logic32;		/* Complexity Weight of 2 */
	int32_t Test;		/* Complexity Weight of 2 */
} BASIC_OP;

int32_t w_TotalWeightedOperation(void);
int32_t w_DeltaWeightedOperation(void);

void Init_WMOPS_w_counter(void);
void Reset_WMOPS_w_counter(void);
void w_WMOPS_output(int16_t w_dtx_mode);
int32_t w_fw_wc(void);

void ww_move16(void);
void ww_move32(void);
void ww_logic16(void);
void ww_logic32(void);
void ww_test(void);
