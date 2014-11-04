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

#include "sc1200.h"
#include "mathhalf.h"
#include "mat_lib.h"

/* Compiler constants */
#define UV_PIND			0	/* Unvoiced pitch index */
#define INVAL_PIND		1	/* Invalid pitch index */
#define BEP_CORR		-1	/* "Correct" bit error position */
#define BEP_UNCORR		-2	/* "Uncorrectable" bit error position */

static int16_t codewd74[7];
static int16_t codewd84[8];
static int16_t codewd_13x9[13];

/* (7,4) Hamming code tables.  Parity generator matrix. */
static const int16_t pmat74[3][4] = {
	{1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}
};

/* Syndrome table. */
static const int16_t syntab74[8] = { BEP_CORR, 6, 5, 2, 4, 1, 0, 3 };

/* (8,4) extended Hamming code tables.  Parity generator matrix. */
static const int16_t pmat84[4][4] = {
	{1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 0}
};

/* Syndrome->error position lookup table. */
static const int16_t syntab84[16] = {
	BEP_CORR, 7, 6, BEP_UNCORR,
	5, BEP_UNCORR, BEP_UNCORR, 2,
	4, BEP_UNCORR, BEP_UNCORR, 1,
	BEP_UNCORR, 0, 3, BEP_UNCORR
};

/* Pitch index encoding table.  Reserve Hamming weight 0,1 words for unvoiced */
/* pitch value.  Reserve Hamming weight 2 words for invalid (protect against  */
/* single bit voiced pitch errors.  Assign voiced pitch codes to values       */
/* having Hamming weight > 2.                                                 */

static const int16_t pitch_enc[PIT_QLEV + 1] = {
	0x0,			/* UV_PIND */
	0x7,			/* 1 (first pitch QL - note offset) */
	0xB,			/* 2 */
	0xD,			/* 3 */
	0xE,			/* 4 */
	0xF,			/* 5 */
	0x13,			/* 6 */
	0x15,			/* 7 */
	0x16,			/* 8 */
	0x17,			/* 9 */
	0x19,			/* 10 */
	0x1A,			/* 11 */
	0x1B,			/* 12 */
	0x1C,			/* 13 */
	0x1D,			/* 14 */
	0x1E,			/* 15 */
	0x1F,			/* 16 */
	0x23,			/* 17 */
	0x25,			/* 18 */
	0x26,			/* 19 */
	0x27,			/* 20 */
	0x29,			/* 21 */
	0x2A,			/* 22 */
	0x2B,			/* 23 */
	0x2C,			/* 24 */
	0x2D,			/* 25 */
	0x2E,			/* 26 */
	0x2F,			/* 27 */
	0x31,			/* 28 */
	0x32,			/* 29 */
	0x33,			/* 30 */
	0x34,			/* 31 */
	0x35,			/* 32 */
	0x36,			/* 33 */
	0x37,			/* 34 */
	0x38,			/* 35 */
	0x39,			/* 36 */
	0x3A,			/* 37 */
	0x3B,			/* 38 */
	0x3C,			/* 39 */
	0x3D,			/* 40 */
	0x3E,			/* 41 */
	0x3F,			/* 42 */
	0x43,			/* 43 */
	0x45,			/* 44 */
	0x46,			/* 45 */
	0x47,			/* 46 */
	0x49,			/* 47 */
	0x4A,			/* 48 */
	0x4B,			/* 49 */
	0x4C,			/* 50 */
	0x4D,			/* 51 */
	0x4E,			/* 52 */
	0x4F,			/* 53 */
	0x51,			/* 54 */
	0x52,			/* 55 */
	0x53,			/* 56 */
	0x54,			/* 57 */
	0x55,			/* 58 */
	0x56,			/* 59 */
	0x57,			/* 60 */
	0x58,			/* 61 */
	0x59,			/* 62 */
	0x5A,			/* 63 */
	0x5B,			/* 64 */
	0x5C,			/* 65 */
	0x5D,			/* 66 */
	0x5E,			/* 67 */
	0x5F,			/* 68 */
	0x61,			/* 69 */
	0x62,			/* 70 */
	0x63,			/* 71 */
	0x64,			/* 72 */
	0x65,			/* 73 */
	0x66,			/* 74 */
	0x67,			/* 75 */
	0x68,			/* 76 */
	0x69,			/* 77 */
	0x6A,			/* 78 */
	0x6B,			/* 79 */
	0x6C,			/* 80 */
	0x6D,			/* 81 */
	0x6E,			/* 82 */
	0x6F,			/* 83 */
	0x70,			/* 84 */
	0x71,			/* 85 */
	0x72,			/* 86 */
	0x73,			/* 87 */
	0x74,			/* 88 */
	0x75,			/* 89 */
	0x76,			/* 90 */
	0x77,			/* 91 */
	0x78,			/* 92 */
	0x79,			/* 93 */
	0x7A,			/* 94 */
	0x7B,			/* 95 */
	0x7C,			/* 96 */
	0x7D,			/* 97 */
	0x7E,			/* 98 */
	0x7F			/* 99 */
};

const int16_t low_rate_pitch_enc[4][PIT_QLEV] = {
	{0},			/* All zeros */
	{			/* for UUV (6,7) */
	 63, 95, 111, 119, 123, 125, 126, 159, 175, 183,
	 187, 189, 190, 207, 215, 219, 221, 222, 231, 235,
	 237, 238, 243, 245, 246, 249, 250, 252, 287, 303,
	 311, 315, 317, 318, 335, 343, 347, 349, 350, 359,
	 363, 365, 366, 371, 373, 374, 377, 378, 380, 399,
	 407, 411, 413, 414, 423, 427, 429, 430, 435, 437,
	 438, 441, 442, 444, 455, 459, 461, 462, 467, 469,
	 470, 473, 474, 476, 483, 485, 486, 489, 490, 492,
	 497, 498, 500, 504, 127, 191, 223, 239, 247, 251,
	 253, 254, 319, 351, 367, 375, 379, 381, 382},
	{			/* for UVU (4) */
	 15, 23, 27, 29, 30, 39, 43, 45, 46, 51,
	 53, 54, 57, 58, 60, 71, 75, 77, 78, 83,
	 85, 86, 89, 90, 92, 99, 101, 102, 105, 106,
	 108, 113, 114, 116, 120, 135, 139, 141, 142, 147,
	 149, 150, 153, 154, 156, 163, 165, 166, 169, 170,
	 172, 177, 178, 180, 184, 195, 197, 198, 201, 202,
	 204, 209, 210, 212, 216, 225, 226, 228, 232, 240,
	 263, 267, 269, 270, 275, 277, 278, 281, 282, 284,
	 291, 293, 294, 297, 298, 300, 305, 306, 308, 312,
	 323, 325, 326, 329, 330, 332, 337, 338, 340},
	{			/* for VUU (5) */
	 31, 47, 55, 59, 61, 62, 79, 87, 91, 93,
	 94, 103, 107, 109, 110, 115, 117, 118, 121, 122,
	 124, 143, 151, 155, 157, 158, 167, 171, 173, 174,
	 179, 181, 182, 185, 186, 188, 199, 203, 205, 206,
	 211, 213, 214, 217, 218, 220, 227, 229, 230, 233,
	 234, 236, 241, 242, 244, 248, 271, 279, 283, 285,
	 286, 295, 299, 301, 302, 307, 309, 310, 313, 314,
	 316, 327, 331, 333, 334, 339, 341, 342, 345, 346,
	 348, 355, 357, 358, 361, 362, 364, 369, 370, 372,
	 376, 391, 395, 397, 398, 403, 405, 406, 409}
};

