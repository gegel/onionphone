/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

    Definitions for callers of the CELP Codec
    
*/

#define CELP_OK 0		/* Decode OK */
#define CELP_TWOERR 1		/* Uncorrectable error in stream bits */
#define CELP_LSPERR 2		/* Error encoding LSPs */
#define CELP_ERR_DCODCBG 3	/* Error decoding cbgain */
#define CELP_ERR_DCODCBI 4	/* Error decoding code book index */
#define CELP_ERR_DELAY	 5	/* Invalid pitch delay */
#define CELP_ERR_GAINDECODE 6	/* Unquantised cbgain in gaincode.c */
#define CELP_ERR_POLEFILT 7	/* Bad coefficients in polefilt.c */
#define CELP_ERR_IMPULSE_LENGTH 8	/* Impulse response too long in psearch.c */
#define CELP_ERR_MAXLP 9	/* MAXLP < MAXL in psearch.c */
#define CELP_ERR_PITCH_TYPE 10	/* Bad pitch search type in psearch.c */
#define CELP_ERR_DCODPG 11	/* Error decoding pitch gain in dcodpg.c */

extern void celp_init(int prot);
extern int celp_encode(short iarf[240], char packedbits[144 / 8]);
extern int celp_decode(char packedbits[144 / 8], short pf[240]);
