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

OBJ = bands.o \
      celt.o \
      celt_encoder.o \
      celt_decoder.o \
      cwrs.o \
      entcode.o \
      entdec.o \
      entenc.o \
      kiss_fft.o \
      laplace.o \
      mathops.o \
      mdct.o \
      modes.o \
      pitch.o \
      celt_lpc.o \
      quant_bands.o \
      rate.o \
      vq.o 

INCADD ?= -I. -I../include -I../../../common/inc

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

