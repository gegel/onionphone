BLOB = builtin.o
CC ?= cc
LD ?= ld

# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3
MODE_DEBUG ?= -O0 -g -ggdb

# -DLINUX_ALSA -DM_LITTLE_ENDIAN -D_GNU_SOURCE
EXTRADEFS = -DLINUX_ALSA -DM_LITTLE_ENDIAN -D_GNU_SOURCE

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

OBJ = audio.o \
      cntrls.o \
      codecs.o \
      crypto.o \
      oph.o \
      sha1.o \
      tcp.o

INCADD ?= -I. -I../common/crp -I../common/inc -I../common/helpers -I../common/kiss_fft

all: build

build: $(BLOB)

$(BLOB): $(OBJ)
	$(LD) -r $(OBJ) -o $(BLOB)

audio.o: audio.c audio_wave.c audio_alsa.c

audio.c: audio.h

audio_wave.c: audio_wave.h

audio_alsa.c: audio_alsa.h

%.o: %.c
	$(CC) $(CFLAGS) $(INCADD) -c $<

clean:
	rm -f $(OBJ) $(BLOB)

test:
	cppcheck --enable=all .
	scan-build $(MAKE)

.PHONY: all clean test

