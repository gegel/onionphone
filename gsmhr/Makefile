#CFLAGS = -Wall -O4 

#CFLAGS=-fomit-frame-pointer -ffast-math -funroll-loops
CFLAGS = -Wall -fomit-frame-pointer -ffast-math -funroll-loops -O4

PROGRAM = gsm_hr
 
OBJECTS = dtx.o \
	err_conc.o \
	globdefs.o \
	homing.o \
	host.o \
	mathdp31.o \
	mathhalf.o \
	sp_dec.o \
	sp_enc.o \
	sp_frm.o \
	sp_rom.o \
	sp_sfrm.o \
	vad.o 

LOBJECTS = dtx.o \
	err_conc.o \
	globdefs.o \
	homing.o \
	host.o \
	mathdp31.o \
	mathhalf.o \
	sp_dec.o \
	sp_enc.o \
	sp_frm.o \
	sp_rom.o \
	sp_sfrm.o \
	vad.o \
	libgsmhr.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libgsmhr.a: $(LOBJECTS)
	/bin/rm -f libgsmhr.a
	ar cr libgsmhr.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libgsmhr.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libgsmhr.a; \
	else exit 0; fi; fi

tester: libgsmhr.a gsm_hr.o
	cc -o gsm_hr gsm_hr.o libgsmhr.a $(LIBS)

$(PROGRAM): $(OBJECTS) $(ULIBS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(ULIBS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)

# DO NOT DELETE
