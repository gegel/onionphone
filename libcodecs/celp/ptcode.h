/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		pitchencode
*
* FUNCTION
*
*		encode and quantize pitch gain (bb(3)) for various
*		quantizer types.
*
* SYNOPSIS
*		subroutine pitchencode(input, index)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	input		int	i	pitch gain input (true value)
*	index		float	o	encoded pitch gain index
*	pitchencode	float	fun	encoded pitch gain
*
***************************************************************************
*
* DESCRIPTION
*
*       This funtion uses output level data obtained by Max's minimum
*	distortion quantization priciples and quantizes to the nearest
*	level (L1 norm).  (Using level data only was found superior to
*       using both of Max's level and boundry data.)
*
***************************************************************************
*
* CALLED BY
*
*	psearch
*
* CALLS
*
*	
***************************************************************************
*
* REFERENCES
*
*	Quantizing for Minimum Distorion
*	J. Max
*	IRE Trans. Inform. Theory, vol. IT-6, pp.7-12, Mar. 1960
*
***************************************************************************
*
*	The data used in the table generation is from 3m3f.spd.
*
**************************************************************************/

static const float pitch2max5[32] = {
	-0.993, -0.831, -0.693, -0.555, -0.414, -0.229, 0.0, 0.139,
	0.255, 0.368, 0.457, 0.531, 0.601, 0.653, 0.702, 0.745,
	0.780, 0.816, 0.850, 0.881, 0.915, 0.948, 0.983, 1.020,
	1.062, 1.117, 1.193, 1.289, 1.394, 1.540, 1.765, 1.991
};

static float pitchencode(float input, int *index)
{
	int i;
	float dist, low;

	low = dist = fabs(input - *pitch2max5);
	*index = 0;
	for (i = 1; i < 32; i++) {
		dist = fabs(input - pitch2max5[i]);
		if (dist < low) {
			low = dist;
			*index = i;
		}
	}
	return (pitch2max5[*index]);
}

/**************************************************************************
*
* ROUTINE
*		pitchdecode
*
* FUNCTION
*
*		decode pitch gain (bb(3)) from pitch index and bit
*		index (bits)
*
* SYNOPSIS
*		subroutine pitchdecode(pindex, pitch)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	pindex		int	i	pitch index value
*	pitch		float	o	pitch gain decoded
*
***************************************************************************
*
* DESCRIPTION
*
*
***************************************************************************
*
* CALLED BY
*
*	dcodpg
*
* CALLS
*
*
***************************************************************************
*
*	The data used in the table generation is from 3m3f.spd.
*
***************************************************************************
*
* REFERENCES
*
*	Quantizing for Minimum Distorion
*	J. Max
*	IRE Trans. Inform. Theory, vol. IT-6, pp.7-12, Mar. 1960
*
***************************************************************************
*
*	The data used in the table generation is from 3m3f.spd.
*
**************************************************************************/

static void pitchdecode(int pindex, float *pitch)
{
	*pitch = pitch2max5[pindex];
}