/* Pitch index decoding table.  Hamming weight 1 codes map to UV_PIND,        */
/* allowing for 1-bit error correction of unvoiced marker.	Hamming weight 2  */
/* codes map to INVAL_PIND, protecting against 1-bit errors in voiced pitches */
/* creating false unvoiced condition.                                         */

static const int16_t pitch_dec[1 << PIT_BITS] = {
	/* pitch index decoding table */
	UV_PIND,		/* 0x0 */
	UV_PIND,		/* 0x1 */
	UV_PIND,		/* 0x2 */
	INVAL_PIND,		/* 0x3 */
	UV_PIND,		/* 0x4 */
	INVAL_PIND,		/* 0x5 */
	INVAL_PIND,		/* 0x6 */
	2,			/* 0x7 */
	UV_PIND,		/* 0x8 */
	INVAL_PIND,		/* 0x9 */
	INVAL_PIND,		/* 0xA */
	3,			/* 0xB */
	INVAL_PIND,		/* 0xC */
	4,			/* 0xD */
	5,			/* 0xE */
	6,			/* 0xF */
	UV_PIND,		/* 0x10 */
	INVAL_PIND,		/* 0x11 */
	INVAL_PIND,		/* 0x12 */
	7,			/* 0x13 */
	INVAL_PIND,		/* 0x14 */
	8,			/* 0x15 */
	9,			/* 0x16 */
	10,			/* 0x17 */
	INVAL_PIND,		/* 0x18 */
	11,			/* 0x19 */
	12,			/* 0x1A */
	13,			/* 0x1B */
	14,			/* 0x1C */
	15,			/* 0x1D */
	16,			/* 0x1E */
	17,			/* 0x1F */
	UV_PIND,		/* 0x20 */
	INVAL_PIND,		/* 0x21 */
	INVAL_PIND,		/* 0x22 */
	18,			/* 0x23 */
	INVAL_PIND,		/* 0x24 */
	19,			/* 0x25 */
	20,			/* 0x26 */
	21,			/* 0x27 */
	INVAL_PIND,		/* 0x28 */
	22,			/* 0x29 */
	23,			/* 0x2A */
	24,			/* 0x2B */
	25,			/* 0x2C */
	26,			/* 0x2D */
	27,			/* 0x2E */
	28,			/* 0x2F */
	INVAL_PIND,		/* 0x30 */
	29,			/* 0x31 */
	30,			/* 0x32 */
	31,			/* 0x33 */
	32,			/* 0x34 */
	33,			/* 0x35 */
	34,			/* 0x36 */
	35,			/* 0x37 */
	36,			/* 0x38 */
	37,			/* 0x39 */
	38,			/* 0x3A */
	39,			/* 0x3B */
	40,			/* 0x3C */
	41,			/* 0x3D */
	42,			/* 0x3E */
	43,			/* 0x3F */
	UV_PIND,		/* 0x40 */
	INVAL_PIND,		/* 0x41 */
	INVAL_PIND,		/* 0x42 */
	44,			/* 0x43 */
	INVAL_PIND,		/* 0x44 */
	45,			/* 0x45 */
	46,			/* 0x46 */
	47,			/* 0x47 */
	INVAL_PIND,		/* 0x48 */
	48,			/* 0x49 */
	49,			/* 0x4A */
	50,			/* 0x4B */
	51,			/* 0x4C */
	52,			/* 0x4D */
	53,			/* 0x4E */
	54,			/* 0x4F */
	INVAL_PIND,		/* 0x50 */
	55,			/* 0x51 */
	56,			/* 0x52 */
	57,			/* 0x53 */
	58,			/* 0x54 */
	59,			/* 0x55 */
	60,			/* 0x56 */
	61,			/* 0x57 */
	62,			/* 0x58 */
	63,			/* 0x59 */
	64,			/* 0x5A */
	65,			/* 0x5B */
	66,			/* 0x5C */
	67,			/* 0x5D */
	68,			/* 0x5E */
	69,			/* 0x5F */
	INVAL_PIND,		/* 0x60 */
	70,			/* 0x61 */
	71,			/* 0x62 */
	72,			/* 0x63 */
	73,			/* 0x64 */
	74,			/* 0x65 */
	75,			/* 0x66 */
	76,			/* 0x67 */
	77,			/* 0x68 */
	78,			/* 0x69 */
	79,			/* 0x6A */
	80,			/* 0x6B */
	81,			/* 0x6C */
	82,			/* 0x6D */
	83,			/* 0x6E */
	84,			/* 0x6F */
	85,			/* 0x70 */
	86,			/* 0x71 */
	87,			/* 0x72 */
	88,			/* 0x73 */
	89,			/* 0x74 */
	90,			/* 0x75 */
	91,			/* 0x76 */
	92,			/* 0x77 */
	93,			/* 0x78 */
	94,			/* 0x79 */
	95,			/* 0x7A */
	96,			/* 0x7B */
	97,			/* 0x7C */
	98,			/* 0x7D */
	99,			/* 0x7E */
	100			/* 0x7F */
};

