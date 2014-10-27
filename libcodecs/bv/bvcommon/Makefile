BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# none
EXTRADEFS =

CFLAGS ?= -std=c99 -Wall -Wextra -pedantic $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = a2lsp.o \
      allpole.o \
      allzero.o \
      autocor.o \
      basop32.o \
      cmtables.o \
      levdur.o \
      lsp2a.o \
      mathtables.o \
      mathutil.o \
      ptdec.o \
      stblzlsp.o \
      utility.o \
      vqdecode.o

INCADD ?= -I. -I../../../common/inc

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
