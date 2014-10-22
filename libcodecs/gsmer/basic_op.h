/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
extern Flag w_Overflow;
extern Flag w_Carry;

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

Word16 w_add(Word16 var1, Word16 var2);	/* Short w_add,           1   */
Word16 w_sub(Word16 var1, Word16 var2);	/* Short w_sub,           1   */
Word16 w_abs_s(Word16 var1);	/* Short abs,           1   */
Word16 w_shl(Word16 var1, Word16 var2);	/* Short shift left,    1   */
Word16 w_shr(Word16 var1, Word16 var2);	/* Short shift right,   1   */
Word16 w_mult(Word16 var1, Word16 var2);	/* Short w_mult,          1   */
Word32 w_L_w_mult(Word16 var1, Word16 var2);	/* Long w_mult,           1   */
Word16 w_negate(Word16 var1);	/* Short w_negate,        1   */
Word16 w_extract_h(Word32 L_var1);	/* Extract high,        1   */
Word16 w_extract_l(Word32 L_var1);	/* Extract low,         1   */
Word16 w_round(Word32 L_var1);	/* Round,               1   */
Word32 w_L_mac(Word32 L_var3, Word16 var1, Word16 var2);	/* Mac,  1  */
Word32 w_L_msu(Word32 L_var3, Word16 var1, Word16 var2);	/* Msu,  1  */
Word32 w_w_L_macNs(Word32 L_var3, Word16 var1, Word16 var2);	/* Mac without
								   sat, 1   */
Word32 w_w_L_msuNs(Word32 L_var3, Word16 var1, Word16 var2);	/* Msu without
								   sat, 1   */
Word32 L_w_add(Word32 L_var1, Word32 L_var2);	/* Long w_add,        2 */
Word32 w_L_w_sub(Word32 L_var1, Word32 L_var2);	/* Long w_sub,        2 */
Word32 L_w_add_c(Word32 L_var1, Word32 L_var2);	/* Long w_add with c, 2 */
Word32 w_w_L_w_sub_c(Word32 L_var1, Word32 L_var2);	/* Long w_sub with c, 2 */
Word32 w_L_w_negate(Word32 L_var1);	/* Long w_negate,     2 */
Word16 w_w_mult_r(Word16 var1, Word16 var2);	/* Mult with w_round, 2 */
Word32 w_L_w_shl(Word32 L_var1, Word16 var2);	/* Long shift left, 2 */
Word32 w_L_w_shr(Word32 L_var1, Word16 var2);	/* Long shift right, 2 */
Word16 w_w_shr_r(Word16 var1, Word16 var2);	/* Shift right with
						   w_round, 2           */
Word16 w_mac_r(Word32 L_var3, Word16 var1, Word16 var2);	/* Mac with
								   w_rounding,2 */
Word16 w_msu_r(Word32 L_var3, Word16 var1, Word16 var2);	/* Msu with
								   w_rounding,2 */
Word32 w_L_deposit_h(Word16 var1);	/* 16 bit var1 -> MSB,     2 */
Word32 w_L_deposit_l(Word16 var1);	/* 16 bit var1 -> LSB,     2 */

Word32 w_w_L_w_w_shr_r(Word32 L_var1, Word16 var2);	/* Long shift right with
							   w_round,  3             */
Word32 w_L_abs(Word32 L_var1);	/* Long abs,              3  */
Word32 w_L_sat(Word32 L_var1);	/* Long saturation,       4  */
Word16 w_norm_s(Word16 var1);	/* Short norm,           15  */
Word16 w_div_s(Word16 var1, Word16 var2);	/* Short division,       18  */
Word16 w_norm_l(Word32 L_var1);	/* Long norm,            30  */
