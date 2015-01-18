///////////////////////////////////////////////
//
// **************************
//
// Project/Software name: X-Phone
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


//This file contains voice processing procedures for X-Phone (OnionPhone) project

//=========================Codec's Libraries includes================================

#include "../libcodecs/melpe/melpe.h"
///*
#include "../libcodecs/codec2/codec2.h"
#include "../libcodecs/melp/melplib.h"
#include "../libcodecs/lpc10/lpc10.h"
#include "../libcodecs/lpc10/lpc10tools.h"
#include "../libcodecs/celp/celp.h"
#include "../libcodecs/g723/lbccodec.h"
#include "../libcodecs/g729/g729.h"
//*/
#include "../libcodecs/gsmer/gsme.h"
///*
#include "../libcodecs/silk/libsilk.h"
#include "../libcodecs/gsm/inc/gsm.h"
#include "../libcodecs/melp/melplib.h"
//*/
#include "../libcodecs/lpc/lpc.h"
///*
//#include "../libcodecs/opus/include/opus_types.h"
#include "../libcodecs/opus/include/opus_defines.h"
#include "../libcodecs/opus/include/opus.h"
#include "../libcodecs/gsmhr/gsmhr.h"
#include "../libcodecs/ilbc/ilbc.h"
#include "../libcodecs/bv/bv16/bvcommon.h"
#include "../libcodecs/bv/bv16/bv16cnst.h"
#include "../libcodecs/bv/bv16/bv16strct.h"
#include "../libcodecs/bv/bv16/bv16.h"
#include "../libcodecs/bv/bv16/utility.h"
#include "../libcodecs/bv/bv16/bitpack.h"
//*/
#include "../libcodecs/speex/speex/speex.h"
#include "../libcodecs/speex/speex/speex_preprocess.h"
#include "../libcodecs/speex/speex/speex_resampler.h"

#include "../libcodecs/amr/interf_enc.h"
#include "../libcodecs/amr/interf_dec.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32

#include <stddef.h>
#include <stdlib.h>
#include <basetsd.h>
#include <stdint.h>

 #include <windows.h>
 #include <time.h>
#endif

#include "libcrp.h"
#include "crypto.h"
#include "cntrls.h"
#include "audio.h"
#include "codecs.h"
#include "ringwave.h"


#define RAWBUFLEN 240 //samples in raw buffer for preprocessing
//=============================Constants================================
//Codec's names by type
const char* cd_name[]={ "NONE","MELPE", "CODEC2_1","LPC10","MELP","CODEC2_2",
		"CELP",	"AMR","LPC","GSM_HR","G723","G729","GSM_EFR","GSM_FR",
		"ILBC","BV16","OPUS","SILK","SPEEX"}; 
//Codec's speech frame length (in 8 KHz short PCM samples)
const int frm_len[24]={
//		MELPE	CODEC21	LPC10	MELP	CODEC22	CELP	AMR0	LPC	
	0,	540,	320,	180,	180,	160,	240,	160,	160,
//	GSMH	G723	G729	GSME	GSM	ILBC	BV16		
	160,	240,	80,	160,	160,	240,	40,	
//	OPUS	SILK	SPEEX   AMRV
	480,	480,	320,	160,	0,	0,	0, 	0	
};
//Length of compressed frame in bytes
const int buf_len[40]={
//		MELPE	CODEC21	LPC10	MELP	CODEC22	CELP	AMR0	LPC
	0,	10,	7,	7,	7,	8,	18,	12,	14,	
//	GSMH	G723	G729	GSME	GSM	ILBC	BV16
	14,	24,	10,	31,	33,	50,	10,
//	OPUS	SILK	SPEEX   AMRV
	0,	0,	0,	12,	0,	0,	0,	0
};
//Frames per packet
const int frm_ppk[40]={
//		MELPE	CODEC21	LPC10	MELP	CODEC22	CELP	AMR0	LPC
	0,	5,	8,	11,	9,	9,	6,	10,	9,
//	GSMH	G723	G729	GSME	GSM	ILBC	BV16	
	8,	4,	11,	4,	3,	2,	8,
//	OPUS	SILK	SPEEX   AMRV
	2,	1,	1,	10,	0,	0,	0, 	0
};

//==========================Buffers definitions===================================
#define MAX_FRM_LEN 544 //samples in frame (540 samples for melpe)
#define MAX_SND_LEN 3200 //grabbed samples in input buffer 
#define MAX_RDD_LEN 24 //length of speex redudant area in bytes
#define MAX_RESAMPL_BUF 816 //1.5 * max_frame_length (540 samples for melpe)
#define JIT_BUF_LEN 5*MAX_RESAMPL_BUF //5 frames per packet for melpe in jitter buffer
#define DEFRATE 8000 //nominal samles rate
#define MAX_PKT_LEN 128 //length of packet in bytes
#define MAX_PKT 16 //number of packets in circular packets buffer

//-----------audio input--------------------------
int enc_type=0; //encoder type
short raw_buf[2*RAWBUFLEN]; //raw buffer for collect to 160 grabbed samples before preprocessing
short l_raw=0; //actual number of samples in raw buffer
short in_buf[MAX_SND_LEN]; //input buffer to collect preprocessed samples before encoding
short l_in=0; //actual number of samples in input buffer
int snd_need=1440; //number of samples needed for encode compleet packet by specified codec
char tx_flag=0;  //actual transmission state
char etx_flag=0; //TX_ON; //estimated transmittion state (setted by user or by vad detector)
char tx_note=0; //flag for notify remote then transmition disabled
int vad_t=0; //vad trashhold: number of vad-inactive frames still transmitted
unsigned int ptt_flag=0; //timestamp of disabling PTT after TAB key releasing
int Vad_cnt=0;  //counter for frames in VAD tail
char vox_level=90; //Level of amlitude for altenative VAD or SpeexVAD if 0
int sp_npp=1;		//noise reduction
int sp_agc=1;		//mike auto gain
int sp_voc=0;            //LPC vocoder type
char speex_rs=1; //using of speex resampler
char npp7=0; //using npp7 supressor
int RawBufSize=160; //samples for preprocessing
int sp_jit=0; //specified fixed jitter compensation in mS (0 for auto, -1 for no buffer)
int vad_signal=0; //length of noise signal transmitted after vad disabled
int vad_level=0; //level of 3-tone signal (end of remote transmition) 
int vad_tail=1; //number of transmitted inactive frames before squelch
//-----------audio output------------------------
int dec_type=0; //decoder usefull for last incoming packet
unsigned int rx_flg=0; //user id of last packet sender or 0 after alsa underrun
unsigned int rx_flg1=0;
unsigned char pkt_buf[MAX_PKT][MAX_PKT_LEN]; //buffer for incoming undecoded packets
short l_pkt[MAX_PKT]; //average number of samples in packet (before resampling)
short l_pkt_buf=0; //average total number of buffering samples (before resampling)
unsigned char n_pkt=0; //current index of circular packets buffer
short jit_buf[JIT_BUF_LEN]; //buffer for undecoded samples (for jitter compensation)
short jit_buf1[JIT_BUF_LEN]; //silency buffer for prevention underruns after first PTT packets
short* p_jit_buf=jit_buf; //pointer to unplayed samples in the buffer
short l_jit_buf=0; //number of unplayed samples in the buffer
int chunk; //alsa chunk size in samples
int sdelay=0; //actual total number of buffered samples in all buffers (delay)
int last_delay=0; //buffered samples on moment of receiving previous pkt
int up_level=0; //jitter trashhold for increase buffer filling
int down_level=0; //jitter trashhold for decrease buffer filling
int actual_jitter=0; //actual jitter in samples
int est_jit; //estimated number of buffered samples for actual jitter
int max_jitter; //maximum number of buffered samples
int min_jitter; //minimum number of buffered samples
int rd_jit=300; //min_jitter i samples
int crate=DEFRATE; //actual outed sampling rate for resampler
int erate=DEFRATE; //estimated outed sampling rate for resampler (for smoothly rate correction)
static short spp[MAX_FRM_LEN]; //Before resampling decoder's buffer: max frame length + 4 extra
//speex redundant data from previous packet (uses in case of packet loss)
static short speex_rb[MAX_RDD_LEN]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char redundant=0; //flag of single packetloss for playing stored redundant data first
char sound_test=0; //flag of continuous notification of buffering status 
//------------------external  values-----------------