const int16_t low_rate_pitch_dec[PITCH_VQ_SIZE] = {
	UV_PIND,		/*   0 */
	UV_PIND,		/*   1 */
	UV_PIND,		/*   2 */
	INVAL_PIND,		/*   3 */
	UV_PIND,		/*   4 */
	INVAL_PIND,		/*   5 */
	INVAL_PIND,		/*   6 */
	INVAL_PIND,		/*   7 */
	UV_PIND,		/*   8 */
	INVAL_PIND,		/*   9 */
	INVAL_PIND,		/*  10 */
	INVAL_PIND,		/*  11 */
	INVAL_PIND,		/*  12 */
	INVAL_PIND,		/*  13 */
	INVAL_PIND,		/*  14 */
	2,			/*  15 *//* UVU */
	UV_PIND,		/*  16 */
	INVAL_PIND,		/*  17 */
	INVAL_PIND,		/*  18 */
	INVAL_PIND,		/*  19 */
	INVAL_PIND,		/*  20 */
	INVAL_PIND,		/*  21 */
	INVAL_PIND,		/*  22 */
	3,			/*  23 *//* UVU */
	INVAL_PIND,		/*  24 */
	INVAL_PIND,		/*  25 */
	INVAL_PIND,		/*  26 */
	4,			/*  27 *//* UVU */
	INVAL_PIND,		/*  28 */
	5,			/*  29 *//* UVU */
	6,			/*  30 *//* UVU */
	2,			/*  31 *//* VUU */
	UV_PIND,		/*  32 */
	INVAL_PIND,		/*  33 */
	INVAL_PIND,		/*  34 */
	INVAL_PIND,		/*  35 */
	INVAL_PIND,		/*  36 */
	INVAL_PIND,		/*  37 */
	INVAL_PIND,		/*  38 */
	7,			/*  39 *//* UVU */
	INVAL_PIND,		/*  40 */
	INVAL_PIND,		/*  41 */
	INVAL_PIND,		/*  42 */
	8,			/*  43 *//* UVU */
	INVAL_PIND,		/*  44 */
	9,			/*  45 *//* UVU */
	10,			/*  46 *//* UVU */
	3,			/*  47 *//* VUU */
	INVAL_PIND,		/*  48 */
	INVAL_PIND,		/*  49 */
	INVAL_PIND,		/*  50 */
	11,			/*  51 *//* UVU */
	INVAL_PIND,		/*  52 */
	12,			/*  53 *//* UVU */
	13,			/*  54 *//* UVU */
	4,			/*  55 *//* VUU */
	INVAL_PIND,		/*  56 */
	14,			/*  57 *//* UVU */
	15,			/*  58 *//* UVU */
	5,			/*  59 *//* VUU */
	16,			/*  60 *//* UVU */
	6,			/*  61 *//* VUU */
	7,			/*  62 *//* VUU */
	2,			/*  63 *//* UUV */
	UV_PIND,		/*  64 */
	INVAL_PIND,		/*  65 */
	INVAL_PIND,		/*  66 */
	INVAL_PIND,		/*  67 */
	INVAL_PIND,		/*  68 */
	INVAL_PIND,		/*  69 */
	INVAL_PIND,		/*  70 */
	17,			/*  71 *//* UVU */
	INVAL_PIND,		/*  72 */
	INVAL_PIND,		/*  73 */
	INVAL_PIND,		/*  74 */
	18,			/*  75 *//* UVU */
	INVAL_PIND,		/*  76 */
	19,			/*  77 *//* UVU */
	20,			/*  78 *//* UVU */
	8,			/*  79 *//* VUU */
	INVAL_PIND,		/*  80 */
	INVAL_PIND,		/*  81 */
	INVAL_PIND,		/*  82 */
	21,			/*  83 *//* UVU */
	INVAL_PIND,		/*  84 */
	22,			/*  85 *//* UVU */
	23,			/*  86 *//* UVU */
	9,			/*  87 *//* VUU */
	INVAL_PIND,		/*  88 */
	24,			/*  89 *//* UVU */
	25,			/*  90 *//* UVU */
	10,			/*  91 *//* VUU */
	26,			/*  92 *//* UVU */
	11,			/*  93 *//* VUU */
	12,			/*  94 *//* VUU */
	3,			/*  95 *//* UUV */
	INVAL_PIND,		/*  96 */
	INVAL_PIND,		/*  97 */
	INVAL_PIND,		/*  98 */
	27,			/*  99 *//* UVU */
	INVAL_PIND,		/* 100 */
	28,			/* 101 *//* UVU */
	29,			/* 102 *//* UVU */
	13,			/* 103 *//* VUU */
	INVAL_PIND,		/* 104 */
	30,			/* 105 *//* UVU */
	31,			/* 106 *//* UVU */
	14,			/* 107 *//* VUU */
	32,			/* 108 *//* UVU */
	15,			/* 109 *//* VUU */
	16,			/* 110 *//* VUU */
	4,			/* 111 *//* UUV */
	INVAL_PIND,		/* 112 */
	33,			/* 113 *//* UVU */
	34,			/* 114 *//* UVU */
	17,			/* 115 *//* VUU */
	35,			/* 116 *//* UVU */
	18,			/* 117 *//* VUU */
	19,			/* 118 *//* VUU */
	5,			/* 119 *//* UUV */
	36,			/* 120 *//* UVU */
	20,			/* 121 *//* VUU */
	21,			/* 122 *//* VUU */
	6,			/* 123 *//* UUV */
	22,			/* 124 *//* VUU */
	7,			/* 125 *//* UUV */
	8,			/* 126 *//* UUV */
	86,			/* 127 *//* UUV */
	UV_PIND,		/* 128 */
	INVAL_PIND,		/* 129 */
	INVAL_PIND,		/* 130 */
	INVAL_PIND,		/* 131 */
	INVAL_PIND,		/* 132 */
	INVAL_PIND,		/* 133 */
	INVAL_PIND,		/* 134 */
	37,			/* 135 *//* UVU */
	INVAL_PIND,		/* 136 */
	INVAL_PIND,		/* 137 */
	INVAL_PIND,		/* 138 */
	38,			/* 139 *//* UVU */
	INVAL_PIND,		/* 140 */
	39,			/* 141 *//* UVU */
	40,			/* 142 *//* UVU */
	23,			/* 143 *//* VUU */
	INVAL_PIND,		/* 144 */
	INVAL_PIND,		/* 145 */
	INVAL_PIND,		/* 146 */
	41,			/* 147 *//* UVU */
	INVAL_PIND,		/* 148 */
	42,			/* 149 *//* UVU */
	43,			/* 150 *//* UVU */
	24,			/* 151 *//* VUU */
	INVAL_PIND,		/* 152 */
	44,			/* 153 *//* UVU */
	45,			/* 154 *//* UVU */
	25,			/* 155 *//* VUU */
	46,			/* 156 *//* UVU */
	26,			/* 157 *//* VUU */
	27,			/* 158 *//* VUU */
	9,			/* 159 *//* UUV */
	INVAL_PIND,		/* 160 */
	INVAL_PIND,		/* 161 */
	INVAL_PIND,		/* 162 */
	47,			/* 163 *//* UVU */
	INVAL_PIND,		/* 164 */
	48,			/* 165 *//* UVU */
	49,			/* 166 *//* UVU */
	28,			/* 167 *//* VUU */
	INVAL_PIND,		/* 168 */
	50,			/* 169 *//* UVU */
	51,			/* 170 *//* UVU */
	29,			/* 171 *//* VUU */
	52,			/* 172 *//* UVU */
	30,			/* 173 *//* VUU */
	31,			/* 174 *//* VUU */
	10,			/* 175 *//* UUV */
	INVAL_PIND,		/* 176 */
	53,			/* 177 *//* UVU */
	54,			/* 178 *//* UVU */
	32,			/* 179 *//* VUU */
	55,			/* 180 *//* UVU */
	33,			/* 181 *//* VUU */
	34,			/* 182 *//* VUU */
	11,			/* 183 *//* UUV */
	56,			/* 184 *//* UVU */
	35,			/* 185 *//* VUU */
	36,			/* 186 *//* VUU */
	12,			/* 187 *//* UUV */
	37,			/* 188 *//* VUU */
	13,			/* 189 *//* UUV */
	14,			/* 190 *//* UUV */
	87,			/* 191 *//* UUV */
	INVAL_PIND,		/* 192 */
	INVAL_PIND,		/* 193 */
	INVAL_PIND,		/* 194 */
	57,			/* 195 *//* UVU */
	INVAL_PIND,		/* 196 */
	58,			/* 197 *//* UVU */
	59,			/* 198 *//* UVU */
	38,			/* 199 *//* VUU */
	INVAL_PIND,		/* 200 */
	60,			/* 201 *//* UVU */
	61,			/* 202 *//* UVU */
	39,			/* 203 *//* VUU */
	62,			/* 204 *//* UVU */
	40,			/* 205 *//* VUU */
	41,			/* 206 *//* VUU */
	15,			/* 207 *//* UUV */
	INVAL_PIND,		/* 208 */
	63,			/* 209 *//* UVU */
	64,			/* 210 *//* UVU */
	42,			/* 211 *//* VUU */
	65,			/* 212 *//* UVU */
	43,			/* 213 *//* VUU */
	44,			/* 214 *//* VUU */
	16,			/* 215 *//* UUV */
	66,			/* 216 *//* UVU */
	45,			/* 217 *//* VUU */
	46,			/* 218 *//* VUU */
	17,			/* 219 *//* UUV */
	47,			/* 220 *//* VUU */
	18,			/* 221 *//* UUV */
	19,			/* 222 *//* UUV */
	88,			/* 223 *//* UUV */
	INVAL_PIND,		/* 224 */
	67,			/* 225 *//* UVU */
	68,			/* 226 *//* UVU */
	48,			/* 227 *//* VUU */
	69,			/* 228 *//* UVU */
	49,			/* 229 *//* VUU */
	50,			/* 230 *//* VUU */
	20,			/* 231 *//* UUV */
	70,			/* 232 *//* UVU */
	51,			/* 233 *//* VUU */
	52,			/* 234 *//* VUU */
	21,			/* 235 *//* UUV */
	53,			/* 236 *//* VUU */
	22,			/* 237 *//* UUV */
	23,			/* 238 *//* UUV */
	89,			/* 239 *//* UUV */
	71,			/* 240 *//* UVU */
	54,			/* 241 *//* VUU */
	55,			/* 242 *//* VUU */
	24,			/* 243 *//* UUV */
	56,			/* 244 *//* VUU */
	25,			/* 245 *//* UUV */
	26,			/* 246 *//* UUV */
	90,			/* 247 *//* UUV */
	57,			/* 248 *//* VUU */
	27,			/* 249 *//* UUV */
	28,			/* 250 *//* UUV */
	91,			/* 251 *//* UUV */
	29,			/* 252 *//* UUV */
	92,			/* 253 *//* UUV */
	93,			/* 254 *//* UUV */
	INVAL_PIND,		/* 255 */
	UV_PIND,		/* 256 */
	INVAL_PIND,		/* 257 */
	INVAL_PIND,		/* 258 */
	INVAL_PIND,		/* 259 */
	INVAL_PIND,		/* 260 */
	INVAL_PIND,		/* 261 */
	INVAL_PIND,		/* 262 */
	72,			/* 263 *//* UVU */
	INVAL_PIND,		/* 264 */
	INVAL_PIND,		/* 265 */
	INVAL_PIND,		/* 266 */
	73,			/* 267 *//* UVU */
	INVAL_PIND,		/* 268 */
	74,			/* 269 *//* UVU */
	75,			/* 270 *//* UVU */
	58,			/* 271 *//* VUU */
	INVAL_PIND,		/* 272 */
	INVAL_PIND,		/* 273 */
	INVAL_PIND,		/* 274 */
	76,			/* 275 *//* UVU */
	INVAL_PIND,		/* 276 */
	77,			/* 277 *//* UVU */
	78,			/* 278 *//* UVU */
	59,			/* 279 *//* VUU */
	INVAL_PIND,		/* 280 */
	79,			/* 281 *//* UVU */
	80,			/* 282 *//* UVU */
	60,			/* 283 *//* VUU */
	81,			/* 284 *//* UVU */
	61,			/* 285 *//* VUU */
	62,			/* 286 *//* VUU */
	30,			/* 287 *//* UUV */
	INVAL_PIND,		/* 288 */
	INVAL_PIND,		/* 289 */
	INVAL_PIND,		/* 290 */
	82,			/* 291 *//* UVU */
	INVAL_PIND,		/* 292 */
	83,			/* 293 *//* UVU */
	84,			/* 294 *//* UVU */
	63,			/* 295 *//* VUU */
	INVAL_PIND,		/* 296 */
	85,			/* 297 *//* UVU */
	86,			/* 298 *//* UVU */
	64,			/* 299 *//* VUU */
	87,			/* 300 *//* UVU */
	65,			/* 301 *//* VUU */
	66,			/* 302 *//* VUU */
	31,			/* 303 *//* UUV */
	INVAL_PIND,		/* 304 */
	88,			/* 305 *//* UVU */
	89,			/* 306 *//* UVU */
	67,			/* 307 *//* VUU */
	90,			/* 308 *//* UVU */
	68,			/* 309 *//* VUU */
	69,			/* 310 *//* VUU */
	32,			/* 311 *//* UUV */
	91,			/* 312 *//* UVU */
	70,			/* 313 *//* VUU */
	71,			/* 314 *//* VUU */
	33,			/* 315 *//* UUV */
	72,			/* 316 *//* VUU */
	34,			/* 317 *//* UUV */
	35,			/* 318 *//* UUV */
	94,			/* 319 *//* UUV */
	INVAL_PIND,		/* 320 */
	INVAL_PIND,		/* 321 */
	INVAL_PIND,		/* 322 */
	92,			/* 323 *//* UVU */
	INVAL_PIND,		/* 324 */
	93,			/* 325 *//* UVU */
	94,			/* 326 *//* UVU */
	73,			/* 327 *//* VUU */
	INVAL_PIND,		/* 328 */
	95,			/* 329 *//* UVU */
	96,			/* 330 *//* UVU */
	74,			/* 331 *//* VUU */
	97,			/* 332 *//* UVU */
	75,			/* 333 *//* VUU */
	76,			/* 334 *//* VUU */
	36,			/* 335 *//* UUV */
	INVAL_PIND,		/* 336 */
	98,			/* 337 *//* UVU */
	99,			/* 338 *//* UVU */
	77,			/* 339 *//* VUU */
	100,			/* 340 *//* UVU */
	78,			/* 341 *//* VUU */
	79,			/* 342 *//* VUU */
	37,			/* 343 *//* UUV */
	INVAL_PIND,		/* 344 */
	80,			/* 345 *//* VUU */
	81,			/* 346 *//* VUU */
	38,			/* 347 *//* UUV */
	82,			/* 348 *//* VUU */
	39,			/* 349 *//* UUV */
	40,			/* 350 *//* UUV */
	95,			/* 351 *//* UUV */
	INVAL_PIND,		/* 352 */
	INVAL_PIND,		/* 353 */
	INVAL_PIND,		/* 354 */
	83,			/* 355 *//* VUU */
	INVAL_PIND,		/* 356 */
	84,			/* 357 *//* VUU */
	85,			/* 358 *//* VUU */
	41,			/* 359 *//* UUV */
	INVAL_PIND,		/* 360 */
	86,			/* 361 *//* VUU */
	87,			/* 362 *//* VUU */
	42,			/* 363 *//* UUV */
	88,			/* 364 *//* VUU */
	43,			/* 365 *//* UUV */
	44,			/* 366 *//* UUV */
	96,			/* 367 *//* UUV */
	INVAL_PIND,		/* 368 */
	89,			/* 369 *//* VUU */
	90,			/* 370 *//* VUU */
	45,			/* 371 *//* UUV */
	91,			/* 372 *//* VUU */
	46,			/* 373 *//* UUV */
	47,			/* 374 *//* UUV */
	97,			/* 375 *//* UUV */
	92,			/* 376 *//* VUU */
	48,			/* 377 *//* UUV */
	49,			/* 378 *//* UUV */
	98,			/* 379 *//* UUV */
	50,			/* 380 *//* UUV */
	99,			/* 381 *//* UUV */
	100,			/* 382 *//* UUV */
	INVAL_PIND,		/* 383 */
	INVAL_PIND,		/* 384 */
	INVAL_PIND,		/* 385 */
	INVAL_PIND,		/* 386 */
	INVAL_PIND,		/* 387 */
	INVAL_PIND,		/* 388 */
	INVAL_PIND,		/* 389 */
	INVAL_PIND,		/* 390 */
	93,			/* 391 *//* VUU */
	INVAL_PIND,		/* 392 */
	INVAL_PIND,		/* 393 */
	INVAL_PIND,		/* 394 */
	94,			/* 395 *//* VUU */
	INVAL_PIND,		/* 396 */
	95,			/* 397 *//* VUU */
	96,			/* 398 *//* VUU */
	51,			/* 399 *//* UUV */
	INVAL_PIND,		/* 400 */
	INVAL_PIND,		/* 401 */
	INVAL_PIND,		/* 402 */
	97,			/* 403 *//* VUU */
	INVAL_PIND,		/* 404 */
	98,			/* 405 *//* VUU */
	99,			/* 406 *//* VUU */
	52,			/* 407 *//* UUV */
	INVAL_PIND,		/* 408 */
	100,			/* 409 *//* VUU */
	INVAL_PIND,		/* 410 */
	53,			/* 411 *//* UUV */
	INVAL_PIND,		/* 412 */
	54,			/* 413 *//* UUV */
	55,			/* 414 *//* UUV */
	INVAL_PIND,		/* 415 */
	INVAL_PIND,		/* 416 */
	INVAL_PIND,		/* 417 */
	INVAL_PIND,		/* 418 */
	INVAL_PIND,		/* 419 */
	INVAL_PIND,		/* 420 */
	INVAL_PIND,		/* 421 */
	INVAL_PIND,		/* 422 */
	56,			/* 423 *//* UUV */
	INVAL_PIND,		/* 424 */
	INVAL_PIND,		/* 425 */
	INVAL_PIND,		/* 426 */
	57,			/* 427 *//* UUV */
	INVAL_PIND,		/* 428 */
	58,			/* 429 *//* UUV */
	59,			/* 430 *//* UUV */
	INVAL_PIND,		/* 431 */
	INVAL_PIND,		/* 432 */
	INVAL_PIND,		/* 433 */
	INVAL_PIND,		/* 434 */
	60,			/* 435 *//* UUV */
	INVAL_PIND,		/* 436 */
	61,			/* 437 *//* UUV */
	62,			/* 438 *//* UUV */
	INVAL_PIND,		/* 439 */
	INVAL_PIND,		/* 440 */
	63,			/* 441 *//* UUV */
	64,			/* 442 *//* UUV */
	INVAL_PIND,		/* 443 */
	65,			/* 444 *//* UUV */
	INVAL_PIND,		/* 445 */
	INVAL_PIND,		/* 446 */
	INVAL_PIND,		/* 447 */
	INVAL_PIND,		/* 448 */
	INVAL_PIND,		/* 449 */
	INVAL_PIND,		/* 450 */
	INVAL_PIND,		/* 451 */
	INVAL_PIND,		/* 452 */
	INVAL_PIND,		/* 453 */
	INVAL_PIND,		/* 454 */
	66,			/* 455 *//* UUV */
	INVAL_PIND,		/* 456 */
	INVAL_PIND,		/* 457 */
	INVAL_PIND,		/* 458 */
	67,			/* 459 *//* UUV */
	INVAL_PIND,		/* 460 */
	68,			/* 461 *//* UUV */
	69,			/* 462 *//* UUV */
	INVAL_PIND,		/* 463 */
	INVAL_PIND,		/* 464 */
	INVAL_PIND,		/* 465 */
	INVAL_PIND,		/* 466 */
	70,			/* 467 *//* UUV */
	INVAL_PIND,		/* 468 */
	71,			/* 469 *//* UUV */
	72,			/* 470 *//* UUV */
	INVAL_PIND,		/* 471 */
	INVAL_PIND,		/* 472 */
	73,			/* 473 *//* UUV */
	74,			/* 474 *//* UUV */
	INVAL_PIND,		/* 475 */
	75,			/* 476 *//* UUV */
	INVAL_PIND,		/* 477 */
	INVAL_PIND,		/* 478 */
	INVAL_PIND,		/* 479 */
	INVAL_PIND,		/* 480 */
	INVAL_PIND,		/* 481 */
	INVAL_PIND,		/* 482 */
	76,			/* 483 *//* UUV */
	INVAL_PIND,		/* 484 */
	77,			/* 485 *//* UUV */
	78,			/* 486 *//* UUV */
	INVAL_PIND,		/* 487 */
	INVAL_PIND,		/* 488 */
	79,			/* 489 *//* UUV */
	80,			/* 490 *//* UUV */
	INVAL_PIND,		/* 491 */
	81,			/* 492 *//* UUV */
	INVAL_PIND,		/* 493 */
	INVAL_PIND,		/* 494 */
	INVAL_PIND,		/* 495 */
	INVAL_PIND,		/* 496 */
	82,			/* 497 *//* UUV */
	83,			/* 498 *//* UUV */
	INVAL_PIND,		/* 499 */
	84,			/* 500 *//* UUV */
	INVAL_PIND,		/* 501 */
	INVAL_PIND,		/* 502 */
	INVAL_PIND,		/* 503 */
	85,			/* 504 *//* UUV */
	INVAL_PIND,		/* 505 */
	INVAL_PIND,		/* 506 */
	INVAL_PIND,		/* 507 */
	INVAL_PIND,		/* 508 */
	INVAL_PIND,		/* 509 */
	INVAL_PIND,		/* 510 */
	INVAL_PIND,		/* 511 */
};

