/* -*- c -*- */

/*
 * memory.c
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

#include <assert.h>
#include <stdio.h>

#include "memory.h"

void*
memXAlloc (int numBytes)
{
    if (numBytes == 0)
	return 0;
    else
    {
	void *object = memAlloc(numBytes);

	if (object == 0)
	{
	    fprintf(stderr, "memory exhausted\n");
	    exit(1);
	}

	return object;
    }
}

void*
memXAllocAtomic (int numBytes)
{
    if (numBytes == 0)
	return 0;
    else
    {
	void *object = memAllocAtomic(numBytes);

	if (object == 0)
	{
	    fprintf(stderr, "memory exhausted\n");
	    exit(1);
	}

	return object;
    }
}

void*
memXRealloc (void *object, int numBytes)
{
    if (numBytes == 0)
	return 0;
    else
    {
	object = memRealloc(object, numBytes);
	if (object == 0)
	{
	    fprintf(stderr, "memory exhausted\n");
	    exit(1);
	}

	return object;
    }
}
