/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SILK_STRUCTS_H
#define SILK_STRUCTS_H

#include "typedef.h"
#include "SigProc_FIX.h"
#include "define.h"
#include "entenc.h"
#include "entdec.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************/
/* Noise shaping quantization state */
/************************************/
	typedef struct {
		int16_t xq[2 * MAX_FRAME_LENGTH];	/* Buffer for quantized output signal                             */
		int32_t sLTP_shp_Q14[2 * MAX_FRAME_LENGTH];
		int32_t sLPC_Q14[MAX_SUB_FRAME_LENGTH + NSQ_LPC_BUF_LENGTH];
		int32_t sAR2_Q14[MAX_SHAPE_LPC_ORDER];
		int32_t sLF_AR_shp_Q14;
		int lagPrev;
		int sLTP_buf_idx;
		int sLTP_shp_buf_idx;
		int32_t rand_seed;
		int32_t prev_gain_Q16;
		int rewhite_flag;
	} silk_nsq_state;

/********************************/
/* VAD state                    */
/********************************/
	typedef struct {
		int32_t AnaState[2];	/* Analysis filterbank state: 0-8 kHz                                   */
		int32_t AnaState1[2];	/* Analysis filterbank state: 0-4 kHz                                   */
		int32_t AnaState2[2];	/* Analysis filterbank state: 0-2 kHz                                   */
		int32_t XnrgSubfr[VAD_N_BANDS];	/* Subframe energies                                                    */
		int32_t NrgRatioSmth_Q8[VAD_N_BANDS];	/* Smoothed energy level in each band                                   */
		int16_t HPstate;	/* State of differentiator in the lowest band                           */
		int32_t NL[VAD_N_BANDS];	/* Noise energy level in each band                                      */
		int32_t inv_NL[VAD_N_BANDS];	/* Inverse noise energy level in each band                              */
		int32_t NoiseLevelBias[VAD_N_BANDS];	/* Noise level estimator bias/offset                                    */
		int32_t counter;	/* Frame counter used in the initial phase                              */
	} silk_VAD_state;

/* Variable cut-off low-pass filter state */
	typedef struct {
		int32_t In_LP_State[2];	/* Low pass filter state */
		int32_t transition_frame_no;	/* Counter which is mapped to a cut-off frequency */
		int mode;	/* Operating mode, <0: switch down, >0: switch up; 0: do nothing           */
	} silk_LP_state;

/* Structure containing NLSF codebook */
	typedef struct {
		const int16_t nVectors;
		const int16_t order;
		const int16_t quantStepSize_Q16;
		const int16_t invQuantStepSize_Q6;
		const uint8_t *CB1_NLSF_Q8;
		const uint8_t *CB1_iCDF;
		const uint8_t *pred_Q8;
		const uint8_t *ec_sel;
		const uint8_t *ec_iCDF;
		const uint8_t *ec_Rates_Q5;
		const int16_t *deltaMin_Q15;
	} silk_NLSF_CB_struct;

	typedef struct {
		int16_t pred_prev_Q13[2];
		int16_t sMid[2];
		int16_t sSide[2];
		int32_t mid_side_amp_Q0[4];
		int16_t smth_width_Q14;
		int16_t width_prev_Q14;
		int16_t silent_side_len;
		int8_t predIx[MAX_FRAMES_PER_PACKET][2][3];
		int8_t mid_only_flags[MAX_FRAMES_PER_PACKET];
	} stereo_enc_state;

	typedef struct {
		int16_t pred_prev_Q13[2];
		int16_t sMid[2];
		int16_t sSide[2];
	} stereo_dec_state;

	typedef struct {
		int8_t GainsIndices[MAX_NB_SUBFR];
		int8_t LTPIndex[MAX_NB_SUBFR];
		int8_t NLSFIndices[MAX_LPC_ORDER + 1];
		int16_t lagIndex;
		int8_t contourIndex;
		int8_t signalType;
		int8_t quantOffsetType;
		int8_t NLSFInterpCoef_Q2;
		int8_t PERIndex;
		int8_t LTP_scaleIndex;
		int8_t Seed;
	} SideInfoIndices;

