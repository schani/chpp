/* -*- c -*- */

/*
 * builtins/hashops.c
 *
 * chpp
 *
 * Copyright (C) 1997-1998 Mark Probst
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

#include "../error.h"
#include "../environment.h"
#include "../output.h"

#include "builtins.h"

void
builtInHashcount (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char lengthString[64];

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "hcount");
	return;
    }

    if (args[0].value.value->type != VALUE_HASH)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_HASH));
    else
	OUT_STRING(ow, lengthString,
		   sprintf(lengthString, "%d", valueHashCount(args[0].value.value)));
}

void
builtInHcontains (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "hcontains");
	return;
    }

    if (args[0].value.value->type != VALUE_HASH)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_HASH));
    else
    {
	transformArgumentToScalar(&args[1]);
	if (valueHashLookup(args[0].value.value, &args[1].value.value->v.scalar.scalar) != 0)
	{
	    OUT_CHAR(ow, '1');
	}
	else
	{
	    OUT_CHAR(ow, '0');
	}
    }
}

void
builtInKeys (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "hkeys");
	return;
    }

    if (args[0].value.value->type != VALUE_HASH)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_HASH));
    else
    {
	value *result = valueNewList();
	hstate state;
	value *aValue;
	char *aKey;
	int i = 0;

	state = hash_state(args[0].value.value->v.hash.hash);
	while ((aValue = (value*)hash_next(&state, &aKey)) != 0)
	    valueListSetElement(result, i++, valueNewScalarFromCString(aKey));
	OUT_VALUE(ow, result);
    }
}

void
builtInHdelete (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "hdelete");
	return;
    }

    if (args[0].value.value->type != VALUE_HASH)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_HASH));
    else
	valueHashDelete(args[0].value.value, &transformArgumentToScalar(&args[1])->v.scalar.scalar);
}

void
registerHashOps (void)
{
    registerBuiltIn("hcount", builtInHashcount, 1, 0, 0);
    registerBuiltIn("hcontains", builtInHcontains, 1, 0, 0);
    registerBuiltIn("hkeys", builtInKeys, 1, 0, 0);
    registerBuiltIn("hdelete", builtInHdelete, 1, 0, 0);
}
