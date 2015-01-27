/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Copyright (c) 2012 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OPUS_PRIVATE_H
#define OPUS_PRIVATE_H

#include "arch.h"
#include "opus.h"
#include "celt.h"

struct OpusRepacketizer {
	unsigned char toc;
	int nb_frames;
	const unsigned char *frames[48];
	int16_t len[48];
	int framesize;
};

typedef struct ChannelLayout {
	int nb_channels;
	int nb_streams;
	int nb_coupled_streams;
	unsigned char mapping[256];
} ChannelLayout;

int validate_layout(const ChannelLayout * layout);
int get_left_channel(const ChannelLayout * layout, int stream_id, int prev);
int get_right_channel(const ChannelLayout * layout, int stream_id, int prev);
int get_mono_channel(const ChannelLayout * layout, int stream_id, int prev);

#define MODE_SILK_ONLY          1000
#define MODE_HYBRID             1001
#define MODE_CELT_ONLY          1002

#define OPUS_SET_VOICE_RATIO_REQUEST         11018
#define OPUS_GET_VOICE_RATIO_REQUEST         11019

/** Configures the encoder's expected percentage of voice
  * opposed to music or other signals.
  *
  * @note This interface is currently more aspiration than actuality. It's
  * ultimately expected to bias an automatic signal classifier, but it currently
  * just shifts the static bitrate to mode mapping around a little bit.
  *
  * @param[in] x <tt>int</tt>:   Voice percentage in the range 0-100, inclusive.
  * @hideinitializer */
#define OPUS_SET_VOICE_RATIO(x) OPUS_SET_VOICE_RATIO_REQUEST, __opus_check_int(x)
/** Gets the encoder's configured voice ratio value, @see OPUS_SET_VOICE_RATIO
  *
  * @param[out] x <tt>int*</tt>:  Voice percentage in the range 0-100, inclusive.
  * @hideinitializer */
#define OPUS_GET_VOICE_RATIO(x) OPUS_GET_VOICE_RATIO_REQUEST, __opus_check_int_ptr(x)

#define OPUS_SET_FORCE_MODE_REQUEST    11002
#define OPUS_SET_FORCE_MODE(x) OPUS_SET_FORCE_MODE_REQUEST, __opus_check_int(x)

typedef void (*downmix_func) (const void *, opus_val32 *, int, int, int, int,
			      int);
void downmix_int(const void *_x, opus_val32 * sub, int subframe, int offset,
		 int c1, int c2, int C);

int optimize_framesize(const opus_val16 * x, int len, int C, int32_t Fs,
		       int bitrate, opus_val16 tonality, float *mem,
		       int buffering, downmix_func downmix);

int encode_size(int size, unsigned char *data);

int32_t frame_size_select(int32_t frame_size, int variable_duration,
			     int32_t Fs);

int32_t compute_frame_size(const void *analysis_pcm, int frame_size,
			      int variable_duration, int C, int32_t Fs,
			      int bitrate_bps, int delay_compensation,
			      downmix_func downmix
#ifndef DISABLE_FLOAT_API
			      , float *subframe_mem
#endif
    );

int32_t opus_encode_native(OpusEncoder * st, const opus_val16 * pcm,
			      int frame_size, unsigned char *data,
			      int32_t out_data_bytes, int lsb_depth,
			      const void *analysis_pcm,
			      int32_t analysis_size, int c1, int c2,
			      int analysis_channels, downmix_func downmix);

int opus_decode_native(OpusDecoder * st, const unsigned char *data,
		       int32_t len, opus_val16 * pcm, int frame_size,
		       int decode_fec, int self_delimited,
		       int32_t * packet_offset, int soft_clip);

/* Make sure everything's aligned to sizeof(void *) bytes */
static inline int align(int i)
{
	return (i + (int)sizeof(void *) - 1) & -(int)sizeof(void *);
}

int opus_packet_parse_impl(const unsigned char *data, int32_t len,
			   int self_delimited, unsigned char *out_toc,
			   const unsigned char *frames[48], int16_t size[48],
			   int *payload_offset, int32_t * packet_offset);

int32_t opus_repacketizer_out_range_impl(OpusRepacketizer * rp, int begin,
					    int end, unsigned char *data,
					    int32_t maxlen,
					    int self_delimited, int pad);

int pad_frame(unsigned char *data, int32_t len, int32_t new_len);

#endif				/* OPUS_PRIVATE_H */
