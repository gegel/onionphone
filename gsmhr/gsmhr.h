/* HR (GSM 06.20) codec wrapper header */


typedef struct {
	int dec_reset_flg;
} gsmhr;


struct gsmhr *
gsmhr_init(short isDTX);

void
gsmhr_exit(struct gsmhr *state);

int
gsmhr_encode(struct gsmhr *state, unsigned char* rb, const short *pcm);

int
gsmhr_decode(struct gsmhr *state, short *pcm, const unsigned char *rb);
