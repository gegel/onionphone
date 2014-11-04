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

/*

2.4 kbps MELP Proposed Federal Standard speech coder

Fixed-point C code, version 1.0

Copyright (c) 1998, Texas Instruments, Inc.

Texas Instruments has intellectual property rights on the MELP
algorithm.	The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).

The fixed-point version of the voice codec Mixed Excitation Linear
Prediction (MELP) is based on specifications on the C-language software
simulation contained in GSM 06.06 which is protected by copyright and
is the property of the European Telecommunications Standards Institute
(ETSI). This standard is available from the ETSI publication office
tel. +33 (0)4 92 94 42 58. ETSI has granted a license to United States
Department of Defense to use the C-language software simulation contained
in GSM 06.06 for the purposes of the development of a fixed-point
version of the voice codec Mixed Excitation Linear Prediction (MELP).
Requests for authorization to make other use of the GSM 06.06 or
otherwise distribute or modify them need to be addressed to the ETSI
Secretariat fax: +33 493 65 47 16.

*/

/*
	Name: melp_chn_write, melp_chn_read
	Description: Write/read MELP channel bitstream
	Inputs:
	  MELP parameter structure
	Outputs:
	  updated MELP parameter structure (channel pointers)
	Returns: void
*/

#include "sc1200.h"
#include "vq_lib.h"
#include "melp_sub.h"
#include "math_lib.h"
#include "constant.h"
#include "global.h"
#include "qnt12_cb.h"
#include "mathhalf.h"
#include "mathdp31.h"
#include "mat_lib.h"
#include "qnt12.h"
#include "msvq_cb.h"
#include "fsvq_cb.h"
#include "dsp_sub.h"
#include "fec_code.h"
#include "macro.h"

#define X0333_Q15		10923	/* (1.0/3.0) * (1 << 15) */
#define X0667_Q15		21845	/* (2.0/3.0) * (1 << 15) */
#define UV_PIND			0	/* Unvoiced pitch index */
#define INVAL_PIND		1	/* Invalid pitch index  */
#define SMOOTH			16383	/* Q15 */

#define ORIGINAL_BIT_ORDER	0	/* flag to use bit order of original version */
#if (ORIGINAL_BIT_ORDER)	/* Original linear order */
static int16_t bit_order[NUM_CH_BITS] = {
	0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35,
	36, 37, 38, 39, 40, 41,
	42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53
};
#else				/* Order based on priority of bits */
static int16_t bit_order[NUM_CH_BITS] = {
	0, 17, 9, 28, 34, 3,
	4, 39, 1, 2, 13, 38,
	14, 10, 11, 40, 15, 21,
	27, 45, 12, 26, 25, 33,
	20, 24, 23, 32, 44, 46,
	22, 31, 53, 52, 51, 7,
	6, 19, 18, 29, 37, 30,
	36, 35, 43, 42, 16, 41,
	50, 49, 48, 47, 8, 5
};
#endif

/* Define bit buffer */
static unsigned char bit_buffer[NUM_CH_BITS];

static int16_t sync_bit = 0;	/* sync bit */

int16_t parity(int16_t x, int16_t leng);

void melp_chn_write(struct quant_param *qpar, unsigned char chbuf[])
{
	register int16_t i;
	unsigned char *bit_ptr;
	int16_t bit_cntr;

	/* FEC: code additional information in redundant indeces */
	fec_code(qpar);

	/* Fill bit buffer */
	bit_ptr = bit_buffer;
	bit_cntr = 0;

	pack_code(qpar->gain_index[1], &bit_ptr, &bit_cntr, 5, 1);

	/* Toggle and write sync bit */
	sync_bit = melpe_sub(1, sync_bit);
	pack_code(sync_bit, &bit_ptr, &bit_cntr, 1, 1);

	pack_code(qpar->gain_index[0], &bit_ptr, &bit_cntr, 3, 1);
	pack_code(qpar->pitch_index, &bit_ptr, &bit_cntr, PIT_BITS, 1);
	pack_code(qpar->jit_index[0], &bit_ptr, &bit_cntr, 1, 1);
	pack_code(qpar->bpvc_index[0], &bit_ptr, &bit_cntr, NUM_BANDS - 1, 1);

	for (i = 0; i < MSVQ_STAGES; i++)
		pack_code(qpar->msvq_index[i], &bit_ptr, &bit_cntr,
			  msvq_bits[i], 1);

	pack_code(qpar->fsvq_index, &bit_ptr, &bit_cntr, FS_BITS, 1);

	/* Write channel output buffer */
	qpar->chptr = chbuf;
	qpar->chbit = 0;
	for (i = 0; i < bitNum24; i++) {
		pack_code(bit_buffer[bit_order[i]], &qpar->chptr, &qpar->chbit,
			  1, chwordsize);
		if (i == 0)	/* set beginning of frame bit */
			*(qpar->chptr) |= (uint16_t) 0x8000;
	}
}

BOOLEAN melp_chn_read(struct quant_param *qpar, struct melp_param *par,
		      struct melp_param *prev_par, unsigned char chbuf[])
{
	register int16_t i;
	unsigned char *bit_ptr;
	BOOLEAN erase = FALSE;
	int16_t index, bit_cntr, dontcare;

	/* Read channel output buffer into bit buffer */
	bit_ptr = bit_buffer;
	qpar->chptr = chbuf;
	qpar->chbit = 0;
	for (i = 0; i < bitNum24; i++) {
		erase |= unpack_code(&(qpar->chptr), &(qpar->chbit), &index, 1,
				     chwordsize, ERASE_MASK);
		bit_buffer[bit_order[i]] = (unsigned char)index;
		bit_ptr++;
	}

	/* Read information from  bit buffer */
	bit_ptr = bit_buffer;
	bit_cntr = 0;

	(void)unpack_code(&bit_ptr, &bit_cntr, &qpar->gain_index[1], 5, 1, 0);

	/* Read sync bit */
	(void)unpack_code(&bit_ptr, &bit_cntr, &dontcare, 1, 1, 0);
	(void)unpack_code(&bit_ptr, &bit_cntr, &qpar->gain_index[0], 3, 1, 0);
	(void)unpack_code(&bit_ptr, &bit_cntr, &(qpar->pitch_index), PIT_BITS,
			  1, 0);

	(void)unpack_code(&bit_ptr, &bit_cntr, &qpar->jit_index[0], 1, 1, 0);
	(void)unpack_code(&bit_ptr, &bit_cntr, &qpar->bpvc_index[0],
			  NUM_BANDS - 1, 1, 0);

	for (i = 0; i < MSVQ_STAGES; i++)
		(void)unpack_code(&bit_ptr, &bit_cntr, &(qpar->msvq_index[i]),
				  msvq_bits[i], 1, 0);

	(void)unpack_code(&bit_ptr, &bit_cntr, &qpar->fsvq_index, FS_BITS, 1,
			  0);

