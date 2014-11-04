/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SKP_SILK_STRUCTS_H
#define SKP_SILK_STRUCTS_H

#include "SKP_Silk_typedef.h"
#include "SKP_Silk_SigProc_FIX.h"
#include "SKP_Silk_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************/
/* Noise shaping quantization state */
/************************************/
	typedef struct {
		int16_t xq[2 * MAX_FRAME_LENGTH];	/* Buffer for quantized output signal */
		int32_t sLTP_shp_Q10[2 * MAX_FRAME_LENGTH];
		int32_t sLPC_Q14[MAX_FRAME_LENGTH / NB_SUBFR + MAX_LPC_ORDER];
		int32_t sLF_AR_shp_Q12;
		int lagPrev;
		int sLTP_buf_idx;
		int sLTP_shp_buf_idx;
		int32_t rand_seed;
		int32_t prev_inv_gain_Q16;
		int rewhite_flag;
	} SKP_Silk_nsq_state;	/* FIX */

/* Struct for Low BitRate Redundant (LBRR) information */
	typedef struct {
		uint8_t payload[MAX_ARITHM_BYTES];
		int nBytes;	/* Number of bytes in payload                               */
		int usage;	/* Tells how the payload should be used as FEC              */
	} SKP_SILK_LBRR_struct;

/********************************/
/* VAD state                    */
/********************************/
	typedef struct {
		int32_t AnaState[2];	/* Analysis filterbank state: 0-8 kHz                       */
		int32_t AnaState1[2];	/* Analysis filterbank state: 0-4 kHz                       */
		int32_t AnaState2[2];	/* Analysis filterbank state: 0-2 kHz                       */
		int32_t XnrgSubfr[VAD_N_BANDS];	/* Subframe energies                                        */
		int32_t NrgRatioSmth_Q8[VAD_N_BANDS];	/* Smoothed energy level in each band                       */
		int16_t HPstate;	/* State of differentiator in the lowest band               */
		int32_t NL[VAD_N_BANDS];	/* Noise energy level in each band                          */
		int32_t inv_NL[VAD_N_BANDS];	/* Inverse noise energy level in each band                  */
		int32_t NoiseLevelBias[VAD_N_BANDS];	/* Noise level estimator bias/offset                        */
		int32_t counter;	/* Frame counter used in the initial phase                  */
	} SKP_Silk_VAD_state;

/*******************************/
/* Range encoder/decoder state */
/*******************************/
	typedef struct {
		int32_t bufferLength;
		int32_t bufferIx;
		uint32_t base_Q32;
		uint32_t range_Q16;
		int32_t error;
		uint8_t buffer[MAX_ARITHM_BYTES];	/* Buffer containing payload                                */
	} SKP_Silk_range_coder_state;

/* Input frequency range detection struct */
	typedef struct {
		int32_t S_HP_8_kHz[NB_SOS][2];	/* HP filter State */
		int32_t ConsecSmplsAboveThres;
		int32_t ActiveSpeech_ms;	/* Accumulated time with active speech */
		int SWB_detected;	/* Flag to indicate SWB input */
		int WB_detected;	/* Flag to indicate WB input */
	} SKP_Silk_detect_SWB_state;

#if SWITCH_TRANSITION_FILTERING
/* Variable cut-off low-pass filter state */
	typedef struct {
		int32_t In_LP_State[2];	/* Low pass filter state */
		int32_t transition_frame_no;	/* Counter which is mapped to a cut-off frequency */
		int mode;	/* Operating mode, 0: switch down, 1: switch up */
	} SKP_Silk_LP_state;
#endif

/* Structure for one stage of MSVQ */
	typedef struct {
		const int32_t nVectors;
		const int16_t *CB_NLSF_Q15;
		const int16_t *Rates_Q5;
	} SKP_Silk_NLSF_CBS;

/* Structure containing NLSF MSVQ codebook */
	typedef struct {
		const int32_t nStages;

		/* Fields for (de)quantizing */
		const SKP_Silk_NLSF_CBS *CBStages;
		const int *NDeltaMin_Q15;

		/* Fields for arithmetic (de)coding */
		const uint16_t *CDF;
		const uint16_t *const *StartPtr;
		const int *MiddleIx;
	} SKP_Silk_NLSF_CB_struct;