/* ----- Prototypes ----- */
static int16_t binprod_int(int16_t x[], const int16_t y[], int16_t n);

static void vgetbits(int16_t dest[], int16_t source, int16_t bit_pos,
		     int16_t n);

static void vsetbits(int16_t * dest, int16_t bit_pos, int16_t n,
		     int16_t source[]);

static void sbc_enc(int16_t x[], int16_t n, int16_t k,
		    const int16_t pmat[]);

static int16_t sbc_dec(int16_t x[], int16_t n, int16_t k,
			 const int16_t pmat[], const int16_t syntab[]);

static int16_t sbc_syn(int16_t x[], int16_t n, int16_t k,
			 const int16_t pmat[]);

static void crc4_enc(int16_t bit[], int16_t num_bits);

static int16_t crc4_dec(int16_t bit[], int16_t num_bits);

/*
    Name: fec_code.c, fec_decode.c
    Description: Encode/decode FEC (Hamming codes) on selected MELP parameters
    Inputs:
      MELP parameter structure
    Outputs:
      updated MELP parameter structure
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

void fec_code(struct quant_param *qpar)
{
	/* Increment pitch index to allow for unvoiced pitch code */
	qpar->pitch_index++;

	/* Unvoiced case - use spare parameter bits for error protection. */
	if (qpar->uv_flag[0]) {
		/* Set pitch index to unvoiced value */
		qpar->pitch_index = UV_PIND;

		/* Code 4 MSB of first vq stage index using (8,4) Hamming code;       */
		/* parity bits in bpvc index.                                         */

		vgetbits(codewd84, qpar->msvq_index[0], 6, 4);
		sbc_enc(codewd84, 8, 4, &pmat84[0][0]);
		vsetbits(qpar->bpvc_index, 3, 4, &codewd84[4]);

		/* Code 3 LSB of first vq stage index using (7,4) Hamming code;       */
		/* parity bits in 3 MSB of fsvq index.                                */

		vgetbits(codewd74, qpar->msvq_index[0], 2, 3);
		codewd74[3] = 0;
		sbc_enc(codewd74, 7, 4, &pmat74[0][0]);
		vsetbits(&(qpar->fsvq_index), 7, 3, &codewd74[4]);

		/* Code 4 MSB of second gain index using (7,4) Hamming code; parity   */
		/* bits in next 3 MSB of fsvq index.                                  */

		vgetbits(codewd74, qpar->gain_index[1], 4, 4);
		sbc_enc(codewd74, 7, 4, &pmat74[0][0]);
		vsetbits(&(qpar->fsvq_index), 4, 3, &codewd74[4]);

		/* Code LSB of second gain index, first gain index using (7,4)        */
		/* Hamming code; parity bits in 2 LSB of fsvq index, jitter index     */
		/* bit.                                                               */

		vgetbits(codewd74, qpar->gain_index[1], 0, 1);
		vgetbits(&codewd74[1], qpar->gain_index[0], 2, 3);
		sbc_enc(codewd74, 7, 4, &pmat74[0][0]);
		vsetbits(&(qpar->fsvq_index), 1, 2, &codewd74[4]);
		vsetbits(qpar->jit_index, 0, 1, &codewd74[6]);
	}

	/* Encode pitch index */
	qpar->pitch_index = pitch_enc[qpar->pitch_index];
}

