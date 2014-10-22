#pragma once

#ifndef _OPHMCONSTS_H_
#define _OPHMCONSTS_H_

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef PI
#define PI M_PI
#endif

#ifndef TWOPI
#define TWOPI 6.28318530717958647692
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.4426950408889634074
#endif

#ifndef log2
#define log2(A) (log(A) * M_LOG2E)
#endif

#ifndef exp2
#define exp2(A) (exp(A / M_LOG2E))
#endif

#define F FLOAT
typedef float FLOAT;
typedef float Float32;
typedef double Float64;
typedef short INT16;
typedef int INT32;
typedef char Word8;
typedef short Word16;
typedef int Word32;
typedef unsigned char UWord8;
typedef unsigned int UWord32;
typedef int Flag;

#endif /* _OPHMCONSTS_H_ */

