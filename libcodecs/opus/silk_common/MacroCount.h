/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SIGPROCFIX_API_MACROCOUNT_H
#define SIGPROCFIX_API_MACROCOUNT_H
#include <stdio.h>

#ifdef    silk_MACRO_COUNT
#define varDefine int64_t ops_count = 0;

extern int64_t ops_count;

static inline int64_t silk_SaveCount()
{
	return (ops_count);
}

static inline int64_t silk_SaveResetCount()
{
	int64_t ret;

	ret = ops_count;
	ops_count = 0;
	return (ret);
}

static inline silk_PrintCount()
{
	printf("ops_count = %d \n ", (int32_t) ops_count);
}

#undef silk_MUL
static inline int32_t silk_MUL(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 4;
	ret = a32 * b32;
	return ret;
}

#undef silk_MUL_uint
static inline uint32_t silk_MUL_uint(uint32_t a32, uint32_t b32)
{
	uint32_t ret;
	ops_count += 4;
	ret = a32 * b32;
	return ret;
}

#undef silk_MLA
static inline int32_t silk_MLA(int32_t a32, int32_t b32,
				  int32_t c32)
{
	int32_t ret;
	ops_count += 4;
	ret = a32 + b32 * c32;
	return ret;
}

#undef silk_MLA_uint
static inline int32_t silk_MLA_uint(uint32_t a32, uint32_t b32,
				       uint32_t c32)
{
	uint32_t ret;
	ops_count += 4;
	ret = a32 + b32 * c32;
	return ret;
}

#undef silk_SMULWB
static inline int32_t silk_SMULWB(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 5;
	ret =
	    (a32 >> 16) * (int32_t) ((int16_t) b32) +
	    (((a32 & 0x0000FFFF) * (int32_t) ((int16_t) b32)) >> 16);
	return ret;
}

#undef    silk_SMLAWB
static inline int32_t silk_SMLAWB(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	ops_count += 5;
	ret =
	    ((a32) +
	     ((((b32) >> 16) * (int32_t) ((int16_t) (c32))) +
	      ((((b32) & 0x0000FFFF) *
		(int32_t) ((int16_t) (c32))) >> 16)));
	return ret;
}

#undef silk_SMULWT
static inline int32_t silk_SMULWT(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 4;
	ret =
	    (a32 >> 16) * (b32 >> 16) +
	    (((a32 & 0x0000FFFF) * (b32 >> 16)) >> 16);
	return ret;
}

#undef silk_SMLAWT
static inline int32_t silk_SMLAWT(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	ops_count += 4;
	ret =
	    a32 + ((b32 >> 16) * (c32 >> 16)) +
	    (((b32 & 0x0000FFFF) * ((c32 >> 16)) >> 16));
	return ret;
}

#undef silk_SMULBB
static inline int32_t silk_SMULBB(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 1;
	ret = (int32_t) ((int16_t) a32) * (int32_t) ((int16_t) b32);
	return ret;
}

#undef silk_SMLABB
static inline int32_t silk_SMLABB(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	ops_count += 1;
	ret =
	    a32 +
	    (int32_t) ((int16_t) b32) * (int32_t) ((int16_t) c32);
	return ret;
}

#undef silk_SMULBT
static inline int32_t silk_SMULBT(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 4;
	ret = ((int32_t) ((int16_t) a32)) * (b32 >> 16);
	return ret;
}

#undef silk_SMLABT
static inline int32_t silk_SMLABT(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	ops_count += 1;
	ret = a32 + ((int32_t) ((int16_t) b32)) * (c32 >> 16);
	return ret;
}

#undef silk_SMULTT
static inline int32_t silk_SMULTT(int32_t a32, int32_t b32)
{
	int32_t ret;
	ops_count += 1;
	ret = (a32 >> 16) * (b32 >> 16);
	return ret;
}

