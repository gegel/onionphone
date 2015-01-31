/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		variance
*
* FUNCTION
*
*		calculates variance of the input array
*
* SYNOPSIS
*		subroutine variance(arr, no, var, avg)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	arr		float	i	input data array
*	no		int	i	number of elements in array
*	var		float	o	variance
*	avg		real	i/o	average
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
*	smoothcbgain   smoothtau
*
* CALLS
*
*
*
**************************************************************************/

static void variance(float arr[], int no, float *var, float *avg)
{
	int i;
	float sum, sum2;

	for (sum = 0.0, i = 0; i < no; i++)
		sum += arr[i];
	*avg = sum / no;
	for (sum2 = 0.0, i = 0; i < no; i++)
		sum2 += (*avg - arr[i]) * (*avg - arr[i]);
	*var = sum2 / (no - 1);
}
