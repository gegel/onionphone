
CC = gcc
MAKEFILENAME = makefile

MODE=NORM



CFLAGS_NORM  = 
CFLAGS_DEBUG = -g -DDEBUG
CFLAGS = -std=c99 -Wall -Wextra -pedantic -I. $(CFLAGS_$(MODE))
CFLAGSDEPEND = -MM $(CFLAGS)                    # for make depend

TMP=$(MODE:NORM=)
TMP2=$(TMP:DEBUG=_debug)


#
# source/object files
#
TESTER_SRCS=curve.c sponge.c sprng.c havege.c timing.c b64.c wordlist.c test.c
LIBCRP_SRCS=curve.c sponge.c sprng.c havege.c timing.c b64.c wordlist.c

TESTER_OBJS=$(TESTER_SRCS:.c=.o) 
LIBCRP_OBJS=$(LIBCRP_SRCS:.c=.o)

ALL_SRCS=$(TESTER_SRCS)

#
# default target: build standalone speech encoder and decoder
#
default: libcrp.a test


test: $(TESTER_OBJS)
	$(CC) -o test $(CFLAGS) $(TESTER_OBJS) $(LDFLAGS) -lm

libcrp.a: $(LIBCRP_OBJS)
	/bin/rm -f libcrp.a
	ar cr libcrp.a $(LIBCRP_OBJS)
	-if test -s /bin/ranlib; then /bin/ranlib libcrp.a; \
	else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libcrp.a; \
	else exit 0; fi; fi

#
# how to compile a .c file into a .o
#
.SUFFIXES: .c .h .o
.c.o:
	$(CC) -c $(CFLAGS) $<

#
# make / clean dependency list
#
depend:
	$(MAKE) -f $(MAKEFILENAME) $(MFLAGS) $(MAKEDEFS) clean_depend
	$(CC) $(CFLAGSDEPEND) $(ALL_SRCS) >> $(MAKEFILENAME)

clean_depend:
	chmod u+w $(MAKEFILENAME)
	(awk 'BEGIN{f=1}{if (f) print $0}/^\# DO NOT DELETE THIS LINE -- make depend depends on it./{f=0}'\
	    < $(MAKEFILENAME) > .depend && \
	mv .depend $(MAKEFILENAME)) || exit 1;

#
# remove object/executable files
#
clean:
	rm -f *.o test test.exe libcrp.a


# DO NOT DELETE THIS LINE -- make depend depends on it.
curve.0: curve.c curve.h 
sponge.o: sponge.c sponge.h Keccak512_data.h
sprng.o: sprng.c sprng.h
havege.o: havege.c havege.h 
timing.o: timing.c timing.h 
b64.o: b64.c b64.h 
wordlist.o: wordlist.c wordlist.h 
test.o: test.c

