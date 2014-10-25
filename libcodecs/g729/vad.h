/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
 File : VAD.H
*/

#define     NP            12	/* Increased LPC order */
#define     NOISE         0
#define     VOICE         1
#define     INIT_FRAME    32
#define     INIT_COUNT    20
#define     ZC_START      120
#define     ZC_END        200

void vad_init(void);

void vad(float rc, float * lsf, float * rxx, float * sigpp, int frm_count,
	 int prev_marker, int pprev_marker, int *marker, float * Energy_db);
