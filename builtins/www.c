/* -*- c -*- */

/*
 * builtins/www.c
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

#include "../environment.h"
#include "../output.h"
#include "../error.h"

#include "builtins.h"

void
builtInUrldecode (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i,
	length;
    char *data;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "w3urldecode");
	return;
    }

    transformArgumentToScalar(&args[0]);

    length = args[0].value.value->v.scalar.scalar.length;
    data = args[0].value.value->v.scalar.scalar.data;
    for (i = 0; i < length; ++i)
    {
	int c;

	if (data[i] == '+')
	    c = ' ';
	else if (data[i] == '%')
	{
	    assert(i + 2 < length);
	    ++i;

	    if (data[i] >= '0' && data[i] <= '9')
		c = data[i] - '0';
	    else if (data[i] >= 'a' && data[i] <= 'f')
		c = data[i] - 'a' + 10;
	    else if (data[i] >= 'A' && data[i] <= 'F')
		c = data[i] - 'A' + 10;
	    else
		assert(0);

	    c <<= 4;
	    ++i;

	    if (data[i] >= '0' && data[i] <= '9')
		c |= data[i] - '0';
	    else if (data[i] >= 'a' && data[i] <= 'f')
		c |= data[i] - 'a' + 10;
	    else if (data[i] >= 'A' && data[i] <= 'F')
		c |= data[i] - 'A' + 10;
	    else
		assert(0);
	}
	else
	    c = data[i];

	OUT_CHAR(ow, c);
    }
}

void
registerWWWBuiltIns (void)
{
    registerBuiltIn("w3urldecode", builtInUrldecode, 1, 0, 0);
}