	/* Clear unvoiced flag */
	qpar->uv_flag[0] = FALSE;

	erase = (BOOLEAN) fec_decode(qpar, erase);

	/* Decode new frame if no erasures occurred */
	if (erase) {		/* Erasure: frame repeat */
		*par = *prev_par;

		/* Force all subframes to equal last one */
		fill(par->gain, par->gain[NUM_GAINFR - 1], NUM_GAINFR - 1);

	} else {

		/* Decode line spectrum frequencies */
		vq_msd2(msvq_cb, par->lsf, msvq_cb_mean, qpar->msvq_index,
			msvq_levels, MSVQ_STAGES, LPC_ORD, 2);
		dontcare = FS_LEVELS;
		if (qpar->uv_flag[0])
			fill(par->fs_mag, ONE_Q13, NUM_HARM);
		else		/* Decode Fourier magnitudes */
			vq_msd2(fsvq_cb, par->fs_mag, (int16_t *) NULL,
				&(qpar->fsvq_index), &dontcare, 1, NUM_HARM, 0);

		/* Decode gain terms with uniform log quantizer */
		q_gain_dec(par->gain, qpar->gain_index, GN_QLO_Q8, GN_QUP_Q8,
			   GN_QLEV_M1_Q10, 5);

		/* Decode voicing information */
		par->uv_flag = qpar->uv_flag[0];

		/* Fractional pitch: Decode logarithmic pitch period */
		if (qpar->uv_flag[0])
			par->pitch = UV_PITCH_Q7;
		else {
			quant_u_dec(qpar->pitch_index, &par->pitch, PIT_QLO_Q12,
				    PIT_QUP_Q12, PIT_QLEV_M1_Q8, 7);
			par->pitch = pow10_fxp(par->pitch, 7);
		}

		/* Decode jitter */
		/*      quant_u_dec(par->jit_index, &par->jitter, 0.0, MAX_JITTER, 2);    */

		if (qpar->jit_index[0] == 0)
			par->jitter = 0;
		else
			par->jitter = MAX_JITTER_Q15;

		/* Decode bandpass voicing */
		q_bpvc_dec(par->bpvc, qpar->bpvc_index[0], qpar->uv_flag[0],
			   NUM_BANDS);
	}

	/* Return erase flag */
	return (erase);
}

/****************************************************************************
**
** Function:		low_rate_chn_write
**
** Description: Write channel buffer for low rate ( 1200bps )
**
** Arguments:
**
**	quant_param par ---- The quantization structure
**
** Return value:	None
**
*****************************************************************************/
void low_rate_chn_write(struct quant_param *qpar)
{
	register int16_t i;
	int16_t bit_cntr, cnt;
	int16_t uv1, uv2, cuv;
	int16_t uv_index = 0, bp_prot1 = 0, bp_prot2 = 0, lsp_prot = 0;
	int16_t uv_parity;
	unsigned char *bit_ptr;

	/* FEC: code additional information in redundant indices */
	low_rate_fec_code(qpar);

	/* ====== Fill bit buffer ====== */
	bit_ptr = bit_buffer;
	bit_cntr = 0;

	/* ====== Toggle and write sync bit ====== */
	sync_bit = melpe_sub(1, sync_bit);
	pack_code(sync_bit, &bit_ptr, &bit_cntr, 1, 1);

	/* ===== Count the number of voiced frame ===== */
	cnt = 0;
	for (i = 0; i < NF; i++) {
		if (!qpar->uv_flag[i])
			cnt++;
	}

	/* ====== Packing global voicing and pitch information ====== */
	if (cnt <= 1) {		/* UUU UUV UVU VUU */
		uv_index = 0;
		bp_prot1 = 0;
		if (!qpar->uv_flag[0])
			bp_prot2 = 3;
		else if (!qpar->uv_flag[1])
			bp_prot2 = 2;
		else if (!qpar->uv_flag[2])
			bp_prot2 = 1;
		else {
			bp_prot2 = 0;
			lsp_prot = 0;
		}

		if (bp_prot2 == 0)
			qpar->pitch_index = UV_PIND;
		else
			qpar->pitch_index =
			    low_rate_pitch_enc[bp_prot2][qpar->pitch_index];
	} else if (cnt == 2) {	/* UVV VUV VVU */
		if (qpar->uv_flag[0]) {
			uv_index = 4;
			bp_prot1 = 3;
		} else if (qpar->uv_flag[1]) {
			uv_index = 2;
			bp_prot1 = 2;
		} else if (qpar->uv_flag[2]) {
			uv_index = 1;
			bp_prot1 = 1;
			lsp_prot = 7;
		}
	} else if (cnt == 3) {	/* VVV */
		uv_index = (int16_t) (qpar->pitch_index / PITCH_VQ_SIZE);
		qpar->pitch_index = melpe_sub(qpar->pitch_index,
					(int16_t) (uv_index * PITCH_VQ_SIZE));
		uv_index = vvv_index_map[uv_index];
	}

	pack_code(uv_index, &bit_ptr, &bit_cntr, UV_BITS, 1);

	uv_parity = parity(uv_index, 3);

	pack_code(uv_parity, &bit_ptr, &bit_cntr, 1, 1);

	pack_code(qpar->pitch_index, &bit_ptr, &bit_cntr, PITCH_VQ_BITS, 1);

	/* ====== Packing LSF indices ====== */
	uv1 = qpar->uv_flag[0];
	uv2 = qpar->uv_flag[1];
	cuv = qpar->uv_flag[2];

	if ((uv1 == 1) && (uv2 == 1) && (cuv == 1)) {
		pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[1][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[0][1], &bit_ptr, &bit_cntr, 4, 1);
		pack_code(qpar->lsf_index[1][1], &bit_ptr, &bit_cntr, 4, 1);
		pack_code(qpar->lsf_index[2][1], &bit_ptr, &bit_cntr, 4, 1);
		pack_code(lsp_prot, &bit_ptr, &bit_cntr, 3, 1);
	} else if ((uv1 == 1) && (uv2 == 1) && (cuv != 1)) {
		pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[1][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 8, 1);
		pack_code(qpar->lsf_index[2][1], &bit_ptr, &bit_cntr, 6, 1);
		pack_code(qpar->lsf_index[2][2], &bit_ptr, &bit_cntr, 5, 1);
		pack_code(qpar->lsf_index[2][3], &bit_ptr, &bit_cntr, 5, 1);
	} else if ((uv1 == 1) && (uv2 != 1) && (cuv == 1)) {
		pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[1][0], &bit_ptr, &bit_cntr, 8, 1);
		pack_code(qpar->lsf_index[1][1], &bit_ptr, &bit_cntr, 6, 1);
		pack_code(qpar->lsf_index[1][2], &bit_ptr, &bit_cntr, 5, 1);
		pack_code(qpar->lsf_index[1][3], &bit_ptr, &bit_cntr, 5, 1);
		pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 9, 1);
	} else if ((uv1 != 1) && (uv2 == 1) && (cuv == 1)) {
		pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 8, 1);
		pack_code(qpar->lsf_index[0][1], &bit_ptr, &bit_cntr, 6, 1);
		pack_code(qpar->lsf_index[0][2], &bit_ptr, &bit_cntr, 5, 1);
		pack_code(qpar->lsf_index[0][3], &bit_ptr, &bit_cntr, 5, 1);
		pack_code(qpar->lsf_index[1][0], &bit_ptr, &bit_cntr, 9, 1);
		pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 9, 1);
	} else {
		if ((uv1 != 1) && (uv2 != 1) && (cuv == 1)) {
			/* ---- Interpolation [4 inp + (8+6+6+6) res + 9 uv] ---- */
			pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 9,
				  1);
		} else {
			pack_code(qpar->lsf_index[0][0], &bit_ptr, &bit_cntr, 8,
				  1);
			pack_code(qpar->lsf_index[0][1], &bit_ptr, &bit_cntr, 6,
				  1);
			pack_code(qpar->lsf_index[0][2], &bit_ptr, &bit_cntr, 5,
				  1);
			pack_code(qpar->lsf_index[0][3], &bit_ptr, &bit_cntr, 5,
				  1);
		}

		pack_code(qpar->lsf_index[1][0], &bit_ptr, &bit_cntr, 4, 1);

		if ((uv1 != 1) && (uv2 != 1) && (cuv == 1)) {
			pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 8,
				  1);
			pack_code(qpar->lsf_index[2][1], &bit_ptr, &bit_cntr, 6,
				  1);
			pack_code(qpar->lsf_index[2][2], &bit_ptr, &bit_cntr, 6,
				  1);
			pack_code(qpar->lsf_index[2][3], &bit_ptr, &bit_cntr, 6,
				  1);
			pack_code(lsp_prot, &bit_ptr, &bit_cntr, 3, 1);
		} else {
			pack_code(qpar->lsf_index[2][0], &bit_ptr, &bit_cntr, 8,
				  1);
			pack_code(qpar->lsf_index[2][1], &bit_ptr, &bit_cntr, 6,
				  1);
		}
	}

	/* ====== Packing gain index ====== */
	pack_code(qpar->gain_index[0], &bit_ptr, &bit_cntr, GAIN_VQ_BITS, 1);

	/* ====== Packing voicing indices ====== */
	for (i = 0; i < NF; i++) {
		if (!qpar->uv_flag[i])
			pack_code(qpar->bpvc_index[i], &bit_ptr, &bit_cntr, 2,
				  1);
	}
	if (cnt == 2)
		pack_code(bp_prot1, &bit_ptr, &bit_cntr, 2, 1);
	else if (cnt == 1) {
		pack_code(bp_prot2, &bit_ptr, &bit_cntr, 2, 1);
		pack_code(bp_prot1, &bit_ptr, &bit_cntr, 2, 1);
	} else if (cnt == 0) {	/* UUU */
		pack_code(qpar->bpvc_index[0], &bit_ptr, &bit_cntr, 2, 1);
		pack_code(bp_prot2, &bit_ptr, &bit_cntr, 2, 1);
		pack_code(bp_prot1, &bit_ptr, &bit_cntr, 2, 1);
	}

	/* ====== Packing Fourier magnitudes ====== */
	pack_code(qpar->fs_index, &bit_ptr, &bit_cntr, FS_BITS, 1);

	/* ====== Packing jitter index ====== */
	pack_code(qpar->jit_index[0], &bit_ptr, &bit_cntr, 1, 1);

	/* ======== Write channel output buffer ======== */
	qpar->chptr = chbuf;
	qpar->chbit = 0;
	for (i = 0; i < bitNum12; i++) {
		pack_code(bit_buffer[i], &qpar->chptr, &qpar->chbit, 1,
			  chwordsize);
		if (i == 0)
			*(qpar->chptr) |= 0x8000;	/* set beginning of frame bit */
	}
}

