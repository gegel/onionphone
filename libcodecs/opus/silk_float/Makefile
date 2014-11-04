BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# -DHAVE_CONFIG_H
EXTRADEFS = -DHAVE_CONFIG_H

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = apply_sine_window_FLP.o \
      corrMatrix_FLP.o \
      encode_frame_FLP.o \
      find_LPC_FLP.o \
      find_LTP_FLP.o \
      find_pitch_lags_FLP.o \
      find_pred_coefs_FLP.o \
      LPC_analysis_filter_FLP.o \
      LTP_analysis_filter_FLP.o \
      LTP_scale_ctrl_FLP.o \
      noise_shape_analysis_FLP.o \
      prefilter_FLP.o \
      process_gains_FLP.o \
      regularize_correlations_FLP.o \
      residual_energy_FLP.o \
      solve_LS_FLP.o \
      warped_autocorrelation_FLP.o \
      wrappers_FLP.o \
      autocorrelation_FLP.o \
      burg_modified_FLP.o \
      bwexpander_FLP.o \
      energy_FLP.o \
      inner_product_FLP.o \
      k2a_FLP.o \
      levinsondurbin_FLP.o \
      LPC_inv_pred_gain_FLP.o \
      pitch_analysis_core_FLP.o \
      scale_copy_vector_FLP.o \
      scale_vector_FLP.o \
      schur_FLP.o \
      sort_FLP.o

INCADD ?= -I. -I../include -I../silk_common -I../celt -I../../../common/inc

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

