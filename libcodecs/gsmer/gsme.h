/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* GSM_EFR (GSM 06.53) codec wrapper */
/***************************************************************************
 *    Format for w_speech:
 *      Speech is 16 bits data.
 *
 *    Format for bitstream:
 *      244  words (2-byte) containing 244 bits.
 *          Bit 0 = 0x0000 and Bit 1 = 0x0001
 *      One word (2-byte) for voice activity decision (VAD) flag bit
 *          0x0000 -> inactive (no detected w_speech activity);
 *          0x0001 -> active
 *      One word (2-byte) for w_speech (SP) flag bit
 *          0x0000 -> inactive (no transmission of w_speech frames);
 *          0x0001 -> active
 *		One word (2-byte) for no w_error/transmitted (Not BFI) flag bit
 *          0x0000 -> dummy (not transmitted) frame;
 *          0x0001 -> received farme
 *      One word (2-byte) for TAF flag
 *          0x0000 -> regullag frame;
 *          0x0001 -> time aligned frame
 *
 ***************************************************************************/

//Initialise encoder & decoder
void gsmer_init(int dtx);

//encode 160 shorts samples to 31 uchars stream
//returns SP + 2*VAD flags
int gsmer_encode(unsigned char *rb, const short *pcm);

//decode 31 uchars stream to 160 shorts samples 
//returns SP + 2*VAD flags
int gsmer_decode(short *pcm, const unsigned char *rb);
