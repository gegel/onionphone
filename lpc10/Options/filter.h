typedef struct
{
   float *num;		/* numerator coefficients, subscript is power of z^-1		*/
   float *den;		/* denominator coefficients, subscript is power of z^-1		*/
   float *buf;		/* buffer for filter history					*/
   int nl;		/* # of numerator coefficients					*/
   int dl;		/* # of denominator coefficients				*/
   int bufl;		/* length of buf						*/
} FILTER;

extern FILTER		*filter_create();
extern float		 filter();
extern float		*filter_state_read();
extern int		 filter_state_set();
