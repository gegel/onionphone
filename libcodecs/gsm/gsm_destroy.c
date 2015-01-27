/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /cvsroot/speak-freely-u/speak_freely/gsm/src/gsm_destroy.c,v 1.1.1.1 2002/11/09 12:41:02 johnwalker Exp $ */

#include "config.h"
#ifdef HAS_STRING_H
#include <string.h>
#endif
#ifdef HAS_STDLIB_H
#include <stdlib.h>
#endif
#include "gsm.h"


void gsm_destroy(gsm S)
{
	if (S)
		free((char *)S);
}
