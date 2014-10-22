/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:    lpc.c
**
** Description: Functions that implement linear predictive coding 
**      (LPC) operations.  
**
** Functions:
**
**  Computing LPC coefficients:
**
**      Comp_Lpc()
**      Durbin()
**
**  Perceptual noise weighting:
**
**      Wght_Lpc()
**      Error_Wght()
**
**  Computing combined impulse response:
**
**      Comp_Ir()
**
**  Computing ringing response:
**
**      Sub_Ring()
**      Upd_Ring()
**
**  Synthesizing speech:
**
**      Synt()
**      Spf()
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include <stdio.h>

#include "g723_const.h"
#include "lbccodec.h"

extern Word16 PerFiltZeroTable[LpcOrder];
extern Word16 PerFiltPoleTable[LpcOrder];
extern CODSTATDEF CodStat;
extern DECSTATDEF DecStat;
extern Word16 PostFiltZeroTable[LpcOrder];
extern Word16 PostFiltPoleTable[LpcOrder];
extern Word16 HammingWindowTable[LpcFrame];
extern Word16 BinomialWindowTable[LpcOrder];
extern Flag UsePf;

extern Word16 Vec_Norm(Word16 * Vect, Word16 Len);
extern Word16 g723_mult_r(Word16 var1, Word16 var2);	/* Mult with round,     2 */
extern Word32 L_g723_mult(Word16 var1, Word16 var2);	/* Long mult,           1 */
extern Word32 L_g723_shr(Word32 L_var1, Word16 var2);	/* Long shift right,    2 */
extern Word32 L_g723_add(Word32 L_var1, Word32 L_var2);	/* Long add,        2 */
extern Word16 g723_norm_l(Word32 L_var1);	/* Long norm,            30 */
extern Word32 L_g723_shl(Word32 L_var1, Word16 var2);	/* Long shift left,     2 */
extern Word16 round_(Word32 L_var1);	/* Round,               1 */
extern Word32 L_mls(Word32, Word16);	/* Wght ?? */
extern Word16 g723_add(Word16 var1, Word16 var2);	/* Short add,           1 */
extern Word16 g723_shl(Word16 var1, Word16 var2);	/* Short shift left,    1 */

Word16 Durbin(Word16 * Lpc, Word16 * Corr, Word16 Err, Word16 * Pk2);
extern void Update_Acf(Word16 * Acfsf, Word16 * Shsf);

