CFLAGS = -Wall -fomit-frame-pointer -ffast-math -funroll-loops -O4

PROGRAM = lpc2400

OBJECTS = analys.o bitio.o bsynz.o channel.o dcbias.o decode.o deemp.o \
	  difmag.o diskio.o dyptrk.o encode.o energy.o filter.o ham84.o \
	  hp100.o  initial.o invert.o irc2pc.o ivfilt.o lpc_both.o \
	  lpfilt31.o median.o mload.o onset.o pitsyn.o placea.o \
	  placev.o preemp.o random.o rcchk.o round.o setup.o synths.o \
	  tbdm.o trans.o voice.o voice_nn.o vparms.o

LOBJECTS = analys.o bsynz.o channel.o dcbias.o decode.o deemp.o \
	   difmag.o dyptrk.o encode.o energy.o filter.o ham84.o \
	   hp100.o  initial.o invert.o irc2pc.o ivfilt.o lpc10.o \
	   lpfilt31.o median.o mload.o onset.o pitsyn.o placea.o \
	   placev.o preemp.o random.o round.o rcchk.o setup.o synths.o \
	   tbdm.o voice.o voice_nn.o vparms.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

liblpc10.a: $(LOBJECTS)
	/bin/rm -f liblpc10.a
	ar cr liblpc10.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib liblpc10.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib liblpc10.a; \
	else exit 0; fi; fi

tester: liblpc10.a main.o
	cc -o lpc2400 main.o liblpc10.a ../ulaw.o $(LIBS)

$(PROGRAM): $(OBJECTS) $(ULIBS)
	$(CC) -o $(PROGRAM) $(OBJECTS) ../ulaw.o $(ULIBS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)

test:	$(PROGRAM)
	time $(PROGRAM) test.spd test.out
	sfplay test.out
	rm test.out

suntest:$(PROGRAM)
	time $(PROGRAM) test.au test.out
	cp test.out /dev/audio
	rm test.out

# DO NOT DELETE
