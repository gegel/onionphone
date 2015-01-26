/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*                                                      v.2.0 - 15.Nov.2004
  =============================================================================

                          U    U   GGG    SSSS  TTTTT
                          U    U  G       S       T
                          U    U  G  GG   SSSS    T
                          U    U  G   G       S   T
                           UUU     GG     SSS     T

                   ========================================
                    ITU-T - USER'S GROUP ON SOFTWARE TOOLS
                   ========================================

       =============================================================
       COPYRIGHT NOTE: This source code, and all of its derivations,
       is bv_subject to the "ITU-T General Public License". Please have
       it  read  in    the  distribution  disk,   or  in  the  ITU-T
       Recommendation G.191 on "SOFTWARE TOOLS FOR SPEECH AND  AUDIO
       CODING STANDARDS".
       =============================================================

MODULE:         BASOP32, BASIC OPERATORS

ORIGINAL BY:
                Incorporated from anonymous contributions for 
                ETSI Standards as well as G.723.1, G.729, and G.722.1

DESCRIPTION:
        This file contains the definition of 16- and 32-bit basic
        operators to be used in the implementation of signal
        processing algorithms. The basic operators try to resemble
        assembly language instructions that are commonly found in
        digital signal processor (DSP) CPUs, thus allowing algorithm
        C-code implementations more directly mapeable to DSP assembly
        code.

        *********************************************************
         NOTE: so far, this module does not have a demo program!
        *********************************************************

FUNCTIONS:
  Defined in basop32.h. Self-documentation within each function.

HISTORY:
  26.Jan.00    v1.0     Incorporated to the STL from updated G.723.1/G.729 
                        basic operator library (based on basicop2.c) and 
                        G.723.1's basop.c [bv_L_mls(), bv_div_l(), i_bv_mult()]

  05.Jul.00    v1.1     Added 32-bit shiftless accumulation basic 
                        operators (bv_L_msu0, bv_L_mac0, L_bv_mult0). Improved
                        documentation for i_bv_mult().

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

  =============================================================================
*/

/*___________________________________________________________________________
 |                                                                           |
 | Basic arithmetic operators.                                               |
 |                                                                           |
 | $Id $                                                                     |
 |                                                                           |
 |       saturate()                                                          |
 |       bv_add()                                                               |
 |       bv_sub()                                                               |
 |       bv_abs_s()                                                             |
 |       divide_s()                                                          |
 |       bv_extract_h()                                                         |
 |       bv_extract_l()                                                         |
 |       bv_L_abs()                                                             |
 |       L_bv_add()                                                             |
 |       bv_L_deposit_h()                                                       |
 |       bv_L_deposit_l()                                                       |
 |       bv_L_mac()                                                             |
 |       bv_L_msu()                                                             |
 |       L_bv_mult()                                                            |
 |       L_bv_negate()                                                          |
 |       L_bv_shl()                                                             |
 |       L_bv_shr()                                                             |
 |       L_bv_sub()                                                             |
 |       bv_mac_r()                                                             |
 |       bv_msu_r()                                                             |
 |       bv_mult()                                                              |
 |       bv_bv_mult_r()                                                            |
 |       bv_negate()                                                            |
 |       bv_norm_l()                                                            |
 |       bv_norm_s()                                                            |
 |       round()                                                             |
 |       bv_shl()                                                               |
 |       bv_shr()                                                               |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Include-Files                                                           |
 |___________________________________________________________________________|
*/
#include <stdio.h>
#include <stdlib.h>
#include "stl.h"

#if (WMOPS)
extern BASIC_OP bv_multiCounter[MAXCOUNTERS];
extern int currCounter;
#endif

/*___________________________________________________________________________
 |                                                                           |
 |   Local Functions                                                         |
 |___________________________________________________________________________|
*/
int16_t saturate(int32_t L_var1);

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
int bv_Overflow = 0;
int bv_Carry = 0;

