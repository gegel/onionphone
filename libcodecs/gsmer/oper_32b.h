/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Double precision operations */

void w_L_Extract(int32_t L_32, int16_t * hi, int16_t * lo);
int32_t w_L_Comp(int16_t hi, int16_t lo);
int32_t w_Mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2);
int32_t w_w_Mpy_32_16(int16_t hi, int16_t lo, int16_t n);
int32_t w_Div_32(int32_t L_num, int16_t denom_hi, int16_t denom_lo);
