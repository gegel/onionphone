 ///////////////////////////////////////////////
//
// **************************
// ** ENGLISH - 14/03/2013 **
//
// Project/Software name: sponge.lib
// Author: "Van Gegel" <gegelcopy@ukr.net>
//
// THIS IS A FREE SOFTWARE  AND FOR TEST ONLY!!!
// Please do not use it in the case of life and death
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You’re free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Van Gegel
// * You have to provide a link to the author’s Homepage: <http://torfone.org>
//
///////////////////////////////////////////////

/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
MichaÃ«l Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by Ronny Van Keer,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include "sponge.h"

typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHLEN = 2 } HashReturn;

#define cKeccakR_SizeInBytes    (cKeccakR / 8)
#define cKeccakB_SizeInBytes    (cKeccakB / 8)

#ifndef crypto_hash_BYTES
    #ifdef cKeccakFixedOutputLengthInBytes
        #define crypto_hash_BYTES cKeccakFixedOutputLengthInBytes
    #else
        #define crypto_hash_BYTES cKeccakR_SizeInBytes
    #endif
#endif
#if (crypto_hash_BYTES > cKeccakR_SizeInBytes)
    #error "Full squeezing not yet implemented"
#endif

#define IS_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */
#define IS_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN /* WARNING: This implementation works on little-endian platform. Support for big-endinanness is implemented, but not tested. */

#if     (cKeccakB   == 1600)
    typedef QWORD  UINT64;
    typedef UINT64 tKeccakLane;
    #define cKeccakNumberOfRounds   24
#elif   (cKeccakB   == 800)
    typedef DWORD        UINT32;
    // WARNING: on 8-bit and 16-bit platforms, this should be replaced by:
    //typedef unsigned long       UINT32;
    typedef UINT32 tKeccakLane;
    #define cKeccakNumberOfRounds   22
#elif   (cKeccakB   == 400)
    typedef WORD      UINT16;
    typedef UINT16 tKeccakLane;
    #define cKeccakNumberOfRounds   20
#elif   (cKeccakB   == 200)
    typedef BYTE       UINT8;
    typedef UINT8 tKeccakLane;
    #define cKeccakNumberOfRounds   18
#else
    #error  "Unsupported Keccak-f width"
#endif

typedef unsigned int tSmallUInt; /*INFO It could be more optimized to use "unsigned char" on an 8-bit CPU	*/

#define cKeccakLaneSizeInBits   (sizeof(tKeccakLane) * 8)

#define ROL(a, offset) (tKeccakLane)((((tKeccakLane)a) << ((offset) % cKeccakLaneSizeInBits)) ^ (((tKeccakLane)a) >> (cKeccakLaneSizeInBits-((offset) % cKeccakLaneSizeInBits))))

void KeccakF( tKeccakLane * state, const tKeccakLane *in, int laneCount );


int crypto_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen )
{
    tKeccakLane		state[5 * 5];
	#if (crypto_hash_BYTES >= cKeccakR_SizeInBytes)
    #define temp out
	#else
    unsigned char 	temp[cKeccakR_SizeInBytes];
	#endif

    memset( state, 0, sizeof(state) );

    for ( /* empty */; inlen >= cKeccakR_SizeInBytes; inlen -= cKeccakR_SizeInBytes, in += cKeccakR_SizeInBytes )
    {
        KeccakF( state, (const tKeccakLane*)in, cKeccakR_SizeInBytes / sizeof(tKeccakLane) );
    }

    /*    Last data and padding	*/
    memcpy( temp, in, (size_t)inlen );
    temp[inlen++] = 1;
    memset( temp+inlen, 0, cKeccakR_SizeInBytes - (size_t)inlen );
    temp[cKeccakR_SizeInBytes-1] |= 0x80;
    KeccakF( state, (const tKeccakLane*)temp, cKeccakR_SizeInBytes / sizeof(tKeccakLane) );

    #if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN) || (cKeccakB == 200)

    memcpy( out, state, crypto_hash_BYTES );

	#else

    for ( i = 0; i < (crypto_hash_BYTES / sizeof(tKeccakLane)); ++i )
	{
		tSmallUInt		j;
	    tKeccakLane		t;

		t = state[i];
		for ( j = 0; j < sizeof(tKeccakLane); ++j )
		{
			*(out++) = (unsigned char)t;
			t >>= 8;
		}
	}

	#endif
	#if (crypto_hash_BYTES >= cKeccakR_SizeInBytes)
    #undef temp
	#endif

    return ( 0 );
}


