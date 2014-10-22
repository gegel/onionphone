/*******************************************************************
*
*	PLACEV Version 48
*
*******************************************************************/

#include "lpcdefs.h"

void placev(int osbuf[], int osptr, int *obound, int vwin[2][AF])
{

/*   Arguments
*    OSBUF	Buffer which holds sorted indexes of onsets
*    OSPTR	Free pointer into OSBUF
*    VWIN	Buffer of Voicing Window Positions (Modified)
*    OBOUND	This variable is set by this procedure and used
*		in placing analysis windows (PLACEA).  Bit 1
*		indicates whether an onset bounds the left side
*		of the voicing window, and bit 2 indicates whether
*		an onset bounds the right side of the voicing window.
*   Variables
*    LRANGE, HRANGE  Range in which window is placed
*    OSPTR1	OSPTR excluding samaples in 3F
*/

int lrange, hrange, i;
int q;
int osptr1;
short crit;

/*   Voicing Window Placement
*
*	  __________________ __________________ ______________
*	 |		    |		       |
*	 |	  1F	    |	     2F        |	3F ...
*	 |__________________|__________________|______________
*
*    Previous |
*      Window |
*  ...________|
*
*	      | 			       |
*      ------>| This window's placement range  |<------
*	      | 			       |
*
*   There are three cases.  Note that these are different from those
*   given in the LPC-10e phase 1 report.
*
*   1.	If there are no onsets in this range, then the voicing window
*   is centered in the pitch window.  If such a placement is not within
*   the window's placement range, then the window is placed in the left-
*   most portion of the placement range.  Its length is always MAXWIN.
*
*   2.	If the first onset is in 2F and there is sufficient room to place
*   the window immediately before this onset, then the window is placed
*   there, and its length is set to the maximum possible under these
*   constraints.
*
*       "Critical Region Exception":  If there is another onset in 2F
*	such that a window can be placed between the two onsets, the
*	window is placed there (ie, as in case 3).
*
*   3.	Otherwise, the window is placed immediately AFter the onset.  The 
*   window's length
*   is the longest length that can fit in the range under these constraints,
*   except that the window may be shortened even further to avoid overlapping
*   other onsets in the placement range.  In any case, the window's length
*   is at least MINWIN.
*
*   Note that the values of MINWIN and LFRAME must be chosen such
*   that case 2 = false implies case 3 = true.	 This means that
*   MINWIN <= LFRAME/2.  If this were not the case, then a fourth case
*   would have to be added for when the window cannot fit either before
*   or AFter the onset.
*
*   Note also that onsets which weren't in 2F last time may be in 1F this
*   time, due to the filter delays in computing onsets.  The result is that
*   occasionally a voicing window will overlap that onset.  The only way
*   to circumvent this problem is to add more delay in processing input
*   speech.  In the trade-off between delay and window-placement, window
*   placement lost.
*/

/* Compute the placement range	*/


lrange = mmax(vwin[1][AF-2]+1, (AF-2)*LFRAME+1);
hrange = AF*LFRAME;

/* Compute OSPTR1, so the following code only looks at relevant onsets. */

for (osptr1=osptr-1; osptr1>=1; osptr1--) {
   if (osbuf[osptr1-1] <= hrange) break;
}
osptr1++;

/* Check for case 1 first (fast case):	*/

if ((osptr1 <= 1) || (osbuf[osptr1-2] < lrange)) {
	vwin[0][AF-1] = mmax(vwin[1][AF-2]+1, DVWINL);
	vwin[1][AF-1] = vwin[0][AF-1] + MAXWIN - 1;
	*obound = 0;
}
else {

/* Search backward in OSBUF for first onset in range.
* This code relies on the above check being performed first.	*/

	for(q=osptr1-1;q>=1;q--) {
		if (osbuf[q-1] < lrange) break;
	}
	q++;

/* Check for case 2 (placement before onset):

* Check for critical region exception:	*/
	
	crit = 0;
	for(i=q+1;i<=osptr1-1;i++) {
		if (osbuf[i-1] - osbuf[q-1] >= MINWIN) {
			crit = 1;
			break;
		}
	}

	if (!crit && osbuf[q-1] > mmax((AF-1)*LFRAME, lrange+MINWIN-1)) {
		vwin[1][AF-1] = osbuf[q-1] - 1;
		vwin[0][AF-1] = mmax (lrange, vwin[1][AF-1]-MAXWIN+1);
		*obound = 2;
	}
/* Case 3 (placement AFter onset)	*/
	
	else {
		vwin[0][AF-1] = osbuf[q-1];
L110:		q++;
		if(q < osptr1) {
			if(osbuf[q-1] <= vwin[0][AF-1] + MAXWIN) {
				if (osbuf[q-1]	< vwin[0][AF-1] + MINWIN) goto L110;
				vwin[1][AF-1] = osbuf[q-1] - 1;
				*obound = 3;
			}
			else {
				vwin[1][AF-1] = mmin(vwin[0][AF-1] + MAXWIN - 1, hrange);
				*obound = 1;
			}
		}
		else {
			vwin[1][AF-1] = mmin(vwin[0][AF-1] + MAXWIN - 1, hrange);
			*obound = 1;
		}
	}

}
}
