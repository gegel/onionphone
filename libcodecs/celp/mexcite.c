/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME
*		mexcite
*
* FUNCTION
*		Modify the stochastic code book excitation gain
*
* SYNOPSIS
*		subroutine mexcite1(l)
*		subroutine mexcite2(l)
*		subroutine mexcite3(cgain)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	------------------------------------------------------------------
*	l		int	i	length of error signal
*	cgain		float	i/o	stochastic code book gain
*
*   global 
*			data	I/O
*	name		type	type	function
*	------------------------------------------------------------------
*	/ccsub/ 	see description include file
*	e0		float	i	error signal array
*	
***************************************************************************
*
* DESCRIPTION
*
*	Depending on the current system state, the stochastic code book
*	excitation is reduced to a level that is low enough to produce
*	positive perceptual effects, yet is high enough so as not to upset
*	the dynamics of the system.  The main effect of the method is that
*	during sustained voiced sounds, the excitation level is attenuated
*	and in unvoiced and transition regions the level is amplified to a
*	level slightly more than that of standard CELP.
*
*	The relative adaptive code book excitation component is
*	increased in voiced regions by decreasing the stochastic code book
*	excitation component.  The amount of decrease in the stochastic
*	component depends on the efficiency of the adaptive component.
*	More reconstruction burden is placed on the adaptive component as
*	its efficiency increases.  The efficiency is measured by the
*	closeness (in the crosscorrelation sense) of the residual signals
*	before and after pitch prediction.  When the efficiency is high
*	(e.g., > 0.9), the stochastic component is amplified slightly
*	(e.g., one quantizer level). 
*
*	The procedure for modifying the stochastic gain outside the
*	search loop is:
*	1)  Measure the efficiency of the adaptive component (ccor)
*	2)  Search the stochastic code book for the optimum codeword
*	3)  Modify the stochastic code book gain
*
*	This method is compatible with Federal Standard 1016.
*
*
*
***************************************************************************
*
* CALLED BY
*
*	csub csearch
*
* CALLS
*
***************************************************************************
* REFERENCES
*
*       Shoham, Yair, "Constrained-Stochastic Excitation Coding of Speech
*       at 4.8 kbps," in Advances in Speech Coding, ed. B. Atal, V.
*	Cuperman, and A. Gersho, submitted to Kluwer Academic Publishers.
*
*       Shoham, Yair, "Constrained-Stochastic Excitation Coding of Speech," 
*	Abstracts of the IEEE Workshop on Speech Coding for
*	Telecommunications, 1989, p. 65.
*
***************************************************************************/

static float ccor;
static float e1, e0save[60];

static void mexcite1(int l)
{
	int i;

	/* *e1 = Euclidean norm of the first error signal             */
	/* (note: the error signal array e0 is reused)                */

	e1 = 1e-6;
	for (i = 0; i < l; i++) {
		e0save[i] = e0[i];
		e1 += e0[i] * e0[i];
	}
}

static void mexcite2(int l)
{
	int i;

	/* *ccor = crosscorrelation of the residual signals before    */
	/* *and after pitch prediction                                */
	/* *(note: the error signal array e0 is reused                */

	ccor = 1e-6;
	for (i = 0; i < l; i++)
		ccor += e0[i] * e0save[i];

	/* *normalize the crosscorrelation                            */

	ccor = ccor / e1;
}

static void mexcite3(float *cgain)
{
	float scale;

	/* *square root crosscorrelation scaling                      */

	scale = sqrt(fabs(ccor));

	/* *modify scale                                              */

	if (scale < 0.2)
		scale = 0.2;
	else if (scale > 0.9)
		scale = scale * 1.4;

	/* *modify the stochastic component                           */

	*cgain = *cgain * scale;
}