extern Word32 g723_L_deposit_h(Word16 var1);	/* 16 bit var1 -> MSB,     2 */
extern Word32 g723_L_msu(Word32 L_var3, Word16 var1, Word16 var2);	/* Msu,    1 */
extern Word32 g723_L_abs(Word32 L_var1);	/* Long abs,              3 */
extern Word16 div_l(Word32, Word16);
extern Word16 g723_negate(Word16 var1);	/* Short negate,        1 */
extern Word32 g723_L_mac(Word32 L_var3, Word16 var1, Word16 var2);	/* Mac,    1 */
extern Word16 g723_sub(Word16 var1, Word16 var2);	/* Short sub,           1 */
extern Word16 g723_extract_h(Word32 L_var1);	/* Extract high,        1 */
extern Word16 g723_mult(Word16 var1, Word16 var2);	/* Short mult,          1 */
/*
**
** Function:        Comp_Lpc()
**
** Description:     Computes the tenth-order LPC filters for an
**          entire frame.  For each subframe, a
**          Hamming-windowed block of 180 samples,
**          centered around the subframe, is used to
**          compute eleven autocorrelation coefficients.
**          The Levinson-Durbin algorithm then generates
**          the LPC coefficients.  This function requires
**          a look-ahead of one subframe, and hence
**          introduces a 7.5 ms encoding delay.
**
** Links to text:   Section 2.4
**
** Arguments:
**
**  Word16 *UnqLpc      Empty Buffer
**  Word16 PrevDat[]    Previous 2 subframes of samples (120 words)
**  Word16 DataBuff[]   Current frame of samples (240 words)
**
** Outputs:

**
**  Word16 UnqLpc[]     LPC coefficients for entire frame (40 words)
**
** Return value:    None
**
*/
void Comp_Lpc(Word16 * UnqLpc, Word16 * PrevDat, Word16 * DataBuff)
{
	int i, j, k;

	Word16 Dpnt[Frame + LpcFrame - SubFrLen];
	Word16 Vect[LpcFrame];
	Word16 Acf_sf[LpcOrderP1 * SubFrames];
	Word16 ShAcf_sf[SubFrames];
	Word16 Exp;
	Word16 *curAcf;
	Word16 Pk2;

	Word32 Acc0, Acc1;

	/*
	 * Generate a buffer of 360 samples.  This consists of 120 samples
	 * from the previous frame and 240 samples from the current frame.
	 */
	for (i = 0; i < LpcFrame - SubFrLen; i++)
		Dpnt[i] = PrevDat[i];
	for (i = 0; i < Frame; i++)
		Dpnt[i + LpcFrame - SubFrLen] = DataBuff[i];

	/*
	 * Repeat for all subframes
	 */
	curAcf = Acf_sf;
	for (k = 0; k < SubFrames; k++) {

		/*
		 * Do windowing
		 */

		/* Get block of 180 samples centered around current subframe */
		for (i = 0; i < LpcFrame; i++)
			Vect[i] = Dpnt[k * SubFrLen + i];

		/* Normalize */
		ShAcf_sf[k] = Vec_Norm(Vect, (Word16) LpcFrame);

		/* Apply the Hamming window */
		for (i = 0; i < LpcFrame; i++)
			Vect[i] = g723_mult_r(Vect[i], HammingWindowTable[i]);

		/*
		 * Compute the autocorrelation coefficients
		 */

		/* Compute the zeroth-order coefficient (energy) */
		Acc1 = (Word32) 0;
		for (i = 0; i < LpcFrame; i++) {
			Acc0 = L_g723_mult(Vect[i], Vect[i]);
			Acc0 = L_g723_shr(Acc0, (Word16) 1);
			Acc1 = L_g723_add(Acc1, Acc0);
		}

		/* Apply a white noise correction factor of (1025/1024) */
		Acc0 = L_g723_shr(Acc1, (Word16) RidgeFact);
		Acc1 = L_g723_add(Acc1, Acc0);

		/* Normalize the energy */
		Exp = g723_norm_l(Acc1);
		Acc1 = L_g723_shl(Acc1, Exp);

		curAcf[0] = round_(Acc1);
		if (curAcf[0] == 0) {
			for (i = 1; i <= LpcOrder; i++)
				curAcf[i] = 0;
			ShAcf_sf[k] = 40;
		}

		else {
			/* Compute the rest of the autocorrelation coefficients.
			   Multiply them by a binomial coefficients lag window. */
			for (i = 1; i <= LpcOrder; i++) {
				Acc1 = (Word32) 0;
				for (j = i; j < LpcFrame; j++) {
					Acc0 =
					    L_g723_mult(Vect[j], Vect[j - i]);
					Acc0 = L_g723_shr(Acc0, (Word16) 1);
					Acc1 = L_g723_add(Acc1, Acc0);
				}
				Acc0 = L_g723_shl(Acc1, Exp);
				Acc0 = L_mls(Acc0, BinomialWindowTable[i - 1]);
				curAcf[i] = round_(Acc0);
			}
			/* Save Acf scaling factor */
			ShAcf_sf[k] = g723_add(Exp, g723_shl(ShAcf_sf[k], 1));
		}

		/*
		 * Apply the Levinson-Durbin algorithm to generate the LPC
		 * coefficients
		 */
		Durbin(&UnqLpc[k * LpcOrder], &curAcf[1], curAcf[0], &Pk2);
		CodStat.SinDet <<= 1;
		if (Pk2 > 0x799a) {
			CodStat.SinDet++;
		}
		curAcf += LpcOrderP1;
	}

	/* Update sine detector */
	CodStat.SinDet &= 0x7fff;

	j = CodStat.SinDet;
	k = 0;
	for (i = 0; i < 15; i++) {
		k += j & 1;
		j >>= 1;
	}
	if (k >= 14)
		CodStat.SinDet |= 0x8000;

	/* Update CNG Acf memories */
	Update_Acf(Acf_sf, ShAcf_sf);

}

