/***************************************************************************
 *
 *     File Name: ed_iface.c
 *
 *     Purpose: Speech encoder/decoder interface device
 *
 *     This program transforms the output file format of the GSM Enhanced
 *     Full Rate Encoder module consisting of:
 *         * 244 w_speech parameter bits (see GSM TS 06.60)
 *         *   1 voice activity flag VAD (see GSM TS 06.82)
 *         *   1 w_speech flag SP (see GSM TS 06.81)
 *
 *     to the input file format of the GSM Enhanced Full Rate Decoder module
 *     requiring:
 *         *   1 channel condition flag BFI (see GSM TS 06.61, 05.05)
 *         * 244 w_speech parameter bits (see GSM TS 06.60)
 *         *   1 SID flag (2 bits) (see GSM TS 06.81, 05.05)
 *         *   1 time alignment flag TAF (see GSM TS 06.81)
 *
 *     Between SID updates the w_speech parameters are replaced by random
 *     values simulating an interrupted transmission on the air interface
 *
 *     Below is a listing of all the functions appearing in the file,
 *     with a short description of their purpose.
 *
 *     Convert single frame from encoder output format to decoder
 *     input format:
 *       enc_dec_interface()
 *     Receive single encoder output parameter frame:
 *       encoder_interface()
 *     Send single decoder input parameter frame:
 *       decoder_interface()
 *     Open file for binary read or write:
 *       open_bin_file()
 *     Set the w_speech parameters to random values:
 *       random_parameters()
 *
 **************************************************************************/

/***************************************************************************
 *
 *      Include-Files
 *
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "typedef.h"
#include "cnst.h"
#include "dtx.h"

#define OPEN_WB  "wb"
#define OPEN_RB  "rb"

/***************************************************************************
 *
 *      Local function prototypes
 *
 **************************************************************************/

static Word16 enc_dec_interface (FILE *infile, FILE *outfile);
static Word16 encoder_interface (FILE *infile, Word16 w_serial_in_para[]);
static Word16 decoder_interface (Word16 w_serial_out_para[], FILE *outfile);
FILE *open_bin_file (char *name, char *mode);
static void random_parameters (Word16 w_serial_params[]);

/***************************************************************************
 *
 *      Local functions
 *
 **************************************************************************/

static Word16 enc_dec_interface (FILE *infile, FILE *outfile)
{

#define SPEECH      1
#define CNIFIRSTSID 2
#define CNICONT     3
#define VALIDSID    11
#define GOODSPEECH  33

    static Word16 decoding_mode = {SPEECH};
    static Word16 w_TAF_count = {1};
    Word16 w_serial_in_para[246], i, frame_type;
    Word16 w_serial_out_para[247];
 
    if (encoder_interface (infile, w_serial_in_para) != 0)
    {
        return (1);
    }

    /* w_Copy input parameters to output parameters */
    /* ------------------------------------------ */
    for (i = 0; i < 244; i++)
    {
        w_serial_out_para[i+1] = w_serial_in_para[i];
    }

    /* Set channel status (BFI) flag to w_zero */
    /* --------------------------------------*/
    w_serial_out_para[0] = 0;     /* BFI flag */

    /* Evaluate SID flag                                  */
    /* Function w_sid_frame_detection() is defined in dtx.c */
    /* -------------------------------------------------- */
    w_serial_out_para[245] = w_sid_frame_detection (&w_serial_out_para[1]);

    /* Evaluate TAF flag */
    /* ----------------- */
    if (w_TAF_count == 0)
    {
        w_serial_out_para[246] = 1;
    }
    else
    {
        w_serial_out_para[246] = 0;
    }

    w_TAF_count = (w_TAF_count + 1) % 24;
   
    /* Frame classification:                                                */
    /* Since the transmission is w_error free, the received frames are either */
    /* valid w_speech or valid SID frames                                     */
    /* -------------------------------------------------------------------- */
    if (w_serial_out_para[245] == 2)
    {
        frame_type = VALIDSID;
    }
    else if (w_serial_out_para[245] == 0)
    {
        frame_type = GOODSPEECH;
    }
    else {
        fprintf (stderr, "Error in SID detection\n");
        return (1);
    }

    /* Update of decoder w_state */
    /* ----------------------- */
    if (decoding_mode == SPEECH) /* State of previous frame */
    {
        if (frame_type == VALIDSID)
        {
            decoding_mode = CNIFIRSTSID;
        }
        else if (frame_type == GOODSPEECH)
        {
            decoding_mode = SPEECH;
        }
    }
    else  /* comfort noise insertion mode */
    {
        if (frame_type == VALIDSID)
        {
            decoding_mode = CNICONT;
        }
        else if (frame_type == GOODSPEECH)
        {
            decoding_mode = SPEECH;
        }
    }

    /* Replace parameters by random data if in CNICONT-mode and TAF=0 */
    /* -------------------------------------------------------------- */
    if ((decoding_mode == CNICONT) && (w_serial_out_para[246] == 0))
    {
        random_parameters (&w_serial_out_para[1]);

        /* Set flags such that an "unusable frame" is produced */
        w_serial_out_para[0] = 1;       /* BFI flag */
        w_serial_out_para[245] = 0;     /* SID flag */
    }    

    if (decoder_interface (w_serial_out_para, outfile) != 0)
    {
        fprintf (stderr, "Error writing File\n");
        return (1);
    }

    return (0);
}