void low_rate_fec_code(struct quant_param *par)
{
	if (par->uv_flag[0] && par->uv_flag[1] && par->uv_flag[2]) {

		/* for 4 MSB of Gain */
		vgetbits(codewd84, par->gain_index[0], 9, 4);
		sbc_enc(codewd84, 8, 4, &pmat84[0][0]);
		vsetbits(&(par->fs_index), 7, 4, &codewd84[4]);

		/* for next 4 MSB of Gain */
		vgetbits(codewd84, par->gain_index[0], 5, 4);
		sbc_enc(codewd84, 8, 4, &pmat84[0][0]);
		vsetbits(&(par->fs_index), 3, 4, &codewd84[4]);

		/* for 3 LSB of Gain */
		vgetbits(codewd74, par->gain_index[0], 1, 2);
		codewd74[2] = codewd74[3] = 0;
		sbc_enc(codewd74, 7, 4, &pmat74[0][0]);
		vsetbits(&(par->bpvc_index[0]), 1, 2, &codewd74[4]);
		vsetbits(&(par->jit_index[0]), 0, 1, &codewd74[6]);

		/* for LSPs of 1st frame */
		vgetbits(&codewd_13x9[4], par->lsf_index[0][0], 8, 9);
		crc4_enc(codewd_13x9, 9);
		vsetbits(&(par->lsf_index[0][1]), 3, 4, &codewd_13x9[0]);

		/* for LSPs of 2nd frame */
		vgetbits(&codewd_13x9[4], par->lsf_index[1][0], 8, 9);
		crc4_enc(codewd_13x9, 9);
		vsetbits(&(par->lsf_index[1][1]), 3, 4, &codewd_13x9[0]);

		/* for LSPs of 3rd frame */
		vgetbits(&codewd_13x9[4], par->lsf_index[2][0], 8, 9);
		crc4_enc(codewd_13x9, 9);
		vsetbits(&(par->lsf_index[2][1]), 3, 4, &codewd_13x9[0]);
	}
}