/*
**
** Function:        Durbin()
**
** Description:     Implements the Levinson-Durbin algorithm for a
**          subframe.  The Levinson-Durbin algorithm
**          recursively computes the minimum mean-squared
**          error (MMSE) linear prediction filter based on the
**          estimated autocorrelation coefficients.
**
** Links to text:   Section 2.4
**
** Arguments:       
**
**  Word16 *Lpc Empty buffer
**  Word16 Corr[]   First- through tenth-order autocorrelations (10 words)
**  Word16 Err  Zeroth-order autocorrelation, or energy
**
** Outputs:     
**
**  Word16 Lpc[]    LPC coefficients (10 words)
**
** Return value:    The error
**
*/
Word16 Durbin(Word16 * Lpc, Word16 * Corr, Word16 Err, Word16 * Pk2)
{
	int i, j;

	Word16 Temp[LpcOrder];
	Word16 Pk;

	Word32 Acc0, Acc1, Acc2;

	/*
	 * Initialize the LPC vector
	 */
	for (i = 0; i < LpcOrder; i++)
		Lpc[i] = (Word16) 0;

	/*
	 * Do the recursion.  At the ith step, the algorithm computes the
	 * (i+1)th - order MMSE linear prediction filter.
	 */
	for (i = 0; i < LpcOrder; i++) {

/*
 * Compute the partial correlation (parcor) coefficient
 */

		/* Start parcor computation */
		Acc0 = g723_L_deposit_h(Corr[i]);
		Acc0 = L_g723_shr(Acc0, (Word16) 2);
		for (j = 0; j < i; j++)
			Acc0 = g723_L_msu(Acc0, Lpc[j], Corr[i - j - 1]);
		Acc0 = L_g723_shl(Acc0, (Word16) 2);

		/* Save sign */
		Acc1 = Acc0;
		Acc0 = g723_L_abs(Acc0);

		/* Finish parcor computation */
		Acc2 = g723_L_deposit_h(Err);
		if (Acc0 >= Acc2) {
			*Pk2 = 32767;
			break;
		}

		Pk = div_l(Acc0, Err);

		if (Acc1 >= 0)
			Pk = g723_negate(Pk);

		/*
		 * Sine detector
		 */
		if (i == 1)
			*Pk2 = Pk;

		/*
		 * Compute the ith LPC coefficient
		 */
		Acc0 = g723_L_deposit_h(g723_negate(Pk));
		Acc0 = L_g723_shr(Acc0, (Word16) 2);
		Lpc[i] = round_(Acc0);

		/*
		 * Update the prediction error
		 */
		Acc1 = L_mls(Acc1, Pk);
		Acc1 = L_g723_add(Acc1, Acc2);
		Err = round_(Acc1);

		/*
		 * Compute the remaining LPC coefficients
		 */
		for (j = 0; j < i; j++)
			Temp[j] = Lpc[j];

		for (j = 0; j < i; j++) {
			Acc0 = g723_L_deposit_h(Lpc[j]);
			Acc0 = g723_L_mac(Acc0, Pk, Temp[i - j - 1]);
			Lpc[j] = round_(Acc0);
		}
	}

	return Err;
}

