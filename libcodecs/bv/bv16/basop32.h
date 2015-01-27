/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
  ===========================================================================
   File: BASOP32.H                                       v.2.0 - 15.Nov.2004
  ===========================================================================

            ITU-T STL  BASIC OPERATORS

            GLOBAL FUNCTION PROTOTYPES

   History:
   26.Jan.00   v1.0     Incorporated to the STL from updated G.723.1/G.729 
                        basic operator library (based on basic_op.h) and 
                        G.723.1's basop.h.
   05.Jul.00    v1.1    Added 32-bit shiftless bv_mult/mac/mbv_sub operators

   03 Nov 04   v2.0     Incorporation of new 32-bit / 40-bit / control
                        operators for the ITU-T Standard Tool Library as 
                        described in Geneva, 20-30 January 2004 WP 3/16 Q10/16
                        TD 11 document and bv_subsequent discussions on the
                        wp3audio@yahoogroups.com email reflector.
                        bv_norm_s()      weight reduced from 15 to 1.
                        bv_norm_l()      weight reduced from 30 to 1.
                        bv_L_abs()       weight reduced from  2 to 1.
                        L_bv_add()       weight reduced from  2 to 1.
                        L_bv_negate()    weight reduced from  2 to 1.
                        L_bv_shl()       weight reduced from  2 to 1.
                        L_bv_shr()       weight reduced from  2 to 1.
                        L_bv_sub()       weight reduced from  2 to 1.
                        bv_mac_r()       weight reduced from  2 to 1.
                        bv_msu_r()       weight reduced from  2 to 1.
                        bv_bv_mult_r()      weight reduced from  2 to 1.
                        bv_L_deposit_h() weight reduced from  2 to 1.
                        bv_L_deposit_l() weight reduced from  2 to 1.
   15 Nov 04   v2.0     bv_L_mls() weight of 5.
						bv_div_l() weight of 32.
						i_bv_mult() weight of 3.

  ============================================================================
*/

#ifndef _BASIC_OP_H
#define _BASIC_OP_H

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 | $Id $
 |___________________________________________________________________________|
*/
extern int bv_Overflow;
extern int bv_Carry;

#define MAX_32 (int32_t)0x7fffffffL
#define MIN_32 (int32_t)0x80000000L

#define MAX_16 (int16_t)0x7fff
#define MIN_16 (int16_t)0x8000

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

int16_t bv_add(int16_t var1, int16_t var2);	/* Short bv_add,           1   */
int16_t bv_sub(int16_t var1, int16_t var2);	/* Short bv_sub,           1   */
int16_t bv_abs_s(int16_t var1);	/* Short abs,           1   */
int16_t bv_shl(int16_t var1, int16_t var2);	/* Short shift left,    1   */
int16_t bv_shr(int16_t var1, int16_t var2);	/* Short shift right,   1   */
int16_t bv_mult(int16_t var1, int16_t var2);	/* Short bv_mult,          1   */
int32_t L_bv_mult(int16_t var1, int16_t var2);	/* Long bv_mult,           1   */
int16_t bv_negate(int16_t var1);	/* Short bv_negate,        1   */
int16_t bv_extract_h(int32_t L_var1);	/* Extract high,        1   */
int16_t bv_extract_l(int32_t L_var1);	/* Extract low,         1   */
int16_t intround(int32_t L_var1);	/* Round,               1   */
int32_t bv_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,  1  */
int32_t bv_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,  1  */
int32_t L_bv_add(int32_t L_var1, int32_t L_var2);	/* Long bv_add,        1 */
int32_t L_bv_sub(int32_t L_var1, int32_t L_var2);	/* Long bv_sub,        1 */
int32_t L_bv_negate(int32_t L_var1);	/* Long bv_negate,     1 */
int16_t bv_bv_mult_r(int16_t var1, int16_t var2);	/* Mult with round, 1 */
int32_t L_bv_shl(int32_t L_var1, int16_t var2);	/* Long shift left, 1 */
int32_t L_bv_shr(int32_t L_var1, int16_t var2);	/* Long shift right, 1 */
int32_t bv_L_deposit_h(int16_t var1);	/* 16 bit var1 -> MSB,     1 */
int32_t bv_L_deposit_l(int16_t var1);	/* 16 bit var1 -> LSB,     1 */

int32_t L_bv_bv_shr_r(int32_t L_var1, int16_t var2);	/* Long shift right with
							   round,             3  */
int32_t bv_L_abs(int32_t L_var1);	/* Long abs,              1  */
int16_t bv_norm_s(int16_t var1);	/* Short norm,            1  */
int16_t bv_div_s(int16_t var1, int16_t var2);	/* Short division,       18  */
int16_t bv_norm_l(int32_t L_var1);	/* Long norm,             1  */

/*
 *  New shiftless operators, not used in G.729/G.723.1
*/
int32_t L_bv_mult0(int16_t v1, int16_t v2);	/* 32-bit Multiply w/o shift         1 */
int32_t bv_L_mac0(int32_t L_v3, int16_t v1, int16_t v2);	/* 32-bit Mac w/o shift  1 */
int32_t bv_L_msu0(int32_t L_v3, int16_t v1, int16_t v2);	/* 32-bit Msu w/o shift  1 */

#endif				/* ifndef _BASIC_OP_H */

/* end of file */