/********************************/
/* Encoder state                */
/********************************/
	typedef struct {
		SKP_Silk_range_coder_state sRC;	/* Range coder state                                                    */
		SKP_Silk_range_coder_state sRC_LBRR;	/* Range coder state (for low bitrate redundancy)                       */
#if HIGH_PASS_INPUT
		int32_t In_HP_State[2];	/* High pass filter state                                               */
#endif
#if SWITCH_TRANSITION_FILTERING
		SKP_Silk_LP_state sLP;	/* Low pass filter state */
#endif
		SKP_Silk_VAD_state sVAD;	/* Voice activity detector state                                        */

		int LBRRprevLastGainIndex;
		int prev_sigtype;
		int typeOffsetPrev;	/* Previous signal type and quantization offset                         */
		int prevLag;
		int prev_lagIndex;
		int fs_kHz;	/* Sampling frequency (kHz)                                             */
		int fs_kHz_changed;	/* Did we switch yet?                                                   */
		int frame_length;	/* Frame length (samples)                                               */
		int subfr_length;	/* Subframe length (samples)                                            */
		int la_pitch;	/* Look-ahead for pitch analysis (samples)                              */
		int la_shape;	/* Look-ahead for noise shape analysis (samples)                        */
		int32_t TargetRate_bps;	/* Target bitrate (bps)                                                 */
		int PacketSize_ms;	/* Number of milliseconds to put in each packet                         */
		int PacketLoss_perc;	/* Packet loss rate measured by farend                                  */
		int32_t frameCounter;
		int Complexity;	/* Complexity setting: 0-> low; 1-> medium; 2->high                     */
		int nStatesDelayedDecision;	/* Number of states in delayed decision quantization                    */
		int useInterpolatedNLSFs;	/* Flag for using NLSF interpolation                                    */
		int shapingLPCOrder;	/* Filter order for noise shaping filters                               */
		int predictLPCOrder;	/* Filter order for prediction filters                                  */
		int pitchEstimationComplexity;	/* Complexity level for pitch estimator                                 */
		int pitchEstimationLPCOrder;	/* Whitening filter order for pitch estimator                           */
		int LTPQuantLowComplexity;	/* Flag for low complexity LTP quantization                             */
		int NLSF_MSVQ_Survivors;	/* Number of survivors in NLSF MSVQ                                     */
		int first_frame_after_reset;	/* Flag for deactivating NLSF interp. and fluc. reduction after resets  */

		/* Input/output buffering */
		int16_t inputBuf[MAX_FRAME_LENGTH];	/* buffer containin input signal                                        */
		int inputBufIx;
		int nFramesInPayloadBuf;	/* number of frames sitting in outputBuf                                */
		int nBytesInPayloadBuf;	/* number of bytes sitting in outputBuf                                 */

		/* Parameters For LTP scaling Control */
		int frames_since_onset;

		const SKP_Silk_NLSF_CB_struct *psNLSF_CB[2];	/* Pointers to voiced/unvoiced NLSF codebooks */

		/* Struct for Inband LBRR */
		SKP_SILK_LBRR_struct LBRR_buffer[MAX_LBRR_DELAY];
		int oldest_LBRR_idx;
		int LBRR_enabled;
		int LBRR_GainIncreases;	/* Number of shifts to Gains to get LBRR rate Voiced frames             */

		/* Bitrate control */
		int32_t bitrateDiff;	/* accumulated diff. between the target bitrate and the SWB/WB limits   */

#if LOW_COMPLEXITY_ONLY
		/* state for downsampling from 24 to 16 kHz in low complexity mode */
		int16_t
		    resample24To16state
		    [SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1];
#else
		int32_t resample24To16state[11];	/* state for downsampling from 24 to 16 kHz                             */
#endif
		int32_t resample24To12state[6];	/* state for downsampling from 24 to 12 kHz                             */
		int32_t resample24To8state[7];	/* state for downsampling from 24 to  8 kHz                             */
		int32_t resample16To12state[15];	/* state for downsampling from 16 to 12 kHz                             */
		int32_t resample16To8state[6];	/* state for downsampling from 16 to  8 kHz                             */
#if LOW_COMPLEXITY_ONLY
		/* state for downsampling from 12 to 8 kHz in low complexity mode */
		int16_t
		    resample12To8state[SigProc_Resample_2_3_coarse_NUM_FIR_COEFS
				       - 1];
#else
		int32_t resample12To8state[11];	/* state for downsampling from 12 to  8 kHz                             */
#endif

		/* DTX */
		int noSpeechCounter;	/* Counts concecutive nonactive frames, used by DTX                     */
		int useDTX;	/* Flag to enable DTX                                                   */
		int inDTX;	/* Flag to signal DTX period                                            */
		int vadFlag;	/* Flag to indicate Voice Activity                                      */

		/* Struct for detecting SWB input */
		SKP_Silk_detect_SWB_state sSWBdetect;

    /********************************************/
		/* Buffers etc used by the new bitstream V4 */
    /********************************************/
		int q[MAX_FRAME_LENGTH * MAX_FRAMES_PER_PACKET];	/* pulse signal buffer */
		int q_LBRR[MAX_FRAME_LENGTH * MAX_FRAMES_PER_PACKET];	/* pulse signal buffer */
		int sigtype[MAX_FRAMES_PER_PACKET];
		int QuantOffsetType[MAX_FRAMES_PER_PACKET];
		int extension_layer;	/* Add extension layer      */
		int bitstream_v;	/* Holds bitstream version  */
	} SKP_Silk_encoder_state;

