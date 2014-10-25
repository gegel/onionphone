/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:            "dec_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
** Functions:       Init_Dec_Cng()
**                  Dec_Cng()
**
**
*/
/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>

#include "g723_const.h"
/* Global Variable */
DECCNGDEF DecCng;
extern int16_t LspDcTable[LpcOrder];
extern DECSTATDEF DecStat;

extern void Lsp_Inq(int16_t * Lsp, int16_t * PrevLsp, int32_t LspId,
		    int16_t Crc);
extern int16_t Qua_SidGain(int16_t * Ener, int16_t * shEner, int16_t nq);
extern int16_t g723_extract_h(int32_t L_var1);	/* Extract high,        1 */
extern int32_t L_g723_add(int32_t L_var1, int32_t L_var2);	/* Long add,        2 */

extern int32_t L_g723_mult(int16_t var1, int16_t var2);	/* Long mult,           1 */
extern void Calc_Exc_Rand(int16_t cur_gain, int16_t * PrevExc,
			  int16_t * DataExc,
			  int16_t * nRandom, LINEDEF * Line);
extern void Lsp_Int(int16_t * QntLpc, int16_t * CurrLsp, int16_t * PrevLsp);
extern int16_t Dec_SidGain(int16_t i_gain);

/*
**
** Function:        Init_Dec_Cng()
**
** Description:     Initialize Dec_Cng static variables
**
** Links to text:
**
** Arguments:       None
**
** Outputs:         None
**
** Return value:    None
**
*/
void Init_Dec_Cng(void)
{
	int i;

	DecCng.PastFtyp = 1;
	DecCng.SidGain = 0;
	for (i = 0; i < LpcOrder; i++)
		DecCng.LspSid[i] = LspDcTable[i];
	DecCng.RandSeed = 12345;
	return;
}

/*
**
** Function:           Dec_Cng()
**
** Description:        Receives Ftyp
**                     0  :  for untransmitted frames
**                     2  :  for SID frames
**                     Decodes SID frames
**                     Computes current frame excitation
**                     Computes current frame LSPs
**
** Links to text:
**
** Arguments:
**
**  int16_t Ftyp        Type of silence frame
**  LINEDEF *Line      Coded parameters
**  int16_t *DataExc    Current frame excitation
**  int16_t *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  int16_t *DataExc
**  int16_t *QntLpc
**
** Return value:       None
**
*/
void Dec_Cng(int16_t Ftyp, LINEDEF * Line, int16_t * DataExc,
	     int16_t * QntLpc)
{

	int16_t temp;
	int i;

	if (Ftyp == 2) {

		/*
		 * SID Frame decoding
		 */
		DecCng.SidGain = Dec_SidGain(Line->Sfs[0].Mamp);

		/* Inverse quantization of the LSP */
		Lsp_Inq(DecCng.LspSid, DecStat.PrevLsp, Line->LspId, 0);
	}

	else {

/*
 * non SID Frame
 */
		if (DecCng.PastFtyp == 1) {

			/*
			 * Case of 1st SID frame erased : quantize-decode
			 * energy estimate stored in DecCng.SidGain
			 * scaling factor in DecCng.CurGain
			 */
			temp = Qua_SidGain(&DecCng.SidGain, &DecCng.CurGain, 0);
			DecCng.SidGain = Dec_SidGain(temp);
		}
	}

	if (DecCng.PastFtyp == 1) {
		DecCng.CurGain = DecCng.SidGain;
	} else {
		DecCng.CurGain =
		    g723_extract_h(L_g723_add
				   (L_g723_mult(DecCng.CurGain, 0x7000),
				    L_g723_mult(DecCng.SidGain, 0x1000)));
	}
	Calc_Exc_Rand(DecCng.CurGain, DecStat.PrevExc, DataExc,
		      &DecCng.RandSeed, Line);

	/* Interpolate the Lsp vectors */
	Lsp_Int(QntLpc, DecCng.LspSid, DecStat.PrevLsp);

	/* Copy the LSP vector for the next frame */
	for (i = 0; i < LpcOrder; i++)
		DecStat.PrevLsp[i] = DecCng.LspSid[i];

	return;
}
