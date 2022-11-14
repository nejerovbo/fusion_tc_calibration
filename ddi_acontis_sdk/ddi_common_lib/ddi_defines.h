/*****************************************************************************
 * (c) Copyright 2018-2020 Digital Dynamics Inc. Scotts Valley CA USA.
 * Unpublished copyright. All rights reserved. Contains proprietary and
 * confidential trade secrets belonging to DDI. Disclosure or release without
 * prior written authorization of DDI is prohibited.
 *****************************************************************************/
/*  ddi_defines.h
 *  Created by Johana Lehrer on 2018-09-07
 */

#ifndef DDI_DEFINES_H
#define DDI_DEFINES_H

#include <stdint.h>

#ifdef __linux__
#include <arpa/inet.h>
#endif

#define OK                        1
#define NOT_OK                    0

// Uncomment or define in build environment for big endian CPU architecture
//#define BIG_ENDIAN

#ifdef _WIN32

#define PACKED_BEGIN __pragma(pack(push, 1))
#define PACKED
#define PACKED_END __pragma(pack(pop))

#else

#define PACKED_BEGIN
#define PACKED  __attribute__((__packed__))
#define PACKED_END

#endif

#ifndef NULL
#if (defined(__cplusplus) && (__cplusplus >= 201103L))
 #define NULL nullptr
#else
 #define NULL ((void *)0)
#endif
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y)) ? (x) : (y)
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y)) ? (x) : (y)
#endif

#define ARRAY_ELEMENTS(x) (sizeof((x))/sizeof((x)[0]))

#define SWAP32(v)  (((((uint32_t)(v)) >> 24) & 0x000000ff) | \
                    ((((uint32_t)(v)) >> 8)  & 0x0000ff00) | \
                    ((((uint32_t)(v)) << 8)  & 0x00ff0000) | \
                    ((((uint32_t)(v)) << 24) & 0xff000000))

#define SWAP16(v) ((((v) >> 8) & 0xff) | (((v) << 8) & 0xff00))

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#ifndef htons
#define htons(v)    (v)
#define ntohs htons
#endif

#ifndef htonl
#define htonl(v)    (v)
#define ntohl htonl
#endif

#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#ifndef htons
#define htons(v) SWAP16(v)
#define ntohs htons
#endif

#ifndef htonl
#define htonl(v) SWAP32(v)
#define ntohl htonl
#endif

#else
#ifndef _WIN32
#error Endian BYTE ORDER not defined
#endif
#endif

#endif // DDI_DEFINES_H