extern int cmdptr;  //actual number of chars in command strings buffer
extern int rc_cnt; //onion counter (for test only) (from tcp.c)
extern char sound_loop; //sound self-test mode flag (from ctrls.c)
extern char crp_state; //crypto state (from crypto.c)
//==========================Codec's states==============================
//------------------------------SPEEX-------------------------------
//encoder
SpeexBits spx_bits; 
void* spx_enc_state;
SpeexBits spx_bits_r;
void* spx_enc_state_r;
//decoder
SpeexBits spx_dbits; 
void* spx_dec_state;
SpeexBits spx_dbits_r;
void* spx_dec_state_r;
//globals
int spx_enh=1;
int spx_frame_size=160;
int spx_frames_per_packet=1;
int spx_bitrate=5950;
int spx_bitrate_r=2150; //bits per second for redundant frames
int spx_vbr=1;
static float spx_vbr_quality = 5;        //Speex vbr quality setting
int speex_redundant=1;
int spx_denoise=1;
int spx_agc=1;
//preprocessor
SpeexPreprocessState *spx_preprocess = 0;
SpeexPreprocessState *spx_preprocess1 = 0;
int Pp_frame_size=0;

//resampler
SpeexResamplerState *resampler = 0;
int Rs_out_rate=0;
int resampler_quality=3; 

///*
//-----------------------------------------GSM_HR-------------------------
static struct gsmhr *gsmhd=0; //half-rate coder state
//-----------------------------------------GSM_FR-------------------------
static gsm gsmha; //encoders state
static gsm gsmhs; //decoders state 
//*/
//------------------------------------------LPC---------------------------
static lpcstate_t lpcc; //codec state
static lpcstate_t lpcv; //vocoder state
//------------------------------------------LPC10-------------------------
static struct lpc10_encoder_state* lpc10_enc_state;
static struct lpc10_decoder_state* lpc10_dec_state;
///*
//------------------------------------------ILBC-------------------------
iLBC_Enc_Inst_t *Enc_Inst;
iLBC_Dec_Inst_t *Dec_Inst;
int ilbc_frameLen;
//*/
//------------------------------------------AMR---------------------------
int *amrenstate=0;
int *amrdestate=0;
char amrmode=0; //4750bps
char amrcbr=0; //vbr active
char amrfpp=10; //10 frames (200 mS) per packet
char amrsid=0; //last frame type
const char amr_block_size[8]={ 12, 13, 15, 17, 19, 20, 26, 31 }; //our format
 
///*
//------------------------------------------BV16--------------------------
struct BV16_Encoder_State estate;
struct BV16_Decoder_State dstate;
struct BV16_Bit_Stream bs;
//------------------------------------------CODEC2------------------------
struct CODEC2 *cd21;
struct CODEC2 *cd22;
//------------------------------------------OPUS--------------------------
OpusEncoder *enc=NULL;
OpusDecoder *dec=NULL;

//*/
//=================================Codec's services============================
//---------------------------------SPEEX---------------------------------
//speex initialization: quality (0-8), vbr(0/1), frames_per_packet(1-15)
//returns frame length in samples
int speex_i(int squal, int svbr, int prepr, int sfpp)
{
  const int brate[8]={2150, 3950, 5950, 8000, 11000, 15000, 18200, 24600};
 //set speex globals
  spx_vbr=svbr & 1;
  spx_vbr_quality=squal & 7;
  spx_bitrate=brate[(int)spx_vbr_quality];
  spx_frames_per_packet=sfpp & 0x0F; 
  //Encoder init 
  speex_bits_init(&spx_bits);
  spx_enc_state  = speex_encoder_init(&speex_nb_mode); 
  speex_encoder_ctl(spx_enc_state,SPEEX_GET_FRAME_SIZE,&spx_frame_size);
  speex_encoder_ctl(spx_enc_state , SPEEX_SET_VBR, &spx_vbr);
  if(spx_vbr) speex_encoder_ctl(spx_enc_state , SPEEX_SET_VBR_QUALITY, &spx_vbr_quality);
  else speex_encoder_ctl(spx_enc_state, SPEEX_SET_BITRATE, &spx_bitrate);
//Redundant encoder init
  speex_bits_init(&spx_bits_r);
  spx_enc_state_r  = speex_encoder_init(&speex_nb_mode);
  speex_encoder_ctl(spx_enc_state_r, SPEEX_SET_BITRATE, &spx_bitrate_r); 
  //Preprocess init
  if ((spx_denoise || spx_agc) && prepr)   {
    spx_preprocess = speex_preprocess_state_init(spx_frame_size, 8000);
    
    speex_preprocess_ctl(spx_preprocess, SPEEX_PREPROCESS_SET_DENOISE, 
			 &spx_denoise);
    speex_preprocess_ctl(spx_preprocess, SPEEX_PREPROCESS_SET_AGC, 
			 &spx_agc);  
   }                                                                    
//Decoder init
  speex_bits_init(&spx_dbits);
  spx_dec_state = speex_decoder_init(&speex_nb_mode);
  //speex_decoder_ctl(spx_dec_state, SPEEX_SET_ENH, &spx_enh);
//Decoder redundant init
  speex_bits_init(&spx_dbits_r);
  spx_dec_state_r = speex_decoder_init(&speex_nb_mode);

 return spx_frame_size;
}

//*****************************************************************************
//Speex encode frames (160 short samples each) to buf
//return buf length in bytes
int speex_e(unsigned char* buf, short* speech)
{
 int i;
  // encode the frames 
    speex_bits_reset(&spx_bits);   
    
    for (i=0;i<spx_frames_per_packet;i++)
    {
     if(spx_preprocess) speex_preprocess_run(spx_preprocess, speech+i*spx_frame_size);
     speex_encode_int(spx_enc_state, speech+i*spx_frame_size, &spx_bits);
    }
    
    i=speex_bits_write(&spx_bits, (char*)buf, MAX_PKT_LEN);  
    return i;
}

//*****************************************************************************
//encode redundant frames (160 short samples each) to buf using minimal cbr
//return buf length in bytes
int speex_er(unsigned char* buf, short* speech)
{
 int i;
  // encode the frames 
    speex_bits_reset(&spx_bits_r);   
    
    for (i=0;i<spx_frames_per_packet;i++)
    {
     speex_encode_int(spx_enc_state_r, speech+i*spx_frame_size, &spx_bits_r);
    }
    
    i=speex_bits_write(&spx_bits_r, (char*)buf, MAX_RDD_LEN);  
    return i;
}

//*****************************************************************************
//decode len bytes in buf to frames
//return total number of short samples
int speex_d(short* speech, unsigned char* buf, int len)
{
    int i;
   speex_bits_read_from(&spx_dbits, (char*)buf, len); 
   for (i=0;i<spx_frames_per_packet;i++)
   { 
    speex_decode_int(spx_dec_state, &spx_dbits, speech+i*spx_frame_size);
   }
   return (spx_frames_per_packet*spx_frame_size); 
}

//*****************************************************************************
//decode len bytes in buf to frames redundant data
//return total number of short samples
int speex_dr(short* speech, unsigned char* buf, int len)
{
    int i;
   speex_bits_read_from(&spx_dbits_r, (char*)buf, len); 
   for (i=0;i<spx_frames_per_packet;i++)
   { 
    speex_decode_int(spx_dec_state_r, &spx_dbits_r, speech+i*spx_frame_size);
   }
   return (spx_frames_per_packet*spx_frame_size); 
}

//*****************************************************************************
//Preprocessor initilization
void speex_p(int denoise, int agc)
{
 //set globals for external and internal preprocesses
 spx_denoise=denoise&1;
 spx_agc=agc&1;
 Pp_frame_size=0; //external preprocess will be inited on next reques
 Vad_cnt=0; //Speech now
 //Internal Preprocess init (for speex codec only)
  if (spx_preprocess)   {
    
    speex_preprocess_ctl(spx_preprocess, SPEEX_PREPROCESS_SET_DENOISE, 
			 &spx_denoise);
    speex_preprocess_ctl(spx_preprocess, SPEEX_PREPROCESS_SET_AGC, 
			 &spx_agc);  
   } 
}

