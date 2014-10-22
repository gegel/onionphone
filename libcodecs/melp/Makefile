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

OBJ = coeff.o \
      dsp_sub.o \
      fec_code.o \
      fs_lib.o \
      fsvq_cb.o \
      lpc_lib.o \
      mat_lib.o \
      melp.o \
      melp_ana.o \
      melp_chn.o \
      melp_sub.o \
      melp_syn.o \
      msvq_cb.o \
      pit_lib.o \
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