/****************************************************************************
**
** Function:		low_rate_chn_read
**
** Description: Unpacking the channel buffer for low rate ( 1200bps )
**
** Arguments:
**
**	quant_param par ---- The quantization structure
**
** Return value:	None
**
*****************************************************************************/
int16_t low_rate_chn_read(struct quant_param *qpar, struct melp_param *par,
			    struct melp_param *prev_par)
{
	register int16_t i, j, k;
	static int16_t prev_uv = 1;
	static int16_t prev_fsmag[NUM_HARM];
	static int16_t qplsp[LPC_ORD], prev_gain[2 * NF * NUM_GAINFR];
	static int16_t firstTime = TRUE;
	const int16_t *codebook;
	int16_t erase = 0, lsp_check[NF] = { 0, 0, 0 };
	int16_t bit_cntr, bit_cntr1, cnt, last, index, dontcare;
	int16_t prot_bp1, prot_bp2, prot_lsp;
	int16_t uv1, uv2, cuv, uv_index, uv_parity;
	unsigned char *bit_ptr, *bit_ptr1;
	int16_t ilsp1[LPC_ORD], ilsp2[LPC_ORD], res[2 * LPC_ORD];
	int16_t temp1, temp2, p_value, q_value;
	int16_t intfact;
	int16_t weighted_fsmag[NUM_HARM];	/* Q13 */

	int16_t erase_uuu = 0, erase_vvv = 0;
	int16_t flag_parity = 0, flag_dec_lsp = 1, flag_dec_pitch = 1;

	int16_t melp_v_cb_size[4] = { 256, 64, 32, 32 };
	int16_t res_cb_size[4] = { 256, 64, 64, 64 };
	int16_t melp_uv_cb_size[1] = { 512 };
	int32_t L_acc, L_sum1, L_sum2;

	/* In previous versions we use unweighted_fsmag[] and prev_fsmag[] to     */
	/* keep track of a previous par[].fs_mag array.  They were obtained by    */
	/* multiplying the w_fs_inv[] arrays with par[].fs_mag, and they are      */
	/* weighted by w_fs[] when they are used later.  In floating point        */
	/* version this is straightforward but in fixed point the multiplication  */
	/* of w_fs_inv[] and w_fs[] do not yield the original data.  Modifica-    */
	/* tions are made to remove this to and fro inversions.                   */

	if (firstTime) {	/* Initialization */
		temp2 = melpe_shl(LPC_ORD, 10);	/* Q10 */
		temp1 = X08_Q10;	/* Q10 */
		for (i = 0; i < LPC_ORD; i++) {
			/*      qplsp[i] = (i + 1)*0.8/LPC_ORD; */
			qplsp[i] = melpe_divide_s(temp1, temp2);
			temp1 = melpe_add(temp1, X08_Q10);
		}

		fill(prev_gain, 2560, 2 * NF * NUM_GAINFR);
		fill(prev_fsmag, ONE_Q13, NUM_HARM);

		firstTime = FALSE;
	}

