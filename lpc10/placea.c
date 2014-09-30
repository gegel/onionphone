/************************************************************************
*
*	PLACEA Version 48
*
************************************************************************/

#include "lpcdefs.h"

#define lrange (AF-2)*LFRAME + 1
#define hrange AF*LFRAME

void placea(int ipitch, int voibuf[2][AF+1], int obound,
    	    int vwin[2][AF], int awin[2][AF], int ewin[2][AF])
{

/* Local variables and parameters	*/

int i, j, k, l;
short ephase, winv;
static short allv;
int temp;

/*   Place the Analysis window based on the voicing window
*   placement, onsets, tentative voicing decision, and pitch.
*
*   Case 1:  Sustained Voiced Speech
*   If the five most recent voicing decisions are 
*   voiced, then the window is placed phase-synchronously with the 
*   previous window, as close to the present voicing window if possible.
*   If onsets bound the voicing window, then preference is given to
*   a phase-synchronous placement which does not overlap these onsets.
*
*   Case 2:  Voiced Transition
*   If at least one voicing decision in AF is voicied, and there are no
*   onsets, then the window is placed as in case 1.
*
*   Case 3:  Unvoiced Speech or Onsets
*   If both voicing decisions in AF are unvoiced, or there are onsets,
*   then the window is placed coincident with the voicing window.
*
*   Note:  During phase-synchronous placement of windows, the length
*   is not altered from MAXWIN, since this would defeat the purpose
*   of phase-synchronous placement.

* Check for case 1 and case 2	*/

allv =		  (voibuf[1][AF-2] == 1)?1:0;
allv = (allv && voibuf[0][AF-1] == 1)?1:0;
allv = (allv && voibuf[1][AF-1] == 1)?1:0;
allv = (allv && voibuf[0][AF  ] == 1)?1:0;
allv = (allv && voibuf[1][AF  ] == 1)?1:0;

winv = (voibuf[0][AF  ] == 1 ||  voibuf[1][AF  ] == 1)?1:0;

if (allv || (winv && (obound == 0))) {

/* APHASE:  Phase synchronous window placement.
* Get minimum lower index of the window.		*/

	i = (lrange + ipitch - 1 - awin[0][AF-2]) / ipitch;
	i = i * ipitch;
	i = i + awin[0][AF-2];

/* L = the actual length of this frame's analysis window.       */

	l = MAXWIN;

/* Calculate the location where a perfectly centered window would start.  */

	k = (int) ((vwin[0][AF-1] + vwin[1][AF-1] + 1 - l) * 0.5);

/* Choose the actual location to be the pitch multiple closest to this. */
	temp = (int) ((float)((float)((float)(k - i) / (float)ipitch) + .5));
	awin[0][AF-1] = i + temp  * ipitch;
	awin[1][AF-1] = awin[0][AF-1] + l - 1;
/* If there is an onset bounding the right of the voicing window and the
* analysis window overlaps that, then move the analysis window backward
* to avoid this onset.	*/

	if (obound >= 2 && awin[1][AF-1] > vwin[1][AF-1]) {
		awin[0][AF-1] -= ipitch;
		awin[1][AF-1] -= ipitch;
	}

/* Similarly for the left of the voicing window.	*/

	if ((obound == 1 || obound == 3) && awin[0][AF-1] < vwin[0][AF-1]) {
		awin[0][AF-1] += ipitch;
		awin[1][AF-1] += ipitch;
	}

/* If this placement puts the analysis window above HRANGE, then
* move it backward an integer number of pitch periods.	*/

	while (awin[1][AF-1] > hrange)	{
	
		awin[0][AF-1] -= ipitch;
		awin[1][AF-1] -= ipitch;
	}

/* Similarly if the placement puts the analysis window below LRANGE.	*/

	while (awin[0][AF-1] < lrange)	{
		awin[0][AF-1] += ipitch;
		awin[1][AF-1] += ipitch;
	}


/* Make Energy window be phase-synchronous.	*/

	ephase = 1;

/* Case 3	*/
}
else	{
	awin[0][AF-1] = vwin[0][AF-1];
	awin[1][AF-1] = vwin[1][AF-1];
	ephase = 0;
}

/* RMS is computed over an integer number of pitch periods in the analysis
* window.  When it is not placed phase-synchronously, it is placed as close 
* as possible to onsets.  */

j = ((awin[1][AF-1]-awin[0][AF-1]+1)/ipitch)*ipitch;
if (j == 0 || !winv) {
	ewin[0][AF-1] = vwin[0][AF-1];
	ewin[1][AF-1] = vwin[1][AF-1];
}
else	 
	if (!ephase && obound == 2) {
		ewin[0][AF-1] = awin[1][AF-1] - j + 1;
		ewin[1][AF-1] = awin[1][AF-1];
	}
	else	{
		ewin[0][AF-1] = awin[0][AF-1];
		ewin[1][AF-1] = awin[0][AF-1] + j - 1;
	}



}
