/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
extern int w_Overflow;
extern int w_Carry;

#define MAX_32 (int32_t)0x7fffffffL
#define MIN_32 (int32_t)0x80000000L

#define MAX_16 (int16_t)0x7fff
#define MIN_16 (int16_t)0x8000

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

int16_t w_add(int16_t var1, int16_t var2);	/* Short w_add,           1   */
int16_t w_sub(int16_t var1, int16_t var2);	/* Short w_sub,           1   */
int16_t w_abs_s(int16_t var1);	/* Short abs,           1   */
int16_t w_shl(int16_t var1, int16_t var2);	/* Short shift left,    1   */
int16_t w_shr(int16_t var1, int16_t var2);	/* Short shift right,   1   */
int16_t w_mult(int16_t var1, int16_t var2);	/* Short w_mult,          1   */
int32_t w_L_w_mult(int16_t var1, int16_t var2);	/* Long w_mult,           1   */
int16_t w_negate(int16_t var1);	/* Short w_negate,        1   */
int16_t w_extract_h(int32_t L_var1);	/* Extract high,        1   */
int16_t w_extract_l(int32_t L_var1);	/* Extract low,         1   */
int16_t w_round(int32_t L_var1);	/* Round,               1   */
int32_t w_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,  1  */
int32_t w_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,  1  */
int32_t w_w_L_macNs(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac without
								   sat, 1   */
int32_t L_w_add(int32_t L_var1, int32_t L_var2);	/* Long w_add,        2 */
int32_t w_L_w_sub(int32_t L_var1, int32_t L_var2);	/* Long w_sub,        2 */
int32_t L_w_add_c(int32_t L_var1, int32_t L_var2);	/* Long w_add with c, 2 */
int32_t w_L_w_negate(int32_t L_var1);	/* Long w_negate,     2 */
int16_t w_w_mult_r(int16_t var1, int16_t var2);	/* Mult with w_round, 2 */
int32_t w_L_w_shl(int32_t L_var1, int16_t var2);	/* Long shift left, 2 */
int32_t w_L_w_shr(int32_t L_var1, int16_t var2);	/* Long shift right, 2 */
int32_t w_L_deposit_h(int16_t var1);	/* 16 bit var1 -> MSB,     2 */
int32_t w_L_deposit_l(int16_t var1);	/* 16 bit var1 -> LSB,     2 */

int32_t w_w_L_w_w_shr_r(int32_t L_var1, int16_t var2);	/* Long shift right with
							   w_round,  3             */
int32_t w_L_abs(int32_t L_var1);	/* Long abs,              3  */
int16_t w_norm_s(int16_t var1);	/* Short norm,           15  */
int16_t w_div_s(int16_t var1, int16_t var2);	/* Short division,       18  */
int16_t w_norm_l(int32_t L_var1);	/* Long norm,            30  */
