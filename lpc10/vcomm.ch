/*   Voicing coefficient and Linear Discriminant Analysis variables:
 Max number of VDC's and VDC levels
*/
#define MAXVDC 10
#define MXVDCL 10

/* Actual number of VDC's and levels  */

int nvdc, nvdcl=5;

/*   Voicing Decision Parameter vector (* denotes zero coefficient):
*
*	* MAXMIN
*	  LBE/LBVE
*	  ZC
*	  RC1
*	  QS
*	  IVRC2
*	  aR_B
*	  aR_F
*	* LOG(LBE/LBVE)

*   Define 2-D voicing decision coefficient vector according to the voicing
*   parameter order above.  Each row (VDC vector) is optimized for a specific
*   SNR.  The last element of the vector is the constant.

*	         E    ZC    RC1    Qs   IVRC2  aRb   aRf        c		*/

float vdc[MAXVDC][MXVDCL]={
    	{    0.,     0.,     0.,      0.,      0., 0., 0., 0., 0., 0. }, 
	{ 1714.,   874.,   510.,    500.,    500., 0., 0., 0., 0., 0. },
	{ -110.,   -97.,   -70.,    -10.,      0., 0., 0., 0., 0., 0. },
	{  334.,   300.,   250.,    200.,      0., 0., 0., 0., 0., 0. },
        {-4096., -4096., -4096.,  -4096.,  -4096., 0., 0., 0., 0., 0. },
	{ -654., -1021., -1270.,  -1300.,  -1300., 0., 0., 0., 0., 0. },
	{ 3752.,  2451.,  2194.,   2000.,   2000., 0., 0., 0., 0., 0. },
	{ 3769.,  2527.,  2491.,   2000.,   2000., 0., 0., 0., 0., 0. },
	{    0.,     0.,     0.,      0.,      0., 0., 0., 0., 0., 0. },
	{ 1181.,  -500., -1500.,  -2000.,  -2500., 0., 0., 0., 0., 0. }

};

float vdcl[MXVDCL];
