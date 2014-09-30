/***************************************************************************
 *
 *  FILE NAME:  decoder.c
 *
 *         Usage : decoder  bitstream_file  w_w_synth_file
 *
 *         Format for bitstream_file:
 *           One word (2-byte) for bad frame indication (BFI) flag bit
 *               0x0000 -> good frame;  0x0001 -> bad frame
 *           244  words (2-byte) containing 244 bits.
 *               Bit 0 = 0x0000 and Bit 1 = 0x0001
 *           One word (2-byte) for ternary Silence Descriptor (SID) flag
 *               0x0000 -> inactive (no detected w_speech activity);
 *               0x0001 -> active
 *           One word (2-byte) for Time Alignment Flag (TAF) bit
 *               0x0000 -> inactive (no transmission of w_speech frames);
 *               0x0001 -> active
 *
 *         Format for w_w_synth_file:
 *           Synthesis is written to a binary file of 16 bits data.
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "n_stack.h"
#include "basic_op.h"
#include "sig_proc.h"
#include "count.h"
#include "codec.h"
#include "cnst.h"
#include "d_homing.h"


/* These constants define the number of consecutive parameters
   that function w_decoder_homing_frame_w_test() checks */

#define WHOLE_FRAME 57
#define TO_FIRST_SUBFRAME 18


Word16 synth_buf[L_FRAME + M];

/* L_FRAME, M, PRM_SIZE, AZ_SIZE, SERIAL_SIZE: defined in "cnst.h" */

/*-----------------------------------------------------------------*
 *             Global variables                                    *
 *-----------------------------------------------------------------*/

#if (WMOPS)
Word16 w_dtx_mode = 0;

#endif

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int 
main (int argc, char *argv[])
{
    Word16 *w_w_synth;              /* Synthesis                  */
    Word16 w_parm[PRM_SIZE + 1];  /* Synthesis parameters       */
    Word16 w_serial[SERIAL_SIZE+2];/* Serial stream              */
    Word16 w_Az_dec[AZ_SIZE];     /* Decoded Az for post-filter */
                                /* in 4 w_subframes, length= 44 */
    Word16 i, frame, temp;
    FILE *f_w_syn, *f_w_serial;

    Word16 TAF, SID_flag;

    Word16 w_reset_flag;
    static Word16 w_w_reset_flag_old = 1;

    w_proc_head ("Decoder");

    /*-----------------------------------------------------------------*
     *           Read passed arguments and open in/out files           *
     *-----------------------------------------------------------------*/

    if (argc != 3)
    {
        fprintf (stderr,
                 "   Usage:\n\n   decoder  bitstream_file  w_w_synth_file\n");
        fprintf (stderr, "\n");
        exit (1);
    }
    /* Open file for w_w_synthesis and packed w_serial stream */

    if ((f_w_serial = fopen (argv[1], "rb")) == NULL)
    {
        fprintf (stderr, "Input file '%s' does not exist !!\n", argv[1]);
        exit (0);
    }
    else
        fprintf (stderr, "Input bitstream file:   %s\n", argv[1]);

    if ((f_w_syn = fopen (argv[2], "wb")) == NULL)
    {
        fprintf (stderr, "Cannot open file '%s' !!\n", argv[2]);
        exit (0);
    }
    else
        fprintf (stderr, "Synthesis w_speech file:   %s\n", argv[2]);

    /*-----------------------------------------------------------------*
     *           Initialization of decoder                             *
     *-----------------------------------------------------------------*/

    w_w_synth = synth_buf + M;

    w_reset_dec (); /* Bring the decoder and receive DTX to the initial w_state */

#if (WMOPS)
    Init_WMOPS_w_counter ();
#endif

    /*-----------------------------------------------------------------*
     *            Loop for each "L_FRAME" w_speech data                  *
     *-----------------------------------------------------------------*/

    frame = 0;

    while (fread (w_serial, sizeof (Word16), 247, f_w_serial) == 247)
    {
#if (WMOPS)
        fprintf (stderr, "frame=%d  ", ++frame);
#else
        fprintf (stderr, "\nframe=%d  ", ++frame);
#endif

#if (WMOPS)
        Reset_WMOPS_w_counter (); /* reset WMOPS w_counter for the new frame */
#endif

        SID_flag = w_serial[245];         /* Receive SID flag */
        TAF = w_serial[246];              /* Receive TAF flag */

        w_Bits2w_prm_12k2 (w_serial, w_parm);   /* w_serial to parameters   */

#if (WMOPS)
                                  /* function worst case */
#endif

        if (w_parm[0] == 0)               /* BFI == 0, perform DHF check */
        {
            if (w_w_reset_flag_old == 1)    /* Check for second and further
                                           successive DHF (to first w_subfr.) */
            {
                w_reset_flag = w_decoder_homing_frame_w_test (&w_parm[1],
                                                        TO_FIRST_SUBFRAME);
            }
            else
            {
                w_reset_flag = 0;
            }
        }
        else                          /* BFI==1, bypass DHF check (frame
                                           is taken as not being a DHF) */
        {
            w_reset_flag = 0;
        }

        if ((w_reset_flag != 0) && (w_w_reset_flag_old != 0))
        {
            /* Force the output to be the encoder homing frame pattern */

            for (i = 0; i < L_FRAME; i++)
            {
                w_w_synth[i] = EHF_MASK;
            }
        }
        else
        {
            w_Decoder_12k2 (w_parm, w_w_synth, w_Az_dec, TAF, SID_flag);/* Synthesis */

            w_Post_Filter (w_w_synth, w_Az_dec);                      /* Post-filter */
#if (WMOPS)
                          /* function worst case */
#endif

            for (i = 0; i < L_FRAME; i++) 
                /* Upscale the 15 bit linear PCM to 16 bits,
                   then truncate to 13 bits */
            {
                temp = w_shl (w_w_synth[i], 1);
                w_w_synth[i] = temp & 0xfff8;           
            }

#if (WMOPS)
                                          /* function worst case */
#endif

#if (WMOPS)
            w_WMOPS_output (w_dtx_mode);/* output WMOPS values for current frame */
#endif
        }                       /* else */

        fwrite (w_w_synth, sizeof (Word16), L_FRAME, f_w_syn);

        /* BFI == 0, perform check for first DHF (whole frame) */
        if ((w_parm[0] == 0) && (w_w_reset_flag_old == 0))
        {
            w_reset_flag = w_decoder_homing_frame_w_test (&w_parm[1], WHOLE_FRAME);
        }

        if (w_reset_flag != 0)
        {
            /* Bring the decoder and receive DTX to the home w_state */
            w_reset_dec ();
        }
        w_w_reset_flag_old = w_reset_flag;

    }                           /* while */

    return 0;
}
