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


extern const Shortword	bp_index_map[];

extern const Shortword	inv_bp_index_map[];

extern const Shortword	pitch_uvflag_map[];

extern const Shortword	vvv_index_map[];

extern const Shortword	pitch_vq_cb_vvv[];

extern const Shortword	pitch_vq_cb_uvv[];

extern const Shortword	gain_vq_cb[];

extern const Shortword	inpCoef[][2*LPC_ORD];

extern const Shortword	lsp_v_256x64x32x32[];

extern const Shortword	lsp_uv_9[];

extern const Shortword	res256x64x64x64[];


#endif