/*
**
** Function:        Wght_Lpc()
**
** Description:     Computes the formant perceptual weighting
**          filter coefficients for a frame.  These
**          coefficients are geometrically scaled versions
**          of the unquantized LPC coefficients.
**
** Links to text:   Section 2.8  
**
** Arguments:       
**
**  Word16 *PerLpc      Empty Buffer
**  Word16 UnqLpc[]     Unquantized LPC coefficients (40 words)
**
** Outputs:     

**
**  Word16 PerLpc[]     Perceptual weighting filter coefficients
**              (80 words)
**
** Return value:    None
**
*/
void Wght_Lpc(Word16 * PerLpc, Word16 * UnqLpc)
{
	int i, j;

	/*
	 * Do for all subframes
	 */
	for (i = 0; i < SubFrames; i++) {

		/*
		 * Compute the jth FIR coefficient by multiplying the jth LPC
		 * coefficient by (0.9)^j.
		 */
		for (j = 0; j < LpcOrder; j++)
			PerLpc[j] = g723_mult_r(UnqLpc[j], PerFiltZeroTable[j]);
		PerLpc += LpcOrder;

/*
 * Compute the jth IIR coefficient by multiplying the jth LPC
 * coefficient by (0.5)^j.
 */
		for (j = 0; j < LpcOrder; j++)
			PerLpc[j] = g723_mult_r(UnqLpc[j], PerFiltPoleTable[j]);
		PerLpc += LpcOrder;
		UnqLpc += LpcOrder;
	}
}

/*
**
** Function:        Error_Wght()
**
** Description:     Implements the formant perceptual weighting
**          filter for a frame. This filter effectively
**          deemphasizes the formant frequencies in the
**          error signal.
**
** Links to text:   Section 2.8
**
** Arguments:
**
**  Word16 Dpnt[]       Highpass filtered speech x[n] (240 words)
**  Word16 PerLpc[]     Filter coefficients (80 words)
**
** Inputs:
**
**  CodStat.WghtFirDl[] FIR filter memory from previous frame (10 words)
**  CodStat.WghtIirDl[] IIR filter memory from previous frame (10 words)

**
** Outputs:
**
**  Word16 Dpnt[]       Weighted speech f[n] (240 words)
**
** Return value:    None
**
*/
void Error_Wght(Word16 * Dpnt, Word16 * PerLpc)
{
	int i, j, k;

	Word32 Acc0;

/*
 * Do for all subframes
 */
	for (k = 0; k < SubFrames; k++) {

		for (i = 0; i < SubFrLen; i++) {

/*
 * Do the FIR part
 */
			/* Filter */
			Acc0 = L_g723_mult(*Dpnt, (Word16) 0x2000);
			for (j = 0; j < LpcOrder; j++)
				Acc0 =
				    g723_L_msu(Acc0, PerLpc[j],
					       CodStat.WghtFirDl[j]);

			/* Update memory */
			for (j = LpcOrder - 1; j > 0; j--)
				CodStat.WghtFirDl[j] = CodStat.WghtFirDl[j - 1];
			CodStat.WghtFirDl[0] = *Dpnt;

			/*
			 * Do the IIR part
			 */

			/* Filter */
			for (j = 0; j < LpcOrder; j++)
				Acc0 = g723_L_mac(Acc0, PerLpc[LpcOrder + j],
						  CodStat.WghtIirDl[j]);
			for (j = LpcOrder - 1; j > 0; j--)
				CodStat.WghtIirDl[j] = CodStat.WghtIirDl[j - 1];
			Acc0 = L_g723_shl(Acc0, (Word16) 2);

			/* Update memory */
			CodStat.WghtIirDl[0] = round_(Acc0);
			*Dpnt++ = CodStat.WghtIirDl[0];
		}
		PerLpc += 2 * LpcOrder;
	}
}

