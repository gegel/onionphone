/*

    	Definitions for callers of the LPC-10 CODEC
	
*/

extern void lpc10init(void);
extern int lpc10encode(short *in, unsigned char *out, int inlen);
extern int lpc10decode(unsigned char *in, short *out, int inlen);