#undef    silk_SMLATT
static inline int32_t silk_SMLATT(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	ops_count += 1;
	ret = a32 + (b32 >> 16) * (c32 >> 16);
	return ret;
}

/* multiply-accumulate macros that allow overflow in the addition (ie, no asserts in debug mode)*/
#undef    silk_MLA_ovflw
#define silk_MLA_ovflw silk_MLA

#undef silk_SMLABB_ovflw
#define silk_SMLABB_ovflw silk_SMLABB

#undef silk_SMLABT_ovflw
#define silk_SMLABT_ovflw silk_SMLABT

#undef silk_SMLATT_ovflw
#define silk_SMLATT_ovflw silk_SMLATT

#undef silk_SMLAWB_ovflw
#define silk_SMLAWB_ovflw silk_SMLAWB

#undef silk_SMLAWT_ovflw
#define silk_SMLAWT_ovflw silk_SMLAWT

#undef silk_SMULL
static inline int64_t silk_SMULL(int32_t a32, int32_t b32)
{
	int64_t ret;
	ops_count += 8;
	ret = ((int64_t) (a32) * /*(int64_t) */ (b32));
	return ret;
}

#undef    silk_SMLAL
static inline int64_t silk_SMLAL(int64_t a64, int32_t b32,
				    int32_t c32)
{
	int64_t ret;
	ops_count += 8;
	ret = a64 + ((int64_t) (b32) * /*(int64_t) */ (c32));
	return ret;
}

#undef    silk_SMLALBB
static inline int64_t silk_SMLALBB(int64_t a64, int16_t b16,
				      int16_t c16)
{
	int64_t ret;
	ops_count += 4;
	ret = a64 + ((int64_t) (b16) * /*(int64_t) */ (c16));
	return ret;
}

#undef    SigProcFIX_CLZ16
static inline int32_t SigProcFIX_CLZ16(int16_t in16)
{
	int32_t out32 = 0;
	ops_count += 10;
	if (in16 == 0) {
		return 16;
	}
	/* test nibbles */
	if (in16 & 0xFF00) {
		if (in16 & 0xF000) {
			in16 >>= 12;
		} else {
			out32 += 4;
			in16 >>= 8;
		}
	} else {
		if (in16 & 0xFFF0) {
			out32 += 8;
			in16 >>= 4;
		} else {
			out32 += 12;
		}
	}
	/* test bits and return */
	if (in16 & 0xC) {
		if (in16 & 0x8)
			return out32 + 0;
		else
			return out32 + 1;
	} else {
		if (in16 & 0xE)
			return out32 + 2;
		else
			return out32 + 3;
	}
}

#undef SigProcFIX_CLZ32
static inline int32_t SigProcFIX_CLZ32(int32_t in32)
{
	/* test highest 16 bits and convert to int16_t */
	ops_count += 2;
	if (in32 & 0xFFFF0000) {
		return SigProcFIX_CLZ16((int16_t) (in32 >> 16));
	} else {
		return SigProcFIX_CLZ16((int16_t) in32) + 16;
	}
}

#undef silk_DIV32
static inline int32_t silk_DIV32(int32_t a32, int32_t b32)
{
	ops_count += 64;
	return a32 / b32;
}

#undef silk_DIV32_16
static inline int32_t silk_DIV32_16(int32_t a32, int32_t b32)
{
	ops_count += 32;
	return a32 / b32;
}

#undef silk_SAT8
static inline int8_t silk_SAT8(int64_t a)
{
	int8_t tmp;
	ops_count += 1;
	tmp = (int8_t) ((a) > silk_int8_MAX ? silk_int8_MAX :
			   ((a) < silk_int8_MIN ? silk_int8_MIN : (a)));
	return (tmp);
}

#undef silk_SAT16
static inline int16_t silk_SAT16(int64_t a)
{
	int16_t tmp;
	ops_count += 1;
	tmp = (int16_t) ((a) > silk_int16_MAX ? silk_int16_MAX :
			    ((a) < silk_int16_MIN ? silk_int16_MIN : (a)));
	return (tmp);
}

