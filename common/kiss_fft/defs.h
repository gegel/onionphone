/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdint.h>

typedef int16_t spx_int16_t;
typedef uint16_t spx_uint16_t;
typedef int32_t spx_int32_t;
typedef uint32_t spx_uint32_t;

#define NEG16(x) (-(x))
#define NEG32(x) (-(x))
#define MIN16(a,b) ((a) < (b) ? (a) : (b))   /**< Maximum 16-bit value.   */

#ifdef FIXED_POINT

typedef spx_int16_t spx_word16_t;
typedef spx_int32_t spx_word32_t;
typedef spx_word32_t spx_mem_t;
typedef spx_word16_t spx_coef_t;
typedef spx_word16_t spx_lsp_t;
typedef spx_word32_t spx_sig_t;

#define SHR32(a,shift) ((a) >> (shift))
#define SUB32(a,b) ((spx_word32_t)(a)-(spx_word32_t)(b))
#define MULT16_16(a,b)     (((spx_word32_t)(spx_word16_t)(a))*((spx_word32_t)(spx_word16_t)(b)))
#define ADD32(a,b) ((spx_word32_t)(a)+(spx_word32_t)(b))
#define EXTEND32(x) ((spx_word32_t)(x))
#define PSHR32(a,shift) (SHR32((a)+((EXTEND32(1)<<((shift))>>1)),shift))
#define SHL32(a,shift) ((a) << (shift))
#define PSHR16(a,shift) (SHR16((a)+((1<<((shift))>>1)),shift))
#define EXTRACT16(x) ((spx_word16_t)(x))
#define VSHR32(a, shift) (((shift)>0) ? SHR32(a, shift) : SHL32(a, -(shift)))
#define ADD16(a,b) ((spx_word16_t)((spx_word16_t)(a)+(spx_word16_t)(b)))
#define MULT16_16_Q14(a,b) (SHR(MULT16_16((a),(b)),14))
#define SHR(a,shift) ((a) >> (shift))
#define SUB16(a,b) ((spx_word16_t)(a)-(spx_word16_t)(b))
#define MAC16_16_Q13(c,a,b)     (ADD32((c),SHR(MULT16_16((a),(b)),13)))
#define MULT16_16_P13(a,b) (SHR(ADD32(4096,MULT16_16((a),(b))),13))
#define MULT16_16_P15(a,b) (SHR(ADD32(16384,MULT16_16((a),(b))),15))
#define MULT16_16_Q13(a,b) (SHR(MULT16_16((a),(b)),13))
#define SHR16(a,shift) ((a) >> (shift))
#define SHL16(a,shift) ((a) << (shift))
#define MULT16_16_P14(a,b) (SHR(ADD32(8192,MULT16_16((a),(b))),14))
#define DIV32_16(a,b) ((spx_word16_t)(((spx_word32_t)(a))/((spx_word16_t)(b))))
#define DIV32(a,b) (((spx_word32_t)(a))/((spx_word32_t)(b)))

#else

typedef float spx_mem_t;
typedef float spx_coef_t;
typedef float spx_lsp_t;
typedef float spx_sig_t;
typedef float spx_word16_t;
typedef float spx_word32_t;

#define SHR32(a,shift) (a)
#define SUB32(a,b) ((a)-(b))
#define MULT16_16(a,b)     ((spx_word32_t)(a)*(spx_word32_t)(b))
#define ADD32(a,b) ((a)+(b))
#define EXTEND32(x) (x)
#define PSHR32(a,shift) (a)
#define SHL32(a,shift) (a)
#define SHR16(a,shift) (a)
#define PSHR16(a,shift) (a)
#define EXTRACT16(x) (x)
#define VSHR32(a,shift) (a)
#define ADD16(a,b) ((a)+(b))
#define MULT16_16_Q14(a,b)     ((a)*(b))
#define SHR(a,shift)       (a)
#define SUB16(a,b) ((a)-(b))
#define MULT16_16_Q13(a,b)     ((a)*(b))
#define MULT16_16_P13(a,b)     ((a)*(b))
#define MULT16_16_P15(a,b)     ((a)*(b))
#define MAC16_16_Q13(c,a,b)     ((c)+(a)*(b))
#define SHL16(a,shift) (a)
#define MULT16_16_P14(a,b)     ((a)*(b))
#define DIV32_16(a,b)     (((spx_word32_t)(a))/(spx_word16_t)(b))
#define DIV32(a,b)     (((spx_word32_t)(a))/(spx_word32_t)(b))

#endif /* FIXED_POINT */

#endif /* _DEFS_H_ */

