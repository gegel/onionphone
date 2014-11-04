/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ================================================================== */
/*                                                                    */
/*    Microsoft Speech coder     ANSI-C Source Code                   */
/*    SC1200 1200 bps speech coder                                    */
/*    Fixed Point Implementation      Version 7.0                     */
/*    Copyright (C) 2000, Microsoft Corp.                             */
/*    All rights reserved.                                            */
/*                                                                    */
/* ================================================================== */

/*------------------------------------------------------------------*/
/*																	*/
/* File:		"fec_code.h"										*/
/*																	*/
/* Description: 	header filt for fec_code						*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _FEC_CODE_H_
#define _FEC_CODE_H_

extern const int16_t low_rate_pitch_enc[][PIT_QLEV];

extern const int16_t low_rate_pitch_dec[];

extern void low_rate_fec_code(struct quant_param *par);

extern int16_t low_rate_fec_decode(struct quant_param *qpar,
				     int16_t erase, int16_t lsp_check[]);

#endif
