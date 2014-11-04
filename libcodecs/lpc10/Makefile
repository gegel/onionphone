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

OBJ = analys.o \
      bsynz.o \
      channel.o \
      dcbias.o \
      decode.o \
      deemp.o \
      difmag.o \
      dyptrk.o \
      encode.o \
      energy.o \
      filter.o \
      ham84.o \
      hp100.o \
      initial.o \
      invert.o \
      irc2pc.o \
      ivfilt.o \
      lpc10.o \
      lpc_both.o \
      lpfilt31.o \
      median.o \
      mload.o \
      onset.o \
      pitsyn.o \
      placea.o \
      placev.o \
      preemp.o \
      random.o \
      rcchk.o \
      round.o \
      setup.o \
      synths.o \
      tbdm.o \
      voice.o \
      voice_nn.o \
      vparms.o

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