#undef silk_SAT32
static inline int32_t silk_SAT32(int64_t a)
{
	int32_t tmp;
	ops_count += 1;
	tmp = (int32_t) ((a) > silk_int32_MAX ? silk_int32_MAX :
			    ((a) < silk_int32_MIN ? silk_int32_MIN : (a)));
	return (tmp);
}

#undef silk_POS_SAT32
static inline int32_t silk_POS_SAT32(int64_t a)
{
	int32_t tmp;
	ops_count += 1;
	tmp = (int32_t) ((a) > silk_int32_MAX ? silk_int32_MAX : (a));
	return (tmp);
}

#undef silk_ADD_POS_SAT8
static inline int8_t silk_ADD_POS_SAT8(int64_t a, int64_t b)
{
	int8_t tmp;
	ops_count += 1;
	tmp = (int8_t) ((((a) + (b)) & 0x80) ? silk_int8_MAX : ((a) + (b)));
	return (tmp);
}

#undef silk_ADD_POS_SAT16
static inline int16_t silk_ADD_POS_SAT16(int64_t a, int64_t b)
{
	int16_t tmp;
	ops_count += 1;
	tmp =
	    (int16_t) ((((a) + (b)) & 0x8000) ? silk_int16_MAX : ((a) +
								     (b)));
	return (tmp);
}

#undef silk_ADD_POS_SAT32
static inline int32_t silk_ADD_POS_SAT32(int64_t a, int64_t b)
{
	int32_t tmp;
	ops_count += 1;
	tmp =
	    (int32_t) ((((a) + (b)) & 0x80000000) ? silk_int32_MAX : ((a) +
									 (b)));
	return (tmp);
}

#undef silk_ADD_POS_SAT64
static inline int64_t silk_ADD_POS_SAT64(int64_t a, int64_t b)
{
	int64_t tmp;
	ops_count += 1;
	tmp =
	    ((((a) + (b)) & 0x8000000000000000LL) ? silk_int64_MAX : ((a) +
								      (b)));
	return (tmp);
}

#undef    silk_LSHIFT8
static inline int8_t silk_LSHIFT8(int8_t a, int32_t shift)
{
	int8_t ret;
	ops_count += 1;
	ret = a << shift;
	return ret;
}

#undef    silk_LSHIFT16
static inline int16_t silk_LSHIFT16(int16_t a, int32_t shift)
{
	int16_t ret;
	ops_count += 1;
	ret = a << shift;
	return ret;
}

#undef    silk_LSHIFT32
static inline int32_t silk_LSHIFT32(int32_t a, int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a << shift;
	return ret;
}

#undef    silk_LSHIFT64
static inline int64_t silk_LSHIFT64(int64_t a, int shift)
{
	ops_count += 1;
	return a << shift;
}

#undef    silk_LSHIFT_ovflw
static inline int32_t silk_LSHIFT_ovflw(int32_t a, int32_t shift)
{
	ops_count += 1;
	return a << shift;
}

#undef    silk_LSHIFT_uint
static inline uint32_t silk_LSHIFT_uint(uint32_t a, int32_t shift)
{
	uint32_t ret;
	ops_count += 1;
	ret = a << shift;
	return ret;
}

#undef    silk_RSHIFT8
static inline int8_t silk_RSHIFT8(int8_t a, int32_t shift)
{
	ops_count += 1;
	return a >> shift;
}

#undef    silk_RSHIFT16
static inline int16_t silk_RSHIFT16(int16_t a, int32_t shift)
{
	ops_count += 1;
	return a >> shift;
}

#undef    silk_RSHIFT32
static inline int32_t silk_RSHIFT32(int32_t a, int32_t shift)
{
	ops_count += 1;
	return a >> shift;
}

#undef    silk_RSHIFT64
static inline int64_t silk_RSHIFT64(int64_t a, int64_t shift)
{
	ops_count += 1;
	return a >> shift;
}

