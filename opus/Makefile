
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O3 -fomit-frame-pointer -ffast-math -funroll-loops -Iinclude -Icelt -Isilk -Isilk/float -DHAVE_CONFIG_H

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops

LOBJECTS = celt/bands.o celt/celt.o \
	celt/celt_encoder.o celt/celt_decoder.o celt/cwrs.o \
	celt/entcode.o celt/entdec.o celt/entenc.o celt/kiss_fft.o \
	celt/laplace.o celt/mathops.o celt/mdct.o celt/modes.o \
	celt/pitch.o celt/celt_lpc.o celt/quant_bands.o celt/rate.o \
	celt/vq.o  silk/CNG.o \
	silk/code_signs.o silk/init_decoder.o silk/decode_core.o \
	silk/decode_frame.o silk/decode_parameters.o \
	silk/decode_indices.o silk/decode_pulses.o \
	silk/decoder_set_fs.o silk/dec_API.o silk/enc_API.o \
	silk/encode_indices.o silk/encode_pulses.o silk/gain_quant.o \
	silk/interpolate.o silk/LP_variable_cutoff.o \
	silk/NLSF_decode.o silk/NSQ.o silk/NSQ_del_dec.o silk/PLC.o \
	silk/shell_coder.o silk/tables_gain.o silk/tables_LTP.o \
	silk/tables_NLSF_CB_NB_MB.o silk/tables_NLSF_CB_WB.o \
	silk/tables_other.o silk/tables_pitch_lag.o \
	silk/tables_pulses_per_block.o silk/VAD.o \
	silk/control_audio_bandwidth.o silk/quant_LTP_gains.o \
	silk/VQ_WMat_EC.o silk/HP_variable_cutoff.o silk/NLSF_encode.o \
	silk/NLSF_VQ.o silk/NLSF_unpack.o silk/NLSF_del_dec_quant.o \
	silk/process_NLSFs.o silk/stereo_LR_to_MS.o \
	silk/stereo_MS_to_LR.o silk/check_control_input.o \
	silk/control_SNR.o silk/init_encoder.o silk/control_codec.o \
	silk/A2NLSF.o silk/ana_filt_bank_1.o silk/biquad_alt.o \
	silk/bwexpander_32.o silk/bwexpander.o silk/debug.o \
	silk/decode_pitch.o silk/inner_prod_aligned.o silk/lin2log.o \
	silk/log2lin.o silk/LPC_analysis_filter.o \
	silk/LPC_inv_pred_gain.o silk/table_LSF_cos.o silk/NLSF2A.o \
	silk/NLSF_stabilize.o silk/NLSF_VQ_weights_laroia.o \
	silk/pitch_est_tables.o silk/resampler.o \
	silk/resampler_down2_3.o silk/resampler_down2.o \
	silk/resampler_private_AR2.o silk/resampler_private_down_FIR.o \
	silk/resampler_private_IIR_FIR.o \
	silk/resampler_private_up2_HQ.o silk/resampler_rom.o \
	silk/sigm_Q15.o silk/sort.o silk/sum_sqr_shift.o \
	silk/stereo_decode_pred.o silk/stereo_encode_pred.o \
	silk/stereo_find_predictor.o silk/stereo_quant_pred.o \
	silk/fixed/LTP_analysis_filter_FIX.o \
	silk/fixed/LTP_scale_ctrl_FIX.o silk/fixed/corrMatrix_FIX.o \
	silk/fixed/encode_frame_FIX.o silk/fixed/find_LPC_FIX.o \
	silk/fixed/find_LTP_FIX.o silk/fixed/find_pitch_lags_FIX.o \
	silk/fixed/find_pred_coefs_FIX.o \
	silk/fixed/noise_shape_analysis_FIX.o \
	silk/fixed/prefilter_FIX.o silk/fixed/process_gains_FIX.o \
	silk/fixed/regularize_correlations_FIX.o \
	silk/fixed/residual_energy16_FIX.o \
	silk/fixed/residual_energy_FIX.o silk/fixed/solve_LS_FIX.o \
	silk/fixed/warped_autocorrelation_FIX.o \
	silk/fixed/apply_sine_window_FIX.o silk/fixed/autocorr_FIX.o \
	silk/fixed/burg_modified_FIX.o silk/fixed/k2a_FIX.o \
	silk/fixed/k2a_Q16_FIX.o silk/fixed/pitch_analysis_core_FIX.o \
	silk/fixed/vector_ops_FIX.o silk/fixed/schur64_FIX.o \
	silk/fixed/schur_FIX.o silk/float/apply_sine_window_FLP.o \
	silk/float/corrMatrix_FLP.o silk/float/encode_frame_FLP.o \
	silk/float/find_LPC_FLP.o silk/float/find_LTP_FLP.o \
	silk/float/find_pitch_lags_FLP.o \
	silk/float/find_pred_coefs_FLP.o \
	silk/float/LPC_analysis_filter_FLP.o \
	silk/float/LTP_analysis_filter_FLP.o \
	silk/float/LTP_scale_ctrl_FLP.o \
	silk/float/noise_shape_analysis_FLP.o \
	silk/float/prefilter_FLP.o silk/float/process_gains_FLP.o \
	silk/float/regularize_correlations_FLP.o \
	silk/float/residual_energy_FLP.o silk/float/solve_LS_FLP.o \
	silk/float/warped_autocorrelation_FLP.o \
	silk/float/wrappers_FLP.o silk/float/autocorrelation_FLP.o \
	silk/float/burg_modified_FLP.o silk/float/bwexpander_FLP.o \
	silk/float/energy_FLP.o silk/float/inner_product_FLP.o \
	silk/float/k2a_FLP.o silk/float/levinsondurbin_FLP.o \
	silk/float/LPC_inv_pred_gain_FLP.o \
	silk/float/pitch_analysis_core_FLP.o \
	silk/float/scale_copy_vector_FLP.o \
	silk/float/scale_vector_FLP.o silk/float/schur_FLP.o \
	silk/float/sort_FLP.o src/opus.o src/opus_decoder.o \
	src/opus_encoder.o src/opus_multistream.o \
	src/opus_multistream_encoder.o src/opus_multistream_decoder.o \
	src/repacketizer.o src/analysis.o src/mlp.o src/mlp_data.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

all: libopus.a
libopus.a: $(LOBJECTS)
	/bin/rm -f libopus.a
	ar cr libopus.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libopus.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libopus.a; \
	else exit 0; fi; fi



clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)

# DO NOT DELETE
