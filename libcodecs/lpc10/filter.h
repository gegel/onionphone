typedef struct
{
   float *num;		/* numerator coefficients, subscript is power of z^-1		*/
   float *den;		/* denominator coefficients, subscript is power of z^-1 	*/
   float *buf;		/* buffer for filter history					*/
   int nl;		/* # of numerator coefficients					*/
   int dl;		/* # of denominator coefficients				*/
   int bufl;		/* length of buf						*/
} FILTER;

extern FILTER		*filter_create(float *num, int nl, float *den, int dl);
extern float		 filter(FILTER *fp, float in);
extern float		*filter_state_read(FILTER *fp, int *n);
extern int		 filter_state_set(FILTER *fp, int n, float x[]);