int16_t fec_decode(struct quant_param *qpar, int16_t erase)
{
	int16_t berr_pos;

	/* Decode pitch index */
	qpar->pitch_index = pitch_dec[qpar->pitch_index];

	/* Set unvoiced flag for pitch index of UV_PIND; set erase flag for       */
	/* invalid pitch index INVAL_PIND.  Otherwise, convert pitch index into   */
	/* quantization level.                                                    */

	qpar->uv_flag[0] = (BOOLEAN) (qpar->pitch_index == UV_PIND);
	if (!qpar->uv_flag[0]) {
		erase |= (qpar->pitch_index == INVAL_PIND);
		if (!erase)
			qpar->pitch_index -= 2;	/* Subtract to acct. for */
		/* reserved pitch codes. */
	}

	if (qpar->uv_flag[0] && !erase) {

		/* Unvoiced case - use spare parameter bits for error control coding. */
		/* Decode 4 MSB of first vq stage index using (8,4) Hamming code;     */
		/* parity bits in bpvc index.  Set bpvc index to zero.                */

		vgetbits(codewd84, qpar->msvq_index[0], 6, 4);
		vgetbits(&codewd84[4], qpar->bpvc_index[0], 3, 4);
		berr_pos = sbc_dec(codewd84, 8, 4, &pmat84[0][0], syntab84);
		erase |= (berr_pos == BEP_UNCORR);
		vsetbits(qpar->msvq_index, 6, 4, codewd84);
		qpar->bpvc_index[0] = 0;

		/* Perform remaining decoding only if no frame repeat flagged. */
		if (!erase) {

			/* Decode 3 LSB of first vq stage index using (7,4) Hamming code; */
			/* parity bits in 3 MSB of fsvq index.                            */

			vgetbits(codewd74, qpar->msvq_index[0], 2, 3);
			codewd74[3] = 0;
			vgetbits(&codewd74[4], qpar->fsvq_index, 7, 3);
			sbc_dec(codewd74, 7, 4, &pmat74[0][0], syntab74);
			vsetbits(qpar->msvq_index, 2, 3, codewd74);

			/* Decode 4 MSB of second gain index using (7,4) Hamming code;    */
			/* parity bits in next 3 MSB of fsvq index.                       */

			vgetbits(codewd74, qpar->gain_index[1], 4, 4);
			vgetbits(&codewd74[4], qpar->fsvq_index, 4, 3);
			sbc_dec(codewd74, 7, 4, &pmat74[0][0], syntab74);
			vsetbits(&(qpar->gain_index[1]), 4, 4, codewd74);

			/* Decode LSB of second gain index, first gain index using (7,4)  */
			/* Hamming code; parity bits in 2 LSB of fsvq index, jitter index */
			/* bit.  Set jitter index bits to one.                            */

			vgetbits(codewd74, qpar->gain_index[1], 0, 1);
			vgetbits(&codewd74[1], qpar->gain_index[0], 2, 3);
			vgetbits(&codewd74[4], qpar->fsvq_index, 1, 2);
			vgetbits(&codewd74[6], qpar->jit_index[0], 0, 1);
			sbc_dec(codewd74, 7, 4, &pmat74[0][0], syntab74);
			vsetbits(&(qpar->gain_index[1]), 0, 1, codewd74);
			vsetbits(qpar->gain_index, 2, 3, &codewd74[1]);
			qpar->jit_index[0] = 1;
		}
	}
	return (erase);
}

