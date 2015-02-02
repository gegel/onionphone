BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

#
EXTRADEFS = -DFLOATING_POINT -DUSE_KISS_FFT

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = fftwrap.o \
	kiss_fft.o \
	kiss_fftr.o \
	smallft.o

INCADD ?= -I.

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

