BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3 -ffast-math -funroll-loops
MODE_DEBUG ?= -O0 -g -ggdb

# FIXED FLOAT
ARITHMETICS ?= FIXED

# -DHAVE_CONFIG_H -DFIXED_POINT
EXTRADEFS = -DHAVE_CONFIG_H -D$(ARITHMETICS)_POINT

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = opus.o \
      opus_decoder.o \
      opus_encoder.o \
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

