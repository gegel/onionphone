PROGRAM = sc1200

CFLAGS = -std=c99 -Wall -Wextra -pedantic -O3

OBJECTS = classify.o coeff.o dsp_sub.o fec_code.o fft_lib.o \
       fs_lib.o fsvq_cb.o global.o harm.o lpc_lib.o mathdp31.o \
       mathhalf.o math_lib.o mat_lib.o melp_ana.o melp_chn.o melp_sub.o \
       melp_syn.o msvq_cb.o npp.o pitch.o pit_lib.o postfilt.o qnt12.o \
       qnt12_cb.o transcode.o vq_lib.o sc1200.o

LOBJECTS = classify.o coeff.o dsp_sub.o fec_code.o fft_lib.o \
       fs_lib.o fsvq_cb.o global.o harm.o lpc_lib.o mathdp31.o \
       mathhalf.o math_lib.o mat_lib.o melp_ana.o melp_chn.o melp_sub.o \
       melp_syn.o msvq_cb.o npp.o pitch.o pit_lib.o postfilt.o qnt12.o \
       qnt12_cb.o transcode.o vq_lib.o melpe.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libmelpe.a: $(LOBJECTS)
	/bin/rm -f libmelpe.a
	ar cr libmelpe.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libmelpe.a; \
      else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libmelpe.a; \
	else exit 0; fi; fi

$(PROGRAM): $(OBJECTS) $(ULIBS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(ULIBS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)


# DO NOT DELETE
