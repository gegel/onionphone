/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).


*/

/*
   spbstd.h   SPB standard header file.

   Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

#ifndef _spbstd_h
#define _spbstd_h

/*
** Needed include files.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ophmconsts.h>

/* OSTYPE-dependent definitions/macros. */

#ifdef SunOS4

/* some standard C function definitions missing from SunOS4 */
extern int fclose(FILE * stream);
extern int fprintf(FILE * stream, const char *format, ...);
extern size_t fread(void *ptr, size_t size, size_t nobj, FILE * stream);
extern int fseek(FILE * stream, long offset, int origin);
extern size_t fwrite(const void *ptr, size_t size, size_t nobj, FILE * stream);
extern int printf(const char *format, ...);
extern long random(void);
extern int sscanf(char *s, const char *format, ...);
extern void rewind(FILE * stream);

#else

#endif

/*
** Constant definitions.
*/
#ifndef FALSE
#define FALSE           0
#endif
#ifndef TRUE
#define TRUE            1
#endif

/*
** Macros.
*/

#ifndef SQR
#define SQR(x)          ((x)*(x))
#endif

/* lint-dependent macros. */

#ifdef lint
#define VA_ARG(v,type) (v,(type)NULL)
#else
#define VA_ARG(v,type) va_arg(v,type)
#endif

#endif				/* #ifndef _spbstd_h */
