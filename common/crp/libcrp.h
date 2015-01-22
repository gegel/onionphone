#pragma once

#ifndef _LIBCRP_H_
#define _LIBCRP_H_

#include "sponge.h"
#include "sprng.h"
#include "curve.h"
#include "b64.h"
#include "wordlist.h"
#include "havege.h"

//int curve25519_donna(unsigned char *mypublic, const unsigned char *secret, const unsigned char *basepoint);
//int get_pubkey(unsigned char *mypublic, const unsigned char *secret);
//char* getword(short b);
//int randInit(uchar const *seed, int len);
//void randFeed(uchar const *seed, int len);
//void randFetch(uchar *randout, int len);
//void randForget(void);
//int randDestroy(void);
//void Sponge_init(KECCAK512_DATA *keccak, const BYTE *key, int klen, const BYTE *header, int hlen);
//int Sponge_data(KECCAK512_DATA *keccak, const BYTE *buffer, int len, BYTE *output, char mode);
//void Sponge_finalize(KECCAK512_DATA *keccak, BYTE *tag, int taglen);
//int b64encode( FILE *infile, FILE *outfile, int linesize );
//int b64estr(const unsigned char* data, int bytes, char* str);
//int b64decode( FILE *infile, FILE *outfile );
//int b64dstr( const char *str, unsigned char* data);
//int b64( char opt, char *infilename, char *outfilename, int linesize );

#endif /* _LIBCRP_H_ */

