// Contact: <torfone@ukr.net>
// Author: Van Gegel
//
// THIS IS A FREE SOFTWARE
//
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You're free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Van Gegel
// * You have to provide a link to the author's Homepage: <http://torfone.org/>
//
///////////////////////////////////////////////

#include <stdlib.h>
#include "libcrp.h"
#include <string.h>

void thex2asc(unsigned  char* input, int bytes, char* output)
{
 int i;
 if(output)
 {
  output[0]=0;
  for (i=0;i<bytes;i++) sprintf(output+strlen(output), "%02X", (unsigned int)input[i]);
 }
}

void test_memeq_hex(char* step, unsigned char* digest, const char* hex) {
 unsigned char buf[64]={0};
 unsigned int i;
 char s[4]={0};
 for(i=0; i<(strlen(hex)>>1); i++) {
  s[0]=hex[i<<1];
  s[1]=hex[1+(i<<1)];
  buf[i]=(unsigned char)strtol(s, NULL,16);
 }
 printf("%s", step);
 if(!memcmp(buf, digest, i)) printf(" OK\r\n"); else printf(" FAILED\r\n");
}

/** Run unit tests for our ed25519 library functionality */
//Source from: https://github.com/nightcracker/ed25519
//Vectors from: http://cr.yp.to/highspeed/naclcrypto-20090310.pdf
void ec25519_test(void) {
        //unsigned char alicepk_edwards[32];
        unsigned char alicepk_montgomery[32];
        //unsigned char bobpk_edwards[32];
        unsigned char bobpk_montgomery[32];
        unsigned char alice_secret[32];
        unsigned char bob_secret[32];
   
        
        unsigned char alicesk[32] = {
        0x77,0x07,0x6d,0x0a,0x73,0x18,0xa5,0x7d
        ,0x3c,0x16,0xc1,0x72,0x51,0xb2,0x66,0x45
        ,0xdf,0x4c,0x2f,0x87,0xeb,0xc0,0x99,0x2a
        ,0xb1,0x77,0xfb,0xa5,0x1d,0xb9,0x2c,0x2a
        } ;
        unsigned char exp_alicepk_montgomery[32] = {
        0x85,0x20,0xf0,0x09,0x89,0x30,0xa7,0x54
        ,0x74,0x8b,0x7d,0xdc,0xb4,0x3e,0xf7,0x5a
        ,0x0d,0xbf,0x3a,0x0d,0x26,0x38,0x1a,0xf4
        ,0xeb,0xa4,0xa9,0x8e,0xaa,0x9b,0x4e,0x6a
        };
        unsigned char bobsk[32] = {
        0x5d,0xab,0x08,0x7e,0x62,0x4a,0x8a,0x4b
        ,0x79,0xe1,0x7f,0x8b,0x83,0x80,0x0e,0xe6
        ,0x6f,0x3b,0xb1,0x29,0x26,0x18,0xb6,0xfd
        ,0x1c,0x2f,0x8b,0x27,0xff,0x88,0xe0,0xeb
        } ;
        unsigned char exp_bobpk_montgomery[32] = {
        0xde,0x9e,0xdb,0x7d,0x7b,0x7d,0xc1,0xb4
        ,0xd3,0x5b,0x61,0xc2,0xec,0xe4,0x35,0x37
        ,0x3f,0x83,0x43,0xc8,0x5b,0x78,0x67,0x4d
        ,0xad,0xfc,0x7e,0x14,0x6f,0x88,0x2b,0x4f
        } ;
        unsigned char exp_secret[32] = {
        0x4a,0x5d,0x9d,0x5b,0xa4,0xce,0x2d,0xe1
        ,0x72,0x8e,0x3b,0xf4,0x80,0x35,0x0f,0x25
        ,0xe0,0x7e,0x21,0xc9,0x47,0xd1,0x9e,0x33
        ,0x76,0xf0,0x9b,0x3c,0x1e,0x16,0x17,0x42
        } ;

        printf("\r\nRun EC25519 integrity test:\r\n");

        //ed25519_compute_new_public(alicepk_edwards, alicesk);
        //edwardspk2mongomerypk(alicepk_montgomery, alicepk_edwards);
        get_pubkey(alicepk_montgomery, alicesk);
        if(!memcmp(alicepk_montgomery, exp_alicepk_montgomery, 32))
        printf("Public A - OK\r\n"); else printf("Public A - FAILURE\r\n");

        //ed25519_compute_new_public(bobpk_edwards, bobsk);
        //edwardspk2mongomerypk(bobpk_montgomery, bobpk_edwards);
        get_pubkey(bobpk_montgomery, bobsk);
        if(!memcmp(bobpk_montgomery, exp_bobpk_montgomery, 32))
        printf("Public B - OK\r\n"); else printf("Public B - FAILURE\r\n");

        //ed25519_key_exchange(alice_secret, bobpk_edwards, alicesk);
        curve25519_donna(alice_secret, alicesk, bobpk_montgomery);
        if(!memcmp(alice_secret, exp_secret, 32))
        printf("Secret A - OK\r\n"); else printf("Secret A - FAILURE\r\n");

        //ed25519_key_exchange(bob_secret, alicepk_edwards, bobsk);
        curve25519_donna(bob_secret, bobsk, alicepk_montgomery);
        if(!memcmp(bob_secret, exp_secret, 32))
        printf("Secret B - OK\r\n"); else printf("Secret B - FAILURE\r\n");
}


