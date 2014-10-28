/* Copyright (C) 2002-2003 Jean-Marc Valin
   Copyright (C) 2007-2009 Xiph.Org Foundation */
/**
   @file stack_alloc.h
   @brief Temporary memory allocation on stack
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include "opus_types.h"
#include "opus_defines.h"

#if (!defined (VAR_ARRAYS) && !defined (USE_ALLOCA) && !defined (NONTHREADSAFE_PSEUDOSTACK))
#error "Opus requires one of VAR_ARRAYS, USE_ALLOCA, or NONTHREADSAFE_PSEUDOSTACK be defined to select the temporary allocation mode."
#endif

#ifdef USE_ALLOCA
# ifdef WIN32
#  include <malloc.h>
# else
#  ifdef HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   include <stdlib.h>
#  endif
# endif
#endif

#if defined(VAR_ARRAYS)

/* C99 does not allow VLAs of size zero */
#define ALLOC_NONE 1

#elif defined(USE_ALLOCA)
#define ALLOC_NONE 0

#else

#include "os_support.h"
#define ALLOC_NONE 0

#endif /* VAR_ARRAYS */

static OPUS_INLINE int _opus_false(void) {return 0;}

#endif /* STACK_ALLOC_H */