//*****************************************************************************
//Preprocess len samples for other codecs
int speex_n(short* speech, int len)
{
 int i,j;

 //reinitialisation  
 if(len!=Pp_frame_size)
   {
    //Preprocess init
    
     if(!spx_preprocess1) spx_preprocess1 = speex_preprocess_state_init(len, 8000);
    
     speex_preprocess_ctl(spx_preprocess1, SPEEX_PREPROCESS_SET_DENOISE, 
			 &spx_denoise);
     speex_preprocess_ctl(spx_preprocess1, SPEEX_PREPROCESS_SET_AGC, 
			 &spx_agc);
     Pp_frame_size=len;      
   } 
   //preprocess samles
   if(spx_preprocess1) speex_preprocess_run(spx_preprocess1, speech);
   //get current speech probability (VAD)
   speex_preprocess_ctl(spx_preprocess1, SPEEX_PREPROCESS_GET_PROB, &i);
   if(Vad_cnt) //trashhold for go from silence to voice 
   speex_preprocess_ctl(spx_preprocess1, SPEEX_PREPROCESS_GET_PROB_START, &j);
   else  //trashhold for stay in the voice state
   speex_preprocess_ctl(spx_preprocess1, SPEEX_PREPROCESS_GET_PROB_CONTINUE, &j);
   if(i>=j) Vad_cnt=0; //Speech now
   else Vad_cnt++; //silency tail
  
   return Vad_cnt;
}

//*****************************************************************************
//resample intlen samples from spin to spout with new outrate
//return number of samples in spout
int speex_r(short* spin, short* spout, int inlen,  int outrate)
{
 int res=0;
 //initialisation
 if(!resampler) 
 { 
  resampler=speex_resampler_init(1, 8000, outrate, resampler_quality, &res);
  Rs_out_rate=outrate;
 }
 //reinitialisation
 if(outrate!=Rs_out_rate) speex_resampler_set_rate(resampler, 8000, outrate); 
 //processing
 res=MAX_RESAMPL_BUF;
 if(resampler) speex_resampler_process_int(resampler, 0, spin, (uint32_t*)&inlen, spout, (uint32_t*)&res);
 else res=1;
 return res;
}

//*****************************************************************************
//finalize Speex
void speex_f(void) {
 speex_bits_destroy(&spx_bits);
 speex_bits_destroy(&spx_dbits);
 if(spx_enc_state) speex_encoder_destroy(spx_enc_state);
 if(spx_dec_state) speex_decoder_destroy(spx_dec_state);
 if(spx_preprocess) speex_preprocess_state_destroy(spx_preprocess);
 if(spx_preprocess1) speex_preprocess_state_destroy(spx_preprocess1);
 if(resampler) speex_resampler_destroy(resampler);
 spx_preprocess=0;
 spx_preprocess1=0;
 resampler=0;
}

//---------------------------------AMR------------------------------------
//AMR initialization
void amr_ini(int dtx)
{
  amrenstate = Encoder_Interface_init(dtx);
  amrdestate = Decoder_Interface_init();
}
//*****************************************************************************
//AMR finalization
void amr_fin(void)
{
 if(amrenstate) Encoder_Interface_exit(amrenstate);
 if(amrdestate) Decoder_Interface_exit(amrdestate);
}


///*
//--------------------------------GSM_FR-----------------------------
//initialisation GSM_FR codec
void gsm_ini(void)
{ 
 gsmha = gsm_create();
 gsmhs = gsm_create();
}
//*****************************************************************************
//finalization GSM_FR codec
void gsm_fin(void)
{
 gsm_destroy(gsmha);
 gsm_destroy(gsmhs);
}
//*****************************************************************************
//gsm_fr encode 160 samples sp (20 mS) to 33 bytes bf 
void gsm_a(short *sp, unsigned char* bf)
{
 gsm_encode(gsmha, (gsm_signal*)sp, (gsm_byte *)bf);
}
//*****************************************************************************
//gsm_fr decode 33 bytes bf to 160 samples sp (20 mS) 
void gsm_s(unsigned char *bf, short *sp)
{
 gsm_decode(gsmhs, (gsm_byte *)bf, (gsm_signal*)sp);
}
//*****************************************************************************
//---------------------------------GSM_HR-------------------------
//initialisation GSM_HR codec
void gsmhr_ini(short isDTX)
{ 
 gsmhd = gsmhr_init(isDTX);
}
 //*****************************************************************************
//finalization GSM_HR codec
void gsmhr_fin(void)
{
 gsmhr_exit(gsmhd);
}
//*****************************************************************************
//gsm half-rate encode 160 samples sp (20 mS) to 14 bytes bf 
int gsmhr_a(short *sp, unsigned char* bf)
{
 return gsmhr_encode(gsmhd, bf, sp);
}
//*****************************************************************************
//gsm half-rate decode 14 bytes bf to 160 samples sp (20 mS) 
void gsmhr_s(unsigned char *bf, short *sp)
{
 gsmhr_decode(gsmhd, sp, bf);
}
//*****************************************************************************
//-------------------------------OPUS-------------------------------
//create OPUS codec
int opus_i(void)
{
 int err;
 //setup encoder: voice narrowband, vbr=6000, no DTX
 if(enc==NULL) enc = opus_encoder_create(8000, 1, OPUS_APPLICATION_VOIP, &err);
 else err=opus_encoder_init(enc, 8000, 1, OPUS_APPLICATION_VOIP);
 if (err != OPUS_OK) 
 {
  fprintf(stderr, "Cannot init OPUS encoder\n");
  return 1;
 }
 opus_encoder_ctl(enc, OPUS_SET_BITRATE(6000));
 opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_AUTO));
 opus_encoder_ctl(enc, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
 opus_encoder_ctl(enc, OPUS_SET_VBR(1));
 opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(1));
 opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
 opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
 opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(1));
 opus_encoder_ctl(enc, OPUS_SET_DTX(0));
 opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));
 opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
 //opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
 opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));

 //setup decoder	
 if(dec==NULL) dec = opus_decoder_create(8000, 1, &err);
 else err=opus_decoder_init(dec, 8000, 1);
 if (err != OPUS_OK) 
 {
  fprintf(stderr, "Cannot init OPUS decoder\n");
  return 1;
 }
 return 0;
}
//*****************************************************************************
//opus encode 480 samples (60mS) to 25-61 bytes
int opus_e(unsigned char* buf, short* speech)
{	
 return (opus_encode(enc, speech, 480, buf, 60));
}
//*****************************************************************************
//opus decode len bytes to 480 samples
int opus_d(short* speech, unsigned char* buf, int len)
{
 return (opus_decode(dec, buf, len, speech, 960, 0));
}

// LPC10
// Init LPC10
void lpc10_i(void)
{
	lpc10_enc_state = create_lpc10_encoder_state();
	lpc10_dec_state = create_lpc10_decoder_state();
	init_lpc10_encoder_state(lpc10_enc_state);
	init_lpc10_decoder_state(lpc10_dec_state);
}

// Encode LPC10
void lpc10_e(short* in, unsigned char* out)
{
	float speech[LPC10_SAMPLES_PER_FRAME];
	int32_t bits[LPC10_BITS_IN_COMPRESSED_FRAME];
	unsigned int i;

	for (i = 0; i < LPC10_SAMPLES_PER_FRAME; i++)
		speech[i] = (float)in[i] / 32768.0;
	lpc10_encode(speech, bits, lpc10_enc_state);
	lpc10_build_bits(out, bits);

	return;
}

// Decode LPC10
void lpc10_d(unsigned char* in, short* out)
{
	int32_t bits[LPC10_BITS_IN_COMPRESSED_FRAME];
	float speech[LPC10_SAMPLES_PER_FRAME];
	unsigned int i;

	lpc10_extract_bits(bits, in);
	lpc10_decode(bits, speech, lpc10_dec_state);
	for (i = 0; i < LPC10_SAMPLES_PER_FRAME; i++)
		out[i] = (short)(speech[i] * 32768.0);

	return;
}

