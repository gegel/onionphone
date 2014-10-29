
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Define codec specific settings 
#define MAX_BYTES_ENC_PER_FRAME     64 // Equals peak bitrate of 25600 kbps: 576 bytes per packet maximum 
#define MAX_INPUT_FRAMES        5
#define	MAX_FRAME			160
 

#include "SKP_Silk_SDK_API.h"
#include "SKP_Silk_SigProc_FIX.h"

#include "libsilk.h"

// encoder parameters
    static int32_t encSizeBytes;
    static void      *psEnc=0;
    static SKP_SILK_SDK_EncControlStruct encControl;
 
  // default settings 
   
    static int   skp_frames_pp = 1; //Frames per packet
    static int   packetSize_ms = 20; //frame duration
    static int   targetRate_bps = 10000; //average bitrate
    static int   compression=2; //Complexity
    static int   packetLoss_perc = 0; //redudant
    static int   INBandFec_enabled = 0; //redudant
    static int   DTX_enabled = 0; //DTX
    
           

// decoder parameters 
    static int32_t decSizeBytes;
    static void      *psDec=0;
    static SKP_SILK_SDK_DecControlStruct DecControl;


//initialize SILK codec 
//sets number of 20mS(160 samples) frames per packet (1-5)
void SILK8_open (int fpp) {
       //set number of 20 mS (160 samoles) frames per packet (1-5)
    if((fpp<=MAX_INPUT_FRAMES)&&(fpp>0)) skp_frames_pp=fpp;
    packetSize_ms = skp_frames_pp * 20;

	// Set the samplingrate that is requested for the output
    DecControl.sampleRate = 8000;
		
   // Create decoder 
    SKP_Silk_SDK_Get_Decoder_Size( &decSizeBytes );
    psDec = malloc( decSizeBytes );

    // Reset decoder 
    SKP_Silk_SDK_InitDecoder( psDec );

    // Create Encoder 
    SKP_Silk_SDK_Get_Encoder_Size( &encSizeBytes );
    psEnc = malloc( encSizeBytes );
    
    // Reset Encoder 
    SKP_Silk_SDK_InitEncoder( psEnc, &encControl );
    
    // Set Encoder parameters 
    encControl.sampleRate           = 8000;
    encControl.packetSize           = packetSize_ms * 8; //samples per packet
    encControl.packetLossPercentage = packetLoss_perc;
    encControl.useInBandFEC         = INBandFec_enabled;
    encControl.useDTX               = DTX_enabled;
    encControl.complexity           = compression;
    encControl.bitRate              = targetRate_bps;			
}


//encode frames_per_packet * 160 short samples to packet
//returns total packet size in bytes
int SILK8_encode
    (short *in, unsigned char *enc_payload) {	
	
	int i,frsz=MAX_FRAME;
	int16_t nBytes;
	unsigned int lin_pos = 0;
       
	for (i = 0; i < skp_frames_pp; i++) {
        // max payload size 
        nBytes = MAX_BYTES_ENC_PER_FRAME * skp_frames_pp;
        SKP_Silk_SDK_Encode( psEnc, &encControl, in+i*frsz, (int16_t)frsz, (uint8_t *)(enc_payload+lin_pos), &nBytes );
		lin_pos += nBytes;
	}
	
    return lin_pos;
}


//decode packet of size bytes in buffer to short samples output_buffer 
//returns number of samples
int SILK8_decode
    (short* output_buffer, unsigned char* buffer, int size) {

	int16_t len;
	int16_t	*outPtr;

    outPtr = output_buffer;

	do {
		SKP_Silk_SDK_Decode( psDec, &DecControl, 0,(uint8_t *) buffer, size, outPtr ,&len );
        outPtr  += len;	
	} while( DecControl.moreInternalDecoderFrames );

	return (int)(outPtr-output_buffer); //number of samples
}

//free SILK memory
void SILK8_close
    (void) {
    // Free decoder 
    free( psDec );
    // Free Encoder 
    free( psEnc );
}
