/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
 *
 *   This file contains functions for the automatic complexity calculation
 *
*************************************************************************/

#include <stdio.h>
#include "ophint.h"
#include "count.h"

/* Global w_counter variable for calculation of complexity weight */

BASIC_OP w_counter;

const BASIC_OP w_op_weight = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 4, 15, 18, 30, 1, 2, 1, 2, 2
};

/* local variable */

#define NbFuncMax  1024

static Word16 w_funcid, w_nbframe;
static Word32 w_glob_w_wc, w_wc[NbFuncMax];
static float w_total_wmops;

static Word32 w_LastWOper;

Word32 w_TotalWeightedOperation()
{
	Word16 i;
	Word32 tot, *ptr, *ptr2;

	tot = 0;
	ptr = (Word32 *) & w_counter;
	ptr2 = (Word32 *) & w_op_weight;
	for (i = 0; (unsigned int)i < (sizeof(w_counter) / sizeof(Word32)); i++)
	{
		tot += ((*ptr++) * (*ptr2++));
	}

	return ((Word32) tot);
}

Word32 w_DeltaWeightedOperation()
{
	Word32 NewWOper, delta;

	NewWOper = w_TotalWeightedOperation();
	delta = NewWOper - w_LastWOper;
	w_LastWOper = NewWOper;
	return (delta);
}

void ww_move16(void)
{
	w_counter.DataMove16++;
}

void ww_move32(void)
{
	w_counter.DataMove32++;
}

void ww_test(void)
{
	w_counter.Test++;
}

void ww_logic16(void)
{
	w_counter.Logic16++;
}

void ww_logic32(void)
{
	w_counter.Logic32++;
}

void Init_WMOPS_w_counter(void)
{
	Word16 i;

	/* reset function weight operation w_counter variable */

	for (i = 0; i < NbFuncMax; i++)
		w_wc[i] = (Word32) 0;
	w_glob_w_wc = 0;
	w_nbframe = 0;
	w_total_wmops = 0.0;

}

void Reset_WMOPS_w_counter(void)
{
	Word16 i;
	Word32 *ptr;

	ptr = (Word32 *) & w_counter;
	for (i = 0; (unsigned int)i < (sizeof(w_counter) / sizeof(Word32)); i++)
	{
		*ptr++ = 0;
	}
	w_LastWOper = 0;

	w_funcid = 0;		/* new frame, set function id to w_zero */
}

Word32 w_fw_wc(void)
{				/* function worst case */
	Word32 tot;

	tot = w_DeltaWeightedOperation();
	if (tot > w_wc[w_funcid])
		w_wc[w_funcid] = tot;

	w_funcid++;

	return (tot);
}

void w_WMOPS_output(Word16 w_dtx_mode)
{
	Word16 i;
	Word32 tot, tot_w_wc;

	tot = w_TotalWeightedOperation();
	if (tot > w_glob_w_wc)
		w_glob_w_wc = tot;

	fprintf(stderr, "WMOPS=%.2f", ((float)tot) * 0.00005);

	w_nbframe++;
	w_total_wmops += ((float)tot) * 0.00005;
	fprintf(stderr, "  Average=%.2f", w_total_wmops / (float)w_nbframe);

	fprintf(stderr, "  WorstCase=%.2f", ((float)w_glob_w_wc) * 0.00005);

	/* Worst worst case printed only when not in DTX mode */
	if (w_dtx_mode == 0) {
		tot_w_wc = 0L;
		for (i = 0; i < w_funcid; i++)
			tot_w_wc += w_wc[i];
		fprintf(stderr, "  WorstWC=%.2f", ((float)tot_w_wc) * 0.00005);
	}
	fprintf(stderr, "\n");
}