#undef    silk_RSHIFT_uint
static inline uint32_t silk_RSHIFT_uint(uint32_t a, int32_t shift)
{
	ops_count += 1;
	return a >> shift;
}

#undef    silk_ADD_LSHIFT
static inline int32_t silk_ADD_LSHIFT(int32_t a, int32_t b,
					 int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a + (b << shift);
	return ret;		/* shift >= 0 */
}

#undef    silk_ADD_LSHIFT32
static inline int32_t silk_ADD_LSHIFT32(int32_t a, int32_t b,
					   int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a + (b << shift);
	return ret;		/* shift >= 0 */
}

#undef    silk_ADD_LSHIFT_uint
static inline uint32_t silk_ADD_LSHIFT_uint(uint32_t a, uint32_t b,
					       int32_t shift)
{
	uint32_t ret;
	ops_count += 1;
	ret = a + (b << shift);
	return ret;		/* shift >= 0 */
}

#undef    silk_ADD_RSHIFT
static inline int32_t silk_ADD_RSHIFT(int32_t a, int32_t b,
					 int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a + (b >> shift);
	return ret;		/* shift  > 0 */
}

#undef    silk_ADD_RSHIFT32
static inline int32_t silk_ADD_RSHIFT32(int32_t a, int32_t b,
					   int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a + (b >> shift);
	return ret;		/* shift  > 0 */
}

#undef    silk_ADD_RSHIFT_uint
static inline uint32_t silk_ADD_RSHIFT_uint(uint32_t a, uint32_t b,
					       int32_t shift)
{
	uint32_t ret;
	ops_count += 1;
	ret = a + (b >> shift);
	return ret;		/* shift  > 0 */
}

#undef    silk_SUB_LSHIFT32
static inline int32_t silk_SUB_LSHIFT32(int32_t a, int32_t b,
					   int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a - (b << shift);
	return ret;		/* shift >= 0 */
}

#undef    silk_SUB_RSHIFT32
static inline int32_t silk_SUB_RSHIFT32(int32_t a, int32_t b,
					   int32_t shift)
{
	int32_t ret;
	ops_count += 1;
	ret = a - (b >> shift);
	return ret;		/* shift  > 0 */
}

#undef    silk_RSHIFT_ROUND
static inline int32_t silk_RSHIFT_ROUND(int32_t a, int32_t shift)
{
	int32_t ret;
	ops_count += 3;
	ret = shift == 1 ? (a >> 1) + (a & 1) : ((a >> (shift - 1)) + 1) >> 1;
	return ret;
}

#undef    silk_RSHIFT_ROUND64
static inline int64_t silk_RSHIFT_ROUND64(int64_t a, int32_t shift)
{
	int64_t ret;
	ops_count += 6;
	ret = shift == 1 ? (a >> 1) + (a & 1) : ((a >> (shift - 1)) + 1) >> 1;
	return ret;
}

#undef    silk_abs_int64
static inline int64_t silk_abs_int64(int64_t a)
{
	ops_count += 1;
	return (((a) > 0) ? (a) : -(a));	/* Be careful, silk_abs returns wrong when input equals to silk_intXX_MIN */
}

#undef    silk_abs_int32
static inline int32_t silk_abs_int32(int32_t a)
{
	ops_count += 1;
	return silk_abs(a);
}

#undef silk_min
static silk_min(a, b)
{
	ops_count += 1;
	return (((a) < (b)) ? (a) : (b));
}

#undef silk_max
static silk_max(a, b)
{
	ops_count += 1;
	return (((a) > (b)) ? (a) : (b));
}

#undef silk_sign
static silk_sign(a)
{
	ops_count += 1;
	return ((a) > 0 ? 1 : ((a) < 0 ? -1 : 0));
}

