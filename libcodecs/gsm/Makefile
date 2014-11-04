BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3 -ffast-math -funroll-loops
MODE_DEBUG ?= -O0 -g -ggdb

# -DSASR -DMULHACK -DFAST
EXTRADEFS = -DSASR

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = add.o \
      code.o \
      debug.o \
      decode.o \
      gsm_create.o \
      gsm_decode.o \
      gsm_destroy.o \
      gsm_encode.o \
      gsm_explode.o \
      gsm_implode.o \
      gsm_option.o \
      gsm_print.o \
      long_term.o \
      lpc.o \
      preprocess.o \
      rpe.o \
      short_term.o \
      table.o

INCADD ?= -I. -I./inc -I../../common/inc

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

