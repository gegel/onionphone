/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Version 3.3    Last modified: December 26, 1995 */

/* Double precision operations */

void L_Extract(int32_t L_32, int16_t * hi, int16_t * lo);
int32_t L_Comp(int16_t hi, int16_t lo);
int32_t Mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2);
int32_t Mpy_32_16(int16_t hi, int16_t lo, int16_t n);
int32_t Div_32(int32_t L_num, int16_t denom_hi, int16_t denom_lo);