int16_t low_rate_fec_decode(struct quant_param * qpar, int16_t erase,
			      int16_t lsp_check[])
{
	int16_t berr_pos;

	if (qpar->uv_flag[0] && qpar->uv_flag[1] && qpar->uv_flag[2]) {
		/* for 4 MSB of Gain */
		vgetbits(codewd84, qpar->gain_index[0], 9, 4);
		vgetbits(&codewd84[4], qpar->fs_index, 7, 4);
		berr_pos = sbc_dec(codewd84, 8, 4, &pmat84[0][0], syntab84);
		erase |= berr_pos == BEP_UNCORR;
		vsetbits(qpar->gain_index, 9, 4, codewd84);

		/* for next 4 MSB of Gain */
		vgetbits(codewd84, qpar->gain_index[0], 5, 4);
		vgetbits(&codewd84[4], qpar->fs_index, 3, 4);
		berr_pos = sbc_dec(codewd84, 8, 4, &pmat84[0][0], syntab84);
		erase |= berr_pos == BEP_UNCORR;
		vsetbits(qpar->gain_index, 5, 4, codewd84);

		if (!erase) {

			/* for 2 LSB of Gain */
			vgetbits(codewd74, qpar->gain_index[0], 1, 2);
			codewd74[2] = codewd74[3] = 0;
			vgetbits(&codewd74[4], qpar->bpvc_index[0], 1, 2);
			vgetbits(&codewd74[6], qpar->jit_index[0], 0, 1);
			sbc_dec(codewd74, 7, 4, &pmat74[0][0], syntab74);
			vsetbits(qpar->gain_index, 1, 2, codewd74);

			/* for LSPs of 1st frame */
			vgetbits(&codewd_13x9[4], qpar->lsf_index[0][0], 8, 9);
			vgetbits(&codewd_13x9[0], qpar->lsf_index[0][1], 3, 4);
			lsp_check[0] = crc4_dec(codewd_13x9, 13);

			/* for LSPs of 1st frame */
			vgetbits(&codewd_13x9[4], qpar->lsf_index[1][0], 8, 9);
			vgetbits(&codewd_13x9[0], qpar->lsf_index[1][1], 3, 4);
			lsp_check[1] = crc4_dec(codewd_13x9, 13);

			/* for LSPs of 1st frame */
			vgetbits(&codewd_13x9[4], qpar->lsf_index[2][0], 8, 9);
			vgetbits(&codewd_13x9[0], qpar->lsf_index[2][1], 3, 4);
			lsp_check[2] = crc4_dec(codewd_13x9, 13);
		}
	}
	return (erase);
}

