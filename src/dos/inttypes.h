#ifndef INT_TYPES_H_
#define INT_TYPES_H_

#if defined(__DOS__) || defined(WIN32)
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef unsigned long intptr_t;
#else
#include <stdint.h>
#endif

#endif	/* INT_TYPES_H_ */
