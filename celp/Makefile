# Select the following #ifdef switches for the CFLAGS macro:
#
# -DPOSTFIL2	if present, uses postfilter style 2 (simpler)
#

CFLAGS = -Wall -fomit-frame-pointer -ffast-math -funroll-loops -O4

all:	celp.o
	
celp_test: celp_test.o celp.o
	   $(CC) $(CFLAGS) -o celp_test celp_test.o celp.o -lm

depend:
	makedepend -- $(CFLAGS) -- *.c
	
#	Notes on testing the CELP library:
#
#	The following target uses a small test driver program
#	to encode a speech test file in CELP encoding and then
#	decode it.  Both the CELP encoded file (test.clp) and
#	the results of synthesising it (ofile.spd) are then
#	compared against reference files produced by a run of
#	the encoder which passed the "ear test".  The CELP
#	Codec performs extensive computation using 32 bit
#	floating point variables.  Round-off in computations
#	performed with such limited (yet entirely adequate)
#	precision quantities will depend on the order in which
#	arithmetic operations are performed, which will differ
#	from compiler to compiler, and even on the same
#	compiler with different optimisation options.
#	Round-off induced changes may consequently appear
#	in either the PCM output file or the CELP encoded
#	intermediate file, resulting in compare errors
#	against the reference files.  Legitimate differences
#	due to round-off are imperceptible to the ear.
#	This test is primarily useful as a reality check
#	when hacking on the CELP library in the interest of
#	optimisation.  Keeping the compiler and options
#	constant, it permits one to determine whether a
#	supposedly transparent change was, in fact, as
#	intended.
#
#	In the interest of reducing the size of the Speak Freely
#	distribution archive, the test and reference files are not
#	included in the regular distribution.  You may download
#	a ZIP containing them from:
#	    http://www.fourmilab.ch/speakfree/unix/download/celp_test/testdata.zip
#	The reference files in this archive were made using the
#	CELP library compiled with GCC 2.96 on an Intel Pentium III
#	processor using the "-g -Wall" compiler options.
	
check:	celp_test
	time -p ./celp_test
	-cmp ofile.spd ofile.spd.1
	-cmp test.clp test.clp.1
	
profile: celp_test
	time -p ./celp_test
	gprof celp_test >gprof.out
	-cmp ofile.spd ofile.spd.1
	-cmp test.clp test.clp.1
	
TESTDATA = speech.spd test.clp.1 ofile.spd.1

testdata.zip:
	rm -f /tmp/testdata.zip
	zip /tmp/testdata.zip $(TESTDATA)

clean:
	rm -f celp_test core *.o *.a ofile.spd *.clp *.out

distclean: clean
	rm -f $(TESTDATA)

# DO NOT DELETE
