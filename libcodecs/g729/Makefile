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

OBJ = acelp_cp.o \
      bitscp.o \
      bwfw.o \
      bwfwfunc.o \
      calcexc.o \
      codld8cp.o \
      cor_func.o \
      dacelpcp.o \
      dec_sid.o \
      declagcp.o \
      decld8cp.o \
      degaincp.o \
      dtx.o \
      filtere.o \
      gainpred.o \
      libg729.o \
      lpccp.o \
      lpfunccp.o \
      lspdece.o \
      lspgetqe.o \
      mus_dtct.o \
      p_parity.o \
      phdisp.o \
      pitchcp.o \
      post_pro.o \
      pre_proc.o \
      pred_lt3.o \
      pstcp.o \
      pwf.o \
      pwfe.o \
      q_gaincp.o \
      qsidgain.o \
      qsidlsf.o \
      qua_lspe.o \
      tab_dtx.o \
      tab_ld8k.o \
      tabld8cp.o \
      taming.o \
      track_pi.o \
      utilcp.o \
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

