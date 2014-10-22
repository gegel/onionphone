/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Double precision operations */

void w_L_Extract(Word32 L_32, Word16 * hi, Word16 * lo);
Word32 w_L_Comp(Word16 hi, Word16 lo);
Word32 w_Mpy_32(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2);
Word32 w_w_Mpy_32_16(Word16 hi, Word16 lo, Word16 n);
Word32 w_Div_32(Word32 L_num, Word16 denom_hi, Word16 denom_lo);
