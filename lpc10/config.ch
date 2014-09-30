/*   LPC Configuration parameters:
 Frame size, Prediction order, Pitch period
*/

#define MAXFRM 180
#define MAXORD 10
#define MAXPIT 156
#define MAXNB 320

/*
#ifdef LOWRATE
#ifdef LOWRATE_600
#define NFBLK 8
#define MAXNB ((600 * NFBLK) / (8000 / MAXFRM))
#endif
#ifdef LOWRATE_800
#define NFBLK 8
#define MAXNB ((800 * NFBLK) / (8000 / MAXFRM))
#endif
#ifdef LOWRATE_1200
#define NFBLK 4
#define MAXNB ((1200 * NFBLK) / (8000 / MAXFRM))
#endif
#else
#define MAXNB (2400 / (8000 / MAXFRM)
#endif
*/
