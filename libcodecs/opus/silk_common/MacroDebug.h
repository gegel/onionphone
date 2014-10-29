/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Copyright (C) 2012 Xiph.Org Foundation
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

#ifndef MACRO_DEBUG_H
#define MACRO_DEBUG_H

/* Redefine macro functions with extensive assertion in DEBUG mode.
   As functions can't be undefined, this file can't work with SigProcFIX_MacroCount.h */

#if ( defined (FIXED_DEBUG) || ( 0 && defined (_DEBUG) ) ) && !defined (silk_MACRO_COUNT)

#undef silk_ADD16
#define silk_ADD16(a,b) silk_ADD16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_ADD16_(int16_t a, int16_t b, char *file,
				     int line)
{
	int16_t ret;

	ret = a + b;
	if (ret != silk_ADD_SAT16(a, b)) {
		fprintf(stderr, "silk_ADD16(%d, %d) in %s: line %d\n", a, b,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_ADD32
#define silk_ADD32(a,b) silk_ADD32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_ADD32_(int32_t a, int32_t b, char *file,
				     int line)
{
	int32_t ret;

	ret = a + b;
	if (ret != silk_ADD_SAT32(a, b)) {
		fprintf(stderr, "silk_ADD32(%d, %d) in %s: line %d\n", a, b,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_ADD64
#define silk_ADD64(a,b) silk_ADD64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_ADD64_(int64_t a, int64_t b, char *file,
				     int line)
{
	int64_t ret;

	ret = a + b;
	if (ret != silk_ADD_SAT64(a, b)) {
		fprintf(stderr, "silk_ADD64(%lld, %lld) in %s: line %d\n",
			(long long)a, (long long)b, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SUB16
#define silk_SUB16(a,b) silk_SUB16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_SUB16_(int16_t a, int16_t b, char *file,
				     int line)
{
	int16_t ret;

	ret = a - b;
	if (ret != silk_SUB_SAT16(a, b)) {
		fprintf(stderr, "silk_SUB16(%d, %d) in %s: line %d\n", a, b,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SUB32
#define silk_SUB32(a,b) silk_SUB32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_SUB32_(int32_t a, int32_t b, char *file,
				     int line)
{
	int32_t ret;

	ret = a - b;
	if (ret != silk_SUB_SAT32(a, b)) {
		fprintf(stderr, "silk_SUB32(%d, %d) in %s: line %d\n", a, b,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SUB64
#define silk_SUB64(a,b) silk_SUB64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_SUB64_(int64_t a, int64_t b, char *file,
				     int line)
{
	int64_t ret;

	ret = a - b;
	if (ret != silk_SUB_SAT64(a, b)) {
		fprintf(stderr, "silk_SUB64(%lld, %lld) in %s: line %d\n",
			(long long)a, (long long)b, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_ADD_SAT16
#define silk_ADD_SAT16(a,b) silk_ADD_SAT16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_ADD_SAT16_(int16_t a16, int16_t b16,
					 char *file, int line)
{
	int16_t res;
	res = (int16_t) silk_SAT16(silk_ADD32((int32_t) (a16), (b16)));
	if (res != silk_SAT16((int32_t) a16 + (int32_t) b16)) {
		fprintf(stderr, "silk_ADD_SAT16(%d, %d) in %s: line %d\n", a16,
			b16, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_ADD_SAT32
#define silk_ADD_SAT32(a,b) silk_ADD_SAT32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_ADD_SAT32_(int32_t a32, int32_t b32,
					 char *file, int line)
{
	int32_t res;
	res = ((((uint32_t) (a32) + (uint32_t) (b32)) & 0x80000000) == 0 ?
	       ((((a32) & (b32)) & 0x80000000) !=
		0 ? silk_int32_MIN : (a32) +
		(b32)) : ((((a32) | (b32)) & 0x80000000) ==
			  0 ? silk_int32_MAX : (a32) + (b32)));
	if (res != silk_SAT32((int64_t) a32 + (int64_t) b32)) {
		fprintf(stderr, "silk_ADD_SAT32(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_ADD_SAT64
#define silk_ADD_SAT64(a,b) silk_ADD_SAT64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_ADD_SAT64_(int64_t a64, int64_t b64,
					 char *file, int line)
{
	int64_t res;
	int fail = 0;
	res = ((((a64) + (b64)) & 0x8000000000000000LL) == 0 ?
	       ((((a64) & (b64)) & 0x8000000000000000LL) !=
		0 ? silk_int64_MIN : (a64) +
		(b64)) : ((((a64) | (b64)) & 0x8000000000000000LL) ==
			  0 ? silk_int64_MAX : (a64) + (b64)));
	if (res != a64 + b64) {
		/* Check that we saturated to the correct extreme value */
		if (!
		    ((res == silk_int64_MAX
		      && ((a64 >> 1) + (b64 >> 1) > (silk_int64_MAX >> 3)))
		     || (res == silk_int64_MIN
			 && ((a64 >> 1) + (b64 >> 1) <
			     (silk_int64_MIN >> 3))))) {
			fail = 1;
		}
	} else {
		/* Saturation not necessary */
		fail = res != a64 + b64;
	}
	if (fail) {
		fprintf(stderr, "silk_ADD_SAT64(%lld, %lld) in %s: line %d\n",
			(long long)a64, (long long)b64, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_SUB_SAT16
#define silk_SUB_SAT16(a,b) silk_SUB_SAT16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_SUB_SAT16_(int16_t a16, int16_t b16,
					 char *file, int line)
{
	int16_t res;
	res = (int16_t) silk_SAT16(silk_SUB32((int32_t) (a16), (b16)));
	if (res != silk_SAT16((int32_t) a16 - (int32_t) b16)) {
		fprintf(stderr, "silk_SUB_SAT16(%d, %d) in %s: line %d\n", a16,
			b16, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_SUB_SAT32
#define silk_SUB_SAT32(a,b) silk_SUB_SAT32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_SUB_SAT32_(int32_t a32, int32_t b32,
					 char *file, int line)
{
	int32_t res;
	res = ((((uint32_t) (a32) - (uint32_t) (b32)) & 0x80000000) == 0 ?
	       (((a32) & ((b32) ^ 0x80000000) & 0x80000000) ? silk_int32_MIN
		: (a32) -
		(b32)) : ((((a32) ^ 0x80000000) & (b32) & 0x80000000) ?
			  silk_int32_MAX : (a32) - (b32)));
	if (res != silk_SAT32((int64_t) a32 - (int64_t) b32)) {
		fprintf(stderr, "silk_SUB_SAT32(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_SUB_SAT64
#define silk_SUB_SAT64(a,b) silk_SUB_SAT64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_SUB_SAT64_(int64_t a64, int64_t b64,
					 char *file, int line)
{
	int64_t res;
	int fail = 0;
	res = ((((a64) - (b64)) & 0x8000000000000000LL) == 0 ?
	       (((a64) & ((b64) ^ 0x8000000000000000LL) & 0x8000000000000000LL)
		? silk_int64_MIN : (a64) -
		(b64)) : ((((a64) ^ 0x8000000000000000LL) & (b64) &
			   0x8000000000000000LL) ? silk_int64_MAX : (a64) -
			  (b64)));
	if (res != a64 - b64) {
		/* Check that we saturated to the correct extreme value */
		if (!
		    ((res == silk_int64_MAX
		      && ((a64 >> 1) + (b64 >> 1) > (silk_int64_MAX >> 3)))
		     || (res == silk_int64_MIN
			 && ((a64 >> 1) + (b64 >> 1) <
			     (silk_int64_MIN >> 3))))) {
			fail = 1;
		}
	} else {
		/* Saturation not necessary */
		fail = res != a64 - b64;
	}
	if (fail) {
		fprintf(stderr, "silk_SUB_SAT64(%lld, %lld) in %s: line %d\n",
			(long long)a64, (long long)b64, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return res;
}

#undef silk_MUL
#define silk_MUL(a,b) silk_MUL_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_MUL_(int32_t a32, int32_t b32, char *file,
				   int line)
{
	int32_t ret;
	int64_t ret64;
	ret = a32 * b32;
	ret64 = (int64_t) a32 *(int64_t) b32;
	if ((int64_t) ret != ret64) {
		fprintf(stderr, "silk_MUL(%d, %d) in %s: line %d\n", a32, b32,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_MUL_uint
#define silk_MUL_uint(a,b) silk_MUL_uint_((a), (b), __FILE__, __LINE__)
static inline uint32_t silk_MUL_uint_(uint32_t a32, uint32_t b32,
					 char *file, int line)
{
	uint32_t ret;
	ret = a32 * b32;
	if ((uint64_t) ret != (uint64_t) a32 * (uint64_t) b32) {
		fprintf(stderr, "silk_MUL_uint(%u, %u) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_MLA
#define silk_MLA(a,b,c) silk_MLA_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_MLA_(int32_t a32, int32_t b32,
				   int32_t c32, char *file, int line)
{
	int32_t ret;
	ret = a32 + b32 * c32;
	if ((int64_t) ret !=
	    (int64_t) a32 + (int64_t) b32 * (int64_t) c32) {
		fprintf(stderr, "silk_MLA(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_MLA_uint
#define silk_MLA_uint(a,b,c) silk_MLA_uint_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_MLA_uint_(uint32_t a32, uint32_t b32,
					uint32_t c32, char *file, int line)
{
	uint32_t ret;
	ret = a32 + b32 * c32;
	if ((int64_t) ret !=
	    (int64_t) a32 + (int64_t) b32 * (int64_t) c32) {
		fprintf(stderr, "silk_MLA_uint(%d, %d, %d) in %s: line %d\n",
			a32, b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMULWB
#define silk_SMULWB(a,b) silk_SMULWB_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_SMULWB_(int32_t a32, int32_t b32,
				      char *file, int line)
{
	int32_t ret;
	ret =
	    (a32 >> 16) * (int32_t) ((int16_t) b32) +
	    (((a32 & 0x0000FFFF) * (int32_t) ((int16_t) b32)) >> 16);
	if ((int64_t) ret != ((int64_t) a32 * (int16_t) b32) >> 16) {
		fprintf(stderr, "silk_SMULWB(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMLAWB
#define silk_SMLAWB(a,b,c) silk_SMLAWB_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLAWB_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret;
	ret = silk_ADD32(a32, silk_SMULWB(b32, c32));
	if (silk_ADD32(a32, silk_SMULWB(b32, c32)) !=
	    silk_ADD_SAT32(a32, silk_SMULWB(b32, c32))) {
		fprintf(stderr, "silk_SMLAWB(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMULWT
#define silk_SMULWT(a,b) silk_SMULWT_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_SMULWT_(int32_t a32, int32_t b32,
				      char *file, int line)
{
	int32_t ret;
	ret =
	    (a32 >> 16) * (b32 >> 16) +
	    (((a32 & 0x0000FFFF) * (b32 >> 16)) >> 16);
	if ((int64_t) ret != ((int64_t) a32 * (b32 >> 16)) >> 16) {
		fprintf(stderr, "silk_SMULWT(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMLAWT
#define silk_SMLAWT(a,b,c) silk_SMLAWT_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLAWT_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret;
	ret =
	    a32 + ((b32 >> 16) * (c32 >> 16)) +
	    (((b32 & 0x0000FFFF) * ((c32 >> 16)) >> 16));
	if ((int64_t) ret !=
	    (int64_t) a32 + (((int64_t) b32 * (c32 >> 16)) >> 16)) {
		fprintf(stderr, "silk_SMLAWT(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMULL
#define silk_SMULL(a,b) silk_SMULL_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_SMULL_(int64_t a64, int64_t b64, char *file,
				     int line)
{
	int64_t ret64;
	int fail = 0;
	ret64 = a64 * b64;
	if (b64 != 0) {
		fail = a64 != (ret64 / b64);
	} else if (a64 != 0) {
		fail = b64 != (ret64 / a64);
	}
	if (fail) {
		fprintf(stderr, "silk_SMULL(%lld, %lld) in %s: line %d\n",
			(long long)a64, (long long)b64, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret64;
}

/* no checking needed for silk_SMULBB */
#undef silk_SMLABB
#define silk_SMLABB(a,b,c) silk_SMLABB_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLABB_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret;
	ret =
	    a32 +
	    (int32_t) ((int16_t) b32) * (int32_t) ((int16_t) c32);
	if ((int64_t) ret !=
	    (int64_t) a32 + (int64_t) b32 * (int16_t) c32) {
		fprintf(stderr, "silk_SMLABB(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

/* no checking needed for silk_SMULBT */
#undef silk_SMLABT
#define silk_SMLABT(a,b,c) silk_SMLABT_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLABT_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret;
	ret = a32 + ((int32_t) ((int16_t) b32)) * (c32 >> 16);
	if ((int64_t) ret !=
	    (int64_t) a32 + (int64_t) b32 * (c32 >> 16)) {
		fprintf(stderr, "silk_SMLABT(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

/* no checking needed for silk_SMULTT */
#undef silk_SMLATT
#define silk_SMLATT(a,b,c) silk_SMLATT_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLATT_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret;
	ret = a32 + (b32 >> 16) * (c32 >> 16);
	if ((int64_t) ret != (int64_t) a32 + (b32 >> 16) * (c32 >> 16)) {
		fprintf(stderr, "silk_SMLATT(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_SMULWW
#define silk_SMULWW(a,b) silk_SMULWW_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_SMULWW_(int32_t a32, int32_t b32,
				      char *file, int line)
{
	int32_t ret, tmp1, tmp2;
	int64_t ret64;
	int fail = 0;

	ret = silk_SMULWB(a32, b32);
	tmp1 = silk_RSHIFT_ROUND(b32, 16);
	tmp2 = silk_MUL(a32, tmp1);

	fail |= (int64_t) tmp2 != (int64_t) a32 *(int64_t) tmp1;

	tmp1 = ret;
	ret = silk_ADD32(tmp1, tmp2);
	fail |= silk_ADD32(tmp1, tmp2) != silk_ADD_SAT32(tmp1, tmp2);

	ret64 = silk_RSHIFT64(silk_SMULL(a32, b32), 16);
	fail |= (int64_t) ret != ret64;

	if (fail) {
		fprintf(stderr, "silk_SMULWT(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}

	return ret;
}

#undef silk_SMLAWW
#define silk_SMLAWW(a,b,c) silk_SMLAWW_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SMLAWW_(int32_t a32, int32_t b32,
				      int32_t c32, char *file, int line)
{
	int32_t ret, tmp;

	tmp = silk_SMULWW(b32, c32);
	ret = silk_ADD32(a32, tmp);
	if (ret != silk_ADD_SAT32(a32, tmp)) {
		fprintf(stderr, "silk_SMLAWW(%d, %d, %d) in %s: line %d\n", a32,
			b32, c32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

/* Multiply-accumulate macros that allow overflow in the addition (ie, no asserts in debug mode) */
#undef  silk_MLA_ovflw
#define silk_MLA_ovflw(a32, b32, c32)    ((a32) + ((b32) * (c32)))
#undef  silk_SMLABB_ovflw
#define silk_SMLABB_ovflw(a32, b32, c32)    ((a32) + ((int32_t)((int16_t)(b32))) * (int32_t)((int16_t)(c32)))

/* no checking needed for silk_SMULL
   no checking needed for silk_SMLAL
   no checking needed for silk_SMLALBB
   no checking needed for SigProcFIX_CLZ16
   no checking needed for SigProcFIX_CLZ32*/

#undef silk_DIV32
#define silk_DIV32(a,b) silk_DIV32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_DIV32_(int32_t a32, int32_t b32, char *file,
				     int line)
{
	if (b32 == 0) {
		fprintf(stderr, "silk_DIV32(%d, %d) in %s: line %d\n", a32, b32,
			file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a32 / b32;
}

#undef silk_DIV32_16
#define silk_DIV32_16(a,b) silk_DIV32_16_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_DIV32_16_(int32_t a32, int32_t b32,
					char *file, int line)
{
	int fail = 0;
	fail |= b32 == 0;
	fail |= b32 > silk_int16_MAX;
	fail |= b32 < silk_int16_MIN;
	if (fail) {
		fprintf(stderr, "silk_DIV32_16(%d, %d) in %s: line %d\n", a32,
			b32, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a32 / b32;
}

/* no checking needed for silk_SAT8
   no checking needed for silk_SAT16
   no checking needed for silk_SAT32
   no checking needed for silk_POS_SAT32
   no checking needed for silk_ADD_POS_SAT8
   no checking needed for silk_ADD_POS_SAT16
   no checking needed for silk_ADD_POS_SAT32
   no checking needed for silk_ADD_POS_SAT64 */

#undef silk_LSHIFT8
#define silk_LSHIFT8(a,b) silk_LSHIFT8_((a), (b), __FILE__, __LINE__)
static inline int8_t silk_LSHIFT8_(int8_t a, int32_t shift, char *file,
				      int line)
{
	int8_t ret;
	int fail = 0;
	ret = a << shift;
	fail |= shift < 0;
	fail |= shift >= 8;
	fail |= (int64_t) ret != ((int64_t) a) << shift;
	if (fail) {
		fprintf(stderr, "silk_LSHIFT8(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_LSHIFT16
#define silk_LSHIFT16(a,b) silk_LSHIFT16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_LSHIFT16_(int16_t a, int32_t shift,
					char *file, int line)
{
	int16_t ret;
	int fail = 0;
	ret = a << shift;
	fail |= shift < 0;
	fail |= shift >= 16;
	fail |= (int64_t) ret != ((int64_t) a) << shift;
	if (fail) {
		fprintf(stderr, "silk_LSHIFT16(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_LSHIFT32
#define silk_LSHIFT32(a,b) silk_LSHIFT32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_LSHIFT32_(int32_t a, int32_t shift,
					char *file, int line)
{
	int32_t ret;
	int fail = 0;
	ret = a << shift;
	fail |= shift < 0;
	fail |= shift >= 32;
	fail |= (int64_t) ret != ((int64_t) a) << shift;
	if (fail) {
		fprintf(stderr, "silk_LSHIFT32(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_LSHIFT64
#define silk_LSHIFT64(a,b) silk_LSHIFT64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_LSHIFT64_(int64_t a, int shift,
					char *file, int line)
{
	int64_t ret;
	int fail = 0;
	ret = a << shift;
	fail |= shift < 0;
	fail |= shift >= 64;
	fail |= (ret >> shift) != ((int64_t) a);
	if (fail) {
		fprintf(stderr, "silk_LSHIFT64(%lld, %d) in %s: line %d\n",
			(long long)a, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_LSHIFT_ovflw
#define silk_LSHIFT_ovflw(a,b) silk_LSHIFT_ovflw_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_LSHIFT_ovflw_(int32_t a, int32_t shift,
					    char *file, int line)
{
	if ((shift < 0) || (shift >= 32)) {	/* no check for overflow */
		fprintf(stderr, "silk_LSHIFT_ovflw(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a << shift;
}

#undef silk_LSHIFT_uint
#define silk_LSHIFT_uint(a,b) silk_LSHIFT_uint_((a), (b), __FILE__, __LINE__)
static inline uint32_t silk_LSHIFT_uint_(uint32_t a, int32_t shift,
					    char *file, int line)
{
	uint32_t ret;
	ret = a << shift;
	if ((shift < 0) || ((int64_t) ret != ((int64_t) a) << shift)) {
		fprintf(stderr, "silk_LSHIFT_uint(%u, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_RSHIFT8
#define silk_RSHITF8(a,b) silk_RSHIFT8_((a), (b), __FILE__, __LINE__)
static inline int8_t silk_RSHIFT8_(int8_t a, int32_t shift, char *file,
				      int line)
{
	if ((shift < 0) || (shift >= 8)) {
		fprintf(stderr, "silk_RSHITF8(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a >> shift;
}

#undef silk_RSHIFT16
#define silk_RSHITF16(a,b) silk_RSHIFT16_((a), (b), __FILE__, __LINE__)
static inline int16_t silk_RSHIFT16_(int16_t a, int32_t shift,
					char *file, int line)
{
	if ((shift < 0) || (shift >= 16)) {
		fprintf(stderr, "silk_RSHITF16(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a >> shift;
}

#undef silk_RSHIFT32
#define silk_RSHIFT32(a,b) silk_RSHIFT32_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_RSHIFT32_(int32_t a, int32_t shift,
					char *file, int line)
{
	if ((shift < 0) || (shift >= 32)) {
		fprintf(stderr, "silk_RSHITF32(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a >> shift;
}

#undef silk_RSHIFT64
#define silk_RSHIFT64(a,b) silk_RSHIFT64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_RSHIFT64_(int64_t a, int64_t shift,
					char *file, int line)
{
	if ((shift < 0) || (shift >= 64)) {
		fprintf(stderr, "silk_RSHITF64(%lld, %lld) in %s: line %d\n",
			(long long)a, (long long)shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a >> shift;
}

#undef silk_RSHIFT_uint
#define silk_RSHIFT_uint(a,b) silk_RSHIFT_uint_((a), (b), __FILE__, __LINE__)
static inline uint32_t silk_RSHIFT_uint_(uint32_t a, int32_t shift,
					    char *file, int line)
{
	if ((shift < 0) || (shift > 32)) {
		fprintf(stderr, "silk_RSHIFT_uint(%u, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return a >> shift;
}

#undef silk_ADD_LSHIFT
#define silk_ADD_LSHIFT(a,b,c) silk_ADD_LSHIFT_((a), (b), (c), __FILE__, __LINE__)
static inline int silk_ADD_LSHIFT_(int a, int b, int shift, char *file,
				   int line)
{
	int16_t ret;
	ret = a + (b << shift);
	if ((shift < 0) || (shift > 15)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) << shift))) {
		fprintf(stderr, "silk_ADD_LSHIFT(%d, %d, %d) in %s: line %d\n",
			a, b, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift >= 0 */
}

#undef silk_ADD_LSHIFT32
#define silk_ADD_LSHIFT32(a,b,c) silk_ADD_LSHIFT32_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_ADD_LSHIFT32_(int32_t a, int32_t b,
					    int32_t shift, char *file,
					    int line)
{
	int32_t ret;
	ret = a + (b << shift);
	if ((shift < 0) || (shift > 31)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) << shift))) {
		fprintf(stderr,
			"silk_ADD_LSHIFT32(%d, %d, %d) in %s: line %d\n", a, b,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift >= 0 */
}

#undef silk_ADD_LSHIFT_uint
#define silk_ADD_LSHIFT_uint(a,b,c) silk_ADD_LSHIFT_uint_((a), (b), (c), __FILE__, __LINE__)
static inline uint32_t silk_ADD_LSHIFT_uint_(uint32_t a, uint32_t b,
						int32_t shift, char *file,
						int line)
{
	uint32_t ret;
	ret = a + (b << shift);
	if ((shift < 0) || (shift > 32)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) << shift))) {
		fprintf(stderr,
			"silk_ADD_LSHIFT_uint(%u, %u, %d) in %s: line %d\n", a,
			b, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift >= 0 */
}

#undef silk_ADD_RSHIFT
#define silk_ADD_RSHIFT(a,b,c) silk_ADD_RSHIFT_((a), (b), (c), __FILE__, __LINE__)
static inline int silk_ADD_RSHIFT_(int a, int b, int shift, char *file,
				   int line)
{
	int16_t ret;
	ret = a + (b >> shift);
	if ((shift < 0) || (shift > 15)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) >> shift))) {
		fprintf(stderr, "silk_ADD_RSHIFT(%d, %d, %d) in %s: line %d\n",
			a, b, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift  > 0 */
}

#undef silk_ADD_RSHIFT32
#define silk_ADD_RSHIFT32(a,b,c) silk_ADD_RSHIFT32_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_ADD_RSHIFT32_(int32_t a, int32_t b,
					    int32_t shift, char *file,
					    int line)
{
	int32_t ret;
	ret = a + (b >> shift);
	if ((shift < 0) || (shift > 31)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) >> shift))) {
		fprintf(stderr,
			"silk_ADD_RSHIFT32(%d, %d, %d) in %s: line %d\n", a, b,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift  > 0 */
}

#undef silk_ADD_RSHIFT_uint
#define silk_ADD_RSHIFT_uint(a,b,c) silk_ADD_RSHIFT_uint_((a), (b), (c), __FILE__, __LINE__)
static inline uint32_t silk_ADD_RSHIFT_uint_(uint32_t a, uint32_t b,
						int32_t shift, char *file,
						int line)
{
	uint32_t ret;
	ret = a + (b >> shift);
	if ((shift < 0) || (shift > 32)
	    || ((int64_t) ret !=
		(int64_t) a + (((int64_t) b) >> shift))) {
		fprintf(stderr,
			"silk_ADD_RSHIFT_uint(%u, %u, %d) in %s: line %d\n", a,
			b, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift  > 0 */
}

#undef silk_SUB_LSHIFT32
#define silk_SUB_LSHIFT32(a,b,c) silk_SUB_LSHIFT32_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SUB_LSHIFT32_(int32_t a, int32_t b,
					    int32_t shift, char *file,
					    int line)
{
	int32_t ret;
	ret = a - (b << shift);
	if ((shift < 0) || (shift > 31)
	    || ((int64_t) ret !=
		(int64_t) a - (((int64_t) b) << shift))) {
		fprintf(stderr,
			"silk_SUB_LSHIFT32(%d, %d, %d) in %s: line %d\n", a, b,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift >= 0 */
}

#undef silk_SUB_RSHIFT32
#define silk_SUB_RSHIFT32(a,b,c) silk_SUB_RSHIFT32_((a), (b), (c), __FILE__, __LINE__)
static inline int32_t silk_SUB_RSHIFT32_(int32_t a, int32_t b,
					    int32_t shift, char *file,
					    int line)
{
	int32_t ret;
	ret = a - (b >> shift);
	if ((shift < 0) || (shift > 31)
	    || ((int64_t) ret !=
		(int64_t) a - (((int64_t) b) >> shift))) {
		fprintf(stderr,
			"silk_SUB_RSHIFT32(%d, %d, %d) in %s: line %d\n", a, b,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;		/* shift  > 0 */
}

#undef silk_RSHIFT_ROUND
#define silk_RSHIFT_ROUND(a,b) silk_RSHIFT_ROUND_((a), (b), __FILE__, __LINE__)
static inline int32_t silk_RSHIFT_ROUND_(int32_t a, int32_t shift,
					    char *file, int line)
{
	int32_t ret;
	ret = shift == 1 ? (a >> 1) + (a & 1) : ((a >> (shift - 1)) + 1) >> 1;
	/* the marco definition can't handle a shift of zero */
	if ((shift <= 0) || (shift > 31)
	    || ((int64_t) ret !=
		((int64_t) a + ((int64_t) 1 << (shift - 1))) >> shift)) {
		fprintf(stderr, "silk_RSHIFT_ROUND(%d, %d) in %s: line %d\n", a,
			shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return ret;
}

#undef silk_RSHIFT_ROUND64
#define silk_RSHIFT_ROUND64(a,b) silk_RSHIFT_ROUND64_((a), (b), __FILE__, __LINE__)
static inline int64_t silk_RSHIFT_ROUND64_(int64_t a, int32_t shift,
					      char *file, int line)
{
	int64_t ret;
	/* the marco definition can't handle a shift of zero */
	if ((shift <= 0) || (shift >= 64)) {
		fprintf(stderr,
			"silk_RSHIFT_ROUND64(%lld, %d) in %s: line %d\n",
			(long long)a, shift, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	ret = shift == 1 ? (a >> 1) + (a & 1) : ((a >> (shift - 1)) + 1) >> 1;
	return ret;
}

/* silk_abs is used on floats also, so doesn't work... */
/*#undef silk_abs
static inline int32_t silk_abs(int32_t a){
    assert(a != 0x80000000);
    return (((a) >  0)  ? (a) : -(a));            // Be careful, silk_abs returns wrong when input equals to silk_intXX_MIN
}*/

#undef silk_abs_int64
#define silk_abs_int64(a) silk_abs_int64_((a), __FILE__, __LINE__)
static inline int64_t silk_abs_int64_(int64_t a, char *file, int line)
{
	if (a == silk_int64_MIN) {
		fprintf(stderr, "silk_abs_int64(%lld) in %s: line %d\n",
			(long long)a, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return (((a) > 0) ? (a) : -(a));	/* Be careful, silk_abs returns wrong when input equals to silk_intXX_MIN */
}

#undef silk_abs_int32
#define silk_abs_int32(a) silk_abs_int32_((a), __FILE__, __LINE__)
static inline int32_t silk_abs_int32_(int32_t a, char *file, int line)
{
	if (a == silk_int32_MIN) {
		fprintf(stderr, "silk_abs_int32(%d) in %s: line %d\n", a, file,
			line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return silk_abs(a);
}

#undef silk_CHECK_FIT8
#define silk_CHECK_FIT8(a) silk_CHECK_FIT8_((a), __FILE__, __LINE__)
static inline int8_t silk_CHECK_FIT8_(int64_t a, char *file, int line)
{
	int8_t ret;
	ret = (int8_t) a;
	if ((int64_t) ret != a) {
		fprintf(stderr, "silk_CHECK_FIT8(%lld) in %s: line %d\n",
			(long long)a, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return (ret);
}

#undef silk_CHECK_FIT16
#define silk_CHECK_FIT16(a) silk_CHECK_FIT16_((a), __FILE__, __LINE__)
static inline int16_t silk_CHECK_FIT16_(int64_t a, char *file, int line)
{
	int16_t ret;
	ret = (int16_t) a;
	if ((int64_t) ret != a) {
		fprintf(stderr, "silk_CHECK_FIT16(%lld) in %s: line %d\n",
			(long long)a, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return (ret);
}

#undef silk_CHECK_FIT32
#define silk_CHECK_FIT32(a) silk_CHECK_FIT32_((a), __FILE__, __LINE__)
static inline int32_t silk_CHECK_FIT32_(int64_t a, char *file, int line)
{
	int32_t ret;
	ret = (int32_t) a;
	if ((int64_t) ret != a) {
		fprintf(stderr, "silk_CHECK_FIT32(%lld) in %s: line %d\n",
			(long long)a, file, line);
#ifdef FIXED_DEBUG_ASSERT
		assert(0);
#endif
	}
	return (ret);
}

/* no checking for silk_NSHIFT_MUL_32_32
   no checking for silk_NSHIFT_MUL_16_16
   no checking needed for silk_min
   no checking needed for silk_max
   no checking needed for silk_sign
*/

#endif
#endif				/* MACRO_DEBUG_H */
