#define ULAW

#ifdef ULAW
#include "../ulaw2linear.h"
#endif

/******************************************************************
*
*	DISKIO Version 49
*
******************************************************************
*
*    Handle I/O to sampled data (speech) files
*
*      Uses FORTRAN sequential access, unformatted files with
*      signed 16 bit samples, 64 samples per record.
*
*   DISKRD - Read speech from disk file
*   DISKWR - Write speech to disk file
*      CHAN   - Channel number (1 to MAXC)
*      SPEECH - REAL buffer of samples (-1.0 to 1.0)
*      LENGTH - Number of samples to read or write
*		if ZERO, then close input or output file
*      EOF    - If TRUE, end of file encountered on input
*
*   DISKOP - Open a speech file for read or write
*      CHAN   - Channel number (1 to MAXC)
*      FUNCTN - 0 for read, 1 for write
*      FNAME  - File name
*
*   DISKCL - Close a speech file
*      CHAN   - Channel number (1 to MAXC)
*
*   DISKFN - Get the filename of an open speech file
*      CHAN   - Channel number (1 to MAXC)
*      FNAME  - File name
*      FLEN   - Number of characters in FNAME
*
*   NOTE: Functions DISKOP and DISKCL are the preferred means
*   for opening and closing speech files.  If DISKOP is not called
*   prior to DISKRD or DISKWR, the user is prompted for a filename.
*   DISKCL should be called for output files to ensure that the final
*   record is written.
*/

#include "lpcdefs.h"

#define RECLEN 8192
#define maxc 4 
#define rd 0
#define wr 1

diskio(which, chan, speech, length, eof, fname, functn)
int which, length, functn;
float *speech;
char *fname;
short *eof;
FILE *chan;
{
int i,k;
#ifdef ULAW
unsigned char buf[RECLEN / 2];
#else
short buf[RECLEN];
#endif
#ifdef WUF
short wuf[RECLEN];
#endif

switch(which) {
case 0:
/******************************************************************
*	Read From Input File
*******************************************************************/


/*    Input Samples	*/

	k = fread(buf, sizeof(buf[0]), length, chan);
	if(k < length) *eof = END;
	for(i=0;i<k;i++)	{
#ifdef ULAW
		speech[i] = (float)audio_u2s(buf[i])/32768;
#else
		speech[i] = (float)buf[i]/32768;
#endif
	}


break;

case 1:
/******************************************************************
*	Write To Output File
*******************************************************************/



/*    Output Samples	*/

	for(i=0;i<length;i++)
#ifndef WUF
#ifdef ULAW
		buf[i] = audio_s2u((short) mmax(-32768., mmin(32768.*speech[i], 32767.)));
#else
		buf[i] = mmax(-32768., mmin(32768.*speech[i], 32767.));
#endif
	k = fwrite(buf, sizeof(buf[0]), length, chan);
#else
wuf[i] = mmax(-32768., mmin(32768.*speech[i], 32767.));
k = fwrite(wuf, sizeof(wuf[0]), length, chan);
#endif
	if (k != length)
	{
                printf(" ***** disk write error ***** \n");
		exit(1);
	}

break;

case 2:
/******************************************************************
*	Open Files
*******************************************************************/
	if(chan== NULL)
		switch(functn)
		{
			/*case READ:*/
			case 0:
                                chan = fopen(fname, "rb");
				if(chan == NULL)
				{
					*eof=NOFILE;
                                        printf("***** Error opening %s for reading *****\n",fname);
				}
				break;
			/*case WRITE:*/
			case 1:
                                chan = fopen(fname, "wb");
				if (chan == NULL)
				{
					*eof=NOFILE;
                                        printf("***** Error opening %s for writing *****\n",fname);
				}
				break;
		} /* end switch functn */
break;
case 3:
/******************************************************************
*	Close Files
*******************************************************************/
/*****
	ENTRY DISKCL(CHAN)

	IF( CHAN.LT.1 .OR. CHAN.GT.MAXC ) GOTO 960
	CH = CHAN

400	IF(OPEN(CH)) THEN
	   CALL SPD_CLOSE(LUN(CH))
	   OPEN(CH) = .FALSE.
	END IF
	RETURN
*****/
printf("******** Close files not yet written ******\n");
break;
case 4:
/******************************************************************
*	Return Filenames
*******************************************************************/
/*****
	ENTRY DISKFN(CHAN, FNAME, FLEN)

	IF( CHAN.LT.1 .OR. CHAN.GT.MAXC ) GOTO 960
	IF(OPEN(CHAN)) THEN
	   INQUIRE(LUN(CHAN),NAME=FNAME)
	ELSE
           FNAME = 'None'
	END IF
	FLEN = LNBLNK(FNAME)
	RETURN

960     WRITE(STDERR,*) 'Invalid Channel Number: ',CHAN
	RETURN

	END
*****/
printf("Return Filenames\n");
break;
} /* end switch */
}