//*****************************************************************************
// */
//--------------------------------------LPC-------------------------
// Initialise LPC codec
void lpc_i(void)
{
 if (!lpc_start()) 
 {
  fprintf(stderr, "Cannot allocate LPC decoding memory.\n");
  return;
 }
 lpc_init(&lpcc);
 lpc_init(&lpcv);
}
//*****************************************************************************
//LPC compress 160 samples to 14 bytes 
void lpc_e(unsigned char* buf, short* speech)
{
        lpcparams_t lp;   
	lpc_analyze(speech, &lp);
        memcpy((char*)buf, &lp, LPCRECSIZE);
}
//*****************************************************************************
//LPC decompress 14 bytes to 160 samples 
void lpc_d(short* speech, unsigned char* buf)
{
 lpc_synthesize(speech, (lpcparams_t *) buf, &lpcc);
}
//*****************************************************************************
//LPC-based vocoder
void lpc_v(short* speech, unsigned char mode)
{
 if(mode)
 {
  lpcparams_t lp;

  lpc_analyze(speech, &lp); //analyse speech
  //change pitch descriptor
  if(mode==3) lp.period+=20; //pitch up
  else if(mode==2) lp.period-=20; //pitch down
  else if(mode==1) lp.period=0; //unvoiced
  else lp.period=mode; //robot
  lpc_synthesize(speech, &lp, &lpcv); //synt new speech
 }
}
//*****************************************************************************
//Finalize LPC 
void lpc_f(void)
{
 lpc_end();
}
//*****************************************************************************
///*
//-----------------------------------ILBC--------------------------
//Internet Low Bitrate Codec initialisation
void ilbc_i(int mode)
{
  //Creates 
  WebRtcIlbcfix_EncoderCreate(&Enc_Inst);
  WebRtcIlbcfix_DecoderCreate(&Dec_Inst);
  if(mode) ilbc_frameLen=240; else ilbc_frameLen=160;
  // Initialization: 20 or 30 ms for 8000 sample rate
  WebRtcIlbcfix_EncoderInit(Enc_Inst, ilbc_frameLen/8); 
  WebRtcIlbcfix_DecoderInit(Dec_Inst, ilbc_frameLen/8);
}
//*****************************************************************************
//ILBC finalisation
void ilbc_f(void)
{
  //Free  
  WebRtcIlbcfix_EncoderFree(Enc_Inst);
  WebRtcIlbcfix_DecoderFree(Dec_Inst);
}
//*****************************************************************************
//ILBC encode 240 samples (30 mS) to 50 bytes
int ilbc_e(unsigned char* buf, short* speech)
{
  return WebRtcIlbcfix_Encode(Enc_Inst, (int16_t*)speech, (int16_t)ilbc_frameLen, (int16_t*)buf);
}
//*****************************************************************************
//ILBC decode 50 bytes to 240 samples
int ilbc_d(short* speech, unsigned char* buf)
{
 int16_t speechType;
 WebRtcIlbcfix_Decode(Dec_Inst, (int16_t*)buf, ilbc_frameLen==240 ? 50 : 38,  (int16_t*)speech,&speechType);
 return (int) speechType;
}
//*****************************************************************************
//-----------------------------------BV16-------------------------
//BroadVoice 16000bps (BV16) codec initialization
void bv16_i(void)
{
 Reset_BV16_Encoder(&estate);
 Reset_BV16_Decoder(&dstate);
}
//*****************************************************************************
//BroadVoice encode 40 short samples (5 mS) to 10 bytes
void bv16_e(unsigned char* buf, short* speech)
{
 BV16_Encode(&bs,&estate, speech);
 BV16_BitPack( buf, &bs );
}
//*****************************************************************************
//BroadVoice decode 10 bytes to 40 short samples (5 mS)
void bv16_d(short* speech, unsigned char* buf)
{
 BV16_BitUnPack ( buf, &bs );
 BV16_Decode(&bs,&dstate, speech);  
}
//*****************************************************************************
//*/
//==================================================================================
//                           Codec wrapers procedures
//==================================================================================

 //determine codec type in voice packet
int codec_type(unsigned char* bf)
{
 //check first byte for general codec type: bit7=1 for CBR and bit7=0 for VBR
 if(bf[0]>=192) return(CODEC_MELPE);	//MELPE: 11 + up to 6_extra_data_bits
 if(bf[0]>128) return(bf[0]&0x0F); //other CBR CODECS: use codec type from byte 0
 
//VBR CODECS: first byte is: bit7=0 and 7 bits is total data length,
// checks second byte for VBR codec type:
//SILK: 11 + 6_extra_data_bits
//OPUS: 10 + 6_bit_length_of_second_frame
//AMR: 01 + 3 bits (mode) + 3 bits (fpp-3)
//SPEEX: 00 + 00 + 4_extra_data_bits
 if(bf[1]&0x80) 
 {
  if(bf[1]&0x40) return(CODEC_SILK); 
  return(CODEC_OPUS);
 }
 if(bf[1]&0x40) return(CODEC_AMRV);
 return(CODEC_SPEEX);
}

//*****************************************************************************
//returns speech length in frames per one internet packet for specified codec
int codec_len(int cdk)
{
 if(cdk==CODEC_AMRV) return 160*amrfpp;
 else return frm_len[cdk]*frm_ppk[cdk];
}


//*****************************************************************************
//encode speech to compleet internet packet
//using global encoder settings
//returns buffer length in bytes including first type/len byte
int sp_encode(short* sp, unsigned char* bf)
{
 int i, l, fpp;
 unsigned char dtxcnt=0;
 unsigned char* bp=bf+1; //pointer to encodec data ares
 
 bf[0]=0x80|((unsigned char)enc_type&0x0F); //set codec type for cbr
 
 if(enc_type==CODEC_AMRV)
 {
  fpp=amrfpp; //sets frame per packets for amr_vbr
  l=amr_block_size[amrmode]; //set encode block size
  bp[0] = 0x40 | (amrmode<<3) | (amrfpp-3); //first packets byte: flag, mode, fpp
  bp++; //amr data from next byte 
  bf[0]=1; //initialize length byte
 }
 else
 {
  fpp=frm_ppk[enc_type]; //frames per packet for other cbr codecs
  l=buf_len[enc_type]; //encoded frames fixed length for cbr or 0 for vbr
 }
 
//process frames
 for(i=0; i<fpp; i++) 
 {
  //encode one frame
  switch(enc_type)
  {
   //adaptive
   case CODEC_AMRV:
	{
	 l=AMR_encode( amrenstate, amrmode, sp, bp, amrcbr );
	 if(l==5) //SID block
	 {
	  if(dtxcnt) bp-=5; //skip this block for SID_update
	  bp[0]&=0xE0;
	  bp[0]|=dtxcnt+1; //put dtx counter	  
	  dtxcnt+=2; //count SID
	 }
	 else  dtxcnt=0; //this is peech block: reset SID counter
	 bf[0]+=l;
	}
	break;


   //constant bitrate
   case CODEC_MELPE: 
	{
	 melpe_a(bp, sp); //result is 81 bits in 11 bytes
	 bf[0]<<=1;        //roll last bits of previous frames
	 bf[0]|=bp[l];    //pack last bit to type byte
	 bf[0]|=0xC0;	  //set melpe type flag  
	}
   	break;
   
  ///*
   case CODEC_CODEC21: codec2_encode(cd21, bp, sp);
	break;
   case CODEC_CODEC22: codec2_encode(cd22, bp, sp);
	break;
   case CODEC_MELP: melp_enc(bp, sp);
	break;
   case CODEC_LPC10: lpc10_e(sp, bp);
	break;	
   case CODEC_CELP: celp_encode(sp, (char*)bp);
	break;
   //*/
   case CODEC_LPC: lpc_e(bp, sp);
	break;
   ///*
   case CODEC_GSMH: gsmhr_a(sp, bp);
	break;
   case CODEC_G723: g723_e(sp, bp);
	break;
   case CODEC_G729: g729enc(sp, bp);
	break;
   //*/
   case CODEC_GSME: gsmer_encode(bp, sp);        
	break;
   ///*
   case CODEC_GSM: gsm_a(sp, bp);
	break;
   case CODEC_ILBC: ilbc_e(bp, sp);
	break;
   case CODEC_BV16: bv16_e(bp, sp);
	break;
   //variable bitrate: first packet bytes contains 7bit packet length and bit7=0
   //second byte contain 7(SPEEX) or 6(SILK) data bits or 6bits of second frame
   //length (OPUS) and type flags in bit7-6: 10-OPUS, 11-SILK, 0-SPEEX  
   case CODEC_OPUS: //2 frames: each 480 samples (60mS) to 22-61 bytes  
	{ 
         unsigned char t=bp[0]; //temporary store last byte of previous frame
 	 l=opus_e(bp, sp);  //returnnes data length incuding first TOC byte
	 bp[0]=t; //restore last byte of previous frame replacing TOC 
	 bf[1]=(unsigned char)l&0x3F; //current frame length (for last frame on exit)
	 bf[0]+=(bf[1]-1); //total length
	 bf[1]|=0x80;  //set vbr flag (first packet bytes contains 7bit packet length)
	 bf[0]&=0x7F;  //set OPUS type flag 10
	 l--; //skip TOC
	}
	break;
   case CODEC_SILK: //single frame 480 samples (60mS) to 42-96 bytes
	{
	 l=SILK8_encode (sp, bp); //once
	 bp[0]|=0xC0; //set SILK type flag 11
	 bf[0]=l&0x7F; //frame length
	}
	break;

    //*/
   case CODEC_SPEEX: //single frame 320 samples (40mS) to 12-76 bytes 
	{ 
     memcpy(bp, speex_rb, 11); //add last redundant bytes to packet
     l=11+speex_e(bp+11, sp); //process current speech 320 frames
	 speex_er((unsigned char*)speex_rb, sp); //process it also to redundant buffer for next
	 bf[0]=l&0x7F; //frame length
	 bp[0]&=0x7F; //set SPEEX type flag 0
	}
   	break;



  } //switch(cd)
  
  bp+=l; //add output data length to data pointer
  sp+=frm_len[enc_type]; //add input frame length to speach pointer
 } //for(i
 
 return (int)(bp-bf); //return total packet length in bytes
} 


