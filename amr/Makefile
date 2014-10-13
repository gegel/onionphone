PROG = libamr.a
CC ?= cc
AR ?= ar

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# -DVAD1 -DVAD2
EXTRADEFS = -DVAD1

CFLAGS ?= -std=c99 -Wall -Wextra -pedantic $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = amr_speech_importance.o \
      interf_dec.o \
      interf_enc.o \
      sp_dec.o \
      sp_enc.o

INCADD ?= -I.
LDADD ?= -lm

all: build

build: $(PROG)

$(PROG): $(OBJ)
	$(AR) cr $(PROG) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) $(INCADD) -c $<

clean:
	rm -f $(OBJ) $(PROG)

test:
	cppcheck --enable=all .
	scan-build $(MAKE)

.PHONY: all clean test
