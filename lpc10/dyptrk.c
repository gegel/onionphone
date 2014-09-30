/**********************************************************************
*
*	DYPTRK Version 52
*
**********************************************************************
*
*   Dynamic Pitch Tracker
*
*  Inputs:
*   AMDF   - Average Magnitude Difference Function array
*   LTAU   - Number of lags in AMDF
*   MINPTR - Location of minimum AMDF value
*   VOICE  - Voicing decision
*  Outputs:
*   PITCH  - Smoothed pitch value, 2 frames delayed
*   MIDX   - Initial estimate of current frame pitch
*  Compile time constant:
*   DEPTH  - Number of frames to trace back
*/

#include "lpcdefs.h"

extern float s[60];
extern int p[60][2];

void dyptrk(float amdf[], int minptr, int voice, int *pitch, int *midx)
{
float sbar, minsc, maxsc, alpha;
int depth=2;
int pbar, i, j, iptr, path[2];
static float alphax, ipoint=0.0;


/*   Calculate the confidence factor ALPHA, used as a threshold slope in
*   SEESAW.  If unvoiced, set high slope so that every point in P array
*   is marked as a potential pitch frequency.  A scaled up version (ALPHAX)
*   is used to maintain arithmetic precision.	*/

if( voice == 1 ) 
	alphax = (float) (.75*alphax + amdf[minptr]*.5);
else
	/*alphax = (63./64.)*alphax;*/
	alphax = (0.984375f)*alphax;
alpha = alphax*0.06250f;

if( voice == 0 && alphax < 128 ) alpha = 8;

/*  SEESAW: Construct a pitch pointer array and intermediate winner function
*   Left to right pass: 	*/

iptr = (int) (ipoint+1);
p[0][iptr-1] = 1;
i = 1;
pbar = 1;
sbar = s[0];

for(i=1;i<=LTAU;i++)	{
	sbar += alpha;
	if (sbar < s[i-1]) {
		s[i-1] = sbar;
		p[i-1][iptr-1] = pbar;
	}
	else	{
		sbar = s[i-1];
		p[i-1][iptr-1] = i;
		pbar = i;
	}
}

/*   Right to left pass:	*/

i = pbar-1;
sbar = s[i];
while (i>=1)	{
	sbar += alpha;
	if (sbar< s[i-1]) {
		s[i-1] = sbar;
		p[i-1][iptr-1] = pbar;
	}
	else	{
		pbar = p[i-1][iptr-1];
		i = pbar;
		sbar = s[i-1];
	}
	i--;
}

/*   Update S using AMDF
*   Find maximum, minimum, and location of minimum	*/

s[0] += (float) (amdf[1]*0.5);
minsc = s[0];
maxsc = minsc;
*midx = 1;

for(i=2;i<=LTAU;i++)	{
	s[i-1] += (float) (amdf[i]*0.5);
	if(s[i-1] > maxsc) maxsc = s[i-1];
	if(s[i-1] < minsc) *midx = i;
	if(s[i-1] < minsc) minsc = s[i-1];
}

/*   Subtract MINSC from S to prevent overflow	*/

for(i=1;i<=LTAU;i++)
	s[i-1] -= minsc;
maxsc -= minsc;

/*   Use higher octave pitch if significant null there	*/

j = 0;
for(i=20;i<=40;i+=10)	
	if (*midx > i) 
		if (s[*midx-i-1] < maxsc*0.25) j = i;
*midx -= j;

/*   TRACE: look back two frames to find minimum cost pitch estimate	*/

j = (int) ipoint;
*pitch = *midx;
for(i=1;i<=depth;i++)	{
	j = j%depth+1;
	*pitch = p[*pitch-1][j-1];
	path[i-1] = *pitch;
}
ipoint = (float) (((int)(ipoint)+depth-1)%depth);




}
