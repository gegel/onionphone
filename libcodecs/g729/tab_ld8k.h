/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : TAB_LD8K.H
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/
extern float b100[3];
extern float a100[3];
extern float b140[3];
extern float a140[3];
extern float hamwindow[L_WINDOW];
extern float grid[GRID_POINTS + 1];
extern float lspcb1[NC0][M];	/*First Stage Codebook */
extern float lspcb2[NC1][M];	/*Second Stage Codebook */
extern float fg[MODE][MA_NP][M];	/*MA prediction coef.  */
extern float fg_sum[MODE][M];	/*present MA prediction coef. */
extern float fg_sum_inv[MODE][M];	/*inverse coef. */
extern float inter_3[FIR_SIZE_ANA];
extern float inter_3l[FIR_SIZE_SYN];
extern float pred[4];
extern float coef[2][2];
extern float thr1[NCODE1 - NCAN1];
extern float thr2[NCODE2 - NCAN2];
extern float gbk1[NCODE1][2];
extern float gbk2[NCODE2][2];
extern int map1[NCODE1];
extern int map2[NCODE2];
extern int imap1[NCODE1];
extern int imap2[NCODE2];
extern float tab_hup_l[SIZ_TAB_HUP_L];
extern float tab_hup_s[SIZ_TAB_HUP_S];
extern int bitsno[PRM_SIZE];
