BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# -DHAVE_CONFIG_H -DFIXED_POINT
EXTRADEFS = -DHAVE_CONFIG_H -DFIXED_POINT

CFLAGS ?= -std=c99 -Wall -Wextra -pedantic $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = opus.o \
      opus_compare.o \
      opus_decoder.o \
      opus_encoder.o \
      opus_multistream.o \
      opus_multistream_encoder.o \
      opus_multistream_decoder.o \
      repacketizer.o \
      analysis.o \
      mlp.o \
      mlp_data.o

INCADD ?= -I. -I../include -I../celt -I../silk_common -I../silk_fixed -I../silk_float -I../../../common/inc

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