const tKeccakLane KeccakF_RoundConstants[cKeccakNumberOfRounds] = 
{
    (tKeccakLane)0x0000000000000001ULL,
    (tKeccakLane)0x0000000000008082ULL,
    (tKeccakLane)0x800000000000808aULL,
    (tKeccakLane)0x8000000080008000ULL,
    (tKeccakLane)0x000000000000808bULL,
    (tKeccakLane)0x0000000080000001ULL,
    (tKeccakLane)0x8000000080008081ULL,
    (tKeccakLane)0x8000000000008009ULL,
    (tKeccakLane)0x000000000000008aULL,
    (tKeccakLane)0x0000000000000088ULL,
    (tKeccakLane)0x0000000080008009ULL,
    (tKeccakLane)0x000000008000000aULL,
    (tKeccakLane)0x000000008000808bULL,
    (tKeccakLane)0x800000000000008bULL,
    (tKeccakLane)0x8000000000008089ULL,
    (tKeccakLane)0x8000000000008003ULL,
    (tKeccakLane)0x8000000000008002ULL,
    (tKeccakLane)0x8000000000000080ULL
	#if		(cKeccakB	>= 400)
  , (tKeccakLane)0x000000000000800aULL,
    (tKeccakLane)0x800000008000000aULL
	#if		(cKeccakB	>= 800)
  , (tKeccakLane)0x8000000080008081ULL,
    (tKeccakLane)0x8000000000008080ULL
	#if		(cKeccakB	== 1600)
  , (tKeccakLane)0x0000000080000001ULL,
    (tKeccakLane)0x8000000080008008ULL
	#endif
	#endif
	#endif
};

const tSmallUInt KeccakF_RotationConstants[25] =
{
	 1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14, 27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
};

const tSmallUInt KeccakF_PiLane[25] = 
{
    10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4, 15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1 
};

const tSmallUInt KeccakF_Mod5[10] = 
{
    0, 1, 2, 3, 4, 0, 1, 2, 3, 4
};


void KeccakF( tKeccakLane * state, const tKeccakLane *in, int laneCount )
{
	tSmallUInt x, y;
    tKeccakLane temp;
    tKeccakLane BC[5];

	#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN) || (cKeccakB == 200)
    while ( --laneCount >= 0 )
	{
        state[laneCount] ^= in[laneCount];
	}
	#else
	temp = 0; /* please compiler */
    while ( --laneCount >= 0 )
	{
		for ( x = 0; x < sizeof(tKeccakLane); ++x )
		{
			temp <<= 8;
			temp |= ((char*)&in[laneCount])[x];
		}
        state[laneCount] = temp;
	}
	#endif

	#define	round	laneCount
    for( round = 0; round < cKeccakNumberOfRounds; ++round )
    {
		// Theta
		for ( x = 0; x < 5; ++x )
		{
			BC[x] = state[x] ^ state[5 + x] ^ state[10 + x] ^ state[15 + x] ^ state[20 + x];
		}
		for ( x = 0; x < 5; ++x )
		{
			temp = BC[KeccakF_Mod5[x+4]] ^ ROL(BC[KeccakF_Mod5[x+1]], 1);
			for ( y = 0; y < 25; y += 5 )
			{
				state[y + x] ^= temp;
			}
		}

        // Rho Pi
		temp = state[1];
		for ( x = 0; x < 24; ++x )
		{
			BC[0] = state[KeccakF_PiLane[x]];
			state[KeccakF_PiLane[x]] = ROL( temp, KeccakF_RotationConstants[x] );
			temp = BC[0];
		}

		//	Chi
		for ( y = 0; y < 25; y += 5 )
		{
			BC[0] = state[y + 0];
			BC[1] = state[y + 1];
			BC[2] = state[y + 2];
			BC[3] = state[y + 3];
			BC[4] = state[y + 4];
			for ( x = 0; x < 5; ++x )
			{
				state[y + x] = BC[x] ^((~BC[KeccakF_Mod5[x+1]]) & BC[KeccakF_Mod5[x+2]]);
			}
		}

		//	Iota
		state[0] ^= KeccakF_RoundConstants[round];
    }
	#undef	round

}

 ///////////Duplex sponge implementatiom Van Gegel, 14.03.2013//////////

