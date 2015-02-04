   ///////////////////////////////////////////////
//
// **************************
// ** ENGLISH - 14/03/2013 **
//
// Project/Software name: sponge.lib
// Author: "Van Gegel" <gegelcopy@ukr.net>
//
// THIS IS A FREE SOFTWARE  AND FOR TEST ONLY!!!
// Please do not use it in the case of life and death
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You’re free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Van Gegel
// * You have to provide a link to the author’s Homepage: <http://torfone.org>
//
///////////////////////////////////////////////


//Set of audio codecs
typedef enum{
    //LowBitrate
    CODEC_CODEC24=0, //450 bps, 1280samp->9 bytes, 2 fpp(320mS)->18bytes
	CODEC_MELPE, //1: 1200bps, 540samp->10 bytes, 5 fpp(337.5mS)->50bytes
	CODEC_CODEC21, //2: 1400bps, 320samp->7bytes, 8 fpp(320mS)->56bytes	
	CODEC_LPC10, //3: 2400bps, 180samp->7bytes, 11 fpp(247,5mS)->77bytes
	CODEC_MELP, //4: 2400bps, 180samp->7bytes, 9 fpp(202,5mS)->63bytes
	CODEC_CODEC22, //5: 3200bps, 160samp->8bytes, 9fpp(180mS)->72bytes
	CODEC_CELP, //6: 4800bps, 240samp->18bytes, 6fpp(180mS)->108bytes
	CODEC_AMRV, //7: AMR (60-200mS, 4750-12200bps, DTX)
	CODEC_LPC, //8: 5600bps, 160samp->14bytes, 9fpp(180mS)->126 bytes
	CODEC_GSMH,//9: 5600bps, 160samp->14bytes, 8fpp(160mS)->112 bytes
	//HQ
	CODEC_G723,//10: 6400bps, 240samp->24bytes, 4fpp(120mS)->96bytes
	CODEC_G729,//11: 8000bps, 80samp->10bytes, 11fpp(110mS)->110bytes
	CODEC_GSME,//12: 12400bps, 160samp->31bytes, 4fpp(100mS)->124 bytes
	CODEC_GSM, //13: 13200bps, 160samp->33bytes, 3fpp(60mS)->99 bytes
	CODEC_ILBC,//14: 13333bps, 240samp->50bytes, 2fpp(60mS)->100bytes
	CODEC_BV16,//15: 16000bps, 40samp->10bytes, 8fpp(40mS)->80bytes
	//vbr codecs
	CODEC_OPUS, //16: 6000vbr, 480sampl->25/61 bytes, 2fpp(120mS)39-122bytes
	CODEC_SILK,//17: 10000vbr, 480samp->42->96 bytes, 1fpp(60mS)
	CODEC_SPEEX //18: 15200vbr, 320 samp->11-75bytes, 1fpp(40mS) 22-86bytes
}CodecSet;

//transmit modes
typedef enum{
    TX_NONE=0, //off
    TX_VAD, //voice activation
    TX_PTT, //push-to-talk
    TX_ON //continuous
}TxMode;


int speex_i(int squal, int svbr, int prepr, int sfpp);
int speex_e(unsigned char* buf, short* speech);
int speex_er(unsigned char* buf, short* speech);
int speex_d(short* speech, unsigned char* buf, int len);
int speex_dr(short* speech, unsigned char* buf, int len);
int speex_n(short* speech, int len);
int speex_r(short* spin, short* spout, int inlen,  int outrate);
void speex_p(int denoise, int agc);
void speex_f(void);

void amr_ini(int dtx);
void amr_fin(void);

void gsm_ini(void);
void gsm_fin(void);
void gsm_a(short *sp, unsigned char* bf);
void gsm_s(unsigned char *bf, short *sp);

void gsmhr_ini(short isDTX);
void gsmhr_fin(void);
int gsmhr_a(short *sp, unsigned char* bf);
void gsmhr_s(unsigned char *bf, short *sp);

int opus_i(void);
int opus_e(unsigned char* buf, short* speech);
int opus_d(short* speech, unsigned char* buf, int len);

void lpc_i(void);
void lpc_e(unsigned char* buf, short* speech);
void lpc_d(short* speech, unsigned char* buf);
void lpc_v(short* speech, unsigned char mode);
void lpc_f(void);

void ilbc_i(int mode);
void ilbc_f(void);
int ilbc_e(unsigned char* buf, short* speech);
int ilbc_d(short* speech, unsigned char* buf);

void bv16_i(void);
void bv16_e(unsigned char* buf, short* speech);
void bv16_d(short* speech, unsigned char* buf);

int codec_type(unsigned char* bf);
int codec_len(int cdk);
int sp_encode(short* sp, unsigned char* bf);
int sp_decode(short* sp, unsigned char* bf);

int playjit(void);
int vox(short* speech, int len, char level, char hist);
int RateChange(short *src, short *dest, int srcLen, int destRate);
void sp_init(void);
void sp_fine(void);
int set_encoder(int cd);
int get_decoder(int cd);
void get_jitter(void);
int amr_setup(char mode, char dtx, char fpp);

void switch_tx(void);
void off_tx(void);
void push_ptt(void);
void go_vad(void);
void playring();

int go_snd(unsigned char* pkt);
int do_snd(unsigned char *pkt);

void setaudio(void);

