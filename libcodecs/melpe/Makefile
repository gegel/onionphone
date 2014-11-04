BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3 -ffast-math -funroll-loops
MODE_DEBUG ?= -O0 -g -ggdb

#
EXTRADEFS =

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = classify.o \
      coeff.o \
      dsp_sub.o \
      fec_code.o \
      fft_lib.o \
      fs_lib.o \
      fsvq_cb.o \
      global.o \
      harm.o \
      lpc_lib.o \
      mat_lib.o \
      math_lib.o \
      mathdp31.o \
      mathhalf.o \
      melp_ana.o \
      melp_chn.o \
      melp_sub.o \
      melp_syn.o \
      melpe.o \
      msvq_cb.o \
      npp.o \
      pit_lib.o \
      pitch.o \
      postfilt.o \
      qnt12.o \
      qnt12_cb.o \
      transcode.o \
      vq_lib.o

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