//Sponge initialization:
//key for shedule Key || Nonce
//header - for force SpongeWrap mode
//Limits for key length and header lenght are R-8 bits
//(will be truncated under limit!!!)
void Sponge_init(KECCAK512_DATA *keccak, const BYTE *key, int klen, const BYTE *header, int hlen)
{
 int i;
 keccak->bytesInQueue = 0;
 memset( keccak->state, 0, sizeof(keccak->state) );

 //Sheduling key||nonce (R-8 bits maximum)
 if((key!=NULL)&&(klen!=0)) { //key defined
  if(klen>(cKeccakR_SizeInBytes-1)) klen=cKeccakR_SizeInBytes-1;
  memcpy(keccak->state, key, klen); //Absorbs keymaterial
  //Padding keymaterial
  if(!header) keccak->state[klen++]=1;   //No frame bit, only first padbit=1
  else if(hlen) keccak->state[klen++]=2; //frame bit=0 for SpongeWrap (key+header) and first padbit=1
  else keccak->state[klen++]=3; //frame bit=1 for SpongeWrap(key only) and first padbit=1
  memset(keccak->state + klen, 0, cKeccakR_SizeInBytes - klen); //padding body (zeroes)
  keccak->state[cKeccakR_SizeInBytes - 1] |= 0x80; //last padbit=1
  KeccakF( (tKeccakLane *)keccak->state, 0, 0 );  //permutes padded block
 }

 //Using header (R-8 bits maximum): force SpongeWrap mode
 if((header==NULL)||(hlen==0)) return; //no prefix defined
 if(hlen>(cKeccakR_SizeInBytes-1)) hlen=cKeccakR_SizeInBytes-1; //Limits for header lenth!!!
 for(i=0;i<hlen;i++) keccak->state[i]^=header[i]; //absorbs header
 //Padding header
 keccak->state[hlen++]^=3; //SpongeWrap frame bit=1 for header and first padbit=1
 keccak->state[cKeccakR_SizeInBytes - 1] ^= 0x80; //last padbit=1  (body is zeroes, not nedds for xor)
 KeccakF( (tKeccakLane *)keccak->state, 0, 0 ); //permutes padded block
 keccak->state[keccak->bytesInQueue]^=0x03;  //pre-inverse bits for correct Wrap padding
}