//*****************************************************************************
//decode buffer to speech: determine codec from packet but use global
//settings for amr mode 
//adjust rate of outputting using global crate setting
//returns number of speech samples after rate adjusting
int sp_decode(short* sp, unsigned char* bf)
{
 int i, l, m, fpp;
 char amrmd=0; //default mode for amc codec
 unsigned char* bp=bf+1; //pointer to codecs block (skip header byte)
 short* spr=sp; //pointer to ouputted speech block
 int cd=0; //codec type

 //detect codec type:
 cd=codec_type(bf);
 //set decoder parameters
 if(cd==CODEC_AMRV)
 {
  amrmd = 0x07&(bf[1]>>3); //amr mode for packet
  l=amr_block_size[amrmd]; ////encoded frames length for this mode 
  fpp = 3 + (0x07&bf[1]); //amr frames in packet
  bp++;
 }
 else
 {
  l=buf_len[cd]; //encoded frames fixed length for cbr or 0 for vbr
  fpp=frm_ppk[cd]; //presetted frames_per_packet for this codec
 }

 //decode data frames to speech
 for(i=0; i<fpp; i++)
 {
  switch(cd) //decode one frame
  {
 //adaptive codec with DTX:
   case CODEC_AMRV:
	{
	 AMR_decode(amrdestate, amrmd, bp, spp, 0); //bfi=0
	 if(1&bp[0]) //this is SID frame
	 {
	  if((0x1F&bp[0])>1) //this is not last SID
	  {
	   l=0; //not go to the next frame, replay current frame once more
	   bp[0]-=2; //and decrement SID counter
	  }
	  else l=5; //this was last SID, go to next frame
	 }
         else l=amr_block_size[amrmd]; //this is speech frame
	}
	break; 

 //constant bitrate codecs:
   case CODEC_MELPE: 
	{
	 unsigned char t=bp[10]; //store first byte of next frame
	 bp[10]=(bf[0]>>5)&1; //extract last bit of frame, place to last byte as bit 0
         melpe_s(spp, bp); //decode 81bits in 11 bytes
	 bp[10]=t; //restore first byte of next frame
	 bf[0]<<=1; //roll frames last bits coneiner (byte 0 of packet)
	}
	break;
  ///*
   case CODEC_CODEC21: codec2_decode(cd21, spp, bp);
	break;
   case CODEC_CODEC22: codec2_decode(cd22, spp, bp);
	break;
   case CODEC_MELP: melp_dec(spp, bp);
	break;
   case CODEC_LPC10: lpc10_d(bp, spp);
	break;	
   case CODEC_CELP: celp_decode((char*)bp, spp);
	break;
  //*/
   case CODEC_LPC: lpc_d(spp, bp);
	break;
   ///*
   case CODEC_GSMH: gsmhr_s(bp, spp);
	break;
   case CODEC_G723: g723_d(bp, spp);
	break;
   case CODEC_G729: g729dec(bp, spp);
	break;
   //*/
   case CODEC_GSME: gsmer_decode(spp, bp);
	break;
   ///*
   case CODEC_GSM: gsm_s(bp, spp);
	break;
   case CODEC_ILBC: ilbc_d(spp, bp);
	break;
   case CODEC_BV16: bv16_d(spp, bp);
	break;
   //variable bitrate codecs:
   case CODEC_OPUS: //total length byte+second length byte+2 frames
	{
	 l=bf[0]-(bp[0]&0x3F)+2; //current frame length = total - second
         bp[0]=24; //restore TOC for vbr narrowband 60mS mode
         opus_d(spp, bp, l);  //variable length
         bp--; //reserve byte for next TOC/frame length
	 bp[l]=l; //place first frame length for calculation of second length
	}
	break;
   case CODEC_SILK: //one block for 60 mS speech
	{
	 l=bf[0]; //block length
	 bp[0]&=0x3F; //mask SILK flag
 	 SILK8_decode (spp, bp, l); //decode whole block
	}
  	break;       //SPEEX: one block for 40 ms redundant + one block for 40 ms speech

   //*/

   case CODEC_SPEEX: //while pkt lossed set data[1]|=0x20; and do for play redundant
	{	     //then call procedure once more for play current speech
	 l=bf[0]; //total length: redundant + regular
     if(l==1) //PTT off signal from remote
     {
       crate=8100;
       est_jit=codec_len(dec_type)+2*chunk;
       if(!vad_level) return 0;
       for(i=0;i<640;i++) spr[i]=2*vad_level*sin(0.75*(double)i);
       for(i=640;i<1280;i++) spr[i]=3*vad_level*sin(1.177*(double)i);
       for(i=1280;i<1920;i++) spr[i]=4*vad_level*sin(1.57*(double)i);
       return 1920;
     }
     if(redundant) //play redundant data if packetloss occured
     {
      speex_dr(spr, bp, 11); //decode redundant data from packet
      spr+=320; //move output pointer
      redundant=0; //clear packetloss flag
     }
     speex_d(spp, bp+11, l-11); //process actual speech
	}
  }  
  bp+=l; //add data length (pointer to next data frame)
  //if(speex_rs)
  m=speex_r(spp, spr, frm_len[cd], crate); //rate adjusting: return new speech length
  //else m=RateChange(spp, spr, frm_len[cd], crate);
  spr+=m; //add speech length (pointer to next speech frame for output)
 }
 return spr-sp; //returns total number of outputted speech samples
}
//*****************************************************************************


//-----------------------Jitter buffer----------------------------------

