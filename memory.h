/* -*- c -*- */

/*
 * memory.h
 *
 * chpp
 *
 * Copyright (C) 1998 Mark Probst
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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#ifndef USE_EFENCE

#include "gc-7.2alpha6/include/gc.h"

#define memAlloc(n)              GC_malloc(n)
#define memAllocAtomic(n)        GC_malloc_atomic(n)
#define memFree(p)
#define memRealloc(p,n)          GC_realloc(p,n)

#else

#include <stdlib.h>

#define memAlloc(n)              calloc(1,n)
#define memAllocAtomic(n)        calloc(1,n)
#define memFree(p)
#define memRealloc(p,n)          realloc(p,n)

#endif

void* memXAlloc (int numBytes);
void* memXAllocAtomic (int numBytes);
void* memXRealloc (void *object, int numBytes);

#endif
