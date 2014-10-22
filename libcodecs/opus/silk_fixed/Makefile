BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# -DHAVE_CONFIG_H
EXTRADEFS = -DHAVE_CONFIG_H

CFLAGS ?= -std=c99 -Wall -Wextra -pedantic $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = LTP_analysis_filter_FIX.o \
      LTP_scale_ctrl_FIX.o \
      corrMatrix_FIX.o \
      encode_frame_FIX.o \
      find_LPC_FIX.o \
      find_LTP_FIX.o \
      find_pitch_lags_FIX.o \
      find_pred_coefs_FIX.o \
      noise_shape_analysis_FIX.o \
      prefilter_FIX.o \
      process_gains_FIX.o \
      regularize_correlations_FIX.o \
      residual_energy16_FIX.o \
      residual_energy_FIX.o \
      solve_LS_FIX.o \
      warped_autocorrelation_FIX.o \
      apply_sine_window_FIX.o \
      autocorr_FIX.o \
      burg_modified_FIX.o \
      k2a_FIX.o \
      k2a_Q16_FIX.o \
      pitch_analysis_core_FIX.o \
      vector_ops_FIX.o \
      schur64_FIX.o \
      schur_FIX.o

INCADD ?= -I. -I../include -I../silk_common -I../celt

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