//play samples from jitter buffer
//this procedure called periodically every 20 mS after 160 samples
//was grabbed
int playjit(void)
{
  int i=0;
  int job=0;
 //periodically check for PTT deactivation after TAB_KEY released
   if(ptt_flag) //if key was pushed and release timestamp was fixed in ptt_flag
   {
    if(getmsec()>ptt_flag) //compare it with now, if time to release
    {
     ptt_flag=0; //clear fixed timestamp
     etx_flag=0; //clear estimated TX flag
    }
   } 
  
 //get number of unplayed samples in alsa buffer
 if(rx_flg1<50) sdelay=getdelay();

 //Sound Underrun

 if(rx_flg && (sdelay<chunk))
 {  //if this is a first underrun after last incoming packet
  if(vad_signal && (!l_jit_buf))
  {   //if vad signal specified in config and jitter buffer is empty
   int j=1;
   j=(j<<(vad_signal+5))-1; //noise signal level
   if(vad_signal==1) memset(jit_buf, 0, 3200); //silency for signal=1
   else for(i=0;i<1600;i++) jit_buf[i]=(rand()%j); //noise signal
   i=soundplay(1600, (unsigned char*)(jit_buf)); //play and returns number of played samples
   if(i<chunk) soundplay(1600, (unsigned char*)(jit_buf)); //play again if underrun was occured
  }
  rx_flg=0; //clear rx_flag: no plagged incoming users now
  rx_flg1=0; //clear rx_flg1: no PTT off remote signal
  job=0x80;
 }

 if(sdelay<chunk) rx_flg1++; //count underruns
 if(rx_flg && rx_flg1) //if there is the first underrun and to many underruns was early
 { //this is the first packet after PTT pause
   if(rx_flg1>20) //prevent play tri-tone after spontane underruns during continuous transmission
   {
    i=est_jit; //number of samples of silency will be played
    if(i>JIT_BUF_LEN/2) i=JIT_BUF_LEN/2; //restriction buffer size
    memset(jit_buf1, 0, 2*i); //put silency to alternative buffer
    i=soundplay(i, (unsigned char*)(jit_buf1)); //play and returns number of played samples
    if(i<chunk) soundplay(min_jitter, (unsigned char*)(jit_buf1)); //play again if underrun was occured
    job+=0x100;
   }
   rx_flg1=0; //clear rx_flag: no plagged incoming users now   
 }

 //play samples in jitter buffer
 if(l_jit_buf)
 {
  i=soundplay(l_jit_buf, (unsigned char*)(p_jit_buf)); //returns number of played samples
  if(i) job+=0x200;
  if(i<=0) i=0; //must play againbif underrun (PTT mode etc.)
  l_jit_buf-=i; //number of unplayed samples
  p_jit_buf+=i; //pointer to unplayed samples
  if(l_jit_buf<=0) //all samples played
  {
   l_jit_buf=0; //correction
   p_jit_buf=jit_buf; //pointer to start of buffer 
  }  
 }  
 return job; //job flag
}
//*****************************************************************************
//-----------------------Alternative VOX control---------------------------------
//Voice active detection by samples level
int vox(short* speech, int len, char level, char hist)
{
 int i;
 short s, s0, s1;

 s1=level+(hist>>1); //from silency to voice
 if(s1>100) s1=100;
 s1<<=5;	//0-32000
 s0=level-(hist>>1); //during voice
 if(s0<0) s0=0;
 s0<<=5; //0-32000

 Vad_cnt++; //count voice tail
 for(i=0;i<len; i++)
 {
  s=speech[i];
  if(s<0) s=-s; //module of samle
  if(Vad_cnt) //silency now
  {
   if(s>s1) Vad_cnt=0; //to voice
  }
  else if(s>s0) Vad_cnt=0; //voice now
 }
 return Vad_cnt;
}

//*****************************************************************************


//----------------Alternative resampler--------------------------------------------

//Change samples rate (from PGPFone)
int
RateChange(short *src, short *dest, int srcLen, int destRate)
{
        int srcRate = 8000;
        short *sourceShortPtr;
	short *destShortPtr;
	float nSrcRate, nDestRate;
	nSrcRate = srcRate;
	nDestRate = destRate;

	// All samples are 16 bit signed values. Pass in the number of source samples,
	// the sample rate of the source data, and the required destination sample
	// rate.
	if(!srcLen)
		return 0;
	// If the sample rates are identical, just copy the data over
	if(srcRate == destRate)
	{
		memcpy((char*)dest, (char*)src, srcLen*2);
		return srcLen;
	}
	sourceShortPtr = src;
	destShortPtr = dest;
	
	// Downsample
	if(srcRate > destRate)
	{
		float destStep = nDestRate / nSrcRate;
		float position = 0;
		
		while(srcLen)
		{
            int destSample = 0;
            int count = 0;
			
			// Accumulate source samples, until the fractional destination position
			// crosses a boundary (or we run out of source data)
			while(srcLen && (position < 1.0))
			{
				destSample += *sourceShortPtr++;
				srcLen--;
				position += destStep;
				count++;
			}
			position = position - 1.0;
			*destShortPtr++ = (destSample/count);
		}
	}
	else // Upsample
	{
		float sourceStep = nSrcRate / nDestRate;
		float position = 0;

		while(--srcLen)
		{
            int leftSample = *sourceShortPtr++;
            int sampleDifference = *sourceShortPtr-leftSample;

			while(position < 1.0)
			{
				*destShortPtr++ = leftSample + ((float)sampleDifference * position);
				position += sourceStep;
			}
			position = position - 1.0;
		}
	}
	// Return the number of samples written
	// Note that this code will sometimes (often?) write one sample too many, so make
	// sure your destination buffer is oversized by one.
	return destShortPtr - dest;
}
//*****************************************************************************



//==================================================================================
//                           Top level procedures
//==================================================================================


//----------------------Setup----------------------------------

//group codecs initialisation
void sp_init(void)
{
 ///*
 cd21=codec2_create(CODEC2_MODE_1400);
 cd22=codec2_create(CODEC2_MODE_3200);
 //*/
 melpe_i();
 ///*
 celp_init(0);
 lpc10_i();
 melp_ini();
 gsm_ini(); 
 gsmhr_ini(0); //dtx 1/0
 opus_i();
 //*/
 speex_i(5, 1, 0, 2); //quality (1-10), vbr(1/0), preproc (1/0), frames_per_packet(1-15)

 lpc_i();
 ///*
 g729ini(1, 0); //rate: G729D(63)=0,G729(80)=1,G729E(118)=2, dtx: 0/1
 g723_i(0, 0);  //0-rate63, 1-rate53; dtx0/1
 ilbc_i(1); //0-20mS/15.2kbps, 1-30mS/13.3kbps
 bv16_i();
 //*/
 gsmer_init(1); //dtx0/1
 ///*
 SILK8_open (3); //frames_per_packet (1-5)
 //*/
 speex_p(1,1); //set denoise and agc

 amr_ini(1); //dtx0/1
}


//*****************************************************************************
//group codecs finalization
void sp_fine(void)
{
 ///*
 gsm_fin();
 gsmhr_fin();
 codec2_destroy(cd21);
 codec2_destroy(cd22);
 //*/
 speex_f();
 lpc_f();
 ///*
 ilbc_f();
 SILK8_close();
 //*/
 amr_fin();
}


//*****************************************************************************
//set encoder type
int set_encoder(int cd)
{
 if((cd>0)&&(cd<19))
 {
  enc_type=cd; //set internal encoder
  snd_need=codec_len(enc_type); //samples needed for compleet packet
 }
 web_printf("\r\nCoder=%s\r\n",cd_name[enc_type]); //notify encoder name
 return enc_type;
}


//*****************************************************************************
int get_decoder(int cd)
{
 if(!cd) cd=dec_type;
 web_printf("Last decoder=%s\r\n", cd_name[cd]);
 if(vad_t) web_printf("VAD detector enabled now\r\n");
 web_printf("Voice transmission ");
 if(!tx_flag) web_printf("disabled\r\n");
 else web_printf("enabled ");
 if(tx_flag==1) web_printf(" by VOX\r\n");
 else if(tx_flag==2) web_printf(" by PTT\r\n");
 else if(tx_flag==3) web_printf(" continuously\r\n");
 else printf("\r\n");
 return dec_type;
}


//*****************************************************************************
void get_jitter(void)
{
 web_printf("Average jitter is %d mS, buffer's latency is %d mS\r\n", actual_jitter/8, (sdelay+l_jit_buf+l_pkt_buf)/8);
}

//set amr_vbr mode for encoder
int amr_setup(char mode, char dtx, char fpp)
{
 amrmode=mode&7; //modes(0=4750bps)
 amrcbr=(!dtx); //force vad active (no dtx)-> condtant bitrate
 amrfpp=fpp; //frames per packet: total packet length must be less 127 bytes
 if(amrfpp<3) amrfpp=3;
 if(amrfpp>10) amrfpp=10; //restriction for fpp
 return 160*amrfpp; //speech samples needed for compleet packet 
}
//*****************************************************************************


//-------------------Controls------------------------------------

