#pragma once

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

#ifndef SPRNG_H
#define SPRNG_H

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

int randInit(uchar const *seed, int len);
void randFeed(uchar const *seed, int len);
void randFetch(uchar *randout, int len);
void randForget(void);
int randDestroy(void);

#endif	// SPRNG_H

