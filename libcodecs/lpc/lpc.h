/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * LPC subroutine declarations
 */
#define LPC_FILTORDER		10

#define FRAMESIZE   160

#ifdef OLDCC
#define signed
#define const
#endif

typedef struct {
	unsigned short period;
	unsigned char gain;
	char pad;
	signed char k[LPC_FILTORDER];
} lpcparams_t;

/*
 * we can't use 'sizeof(lpcparams_t)' because some compilers
 * add random padding so define size of record that goes over net.
 */
#define LPCRECSIZE 14

typedef struct {
	double Oldper, OldG, Oldk[LPC_FILTORDER + 1], bp[LPC_FILTORDER + 1];
	int pitchctr;
} lpcstate_t;

int lpc_start( /*void */ );
void lpc_init( /*lpcstate_t* state */ );
void lpc_analyze( /*short *buf, lpcparams_t *params */ );
void lpc_synthesize( /*short *buf, lpcparams_t *params, lpcstate_t* state */ );
void lpc_end( /*void */ );
