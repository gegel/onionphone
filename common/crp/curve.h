#pragma once

#ifndef _CURVE_H_
#define _CURVE_H_

int curve25519_donna(unsigned char *mypublic, const unsigned char *secret, const unsigned char *basepoint);
int get_pubkey(unsigned char *mypublic, const unsigned char *secret);

#endif /* _CURVE_H_ */

