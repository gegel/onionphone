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

extern int16_t PerFiltZeroTable[LpcOrder];
extern int16_t PerFiltPoleTable[LpcOrder];
extern CODSTATDEF CodStat;
extern DECSTATDEF DecStat;
extern int16_t PostFiltZeroTable[LpcOrder];
extern int16_t PostFiltPoleTable[LpcOrder];
extern int16_t HammingWindowTable[LpcFrame];
extern int16_t BinomialWindowTable[LpcOrder];
extern int UsePf;

extern int16_t Vec_Norm(int16_t * Vect, int16_t Len);
extern int16_t g723_mult_r(int16_t var1, int16_t var2);	/* Mult with round,     2 */
extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern int32_t L_g723_shr(int32_t L_var1, int16_t var2);	/* Long shift right,    2 */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */
extern int16_t g723_norm_l(int32_t L_var1);	/* Long norm,            30 */
extern int32_t L_g723_shl(int32_t L_var1, int16_t var2);	/* Long shift left,     2 */
extern int16_t round_(int32_t L_var1);	/* Round,               1 */
extern int32_t L_mls(int32_t, int16_t);	/* Wght ?? */
extern int16_t g723_add(int16_t var1, int16_t var2);	/* Short add,           1 */
extern int16_t g723_shl(int16_t var1, int16_t var2);	/* Short shift left,    1 */

int16_t Durbin(int16_t * Lpc, int16_t * Corr, int16_t Err, int16_t * Pk2);
extern void Update_Acf(int16_t * Acfsf, int16_t * Shsf);

extern int32_t g723_L_deposit_h(int16_t var1);	/* 16 bit var1 -> MSB,     2 */
extern int32_t g723_L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* Msu,    1 */
extern int32_t g723_L_abs(int32_t L_var1);	/* Long abs,              3 */
extern int16_t div_l(int32_t, int16_t);
extern int16_t g723_negate(int16_t var1);	/* Short negate,        1 */
extern int32_t g723_L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* Mac,    1 */
extern int16_t g723_sub(int16_t var1, int16_t var2);	/* Short sub,           1 */
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int16_t g723_mult(int16_t var1, int16_t var2);	/* Short mult,          1 */
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
**  int16_t *UnqLpc      Empty Buffer
**  int16_t PrevDat[]    Previous 2 subframes of samples (120 words)
**  int16_t DataBuff[]   Current frame of samples (240 words)
**
** Outputs:

