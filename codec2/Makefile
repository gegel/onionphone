#CFLAGS = -Wall -O4 

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops

CFLAGS = -std=c99 -Wall -Wextra -pedantic -fomit-frame-pointer -ffast-math -funroll-loops -O3

PROGRAM = c2demo

OBJECTS = ampexp.o \
		codebook.o \
		codebookd.o \
		codebookdt.o \
		codebookge.o \
		codebookjnd.o \
		codebookjvm.o \
		codebookres.o \
		codebookvq.o \
		codebookvqanssi.o \
		codec2.o \
		dump.o \
		fdmdv.o \
		fifo.o \
		globals.o \
		glottal.o \
		golay23.o \
		interp.o \
		kiss_fft.o \
		lpc.o \
		lsp.o \
		nlp.o \
		octave.o \
		pack.o \
		phase.o \
		phaseexp.o \
		postfilter.o \
		quantise.o \
		sine.o \
		c2demo.o
		
LOBJECTS = ampexp.o \
		codebook.o \
		codebookd.o \
		codebookdt.o \
		codebookge.o \
		codebookjnd.o \
		codebookjvm.o \
		codebookres.o \
		codebookvq.o \
		codebookvqanssi.o \
		codec2.o \
		dump.o \
		fdmdv.o \
		fifo.o \
		globals.o \
		glottal.o \
		golay23.o \
		interp.o \
		kiss_fft.o \
		lpc.o \
		lsp.o \
		nlp.o \
		octave.o \
		pack.o \
		phase.o \
		phaseexp.o \
		postfilter.o \
		quantise.o \
		sine.o \

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libcd2.a: $(LOBJECTS)
	/bin/rm -f libcd2.a
	ar cr libcd2.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libcd2.a; \
      else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libcd2.a; \
	else exit 0; fi; fi


$(PROGRAM): $(OBJECTS) $(ULIBS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(ULIBS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)


# DO NOT DELETE
