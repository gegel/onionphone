/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
 *
 *   This file contains functions for the automatic complexity calculation
 *
*************************************************************************/

#include <stdio.h>
#include <stdint.h>
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

static int16_t w_funcid, w_nbframe;
static int32_t w_glob_w_wc, w_wc[NbFuncMax];
static float w_total_wmops;

static int32_t w_LastWOper;

int32_t w_TotalWeightedOperation()
{
	int16_t i;
	int32_t tot, *ptr, *ptr2;

	tot = 0;
	ptr = (int32_t *) & w_counter;
	ptr2 = (int32_t *) & w_op_weight;
	for (i = 0; (unsigned int)i < (sizeof(w_counter) / sizeof(int32_t)); i++)
	{
		tot += ((*ptr++) * (*ptr2++));
	}

	return ((int32_t) tot);
}

int32_t w_DeltaWeightedOperation()
{
	int32_t NewWOper, delta;

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
	int16_t i;

	/* reset function weight operation w_counter variable */

	for (i = 0; i < NbFuncMax; i++)
		w_wc[i] = (int32_t) 0;
	w_glob_w_wc = 0;
	w_nbframe = 0;
	w_total_wmops = 0.0;

}

void Reset_WMOPS_w_counter(void)
{
	int16_t i;
	int32_t *ptr;

	ptr = (int32_t *) & w_counter;
	for (i = 0; (unsigned int)i < (sizeof(w_counter) / sizeof(int32_t)); i++)
	{
		*ptr++ = 0;
	}
	w_LastWOper = 0;

	w_funcid = 0;		/* new frame, set function id to w_zero */
}

int32_t w_fw_wc(void)
{				/* function worst case */
	int32_t tot;

	tot = w_DeltaWeightedOperation();
	if (tot > w_wc[w_funcid])
		w_wc[w_funcid] = tot;

	w_funcid++;

	return (tot);
}

void w_WMOPS_output(int16_t w_dtx_mode)
{
	int16_t i;
	int32_t tot, tot_w_wc;

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