/********************************/
/* Encoder state                */
/********************************/
	typedef struct {
		int32_t In_HP_State[2];	/* High pass filter state                                           */
		int32_t variable_HP_smth1_Q15;	/* State of first smoother                                          */
		int32_t variable_HP_smth2_Q15;	/* State of second smoother                                         */
		silk_LP_state sLP;	/* Low pass filter state                                            */
		silk_VAD_state sVAD;	/* Voice activity detector state                                    */
		silk_nsq_state sNSQ;	/* Noise Shape Quantizer State                                      */
		int16_t prev_NLSFq_Q15[MAX_LPC_ORDER];	/* Previously quantized NLSF vector                                 */
		int speech_activity_Q8;	/* Speech activity                                                  */
		int allow_bandwidth_switch;	/* Flag indicating that switching of internal bandwidth is allowed  */
		int8_t LBRRprevLastGainIndex;
		int8_t prevSignalType;
		int prevLag;
		int pitch_LPC_win_length;
		int max_pitch_lag;	/* Highest possible pitch lag (samples)                             */
		int32_t API_fs_Hz;	/* API sampling frequency (Hz)                                      */
		int32_t prev_API_fs_Hz;	/* Previous API sampling frequency (Hz)                             */
		int maxInternal_fs_Hz;	/* Maximum internal sampling frequency (Hz)                         */
		int minInternal_fs_Hz;	/* Minimum internal sampling frequency (Hz)                         */
		int desiredInternal_fs_Hz;	/* Soft request for internal sampling frequency (Hz)                */
		int fs_kHz;	/* Internal sampling frequency (kHz)                                */
		int nb_subfr;	/* Number of 5 ms subframes in a frame                              */
		int frame_length;	/* Frame length (samples)                                           */
		int subfr_length;	/* Subframe length (samples)                                        */
		int ltp_mem_length;	/* Length of LTP memory                                             */
		int la_pitch;	/* Look-ahead for pitch analysis (samples)                          */
		int la_shape;	/* Look-ahead for noise shape analysis (samples)                    */
		int shapeWinLength;	/* Window length for noise shape analysis (samples)                 */
		int32_t TargetRate_bps;	/* Target bitrate (bps)                                             */
		int PacketSize_ms;	/* Number of milliseconds to put in each packet                     */
		int PacketLoss_perc;	/* Packet loss rate measured by farend                              */
		int32_t frameCounter;
		int Complexity;	/* Complexity setting                                               */
		int nStatesDelayedDecision;	/* Number of states in delayed decision quantization                */
		int useInterpolatedNLSFs;	/* Flag for using NLSF interpolation                                */
		int shapingLPCOrder;	/* Filter order for noise shaping filters                           */
		int predictLPCOrder;	/* Filter order for prediction filters                              */
		int pitchEstimationComplexity;	/* Complexity level for pitch estimator                             */
		int pitchEstimationLPCOrder;	/* Whitening filter order for pitch estimator                       */
		int32_t pitchEstimationThreshold_Q16;	/* Threshold for pitch estimator                                    */
		int LTPQuantLowComplexity;	/* Flag for low complexity LTP quantization                         */
		int mu_LTP_Q9;	/* Rate-distortion tradeoff in LTP quantization                     */
		int32_t sum_log_gain_Q7;	/* Cumulative max prediction gain                                                                       */
		int NLSF_MSVQ_Survivors;	/* Number of survivors in NLSF MSVQ                                 */
		int first_frame_after_reset;	/* Flag for deactivating NLSF interpolation, pitch prediction       */
		int controlled_since_last_payload;	/* Flag for ensuring codec_control only runs once per packet        */
		int warping_Q16;	/* Warping parameter for warped noise shaping                       */
		int useCBR;	/* Flag to enable constant bitrate                                  */
		int prefillFlag;	/* Flag to indicate that only buffers are prefilled, no coding      */
		const uint8_t *pitch_lag_low_bits_iCDF;	/* Pointer to iCDF table for low bits of pitch lag index            */
		const uint8_t *pitch_contour_iCDF;	/* Pointer to iCDF table for pitch contour index                    */
		const silk_NLSF_CB_struct *psNLSF_CB;	/* Pointer to NLSF codebook                                         */
		int input_quality_bands_Q15[VAD_N_BANDS];
		int input_tilt_Q15;
		int SNR_dB_Q7;	/* Quality setting                                                  */

		int8_t VAD_flags[MAX_FRAMES_PER_PACKET];
		int8_t LBRR_flag;
		int LBRR_flags[MAX_FRAMES_PER_PACKET];

		SideInfoIndices indices;
		int8_t pulses[MAX_FRAME_LENGTH];

		int arch;

		/* Input/output buffering */
		int16_t inputBuf[MAX_FRAME_LENGTH + 2];	/* Buffer containing input signal                                   */
		int inputBufIx;
		int nFramesPerPacket;
		int nFramesEncoded;	/* Number of frames analyzed in current packet                      */

		int nChannelsAPI;
		int nChannelsInternal;
		int channelNb;

		/* Parameters For LTP scaling Control */
		int frames_since_onset;

		/* Specifically for entropy coding */
		int ec_prevSignalType;
		int16_t ec_prevLagIndex;

		silk_resampler_state_struct resampler_state;

		/* DTX */
		int useDTX;	/* Flag to enable DTX                                               */
		int inDTX;	/* Flag to signal DTX period                                        */
		int noSpeechCounter;	/* Counts concecutive nonactive frames, used by DTX                 */

		/* Inband Low Bitrate Redundancy (LBRR) data */
		int useInBandFEC;	/* Saves the API setting for query                                  */
		int LBRR_enabled;	/* Depends on useInBandFRC, bitrate and packet loss rate            */
		int LBRR_GainIncreases;	/* Gains increment for coding LBRR frames                           */
		SideInfoIndices indices_LBRR[MAX_FRAMES_PER_PACKET];
		int8_t pulses_LBRR[MAX_FRAMES_PER_PACKET][MAX_FRAME_LENGTH];
	} silk_encoder_state;

