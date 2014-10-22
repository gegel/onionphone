 ///////////////////////////////////////////////
//
// **************************
// ** ENGLISH - 04/04/2014 **
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

 //SpongePRG implementation
 //This code based on: http://eprint.iacr.org/2011/499.pdf
 //Self-thread

#define USE_SYSTEM_SEED  //Use system random generator for initialisation SPRNG
#define USE_HAVEGE  //Use Havage entropy  for initialisation SPRNG
#define USE_FILESEED  //Use seed-file for initialisation SPRNG

#ifdef USE_SYSTEM_SEED
 #ifdef _WIN32
   #include <sys/stat.h>
#include <sys/types.h>
//#include <winnt.h>
#include <stddef.h>
#include <stdlib.h>
#include <basetsd.h>
#include <stdint.h>
#include <fixedint.h>
#include "basetsd.h"
//#include <windef.h>

#include <stdint.h>
#include <fixedint.h>
#include <basetsd.h>




  #include <windows.h>
  #include <wincrypt.h>
 #else
  #include <stdio.h>
 #endif
#endif

#ifdef USE_HAVEGE
 #include "havege.h"
#endif

#ifdef USE_FILESEED
 #include <stdio.h>
#endif

#include "sponge.h"
#include "sprng.h"
#define RATE (cKeccakR/8-1)

static KECCAK512_DATA sponge;
static unsigned avaliable = 0;

//---------------------------------------------------------
//Sponge state protection for multitread realization only
//Better use system-dependent mutex support insteed this
static volatile char mutex=1;

void waitMutex(void)
{
 do
 {
  while(mutex)
  {
   ;
  }
  mutex++;
 }while(mutex!=1);
}

void releaseMutex(void)
{
 mutex=0;   //release mutex: PRG is ready now
}
//----------------------------------------------------

//PRG initialization using entropy sources
int randInit(uchar const *seed, int len)
{
 int ret=0;
 uchar sysrand[32];
 #ifdef USE_HAVEGE
  havege_state hv;
 #endif
 #ifdef USE_FILESEED
 FILE* f;
 #endif

 //Once use system PRG for initialization
 Sponge_init(&sponge, 0, 0, 0, 0); //Inites Sponge
 memset(sysrand, 0, sizeof(sysrand));

 //Use system rand
 #ifdef USE_SYSTEM_SEED
  do {
  #ifdef _WIN32
    HCRYPTPROV prov;

    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))  {
        break;
    }

    if (!CryptGenRandom(prov, sizeof(sysrand), sysrand))  {
        CryptReleaseContext(prov, 0);
        break;
    }
    CryptReleaseContext(prov, 0);
    ret=1;
  #else
    FILE *f = fopen("/dev/urandom", "rb");

    if (f == NULL) {
        break;
    }

    fread(sysrand, 1, sizeof(sysrand), f);
    fclose(f);
    ret=1;
  #endif
  } while(0);
  Sponge_data(&sponge, sysrand, sizeof(sysrand), 0, SP_DUPLEX);
 #endif

 //Use Havege
 #ifdef USE_HAVEGE
  memset(sysrand, 0, sizeof(sysrand));
  havege_init(&hv);
  havege_random(&hv, sysrand, sizeof(sysrand));
  Sponge_data(&sponge, sysrand, sizeof(sysrand), 0, SP_DUPLEX);
  ret+=2;
 #endif

 //Use seed from file
 #ifdef USE_FILESEED
  f = fopen( "seed", "rb" );
  if(f) {
   memset(sysrand, 0, sizeof(sysrand));
   fread(sysrand, 1, 32, f);
   fclose(f);
   Sponge_data(&sponge, sysrand, sizeof(sysrand), 0, SP_DUPLEX);
   ret+=4;
  }
 #endif

 //Use external entropy
 if((seed)&&(len))
 {
  Sponge_data(&sponge, seed, len, 0, SP_DUPLEX); //Absorbing seed
  ret+=8;
 }

 avaliable=0; //no randoms avaliable now

 memset(sysrand, 0, sizeof(sysrand)); //Clears sysrand
 releaseMutex();   //release mutex: PRG is ready now
 return ret;
}


//PRG feed request (reseeding)
void randFeed(uchar const *seed, int len)
{
 waitMutex();
 Sponge_data(&sponge, seed, len, 0, SP_DUPLEX); //absorbing entropy
 //some bytes of seed can remained in sponge state - not all seed processed
 if(sponge.bytesInQueue>0) avaliable=0; //in this case no random bytes avaliable yet
 else avaliable=RATE; //otherwise a full block of random is avaliable
 releaseMutex();
}


//PRG feed request (generates random)
void randFetch(uchar *randout, int len)
{
 waitMutex();
 if((sponge.bytesInQueue)||(!avaliable)) //first process remaining seed bytes
 {
  Sponge_data(&sponge, 0, 0, 0, SP_DUPLEX|SP_FORCE); //process seed bytes in queue
  avaliable=RATE; //now avaliable full block of random data
 }

 for ( /* empty */; len>0; len-- ) //outputs avaliable random data first
 {
  (*randout++)=sponge.state[RATE-avaliable]; //squeezing avaliable random bytes
  if(avaliable--) continue;
  Sponge_data(&sponge, 0, 0, 0, SP_DUPLEX|SP_FORCE); //permute state for get new block of avaliable random data
  avaliable=RATE;
 }
 releaseMutex();
}

//PRG forget request
void randForget(void)
{
 waitMutex();
 Sponge_data(&sponge, 0, 0, 0, SP_FORGET); //forgets state
 avaliable=RATE;
 releaseMutex();
}

//PRG secure destroy
int randDestroy(void)
{
 int ret=0;
 //Save seed to file
 #ifdef USE_FILESEED
  FILE* f;
  uchar sysrand[32];
  waitMutex();
  f = fopen( "seed", "wb" );
  if(f) {
   Sponge_finalize(&sponge, sysrand, sizeof(sysrand));
   fwrite(sysrand, 1, 32, f);
   fclose(f);
   ret=1;
  }
  memset(sysrand, 0, sizeof(sysrand));
 #else
  waitMutex();
  Sponge_finalize(&sponge, 0, 0); //no F calls, cleares state only
 #endif
 return ret;
}


#if 0

#include <stdio.h>

void
dumprand(unsigned count)
{
	uchar c;

	printf("%u random bytes:\n", count);

	while (count--) {
		randFetch(&c, 1);
		printf("%02x ", (unsigned)c);
	}
	putchar('\n');
}

int
main(int argc, char **argv)
{
	int len;

	while (--argc) {
		len = strlen(*++argv);
		printf("Adding \"%.*s\\0\" to pool.\n", len, *argv);
		randFeed((uchar *)*argv, len+1);
	}
	dumprand(100);
	return 0;
}

#endif
