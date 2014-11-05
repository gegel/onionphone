BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3 -ffast-math -funroll-loops
MODE_DEBUG ?= -O0 -g -ggdb

# -DFLOATING_POINT -DFIXED_POINT
EXTRADEFS = -DFLOATING_POINT

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = bits.o \
      buffer.o \
      cb_search.o \
      exc_10_16_table.o \
      exc_10_32_table.o \
      exc_20_32_table.o \
      exc_5_256_table.o \
      exc_5_64_table.o \
      exc_8_128_table.o \
      fftwrap.o \
      filterbank.o \
      filters.o \
      gain_table.o \
      gain_table_lbr.o \
      hexc_10_32_table.o \
      hexc_table.o \
      high_lsp_tables.o \
      jitter.o \
      kiss_fft.o \
      kiss_fftr.o \
      lpc.o \
      lsp.o \
      lsp_tables_nb.o \
      ltp.o \
      mdf.o \
      modes.o \
      modes_wb.o \
      nb_celp.o \
      preprocess.o \
      quant_lsp.o \
      resample.o \
      sb_celp.o \
      smallft.o \
      speex.o \
      speex_callbacks.o \
      speex_header.o \
      stereo.o \
      vbr.o \
      vq.o \
      window.o

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

