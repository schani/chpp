/* -*- c -*- */

/*
 * builtins/flowctl.c
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
#include <stdlib.h>
#include <stdio.h>

#include "../memory.h"
#include "../list.h"
#include "../error.h"
#include "../chash.h"
#include "../environment.h"
#include "../bytecode.h"

#include "builtins.h"

void
builtInIf (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *cond;

    if (!(numArgs == 2 || numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "if");
	return;
    }

    cond = bcExecuteIntoValue(args[0].bytecode, env, 0);
    if (valueBoolValue(cond))
	bcExecute(args[1].bytecode, env, ow);
    else if (numArgs == 3)
	bcExecute(args[2].bytecode, env, ow);
}

void
builtInIfGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			 environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));

    if (numArgs == 3)
    {
	environment *intersection =
	    envNewIntersection((environment*)listNThElementData(globalEffectList, 1),
			       (environment*)listNThElementData(globalEffectList, 2));

	envAddBindings(globalEffects, intersection);
    }
}

void
builtInCond (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    if (!(numArgs % 2 == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "cond");
	return;
    }

    for (i = 0; i < numArgs; i += 2)
    {
	if (valueBoolValue(bcExecuteIntoValue(args[i].bytecode, env, 0)))
	{
	    bcExecute(args[i + 1].bytecode, env, ow);
	    return;
	}
    }
}

void
builtInCondGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			   environment *globalEffects)
{
    if (numArgs > 0)
	envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
}

void
builtInCase (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring string;
    int i;

    if (!(numArgs % 2 == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "case");
	return;
    }

    string = bcExecuteIntoDS(args[0].bytecode, env);

    for (i = 1; i < numArgs; i += 2)
    {
	value *theList = bcExecuteIntoValue(args[i].bytecode, env, 0);
	int length,
	    j;

	if (theList->type != VALUE_LIST)
	{
	    if (theList->type == VALUE_SCALAR
		&& strcmp(theList->v.scalar.scalar.data, "else") == 0)
		bcExecute(args[i + 1].bytecode, env, ow);
	    else
		issueError(ERRMAC_VALUE_WRONG_TYPE, cStringForValueType(theList->type),
			   cStringForValueType(VALUE_LIST));

	    return;
	}

	length = valueListLength(theList);
	for (j = 0; j < length; ++j)
	{
	    value *scalar = valueNewScalarFromValue(valueListGetElement(theList, j));

	    if (strcmp(string.data, scalar->v.scalar.scalar.data) == 0)
	    {
		bcExecute(args[i + 1].bytecode, env, ow);
		return;
	    }
	}
    }
}

void
builtInCaseGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			   environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
    if (numArgs > 1)
	envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 1));
}

void
builtInFor (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring counter,
	ds;
    int start,
	stop,
	increment = 1,
	i;

    if (!(numArgs == 4 || numArgs == 5))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "for");
	return;
    }

    ds = bcExecuteIntoDS(args[1].bytecode, env);
    start = atoi(ds.data);

    ds = bcExecuteIntoDS(args[2].bytecode, env);
    stop = atoi(ds.data);

    if (numArgs == 5)
    {
	ds = bcExecuteIntoDS(args[3].bytecode, env);
	increment = atoi(ds.data);

	if (increment == 0)
	{
	    issueError(ERRMAC_NULL_INCREMENT);
	    return;
	}
    }
    else
	if (start <= stop)
	    increment = 1;
	else
	    increment = -1;

    i = start;
    counter = bcExecuteIntoDS(args[0].bytecode, env);

    while ((increment > 0 && i <= stop) || (increment < 0 && i >= stop))
    {
	char numberString[64];
	environment *newEnv = envNew(env);

	sprintf(numberString, "%d", i);
	envAddBinding(newEnv, &counter, valueNewScalarFromCString(numberString));

	bcExecute(args[numArgs - 1].bytecode, newEnv, ow);

	/* i = atoi(entry->value->v.scalar.scalar.data); */
	i += increment;
    }
}

environment*
builtInForEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    if (numArgs < 3)
	return parentEnv;

    if (bcIsString(args->bc))
    {
	dynstring name = bcExecuteIntoDS(args->bc, 0);
	environment *env = envNew(parentEnv);

	envModifyOrAddBinding(env, &name, 0, env);

	return env;
    }

    return parentEnv;
}

void
builtInForeach (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *list;

    if (!(numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "foreach");
	return;
    }

    list = bcExecuteIntoCopiedValue(args[1].bytecode, env);
    if (valueListLength(list) > 0)
    {
	int i;
	dynstring counter = bcExecuteIntoDS(args[0].bytecode, env);

	for (i = 0; i < valueListLength(list); ++i)
	{
	    environment *newEnv = envNew(env);

	    envAddBinding(newEnv, &counter, valueListGetElement(list, i));

	    bcExecute(args[2].bytecode, newEnv, ow);
	}
    }
}

environment*
builtInForeachEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    if (numArgs < 2)
	return parentEnv;

    if (bcIsString(args->bc))
    {
	dynstring name = bcExecuteIntoDS(args->bc, 0);
	environment *env = envNew(parentEnv);

	envModifyOrAddBinding(env, &name, 0, env);

	return env;
    }

    return parentEnv;
}