/*
**
** Function:        Comp_Ir()
**
** Description:     Computes the combined impulse response of the
**          formant perceptual weighting filter, harmonic
**          noise shaping filter, and synthesis filter for
**          a subframe.
**
** Links to text:   Section 2.12
**
** Arguments:
**
**  Word16 *ImpResp     Empty Buffer
**  Word16 QntLpc[]     Quantized LPC coefficients (10 words)
**  Word16 PerLpc[]     Perceptual filter coefficients (20 words)
**  PWDEF Pw        Harmonic noise shaping filter parameters
**
** Outputs:
**
**  Word16 ImpResp[]    Combined impulse response (60 words)
**
** Return value:    None
**
*/
void Comp_Ir(Word16 * ImpResp, Word16 * QntLpc, Word16 * PerLpc, PWDEF Pw)
{
	int i, j;

	Word16 FirDl[LpcOrder];
	Word16 IirDl[LpcOrder];
	Word16 Temp[PitchMax + SubFrLen];

	Word32 Acc0, Acc1;

	/*
	 * Clear all memory.  Impulse response calculation requires
	 * an all-zero initial state.
	 */

	/* Perceptual weighting filter */
	for (i = 0; i < LpcOrder; i++)
		FirDl[i] = IirDl[i] = (Word16) 0;

	/* Harmonic noise shaping filter */
	for (i = 0; i < PitchMax + SubFrLen; i++)
		Temp[i] = (Word16) 0;

	/*
	 * Input a single impulse
	 */
	Acc0 = (Word32) 0x04000000L;

	/*
	 * Do for all elements in a subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, QntLpc[j], FirDl[j]);
		Acc1 = L_g723_shl(Acc0, (Word16) 2);

		/*
		 * Perceptual weighting filter
		 */

		/* FIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_msu(Acc0, PerLpc[j], FirDl[j]);
		Acc0 = L_g723_shl(Acc0, (Word16) 1);
		for (j = LpcOrder - 1; j > 0; j--)
			FirDl[j] = FirDl[j - 1];
		FirDl[0] = round_(Acc1);

		/* Iir part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, PerLpc[LpcOrder + j], IirDl[j]);
		for (j = LpcOrder - 1; j > 0; j--)
			IirDl[j] = IirDl[j - 1];
		Acc0 = L_g723_shl(Acc0, (Word16) 2);
		IirDl[0] = round_(Acc0);
		Temp[PitchMax + i] = IirDl[0];

		/*
		 * Harmonic noise shaping filter
		 */

		Acc0 = g723_L_deposit_h(IirDl[0]);
		Acc0 = g723_L_msu(Acc0, Pw.Gain, Temp[PitchMax - Pw.Indx + i]);
		ImpResp[i] = round_(Acc0);

		Acc0 = (Word32) 0;
	}
}

