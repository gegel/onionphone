# -DLINUX_ALSA -DM_LITTLE_ENDIAN -D_GNU_SOURCE
EXTRADEFS = -DLINUX_ALSA -DM_LITTLE_ENDIAN -D_GNU_SOURCE
INCADD ?= -I. -I../common/crp -I../common/inc -I../common/helpers -I../common/kiss_fft

include ../Makefile-common.inc
include ../Makefile-leaf.inc

audio.o: audio.c audio_wave.c audio_alsa.c

audio.c: audio.h

audio_wave.c: audio_wave.h

audio_alsa.c: audio_alsa.h