void
builtInForeachkey (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *hash;
    dynstring counter;
    int count,
	i;
    hstate state;

    if (!(numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "foreachkey");
	return;
    }

    hash = bcExecuteIntoCopiedValue(args[1].bytecode, env);
    if (hash->type != VALUE_HASH)
    {
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(hash->type),
		   cStringForValueType(VALUE_HASH));
	return;
    }
    counter = bcExecuteIntoDS(args[0].bytecode, env);

    count = hash_count(hash->v.hash.hash);
    state = hash_state(hash->v.hash.hash);
    for (i = 0; i < count; ++i)
    {
	environment *newEnv = envNew(env);
	char *key;

	hash_next(&state, &key);
	envAddBinding(newEnv, &counter, valueNewScalarFromCString(key));

	bcExecute(args[2].bytecode, newEnv, ow);
    }
}

environment*
builtInForeachkeyEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    if (numArgs < 2)
	return parentEnv;

    if (bcIsString(args->bc))
    {
	dynstring name = bcExecuteIntoDS(args->bc, 0);
	environment *env = envNew(parentEnv);

	envModifyOrAddBinding(env, &name, 0, env);

	return env;
    }

    return parentEnv;
}

void
builtInWhile (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "while");
	return;
    }

    while (valueBoolValue(bcExecuteIntoValue(args[0].bytecode, env, 0)))
	bcExecute(args[1].bytecode, env, ow);
}

void
builtInWhileGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			    environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
}

void
builtInUntil (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "until");
	return;
    }

    while (!valueBoolValue(bcExecuteIntoValue(args[0].bytecode, env, 0)))
	bcExecute(args[1].bytecode, env, ow);
}

void
builtInUntilGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			    environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
}

void
builtInDowhile (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "dowhile");
	return;
    }

    do
    {
	bcExecute(args[0].bytecode, env, ow);
    } while (valueBoolValue(bcExecuteIntoValue(args[1].bytecode, env, 0)));
}

void
builtInDowhileGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			      environment *globalEffects)
{
    environment *unionEnv =
	envNewUnion((environment*)listNThElementData(globalEffectList, 0),
		    (environment*)listNThElementData(globalEffectList, 1));

    envAddBindings(globalEffects, unionEnv);
}

void
builtInDountil (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "dountil");
	return;
    }

    do
    {
	bcExecute(args[0].bytecode, env, ow);
    } while (!valueBoolValue(bcExecuteIntoValue(args[1].bytecode, env, 0)));
}

void
builtInDountilGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			      environment *globalEffects)
{
    environment *unionEnv =
	envNewUnion((environment*)listNThElementData(globalEffectList, 0),
		    (environment*)listNThElementData(globalEffectList, 1));

    envAddBindings(globalEffects, unionEnv);
}

void
builtInAnd (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    for (i = 0; i < numArgs; ++i)
	if (!valueBoolValue(bcExecuteIntoValue(args[i].bytecode, env, 0)))
	{
	    OUT_CHAR(ow, '0');
	    return;
	}

    OUT_CHAR(ow, '1');
}

void
builtInAndGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			  environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
}

void
builtInOr (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    for (i = 0; i < numArgs; ++i)
	if (valueBoolValue(bcExecuteIntoValue(args[i].bytecode, env, 0)))
	{
	    OUT_CHAR(ow, '1');
	    return;
	}

    OUT_CHAR(ow, '0');
}

void
builtInOrGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			 environment *globalEffects)
{
    envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, 0));
}

void
builtInNot (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!numArgs == 1)
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "not");
	return;
    }

    if (valueBoolValue(args[0].value.value))
    {
	OUT_CHAR(ow, '0');
    }
    else
    {
	OUT_CHAR(ow, '1');
    }
}

void
registerFlowCtl (void)
{
    registerBuiltIn("if", builtInIf, 0, builtInIfGlobalEffector, 0);
    registerBuiltIn("cond", builtInCond, 0, builtInCondGlobalEffector, 0);
    registerBuiltIn("case", builtInCase, 0, 0, 0);
    registerBuiltIn("for", builtInFor, 0, 0, builtInForEnvironmentor);
    registerBuiltIn("foreach", builtInForeach, 0, 0, builtInForeachEnvironmentor);
    registerBuiltIn("foreachkey", builtInForeachkey, 0, 0, builtInForeachkeyEnvironmentor);
    registerBuiltIn("while", builtInWhile, 0, builtInWhileGlobalEffector, 0);
    registerBuiltIn("until", builtInUntil, 0, builtInUntilGlobalEffector, 0);
    registerBuiltIn("dowhile", builtInDowhile, 0, builtInDowhileGlobalEffector, 0);
    registerBuiltIn("dountil", builtInDountil, 0, builtInDountilGlobalEffector, 0);
    registerBuiltIn("and", builtInAnd, 0, builtInAndGlobalEffector, 0);
    registerBuiltIn("or", builtInOr, 0, builtInOrGlobalEffector, 0);
    registerBuiltIn("not", builtInNot, 1, 0, 0);
}