/*___________________________________________________________________________
 |                                                                           |
 |   Functions                                                               |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : saturate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Limit the 32 bit input to the range of a 16 bit word.                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t saturate(int32_t L_var1)
{
	int16_t var_out;

	if (L_var1 > 0X00007fffL) {
		bv_Overflow = 1;
		var_out = MAX_16;
	} else if (L_var1 < (int32_t) 0xffff8000L) {
		bv_Overflow = 1;
		var_out = MIN_16;
	} else {
		var_out = bv_extract_l(L_var1);

#if (WMOPS)
		bv_multiCounter[currCounter].bv_extract_l--;
#endif
	}

	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_add                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the bv_addition (var1+var2) with overflow control and saturation;|
 |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
 |    when underflow occurs.                                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_add(int16_t var1, int16_t var2)
{
	int16_t var_out;
	int32_t L_sum;

	L_sum = (int32_t) var1 + var2;
	var_out = saturate(L_sum);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_add++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_sub                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the bv_subtraction (var1+var2) with overflow control and satu-   |
 |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
 |    -32768 when underflow occurs.                                          |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_sub(int16_t var1, int16_t var2)
{
	int16_t var_out;
	int32_t L_diff;

	L_diff = (int32_t) var1 - var2;
	var_out = saturate(L_diff);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_sub++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_abs_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of var1; bv_abs_s(-32768) = 32767.                         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_abs_s(int16_t var1)
{
	int16_t var_out;

	if (var1 == (int16_t) MIN_16) {
		var_out = MAX_16;
	} else {
		if (var1 < 0) {
			var_out = -var1;
		} else {
			var_out = var1;
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_abs_s++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_shl                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
 |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
 |   var1 right by -var2 with sign extension. Saturate the result in case of |
 |   underflows or overflows.                                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_shl(int16_t var1, int16_t var2)
{
	int16_t var_out;
	int32_t result;

	if (var2 < 0) {
		if (var2 < -16)
			var2 = -16;
		var2 = -var2;
		var_out = bv_shr(var1, var2);

#if (WMOPS)
		bv_multiCounter[currCounter].bv_shr--;
#endif
	} else {
		result = (int32_t) var1 *((int32_t) 1 << var2);

		if ((var2 > 15 && var1 != 0)
		    || (result != (int32_t) ((int16_t) result))) {
			bv_Overflow = 1;
			var_out = (var1 > 0) ? MAX_16 : MIN_16;
		} else {
			var_out = bv_extract_l(result);

#if (WMOPS)
			bv_multiCounter[currCounter].bv_extract_l--;
#endif
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_shl++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_shr                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
 |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
 |   -var2 with sign extension. Saturate the result in case of underflows or |
 |   overflows.                                                              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_shr(int16_t var1, int16_t var2)
{
	int16_t var_out;

	if (var2 < 0) {
		if (var2 < -16)
			var2 = -16;
		var2 = -var2;
		var_out = bv_shl(var1, var2);

#if (WMOPS)
		bv_multiCounter[currCounter].bv_shl--;
#endif
	} else {
		if (var2 >= 15) {
			var_out = (var1 < 0) ? -1 : 0;
		} else {
			if (var1 < 0) {
				var_out = ~((~var1) >> var2);
			} else {
				var_out = var1 >> var2;
			}
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_shr++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_mult                                                    |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the bv_multiplication of var1 by var2 and gives a 16 bit result  |
 |    which is scaled i.e.:                                                  |
 |             bv_mult(var1,var2) = bv_extract_l(L_bv_shr((var1 times var2),15)) and  |
 |             bv_mult(-32768,-32768) = 32767.                                  |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_mult(int16_t var1, int16_t var2)
{
	int16_t var_out;
	int32_t L_product;

	L_product = (int32_t) var1 *(int32_t) var2;

	L_product = (L_product & (int32_t) 0xffff8000L) >> 15;

	if (L_product & (int32_t) 0x00010000L)
		L_product = L_product | (int32_t) 0xffff0000L;

	var_out = saturate(L_product);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_mult++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_mult                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   L_bv_mult is the 32 bit result of the bv_multiplication of var1 times var2    |
 |   with one shift left i.e.:                                               |
 |        L_bv_mult(var1,var2) = L_bv_shl((var1 times var2),1) and                 |
 |        L_bv_mult(-32768,-32768) = 2147483647.                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_mult(int16_t var1, int16_t var2)
{
	int32_t L_var_out;

	L_var_out = (int32_t) var1 *(int32_t) var2;

	if (L_var_out != (int32_t) 0x40000000L) {
		L_var_out *= 2;
	} else {
		bv_Overflow = 1;
		L_var_out = MAX_32;
	}

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_mult++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_negate                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate var1 with saturation, saturate in the case where input is -32768:|
 |                bv_negate(var1) = bv_sub(0,var1).                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_negate(int16_t var1)
{
	int16_t var_out;

	var_out = (var1 == MIN_16) ? MAX_16 : -var1;

#if (WMOPS)
	bv_multiCounter[currCounter].bv_negate++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_extract_h                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 MSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_extract_h(int32_t L_var1)
{
	int16_t var_out;

	var_out = (int16_t) (L_var1 >> 16);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_extract_h++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_extract_l                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 LSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t bv_extract_l(int32_t L_var1)
{
	int16_t var_out;

	var_out = (int16_t) L_var1;

#if (WMOPS)
	bv_multiCounter[currCounter].bv_extract_l++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : round                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into the MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |               round(L_var1) = bv_extract_h(L_bv_add(L_var1,32768))              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int16_t intround(int32_t L_var1)
{
	int16_t var_out;
	int32_t L_rounded;

	L_rounded = L_bv_add(L_var1, (int32_t) 0x00008000L);
	var_out = bv_extract_h(L_rounded);

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_add--;
	bv_multiCounter[currCounter].bv_extract_h--;
	bv_multiCounter[currCounter].round++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_L_mac                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation, return a 32 bit result:               |
 |        bv_L_mac(L_var3,var1,var2) = L_bv_add(L_var3,L_bv_mult(var1,var2)).         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t bv_L_mac(int32_t L_var3, int16_t var1, int16_t var2)
{
	int32_t L_var_out;
	int32_t L_product;

	L_product = L_bv_mult(var1, var2);
	L_var_out = L_bv_add(L_var3, L_product);

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_mult--;
	bv_multiCounter[currCounter].L_bv_add--;
	bv_multiCounter[currCounter].bv_L_mac++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_L_msu                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result from L_var3 with saturation, return a 32 bit result:         |
 |        bv_L_msu(L_var3,var1,var2) = L_bv_sub(L_var3,L_bv_mult(var1,var2)).         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t bv_L_msu(int32_t L_var3, int16_t var1, int16_t var2)
{
	int32_t L_var_out;
	int32_t L_product;

	L_product = L_bv_mult(var1, var2);
	L_var_out = L_bv_sub(L_var3, L_product);

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_mult--;
	bv_multiCounter[currCounter].L_bv_sub--;
	bv_multiCounter[currCounter].bv_L_msu++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_add                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits bv_addition of the two 32 bits variables (L_var1+L_var2) with      |
 |   overflow control and saturation; the result is set at +2147483647 when  |
 |   overflow occurs or at -2147483648 when underflow occurs.                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_add(int32_t L_var1, int32_t L_var2)
{
	int32_t L_var_out;

	L_var_out = L_var1 + L_var2;

	if (((L_var1 ^ L_var2) & MIN_32) == 0) {
		if ((L_var_out ^ L_var1) & MIN_32) {
			L_var_out = (L_var1 < 0) ? MIN_32 : MAX_32;
			bv_Overflow = 1;
		}
	}
#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_add++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_sub                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits bv_subtraction of the two 32 bits variables (L_var1-L_var2) with   |
 |   overflow control and saturation; the result is set at +2147483647 when  |
 |   overflow occurs or at -2147483648 when underflow occurs.                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_sub(int32_t L_var1, int32_t L_var2)
{
	int32_t L_var_out;

	L_var_out = L_var1 - L_var2;

	if (((L_var1 ^ L_var2) & MIN_32) != 0) {
		if ((L_var_out ^ L_var1) & MIN_32) {
			L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
			bv_Overflow = 1;
		}
	}
#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_sub++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_negate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
 |   where input is -2147483648 (0x8000 0000).                               |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_negate(int32_t L_var1)
{
	int32_t L_var_out;

	L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_negate++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_bv_mult_r                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as bv_mult with rounding, i.e.:                                       |
 |     bv_bv_mult_r(var1,var2) = bv_extract_l(L_bv_shr(((var1 * var2) + 16384),15)) and  |
 |     bv_bv_mult_r(-32768,-32768) = 32767.                                        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x8000 <= var_out <= 0x7fff.                          |
 |___________________________________________________________________________|
*/
int16_t bv_bv_mult_r(int16_t var1, int16_t var2)
{
	int16_t var_out;
	int32_t L_product_arr;

	L_product_arr = (int32_t) var1 *(int32_t) var2;	/* product */
	L_product_arr += (int32_t) 0x00004000L;	/* round */
	L_product_arr &= (int32_t) 0xffff8000L;
	L_product_arr >>= 15;	/* shift */

	if (L_product_arr & (int32_t) 0x00010000L) {	/* sign extend when necessary */
		L_product_arr |= (int32_t) 0xffff0000L;
	}
	var_out = saturate(L_product_arr);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_bv_mult_r++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_shl                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
 |   fill the var2 LSB of the result. If var2 is negative, arithmetically    |
 |   shift L_var1 right by -var2 with sign extension. Saturate the result in |
 |   case of underflows or overflows.                                        |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_shl(int32_t L_var1, int16_t var2)
{

	int32_t L_var_out = 0L;

	if (var2 <= 0) {
		if (var2 < -32)
			var2 = -32;
		var2 = -var2;
		L_var_out = L_bv_shr(L_var1, var2);
#if (WMOPS)
		bv_multiCounter[currCounter].L_bv_shr--;
#endif
	} else {
		for (; var2 > 0; var2--) {
			if (L_var1 > (int32_t) 0X3fffffffL) {
				bv_Overflow = 1;
				L_var_out = MAX_32;
				break;
			} else {
				if (L_var1 < (int32_t) 0xc0000000L) {
					bv_Overflow = 1;
					L_var_out = MIN_32;
					break;
				}
			}
			L_var1 *= 2;
			L_var_out = L_var1;
		}
	}
#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_shl++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_shr                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
 |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
 |   by -var2 and zero fill the -var2 LSB of the result. Saturate the result |
 |   in case of underflows or overflows.                                     |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/
int32_t L_bv_shr(int32_t L_var1, int16_t var2)
{
	int32_t L_var_out;

	if (var2 < 0) {
		if (var2 < -32)
			var2 = -32;
		var2 = -var2;
		L_var_out = L_bv_shl(L_var1, var2);
#if (WMOPS)
		bv_multiCounter[currCounter].L_bv_shl--;
#endif
	} else {
		if (var2 >= 31) {
			L_var_out = (L_var1 < 0L) ? -1 : 0;
		} else {
			if (L_var1 < 0) {
				L_var_out = ~((~L_var1) >> var2);
			} else {
				L_var_out = L_var1 >> var2;
			}
		}
	}
#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_shr++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_L_deposit_h                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
 |   16 LS bits of the output are zeroed.                                    |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff 0000.                |
 |___________________________________________________________________________|
*/
int32_t bv_L_deposit_h(int16_t var1)
{
	int32_t L_var_out;

	L_var_out = (int32_t) var1 << 16;

#if (WMOPS)
	bv_multiCounter[currCounter].bv_L_deposit_h++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_L_deposit_l                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
 |   16 MS bits of the output are sign extended.                             |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/
int32_t bv_L_deposit_l(int16_t var1)
{
	int32_t L_var_out;

	L_var_out = (int32_t) var1;

#if (WMOPS)
	bv_multiCounter[currCounter].bv_L_deposit_l++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : L_bv_bv_shr_r                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as L_bv_shr(L_var1,var2) but with rounding. Saturate the result in    |
 |   case of underflows or overflows :                                       |
 |    - If var2 is greater than zero :                                       |
 |          if (L_bv_sub(L_bv_shl(L_bv_shr(L_var1,var2),1),L_bv_shr(L_var1,bv_sub(var2,1))))|
 |          is equal to zero                                                 |
 |                     then                                                  |
 |                     L_bv_bv_shr_r(L_var1,var2) = L_bv_shr(L_var1,var2)             |
 |                     else                                                  |
 |                     L_bv_bv_shr_r(L_var1,var2) = L_bv_add(L_bv_shr(L_var1,var2),1)    |
 |    - If var2 is less than or equal to zero :                              |
 |                     L_bv_bv_shr_r(L_var1,var2) = L_bv_shr(L_var1,var2).            |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/
int32_t L_bv_bv_shr_r(int32_t L_var1, int16_t var2)
{
	int32_t L_var_out;

	if (var2 > 31) {
		L_var_out = 0;
	} else {
		L_var_out = L_bv_shr(L_var1, var2);

#if (WMOPS)
		bv_multiCounter[currCounter].L_bv_shr--;
#endif
		if (var2 > 0) {
			if ((L_var1 & ((int32_t) 1 << (var2 - 1))) != 0) {
				L_var_out++;
			}
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_bv_shr_r++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_L_abs                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of L_var1; Saturate in case where the input is          |
 |                                                               -214783648  |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x0000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/
int32_t bv_L_abs(int32_t L_var1)
{
	int32_t L_var_out;

	if (L_var1 == MIN_32) {
		L_var_out = MAX_32;
	} else {
		if (L_var1 < 0) {
			L_var_out = -L_var1;
		} else {
			L_var_out = L_var1;
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_L_abs++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_norm_s                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = bv_shl(var1,bv_norm_s(var1)).                    |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
 |___________________________________________________________________________|
*/
int16_t bv_norm_s(int16_t var1)
{
	int16_t var_out;

	if (var1 == 0) {
		var_out = 0;
	} else {
		if (var1 == (int16_t) 0xffff) {
			var_out = 15;
		} else {
			if (var1 < 0) {
				var1 = ~var1;
			}
			for (var_out = 0; var1 < 0x4000; var_out++) {
				var1 <<= 1;
			}
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_norm_s++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_div_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces a result which is the fractional integer division of var1  by  |
 |   var2; var1 and var2 must be positive and var2 must be greater or equal  |
 |   to var1; the result is positive (leading bit equal to 0) and truncated  |
 |   to 16 bits.                                                             |
 |   If var1 = var2 then div(var1,var2) = 32767.                             |
 |                                                                           |
 |   Complexity weight : 18                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x0000 0000 <= var1 <= var2 and var2 != 0.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : var1 <= var2 <= 0x0000 7fff and var2 != 0.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |             It's a Q15 value (point between b15 and b14).                 |
 |___________________________________________________________________________|
*/
int16_t bv_div_s(int16_t var1, int16_t var2)
{
	int16_t var_out = 0;
	int16_t iteration;
	int32_t L_num;
	int32_t L_denom;

	if ((var1 > var2) || (var1 < 0) || (var2 < 0)) {
		printf("Division Error var1=%d  var2=%d\n", var1, var2);
		abort();	/* exit (0); */
	}
	if (var2 == 0) {
		printf("Division by 0, Fatal error \n");
		abort();	/* exit (0); */
	}
	if (var1 == 0) {
		var_out = 0;
	} else {
		if (var1 == var2) {
			var_out = MAX_16;
		} else {
			L_num = bv_L_deposit_l(var1);
			L_denom = bv_L_deposit_l(var2);

#if (WMOPS)
			bv_multiCounter[currCounter].bv_L_deposit_l--;
			bv_multiCounter[currCounter].bv_L_deposit_l--;
#endif

			for (iteration = 0; iteration < 15; iteration++) {
				var_out <<= 1;
				L_num <<= 1;

				if (L_num >= L_denom) {
					L_num = L_bv_sub(L_num, L_denom);
					var_out = bv_add(var_out, 1);
#if (WMOPS)
					bv_multiCounter[currCounter].L_bv_sub--;
					bv_multiCounter[currCounter].bv_add--;
#endif
				}
			}
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_div_s++;
#endif
	return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : bv_norm_l                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shifts needed to normalize the 32 bit varia-|
 |   ble L_var1 for positive values on the interval with minimum of          |
 |   1073741824 and maximum of 2147483647, and for negative values on the in-|
 |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
 |   to normalize the result, the following operation must be done :         |
 |                   norm_L_var1 = L_bv_shl(L_var1,bv_norm_l(L_var1)).             |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (int32_t) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (int16_t) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 001f.                |
 |___________________________________________________________________________|
*/
int16_t bv_norm_l(int32_t L_var1)
{
	int16_t var_out;

	if (L_var1 == 0) {
		var_out = 0;
	} else {
		if (L_var1 == (int32_t) 0xffffffffL) {
			var_out = 31;
		} else {
			if (L_var1 < 0) {
				L_var1 = ~L_var1;
			}
			for (var_out = 0; L_var1 < (int32_t) 0x40000000L;
			     var_out++) {
				L_var1 <<= 1;
			}
		}
	}

#if (WMOPS)
	bv_multiCounter[currCounter].bv_norm_l++;
#endif
	return (var_out);
}

/*
 ******************************************************************************
 * The following three operators are not part of the original 
 * G.729/G.723.1 set of basic operators and implement shiftless
 * accumulation operation.
 ******************************************************************************
*/

/*___________________________________________________________________________
 |
 |   Function Name : L_bv_mult0
 |
 |   Purpose :
 |
 |   L_bv_mult0 is the 32 bit result of the bv_multiplication of var1 times var2
 |   without one left shift.
 |
 |   Complexity weight : 1
 |
 |   Inputs :
 |
 |    var1     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |    var2     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |   Return Value :
 |
 |    L_var_out
 |             32 bit long signed integer (int32_t) whose value falls in the
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.
 |___________________________________________________________________________
*/
int32_t L_bv_mult0(int16_t var1, int16_t var2)
{
	int32_t L_var_out;

	L_var_out = (int32_t) var1 *(int32_t) var2;

#if (WMOPS)
	bv_multiCounter[currCounter].L_bv_mult0++;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |
 |   Function Name : bv_L_mac0
 |
 |   Purpose :
 |
 |   Multiply var1 by var2 (without left shift) and bv_add the 32 bit result to
 |   L_var3 with saturation, return a 32 bit result:
 |        bv_L_mac0(L_var3,var1,var2) = L_bv_add(L_var3,(L_bv_mult0(var1,var2)).
 |
 |   Complexity weight : 1
 |
 |   Inputs :
 |
 |    L_var3   32 bit long signed integer (int32_t) whose value falls in the
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.
 |
 |    var1     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |    var2     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |   Return Value :
 |
 |    L_var_out
 |             32 bit long signed integer (int32_t) whose value falls in the
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.
 |___________________________________________________________________________
*/
int32_t bv_L_mac0(int32_t L_var3, int16_t var1, int16_t var2)
{
	int32_t L_var_out;
	int32_t L_product;

	L_product = L_bv_mult0(var1, var2);
	L_var_out = L_bv_add(L_var3, L_product);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_L_mac0++;
	bv_multiCounter[currCounter].L_bv_mult0--;
	bv_multiCounter[currCounter].L_bv_add--;
#endif
	return (L_var_out);
}

/*___________________________________________________________________________
 |
 |   Function Name : bv_L_msu0
 |
 |   Purpose :
 |
 |   Multiply var1 by var2 (without left shift) and bv_subtract the 32 bit
 |   result to L_var3 with saturation, return a 32 bit result:
 |        bv_L_msu0(L_var3,var1,var2) = L_bv_sub(L_var3,(L_bv_mult0(var1,var2)).
 |
 |   Complexity weight : 1
 |
 |   Inputs :
 |
 |    L_var3   32 bit long signed integer (int32_t) whose value falls in the
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.
 |
 |    var1     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |    var2     16 bit short signed integer (int16_t) whose value falls in the
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.
 |
 |   Return Value :
 |
 |    L_var_out
 |             32 bit long signed integer (int32_t) whose value falls in the
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.
 |___________________________________________________________________________
*/
int32_t bv_L_msu0(int32_t L_var3, int16_t var1, int16_t var2)
{
	int32_t L_var_out;
	int32_t L_product;

	L_product = L_bv_mult0(var1, var2);
	L_var_out = L_bv_sub(L_var3, L_product);

#if (WMOPS)
	bv_multiCounter[currCounter].bv_L_msu0++;
	bv_multiCounter[currCounter].L_bv_mult0--;
	bv_multiCounter[currCounter].L_bv_sub--;
#endif
	return (L_var_out);
}

/* end of file */