//https://gitweb.torproject.org/tor.git/blob_plain/HEAD:/src/test/test_crypto.c
void test_crypto_curve25519_donna(void)
{
  // adapted from curve25519_donna, which adapted it from test-curve25519
  // version 20050915, by D. J. Bernstein, Public domain.
  //curve25519_donna(mypublic, mysecret, basepoint);
  //http://tools.ietf.org/search/draft-josefsson-tls-curve25519-03

#ifdef SLOW_CURVE25519_TEST
  const int loop_max=10000;
  const char e1_expected[]    = "4faf81190869fd742a33691b0e0824d5"
                                "7e0329f4dd2819f5f32d130f1296b500";
  const char e2k_expected[]   = "05aec13f92286f3a781ccae98995a3b9"
                                "e0544770bc7de853b38f9100489e3e79";
  const char e1e2k_expected[] = "cd6e8269104eb5aaee886bd2071fba88"
                                "bd13861475516bc2cd2b6e005e805064";
#else
  const int loop_max=200;
  const char e1_expected[]    = "bc7112cde03f97ef7008cad1bdc56be3"
                                "c6a1037d74cceb3712e9206871dcf654";
  const char e2k_expected[]   = "dd8fa254fb60bdb5142fe05b1f5de44d"
                                "8e3ee1a63c7d14274ea5d4c67f065467";
  const char e1e2k_expected[] = "7ddb98bd89025d2347776b33901b3e7e"
                                "c0ee98cb2257a4545c0cfb2ca3e1812b";
#endif

  unsigned char e1k[32];
  unsigned char e2k[32];
  unsigned char e1e2k[32];
  unsigned char e2e1k[32];
  unsigned char e1[32] = {3};
  unsigned char e2[32] = {5};
  unsigned char k[32] = {9};
  int loop, i;

  printf("\r\nRun EC25519 tests:\r\n");

  for (loop = 0; loop < loop_max; ++loop) {

    printf("\r      \r%d%%", loop*100/loop_max);
    fflush(stdout);

    curve25519_donna(e1k,e1,k);
    curve25519_donna(e2e1k,e2,e1k);
    curve25519_donna(e2k,e2,k);
    curve25519_donna(e1e2k,e1,e2k);


    if(memcmp(e1e2k, e2e1k, 32)) printf("Common secret missmatch!\r\n");

    if (loop == loop_max-1) {
      break;
    }
    for (i = 0;i < 32;++i) e1[i] ^= e2k[i];
    for (i = 0;i < 32;++i) e2[i] ^= e1k[i];
    for (i = 0;i < 32;++i) k[i] ^= e1e2k[i];
  }
  printf("\r%d DH passed\r\n", loop_max);
  test_memeq_hex("EC e1   ", e1, e1_expected);
  test_memeq_hex("EC e2k  ", e2k, e2k_expected);
  test_memeq_hex("EC e1e2k", e1e2k, e1e2k_expected);
}


//Keccak page: http://keccak.noekeon.org/files.html
//Reference document: http://keccak.noekeon.org/Keccak-reference-3.0.pdf
//Sponge WrapMode and SpongePRG:
//http://eprint.iacr.org/2011/499.pdf

//Core Code source:
//http://keccak.noekeon.org/Keccak-reference-3.0-files.zip
//Keccak-compact.c
//mode B/R = 1600/576

