BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

#
EXTRADEFS =

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = ampexp.o \
      codebook.o \
      codebookd.o \
      codebookdt.o \
      codebookge.o \
      codebookjnd.o \
      codebookjvm.o \
      codebookres.o \
      codebookvq.o \
      codebookvqanssi.o \
      codec2.o \
      dump.o \
      fdmdv.o \
      fifo.o \
      globals.o \
      glottal.o \
      golay23.o \
      interp.o \
      kiss_fft.o \
      lpc.o \
      lsp.o \
      nlp.o \
      octave.o \
      pack.o \
      phase.o \
      phaseexp.o \
      postfilter.o \
      quantise.o \
      sine.o

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

