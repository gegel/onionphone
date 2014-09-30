CROSS :=

CC = $(CROSS)cc
AS = $(CC)
AR = $(CROSS)ar

CC_OPT=-fomit-frame-pointer -ffast-math -funroll-loops -O4

#CFLAGS   = -Wall -pg -g -O4

ARFLAGS  := rc

BV16 := .

LIBBV16 = libbv16.a

all: $(LIBBV16)

LIBBV16_SRC = \
	$(BV16)/bvcommon/a2lsp.o \
	$(BV16)/bvcommon/allpole.o \
	$(BV16)/bvcommon/allzero.o  \
	$(BV16)/bvcommon/autocor.o \
	$(BV16)/bvcommon/basop32.o \
	$(BV16)/bvcommon/cmtables.o \
	$(BV16)/bvcommon/levdur.o \
	$(BV16)/bvcommon/lsp2a.o \
	$(BV16)/bvcommon/mathtables.o \
	$(BV16)/bvcommon/mathutil.o \
	$(BV16)/bvcommon/memutil.o \
	$(BV16)/bvcommon/ptdec.o \
	$(BV16)/bvcommon/stblzlsp.o \
	$(BV16)/bvcommon/utility.o \
	$(BV16)/bvcommon/vqdecode.o \
	$(BV16)/bv16/bitpack.o \
	$(BV16)/bv16/coarptch.o \
	$(BV16)/bv16/decoder.o \
	$(BV16)/bv16/encoder.o \
	$(BV16)/bv16/excdec.o \
	$(BV16)/bv16/excquan.o \
	$(BV16)/bv16/fineptch.o \
	$(BV16)/bv16/g192.o \
	$(BV16)/bv16/gaindec.o \
	$(BV16)/bv16/gainquan.o \
	$(BV16)/bv16/levelest.o \
	$(BV16)/bv16/lspdec.o \
	$(BV16)/bv16/lspquan.o \
	$(BV16)/bv16/plc.o \
	$(BV16)/bv16/postfilt.o \
	$(BV16)/bv16/preproc.o \
	$(BV16)/bv16/ptquan.o \
	$(BV16)/bv16/tables.o


LIBBV16_OBJS = $(LIBBV16_SRC:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(AS) $(CC_OPT) -c -o $@ $^
%.a:
	$(AR) $(ARFLAGS) $@ $^


$(LIBBV16): $(LIBBV16_OBJS)

clean:
	rm -f *.o bv16/*.o bvcommon/*.o $(LIBBV16) 

.PHONY: all clean
