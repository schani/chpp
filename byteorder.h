/* -*- c -*- */

/*
 * byteorder.h
 *
 * chpp
 *
 * Copyright (C) 1997-1999 Mark Probst
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#include "config.h"
#include "types.h"

#define NEEDS_ALIGNED_ACCESS

#ifdef WORDS_BIGENDIAN

#define bo_canonical_to_host_2(x)        (x)
#define bo_host_to_canonical_2(x)        (x)

#define bo_canonical_to_host_4(x)        (x)
#define bo_host_to_canonical_4(x)        (x)

#else /* WORDS_BIGENDIAN */

#include <sys/types.h>
#include <netinet/in.h>

#define bo_canonical_to_host_2(x)        ntohs(x)
#define bo_host_to_canonical_2(x)        htons(x)

#define bo_canonical_to_host_4(x)        ntohl(x)
#define bo_host_to_canonical_4(x)        htonl(x)

/*
#define bo_canonical_to_host_4(x)        ((((unsigned4)(x) & 0xff) << 24) | \
                                          (((unsigned4)(x) & 0xff00) << 8) | \
                                          (((unsigned4)(x) & 0xff0000) >> 8) | \
                                          ((unsigned4)(x) >> 24))
#define bo_host_to_canonical_4(x)        ((((unsigned4)(x) & 0xff) << 24) | \
                                          (((unsigned4)(x) & 0xff00) << 8) | \
                                          (((unsigned4)(x) & 0xff0000) >> 8) | \
                                          ((unsigned4)(x) >> 24))
*/

#endif /* HAVE_LITTLE_ENDIAN */

#ifdef NEEDS_ALIGNED_ACCESS

#define bo_unsigned2_at_byteaddress(p)           bo_canonical_to_host_2(bo_canonical_unsigned2_at_byteaddress(p))
#define bo_set_unsigned2_at_byteaddress(p,x)     bo_set_canonical_unsigned2_at_byteaddress(p,bo_host_to_canonical_2(x))

#define bo_unsigned4_at_byteaddress(p)           bo_canonical_to_host_4(bo_canonical_unsigned4_at_byteaddress(p))
#define bo_set_unsigned4_at_byteaddress(p,x)     bo_set_canonical_unsigned4_at_byteaddress(p,bo_host_to_canonical_4(x))

#define bo_canonical_unsigned2_at_byteaddress(p) (((unsigned2)*(unsigned1*)(p) << 8) | \
                                                  ((unsigned2)*(unsigned1*)((p)+1)))
#define bo_set_canonical_unsigned2_at_byteaddress(p,x) (*(unsigned1*)(p) = ((unsigned2)(x) >> 8) & 0xff, \
                                                        *(unsigned1*)((p) + 1) = (unsigned2)(x) & 0xff)

#define bo_canonical_unsigned4_at_byteaddress(p) (((unsigned4)*(unsigned1*)(p) << 24) | \
                                                  ((unsigned4)*(unsigned1*)((p) + 1) << 16) | \
                                                  ((unsigned4)*(unsigned1*)((p) + 2) << 8) | \
                                                  ((unsigned4)*(unsigned1*)((p) + 3)))
#define bo_set_canonical_unsigned4_at_byteaddress(p,x) (*(unsigned1*)(p) = ((unsigned4)(x) >> 24) & 0xff, \
                                                        *(unsigned1*)((p) + 1) = ((unsigned4)(x) >> 16) & 0xff, \
                                                        *(unsigned1*)((p) + 2) = ((unsigned4)(x) >> 8) & 0xff, \
                                                        *(unsigned1*)((p) + 3) = (unsigned4)(x) & 0xff)

#else

#define bo_unsigned2_at_byteaddress(p)       (*((unsigned2*)(p)))
#define bo_set_unsigned2_at_byteaddress(p,x) ((*((unsigned2*)(p)))=(x))

#define bo_unsigned4_at_byteaddress(p)       (*((unsigned4*)(p)))
#define bo_set_unsigned4_at_byteaddress(p,x) ((*((unsigned4*)(p)))=(x))

#define bo_canonical_unsigned2_at_byteaddress(p)       bo_canonical_to_host_2(*((unsigned2*)(p)))
#define bo_set_canonical_unsigned2_at_byteaddress(p,x) ((*((unsigned2*)(p)))=bo_host_to_canonical_2(x))

#define bo_canonical_unsigned4_at_byteaddress(p)       bo_canonical_to_host_4(*((unsigned4*)(p)))
#define bo_set_canonical_unsigned4_at_byteaddress(p,x) ((*((unsigned4*)(p)))=bo_host_to_canonical_4(x))

#endif /* NEEDS_ALIGNED_ACCESS */

#endif
