/*                              lpcdefs.h                       */

#include <math.h>

#define END 88
#define NOFILE 99

#define LFRAME 180
#define ORDER 10

#define mmax(A,B)        ((A)>(B)?(A):(B))
#define mmin(A,B)        ((A)<(B)?(A):(B))

#define L_nint(x)          ((int) ((x) + 0.5))

/* from analys.c */
#define NF 4
#define AF 3
#define OSLEN 10
#define LTAU 60
#define SBUFL ((AF-2)*MAXFRM+1)                         /* (3-2)*180+1 = 181 */
#define SBUFH (NF*MAXFRM)                               /* 4*180 = 720 */
#define LBUFL ((AF-2)*MAXFRM-MAXPIT+1)                  /* (3-2)*180-156+1 = 25 */
#define LBUFH (NF*MAXFRM)                               /* 4*180 = 720 */
#define MINWIN 90
#define MAXWIN 156
#define PWLEN (MAXPIT+MAXWIN)                           /* 156 + 156 = 312 */
#define PWINH (AF*MAXFRM)                               /* 3 * 180 = 540 */
#define PWINL (PWINH-PWLEN+1)                           /*  540 - 312 + 1 = 229 */
#define DVWINL (PWINH-PWLEN/2 - MAXWIN/2 +1)
#define DVWINH (DVWINL+MAXWIN-1)

#include "config.ch"
#include "lpc10lib.h"
