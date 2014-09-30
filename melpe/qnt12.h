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
/* File:		qnt12.c 											*/
/*																	*/
/* Description: quantization for 1200bps							*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _QNT12_H_
#define _QNT12_H_


void	pitch_vq(struct melp_param *par);

void	gain_vq(struct melp_param *par);

void	quant_bp(struct melp_param *par, Shortword num_frames);

BOOLEAN		lspStable(Shortword lsp[], Shortword order);

void	lspSort(Shortword lsp[], Shortword order);

void	lsf_vq(struct melp_param *par);

void	deqnt_msvq(Shortword qout[], const Shortword codebook[], Shortword tos,
				   const Shortword cb_size[], Shortword *index, Shortword dim);

void	quant_jitter(struct melp_param *par);

void	quant_fsmag(struct melp_param *par);


#endif

