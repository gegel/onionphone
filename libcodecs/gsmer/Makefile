BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

#
EXTRADEFS =

CFLAGS ?= -std=c99 -Wall -Wextra -pedantic $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = agc.o \
      autocorr.o \
      az_lsp.o \
      basicop2.o \
      bits2prm.o \
      c1035pf.o \
      cod_12k2.o \
      convolve.o \
      copy.o \
      count.o \
      d1035pf.o \
      d_gains.o \
      d_homing.o \
      d_plsf_5.o \
      dec_12k2.o \
      dec_lag6.o \
      dtx.o \
      e_homing.o \
      enc_lag6.o \
      g_code.o \
      g_pitch.o \
      int_lpc.o \
      inter_6.o \
      inv_sqrt.o \
      lag_wind.o \
      levinson.o \
      libgsme.o \
      log2.o \
      lsp_az.o \
      lsp_lsf.o \
      n_proc.o \
      oper_32b.o \
      pitch_f6.o \
      pitch_ol.o \
      pow2.o \
      pre_proc.o \
      pred_lt6.o \
      preemph.o \
      prm2bits.o \
      pstfilt2.o \
      q_gains.o \
      q_plsf_5.o \
      reorder.o \
      residu.o \
      set_zero.o \
      syn_filt.o \
      vad.o \
      weight_a.o

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

