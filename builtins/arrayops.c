/* -*- c -*- */

/*
 * builtins/arrayops.c
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

#include "../regex.h"
#include "../error.h"
#include "../output.h"
#include "../bytecode.h"
#include "../environment.h"

#include "builtins.h"

void
builtInLinsert (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "linsert");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else
    {
	int index;

	assureArgumentIsCopy(&args[0]);
	transformArgumentToScalar(&args[1]);
	assureArgumentIsCopy(&args[2]);

	index = atoi(args[1].value.value->v.scalar.scalar.data);
	if (index < 0)
	    issueError(ERRMAC_INDEX_OUT_OF_BOUNDS, index);
	else
	    valueListInsertElement(args[0].value.value, index, args[2].value.value);
    }
}

void
builtInLdelete (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "ldelete");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else
    {
	int index;

	assureArgumentIsCopy(&args[0]);
	transformArgumentToScalar(&args[1]);
	index = atoi(args[1].value.value->v.scalar.scalar.data);
	if (index < 0 || index >= valueListLength(args[0].value.value))
	    issueError(ERRMAC_INDEX_OUT_OF_BOUNDS, index);
	else
	    valueListDeleteElement(args[0].value.value, index);
    }
}

void
builtInListlength (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char lengthString[64];

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "llength");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else
	OUT_STRING(ow, lengthString,
		   sprintf(lengthString, "%d", valueListLength(args[0].value.value)));
}

void
builtInJoin (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "ljoin");
	return;
    }

    if (args[1].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[1].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else
    {
	int length = valueListLength(args[1].value.value);

	if (length > 0)
	{
	    int i;

	    transformArgumentToScalar(&args[0]);

	    OUT_VALUE(ow, valueListGetElement(args[1].value.value, 0));
	    for (i = 1; i < length; ++i)
	    {
		OUT_STRING(ow, args[0].value.value->v.scalar.scalar.data,
			   args[0].value.value->v.scalar.scalar.length);
		OUT_VALUE(ow, valueListGetElement(args[1].value.value, i));
	    }
	}
    }
}

#define NODE_VALUE(n)    ((value*)(avlKey(n)))

static int
compareValues (value *v1, value *v2, value *sortMacro, environment *env)
{
    static value *argList = 0;

    dynstring result;

    if (argList == 0)
	argList = valueNewList();

    valueListSetElement(argList, 0, v1);
    valueListSetElement(argList, 1, v2);
    result = bcExecuteMacroIntoDS(sortMacro, argList, env);

    valueListSetElement(argList, 0, 0);
    valueListSetElement(argList, 1, 0);

    return atoi(result.data);
}

static void
quicksortArray (avlTree *tree, avlNode *l, int lIndex, avlNode *r, int rIndex,
		value *sortMacro, environment *env)
{
    avlNode *v, *i, *j;
    int vIndex,
	iIndex,
	jIndex;
    value *x,
	t;

    assert(lIndex < rIndex);

    iIndex = lIndex; i = l;
    jIndex = rIndex; j = r;

    vIndex = (lIndex + rIndex) / 2; v = avlFirst(tree, vIndex);
    x = NODE_VALUE(v);

    do
    {
	while (compareValues(NODE_VALUE(i), x, sortMacro, env) < 0)
	{
	    ++iIndex; i = avlNext(i);
	}
	while (compareValues(x, NODE_VALUE(j), sortMacro, env) < 0)
	{
	    --jIndex; j = avlPrevious(j);
	}

	if (iIndex <= jIndex)
	{
	    valueAssign(&t, NODE_VALUE(i), 1);
	    valueAssign(NODE_VALUE(i), NODE_VALUE(j), 1);
	    valueAssign(NODE_VALUE(j), &t, 1);

	    ++iIndex; i = avlNext(i);
	    --jIndex; j = avlPrevious(j);
	}
    } while (iIndex <= jIndex);

    if (lIndex < jIndex)
	quicksortArray(tree, l, lIndex, j, jIndex, sortMacro, env);
    if (iIndex < rIndex)
	quicksortArray(tree, i, iIndex, r, rIndex, sortMacro, env);
}

void
builtInSort (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int length;

    if (!(numArgs == 1 || numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "lsort");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else if (numArgs == 2 && args[1].value.value->type != VALUE_LAMBDA
	     && args[1].value.value->type != VALUE_BUILT_IN)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[1].value.value->type),
		   cStringForValueType(VALUE_LAMBDA));
    else
    {
	value *sortMacro;

	if (numArgs == 2)
	    sortMacro = args[1].value.value;
	else
	{
	    dynstring ds = dsNewFrom("scmp");

	    sortMacro = envGetValueForBinding(globalEnvironment, &ds);
	}

	assureArgumentIsCopy(&args[0]);
	length = valueListLength(args[0].value.value);
	if (length >= 2)
	    quicksortArray(args[0].value.value->v.list.list,
			   avlFirst(args[0].value.value->v.list.list, 1), 1,
			   avlFirst(args[0].value.value->v.list.list, length), length,
			   sortMacro, env);

	OUT_VALUE(ow, args[0].value.value);
    }
}

void
builtInAppend (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs >= 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "lappend");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else
    {
	int i;

	assureArgumentIsCopy(&args[0]);

	for (i = 1; i < numArgs; ++i)
	{
	    assureArgumentIsCopy(&args[i]);
	    valueListSetElement(args[0].value.value, valueListLength(args[0].value.value),
				args[i].value.value);
	}
    }
}

static int
valuesEqual (value *v1, value *v2, value *equalityMacro, environment *env)
{
    static value *argList = 0;

    value *result;

    if (argList == 0)
	argList = valueNewList();

    valueListSetElement(argList, 0, v1);
    valueListSetElement(argList, 1, v2);
    result = bcExecuteMacroIntoValue(equalityMacro, argList, env, 0);

    valueListSetElement(argList, 0, 0);
    valueListSetElement(argList, 1, 0);

    return valueBoolValue(result);
}

void
builtInUniq (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1 || numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "luniq");
	return;
    }

    if (args[0].value.value->type != VALUE_LIST)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LIST));
    else if (numArgs == 2 && args[1].value.value->type != VALUE_LAMBDA
	     && args[1].value.value->type != VALUE_BUILT_IN)
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[1].value.value->type),
		   cStringForValueType(VALUE_LAMBDA));
    else
    {
	value *equalityMacro;

	if (numArgs == 2)
	    equalityMacro = args[1].value.value;
	else
	{
	    dynstring ds = dsNewFrom("equal");

	    equalityMacro = envGetValueForBinding(globalEnvironment, &ds);
	}

	assureArgumentIsCopy(&args[0]);
	if (valueListLength(args[0].value.value) > 1)
	{
	    value *aValue = valueListGetElement(args[0].value.value, 0);
	    int i = 1;

	    while (i < valueListLength(args[0].value.value))
	    {
		value *element = valueListGetElement(args[0].value.value, i);
		
		if (valuesEqual(aValue, element, equalityMacro, env))
		    valueListDeleteElement(args[0].value.value, i);
		else
		{
		    aValue = element;
		    ++i;
		}
	    }
	}
	OUT_VALUE_REF(ow, args[0].value.value);
    }
}

void
registerArrayOps (void)
{
    registerBuiltIn("linsert", builtInLinsert, 1, 0, 0);
    registerBuiltIn("ldelete", builtInLdelete, 1, 0, 0);
    registerBuiltIn("llength", builtInListlength, 1, 0, 0);
    registerBuiltIn("ljoin", builtInJoin, 1, 0, 0);
    registerBuiltIn("lsort", builtInSort, 1, 0, 0);
    registerBuiltIn("lappend", builtInAppend, 1, 0, 0);
    registerBuiltIn("luniq", builtInUniq, 1, 0, 0);
}