void test_keccak(void) {  //Core integrity test
  
//Test vectors:
//http://keccak.noekeon.org/KeccakKAT-3.zip
//ShortMsgKAT_512.txt
//
/*
Line 37:
Len = 8
Msg = CC
MD = 8630C13CBD066EA74BBE7FE468FEC1DEE10EDC1254FB4C1B7C5FD69B646E44160B8CE01D05A0908CA790DFB080F4B513BC3B6225ECE7A810371441A5AC666EB9

Line 69:
Len = 16
Msg = 41FB
MD = 551DA6236F8B96FCE9F97F1190E901324F0B45E06DBBB5CDB8355D6ED1DC34B3F0EAE7DCB68622FF232FA3CECE0D4616CDEB3931F93803662A28DF1CD535B731
*/

 const unsigned char data1[1]={0xCC};
 const unsigned char vect1[64]={0x86, 0x30, 0xC1, 0x3C,
 0xBD, 0x06, 0x6E, 0xA7, 0x4B, 0xBE, 0x7F, 0xE4, 0x68,
 0xFE, 0xC1, 0xDE, 0xE1, 0x0E, 0xDC, 0x12, 0x54, 0xFB,
 0x4C, 0x1B, 0x7C, 0x5F, 0xD6, 0x9B, 0x64, 0x6E, 0x44,
 0x16, 0x0B, 0x8C, 0xE0, 0x1D, 0x05, 0xA0, 0x90, 0x8C,
 0xA7, 0x90, 0xDF, 0xB0, 0x80, 0xF4, 0xB5, 0x13, 0xBC,
 0x3B, 0x62, 0x25, 0xEC, 0xE7, 0xA8, 0x10, 0x37, 0x14,
 0x41, 0xA5, 0xAC, 0x66, 0x6E, 0xB9};

 const unsigned char data2[2]={0x41, 0xFB};
 const unsigned char vect2[64]={0x55, 0x1D, 0xA6, 0x23,
 0x6F, 0x8B, 0x96, 0xFC, 0xE9, 0xF9, 0x7F, 0x11, 0x90,
 0xE9, 0x01, 0x32, 0x4F, 0x0B, 0x45, 0xE0, 0x6D, 0xBB,
 0xB5, 0xCD, 0xB8, 0x35, 0x5D, 0x6E, 0xD1, 0xDC, 0x34,
 0xB3, 0xF0, 0xEA, 0xE7, 0xDC, 0xB6, 0x86, 0x22, 0xFF,
 0x23, 0x2F, 0xA3, 0xCE, 0xCE, 0x0D, 0x46, 0x16, 0xCD,
 0xEB, 0x39, 0x31, 0xF9, 0x38, 0x03, 0x66, 0x2A, 0x28,
 0xDF, 0x1C, 0xD5, 0x35, 0xB7, 0x31};

 unsigned char result[64];
 KECCAK512_DATA spng;

 printf("\r\nRun Keccak512 tests:\r\n");
 Sponge_init(&spng, 0, 0, 0, 0); //initialize only
 Sponge_data(&spng, data1, 1, 0, SP_NORMAL);  //absorbing data
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, result, 64); //optionaly permute then squeezing hash
 if(!memcmp(vect1, result, 64))
 printf("Test 1 - OK\r\n");
 else printf("Test 1 - FAILURE\r\n");
 Sponge_init(&spng, 0, 0, 0, 0); //initialize only
 Sponge_data(&spng, data2, 2, 0, SP_NORMAL);  //absorbing data
 //Incremental Sponge_data calls avaliable there
 Sponge_finalize(&spng, result, 64); //optionaly permute then squeezing hash
 if(!memcmp(vect2, result, 64))
 printf("Test 2 - OK\r\n");
 else printf("Test 2 - FAILURE\r\n");
}

void test_havege(void)
{
  havege_state hv;
  unsigned char sysrand[32];
  unsigned char decoded[32];
  char result[65]={0};

  printf("\r\nRun Havege and base64 tests:\r\n");
  memset(sysrand, 0, 32);
  memset(decoded, 0, 32);
  havege_init(&hv);
  havege_random(&hv, sysrand, 32);
  havege_random(&hv, decoded, 32);
  thex2asc(sysrand, 32, result);
  printf("HEX: %s - RND ", result);
  if(memcmp(sysrand, decoded, 32)) printf("OK\r\n");
  else printf("FAIL\r\n");
  memset(decoded, 0, 32);
  b64estr(sysrand, 32, result);
  printf("B64: %s - B64 ", result);
  b64dstr(result, decoded, 32);
  if(!memcmp(sysrand, decoded, 32)) printf("OK\r\n");
  else printf("FAIL\r\n");

}


  int
main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	printf("Tiny Crypto Library for OnionPhone\r\n");
	printf("    http://torfone.org\r\n");
	printf("(C) Van Gegel For educatin only!\r\n\r\n");

	ec25519_test();
	test_crypto_curve25519_donna();
	test_keccak();
	test_havege();

 return 0;
}




 
  
  