	/* ======== Read channel output buffer into bit buffer ======== */
	bit_ptr = bit_buffer;
	qpar->chptr = chbuf;
	qpar->chbit = 0;
	for (i = 0; i < bitNum12; i++) {
		erase |= unpack_code(&qpar->chptr, &qpar->chbit, &index, 1,
				     chwordsize, ERASE_MASK);
		bit_buffer[i] = (unsigned char)index;
		bit_ptr++;
	}

	bit_ptr = bit_buffer;
	bit_cntr = 0;

	/* ====== Read sync bit ====== */
	unpack_code(&bit_ptr, &bit_cntr, &dontcare, 1, 1, 0);

	/* ====== Unpacking Global U/V dicision ====== */
	unpack_code(&bit_ptr, &bit_cntr, &uv_index, UV_BITS, 1, 0);

	/* ====== Unpacking parity bit ====== */
	unpack_code(&bit_ptr, &bit_cntr, &uv_parity, 1, 1, 0);

	/* ====== Unpacking pitch information ====== */
	unpack_code(&bit_ptr, &bit_cntr, &qpar->pitch_index, PITCH_VQ_BITS, 1,
		    0);

	/* error check in U/V pattern */
	bit_ptr1 = bit_ptr;
	/* bit_cntr1= bit_cntr;
	   unpack_code(&bit_ptr1, &bit_cntr1, &dontcare, 39, 1, 0);          LSP */
	bit_cntr1 = 0;
	bit_ptr1 += 39;
	unpack_code(&bit_ptr1, &bit_cntr1, &prot_lsp, 3, 1, 0);	/* LSP */
	unpack_code(&bit_ptr1, &bit_cntr1, &dontcare, 10, 1, 0);	/* GAIN */
	unpack_code(&bit_ptr1, &bit_cntr1, &dontcare, 2, 1, 0);	/* BP */
	unpack_code(&bit_ptr1, &bit_cntr1, &prot_bp2, 2, 1, 0);	/* BP */
	unpack_code(&bit_ptr1, &bit_cntr1, &prot_bp1, 2, 1, 0);	/* BP */

	if (uv_parity != parity(uv_index, 3))
		flag_parity |= 1;

