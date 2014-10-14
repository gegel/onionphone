CFLAGS = -std=c99 -Wall -Wextra -pedantic -fomit-frame-pointer -ffast-math -funroll-loops -O3

#CFLAGS = -Wall -O4 

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops


PROGRAM = melp_tst

OBJECTS = coeff.o \
		dsp_sub.o \
		fec_code.o \
		fs_lib.o \
		fsvq_cb.o \
		lpc_lib.o \
		mat_lib.o \
		melp_tst.o \
		melp_ana.o \
		melp_chn.o \
		melp_sub.o \
		melp_syn.o \
		msvq_cb.o \
		pit_lib.o \
		vq_lib.o \
		
LOBJECTS = coeff.o \
		dsp_sub.o \
		fec_code.o \
		fs_lib.o \
		fsvq_cb.o \
		lpc_lib.o \
		mat_lib.o \
		melp.o \
		melp_ana.o \
		melp_chn.o \
		melp_sub.o \
		melp_syn.o \
		msvq_cb.o \
		pit_lib.o \
		vq_lib.o \


LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libmelp.a: $(LOBJECTS)
	/bin/rm -f libmelp.a
	ar cr libmelp.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libmelp.a; \
      else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libmelp.a; \
	else exit 0; fi; fi

$(PROGRAM): $(OBJECTS) $(ULIBS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJECTS) $(ULIBS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)


# DO NOT DELETE
