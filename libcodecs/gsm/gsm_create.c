/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include	"config.h"

#include	<stdlib.h>
#ifdef	HAS_STRING_H
#include	<string.h>
#endif
#include <stdio.h>
#include <ophtools.h>

#include "gsm.h"
#include "private.h"
#include "proto.h"

gsm gsm_create P0()
{
	gsm r;

#ifdef	USE_TABLE_MUL

	static int mul_init = 0;
	if (!mul_init) {
		mul_init = 1;
		init_umul_table();
	}
#endif

	r = (gsm) malloc(sizeof(struct gsm_state));
	if (!r)
		return r;

	memzero((char *)r, sizeof(*r));
	r->nrp = 40;

	return r;
}
