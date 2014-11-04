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

OBJ = dtx.o \
      err_conc.o \
      globdefs.o \
      homing.o \
      host.o \
      libgsmhr.o \
      mathdp31.o \
      mathhalf.o \
      sp_dec.o \
      sp_enc.o \
      sp_frm.o \
      sp_rom.o \
      sp_sfrm.o \
      vad.o

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