//Sponge request in different modes
//Parameters:
//buffer - input data or NULL for blank request,
//output - output data or NULL for mute request
//len - bytes for processing, 0 force full block (up to F-permutation)
//mode switch bits:
//bit 0 - Duplex mode (each block padded)
//bit 1 - Decrypt mode (absorbing output insteed input)
//bit 2 - Wrap mode (adds frame bit before padding)
//bit 3 - frame bit for Wrap mode
//bit 4 - force F-permutation
//bit 5 - Forget mode (zeroes B-R first bits of states)
int Sponge_data(KECCAK512_DATA *keccak, const BYTE *buffer, int len, BYTE *output, char mode)
{
        const BYTE * src; //source for absorbing
        int rate; //block size
        unsigned char c = 0; //temporary output
        
        //check for Final
        if (keccak->bytesInQueue < 0) return keccak->bytesInQueue; // Final() already called


        //Set block size
        if(mode&0x20) //Forgeting mode
        {
         rate=cKeccakB_SizeInBytes-cKeccakR_SizeInBytes; //C=B-R bytes of state cleares
         keccak->bytesInQueue=0; //clears from first byte
         len=rate; //do up to full block and F-permutation
         buffer=0; //force blank
         output=0; //force mute
        }
        else if(mode&1)
        {
         rate=cKeccakR_SizeInBytes-1;//duplex mode (all blocks padded with overhead 8 bits)
        }
        else
        {
         rate=cKeccakR_SizeInBytes;  //normal mode: only last block padded in Final procedure
        }

        //Set number of bytes for processing
        if(mode&0x10)
        {
         len=rate-keccak->bytesInQueue; //force F-permutation
        }
        else if((!len)&&(keccak->bytesInQueue))
        {
         len=rate-keccak->bytesInQueue; //process all absorbed data
        }

        //Wrap correcrion
        if(mode&4)
        {
         keccak->state[keccak->bytesInQueue]^=0x03;  //de-inverse bits for correct Wrap padding
        }

	//Process data: if already data in queue, continue queuing first
	for ( /* empty */; len>0; len-- )   //byte-by-byte procesing up to len
	{
                if(output)
                { //Outputs current state (squeezing)
                 c = keccak->state[keccak->bytesInQueue];
                 //Duplexing: xores sponges output and  input data
                 if(buffer) c ^= (*(buffer));
                }

                //Select source for absorbing
        if(mode&2) src=&c; //Decryption: sponge absorbing output (plaintext)
        else if(mode&0x20) src=keccak->state; //Forgeting mode: state^state=0
        else if(mode&0x40) src=0; //Force no absorbing
        else src=buffer; //Encryption: sponge absorbing input (also planetext)

                //Absorbing
		if(src) keccak->state[keccak->bytesInQueue] ^= *(src);

                //Next byte for processing
                if(output) {
                 *(output) = c;
                 output++;
                }
                if(buffer) buffer++;
                keccak->bytesInQueue++;

                //Process full block
		if ( keccak->bytesInQueue == rate )
		{
                        if(mode&8)
                        {
                         keccak->state[rate]^=0x83; //Duplex Wrap mode: Pads every block with Wrap mode extra frame bit=1 before 7 pad's bits
                        }
                        else if(mode&4)
                        {
                         keccak->state[rate]^=0x82; //Duplex Wrap mode: Pads + frame bit=0
                        }
                        else if(mode&1)
                        {
                         keccak->state[rate]^=0x81; //Duplex mode: Pads every block
                        }
			KeccakF( (tKeccakLane *)keccak->state, 0, 0 ); //Permute full block
			keccak->bytesInQueue = 0; //All accepted bytes were processed 
		}
	}
        //Wrap correction:
        //last block of data body must be padded in Wrap mode as 01000000 (frame bit=0 the first)
        //(03 there)^(01 if Final())=02
        if(mode&4)
        {
         keccak->state[keccak->bytesInQueue]^=0x03; //inverse bits for correct Wrap padding
        }
         //returns number of extra bytes needed for compleet block
        if(!keccak->bytesInQueue) return 0;
        else return(rate - keccak->bytesInQueue);

}

//Final squeezing for tag generation and securety destroying of sponges state
void Sponge_finalize(KECCAK512_DATA *keccak, BYTE *tag, int taglen)
{
 if ( keccak->bytesInQueue < 0 ) return; //Final() already called.
 if(!tag) taglen=0;  //no needs tag
 if(taglen>cKeccakR_SizeInBytes) taglen=cKeccakR_SizeInBytes; //Limits for tag length is R bits
 //Computes tag
 if(taglen)
 {
   //Padding last block
   keccak->state[keccak->bytesInQueue++]^=1;
   keccak->state[cKeccakR_SizeInBytes - 1] ^= 0x80;
   //squeezing tag
   KeccakF( (tKeccakLane *)keccak->state, 0, 0 );
   memcpy(tag, keccak->state, taglen);
 }
 //destroys Sponge
 memset( keccak->state, 0, sizeof(keccak->state) );
 keccak->bytesInQueue = -1;	/* flag final state */
}