#undef    silk_ADD16
static inline int16_t silk_ADD16(int16_t a, int16_t b)
{
	int16_t ret;
	ops_count += 1;
	ret = a + b;
	return ret;
}

#undef    silk_ADD32
static inline int32_t silk_ADD32(int32_t a, int32_t b)
{
	int32_t ret;
	ops_count += 1;
	ret = a + b;
	return ret;
}

#undef    silk_ADD64
static inline int64_t silk_ADD64(int64_t a, int64_t b)
{
	int64_t ret;
	ops_count += 2;
	ret = a + b;
	return ret;
}

#undef    silk_SUB16
static inline int16_t silk_SUB16(int16_t a, int16_t b)
{
	int16_t ret;
	ops_count += 1;
	ret = a - b;
	return ret;
}

#undef    silk_SUB32
static inline int32_t silk_SUB32(int32_t a, int32_t b)
{
	int32_t ret;
	ops_count += 1;
	ret = a - b;
	return ret;
}

#undef    silk_SUB64
static inline int64_t silk_SUB64(int64_t a, int64_t b)
{
	int64_t ret;
	ops_count += 2;
	ret = a - b;
	return ret;
}

#undef silk_ADD_SAT16
static inline int16_t silk_ADD_SAT16(int16_t a16, int16_t b16)
{
	int16_t res;
	/* Nb will be counted in AKP_add32 and silk_SAT16 */
	res = (int16_t) silk_SAT16(silk_ADD32((int32_t) (a16), (b16)));
	return res;
}

#undef silk_ADD_SAT32
static inline int32_t silk_ADD_SAT32(int32_t a32, int32_t b32)
{
	int32_t res;
	ops_count += 1;
	res = ((((a32) + (b32)) & 0x80000000) == 0 ?
	       ((((a32) & (b32)) & 0x80000000) !=
		0 ? silk_int32_MIN : (a32) +
		(b32)) : ((((a32) | (b32)) & 0x80000000) ==
			  0 ? silk_int32_MAX : (a32) + (b32)));
	return res;
}

#undef silk_ADD_SAT64
static inline int64_t silk_ADD_SAT64(int64_t a64, int64_t b64)
{
	int64_t res;
	ops_count += 1;
	res = ((((a64) + (b64)) & 0x8000000000000000LL) == 0 ?
	       ((((a64) & (b64)) & 0x8000000000000000LL) !=
		0 ? silk_int64_MIN : (a64) +
		(b64)) : ((((a64) | (b64)) & 0x8000000000000000LL) ==
			  0 ? silk_int64_MAX : (a64) + (b64)));
	return res;
}

#undef silk_SUB_SAT16
static inline int16_t silk_SUB_SAT16(int16_t a16, int16_t b16)
{
	int16_t res;
	assert(0);
	/* Nb will be counted in sub-macros */
	res = (int16_t) silk_SAT16(silk_SUB32((int32_t) (a16), (b16)));
	return res;
}

#undef silk_SUB_SAT32
static inline int32_t silk_SUB_SAT32(int32_t a32, int32_t b32)
{
	int32_t res;
	ops_count += 1;
	res = ((((a32) - (b32)) & 0x80000000) == 0 ?
	       (((a32) & ((b32) ^ 0x80000000) & 0x80000000) ? silk_int32_MIN
		: (a32) -
		(b32)) : ((((a32) ^ 0x80000000) & (b32) & 0x80000000) ?
			  silk_int32_MAX : (a32) - (b32)));
	return res;
}

#undef silk_SUB_SAT64
static inline int64_t silk_SUB_SAT64(int64_t a64, int64_t b64)
{
	int64_t res;
	ops_count += 1;
	res = ((((a64) - (b64)) & 0x8000000000000000LL) == 0 ?
	       (((a64) & ((b64) ^ 0x8000000000000000LL) & 0x8000000000000000LL)
		? silk_int64_MIN : (a64) -
		(b64)) : ((((a64) ^ 0x8000000000000000LL) & (b64) &
			   0x8000000000000000LL) ? silk_int64_MAX : (a64) -
			  (b64)));

	return res;
}

