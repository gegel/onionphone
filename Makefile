      
#   Make file for OnionPhone
#   Tested with gcc 4.4.3 under Ubuntu 10.04 (requires libsound2-dev)
#   and MinGW (gcc 4.8.1, make 3.81) under Windows XP SP3


# Debugging options  
DEBUG = -O -DHEXDUMP

#Full duplex:
CCFLAGS =  -DAUDIO_BLOCKING -DLINUX -DM_LITTLE_ENDIAN -DNEEDED_LINEAR -DLINUX_DSP_SMALL_BUFFER -DHAVE_DEV_RANDOM

CC = gcc -Wall # for GNU's gcc compiler
CELPFLAGS = -fomit-frame-pointer -ffast-math -funroll-loops
LFLAGS = -lm
CCFLAGS = -DLINUX_ALSA -DM_LITTLE_ENDIAN
SOUNDLIB = -lasound 

#   Compiler flags

CFLAGS = $(DEBUG) $(PKOPTS) -Imelp -Icelp -Imelpe -Igsm/inc -Ilpc10 -Ilpc -Igsmhr -Ig729 -Ig723 -Iilbc -Ibv -Igsmer -Isilk -Iamr -Icrp -Ispeex -Ispeex/speex -Iopus $(CARGS) $(DUPLEX) $(CCFLAGS) $(DOMAIN)

BINARIES = oph addkey

PROGRAMS = $(BINARIES) $(SCRIPTS)

DIRS = celp gsm lpc10 melpe melp codec2 lpc gsmhr g729 g723 ilbc bv gsmer silk amr crp speex opus

all:	$(PROGRAMS)

SPKROBJS = oph.o audio.o codecs.o cntrls.o crypto.o tcp.o
KEYOBJS = addkey.o

#Link

ifdef SYSTEMROOT
#Win32    
oph: $(SPKROBJS) celplib.o lpc10lib.o gsmlib.o melpelib.o melplib.o cd2lib.o lpclib.o gsmhrlib.o g729lib.o g723lib.o ilbclib.o bv16lib.o gsmerlib.o silklib.o amrlib.o crplib.o speexlib.o opuslib.o
	$(CC) $(SPKROBJS)  speex/libspeex.a speex/libspeexdsp.a celp/celp.o lpc10/liblpc10.a gsm/lib/libgsm.a melpe/libmelpe.a melp/libmelp.a opus/libopus.a lpc/lpc.o  gsmhr/libgsmhr.a g729/libg729.a g723/libg723.a ilbc/libilbc.a bv/libbv16.a gsmer/libgsme.a codec2/libcd2.a silk/libsilk.a amr/builtin.o crp/libcrp.a $(LFLAGS) -lcomctl32 -lwinmm -lws2_32 -o oph
else
   ifeq ($(shell uname), Linux)
#Linux      
oph: $(SPKROBJS) celplib.o lpc10lib.o gsmlib.o melpelib.o melplib.o cd2lib.o lpclib.o gsmhrlib.o g729lib.o g723lib.o ilbclib.o bv16lib.o gsmerlib.o silklib.o amrlib.o crplib.o speexlib.o opuslib.o
	$(CC) $(SPKROBJS)  speex/libspeex.a speex/libspeexdsp.a celp/celp.o lpc10/liblpc10.a gsm/lib/libgsm.a melpe/libmelpe.a melp/libmelp.a opus/libopus.a lpc/lpc.o  gsmhr/libgsmhr.a g729/libg729.a g723/libg723.a ilbc/libilbc.a bv/libbv16.a gsmer/libgsme.a codec2/libcd2.a silk/libsilk.a amr/builtin.o crp/libcrp.a $(LFLAGS) $(SOUNDLIB) -o oph
   endif
endif

addkey: $(KEYOBJS)
	$(CC) $(KEYOBJS) crp/libcrp.a $(LFLAGS) -o addkey


#	Compression and encryption libraries.  Each of these creates
#	a place-holder .o file in the main directory (which is not
#	an actual object file, simply a place to hang a time and
#	date stamp) to mark whether the library has been built.
#	Note that if you actually modify a library you'll need to
#	delete the place-holder or manually make within the library
#	directory.  This is tacky but it avoids visiting all the
#	library directories on every build and/or relying on features
#	in make not necessarily available on all platforms.


celplib.o:
	( echo "Building CELP library."; cd celp ; make CC="$(CC) $(CCFLAGS) $(DEBUG) $(CELPFLAGS)" )
	echo "CELP" >celplib.o

lpc10lib.o:
	( echo "Building LPC10 library."; cd lpc10 ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "LPC" >lpc10lib.o
 
gsmlib.o:
	( echo "Building GSM library."; cd gsm ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "GSM" >gsmlib.o

melpelib.o:
	( echo "Building MELPE library."; cd melpe ; make CC="$(CC) $(CCFLAGS) $(DEBUG) $(CELPFLAGS)" )
	echo "MELPE" >melpelib.o

melplib.o:
	( echo "Building MELP library."; cd melp ; make CC="$(CC) $(CCFLAGS) $(DEBUG) $(CELPFLAGS)" )
	echo "MELPE" >melplib.o

cd2lib.o:
	( echo "Building CODEC2 library."; cd codec2 ; make CC="$(CC) $(CCFLAGS) $(DEBUG) $(CELPFLAGS)" )
	echo "MELPE" >cd2lib.o

lpclib.o:
	( echo "Building LPC library."; cd lpc ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "LPC" >lpclib.o

gsmhrlib.o:
	( echo "Building GSM_HR library."; cd gsmhr ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "GSM_HR" >gsmhrlib.o

g729lib.o:
	( echo "Building G729 library."; cd g729 ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "G729" >g729lib.o

g723lib.o:
	( echo "Building G723 library."; cd g723 ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "G723" >g723lib.o

ilbclib.o:
	( echo "Building ILBC library."; cd ilbc ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "ILBC" >ilbclib.o

bv16lib.o:
	( echo "Building BV16 library."; cd bv ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "BV16" >bv16lib.o

gsmerlib.o:
	( echo "Building GSM_ER library."; cd gsmer ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "GSM_ER" >gsmerlib.o

silklib.o:
	( echo "Building SILK library."; cd silk ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "SILK" >silklib.o

amrlib.o:
	( echo "Building AMR library."; cd amr ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "AMR" >amrlib.o

speexlib.o:
	( echo "Building SPEEX library."; cd speex ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "SPEEX" >speexlib.o

opuslib.o:
	( echo "Building OPUS library."; cd opus ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "OPUS" >opuslib.o

crplib.o:
	( echo "Building CRP library."; cd crp ; make CC="$(CC) $(CCFLAGS) $(DEBUG)" )
	echo "CRP" >crplib.o

#   Object file dependencies

audio_alsa.o: Makefile audio_alsa.c

codecs.o: Makefile codecs.c

cntrls.o: Makefile cntrls.c

crypto.o: Makefile crypto.c

tcp.o: Makefile tcp.c

oph.o: oph.c

addkey.o: addkey.c

testgsm:    testgsm.o gsmlib.o
	$(CC) testgsm.o -lm gsm/lib/libgsm.a $(LFLAGS) -o testgsm


#	Clean everything

clean:
	find . -name Makefile.bak -exec rm {} \;
	rm -f core *.out *.o *.bak $(PROGRAMS) *.shar *.exe *.a
	@for I in $(DIRS); \
	  do (cd $$I; echo "==>Entering directory `pwd`"; $(MAKE) $@ || exit 1); done
	

# DO NOT DELETE
