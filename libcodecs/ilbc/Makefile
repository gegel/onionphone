BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O2
MODE_DEBUG ?= -O0 -g -ggdb

#
EXTRADEFS =

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = abs_quant.o \
      abs_quant_loop.o \
      augmented_cb_corr.o \
      auto_corr_to_refl_coef.o \
      auto_correlation.o \
      bw_expand.o \
      cb_construct.o \
      cb_mem_energy.o \
      cb_mem_energy_augmentation.o \
      cb_mem_energy_calc.o \
      cb_search.o \
      cb_search_core.o \
      cb_update_best_index.o \
      chebyshev.o \
      comp_corr.o \
      complex_fft.o \
      constants.o \
      copy_set_operations.o \
      create_augmented_vec.o \
      cross_correlation.o \
      decode.o \
      decode_residual.o \
      decoder_interpolate_lsf.o \
      division_operations.o \
      do_plc.o \
      dot_product_with_scale.o \
      downsample_fast.o \
      encode.o \
      energy.o \
      energy_inverse.o \
      enh_upsample.o \
      enhancer.o \
      enhancer_interface.o \
      filter_ar.o \
      filter_ar_fast_q12.o \
      filter_ma_fast_q12.o \
      filtered_cb_vecs.o \
      frame_classify.o \
      gain_dequant.o \
      gain_quant.o \
      get_cd_vec.o \
      get_hanning_window.o \
      get_lsp_poly.o \
      get_scaling_square.o \
      get_sync_seq.o \
      hp_input.o \
      hp_output.o \
      ilbc.o \
      ilbc_specific_functions.o \
      index_conv_dec.o \
      index_conv_enc.o \
      init_decode.o \
      init_encode.o \
      interpolate.o \
      interpolate_samples.o \
      levinson_durbin.o \
      lpc_encode.o \
      lpc_to_refl_coef.o \
      lsf_check.o \
      lsf_interpolate_to_poly_dec.o \
      lsf_interpolate_to_poly_enc.o \
      lsf_to_lsp.o \
      lsf_to_poly.o \
      lsp_to_lsf.o \
      min_max_operations.o \
      my_corr.o \
      nearest_neighbor.o \
      pack_bits.o \
      poly_to_lsf.o \
      poly_to_lsp.o \
      randomization_functions.o \
      refiner.o \
      refl_coef_to_lpc.o \
      resample.o \
      resample_48khz.o \
      resample_by_2.o \
      resample_by_2_internal.o \
      resample_fractional.o \
      simple_interpolate_lsf.o \
      simple_lpc_analysis.o \
      simple_lsf_dequant.o \
      simple_lsf_quant.o \
      smooth.o \
      smooth_out_data.o \
      sort_sq.o \
      spl_sqrt.o \
      spl_sqrt_floor.o \
      spl_version.o \
      split_vq.o \
      splitting_filter.o \
      sqrt_of_one_minus_x_squared.o \
      state_construct.o \
      state_search.o \
      swap_bytes.o \
      unpack_bits.o \
      vector_scaling_operations.o \
      vq3.o \
      vq4.o \
      window32_w32.o \
      xcorr_coef.o

INCADD ?= -I. -I../../common/inc

all: build

build: $(BLOB)

$(BLOB): $(OBJ)
	$(LD) -r $(OBJ) -o $(BLOB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCADD) -c $<

clean:
	rm -f $(OBJ) $(BLOB)

test:
	cppcheck --enable=all .
	scan-build $(MAKE)

.PHONY: all clean test