//switch TX/mute by pressing ENTER
void switch_tx(void)
{
  ptt_flag=0;
  vad_t=0; //deactivate vad
  if(etx_flag) etx_flag=0; else etx_flag=3; //switch estimated flag
}
//*****************************************************************************
void off_tx(void)
{
 ptt_flag=0;
 vad_t=0; //deactivate vad
 etx_flag=0; //deactivate voice
 tx_flag=0;
}
//*****************************************************************************
//PTT TX activation by push and hold TAB key
void push_ptt(void)
{
   if (ptt_flag) ptt_flag=32+getmsec(); //holding ptt
   else 
   {
    ptt_flag=512+getmsec(); //pressing ptt now
   }  
  vad_t=0; //deactivate vad
  etx_flag=2; //set estimated flag for PTT mode
}
//*****************************************************************************
//VAD detector activation by press SHIFT+TAB
void go_vad(void)
{
 etx_flag=0; //clear estimated flag
 vad_t=vad_tail; //activate vad
}
//*****************************************************************************
//Play ringtone if jitter buffer is empty
void playring(void)
{
 if(!l_jit_buf)
 {  //fill buffer for beep sygnal
  memset(jit_buf, 0, JIT_BUF_LEN); //silency
  memcpy(jit_buf, ringwave, 2*ringlen);
  l_jit_buf=JIT_BUF_LEN; //total samples
 }
}
//*****************************************************************************



//-----------------------Processing----------------------------------------

//Process and play incoming packet:
//The first try to play samples in samples buffer, then decode and
//play packets from packets buffer, then decode and play pkt
int go_snd(unsigned char* pkt)
{
 int i, delay, delta, q2, j=0;
 int job=0;
 job=playjit(); //the first: play samples in jitter buffer

 //if jitter buffer is empty and there is undecoded packet in packets buffer
 while((l_pkt[n_pkt])&&(!l_jit_buf))
 {
  l_jit_buf=sp_decode(jit_buf, pkt_buf[n_pkt]); //decode packet to jitter buffer
  p_jit_buf=jit_buf; //set popiter to start of buffer
  l_pkt_buf-=l_pkt[n_pkt]; //decrese average total nubber of samples in unplayed buffered packets
  if(l_pkt_buf<0) l_pkt_buf=0; //correction
  l_pkt[n_pkt]=0; //mark current packet as  a played
  n_pkt++; //to next position in packet buffer
  n_pkt&=(MAX_PKT-1); //roll
  playjit();  //play decoded samples from jitter buffer
  job=0x20;
 } 

//if new packet received
 if(pkt)
 {
  job+=0x40;
  //fixes user id of packets sender
  rx_flg=1;
  //find codec type in received packet
  q2=codec_type(pkt);
  //notify if decoder changed
  if((dec_type!=q2)&&(pkt[0]!=1))
  {
   web_printf("Decoder=%s\r\n", cd_name[q2]);
   fflush(stdout);
   dec_type=q2;
   chunk=getchunksize();
   //set max_jitter and min_jitter for current decoder
   max_jitter=((1+MAX_PKT)*(codec_len(dec_type))+(getbufsize()))/2;
   min_jitter=codec_len(dec_type)/2;
   if(min_jitter<(2*chunk)) min_jitter=2*(chunk);
   //min_jitter=2*(chunk);
   est_jit=min_jitter;
   last_delay=2*(chunk);
   up_level=0; 
   down_level=0;
  }
  
 //samples in udecoded packets, in jitter buffer and in alsa auidio buffer (total delay)

  delay=sdelay+l_pkt_buf+l_jit_buf;

/*
  if(!cmdptr) //!!!!!!!!!!!!!!debug only!!!!!!!!!!!!!!!!
  {
   fprintf(stderr, "\r                                                                \r%d-%d-%d-%d-%d-%d",
   l_pkt_buf, sdelay,  l_jit_buf, est_jit, crate, erate);
   fflush(stderr);
  }
*/
 //computes actual jitter 
  delta=last_delay-delay; //actual jitter (compared with last packets interval)
  last_delay=delay; //store current packets interval as last interval

  //average separately for positive and negative deviations
  if(delta<0) //current interpackets interval less then previous
  {
   down_level=(down_level-delta)/2;    //average negative deviation
   //if(down_level>max_jitter) down_level=max_jitter; //restriction
  }
  else //current interpackets interval greater then previous 
  {
   up_level=(up_level+delta)/2; //average positive deviation
   if(up_level>max_jitter) up_level=max_jitter; //restriction
  }  
  
  //use smallest of deviation values: evaluates one-way deviation
  if(up_level>down_level) actual_jitter=down_level; else actual_jitter=up_level; //q2 is clear averaje jitter
  //1/3 of diferencies beetwen jitter compensation buffer and it's minimal value
  //for actual codec type 
  delta=(est_jit-min_jitter)/3; //trashhold interval
  if(delta<0) delta=0;

  //adjust jitter buffer size
  if(actual_jitter<delta)  //average jitter too small: decreaze buffers size smoothly
  {
   //est_jit-=(q2/4); //decresing factor depends of actual differencies
   est_jit-=16;
  }
  else if(actual_jitter>2*delta) //averaje jitter too large: increeze buffers size smoothly
  {  //note: increasins of buffer size is faster then decreasing!
   //est_jit+=(q2/4); //increasing factor depends of actual differencies
   est_jit+=16;
  }    
  //now est_jit is optimal buffer filling for actual average jitter
  //use it for filling jitter buffer or use fixed value from config

  if(sp_jit>0) est_jit=sp_jit; //set fixed size of jitter buffer insteed auto adjusting
  if(est_jit>max_jitter) est_jit=max_jitter; //restriction
  if(est_jit<min_jitter) est_jit=min_jitter; //restriction

  //adjust rate for filling jitter buffer 
  if(delay>(est_jit+chunk/2)) //too long delay now: we must play fastly for decresing it
  {
   erate=DEFRATE-(delay-est_jit)/4; //estimated rate for fastly playing
   if(erate<crate) crate-=1; //smoothly fit rate to estimated rate 
  }
  else if(delay<(est_jit-chunk/2)) //too short delay now: we must play slowly for increasing it
  {
   erate=DEFRATE+(est_jit-delay)/4; //estimated rate for slowly playing
  }
  else 
  {
   erate=DEFRATE;
   crate=DEFRATE; //delay approximately matched: use nominal rate (not resampled)
  }

 if(sound_test) //jitter notification (engineering mode)
 {
  printf("erate=%d, crate=%d, delay=%d, est_j=%d, act_j=%d, rc_cnt=%d\r\n",
        erate, crate, delay, est_jit, actual_jitter, rc_cnt);
 }
  //adjust rate if no -1 value specified in config (for disabling)
  if(sp_jit>=0)
  {
   //smoothly fit rate to estimated rate for resampling before playing
   if(erate>crate) crate+=16;
   if(erate<crate) crate-=16;
   if(crate>9000) crate=9000;
   if(crate<7000) crate=7000; //restrict rate  
  }
  else crate=8000; //set nominal rate, buffer not used for jitter compensation

  //process incoming packet
  if(!l_jit_buf) //if jitter buffer is empty now
  {
  //Underrun prevention (for first incoming packet after remote MUTE)
   if(sdelay<chunk) //if less then one chunk in alsa buffer
   {  
    memset(jit_buf, 0, min_jitter*2); //put silency to jitter buffer
    i=soundplay(min_jitter, (unsigned char*)jit_buf); //play it
    if(i<chunk) i=soundplay(min_jitter, (unsigned char*)jit_buf); //if underrun play again
    sdelay=getdelay(); //renew number of samples in alsa buffer
    //crate=8100; //set rate a little more then nominal for start collecting samles in jitter buffer
    i=1; //set flag of first packet after inactivity
   }
   else i=0; //flag for regullar packet
  //decode incoming packet to jitter buffer 
   l_jit_buf=sp_decode(jit_buf, pkt); //decode new packet in jitter buffer
   if(i) //if first packet after inactivity
   {
    //clear some first samples for suppress playing tail from codec internal state
    //memset(jit_buf, 0, 480); //suppress first 30 mS of voice after activation
   }
   playjit(); //play samples in jitter buffer
  }
  else //jitter buffer is not empty now: add unplayed packet to packet bufer
  {
    //find empty position in packet buffer or overwrite most old packet
    for(i=0;i<MAX_PKT;i++) if(!l_pkt[(i+n_pkt)&(MAX_PKT-1)]) break; //search for empty
    j=(i+n_pkt)&(MAX_PKT-1); //pointer to packets position (roll 0-7)
    if(i==MAX_PKT) //no found empty slot, overwrite most old packet
    {
     l_pkt_buf-=l_pkt[j]; //substract length of old overwrited packet from total 
     if(l_pkt_buf<0) l_pkt_buf=0; //correct total
    }      
     //put undecoded packet to buffer    
    l_pkt[j]=codec_len(dec_type); //average number of samples in this packet
    l_pkt_buf+=l_pkt[j]; //add length to total number of buffered samples
 /*
    if(!cmdptr) //!!!!!!!!!!!!!!!debug only!!!!!!!!!!!!!!!!!
    {
     printf(" B=%02X%02X Q=%d L=%d J=%d\r\n", pkt[0], pkt[1], dec_type, l_pkt[j], j);
     fflush(stdout);
    }
 */
    //length of added udecoded packet in bytes
    if(0x80&pkt[0]) i=1+frm_ppk[dec_type]*buf_len[dec_type]; else i=(1+pkt[0])&0x7F; 
    memcpy(pkt_buf[j], pkt, i); //copy packet to buffer's slot
  }
 }
 return job;
}



