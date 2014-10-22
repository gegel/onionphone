//initialize SILK codec for fpp frames per packet (1-5)
void SILK8_open (int fpp);

//encode fpp * 160 short samples frames to packet
//returns packet size in bytes
int SILK8_encode
    (short *in, unsigned char *enc_payload);
    
//decode packet of size bytes in buffer to short samples output_buffer 
//returns number of samples
int SILK8_decode
    (short* output_buffer, unsigned char* buffer, int size);
    

//free SILK memory
void SILK8_close (void);