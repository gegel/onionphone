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
/*Extremally low bitrate but latency 542 ms but equal jitter compensation) modes:*/
	void melpe_al(unsigned char *buf, short *sp);	//Compress 4320 samples sp (524 mS) -> 648 bits buf (81 bytes)
	void melpe_sl(short *sp, unsigned char *buf);	//decompress 648 bits buf (81 bytes) -> 4320 samples sp (524 mS)

	void melpe_i2(void);	//init melpe codec at 2400 bps
	void melpe_a2(unsigned char *buf, short *sp);	//compress 180 samples sp (22.5 mS) -> 54 bits buf (7 bytes)
	void melpe_s2(short *sp, unsigned char *buf);	//decompress 54 bits buf (7 bytes) -> 180 samples sp (22.5 mS)

#endif
#ifdef __cplusplus
}
#endif
