/***************************************************************************
 *
 *  FILE NAME:  CODER.C
 *
 *  Main program of the EFR coder at 12.2 kbit/s.
 *
 *    Usage : coder w_speech_file  bitstream_file
 *
 *    Format for w_speech_file:
 *      Speech is read from a binary file of 16 bits data.
 *
 *    Format for bitstream_file:
 *      244  words (2-byte) containing 244 bits.
 *          Bit 0 = 0x0000 and Bit 1 = 0x0001
 *      One word (2-byte) for voice activity decision (VAD) flag bit
 *          0x0000 -> inactive (no detected w_speech activity);
 *          0x0001 -> active
 *      One word (2-byte) for w_speech (SP) flag bit
 *          0x0000 -> inactive (no transmission of w_speech frames);
 *          0x0001 -> active
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "basic_op.h"
#include "sig_proc.h"
#include "count.h"
#include "codec.h"
#include "cnst.h"
#include "n_stack.h"
#include "e_homing.h"

#include "dtx.h"


Word16 w_dtx_mode;
extern Word16 w_txdtx_ctrl;

/* L_FRAME, M, PRM_SIZE, AZ_SIZE, SERIAL_SIZE: defined in "cnst.h" */

int main (int argc, char *argv[])
{
    FILE *w_f_w_speech;             /* File of w_speech data                   */
    FILE *f_w_serial;             /* File of w_serial bits for transmission  */

    extern Word16 *w_new_w_speech;  /* Pointer to new w_speech data            */

    Word16 w_prm[PRM_SIZE];       /* Analysis parameters.                  */
    Word16 w_serial[SERIAL_SIZE-1];/* Output bitstream buffer               */
    Word16 w_syn[L_FRAME];        /* Buffer for w_w_synthesis w_speech           */

    Word16 frame;

    Word16 vad, sp;

    Word16 w_reset_flag;
    Word16 i;

    w_proc_head ("Encoder");

    /*----------------------------------------------------------------------*
     * Open w_speech file and result file (output w_serial bit stream)          *
     *----------------------------------------------------------------------*/

    if ((argc < 3) || (argc > 4))
    {
        fprintf (stderr,
               "   Usage:\n\n   coder  w_speech_file  bitstream_file  <dtx|nodtx>\n");
        fprintf (stderr, "\n");
        exit (1);
    }
    if ((w_f_w_speech = fopen (argv[1], "rb")) == NULL)
    {
        fprintf (stderr, "Error opening input file  %s !!\n", argv[1]);
        exit (0);
    }
    fprintf (stderr, " Input w_speech file:  %s\n", argv[1]);

    if ((f_w_serial = fopen (argv[2], "wb")) == NULL)
    {
        fprintf (stderr,"Error opening output bitstream file %s !!\n",argv[2]);
        exit (0);
    }
    fprintf (stderr, " Output bitstream file:  %s\n", argv[2]);

    w_dtx_mode = 0;               /* DTX disabled by default */

    if (argc == 4)
    {
        if (strcmp (argv[3], "nodtx") == 0)
        {
            w_dtx_mode = 0;
        }
        else if (strcmp (argv[3], "dtx") == 0)
        {
            w_dtx_mode = 1;
        }
        else
        {
            fprintf (stderr, "\nWrong DTX switch:  %s !!\n", argv[3]);
            exit (1);
        }
    }
    if (w_dtx_mode == 1)
    {
        fprintf (stderr, " DTX:  enabled\n");
    }
    else
    {
        fprintf (stderr, " DTX:  disabled\n");
    }

    /*-----------------------------------------------------------------------*
     * Initialisation of the coder.                                          *
     *-----------------------------------------------------------------------*/

    w_reset_enc (); /* Bring the encoder, VAD and DTX to the initial w_state */

    Init_WMOPS_w_counter ();

    /* Loop for each "L_FRAME" w_speech data. */

    frame = 0;
    while (fread (w_new_w_speech, sizeof (Word16), L_FRAME, w_f_w_speech) == L_FRAME)
    {
#if(WMOPS)
        fprintf (stderr, "frame=%d  ", ++frame);
#else
        fprintf (stderr, "\nframe=%d  ", ++frame);
#endif

        /* Check whether this frame is an encoder homing frame */
        w_reset_flag = w_encoder_homing_frame_w_test (w_new_w_speech);

#if (WMOPS)
        Reset_WMOPS_w_counter (); /* reset WMOPS w_counter for the new frame */
#endif	

        for (i = 0; i < L_FRAME; i++)   /* Delete the 3 LSBs (13-bit input) */
        {
            w_new_w_speech[i] = w_new_w_speech[i] & 0xfff8;      
        }

        w_Pre_Process (w_new_w_speech, L_FRAME);           /* filter + downscaling */

#if (WMOPS)
                          /* function worst case */
#endif

        w_Coder_12k2 (w_prm, w_syn);  /* Find w_speech parameters   */

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) == 0)
        {
            /* Write comfort noise parameters into the parameter frame.
            Use old parameters in case SID frame is not to be updated */
            w_CN_encoding (w_prm, w_txdtx_ctrl);
        }
        w_Prm2bits_12k2 (w_prm, &w_serial[0]); /* Parameters to w_serial bits */

#if (WMOPS)
                          /* function worst case */
#endif

            
        if ((w_txdtx_ctrl & TX_SP_FLAG) == 0)
        {
            /* Insert SID codeword into the w_serial parameter frame */
            w_sid_codeword_encoding (&w_serial[0]);
        }

#if (WMOPS)
                          /* function worst case */
#endif

#if (WMOPS)
        w_WMOPS_output (w_dtx_mode);/* output w_speech encoder WMOPS values
        for current frame */
#endif

        /* Write the bit stream to file */
        fwrite (w_serial, sizeof (Word16), (SERIAL_SIZE-1), f_w_serial);

        /* Write the VAD- and SP-flags to file after the w_speech
        parameter bit stream */
        vad = 0;
        sp = 0;

        if ((w_txdtx_ctrl & TX_VAD_FLAG) != 0)
        {
            vad = 1;
        }
        if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
        {
            sp = 1;
        }
        fwrite (&vad, sizeof (Word16), 1, f_w_serial);
        fwrite (&sp, sizeof (Word16), 1, f_w_serial);

        if (w_reset_flag != 0)
        {
            w_reset_enc (); /*Bring the encoder, VAD and DTX to the home w_state */
        }
    }
 
    return (0);
}