/* binprod returns a bitwise modulo-2 inner product between int arrays x[]    */
/* and y[].                                                                   */

static int16_t binprod_int(int16_t x[], const int16_t y[], int16_t n)
{
	register int16_t i;
	int16_t val = 0;

	for (i = 0; i < n; i++)
		val ^= *x++ & *y++;

	return (val);
}

/* vgetbits() extracts an n-bit pattern beginning at bit position p from an   */
/* integer x, and returns a bit vector containing the pattern.  Conversely,   */
/* vsetbits() takes a length n bit vector and sets the bit pattern in an      */
/* integer.                                                                   */

static void vgetbits(int16_t dest[], int16_t source, int16_t bit_pos,
		     int16_t n)
{
	register int16_t i;
	const int16_t lsb_mask = 0x1;	/* least significant bit mask */

	if ((n >= 0) && (bit_pos >= melpe_sub(n, 1))) {
		source = melpe_shr(source, (int16_t) (bit_pos - n + 1));
		for (i = melpe_sub(n, 1); i >= 0; i--) {
			dest[i] = (int16_t) (source & lsb_mask);
			source = melpe_shr(source, 1);
		}
	}
}

static void vsetbits(int16_t * dest, int16_t bit_pos, int16_t n,
		     int16_t source[])
{
	register int16_t i, j;
	const int16_t lsb_mask = 0x1;

	if ((n >= 0) && (bit_pos >= n - 1)) {
		for (i = 0, j = bit_pos; i < n; i++, j--) {
			*dest &= ~(lsb_mask << j);	/* mask out bit position j */
			*dest |= *(source++) << j;	/* set bit position j to array value */
		}
	}
}

/* Name: code_blk - systematic block error control code functions.            */
/*                                                                            */
/* Description:                                                               */
/*                                                                            */
/*   These functions are designed to implement systematic block codes given   */
/*   input vector and a parity matrix.  These codes are characterized by an   */
/*   leaving the data bits of the protected n-bit block unaltered.  The       */
/*   parity matrix used in these functions is a (n-k) x k matrix which        */
/*   generated the parity bits for a given input block.                       */
/*                                                                            */
/*   sbc_enc() takes a length n bit vector x, applies parity matrix pmat,     */
/*   and writes the parity bits into the last n-k positions of x.             */
/*                                                                            */
/*   sbc_dec() takes x (after processing by sbc_enc) and corrects x for bit   */
/*   errors using a syndrome table lookup.  sbc_dec() returns the index of a  */
/*   detected bit error in x.  sbc_dec() returns -1 if no error is found.     */
/*                                                                            */
/*   sbc_syn() takes x (after processing by sbc_enc) and computes a syndrome  */
/*   index used to look up a bit error position in the syndrome table.        */

static void sbc_enc(int16_t x[], int16_t n, int16_t k,
		    const int16_t pmat[])
{
	register int16_t i;

	for (i = k; i < n; i++, pmat += k)
		x[i] = binprod_int(x, pmat, k);
}

static int16_t sbc_dec(int16_t x[], int16_t n, int16_t k,
			 const int16_t pmat[], const int16_t syntab[])
{
	int16_t bep;

	bep = syntab[sbc_syn(x, n, k, pmat)];
	if (bep > -1)
		x[bep] ^= 0x1;
	return (bep);
}

static int16_t sbc_syn(int16_t x[], int16_t n, int16_t k,
			 const int16_t pmat[])
{
	register int16_t i, j;
	int16_t retval = 0;

	for (i = k, j = (int16_t) (n - k - 1); i < n; i++, j--, pmat += k)
		retval = melpe_add(retval,
			     (int16_t) ((x[i] ^ binprod_int(x, pmat, k)) <<
					  j));
	return (retval);
}

static void crc4_enc(int16_t bit[], int16_t num_bits)
{
	register int16_t i;
	int16_t delay[4], x, ll;

	ll = melpe_add(num_bits, 4);
	v_zap(delay, 4);

	for (i = 1; i <= num_bits; i++) {
		x = (int16_t) (delay[3] ^ bit[ll - i]);

		delay[3] = delay[2];
		delay[2] = delay[1];
		delay[1] = (int16_t) (x ^ delay[0]);
		delay[0] = x;
	}

	v_equ(bit, delay, 4);
}

static int16_t crc4_dec(int16_t bit[], int16_t num_bits)
{
	register int16_t i;
	int16_t delay[4], x;

	for (i = 1; i <= 4; i++)
		delay[4 - i] = bit[num_bits - i];

	for (i = 5; i <= num_bits; i++) {
		x = delay[3];
		delay[3] = delay[2];
		delay[2] = delay[1];
		delay[1] = (int16_t) (x ^ delay[0]);
		delay[0] = (int16_t) (x ^ bit[num_bits - i]);
	}

	x = 0x0;
	for (i = 0; i < 4; i++)
		x |= delay[i];

	return (x);
}
