#include <stdio.h>
#include <windows.h>
#include <float.h>

#include "lpcdefs.h"
#include "config.ch"
#include "common.h"

//#include <sys/time.h>

int main()
{
    FILE *fi = fopen("c:/tmp/test2.au", "rb"),
         *ft = fopen("c:/tmp/lpc.tmp", "w+b"),
         *fo = fopen("c:/tmp/out.au", "wb");
    unsigned char buf[LFRAME], nuf[LFRAME], wuf[LFRAME];
    long i, j, k, l = 0;
//    struct timeval t1, t2;

    if (fi == NULL) {
                fprintf(stderr, "Cannot open input file.\n");
		return 0;
	}

    //	Disable floating point error interrupts
    
    _control87(_EM_ZERODIVIDE | _EM_OVERFLOW | _EM_INVALID,
		   _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_INVALID);

    lpc10init();
	fread(buf, 1, LFRAME, fi);
	fwrite(buf, 1, LFRAME, fo);
//    gettimeofday(&t1, NULL);
    while (1) {
	i = fread(buf, 1, LFRAME, fi);
	if (i == 0) {
	    break;
	}
	j = lpc10encode(buf, nuf, i);
	fwrite(nuf, j, 1, ft);
	l += i;
    }
/*    gettimeofday(&t2, NULL);
    printf("Encoding at %ld samples per second.\n",
	(long) (((double) l) / ((t2.tv_sec - t1.tv_sec) +
		((t2.tv_usec - t1.tv_usec) / 1000000.0))));
*/		
    rewind(ft);
//    gettimeofday(&t1, NULL);
    l = 0;
    while (1) {
	i = fread(nuf, 1, 8, ft);
	if (i == 0) {
	    break;
	}
	k = lpc10decode(nuf, wuf, i);
	fwrite(wuf, k, 1, fo);
	l += k;
    }
/*    gettimeofday(&t2, NULL);
    printf("Decoding at %ld samples per second.\n",
	(long) (((double) l) / ((t2.tv_sec - t1.tv_sec) +
		((t2.tv_usec - t1.tv_usec) / 1000000.0))));
*/		
    fclose(fo);
    fclose(ft);
    fclose(fi);
	return 0;
}
