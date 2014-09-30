
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
    static SKP_int32 encSizeBytes;
    static void      *psEnc=0;
    static SKP_SILK_SDK_EncControlStruct encControl;
 
  // default settings 
   
    static SKP_int   skp_frames_pp = 1; //Frames per packet
    static SKP_int   packetSize_ms = 20; //frame duration
    static SKP_int   targetRate_bps = 10000; //average bitrate
    static SKP_int   compression=2; //Complexity
    static SKP_int   packetLoss_perc = 0; //redudant
    static SKP_int   INBandFec_enabled = 0; //redudant
    static SKP_int   DTX_enabled = 0; //DTX
    
           

// decoder parameters 
    static SKP_int32 decSizeBytes;
    static void      *psDec=0;
    static SKP_SILK_SDK_DecControlStruct DecControl;


//initialize SILK codec 
//sets number of 20mS(160 samples) frames per packet (1-5)
void SILK8_open (int fpp) {
	int ret;
   
       //set number of 20 mS (160 samoles) frames per packet (1-5)
    if((fpp<=MAX_INPUT_FRAMES)&&(fpp>0)) skp_frames_pp=fpp;
    packetSize_ms = skp_frames_pp * 20;

	// Set the samplingrate that is requested for the output
    DecControl.sampleRate = 8000;
		
   // Create decoder 
    ret = SKP_Silk_SDK_Get_Decoder_Size( &decSizeBytes );
    psDec = malloc( decSizeBytes );

    // Reset decoder 
    ret = SKP_Silk_SDK_InitDecoder( psDec );

    // Create Encoder 
    ret = SKP_Silk_SDK_Get_Encoder_Size( &encSizeBytes );	
    psEnc = malloc( encSizeBytes );
    
    // Reset Encoder 
    ret = SKP_Silk_SDK_InitEncoder( psEnc, &encControl );
    
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
	
	int ret,i,frsz=MAX_FRAME;
	SKP_int16 nBytes;
	unsigned int lin_pos = 0;
       
	for (i = 0; i < skp_frames_pp; i++) {
        // max payload size 
        nBytes = MAX_BYTES_ENC_PER_FRAME * skp_frames_pp;
        ret = SKP_Silk_SDK_Encode( psEnc, &encControl, in+i*frsz, (SKP_int16)frsz, (SKP_uint8 *)(enc_payload+lin_pos), &nBytes );	
		lin_pos += nBytes;
	}
	
    return lin_pos;
}


//decode packet of size bytes in buffer to short samples output_buffer 
//returns number of samples
int SILK8_decode
    (short* output_buffer, unsigned char* buffer, int size) {

	int ret;
	SKP_int16 len;
	SKP_int16	*outPtr;

    outPtr = output_buffer;

	do {
		ret = SKP_Silk_SDK_Decode( psDec, &DecControl, 0,(SKP_uint8 *) buffer, size, outPtr ,&len );		
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
