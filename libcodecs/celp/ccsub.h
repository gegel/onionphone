/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*      CELP ARRAY SIZE PARAMETERS                                       
        MAXNCSIZE       maximum code book size   (number of codewords)
        MAXL            maximum codeword vector length
        MAXCODE         maximum code book array length in samples
        MAXLL           maximum LPC analysis frame size
        MAXNO           maximum LPC filter order
        MAXLP           maximum pitch prediction frame size      
        MAXNP           maximum pitch prediction order   
        MMIN            minimum delay pitch predictor (minimum value M)
        MMAX            maximum delay pitch predictor (maximum value M)
        MAXPD           maximum number of pitch delays   
        MAXPA           maximum pitch analysis buffer length (use to be "idb")
                        (assumes code book is updated faster or same as pitch)
        MAXM2           maximum array size for delay parameter 


*/

#define MAXNCSIZE 512
#define MAXL      60
#define MAXCODE   2*(MAXNCSIZE)+MAXL	/* shift by 2 overlapped code book */
#define MAXLL     240
#define MAXNO     10
#define MAXLP     60		/* bug, fixed - See MAXPA */
#define MAXNP     3
#define MMIN      20
#define MMAX      147
#define MAXPD     256
#define MAXM2     20
#define MAXPA     MAXLP+MMAX+2+MAXM2
