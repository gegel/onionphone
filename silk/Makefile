CFLAGS = -std=c99 -Wall -Wextra -pedantic -fomit-frame-pointer -ffast-math -funroll-loops -O3

#CFLAGS = -Wall -O4 

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops


LOBJECTS = SKP_Silk_A2NLSF.o \
	SKP_Silk_allpass_int.o \
	SKP_Silk_ana_filt_bank_1.o \
	SKP_Silk_apply_sine_window.o \
	SKP_Silk_array_maxabs.o \
	SKP_Silk_autocorr.o \
	SKP_Silk_biquad.o \
	SKP_Silk_biquad_alt.o \
	SKP_Silk_burg_modified.o \
	SKP_Silk_bwexpander.o \
	SKP_Silk_bwexpander_32.o \
	SKP_Silk_CNG.o \
	SKP_Silk_code_signs.o \
	SKP_Silk_control_codec_FIX.o \
	SKP_Silk_corrMatrix_FIX.o \
	SKP_Silk_create_init_destroy.o \
	SKP_Silk_dec_API.o \
	SKP_Silk_decode_core.o \
	SKP_Silk_decode_frame.o \
	SKP_Silk_decode_indices_v4.o \
	SKP_Silk_decode_parameters.o \
	SKP_Silk_decode_parameters_v4.o \
	SKP_Silk_decode_pulses.o \
	SKP_Silk_decoder_set_fs.o \
	SKP_Silk_detect_SWB_input.o \
	SKP_Silk_enc_API.o \
	SKP_Silk_encode_frame_FIX.o \
	SKP_Silk_encode_parameters.o \
	SKP_Silk_encode_parameters_v4.o \
	SKP_Silk_encode_pulses.o \
	SKP_Silk_find_LPC_FIX.o \
	SKP_Silk_find_LTP_FIX.o \
	SKP_Silk_find_pitch_lags_FIX.o \
	SKP_Silk_find_pred_coefs_FIX.o \
	SKP_Silk_gain_quant.o \
	SKP_Silk_HP_variable_cutoff_FIX.o \
	SKP_Silk_init_encoder_FIX.o \
	SKP_Silk_inner_prod_aligned.o \
	SKP_Silk_interpolate.o \
	SKP_Silk_k2a.o \
	SKP_Silk_k2a_Q16.o \
	SKP_Silk_LBRR_reset.o \
	SKP_Silk_lin2log.o \
	SKP_Silk_log2lin.o \
	SKP_Silk_lowpass_int.o \
	SKP_Silk_lowpass_short.o \
	SKP_Silk_LP_variable_cutoff.o \
	SKP_Silk_LPC_inv_pred_gain.o \
	SKP_Silk_LPC_stabilize.o \
	SKP_Silk_LPC_synthesis_filter.o \
	SKP_Silk_LPC_synthesis_order16.o \
	SKP_Silk_LSF_cos_table.o \
	SKP_Silk_LTP_analysis_filter_FIX.o \
	SKP_Silk_LTP_scale_ctrl_FIX.o \
	SKP_Silk_MA.o \
	SKP_Silk_NLSF2A.o \
	SKP_Silk_NLSF2A_stable.o \
	SKP_Silk_NLSF_MSVQ_decode.o \
	SKP_Silk_NLSF_MSVQ_encode_FIX.o \
	SKP_Silk_NLSF_stabilize.o \
	SKP_Silk_NLSF_VQ_rate_distortion_FIX.o \
	SKP_Silk_NLSF_VQ_sum_error_FIX.o \
	SKP_Silk_NLSF_VQ_weights_laroia.o \
	SKP_Silk_noise_shape_analysis_FIX.o \
	SKP_Silk_NSQ.o \
	SKP_Silk_NSQ_del_dec.o \
	SKP_Silk_pitch_analysis_core.o \
	SKP_Silk_pitch_est_tables.o \
	SKP_Silk_PLC.o \
	SKP_Silk_prefilter_FIX.o \
	SKP_Silk_process_gains_FIX.o \
	SKP_Silk_process_NLSFs_FIX.o \
	SKP_Silk_pulses_to_bytes.o \
	SKP_Silk_quant_LTP_gains_FIX.o \
	SKP_Silk_range_coder.o \
	SKP_Silk_regularize_correlations_FIX.o \
	SKP_Silk_resample_1_2.o \
	SKP_Silk_resample_1_2_coarse.o \
	SKP_Silk_resample_1_2_coarsest.o \
	SKP_Silk_resample_1_3.o \
	SKP_Silk_resample_2_1_coarse.o \
	SKP_Silk_resample_2_3.o \
	SKP_Silk_resample_2_3_coarse.o \
	SKP_Silk_resample_2_3_coarsest.o \
	SKP_Silk_resample_2_3_rom.o \
	SKP_Silk_resample_3_1.o \
	SKP_Silk_resample_3_2.o \
	SKP_Silk_resample_3_2_rom.o \
	SKP_Silk_resample_3_4.o \
	SKP_Silk_resample_4_3.o \
	SKP_Silk_residual_energy16_FIX.o \
	SKP_Silk_residual_energy_FIX.o \
	SKP_Silk_scale_copy_vector16.o \
	SKP_Silk_scale_vector.o \
	SKP_Silk_schur.o \
	SKP_Silk_schur64.o \
	SKP_Silk_shell_coder.o \
	SKP_Silk_sigm_Q15.o \
	SKP_Silk_solve_LS_FIX.o \
	SKP_Silk_sort.o \
	SKP_Silk_sum_sqr_shift.o \
	SKP_Silk_tables_gain.o \
	SKP_Silk_tables_LTP.o \
	SKP_Silk_tables_NLSF_CB0_10.o \
	SKP_Silk_tables_NLSF_CB0_16.o \
	SKP_Silk_tables_NLSF_CB1_10.o \
	SKP_Silk_tables_NLSF_CB1_16.o \
	SKP_Silk_tables_other.o \
	SKP_Silk_tables_pitch_lag.o \
	SKP_Silk_tables_pulses_per_block.o \
	SKP_Silk_tables_sign.o \
	SKP_Silk_tables_type_offset.o \
	SKP_Silk_VAD.o \
	SKP_Silk_VQ_nearest_neighbor_FIX.o \
	libsilk.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libsilk.a: $(LOBJECTS)
	/bin/rm -f libsilk.a
	ar cr libsilk.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libsilk.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libsilk.a; \
	else exit 0; fi; fi


clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)

# DO NOT DELETE