/*
**
** Function:        Sub_Ring()
**
** Description:     Computes the zero-input response of the
**          combined formant perceptual weighting filter,
**          harmonic noise shaping filter, and synthesis
**          filter for a subframe.  Subtracts the
**          zero-input response from the harmonic noise
**          weighted speech vector to produce the target 
**          speech vector.
**
** Links to text:   Section 2.13
**
** Arguments:       
**
**  Word16 Dpnt[]       Harmonic noise weighted vector w[n] (60 words)
**  Word16 QntLpc[]     Quantized LPC coefficients (10 words)
**  Word16 PerLpc[]     Perceptual filter coefficients (20 words)
**  Word16 PrevErr[]    Harmonic noise shaping filter memory (145 words)
**  PWDEF Pw        Harmonic noise shaping filter parameters
**  
** Inputs:
**
**  CodStat.RingFirDl[] Perceptual weighting filter FIR memory from 
**               previous subframe (10 words)
**  CodStat.RingIirDl[] Perceptual weighting filter IIR memory from 
**               previous subframe (10 words)
**
** Outputs:     
**
**  Word16 Dpnt[]       Target vector t[n] (60 words)
**
** Return value:    None
**
*/
void Sub_Ring(Word16 * Dpnt, Word16 * QntLpc, Word16 * PerLpc, Word16
	      * PrevErr, PWDEF Pw)
{
	int i, j;
	Word32 Acc0, Acc1;

	Word16 FirDl[LpcOrder];
	Word16 IirDl[LpcOrder];
	Word16 Temp[PitchMax + SubFrLen];

	/*
	 * Initialize the memory
	 */
	for (i = 0; i < PitchMax; i++)
		Temp[i] = PrevErr[i];

	for (i = 0; i < LpcOrder; i++) {
		FirDl[i] = CodStat.RingFirDl[i];
		IirDl[i] = CodStat.RingIirDl[i];
	}

	/*
	 * Do for all elements in a subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input zero
		 */
		Acc0 = (Word32) 0;

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, QntLpc[j], FirDl[j]);
		Acc1 = L_g723_shl(Acc0, (Word16) 2);

		/*
		 * Perceptual weighting filter
		 */

		/* Fir part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_msu(Acc0, PerLpc[j], FirDl[j]);
		for (j = LpcOrder - 1; j > 0; j--)
			FirDl[j] = FirDl[j - 1];
		FirDl[0] = round_(Acc1);

		/* Iir part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, PerLpc[LpcOrder + j], IirDl[j]);
		Acc0 = L_g723_shl(Acc0, (Word16) 2);
		for (j = LpcOrder - 1; j > 0; j--)
			IirDl[j] = IirDl[j - 1];
		IirDl[0] = round_(Acc0);
		Temp[PitchMax + i] = IirDl[0];

		/*
		 * Do the harmonic noise shaping filter and subtract the result
		 * from the harmonic noise weighted vector.
		 */
		Acc0 = g723_L_deposit_h(g723_sub(Dpnt[i], IirDl[0]));
		Acc0 =
		    g723_L_mac(Acc0, Pw.Gain,
			       Temp[PitchMax - (int)Pw.Indx + i]);
		Dpnt[i] = round_(Acc0);
	}
}

/*
**
** Function:        Upd_Ring()
**
** Description:     Updates the memory of the combined formant
**          perceptual weighting filter, harmonic noise
**          shaping filter, and synthesis filter for a
**          subframe.  The update is done by passing the
**          current subframe's excitation through the
**          combined filter.
**
** Links to text:   Section 2.19
**
** Arguments:       
**
**  Word16 Dpnt[]       Decoded excitation for the current subframe e[n] 
**               (60 words)
**  Word16 QntLpc[]     Quantized LPC coefficients (10 words)
**  Word16 PerLpc[]     Perceptual filter coefficients (20 words)
**  Word16 PrevErr[]    Harmonic noise shaping filter memory (145 words)
**  
** Inputs:
**
**  CodStat.RingFirDl[] Perceptual weighting filter FIR memory from 

**               previous subframe (10 words)
**  CodStat.RingIirDl[] Perceptual weighting filter IIR memory from 
**               previous subframe (10 words)
**
** Outputs:     
**
**  Word16 PrevErr[]    Updated harmonic noise shaping filter memory 
**  CodStat.RingFirDl[] Updated perceptual weighting filter FIR memory 
**  CodStat.RingIirDl[] Updated perceptual weighting filter IIR memory 
**
** Return value:    None
**
*/
void Upd_Ring(Word16 * Dpnt, Word16 * QntLpc, Word16 * PerLpc, Word16 * PrevErr)
{
	int i, j;

	Word32 Acc0, Acc1;

	/*
	 * Shift the harmonic noise shaping filter memory
	 */
	for (i = SubFrLen; i < PitchMax; i++)
		PrevErr[i - SubFrLen] = PrevErr[i];

	/*
	 * Do for all elements in the subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input the current subframe's excitation
		 */
		Acc0 = g723_L_deposit_h(Dpnt[i]);
		Acc0 = L_g723_shr(Acc0, (Word16) 3);

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_mac(Acc0, QntLpc[j], CodStat.RingFirDl[j]);
		Acc1 = L_g723_shl(Acc0, (Word16) 2);

		Dpnt[i] = g723_shl(round_(Acc1), (Word16) 1);

		/*
		 * Perceptual weighting filter
		 */

		/* FIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_msu(Acc0, PerLpc[j], CodStat.RingFirDl[j]);

		/* Update FIR memory */
		for (j = LpcOrder - 1; j > 0; j--)
			CodStat.RingFirDl[j] = CodStat.RingFirDl[j - 1];
		CodStat.RingFirDl[0] = round_(Acc1);

		/* IIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_mac(Acc0, PerLpc[LpcOrder + j],
				       CodStat.RingIirDl[j]);
		Acc0 = L_g723_shl(Acc0, (Word16) 2);

		/* Update IIR memory */
		for (j = LpcOrder - 1; j > 0; j--)
			CodStat.RingIirDl[j] = CodStat.RingIirDl[j - 1];
		CodStat.RingIirDl[0] = round_(Acc0);

		/* Update harmonic noise shaping memory */
		PrevErr[PitchMax - SubFrLen + i] = CodStat.RingIirDl[0];
	}
}