#undef    silk_SMULWW
static inline int32_t silk_SMULWW(int32_t a32, int32_t b32)
{
	int32_t ret;
	/* Nb will be counted in sub-macros */
	ret =
	    silk_MLA(silk_SMULWB((a32), (b32)), (a32),
		     silk_RSHIFT_ROUND((b32), 16));
	return ret;
}

#undef    silk_SMLAWW
static inline int32_t silk_SMLAWW(int32_t a32, int32_t b32,
				     int32_t c32)
{
	int32_t ret;
	/* Nb will be counted in sub-macros */
	ret =
	    silk_MLA(silk_SMLAWB((a32), (b32), (c32)), (b32),
		     silk_RSHIFT_ROUND((c32), 16));
	return ret;
}

#undef    silk_min_int
static inline int silk_min_int(int a, int b)
{
	ops_count += 1;
	return (((a) < (b)) ? (a) : (b));
}

#undef    silk_min_16
static inline int16_t silk_min_16(int16_t a, int16_t b)
{
	ops_count += 1;
	return (((a) < (b)) ? (a) : (b));
}

#undef    silk_min_32
static inline int32_t silk_min_32(int32_t a, int32_t b)
{
	ops_count += 1;
	return (((a) < (b)) ? (a) : (b));
}

#undef    silk_min_64
static inline int64_t silk_min_64(int64_t a, int64_t b)
{
	ops_count += 1;
	return (((a) < (b)) ? (a) : (b));
}

/* silk_min() versions with typecast in the function call */
#undef    silk_max_int
static inline int silk_max_int(int a, int b)
{
	ops_count += 1;
	return (((a) > (b)) ? (a) : (b));
}

#undef    silk_max_16
static inline int16_t silk_max_16(int16_t a, int16_t b)
{
	ops_count += 1;
	return (((a) > (b)) ? (a) : (b));
}

#undef    silk_max_32
static inline int32_t silk_max_32(int32_t a, int32_t b)
{
	ops_count += 1;
	return (((a) > (b)) ? (a) : (b));
}

#undef    silk_max_64
static inline int64_t silk_max_64(int64_t a, int64_t b)
{
	ops_count += 1;
	return (((a) > (b)) ? (a) : (b));
}

#undef silk_LIMIT_int
static inline int silk_LIMIT_int(int a, int limit1,
				      int limit2)
{
	int ret;
	ops_count += 6;

	ret =
	    ((limit1) >
	     (limit2) ? ((a) >
			 (limit1) ? (limit1) : ((a) <
						(limit2) ? (limit2) : (a)))
	     : ((a) > (limit2) ? (limit2) : ((a) < (limit1) ? (limit1) : (a))));

	return (ret);
}

#undef silk_LIMIT_16
static inline int16_t silk_LIMIT_16(int16_t a, int16_t limit1,
				       int16_t limit2)
{
	int16_t ret;
	ops_count += 6;

	ret =
	    ((limit1) >
	     (limit2) ? ((a) >
			 (limit1) ? (limit1) : ((a) <
						(limit2) ? (limit2) : (a)))
	     : ((a) > (limit2) ? (limit2) : ((a) < (limit1) ? (limit1) : (a))));

	return (ret);
}

#undef silk_LIMIT_32
static inline int silk_LIMIT_32(int32_t a, int32_t limit1,
				     int32_t limit2)
{
	int32_t ret;
	ops_count += 6;

	ret =
	    ((limit1) >
	     (limit2) ? ((a) >
			 (limit1) ? (limit1) : ((a) <
						(limit2) ? (limit2) : (a)))
	     : ((a) > (limit2) ? (limit2) : ((a) < (limit1) ? (limit1) : (a))));
	return (ret);
}

#else
#define varDefine
#define silk_SaveCount()

#endif
#endif
