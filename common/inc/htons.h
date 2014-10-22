#include <stdint.h>

#if defined(M_BIG_ENDIAN) && !defined(M_LITTLE_ENDIAN)

#define htons(A) (A)
#define htonl(A) (A)
#define ntohs(A) (A)
#define ntohl(A) (A)

#elif defined(M_LITTLE_ENDIAN) && !defined(M_BIG_ENDIAN)

#define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
		(((uint16_t)(A) & 0x00ff) << 8))
#define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
		(((uint32_t)(A) & 0x00ff0000) >> 8) | \
		(((uint32_t)(A) & 0x0000ff00) << 8) | \
		(((uint32_t)(A) & 0x000000ff) << 24))
#define ntohs htons
#define ntohl htohl

#else

#error "Either BIG_ENDIAN or LITTLE_ENDIAN must be #defined, but not both."

#endif