**
**  int16_t UnqLpc[]     LPC coefficients for entire frame (40 words)
**
** Return value:    None
**
*/
void Comp_Lpc(int16_t * UnqLpc, int16_t * PrevDat, int16_t * DataBuff)
{
	int i, j, k;

	int16_t Dpnt[Frame + LpcFrame - SubFrLen];
	int16_t Vect[LpcFrame];
	int16_t Acf_sf[LpcOrderP1 * SubFrames];
	int16_t ShAcf_sf[SubFrames];
	int16_t Exp;
	int16_t *curAcf;
	int16_t Pk2;

	int32_t Acc0, Acc1;

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
		ShAcf_sf[k] = Vec_Norm(Vect, (int16_t) LpcFrame);

		/* Apply the Hamming window */
		for (i = 0; i < LpcFrame; i++)
			Vect[i] = g723_mult_r(Vect[i], HammingWindowTable[i]);

		/*
		 * Compute the autocorrelation coefficients
		 */

		/* Compute the zeroth-order coefficient (energy) */
		Acc1 = (int32_t) 0;
		for (i = 0; i < LpcFrame; i++) {
			Acc0 = L_g723_mult(Vect[i], Vect[i]);
			Acc0 = L_g723_shr(Acc0, (int16_t) 1);
			Acc1 = L_g723_add(Acc1, Acc0);
		}

		/* Apply a white noise correction factor of (1025/1024) */
		Acc0 = L_g723_shr(Acc1, (int16_t) RidgeFact);
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
				Acc1 = (int32_t) 0;
				for (j = i; j < LpcFrame; j++) {
					Acc0 =
					    L_g723_mult(Vect[j], Vect[j - i]);
					Acc0 = L_g723_shr(Acc0, (int16_t) 1);
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
**  int16_t *Lpc Empty buffer
**  int16_t Corr[]   First- through tenth-order autocorrelations (10 words)
**  int16_t Err  Zeroth-order autocorrelation, or energy
**
** Outputs:     
**
**  int16_t Lpc[]    LPC coefficients (10 words)
**
** Return value:    The error
**
*/
int16_t Durbin(int16_t * Lpc, int16_t * Corr, int16_t Err, int16_t * Pk2)
{
	int i, j;

	int16_t Temp[LpcOrder];
	int16_t Pk;

	int32_t Acc0, Acc1, Acc2;

	/*
	 * Initialize the LPC vector
	 */
	for (i = 0; i < LpcOrder; i++)
		Lpc[i] = (int16_t) 0;

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
		Acc0 = L_g723_shr(Acc0, (int16_t) 2);
		for (j = 0; j < i; j++)
			Acc0 = g723_L_msu(Acc0, Lpc[j], Corr[i - j - 1]);
		Acc0 = L_g723_shl(Acc0, (int16_t) 2);

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
		Acc0 = L_g723_shr(Acc0, (int16_t) 2);
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
**  int16_t *PerLpc      Empty Buffer
**  int16_t UnqLpc[]     Unquantized LPC coefficients (40 words)
**
** Outputs:     

**
**  int16_t PerLpc[]     Perceptual weighting filter coefficients
**              (80 words)
**
** Return value:    None
**
*/
void Wght_Lpc(int16_t * PerLpc, int16_t * UnqLpc)
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
**  int16_t Dpnt[]       Highpass filtered speech x[n] (240 words)
**  int16_t PerLpc[]     Filter coefficients (80 words)
**
** Inputs:
**
**  CodStat.WghtFirDl[] FIR filter memory from previous frame (10 words)
**  CodStat.WghtIirDl[] IIR filter memory from previous frame (10 words)

**
** Outputs:
**
**  int16_t Dpnt[]       Weighted speech f[n] (240 words)
**
** Return value:    None
**
*/
void Error_Wght(int16_t * Dpnt, int16_t * PerLpc)
{
	int i, j, k;

	int32_t Acc0;

/*
 * Do for all subframes
 */
	for (k = 0; k < SubFrames; k++) {

		for (i = 0; i < SubFrLen; i++) {

/*
 * Do the FIR part
 */
			/* Filter */
			Acc0 = L_g723_mult(*Dpnt, (int16_t) 0x2000);
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
			Acc0 = L_g723_shl(Acc0, (int16_t) 2);

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
**  int16_t *ImpResp     Empty Buffer
**  int16_t QntLpc[]     Quantized LPC coefficients (10 words)
**  int16_t PerLpc[]     Perceptual filter coefficients (20 words)
**  PWDEF Pw        Harmonic noise shaping filter parameters
**
** Outputs:
**
**  int16_t ImpResp[]    Combined impulse response (60 words)
**
** Return value:    None
**
*/
void Comp_Ir(int16_t * ImpResp, int16_t * QntLpc, int16_t * PerLpc,
	     PWDEF Pw)
{
	int i, j;

	int16_t FirDl[LpcOrder];
	int16_t IirDl[LpcOrder];
	int16_t Temp[PitchMax + SubFrLen];

	int32_t Acc0, Acc1;

	/*
	 * Clear all memory.  Impulse response calculation requires
	 * an all-zero initial state.
	 */

	/* Perceptual weighting filter */
	for (i = 0; i < LpcOrder; i++)
		FirDl[i] = IirDl[i] = (int16_t) 0;

	/* Harmonic noise shaping filter */
	for (i = 0; i < PitchMax + SubFrLen; i++)
		Temp[i] = (int16_t) 0;

	/*
	 * Input a single impulse
	 */
	Acc0 = (int32_t) 0x04000000L;

	/*
	 * Do for all elements in a subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, QntLpc[j], FirDl[j]);
		Acc1 = L_g723_shl(Acc0, (int16_t) 2);

		/*
		 * Perceptual weighting filter
		 */

		/* FIR part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_msu(Acc0, PerLpc[j], FirDl[j]);
		Acc0 = L_g723_shl(Acc0, (int16_t) 1);
		for (j = LpcOrder - 1; j > 0; j--)
			FirDl[j] = FirDl[j - 1];
		FirDl[0] = round_(Acc1);

		/* Iir part */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, PerLpc[LpcOrder + j], IirDl[j]);
		for (j = LpcOrder - 1; j > 0; j--)
			IirDl[j] = IirDl[j - 1];
		Acc0 = L_g723_shl(Acc0, (int16_t) 2);
		IirDl[0] = round_(Acc0);
		Temp[PitchMax + i] = IirDl[0];

		/*
		 * Harmonic noise shaping filter
		 */

		Acc0 = g723_L_deposit_h(IirDl[0]);
		Acc0 = g723_L_msu(Acc0, Pw.Gain, Temp[PitchMax - Pw.Indx + i]);
		ImpResp[i] = round_(Acc0);

		Acc0 = (int32_t) 0;
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
**  int16_t Dpnt[]       Harmonic noise weighted vector w[n] (60 words)
**  int16_t QntLpc[]     Quantized LPC coefficients (10 words)
**  int16_t PerLpc[]     Perceptual filter coefficients (20 words)
**  int16_t PrevErr[]    Harmonic noise shaping filter memory (145 words)
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
**  int16_t Dpnt[]       Target vector t[n] (60 words)
**
** Return value:    None
**
*/
void Sub_Ring(int16_t * Dpnt, int16_t * QntLpc, int16_t * PerLpc, int16_t
	      * PrevErr, PWDEF Pw)
{
	int i, j;
	int32_t Acc0, Acc1;

	int16_t FirDl[LpcOrder];
	int16_t IirDl[LpcOrder];
	int16_t Temp[PitchMax + SubFrLen];

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
		Acc0 = (int32_t) 0;

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, QntLpc[j], FirDl[j]);
		Acc1 = L_g723_shl(Acc0, (int16_t) 2);

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
		Acc0 = L_g723_shl(Acc0, (int16_t) 2);
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
**  int16_t Dpnt[]       Decoded excitation for the current subframe e[n] 
**               (60 words)
**  int16_t QntLpc[]     Quantized LPC coefficients (10 words)
**  int16_t PerLpc[]     Perceptual filter coefficients (20 words)
**  int16_t PrevErr[]    Harmonic noise shaping filter memory (145 words)
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
**  int16_t PrevErr[]    Updated harmonic noise shaping filter memory 
**  CodStat.RingFirDl[] Updated perceptual weighting filter FIR memory 
**  CodStat.RingIirDl[] Updated perceptual weighting filter IIR memory 
**
** Return value:    None
**
*/
void Upd_Ring(int16_t * Dpnt, int16_t * QntLpc, int16_t * PerLpc,
	      int16_t * PrevErr)
{
	int i, j;

	int32_t Acc0, Acc1;

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
		Acc0 = L_g723_shr(Acc0, (int16_t) 3);

		/*
		 * Synthesis filter
		 */
		for (j = 0; j < LpcOrder; j++)
			Acc0 =
			    g723_L_mac(Acc0, QntLpc[j], CodStat.RingFirDl[j]);
		Acc1 = L_g723_shl(Acc0, (int16_t) 2);

		Dpnt[i] = g723_shl(round_(Acc1), (int16_t) 1);

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
		Acc0 = L_g723_shl(Acc0, (int16_t) 2);

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
**  int16_t Dpnt[]       Pitch-postfiltered excitation for the current
**               subframe ppf[n] (60 words)
**  int16_t Lpc[]        Quantized LPC coefficients (10 words)
**  
** Inputs:
**
**  DecStat.SyntIirDl[] Synthesis filter memory from previous
subframe (10 words)
**
** Outputs:     
**
**  int16_t Dpnt[]       Synthesized speech vector sy[n]
**  DecStat.SyntIirDl[] Updated synthesis filter memory 
**
** Return value:    None
**
*/
void Synt(int16_t * Dpnt, int16_t * Lpc)
{
	int i, j;

	int32_t Acc0;

	/*
	 * Do for all elements in the subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input the current subframe's excitation
		 */
		Acc0 = g723_L_deposit_h(Dpnt[i]);
		Acc0 = L_g723_shr(Acc0, (int16_t) 3);

		/*
		 * Synthesis
		 */

		/* Filter */
		for (j = 0; j < LpcOrder; j++)
			Acc0 = g723_L_mac(Acc0, Lpc[j], DecStat.SyntIirDl[j]);

		/* Update memory */
		for (j = LpcOrder - 1; j > 0; j--)
			DecStat.SyntIirDl[j] = DecStat.SyntIirDl[j - 1];

		Acc0 = L_g723_shl(Acc0, (int16_t) 2);

		DecStat.SyntIirDl[0] = round_(Acc0);

		/*
		 * Scale output if postfilter is off.  (Otherwise output is
		 * scaled by the gain scaling unit.)
		 */
		if (UsePf)
			Dpnt[i] = DecStat.SyntIirDl[0];
		else
			Dpnt[i] = g723_shl(DecStat.SyntIirDl[0], (int16_t) 1);

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
**  int16_t Tv[]     Synthesized speech vector sy[n] (60 words)
**  int16_t Lpc[]        Quantized LPC coefficients (10 words)
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
**  int16_t Tv[]     Postfiltered speech vector pf[n] (60 words)
**  DecStat.PostIirDl[] Updated postfilter IIR memory
**  DecStat.PostFirDl[] Updated postfilter FIR memory
**  DecStat.Park        Updated compensation filter parameter
**
** Return value: Input vector energy
**
*/
int32_t Spf(int16_t * Tv, int16_t * Lpc)
{
	int i, j;

	int32_t Acc0, Acc1;
	int32_t Sen;
	int16_t Tmp;
	int16_t Exp;

	int16_t FirCoef[LpcOrder];
	int16_t IirCoef[LpcOrder];

	int16_t TmpVect[SubFrLen];

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
	Exp = Vec_Norm(TmpVect, (int16_t) SubFrLen);

	/*
	 * Compute the first two autocorrelation coefficients R[0] and R[1]
	 */
	Acc0 = (int32_t) 0;
	Acc1 = L_g723_mult(TmpVect[0], TmpVect[0]);
	for (i = 1; i < SubFrLen; i++) {
		Acc0 = g723_L_mac(Acc0, TmpVect[i], TmpVect[i - 1]);
		Acc1 = g723_L_mac(Acc1, TmpVect[i], TmpVect[i]);
	}

	/*
	 * Scale the energy for the later use.
	 */
	Sen = L_g723_shr(Acc1, (int16_t) (2 * Exp + 4));

	/*
	 * Compute the first-order partial correlation coefficient of the
	 * input speech vector.
	 */
	Tmp = g723_extract_h(Acc1);
	if (Tmp != (int16_t) 0) {

		/* Compute first parkor */
		Acc0 = L_g723_shr(Acc0, (int16_t) 1);
		Acc1 = Acc0;
		Acc0 = g723_L_abs(Acc0);

		Tmp = div_l(Acc0, Tmp);

		if (Acc1 < (int32_t) 0)
			Tmp = g723_negate(Tmp);
	} else
		Tmp = (int16_t) 0;

	/*
	 * Compute the compensation filter parameter and update the memory
	 */
	Acc0 = g723_L_deposit_h(DecStat.Park);
	Acc0 = g723_L_msu(Acc0, DecStat.Park, (int16_t) 0x2000);
	Acc0 = g723_L_mac(Acc0, Tmp, (int16_t) 0x2000);
	DecStat.Park = round_(Acc0);

	Tmp = g723_mult(DecStat.Park, PreCoef);
	Tmp &= (int16_t) 0xfffc;

	/*
	 *  Do for all elements in the subframe
	 */
	for (i = 0; i < SubFrLen; i++) {

		/*
		 * Input the speech vector
		 */
		Acc0 = g723_L_deposit_h(Tv[i]);
		Acc0 = L_g723_shr(Acc0, (int16_t) 2);

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

		Acc0 = L_g723_shl(Acc0, (int16_t) 2);
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