/*
**
** Function:        Synt()
**
** Description:     Implements the decoder synthesis filter for a
**          subframe.  This is a tenth-order IIR filter.
**
** Links to text:   Section 3.7
**
** Arguments:       
**
**  Word16 Dpnt[]       Pitch-postfiltered excitation for the current
**               subframe ppf[n] (60 words)
**  Word16 Lpc[]        Quantized LPC coefficients (10 words)
**  
** Inputs:
**
**  DecStat.SyntIirDl[] Synthesis filter memory from previous
subframe (10 words)
**
** Outputs:     
**
**  Word16 Dpnt[]       Synthesized speech vector sy[n]
**  DecStat.SyntIirDl[] Updated synthesis filter memory 
**
** Return value:    None
**
*/
void Synt(Word16 * Dpnt, Word16 * Lpc)
{
	int i, j;

	Word32 Acc0;

	/*
	 * Do for all elements in the subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input the current subframe's excitation
		 */
		Acc0 = g723_L_deposit_h(Dpnt[i]);
		Acc0 = L_g723_shr(Acc0, (Word16) 3);

		/*
		 * Synthesis
		 */

		/* Filter */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, Lpc[j], DecStat.SyntIirDl[j]);

		/* Update memory */
		for (j = LpcOrder - 1; j > 0; j--)
			DecStat.SyntIirDl[j] = DecStat.SyntIirDl[j - 1];

		Acc0 = L_g723_shl(Acc0, (Word16) 2);

		DecStat.SyntIirDl[0] = round_(Acc0);

		/*
		 * Scale output if postfilter is off.  (Otherwise output is
		 * scaled by the gain scaling unit.)
		 */
		if (UsePf)
			Dpnt[i] = DecStat.SyntIirDl[0];
		else
			Dpnt[i] = g723_shl(DecStat.SyntIirDl[0], (Word16) 1);

	}

}

