/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		matrixgen
*
* FUNCTION
*		 
*		This routine is used to generate the H matrix and 
*		syndrome table necessary for Hamming encode and decode.  
*		This routine should be called once before calling
*		encodeham and decodeham.
*
* SYNOPSIS
*		subroutine matrixgen(codelength1,codelength2,
*					hmatrix,syndrometable)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	codelength1	int	i	number of data bits (63)
*	codelength2	int	i	number of information bits (57)
*	hmatrix 	int	o	vector to encode and decode by
*	syndrometable	int	o	table containing error masks
*
***************************************************************************
*
* DESCRIPTION
*
*	This subroutine is part of a set of subroutines which perform
*	a Generalized Hamming Code.  As you know, Hamming codes are perfect
*	codes and can only detect and correct one error.  We added an overall
*	parity checkbit, which allows us to detect 2 errors.  When 2 errors 
*	are detected, (in subroutine decodeham) no correction attempt is
*	made.  This would most likely result in more errors.  Instead, a flag
*	is sent to the calling program notifying it of multiple errors so
*	that smoothing may be attempted.  The Hamming codes presently 
*	supported by the routines are (63,57), (31,26), (15,11), and
*	shortened variations thereof.  It could be made even more general 
*	by making minor modifications to the decimal to binary output vector 
*	code in the encodeham procedure.  This routine at present will 
*	calculate a maximum of 6 bits.
*
*	Hamming routines consist of the following files:
*
*		matrixgen - generates the hmatrix and sydrometable.
*		encodeham - generates the codeword and overall paritybit.
*		decodeham - recovers infobits, checks for errors, corrects 1
*			    error, and sends out flag for smoothing.
*
*	This routine initializes all of the tables necessary to perform
*	the Hamming code (G Matrix, Syndrome Table) .  
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*
***************************************************************************
*	
* REFERENCES
*
*	Lin and Costello:  Error Control Coding
*	Berlekamp:  Algebraic Coding Theory
*
**************************************************************************/

static void matrixgen(int codelength1, int codelength2,
		      int hmatrix[], int syndrometable[])
{
	int i, temp1;

	/*    This is the data necessary to construct the G Matrix and the 
	   Syndrome Table.  If a larger code is desired, this table can 
	   be easily added to.  All other routines, except the syndrome 
	   table construction,  are general enough to calculate any size 
	   Hamming Code.                                                      */

	static const int itemplate[] = { 1, 2, 4, 8, 16, 32 };
	static const int ptemplate[] =
	    { 3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36,
		37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
	};

	/*    Construct the parity portion of the hmatrix                     */

	for (i = 0; i < codelength2; i++)
		hmatrix[i] = ptemplate[i];

	/*    Construct the identity portion of the hmatrix.                  */

	for (i = 0; i < codelength1 - codelength2; i++)
		hmatrix[codelength2 + i] = itemplate[i];

	/*    Construct the syndrometable.  This routine is rather simple because
	   I chose to arrange my G matrix sequentially (Berlekamp method).  
	   I placed the parity bits in front in ascending order then added the 
	   bits left over in ascending order.  Since our code is linear I can
	   get away with this.  If a larger Hamming code is needed, then a new 
	   exception must be generated for each parity bit.           */

	temp1 = 1;
	for (i = 1; i <= codelength1; i++) {
		switch (i) {
		case 1:
			syndrometable[i - 1] = codelength2 + 1;
			break;
		case 2:
			syndrometable[i - 1] = codelength2 + 2;
			break;
		case 4:
			syndrometable[i - 1] = codelength2 + 3;
			break;
		case 8:
			syndrometable[i - 1] = codelength2 + 4;
			break;
		case 16:
			syndrometable[i - 1] = codelength2 + 5;
			break;
		case 32:
			syndrometable[i - 1] = codelength2 + 6;
			break;
		default:
			syndrometable[i - 1] = temp1++;
		};
	}
}
