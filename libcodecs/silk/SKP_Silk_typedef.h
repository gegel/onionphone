/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef _SKP_SILK_API_TYPDEF_H_
#define _SKP_SILK_API_TYPDEF_H_

#ifndef SKP_USE_DOUBLE_PRECISION_FLOATS
#define SKP_USE_DOUBLE_PRECISION_FLOATS     0
#endif

#include <float.h>
#if defined( __GNUC__ )
#include <stdint.h>
#endif

#define int_ptr_size intptr_t

#if SKP_USE_DOUBLE_PRECISION_FLOATS
#define SKP_float      double
#define SKP_float_MAX  DBL_MAX
#else
#define SKP_float      float
#define SKP_float_MAX  FLT_MAX
#endif

#define SKP_INLINE      static __inline

#ifdef _WIN32
#define SKP_STR_CASEINSENSITIVE_COMPARE(x, y) _stricmp(x, y)
#else
#define SKP_STR_CASEINSENSITIVE_COMPARE(x, y) strcasecmp(x, y)
#endif

#define int64_t_MAX   ((int64_t)0x7FFFFFFFFFFFFFFFLL)	//  2^63 - 1
#define int64_t_MIN   ((int64_t)0x8000000000000000LL)	// -2^63
#define int32_t_MAX   0x7FFFFFFF	//  2^31 - 1 =  2147483647
#define int32_t_MIN   ((int32_t)0x80000000)	// -2^31     = -2147483648
#define int16_t_MAX   0x7FFF	//  2^15 - 1 =  32767
#define int16_t_MIN   ((int16_t)0x8000)	// -2^15     = -32768
#define int8_t_MAX    0x7F	//  2^7 - 1  =  127
#define int8_t_MIN    ((int8_t)0x80)	// -2^7      = -128

#define uint32_t_MAX  0xFFFFFFFF	// 2^32 - 1 = 4294967295
#define uint32_t_MIN  0x00000000
#define uint16_t_MAX  0xFFFF	// 2^16 - 1 = 65535
#define uint16_t_MIN  0x0000
#define uint8_t_MAX   0xFF	//  2^8 - 1 = 255
#define uint8_t_MIN   0x00

#define SKP_TRUE        1
#define SKP_FALSE       0

#endif
