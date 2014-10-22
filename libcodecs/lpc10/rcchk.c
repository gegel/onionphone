/**********************************************************************
*
*	RCCHK Version 45G
*
**********************************************************************
*
*  Check RC's, repeat previous frame's RC's if unstable
*
* Inputs:
*  ORDER - Number of RC's
*  RC1F  - Previous frame's RC's
* In/Outputs:
*  RC2F  - Present frame's RC's
*/

#include "lpcdefs.h"

void rcchk(float rc1f[MAXORD][AF])
{
int i;

for(i=0;i<ORDER;i++)	{
  if((float)fabs((double)rc1f[i][AF-1]) > .99) break;
}

if(i<ORDER)
  if((float)fabs((double)rc1f[i][AF-1]) > .99)
	{
		for(i=0;i<ORDER;i++)
			rc1f[i][AF-1] = rc1f[i][AF-2];
	}
}