static Word16 encoder_interface (FILE *infile, Word16 w_serial_in_para[])
{
    size_t lgth_read = 0;    
    Word16 ret;

    lgth_read = fread (w_serial_in_para, sizeof (Word16), 246, infile);

    if (lgth_read == 0)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    return (ret);
}

static Word16 decoder_interface (Word16 w_serial_out_file[], FILE *outfile)
{
    size_t lgth_written;
    Word16 ret;

    lgth_written = fwrite (w_serial_out_file, sizeof (Word16), 247, outfile);

    if (lgth_written == 247)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return (ret);
}

FILE *open_bin_file (char *name, char *mode)
{
    FILE *fp;

    if (toupper (*mode) == 'W') /* Write access */
    {
        if ((fp = fopen (name, OPEN_WB)) == NULL)
        { 
            fprintf (stderr, "Can't open output file '%s'\n", name);
            exit (1);
        }
    }
    else /* Read access */
    {
        if ((fp = fopen (name, OPEN_RB)) == NULL)
        { 
            fprintf (stderr, "Can't open file '%s'\n", name);
            exit (1);
        }
    }

    return (fp);
}

static void random_parameters (Word16 w_serial_params[])
{
    static Word32 L_PN_seed = 0x321CEDE2L;
    Word16 i;

    /* Set the 244 w_speech parameter bits to random bit values */
    /* Function w_pseudonoise() is defined in dtx.c             */
    /*--------------------------------------------------------*/

    for (i = 0; i < 244; i++)
    {
        w_serial_params[i] = w_pseudonoise (&L_PN_seed, 1);
    }

    return;
}

/****************************************************************************
 *
 *          Main program of the encoder/decoder interface device
 *
 ***************************************************************************/

int main (int argc, char *argv[])
{
    FILE *infile, *outfile;
    Word16 i;

    if (argc != 3)
    {
        fprintf (stderr, "\n   Usage:\n\n   ed_iface  input  output\n\n");
        return (1);
    }

    fprintf (stderr, "  ____________________________________________________\n");
    fprintf (stderr, " |                                                    |\n");
    fprintf (stderr, " |       Speech Encoder-Decoder Interface Device      |\n");
    fprintf (stderr, " |                                                    |\n");
    fprintf (stderr, " |                         for                        |\n");
    fprintf (stderr, " |                                                    |\n");
    fprintf (stderr, " |   GSM Enhanced Full Rate Speech Codec Simulation   |\n");
    fprintf (stderr, " |____________________________________________________|\n\n");

    fprintf (stderr, "    Input File       : %s\n", argv[1]);
    fprintf (stderr, "    Output File      : %s\n\n", argv[2]);

    infile  = open_bin_file (argv[1], "r");
    outfile = open_bin_file (argv[2], "w");

    i = 0;
    while (enc_dec_interface (infile, outfile) == 0)
    {
        if ((i % 50) == 0)
        {
            fprintf (stderr, "\r    %d", i);
        }
        i++;
    }
    fprintf (stderr, "\r    %d", i);

    fclose (infile);
    fclose (outfile);

    fprintf (stderr, " Frame%s processed      \n\n", ( i != 1 ) ? "s" : "");

    return (0);
}
