/* -*- c -*- */

/*
 * builtins/environments.c
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

#include <assert.h>

#include "../error.h"
#include "../output.h"
#include "../bytecode.h"
#include "../environment.h"

#include "builtins.h"

void
builtInEnvThis (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "envThis");
	return;
    }

    OUT_VALUE(ow, valueNewEnvironment(env));
}

void
builtInEnvNew (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    environment *theEnv = 0;

    if (!(numArgs <= 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "envNew");
	return;
    }

    if (numArgs == 1)
    {
	value *environment = args[0].value.value;

	if (environment->type != VALUE_ENVIRONMENT)
	{
	    issueError(ERRMAC_VALUE_WRONG_TYPE,
		       cStringForValueType(environment->type),
		       cStringForValueType(VALUE_ENVIRONMENT));
	    return;
	}

	theEnv = environment->v.env.env;
    }

    OUT_VALUE(ow, valueNewEnvironment(envNew(theEnv)));
}

void
builtInEnvAdd (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *environment = 0;

    if (!(numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "envAdd");
	return;
    }

    environment = args[0].value.value;

    if (environment->type != VALUE_ENVIRONMENT)
    {
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(environment->type),
		   cStringForValueType(VALUE_ENVIRONMENT));
	return;
    }

    envAddBinding(environment->v.env.env, &transformArgumentToScalar(&args[1])->v.scalar.scalar, args[2].value.value);
}

void
registerEnvironmentOps (void)
{
    registerBuiltIn("envThis", builtInEnvThis, 1, 0, 0);
    registerBuiltIn("envNew", builtInEnvNew, 1, 0, 0);
    registerBuiltIn("envAdd", builtInEnvAdd, 1, 0, 0);
}