/////////////////////EXAMPLES///////////////////////////

//hash-512
void sponge_hash_512(BYTE *hash, const unsigned char *in, int inlen )
{
 KECCAK512_DATA spng;
 Sponge_init(&spng, 0, 0, 0, 0); //initialize only
 Sponge_data(&spng, in, inlen, 0, SP_NORMAL);  //absorbing data
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, hash, 64); //optionaly permute then squeezing hash
}

//HMAC with 128-bits mac
void sponge_hmac_128(BYTE *tag, const unsigned char *key, int keylen, const unsigned char *in, int inlen )
{
 KECCAK512_DATA spng;
 Sponge_init(&spng, key, keylen, 0, 0);  //absorbing mackey  then permute
 Sponge_data(&spng, in, inlen, 0, SP_NORMAL); //absorbing data
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, tag, 16);    //optionaly permute then squeezing tag
}

// KDF / PKDF: uses salt || info fpr derives key from password/material (for kdf iteration=0)
void sponge_kdf(BYTE *key, int keylen, const unsigned char *salt, int saltlen, const unsigned char *pass, int passlen, int iteration )
{
 KECCAK512_DATA spng;
 int i;
 Sponge_init(&spng, pass, passlen, 0, 0);  //init key||info and permute
 Sponge_data(&spng, salt, saltlen, 0, SP_NORMAL); //absorbing salt
 for(i=0;i<iteration; i++)
 {
  Sponge_data(&spng, 0, 0, 0, SP_FORCE); //force extra "blank-mute" request
  Sponge_data(&spng, 0, 0, 0, SP_FORGET); //force extra "forgeting" request
  Sponge_data(&spng, 0, 0, 0, SP_FORCE); //force extra "blank-mute" request
 }
 Sponge_data(&spng, 0, keylen, key, SP_NORMAL); //squeezing key
 Sponge_finalize(&spng, 0, 0); //optionaly permute then squeezing key
}

//Streem encryption like CTR mode
void sponge_ctr(BYTE *out, const unsigned char *key, int keylen, const unsigned char *iv, int ivlen, const unsigned char *in, int inlen )
{
 KECCAK512_DATA spng;
 Sponge_init(&spng, key, keylen, 0, 0); //init key and permute
 Sponge_data(&spng, iv, ivlen, 0, SP_NORMAL); //absorbing iv and permute
 Sponge_data(&spng, 0, 0, 0, SP_NORMAL); //process full block

 Sponge_data(&spng, in, inlen, out, SP_NOABS);  //squeezing gamma, out=in^gamma
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, 0, 0); //destoy sponge
}

//One-pass autenticated encryption
void sponge_enc(unsigned char *out, unsigned char *tag, int taglen, const unsigned char *key, int keylen, const unsigned char *header, int hlen, const unsigned char *in, int inlen )
{
 KECCAK512_DATA spng;
 Sponge_init(&spng, key, keylen, header, hlen); //init key||nonce,  absorbing header, force Wrap mode
 Sponge_data(&spng, in, inlen, out, SP_WRAP1|SP_ENCRYPT); //Duplex+Wrap, absorbing input
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, tag, taglen); //optionaly permute then squeezing autentication tag
}

//one-pass autenticated decryption
void sponge_dec(unsigned char *out, unsigned char *tag, int taglen, const unsigned char *key, int keylen, const unsigned char *header, int hlen, const unsigned char *in, int inlen )
{
 KECCAK512_DATA spng;
 Sponge_init(&spng, key, keylen, header, hlen); //init key||nonce,  absorbing header, force Wrap mode
 Sponge_data(&spng, in, inlen, out, SP_WRAP1|SP_DECRYPT); //Duplex+Wrap, absorbing output
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, tag, taglen);   //optionaly permute then squeezing autentication tag
}

