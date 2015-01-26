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

#include "SKP_Silk_SDK_API.h"
#include "SKP_Silk_main_FIX.h"

/*********************/
/* Decoder functions */
/*********************/

int SKP_Silk_SDK_Get_Decoder_Size(int32_t * decSizeBytes)
{
	int ret = 0;

	*decSizeBytes = sizeof(SKP_Silk_decoder_state);

	return ret;
}

/* Reset decoder state */
int SKP_Silk_SDK_InitDecoder(void *decState	/* I/O: State                                          */
    )
{
	int ret = 0;
	SKP_Silk_decoder_state *struc;

	struc = (SKP_Silk_decoder_state *) decState;

	ret = SKP_Silk_init_decoder(struc);

	return ret;
}

/* Decode a frame */
int SKP_Silk_SDK_Decode(void *decState,	/* I/O: State                                           */
			SKP_SILK_SDK_DecControlStruct * decControl,	/* I/O: Control structure                               */
			int lostFlag,	/* I:   0: no loss, 1 loss                              */
			const uint8_t * inData,	/* I:   Encoded input vector                            */
			const int nBytesIn,	/* I:   Number of input Bytes                           */
			int16_t * samplesOut,	/* O:   Decoded output speech vector                    */
			int16_t * nSamplesOut	/* I/O: Number of samples (vector/decoded)              */
    )
{
	int ret = 0, used_bytes, prev_fs_kHz;
	SKP_Silk_decoder_state *psDec;

	psDec = (SKP_Silk_decoder_state *) decState;

    /**********************************/
	/* Test if first frame in payload */
    /**********************************/
	if (psDec->moreInternalDecoderFrames == 0) {
		/* First Frame in Payload */
		psDec->nFramesDecoded = 0;	/* Used to count frames in packet */
	}

	if (psDec->moreInternalDecoderFrames == 0 &&	/* First frame in packet    */
	    lostFlag == 0 &&	/* Not packet loss          */
	    nBytesIn > MAX_ARITHM_BYTES) {	/* Too long payload         */
		/* Avoid trying to decode a too large packet */
		lostFlag = 1;
		ret = SKP_SILK_DEC_PAYLOAD_TOO_LARGE;
	}

	/* Save previous sample frequency */
	prev_fs_kHz = psDec->fs_kHz;

	/* Call decoder for one frame */
	ret +=
	    SKP_Silk_decode_frame(psDec, samplesOut, nSamplesOut, inData,
				  nBytesIn, lostFlag, &used_bytes);

	if (used_bytes) {	/* Only Call if not a packet loss */
		if (psDec->nBytesLeft > 0
		    && psDec->FrameTermination == SKP_SILK_MORE_FRAMES
		    && psDec->nFramesDecoded < 5) {
			/* We have more frames in the Payload */
			psDec->moreInternalDecoderFrames = 1;
		} else {
			/* Last frame in Payload */
			psDec->moreInternalDecoderFrames = 0;
			psDec->nFramesInPacket = psDec->nFramesDecoded;

			/* Track inband FEC usage */
			if (psDec->vadFlag == VOICE_ACTIVITY) {
				if (psDec->FrameTermination ==
				    SKP_SILK_LAST_FRAME) {
					psDec->no_FEC_counter++;
					if (psDec->no_FEC_counter >
					    NO_LBRR_THRES) {
						psDec->inband_FEC_offset = 0;
					}
				} else if (psDec->FrameTermination ==
					   SKP_SILK_LBRR_VER1) {
					psDec->inband_FEC_offset = 1;	/* FEC info with 1 packet delay */
					psDec->no_FEC_counter = 0;
				} else if (psDec->FrameTermination ==
					   SKP_SILK_LBRR_VER2) {
					psDec->inband_FEC_offset = 2;	/* FEC info with 2 packets delay */
					psDec->no_FEC_counter = 0;
				}
			}
		}
	}

	if (psDec->fs_kHz * 1000 > decControl->sampleRate) {
		ret = SKP_SILK_DEC_WRONG_SAMPLING_FREQUENCY;
	}

	/* Do any resampling if needed */
	if (psDec->fs_kHz * 1000 != decControl->sampleRate) {
		int16_t samplesOut_tmp[2 * MAX_FRAME_LENGTH];
		int32_t scratch[3 * MAX_FRAME_LENGTH];

		/* Copy to a tmpbuffer as the resampling writes to samplesOut */
		memcpy(samplesOut_tmp, samplesOut,
		       *nSamplesOut * sizeof(int16_t));

		/* Clear resampler state when switching internal sampling frequency */
		if (prev_fs_kHz != psDec->fs_kHz) {
			SKP_memset(psDec->resampleState, 0,
				   sizeof(psDec->resampleState));
		}

		if (psDec->fs_kHz == 16 && decControl->sampleRate == 24000) {
			/* Resample from 16 kHz to 24 kHz */
			SKP_Silk_resample_3_2(samplesOut, psDec->resampleState,
					      samplesOut_tmp, *nSamplesOut);
		} else if (psDec->fs_kHz == 12
			   && decControl->sampleRate == 24000) {
			/* Resample from 12 kHz to 24 kHz */
			SKP_Silk_resample_2_1_coarse(samplesOut_tmp,
						     psDec->resampleState,
						     samplesOut, scratch,
						     *nSamplesOut);
		} else if (psDec->fs_kHz == 8
			   && decControl->sampleRate == 24000) {
			/* Resample from 8 kHz to 24 kHz */
			SKP_Silk_resample_3_1(samplesOut, psDec->resampleState,
					      samplesOut_tmp, *nSamplesOut);
		} else if (psDec->fs_kHz == 12
			   && decControl->sampleRate == 16000) {
			/* Resample from 12 kHz to 16 kHz */
			SKP_Silk_resample_4_3(samplesOut, psDec->resampleState,
					      samplesOut_tmp, *nSamplesOut);
		} else if (psDec->fs_kHz == 8
			   && decControl->sampleRate == 16000) {
			/* Resample from 8 kHz to 16 kHz */
			SKP_Silk_resample_2_1_coarse(samplesOut_tmp,
						     psDec->resampleState,
						     samplesOut, scratch,
						     *nSamplesOut);
		} else if (psDec->fs_kHz == 8
			   && decControl->sampleRate == 12000) {
			/* Resample from 8 kHz to 12 kHz */
			SKP_Silk_resample_3_2(samplesOut, psDec->resampleState,
					      samplesOut_tmp, *nSamplesOut);
		}

		*nSamplesOut =
		    SKP_DIV32((int32_t) * nSamplesOut * decControl->sampleRate,
			      psDec->fs_kHz * 1000);
	}

	/* Copy all parameters that are needed out of internal structure to the control stucture */
	decControl->frameSize = (int)psDec->frame_length;
	decControl->framesPerPacket = (int)psDec->nFramesInPacket;
	decControl->inBandFECOffset = (int)psDec->inband_FEC_offset;
	decControl->moreInternalDecoderFrames =
	    (int)psDec->moreInternalDecoderFrames;

	return ret;
}

