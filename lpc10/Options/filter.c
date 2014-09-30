#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

/*     --------------------------------  FILTER_CREATE  --------------------------------
*/

FILTER *filter_create(num, nl, den, dl)

float *num, *den;			/*  num[0:nl-1]			den[0:dl-1]             */
int nl, dl;				/*  nl: numerator length	dl: denominator length  */

{
   int i;
   FILTER *fp;

   fp = (FILTER *) malloc(sizeof(FILTER));

   fp->nl = nl;
   fp->num = (float *) malloc((unsigned) fp->nl * sizeof(float));
   for (i=0; i<fp->nl; i++) fp->num[i] = num[i];

   fp->dl = dl;
   fp->den = (float *) malloc((unsigned) fp->dl * sizeof(float));
   for (i=0; i<fp->dl; i++) fp->den[i] = den[i];

   /*   normalize the denominator, if needed   */
   if (fp->den[0] != 1.0)
   {
      for (i=1; i<fp->dl; i++) fp->den[i] = fp->den[i] / fp->den[0];
      for (i=0; i<fp->nl; i++) fp->num[i] = fp->num[i] / fp->den[0];
      fp->den[0] = 1.0;
   }

   /*   determine needed buffer length and create the buffer   */
   if (fp->nl > fp->dl) fp->bufl = fp->nl - 1;
   else                 fp->bufl = fp->dl - 1;
   fp->buf = (float *) malloc((unsigned) fp->bufl * sizeof(float));
   fp->buf -= 1;		/*   buf[1:bufl]   */
   for (i=1; i<=fp->bufl; i++) fp->buf[i] = 0.0;

   return fp;
}

/*     --------------------------------  FILTER  --------------------------------
*/

float filter(fp, in)

FILTER *fp;
float in;

{
   int i;
   float out;

   for (i=1; i<fp->dl; i++) in -= fp->den[i] * fp->buf[i];	/* denominator */
   out = fp->num[0] * in;
   for (i=1; i<fp->nl; i++) out += fp->num[i] * fp->buf[i];	/* numerator */
   for (i=fp->bufl; i>1; i--) fp->buf[i] = fp->buf[i-1];	/* shift buffer */
   fp->buf[1] = in;

   return out;
}

/*     --------------------------------  FILTER_STATE_READ  --------------------------------
*/

float *filter_state_read(fp, n)

FILTER *fp;
int *n;

{
   *n = fp->bufl;
   return fp->buf + 1;
}

/*     --------------------------------  FILTER_STATE_SET  --------------------------------
*/

int filter_state_set(fp, n, x)

FILTER *fp;
int n;
float x[];

{
   int i;
   if (n != fp->bufl) return -1;
   for (i=1; i<=fp->bufl; i++) fp->buf[i] = x[i-1];
   return 0;
}