/* Struct for Packet Loss Concealment */
	typedef struct {
		int32_t pitchL_Q8;	/* Pitch lag to use for voiced concealment                          */
		int16_t LTPCoef_Q14[LTP_ORDER];	/* LTP coeficients to use for voiced concealment                    */
		int16_t prevLPC_Q12[MAX_LPC_ORDER];
		int last_frame_lost;	/* Was previous frame lost                                          */
		int32_t rand_seed;	/* Seed for unvoiced signal generation                              */
		int16_t randScale_Q14;	/* Scaling of unvoiced random signal                                */
		int32_t conc_energy;
		int conc_energy_shift;
		int16_t prevLTP_scale_Q14;
		int32_t prevGain_Q16[2];
		int fs_kHz;
		int nb_subfr;
		int subfr_length;
	} silk_PLC_struct;

/* Struct for CNG */
	typedef struct {
		int32_t CNG_exc_buf_Q14[MAX_FRAME_LENGTH];
		int16_t CNG_smth_NLSF_Q15[MAX_LPC_ORDER];
		int32_t CNG_synth_state[MAX_LPC_ORDER];
		int32_t CNG_smth_Gain_Q16;
		int32_t rand_seed;
		int fs_kHz;
	} silk_CNG_struct;

/********************************/
/* Decoder state                */
/********************************/
	typedef struct {
		int32_t prev_gain_Q16;
		int32_t exc_Q14[MAX_FRAME_LENGTH];
		int32_t sLPC_Q14_buf[MAX_LPC_ORDER];
		int16_t outBuf[MAX_FRAME_LENGTH + 2 * MAX_SUB_FRAME_LENGTH];	/* Buffer for output signal                     */
		int lagPrev;	/* Previous Lag                                                     */
		int8_t LastGainIndex;	/* Previous gain index                                              */
		int fs_kHz;	/* Sampling frequency in kHz                                        */
		int32_t fs_API_hz;	/* API sample frequency (Hz)                                        */
		int nb_subfr;	/* Number of 5 ms subframes in a frame                              */
		int frame_length;	/* Frame length (samples)                                           */
		int subfr_length;	/* Subframe length (samples)                                        */
		int ltp_mem_length;	/* Length of LTP memory                                             */
		int LPC_order;	/* LPC order                                                        */
		int16_t prevNLSF_Q15[MAX_LPC_ORDER];	/* Used to interpolate LSFs                                         */
		int first_frame_after_reset;	/* Flag for deactivating NLSF interpolation                         */
		const uint8_t *pitch_lag_low_bits_iCDF;	/* Pointer to iCDF table for low bits of pitch lag index            */
		const uint8_t *pitch_contour_iCDF;	/* Pointer to iCDF table for pitch contour index                    */

		/* For buffering payload in case of more frames per packet */
		int nFramesDecoded;
		int nFramesPerPacket;

		/* Specifically for entropy coding */
		int ec_prevSignalType;
		int16_t ec_prevLagIndex;

		int VAD_flags[MAX_FRAMES_PER_PACKET];
		int LBRR_flag;
		int LBRR_flags[MAX_FRAMES_PER_PACKET];

		silk_resampler_state_struct resampler_state;

		const silk_NLSF_CB_struct *psNLSF_CB;	/* Pointer to NLSF codebook                                         */

		/* Quantization indices */
		SideInfoIndices indices;

		/* CNG state */
		silk_CNG_struct sCNG;

		/* Stuff used for PLC */
		int lossCnt;
		int prevSignalType;

		silk_PLC_struct sPLC;

	} silk_decoder_state;

/************************/
/* Decoder control      */
/************************/
	typedef struct {
		/* Prediction and coding parameters */
		int pitchL[MAX_NB_SUBFR];
		int32_t Gains_Q16[MAX_NB_SUBFR];
		/* Holds interpolated and final coefficients, 4-byte aligned */
		silk_DWORD_ALIGN int16_t PredCoef_Q12[2][MAX_LPC_ORDER];
		int16_t LTPCoef_Q14[LTP_ORDER * MAX_NB_SUBFR];
		int LTP_scale_Q14;
	} silk_decoder_control;

#ifdef __cplusplus
}
#endif
#endif