/************************/
/* Encoder control      */
/************************/
	typedef struct {
		/* Quantization indices */
		int lagIndex;
		int contourIndex;
		int PERIndex;
		int LTPIndex[NB_SUBFR];
		int NLSFIndices[NLSF_MSVQ_MAX_CB_STAGES];	/* NLSF path of quantized LSF vector   */
		int NLSFInterpCoef_Q2;
		int GainsIndices[NB_SUBFR];
		int32_t Seed;
		int LTP_scaleIndex;
		int RateLevelIndex;
		int QuantOffsetType;
		int sigtype;

		/* Prediction and coding parameters */
		int pitchL[NB_SUBFR];

		int LBRR_usage;	/* Low bitrate redundancy usage                             */
	} SKP_Silk_encoder_control;

/* Struct for Packet Loss Concealment */
	typedef struct {
		int32_t pitchL_Q8;	/* Pitch lag to use for voiced concealment                  */
		int16_t LTPCoef_Q14[LTP_ORDER];	/* LTP coeficients to use for voiced concealment            */
		int16_t prevLPC_Q12[MAX_LPC_ORDER];
		int last_frame_lost;	/* Was previous frame lost                                  */
		int32_t rand_seed;	/* Seed for unvoiced signal generation                      */
		int16_t randScale_Q14;	/* Scaling of unvoiced random signal                        */
		int32_t conc_energy;
		int conc_energy_shift;
		int16_t prevLTP_scale_Q14;
		int32_t prevGain_Q16[NB_SUBFR];
		int fs_kHz;
	} SKP_Silk_PLC_struct;

/* Struct for CNG */
	typedef struct {
		int32_t CNG_exc_buf_Q10[MAX_FRAME_LENGTH];
		int CNG_smth_NLSF_Q15[MAX_LPC_ORDER];
		int32_t CNG_synth_state[MAX_LPC_ORDER];
		int32_t CNG_smth_Gain_Q16;
		int32_t rand_seed;
		int fs_kHz;
	} SKP_Silk_CNG_struct;

