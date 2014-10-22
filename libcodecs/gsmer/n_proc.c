/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#include <stdio.h>
#include <stdlib.h>

void w_proc_head(char *mes)
{
	fprintf(stderr,
		"\n/**************************************************************\n\n");
	fprintf(stderr,
		"     European digital cellular telecommunications system\n");
	fprintf(stderr, "                12200 bits/s w_speech codec for\n");
	fprintf(stderr,
		"          enhanced full rate w_speech traffic channels\n\n");
	fprintf(stderr, "     Bit-Exact C Simulation Code - %s\n", mes);
	fprintf(stderr, "     Version 5.1.0\n");
	fprintf(stderr, "     June 26, 1996\n\n");
	fprintf(stderr,
		"**************************************************************/\n\n");
}