/*
**
** Function:        Spf()
**
** Description:     Implements the formant postfilter for a
**          subframe.  The formant postfilter is a
**          10-pole, 10-zero ARMA filter followed by a
**          single-tap tilt compensation filter.
**
** Links to text:   Section 3.8
**
** Arguments:
**
**  Word16 Tv[]     Synthesized speech vector sy[n] (60 words)
**  Word16 Lpc[]        Quantized LPC coefficients (10 words)
**
** Inputs:
**
**  DecStat.PostIirDl[] Postfilter IIR memory from previous
subframe (10 words)
**  DecStat.PostFirDl[] Postfilter FIR memory from previous
subframe (10 words)
**  DecStat.Park        Previous value of compensation filter parameter
**
** Outputs:
**
**  Word16 Tv[]     Postfiltered speech vector pf[n] (60 words)
**  DecStat.PostIirDl[] Updated postfilter IIR memory
**  DecStat.PostFirDl[] Updated postfilter FIR memory
**  DecStat.Park        Updated compensation filter parameter
**
** Return value: Input vector energy
**
*/
Word32 Spf(Word16 * Tv, Word16 * Lpc)
{
	int i, j;

	Word32 Acc0, Acc1;
	Word32 Sen;
	Word16 Tmp;
	Word16 Exp;

	Word16 FirCoef[LpcOrder];
	Word16 IirCoef[LpcOrder];

	Word16 TmpVect[SubFrLen];

	/*
	 * Compute ARMA coefficients.  Compute the jth FIR coefficient by
	 * multiplying the jth quantized LPC coefficient by (0.65)^j.
	 * Compute the jth IIR coefficient by multiplying the jth quantized
	 * LPC coefficient by (0.75)^j.  This emphasizes the formants in
	 * the frequency response.
	 */
	for (i = 0; i < LpcOrder; i++) {
		FirCoef[i] = g723_mult_r(Lpc[i], PostFiltZeroTable[i]);
		IirCoef[i] = g723_mult_r(Lpc[i], PostFiltPoleTable[i]);
	}

	/*
	 * Normalize the speech vector.
	 */
	for (i = 0; i < SubFrLen; i++)
		TmpVect[i] = Tv[i];
	Exp = Vec_Norm(TmpVect, (Word16) SubFrLen);

	/*
	 * Compute the first two autocorrelation coefficients R[0] and R[1]
	 */
	Acc0 = (Word32) 0;
	Acc1 = L_g723_mult(TmpVect[0], TmpVect[0]);
	for (i = 1; i < SubFrLen; i++) {
		Acc0 = g723_L_mac(Acc0, TmpVect[i], TmpVect[i - 1]);
		Acc1 = g723_L_mac(Acc1, TmpVect[i], TmpVect[i]);
	}

	/*
	 * Scale the energy for the later use.
	 */
	Sen = L_g723_shr(Acc1, (Word16) (2 * Exp + 4));

	/*
	 * Compute the first-order partial correlation coefficient of the
	 * input speech vector.
	 */
	Tmp = g723_extract_h(Acc1);
	if (Tmp != (Word16) 0) {

		/* Compute first parkor */
		Acc0 = L_g723_shr(Acc0, (Word16) 1);
		Acc1 = Acc0;
		Acc0 = g723_L_abs(Acc0);

		Tmp = div_l(Acc0, Tmp);

		if (Acc1 < (Word32) 0)
			Tmp = g723_negate(Tmp);
	} else
		Tmp = (Word16) 0;

	/*
	 * Compute the compensation filter parameter and update the memory
	 */
	Acc0 = g723_L_deposit_h(DecStat.Park);
	Acc0 = g723_L_msu(Acc0, DecStat.Park, (Word16) 0x2000);
	Acc0 = g723_L_mac(Acc0, Tmp, (Word16) 0x2000);
	DecStat.Park = round_(Acc0);

	Tmp = g723_mult(DecStat.Park, PreCoef);
	Tmp &= (Word16) 0xfffc;

	/*
	 *  Do for all elements in the subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input the speech vector
		 */
		Acc0 = g723_L_deposit_h(Tv[i]);
		Acc0 = L_g723_shr(Acc0, (Word16) 2);

		/*
		 * Formant postfilter
		 */

		/* FIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_msu(Acc0, FirCoef[j], DecStat.PostFirDl[j]);

		/* Update FIR memory */
		for (j = LpcOrder - 1; j > 0; j--)
			DecStat.PostFirDl[j] = DecStat.PostFirDl[j - 1];
		DecStat.PostFirDl[0] = Tv[i];

		/* IIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_mac(Acc0, IirCoef[j], DecStat.PostIirDl[j]);

		/* Update IIR memory */
		for (j = LpcOrder - 1; j > 0; j--)
			DecStat.PostIirDl[j] = DecStat.PostIirDl[j - 1];

		Acc0 = L_g723_shl(Acc0, (Word16) 2);
		Acc1 = Acc0;

		DecStat.PostIirDl[0] = round_(Acc0);

		/*
		 * Compensation filter
		 */
		Acc1 = g723_L_mac(Acc1, DecStat.PostIirDl[1], Tmp);

		Tv[i] = round_(Acc1);
	}
	return Sen;
}