/********************************/
/* Decoder state                */
/********************************/
	typedef struct {
		SKP_Silk_range_coder_state sRC;	/* Range coder state                                                    */
		int32_t prev_inv_gain_Q16;
		int32_t sLTP_Q16[2 * MAX_FRAME_LENGTH];
		int32_t sLPC_Q14[MAX_FRAME_LENGTH / NB_SUBFR + MAX_LPC_ORDER];
		int32_t exc_Q10[MAX_FRAME_LENGTH];
		int32_t res_Q10[MAX_FRAME_LENGTH];
		int16_t outBuf[2 * MAX_FRAME_LENGTH];	/* Buffer for output signal                                             */
		int sLTP_buf_idx;	/* LTP_buf_index                                                        */
		int lagPrev;	/* Previous Lag                                                         */
		int LastGainIndex;	/* Previous gain index                                                  */
		int LastGainIndex_EnhLayer;	/* Previous gain index                                                  */
		int typeOffsetPrev;	/* Previous signal type and quantization offset                         */
		int32_t HPState[DEC_HP_ORDER];	/* HP filter state                                                      */
		const int16_t *HP_A;	/* HP filter AR coefficients                                            */
		const int16_t *HP_B;	/* HP filter MA coefficients                                            */
		int fs_kHz;	/* Sampling frequency in kHz                                            */
		int frame_length;	/* Frame length (samples)                                               */
		int subfr_length;	/* Subframe length (samples)                                            */
		int LPC_order;	/* LPC order                                                            */
		int prevNLSF_Q15[MAX_LPC_ORDER];	/* Used to interpolate LSFs                                             */
		int first_frame_after_reset;	/* Flag for deactivating NLSF interp. and fluc. reduction after resets  */

		/* For buffering payload in case of more frames per packet */
		int nBytesLeft;
		int nFramesDecoded;
		int nFramesInPacket;
		int moreInternalDecoderFrames;
		int FrameTermination;

		int32_t resampleState[15];

		const SKP_Silk_NLSF_CB_struct *psNLSF_CB[2];	/* Pointers to voiced/unvoiced NLSF codebooks */

		int sigtype[MAX_FRAMES_PER_PACKET];
		int QuantOffsetType[MAX_FRAMES_PER_PACKET];
		int GainsIndices[MAX_FRAMES_PER_PACKET][NB_SUBFR];
		int GainsIndices_EnhLayer[MAX_FRAMES_PER_PACKET][NB_SUBFR];
		int NLSFIndices[MAX_FRAMES_PER_PACKET][NLSF_MSVQ_MAX_CB_STAGES];
		int NLSFInterpCoef_Q2[MAX_FRAMES_PER_PACKET];
		int lagIndex[MAX_FRAMES_PER_PACKET];
		int contourIndex[MAX_FRAMES_PER_PACKET];
		int PERIndex[MAX_FRAMES_PER_PACKET];
		int LTPIndex[MAX_FRAMES_PER_PACKET][NB_SUBFR];
		int LTP_scaleIndex[MAX_FRAMES_PER_PACKET];
		int Seed[MAX_FRAMES_PER_PACKET];
		int vadFlagBuf[MAX_FRAMES_PER_PACKET];

		/* Parameters used to investigate if inband FEC is used */
		int vadFlag;
		int no_FEC_counter;	/* Counts number of frames wo inband FEC                                */
		int inband_FEC_offset;	/* 0: no FEC, 1: FEC with 1 packet offset, 2: FEC w 2 packets offset    */

		/* CNG state */
		SKP_Silk_CNG_struct sCNG;

		/* Stuff used for PLC */
		SKP_Silk_PLC_struct sPLC;
		int lossCnt;
		int prev_sigtype;	/* Previous sigtype                                                     */
#ifdef USE_INTERPOLATION_PLC
		int16_t prevQuant[MAX_FRAME_LENGTH];
		int prevPitchL[NB_SUBFR];	/* Previous Lags used                                                   */
		int16_t prevLTPCoef_Q14[NB_SUBFR * LTP_ORDER];	/* Previous LTCoefs used                                                */
		int16_t prevAR_Q12[MAX_LPC_ORDER];
		int interpolDistance;	/* Number of frames between old and new recieved packet                 */
#endif

		int bitstream_v;	/* Holds bitstream version                                              */
	} SKP_Silk_decoder_state;

/************************/
/* Decoder control      */
/************************/
	typedef struct {
		/* prediction and coding parameters */
		int pitchL[NB_SUBFR];
		int32_t Gains_Q16[NB_SUBFR];
		int32_t Seed;
		/* holds interpolated and final coefficients, 4-byte aligned */
		 SKP_array_of_int16_4_byte_aligned(PredCoef_Q12[2],
						   MAX_LPC_ORDER);
		int16_t LTPCoef_Q14[LTP_ORDER * NB_SUBFR];
		int LTP_scale_Q14;

		/* quantization indices */
		int PERIndex;
		int RateLevelIndex;
		int QuantOffsetType;
		int sigtype;
		int NLSFInterpCoef_Q2;
	} SKP_Silk_decoder_control;

#ifdef __cplusplus
}
#endif
#endif
