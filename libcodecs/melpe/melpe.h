/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MELPELIB__
#define  __MELPELIB__
//------------NPP----------------------
	void melpe_n(short *sp);	//denoise 180 samples sp->sp
//------------1200---------------------
	void melpe_i(void);	//init melpe codec at 1200 bps
	void melpe_a(unsigned char *buf, short *sp);	//compress 540 samples sp -> 81 bits buf  
	void melpe_s(short *sp, unsigned char *buf);	//decompress 81 bits buf -> 540 samples sp

#endif
#ifdef __cplusplus
}
#endif