//*****************************************************************************
//Grab and encode sound for outgoing packet:
//Poll and grab alsa, preprocess 160 sampless, collect it for compleet 
//packet, encode voice packet by current codec, returns packets length in bytes
int do_snd(unsigned char *pkt)
{
 int i;
 //check state for activation of audio input
 soundrec((crp_state>2)||(sound_loop));
 //grab sound input device up to RawBufSize samples
 l_raw+=soundgrab((char*)(raw_buf+l_raw), RawBufSize-l_raw); //l_raw = actually grabbed samples
 if(l_raw<RawBufSize) return 0; //grab for RawBufSize samples (one frame ready for preprocessing)
 //now we have exectly RawBufSize samples in raw buffer
 l_raw=0; //pass it to process
 //we use alsa input clock for periodically playing buffered samles
 //go_snd(0); //periodically (every 20 mS) try to play buffered samples
 //now we have frame ready for preprocessing, check vad or randomize sprng if no TX 
 if(tx_flag||etx_flag||vad_t) //preprocess if actual or estimated tx flag or VAD active
 {
  if((npp7)&&(enc_type!=CODEC_MELPE)) melpe_n(raw_buf);
  if(!vox_level) i=speex_n(raw_buf, RawBufSize); //preprocess frame, returns vad counter of previous inactive frames (reset to 0 if frame is active)
  else i=vox(raw_buf, RawBufSize, vox_level, vox_level/15); //alternative vox (by pcm level)
  if(i<vad_t)  etx_flag=TX_VAD; //set vad as active if current frame is active or if vad tail
 }
 else
 {
  //feed SPRNG
  randFeed((const uchar*)raw_buf, 2*RawBufSize);
 }
 //change mode and notify
 if((tx_flag!=etx_flag) && etx_flag) //if active mode was setted while current mode was inactive
 {
  //check for cmdptr is null
  if(!cmdptr) //no users console input now, we can print notify
  {
   if(etx_flag==TX_VAD) web_printf("\r     \rSPEAK "); //VAD activity detected in VOX mode
   else if(etx_flag==TX_PTT) web_printf("\r     \rPUSH "); //user is holding PTT button
   else web_printf("\r     \rTALK "); //continious transmittion started
   fflush(stdout);
  }
  tx_flag=etx_flag; //set currrent mode by estimated mode
  l_in=0; //no data in input buffer now, input started
 }
 
 //check TX mode and not process frame if mode is not active
 //note: we can activate TX there. deactivation can be only after compleet packet
 //processed. I.e. if only one of frames is active all packet will sended
 if(!tx_flag) //not transmitt this packet 
 {
  l_in=0; //clear length of data in input buffer (reject it)
  if(!tx_note) return -1; //if remote notification required (this is a first skipped packet)
  tx_note=0; //clear notify flag
  //prepare notify packet
  pkt[0]=1;
  pkt[1]=0;
  return 2;
 }

 //apply vocoder
 if(sp_voc) lpc_v(raw_buf, sp_voc);

 //add preprocessed frame to input buffer
 memcpy((char*)(in_buf+l_in), raw_buf, 2*RawBufSize); //copy preprocessing samples from raw to input buffer
 l_in+=RawBufSize; //total number of samples in input buffer now
 //check input buffer comleet for current encoder type
 if(l_in<snd_need) return -2; //more samles needed
 //input buffer ready to encode
 if(!etx_flag)  //check for tx mode was disabled by user and not enabled by vad
 { //only now we can disable transmission but current packet will be transmitted 
  if(tx_flag!=etx_flag) //transmission was active: inactivate it
  {
   //check for cmdptr is null
   if(!cmdptr)  //no users console input now, we can print notify
   {
    if(tx_flag==TX_VAD) web_printf("\r     \rQUIET "); //notify nactivation by vad
    else web_printf("\r     \rMUTE "); //notify inactivation by user
    fflush(stdout);
   }
   tx_flag=0; //inactivate actual tx mode for next packets
   tx_note=1; //set remote notification flag
  }
 }
 else if(etx_flag==TX_VAD) etx_flag=0; //clear etx_flag for next VAD detection in VAD mode
 i=sp_encode(in_buf, pkt); //encode voice to packet
 //if(i<2) i=-3; //edcoding error
 l_in-=snd_need;  //number residual (unencoded) samples in buffer(pass for next packet)
 if(l_in) memcpy(in_buf, (char*)(in_buf+snd_need), l_in<<1); //copy tail to start of buffer
 return i; //returns packet's length in bytes
}


//*****************************************************************************
//inites audio system
//returns 0 if OK else error code
void setaudio(void)
{
 char str[256];
 int i;

 strcpy(str, "VoiceCodec");
 if(parseconf(str)>0) i=atoi(str); else i=-1;
 if((i<=0)&&(i>18)) i=7;
 set_encoder(i);

 strcpy(str, "Vocoder");
 if(parseconf(str)>0) i=atoi(str); else i=-1;
 if((i>0)&&(i<256)) sp_voc=i; else sp_voc=0;

 strcpy(str, "DeNoise");
 if(parseconf(str)>0) sp_npp=atoi(str); else sp_npp=0;

 strcpy(str, "AutoGain");
 if(parseconf(str)>0) sp_agc=atoi(str); else sp_agc=0;

 speex_p(sp_npp,sp_agc); //set denoise and agc

 strcpy(str, "Jitter");
 if(parseconf(str)>0) i=atoi(str); else i=0;
 if(i<8000) sp_jit=i; else sp_jit=0;

 strcpy(str, "VAD_level");
 if(parseconf(str)>0) i=atoi(str); else i=0;
 if((i>=0)&&(i<100)) vox_level=i; else vox_level=0;

 strcpy(str, "VAD_tail");
 if(parseconf(str)>0) i=atoi(str); else i=0;
 if((i>0)&&(i<100)) vad_tail=i; else vad_tail=0;

 strcpy(str, "VAD_signal");
 if(parseconf(str)>0)
 {
  if((str[0]>=0x30)&&(str[0]<=0x39)) vad_level=128*(str[0]-0x30);
  if((str[1]>=0x30)&&(str[1]<=0x39)) vad_signal=str[1]-0x30;
 }

 strcpy(str, "RawBufSize");
 if(parseconf(str)>0) i=atoi(str); else i=0;
 if((i>=80)&&(i<=240)) RawBufSize=i; else RawBufSize=160;

 strcpy(str, "SpeexResampler");
 if(parseconf(str)>0) speex_rs=atoi(str);

 strcpy(str, "NPP7");
 if(parseconf(str)>0) i=atoi(str); else i=0;
 if(i)
 {
  RawBufSize=180;
  npp7=1;
 }
 else npp7=0;

 strcpy(str, "Our_secret_access");
 if(parseconf(str)<=0) str[0]=0;
 set_access(str, 0);
 check_access();
}





