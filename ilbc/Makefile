CROSS :=

CC = $(CROSS)cc
AS = $(CC)
AR = $(CROSS)ar

CC_OPT=-fomit-frame-pointer -ffast-math -funroll-loops -O4

#CFLAGS   = -Wall -pg -g -O2

CFLAGS = -Wall -fomit-frame-pointer -ffast-math -funroll-loops -O4

ARFLAGS  := rc


LIBILBC = libilbc.a

all: $(LIBILBC)

LIBILBC_SRC = \
    abs_quant.c \
    abs_quant_loop.c \
    augmented_cb_corr.c \
    bw_expand.c \
    cb_construct.c \
    cb_mem_energy.c \
    cb_mem_energy_augmentation.c \
    cb_mem_energy_calc.c \
    cb_search.c \
    cb_search_core.c \
    cb_update_best_index.c \
    chebyshev.c \
    comp_corr.c \
    constants.c \
    create_augmented_vec.c \
    decode.c \
    decode_residual.c \
    decoder_interpolate_lsf.c \
    do_plc.c \
    encode.c \
    energy_inverse.c \
    enh_upsample.c \
    enhancer.c \
    enhancer_interface.c \
    filtered_cb_vecs.c \
    frame_classify.c \
    gain_dequant.c \
    gain_quant.c \
    get_cd_vec.c \
    get_lsp_poly.c \
    get_sync_seq.c \
    hp_input.c \
    hp_output.c \
    ilbc.c \
    index_conv_dec.c \
    index_conv_enc.c \
    init_decode.c \
    init_encode.c \
    interpolate.c \
    interpolate_samples.c \
    lpc_encode.c \
    lsf_check.c \
    lsf_interpolate_to_poly_dec.c \
    lsf_interpolate_to_poly_enc.c \
    lsf_to_lsp.c \
    lsf_to_poly.c \
    lsp_to_lsf.c \
    my_corr.c \
    nearest_neighbor.c \
    pack_bits.c \
    poly_to_lsf.c \
    poly_to_lsp.c \
    refiner.c \
    simple_interpolate_lsf.c \
    simple_lpc_analysis.c \
    simple_lsf_dequant.c \
    simple_lsf_quant.c \
    smooth.c \
    smooth_out_data.c \
    sort_sq.c \
    split_vq.c \
    state_construct.c \
    state_search.c \
    swap_bytes.c \
    unpack_bits.c \
    vq3.c \
    vq4.c \
    window32_w32.c \
    xcorr_coef.c \
    auto_corr_to_refl_coef.c \
    auto_correlation.c \
    complex_fft.c \
    copy_set_operations.c \
    cross_correlation.c \
    division_operations.c \
    dot_product_with_scale.c \
    downsample_fast.c \
    energy.c \
    filter_ar.c \
    filter_ar_fast_q12.c \
    filter_ma_fast_q12.c \
    get_hanning_window.c \
    get_scaling_square.c \
    ilbc_specific_functions.c \
    levinson_durbin.c \
    lpc_to_refl_coef.c \
    min_max_operations.c \
    randomization_functions.c \
    refl_coef_to_lpc.c \
    resample.c \
    resample_48khz.c \
    resample_by_2.c \
    resample_by_2_internal.c \
    resample_fractional.c \
    spl_sqrt.c \
    spl_sqrt_floor.c \
    spl_version.c \
    splitting_filter.c \
    sqrt_of_one_minus_x_squared.c \
    vector_scaling_operations.c


LIBILBC_OBJS = $(LIBILBC_SRC:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(AS) $(CC_OPT) -c -o $@ $^
%.a:
	$(AR) $(ARFLAGS) $@ $^


$(LIBILBC): $(LIBILBC_OBJS)

clean:
	rm -f *.o $(LIBILBC) 

.PHONY: all clean
