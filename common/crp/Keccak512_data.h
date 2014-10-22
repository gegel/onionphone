/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by Ronny Van Keer,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#ifndef KECCAK512_DATA_H
#define KECCAK512_DATA_H

#include <memory.h>
#ifdef _WIN32
 //#include "pstdint.h"
 #include "stdint.h"
#else
 #include "stdint.h"
#endif



// ** Thread-safe implementation

// ** Keccak hashing
// ** 512bit hash

#ifndef _WIN32
typedef uint32_t DWORD;
#endif

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint64_t QWORD;


#define cKeccakB    1600
#define cKeccakR    576
#define cKeccakFixedOutputLengthInBytes 64

typedef struct {
	BYTE	state[cKeccakB / 8];
        int             bytesInQueue;
} KECCAK512_DATA;

#endif
