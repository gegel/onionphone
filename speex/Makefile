#CFLAGS = -Wall -fomit-frame-pointer -ffast-math -funroll-loops -O4 -DFLOATING_POINT

CFLAGS = -Wall -O4 -DFLOATING_POINT

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops

LOBJECTS = cb_search.o exc_10_32_table.o \
	exc_8_128_table.o filters.o gain_table.o hexc_table.o \
	high_lsp_tables.o lsp.o ltp.o speex.o stereo.o vbr.o \
	vq.o bits.o exc_10_16_table.o exc_20_32_table.o \
	exc_5_256_table.o exc_5_64_table.o gain_table_lbr.o \
	hexc_10_32_table.o lpc.o lsp_tables_nb.o modes.o \
	modes_wb.o nb_celp.o quant_lsp.o sb_celp.o \
	speex_callbacks.o speex_header.o window.o
LDSPOBJECTS = preprocess.o smallft.o jitter.o mdf.o \
	fftwrap.o kiss_fft.o kiss_fftr.o filterbank.o resample.o \
	buffer.o


LSRC = $(LOBJECTS:.o=.c)
LDSPSRC = $(LDSPOBJECTS:.o=.c)

LIBS = -lm

all: libspeex.a libspeexdsp.a

libspeex.a: $(LOBJECTS)
	/bin/rm -f libspeex.a
	ar cr libspeex.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libspeex.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libspeex.a; \
	else exit 0; fi; fi

libspeexdsp.a: $(LDSPOBJECTS)
	/bin/rm -f libspeexdsp.a
	ar cr libspeexdsp.a $(LDSPOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libspeexdsp.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libspeexdsp.a; \
	else exit 0; fi; fi



clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(LDSPOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC) $(LDSPSRC)

# DO NOT DELETE