	/* Mode checking */
	if (uv_index == 0) {
		if (!flag_parity) {
			j = qpar->pitch_index;
			qpar->pitch_index =
			    low_rate_pitch_dec[qpar->pitch_index];
			if (qpar->pitch_index == UV_PIND) {
				qpar->uv_flag[0] = par[0].uv_flag = 1;
				qpar->uv_flag[1] = par[1].uv_flag = 1;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
				if (prot_bp2 != 0) {
					erase_uuu |= 1;
					flag_dec_lsp = 0;
					flag_dec_pitch = 0;
				}
			} else if (qpar->pitch_index == INVAL_PIND) {
				erase_uuu |= 1;
				flag_dec_lsp = 0;
				flag_dec_pitch = 0;
			} else {
				qpar->pitch_index -= 2;
				for (i = k = 0; i < PITCH_VQ_BITS; i++) {
					if ((j & 0x1) == 1)
						k++;
					j >>= 1;
				}

				if (k == 6 || k == 7) {
					qpar->uv_flag[0] = par[0].uv_flag = 1;
					qpar->uv_flag[1] = par[1].uv_flag = 1;
					qpar->uv_flag[2] = par[2].uv_flag = 0;
					if (prot_bp2 != 1) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				} else if (k == 4) {
					qpar->uv_flag[0] = par[0].uv_flag = 1;
					qpar->uv_flag[1] = par[1].uv_flag = 0;
					qpar->uv_flag[2] = par[2].uv_flag = 1;
					if (prot_bp2 != 2) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				} else if (k == 5) {
					qpar->uv_flag[0] = par[0].uv_flag = 0;
					qpar->uv_flag[1] = par[1].uv_flag = 1;
					qpar->uv_flag[2] = par[2].uv_flag = 1;
					if (prot_bp2 != 3) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				}
			}
		} else if (prot_bp1 == 0) {
			j = qpar->pitch_index;
			qpar->pitch_index =
			    low_rate_pitch_dec[qpar->pitch_index];
			if (qpar->pitch_index == UV_PIND) {
				qpar->uv_flag[0] = par[0].uv_flag = 1;
				qpar->uv_flag[1] = par[1].uv_flag = 1;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
				if (prot_bp2 != 0) {
					erase_uuu |= 1;
					flag_dec_lsp = 0;
					flag_dec_pitch = 0;
				}
			} else if (qpar->pitch_index == INVAL_PIND) {
				erase_uuu |= 1;
				flag_dec_lsp = 0;
				flag_dec_pitch = 0;
			} else {
				qpar->pitch_index -= 2;
				for (i = k = 0; i < PITCH_VQ_BITS; i++) {
					if ((j & 0x1) == 1)
						k++;
					j >>= 1;
				}

				if (k == 6 || k == 7) {
					qpar->uv_flag[0] = par[0].uv_flag = 1;
					qpar->uv_flag[1] = par[1].uv_flag = 1;
					qpar->uv_flag[2] = par[2].uv_flag = 0;
					if (prot_bp2 != 1) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				} else if (k == 4) {
					qpar->uv_flag[0] = par[0].uv_flag = 1;
					qpar->uv_flag[1] = par[1].uv_flag = 0;
					qpar->uv_flag[2] = par[2].uv_flag = 1;
					if (prot_bp2 != 2) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				} else if (k == 5) {
					qpar->uv_flag[0] = par[0].uv_flag = 0;
					qpar->uv_flag[1] = par[1].uv_flag = 1;
					qpar->uv_flag[2] = par[2].uv_flag = 1;
					if (prot_bp2 != 3) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				}
			}
		} else {
			if (prot_bp2 == 1 && prot_lsp == 7) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
			} else if (prot_bp2 == 2) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 1;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			} else if (prot_bp2 == 3) {
				qpar->uv_flag[0] = par[0].uv_flag = 1;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			} else {
				erase_vvv |= 1;
				flag_dec_lsp = 0;
				flag_dec_pitch = 2;
			}
		}
	} else if (uv_index == 1 || uv_index == 2 || uv_index == 4) {
		if (!flag_parity) {
			if (uv_index == 1) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
			} else if (uv_index == 2) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 1;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			} else if (uv_index == 4) {
				qpar->uv_flag[0] = par[0].uv_flag = 1;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			}
		} else {
			if (prot_bp1 == 0) {
				j = qpar->pitch_index;
				qpar->pitch_index =
				    low_rate_pitch_dec[qpar->pitch_index];
				if (qpar->pitch_index == UV_PIND) {
					qpar->uv_flag[0] = par[0].uv_flag = 1;
					qpar->uv_flag[1] = par[1].uv_flag = 1;
					qpar->uv_flag[2] = par[2].uv_flag = 1;
					if (prot_lsp != 0) {
						erase_uuu |= 1;
						flag_dec_lsp = 0;
						flag_dec_pitch = 0;
					}
				} else if (qpar->pitch_index == INVAL_PIND) {
					erase_uuu |= 1;
					flag_dec_lsp = 0;
					flag_dec_pitch = 0;
				} else {
					qpar->pitch_index -= 2;
					for (i = k = 0; i < PITCH_VQ_BITS; i++) {
						if ((j & 0x1) == 1)
							k++;
						j >>= 1;
					}

					if (k == 6 || k == 7) {
						qpar->uv_flag[0] =
						    par[0].uv_flag = 1;
						qpar->uv_flag[1] =
						    par[1].uv_flag = 1;
						qpar->uv_flag[2] =
						    par[2].uv_flag = 0;
						if (prot_bp2 != 1) {
							erase_uuu |= 1;
							flag_dec_lsp = 0;
							flag_dec_pitch = 0;
						}
					} else if (k == 4) {
						qpar->uv_flag[0] =
						    par[0].uv_flag = 1;
						qpar->uv_flag[1] =
						    par[1].uv_flag = 0;
						qpar->uv_flag[2] =
						    par[2].uv_flag = 1;
						if (prot_bp2 != 2) {
							erase_uuu |= 1;
							flag_dec_lsp = 0;
							flag_dec_pitch = 0;
						}
					} else if (k == 5) {
						qpar->uv_flag[0] =
						    par[0].uv_flag = 0;
						qpar->uv_flag[1] =
						    par[1].uv_flag = 1;
						qpar->uv_flag[2] =
						    par[2].uv_flag = 1;
						if (prot_bp2 != 3) {
							erase_uuu |= 1;
							flag_dec_lsp = 0;
							flag_dec_pitch = 0;
						}
					}
				}
			} else if (prot_bp1 == 1 && prot_lsp == 7) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
			} else {
				erase_vvv |= 1;
			}
		}
	} else if (uv_index == 3 || uv_index == 5) {
		if (!flag_parity) {
			qpar->uv_flag[0] = par[0].uv_flag = 0;
			qpar->uv_flag[1] = par[1].uv_flag = 0;
			qpar->uv_flag[2] = par[2].uv_flag = 0;
			if (uv_index == 5)
				qpar->pitch_index =
				    (int16_t) (qpar->pitch_index +
						 PITCH_VQ_SIZE);
		} else {
			if (prot_bp1 == 1 && prot_lsp == 7) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 1;
			} else {
				erase_vvv |= 1;
				if (uv_index == 5)
					qpar->pitch_index =
					    (int16_t) (qpar->pitch_index +
							 PITCH_VQ_SIZE);
			}
		}
	} else if (uv_index == 6) {
		if (!flag_parity) {
			qpar->uv_flag[0] = par[0].uv_flag = 0;
			qpar->uv_flag[1] = par[1].uv_flag = 0;
			qpar->uv_flag[2] = par[2].uv_flag = 0;
			qpar->pitch_index =
			    (int16_t) (qpar->pitch_index + 2 * PITCH_VQ_SIZE);
		} else {
			if (prot_bp1 == 2) {
				qpar->uv_flag[0] = par[0].uv_flag = 0;
				qpar->uv_flag[1] = par[1].uv_flag = 1;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			}
			if (prot_bp1 == 3) {
				qpar->uv_flag[0] = par[0].uv_flag = 1;
				qpar->uv_flag[1] = par[1].uv_flag = 0;
				qpar->uv_flag[2] = par[2].uv_flag = 0;
			} else {
				erase_vvv |= 1;
				qpar->pitch_index =
				    (int16_t) (qpar->pitch_index +
						 2 * PITCH_VQ_SIZE);
			}
		}
	} else if (uv_index == 7) {
		if (!flag_parity) {
			qpar->uv_flag[0] = par[0].uv_flag = 0;
			qpar->uv_flag[1] = par[1].uv_flag = 0;
			qpar->uv_flag[2] = par[2].uv_flag = 0;
			qpar->pitch_index =
			    (int16_t) (qpar->pitch_index + 3 * PITCH_VQ_SIZE);
		} else {
			erase_vvv |= 1;
			qpar->pitch_index =
			    (int16_t) (qpar->pitch_index + 3 * PITCH_VQ_SIZE);
		}
	}

	if (erase_uuu) {
		qpar->uv_flag[0] = par[0].uv_flag = 1;
		qpar->uv_flag[1] = par[1].uv_flag = 1;
		qpar->uv_flag[2] = par[2].uv_flag = 1;
	}
	if (erase_vvv) {
		qpar->uv_flag[0] = par[0].uv_flag = 0;
		qpar->uv_flag[1] = par[1].uv_flag = 0;
		qpar->uv_flag[2] = par[2].uv_flag = 0;
	}

	/* ====== Unpacking LSF information ====== */
	last = -1;
	cnt = 0;
	for (i = 0; i < NF; i++) {
		if (!qpar->uv_flag[i]) {
			cnt++;
			last = i;
		}
	}
	uv1 = qpar->uv_flag[0];
	uv2 = qpar->uv_flag[1];
	cuv = qpar->uv_flag[2];

	if ((uv1 == 1) && (uv2 == 1) && (cuv == 1)) {
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][1]), 4, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][1]), 4, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][1]), 4, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &dontcare, 3, 1, 0);
	} else if ((uv1 == 1) && (uv2 == 1) && (cuv != 1)) {
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][0]), 8, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][1]), 6, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][2]), 5, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][3]), 5, 1,
			    0);
	} else if ((uv1 == 1) && (uv2 != 1) && (cuv == 1)) {
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][0]), 8, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][1]), 6, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][2]), 5, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][3]), 5, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][0]), 9, 1,
			    0);
	} else if ((uv1 != 1) && (uv2 == 1) && (cuv == 1)) {
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][0]), 8, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][1]), 6, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][2]), 5, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[0][3]), 5, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][0]), 9, 1,
			    0);
		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[2][0]), 9, 1,
			    0);
	} else {
		if ((uv1 != 1) && (uv2 != 1) && (cuv == 1)) {
			/* ---- Interpolation [4 inp + (8+6+6+6) res + 9 uv] ---- */
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[0][0]), 9, 1, 0);
		} else {
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[0][0]), 8, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[0][1]), 6, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[0][2]), 5, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[0][3]), 5, 1, 0);
		}

		unpack_code(&bit_ptr, &bit_cntr, &(qpar->lsf_index[1][0]), 4, 1,
			    0);
		if ((uv1 != 1) && (uv2 != 1) && (cuv == 1)) {
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][0]), 8, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][1]), 6, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][2]), 6, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][3]), 6, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr, &dontcare, 3, 1, 0);
		} else {
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][0]), 8, 1, 0);
			unpack_code(&bit_ptr, &bit_cntr,
				    &(qpar->lsf_index[2][1]), 6, 1, 0);
		}
	}

	/* ====== Unpacking gain information ====== */
	unpack_code(&bit_ptr, &bit_cntr, &(qpar->gain_index[0]),
		    GAIN_VQ_BITS, 1, 0);

	/* ====== Unpacking voicing information ====== */
	for (i = 0; i < NF; i++) {
		if (!qpar->uv_flag[i])
			unpack_code(&bit_ptr, &bit_cntr, &(qpar->bpvc_index[i]),
				    2, 1, 0);
	}
	if (cnt == 2)
		unpack_code(&bit_ptr, &bit_cntr, &prot_bp1, 2, 1, 0);
	else if (cnt == 1) {
		unpack_code(&bit_ptr, &bit_cntr, &prot_bp2, 2, 1, 0);
		unpack_code(&bit_ptr, &bit_cntr, &prot_bp1, 2, 1, 0);
	} else if (cnt == 0) {	/* UUU */
		for (i = 0; i < NF; i++)
			unpack_code(&bit_ptr, &bit_cntr, &(qpar->bpvc_index[i]),
				    2, 1, 0);
	}

	/* ====== Unpacking Fourier magnitudes information ====== */
	unpack_code(&bit_ptr, &bit_cntr, &(qpar->fs_index), FS_BITS, 1, 0);

	/* ====== Unpacking jitter information ====== */
	unpack_code(&bit_ptr, &bit_cntr, &(qpar->jit_index[0]), 1, 1, 0);

	/* ====== FEC decoding ====== */
	erase = low_rate_fec_decode(qpar, erase, lsp_check);

	/* ====== Decode pitch information ====== */
	if (flag_dec_pitch == 1) {
		if (cnt == 0) {
			for (i = 0; i < NF; i++)
				par[i].pitch = LOG_UV_PITCH_Q12;
		} else if (cnt == 1) {
			for (i = 0; i < NF; i++) {
				if (!par[i].uv_flag)
					quant_u_dec(qpar->pitch_index,
						    &par[i].pitch, PIT_QLO_Q12,
						    PIT_QUP_Q12, PIT_QLEV_M1_Q8,
						    7);
				else
					par[i].pitch = LOG_UV_PITCH_Q12;
				par[i].pitch = pow10_fxp(par[i].pitch, 7);
			}
		} else if (cnt > 1) {
			index = qpar->pitch_index;

			/* ----- set pointer ----- */
			if (cnt == NF)
				codebook = pitch_vq_cb_vvv;
			else
				codebook = pitch_vq_cb_uvv;

			for (i = 0; i < NF; i++) {
				if (par[i].uv_flag == 1)
					par[i].pitch = UV_PITCH_Q7;
				else
					par[i].pitch =
					    pow10_fxp(codebook[index * NF + i],
						      7);
			}
		}
	} else if (flag_dec_pitch == 2) {
		codebook = pitch_vq_cb_uvv;	/* = pitch_vq_cb_uvu = pitch_vq_cb_vvu */
		index = qpar->pitch_index;
		for (i = 0; i < NF; i++)
			par[i].pitch = pow10_fxp(codebook[index * NF + i], 7);
	}

	/* ====== Decode LSF information ====== */
	if (flag_dec_lsp) {
		if ((uv1 == 1) && (uv2 == 1) && (cuv == 1)) {
			deqnt_msvq(&(par[0].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[0],
				   LPC_ORD);
			deqnt_msvq(&(par[1].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[1],
				   LPC_ORD);
			deqnt_msvq(&(par[2].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[2],
				   LPC_ORD);
		} else if ((uv1 == 1) && (uv2 == 1) && (cuv != 1)) {
			deqnt_msvq(&(par[0].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[0],
				   LPC_ORD);
			deqnt_msvq(&(par[1].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[1],
				   LPC_ORD);
			deqnt_msvq(&(par[2].lsf[0]), lsp_v_256x64x32x32, 4,
				   melp_v_cb_size, qpar->lsf_index[2], LPC_ORD);
		} else if ((uv1 == 1) && (uv2 != 1) && (cuv == 1)) {
			deqnt_msvq(&(par[0].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[0],
				   LPC_ORD);
			deqnt_msvq(&(par[1].lsf[0]), lsp_v_256x64x32x32, 4,
				   melp_v_cb_size, qpar->lsf_index[1], LPC_ORD);
			deqnt_msvq(&(par[2].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[2],
				   LPC_ORD);
		} else if ((uv1 != 1) && (uv2 == 1) && (cuv == 1)) {
			deqnt_msvq(&(par[0].lsf[0]), lsp_v_256x64x32x32, 4,
				   melp_v_cb_size, qpar->lsf_index[0], LPC_ORD);
			deqnt_msvq(&(par[1].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[1],
				   LPC_ORD);
			deqnt_msvq(&(par[2].lsf[0]), lsp_uv_9, 1,
				   melp_uv_cb_size, qpar->lsf_index[2],
				   LPC_ORD);
		} else {
			if ((uv1 != 1) && (uv2 != 1) && (cuv == 1))
				/* ---- Interpolation [4 inp + (8+6+6+6) res + 9 uv] ---- */
				deqnt_msvq(&(par[2].lsf[0]), lsp_uv_9, 1,
					   melp_uv_cb_size, qpar->lsf_index[0],
					   LPC_ORD);
			else
				deqnt_msvq(&(par[2].lsf[0]), lsp_v_256x64x32x32,
					   4, melp_v_cb_size,
					   qpar->lsf_index[0], LPC_ORD);

			i = qpar->lsf_index[1][0];
			for (j = 0; j < LPC_ORD; j++) {
				/*      ilsp1[j] = inpCoef[i][j] * qplsp[j] +
				   (1.0 - inpCoef[i][j]) * par[2].lsf[j]; */
				intfact = inpCoef[i][j];	/* Q14 */
				L_acc = melpe_L_mult(intfact, qplsp[j]);	/* Q14 */
				intfact = melpe_sub(ONE_Q14, intfact);
				L_acc = melpe_L_mac(L_acc, intfact, par[2].lsf[j]);
				ilsp1[j] = melpe_extract_h(melpe_L_shl(L_acc, 1));	/* Q15 */
				/*      ilsp2[j] = inpCoef[i][j+10] * qplsp[j] +
				   (1.0 - inpCoef[i][j + LPC_ORD]) * par[2].lsf[j]; */
				intfact = inpCoef[i][j + LPC_ORD];	/* Q14 */
				L_acc = melpe_L_mult(intfact, qplsp[j]);	/* Q14 */
				intfact = melpe_sub(ONE_Q14, intfact);
				L_acc = melpe_L_mac(L_acc, intfact, par[2].lsf[j]);
				ilsp2[j] = melpe_extract_h(melpe_L_shl(L_acc, 1));	/* Q15 */
			}

			if ((uv1 != 1) && (uv2 != 1) && (cuv == 1))
				deqnt_msvq(res, res256x64x64x64, 4, res_cb_size,
					   qpar->lsf_index[2], 20);
			else
				deqnt_msvq(res, res256x64x64x64, 2, res_cb_size,
					   qpar->lsf_index[2], 20);

			/* ---- reconstruct lsp ---- */
			for (i = 0; i < LPC_ORD; i++) {
				/*      par[0].lsf[i] = (res[i] + ilsp1[i]); */
				temp1 = melpe_shr(res[i], 2);
				par[0].lsf[i] = melpe_add(temp1, ilsp1[i]);	/* Q15 */
				/*      par[1].lsf[i] = (res[i + LPC_ORD] + ilsp2[i]); */
				temp2 = melpe_shr(res[i + LPC_ORD], 2);
				par[1].lsf[i] = melpe_add(temp2, ilsp2[i]);	/* Q15 */
			}
		}

		/* --- FEC LSP check --- */
		if (uv1 == 1 && uv2 == 1 && cuv == 1) {
			if (lsp_check[0] == 1 && lsp_check[1] == 1
			    && lsp_check[2] == 1) {
				for (i = 0; i < LPC_ORD; i++) {
					par[0].lsf[i] = prev_par->lsf[i];
					par[1].lsf[i] = prev_par->lsf[i];
					par[2].lsf[i] = prev_par->lsf[i];
				}
			} else if (lsp_check[0] == 1 && lsp_check[1] == 1 &&
				   lsp_check[2] == 0) {
				for (i = 0; i < LPC_ORD; i++) {
					par[0].lsf[i] =
					    melpe_add(melpe_mult
						(prev_par->lsf[i], X0667_Q15),
						melpe_mult(par[2].lsf[i], X0333_Q15));
					par[1].lsf[i] =
					    melpe_add(melpe_mult
						(prev_par->lsf[i], X0333_Q15),
						melpe_mult(par[2].lsf[i], X0667_Q15));
				}
			} else if (lsp_check[0] == 1 && lsp_check[1] == 0 &&
				   lsp_check[2] == 1) {
				for (i = 0; i < LPC_ORD; i++) {
					par[0].lsf[i] =
					    melpe_add(melpe_shr(prev_par->lsf[i], 1),
						melpe_shr(par[1].lsf[i], 1));
					par[2].lsf[i] = par[1].lsf[i];
				}
			} else if (lsp_check[0] == 0 && lsp_check[1] == 1 &&
				   lsp_check[2] == 1) {
				for (i = 0; i < LPC_ORD; i++) {
					par[1].lsf[i] = par[0].lsf[i];
					par[2].lsf[i] = par[0].lsf[i];
				}
			} else if (lsp_check[0] == 1 && lsp_check[1] == 0 &&
				   lsp_check[2] == 0) {
				for (i = 0; i < LPC_ORD; i++)
					par[0].lsf[i] =
					    melpe_add(melpe_shr(prev_par->lsf[i], 1),
						melpe_shr(par[1].lsf[i], 1));
			} else if (lsp_check[0] == 0 && lsp_check[1] == 1
				   && lsp_check[2] == 0) {
				for (i = 0; i < LPC_ORD; i++)
					par[1].lsf[i] =
					    melpe_add(melpe_shr(par[0].lsf[i], 1),
						melpe_shr(par[2].lsf[i], 1));
			} else if (lsp_check[0] == 0 && lsp_check[1] == 0
				   && lsp_check[2] == 1) {
				for (i = 0; i < LPC_ORD; i++) {
					par[2].lsf[i] = par[1].lsf[i];
				}
			}
		}

		/* ---- stable checking ---- */
		if (!lspStable(par[0].lsf, LPC_ORD))
			lspSort(par[0].lsf, LPC_ORD);
		if (!lspStable(par[1].lsf, LPC_ORD))
			lspSort(par[1].lsf, LPC_ORD);
		if (!lspStable(par[2].lsf, LPC_ORD))
			lspSort(par[2].lsf, LPC_ORD);

		/* ---- Save for next frame ---- */
		v_equ(qplsp, par[NF - 1].lsf, LPC_ORD);
	} else {
		for (i = 0; i < NF; i++)
			v_equ(par[i].lsf, prev_par->lsf, LPC_ORD);
		v_equ(qplsp, par[NF - 1].lsf, LPC_ORD);
	}

	/* ====== Decode gain information ====== */
	index = qpar->gain_index[0];
	for (i = 0; i < NF; i++) {
		for (j = 0; j < NUM_GAINFR; j++)
			par[i].gain[j] =
			    gain_vq_cb[index * NUM_GAINFR * NF +
				       i * NUM_GAINFR + j];
	}

	/* ====== Decode voicing information ====== */
	for (i = 0; i < NF; i++) {
		index = inv_bp_index_map[qpar->bpvc_index[i]];
		q_bpvc_dec(&(par[i].bpvc[0]), index, qpar->uv_flag[i],
			   NUM_BANDS);
	}

	/* ====== Decode Fourier magnitudes information ====== */
	if (cnt != 0) {
		index = qpar->fs_index;
		v_equ(par[last].fs_mag, &(fsvq_cb[NUM_HARM * index]), NUM_HARM);
		v_equ(weighted_fsmag, par[last].fs_mag, NUM_HARM);
	}

	if (cnt > 1) {
		if (prev_uv) {	/* U {UVV,VUV,VVU,VVV} */
			for (i = 0; i < last; i++)
				if (!par[i].uv_flag)
					v_equ(par[i].fs_mag, par[last].fs_mag,
					      NUM_HARM);
		} else {
			if (par[0].uv_flag)	/* V UVV */
				v_equ(par[1].fs_mag, par[last].fs_mag,
				      NUM_HARM);
			else if (par[1].uv_flag)	/* V VUV */
				v_equ(par[0].fs_mag, prev_fsmag, NUM_HARM);
			else if (par[2].uv_flag) {	/* V VVU */
				for (i = 0; i < NUM_HARM; i++) {
					/*      par[0].fs_mag[i] = 0.5*(weighted_fsmag[i] +
					   prev_fsmag[i]); */
					temp1 = melpe_shr(weighted_fsmag[i], 1);	/* Q13 */
					temp2 = melpe_shr(prev_fsmag[i], 1);	/* Q13 */
					par[0].fs_mag[i] = melpe_add(temp1, temp2);	/* Q13 */
				}
			} else {	/* V VVV */
				for (i = 0; i < NUM_HARM; i++) {
					p_value = prev_fsmag[i];
					q_value = weighted_fsmag[i];

					/*      par[0].fs_mag[i] = (p + p + q)/3.0; */
					temp1 = melpe_mult(p_value, X0667_Q15);
					temp2 = melpe_mult(q_value, X0333_Q15);
					par[0].fs_mag[i] = melpe_add(temp1, temp2);	/* Q13 */
					/*      par[1].fs_mag[i] = (p + q + q)/3.0; */
					temp1 = melpe_mult(p_value, X0333_Q15);
					temp2 = melpe_mult(q_value, X0667_Q15);
					par[1].fs_mag[i] = melpe_add(temp1, temp2);	/* Q13 */
				}
			}
		}
	}

	prev_uv = par[NF - 1].uv_flag;
	if (par[NF - 1].uv_flag) {
		fill(prev_fsmag, ONE_Q13, NUM_HARM);
		window_Q(prev_fsmag, w_fs, prev_fsmag, NUM_HARM, 14);
	} else
		v_equ(prev_fsmag, par[NF - 1].fs_mag, NUM_HARM);

	/* ====== Decode jitter information ====== */
	for (i = 0; i < NF; i++) {
		if (par[i].uv_flag == 1)
			par[i].jitter = MAX_JITTER_Q15;
		else
			par[i].jitter = 0;
	}

	if (cnt != 0 || !(uv1 == 0 && uv2 == 1 && cuv == 0)) {
		if (qpar->jit_index[0] == 1) {
			if (uv1 && uv2 && cuv) ;	/* UUU */
			else if (uv1 && uv2 && !cuv)	/* UUV */
				par[2].jitter = MAX_JITTER_Q15;
			else if (uv1 && !uv2 && cuv)	/* UVU */
				par[1].jitter = MAX_JITTER_Q15;
			else if (!uv1 && uv2 && cuv)	/* VUU */
				par[0].jitter = MAX_JITTER_Q15;
			else if (uv1 && !uv2 && !cuv)	/* UVV */
				par[1].jitter = MAX_JITTER_Q15;
			else if (!uv1 && uv2 && !cuv) ;	/* VUV */
			else if (!uv1 && !uv2 && cuv)	/* VVU */
				par[1].jitter = MAX_JITTER_Q15;
			else	/* VVV */
				par[0].jitter = par[1].jitter = par[2].jitter =
				    MAX_JITTER_Q15;
		}
	}

	if (flag_parity) {
		p_value = prev_par->pitch;
		for (i = 0; i < NF; i++) {
			if (par[i].uv_flag)
				par[i].pitch = UV_PITCH_Q7;
			else
				par[i].pitch = melpe_add(melpe_mult(SMOOTH, p_value),
						   melpe_mult(melpe_sub(32767, SMOOTH),
							par[i].pitch));
			p_value = par[i].pitch;
		}
		p_value = prev_par->gain[1];
		for (i = 0; i < NF; i++) {
			for (j = 0; j < NUM_GAINFR; j++) {
				par[i].gain[j] = melpe_add(melpe_mult(SMOOTH, p_value),
						     melpe_mult(melpe_sub(32767, SMOOTH),
							  par[i].gain[j]));
				p_value = par[i].gain[j];
			}
		}
		for (j = 0; j < LPC_ORD; j++) {
			p_value = prev_par->lsf[j];
			for (i = 0; i < NF; i++) {
				par[i].lsf[j] = melpe_add(melpe_mult(SMOOTH, p_value),
						    melpe_mult(melpe_sub(32767, SMOOTH),
							 par[i].lsf[j]));
				p_value = par[i].lsf[j];
			}
		}
	} else {

		/* gain check */

		L_sum1 = 0;
		for (i = 0; i < 2 * NF * NUM_GAINFR; i++)
			L_sum1 = melpe_L_add(L_sum1, melpe_L_deposit_l(prev_gain[i]));
		L_sum1 = melpe_L_shr(L_sum1, 1);

		L_sum2 = 0;
		for (i = 0; i < NF; i++) {
			for (j = 0; j < NUM_GAINFR; j++)
				L_sum2 =
				    melpe_L_add(L_sum2, melpe_L_deposit_l(par[i].gain[j]));
		}

		if (L_sum2 > melpe_L_add(L_sum1, 92160L)
		    || L_sum2 < melpe_L_sub(L_sum1, 92160L)) {
			L_sum1 = melpe_L_shr(L_sum1, 1);
			L_sum1 = L_mpy_ls(L_sum1, 10923);
			temp1 = melpe_extract_l(L_sum1);
			for (i = 0; i < NF; i++) {
				for (j = 0; j < NUM_GAINFR; j++) {
					par[i].gain[j] =
					    melpe_add(melpe_mult(SMOOTH, temp1),
						melpe_mult(melpe_sub(32767, SMOOTH),
						     par[i].gain[j]));
					temp1 = par[i].gain[j];
				}
			}
		}
	}

	for (i = 0; i < NF * NUM_GAINFR; i++)
		prev_gain[i] = prev_gain[i + NF * NUM_GAINFR];
	for (i = 0; i < NF; i++) {
		for (j = 0; j < NUM_GAINFR; j++)
			prev_gain[NF * NUM_GAINFR + i * NUM_GAINFR + j] =
			    par[i].gain[j];
	}

	if (erase) {
		for (i = 0; i < NF; i++) {
			par[i].pitch = UV_PITCH_Q7;
			v_equ(par[i].lsf, prev_par->lsf, LPC_ORD);
			for (j = 0; j < NUM_GAINFR; j++)
				par[i].gain[j] = prev_par->gain[1];
			fill(&(par[i].bpvc[0]), 0, NUM_BANDS);
			fill(&(par[i].fs_mag[0]), ONE_Q13, NUM_HARM);
			par[i].jitter = MAX_JITTER_Q15;
		}
		v_equ(qplsp, par[NF - 1].lsf, LPC_ORD);
		prev_uv = 1;
		fill(prev_fsmag, ONE_Q13, NUM_HARM);
	} else if (erase_uuu) {
		for (i = 0; i < NF; i++) {
			par[i].pitch = UV_PITCH_Q7;
			fill(&(par[i].bpvc[0]), 0, NUM_BANDS);
			fill(&(par[i].fs_mag[0]), ONE_Q13, NUM_HARM);
			par[i].jitter = MAX_JITTER_Q15;
		}

		prev_uv = 1;
		fill(prev_fsmag, ONE_Q13, NUM_HARM);
	} else if (erase_vvv) {
		for (i = 0; i < NF; i++) {
			fill(&(par[i].bpvc[1]), 0, NUM_BANDS - 1);
			fill(&(par[i].fs_mag[0]), ONE_Q13, NUM_HARM);
		}
		prev_uv = 0;
		fill(prev_fsmag, ONE_Q13, NUM_HARM);
	}

	/* Return erase flag */
	return (erase);
}

int16_t parity(int16_t x, int16_t leng)
{
	register int16_t i;
	int16_t p = 0x0;

	for (i = 0; i < leng; i++) {
		p ^= x & 0x1;
		x >>= 1;
	}
	return (p);
}
