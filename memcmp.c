/* -*- c -*- */

/*
 * memcmp.c
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

#include <stdlib.h>

int
memcmp (const void *s1, const void *s2, size_t n)
{
    unsigned char *p1 = (unsigned char*)s1,
	*p2 = (unsigned char*)s2;

    while (n-- != 0)
    {
	if (*p1 != *p2)
	{
	    if (*p1 < *p2)
		return -1;
	    else
		return 1;
	}
	++p1;
	++p2;
    }

    return 0;
}
