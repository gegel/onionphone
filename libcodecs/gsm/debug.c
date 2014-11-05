/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /cvsroot/speak-freely-u/speak_freely/gsm/src/debug.c,v 1.1.1.1 2002/11/09 12:41:01 johnwalker Exp $ */

#include "private.h"

#ifndef	NDEBUG

/* If NDEBUG _is_ defined and no debugging should be performed,
 * calls to functions in this module are #defined to nothing
 * in private.h.
 */

#include <stdio.h>
#include "proto.h"

void gsm_debug_int16_ts P4((name, from, to, ptr),
			char *name, int from, int to, int16_t * ptr)
{
	int nprinted = 0;

	fprintf(stderr, "%s [%d .. %d]: ", name, from, to);
	while (from <= to) {
		fprintf(stderr, "%d ", ptr[from]);
		from++;
		if (nprinted++ >= 7) {
			nprinted = 0;
			if (from < to)
				putc('\n', stderr);
		}
	}
	putc('\n', stderr);
}

void gsm_debug_int32_ts P4((name, from, to, ptr),
			    char *name, int from, int to, int32_t * ptr)
{
	int nprinted = 0;

	fprintf(stderr, "%s [%d .. %d]: ", name, from, to);
	while (from <= to) {

		fprintf(stderr, "%d ", ptr[from]);
		from++;
		if (nprinted++ >= 7) {
			nprinted = 0;
			if (from < to)
				putc('\n', stderr);
		}
	}
	putc('\n', stderr);
}

void gsm_debug_int32_t P2((name, value), char *name, int32_t value)
{
	fprintf(stderr, "%s: %ld\n", name, (long)value);
}

void gsm_debug_int16_t P2((name, value), char *name, int16_t value)
{
	fprintf(stderr, "%s: %ld\n", name, (long)value);
}

#endif
