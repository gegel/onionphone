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

#ifndef SPONGE_H
#define SPONGE_H
// ** Thread-safe implementation
#include "Keccak512_data.h"

//Sponge modes:
#define SP_NORMAL 0
#define SP_DUPLEX 1
#define SP_ENCRYPT 1
#define SP_DECRYPT 3
#define SP_WRAP0 5
#define SP_WRAP1 0xD
#define SP_FORCE 0x10
#define SP_FORGET 0x31
#define SP_NOABS 0x40

 //Original Keccak hashing (512bit hash)
//extern int crypto_hash(BYTE *out, const BYTE *in, QWORD inlen );

//Duplex Sponge
extern void Sponge_init(KECCAK512_DATA *keccak, const BYTE *key, int klen, const BYTE *header, int hlen);
extern int Sponge_data(KECCAK512_DATA *keccak, const BYTE *buffer, int len, BYTE *output, char mode);
extern void Sponge_finalize(KECCAK512_DATA *keccak, BYTE *tag, int taglen);

//EXAMPLES
/*
extern void sponge_hash_512(BYTE *hash, const BYTE *in, int inlen );
extern void sponge_hmac_128(BYTE *tag, const BYTE *key, int keylen, const BYTE *in, int inlen );
extern void sponge_kdf(BYTE *key, int keylen, const unsigned char *salt, int saltlen, const unsigned char *pass, int passlen, int iteration );
extern void sponge_ctr(BYTE *out, const unsigned char *key, int keylen, const unsigned char *iv, int ivlen, const unsigned char *in, int inlen );
extern void sponge_enc(unsigned char *out, unsigned char *tag, int taglen, const unsigned char *key, int keylen, const unsigned char *header, int hlen, const unsigned char *in, int inlen );
extern void sponge_dec(unsigned char *out, unsigned char *tag, int taglen, const unsigned char *key, int keylen, const unsigned char *header, int hlen, const unsigned char *in, int inlen );
*/
#endif
