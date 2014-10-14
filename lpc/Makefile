CFLAGS = -std=c99 -Wall -Wextra -pedantic -fomit-frame-pointer -ffast-math -funroll-loops -O3

lpc.o:	lpc.c lpc.h
	$(CC) $(CFLAGS) -c lpc.c

lpctest: lpctest.c lpc.o
	$(CC) lpctest.c lpc.o ../ulaw.o -o lpctest -lm

clean:
	rm -f *.bak *.o lpctest core

depend:
	makedepend -- $(CFLAGS) -- *.c

# DO NOT DELETE
