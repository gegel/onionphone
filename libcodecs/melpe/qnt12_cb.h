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
/* File:		qnt12_cb.h											*/
/*																	*/
/* Description: quantization codebook for 1200bps					*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _QNT12_CB_H_
#define _QNT12_CB_H_

extern const int16_t bp_index_map[];

extern const int16_t inv_bp_index_map[];

extern const int16_t pitch_uvflag_map[];

extern const int16_t vvv_index_map[];

extern const int16_t pitch_vq_cb_vvv[];

extern const int16_t pitch_vq_cb_uvv[];

extern const int16_t gain_vq_cb[];

extern const int16_t inpCoef[][2 * LPC_ORD];

extern const int16_t lsp_v_256x64x32x32[];

extern const int16_t lsp_uv_9[];

extern const int16_t res256x64x64x64[];

#endif
