/* -*- c -*- */

/*
 * bytecode.c
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
#include <stdlib.h>

#include "memory.h"
#include "arith.h"
#include "error.h"
#include "input.h"
#include "parser.h"
#include "bcoutput.h"

#include "bytecode.h"

static bytecode*
allocBytecode (int type)
{
    bytecode *bc = (bytecode*)memXAlloc(sizeof(bytecode));

    bc->type = type;

    return bc;
}

static value*
followOneSubscript (value *theValue, bcSubscript *subscript, environment *env)
{
    value *indexValue;
    int needCopy;

    indexValue = bcExecuteIntoValue(subscript->bc, env, &needCopy);
    if (needCopy)
	indexValue = valueNewScalarFromValue(indexValue);
    else
	valueTransformToScalar(indexValue);

    if (subscript->type == BYTECODE_SUBSCRIPT_LIST)
    {
	int index;

	assert(theValue->type == VALUE_LIST);

	index = atoi(indexValue->v.scalar.scalar.data);

	if (index < 0 || index >= valueListLength(theValue))
	{
	    issueError(ERRMAC_INDEX_OUT_OF_BOUNDS, index);
	    return 0;
	}

	return valueListGetElement(theValue, index);
    }
    else if (subscript->type == BYTECODE_SUBSCRIPT_HASH)
    {
	value *result;

	if (theValue->type != VALUE_HASH)
	{
	    issueError(ERRMAC_LIST_INDEXED_AS_HASH, indexValue->v.scalar.scalar.data);
	    return 0;
	}

	result = valueHashLookup(theValue, &indexValue->v.scalar.scalar);

	if (result == 0)
	{
	    issueError(ERRMAC_KEY_NOT_IN_HASH, indexValue->v.scalar.scalar.data);
	    return 0;
	}

	return result;
    }
    else
	assert(0);

    return 0;
}

static value*
followAllSubscripts (value *theValue, bcSubscript *subscript, environment *env)
{
    while (theValue != 0 && subscript != 0)
    {
	theValue = followOneSubscript(theValue, subscript, env);
	subscript = subscript->next;
    }

    return theValue;
}

bytecode*
bcNewString (dynstring *str)
{
    bytecode *bc = allocBytecode(BYTECODE_STRING);

    bc->v.string.string = dsCopy(str);

    return bc;
}

bytecode*
bcNewQuotedString (dynstring *str)
{
    bytecode *bc = allocBytecode(BYTECODE_QUOTED_STRING);

    bc->v.string.string = dsCopy(str);

    return bc;
}

bytecode*
bcNewEmpty (void)
{
    bytecode *bc = allocBytecode(BYTECODE_QUOTED_STRING);

    bc->v.string.string = dsEmpty();

    return bc;
}

bytecode*
bcNewAssignment (int modify, bytecode *varName, bcSubscript *subscript, bytecode *newValue)
{
    bytecode *bc = allocBytecode(BYTECODE_ASSIGNMENT);

    bc->v.assignment.modify = modify;
    bc->v.assignment.varName = varName;
    bc->v.assignment.subscript = subscript;
    bc->v.assignment.newValue = newValue;

    return bc;
}

bytecode*
bcNewMacro (int flags, bytecode *nameOrValue, bcSubscript *subscript,
	    int numArgs, bcArgument *args)
{
    bytecode *bc = allocBytecode(BYTECODE_MACRO);

    bc->v.macro.flags = flags;
    bc->v.macro.nameOrValue = nameOrValue;
    bc->v.macro.subscript = subscript;
    bc->v.macro.numArgs = numArgs;
    bc->v.macro.arguments = args;

    return bc;
}

bytecode*
bcNewEvaluation (bytecode *bc)
{
    bytecode *result = allocBytecode(BYTECODE_EVAL);

    result->v.eval.bc = bc;

    return result;
}

bytecode*
bcNewArithmetic (bytecode *bc)
{
    bytecode *result = allocBytecode(BYTECODE_ARITH);

    result->v.arith.bc = bc;

    return result;
}

bytecode*
bcRemoveOuterWS (bytecode *bc)
{
    bytecode *first = bc;

    while (bc != 0 && bc->type == BYTECODE_STRING)
    {
	dsRemoveFrontWS(&bc->v.string.string);
	if (bc->v.string.string.length == 0)
	    first = bc = bc->next;
	else
	    break;
    }
    if (bc == 0)
	return first;

    while (bc->next != 0)
	bc = bc->next;
    if (bc->type == BYTECODE_STRING)
	dsRemoveRearWS(&bc->v.string.string);

    return first;
}

int
bcIsString (bytecode *bc)
{
    while (bc != 0)
    {
	if (bc->type != BYTECODE_STRING && bc->type != BYTECODE_QUOTED_STRING)
	    return 0;
	bc = bc->next;
    }

    return 1;
}

void
bcExecute (bytecode *bc, environment *env, outputWriter *ow)
{
    value *aValue;

    while (bc != 0)
    {
	switch (bc->type)
	{
	    case BYTECODE_STRING :
	    case BYTECODE_QUOTED_STRING :
		OUT_STRING(ow, bc->v.string.string.data, bc->v.string.string.length);
		break;

	    case BYTECODE_EVAL :
		{
		    int needCopy;
		    inputReader ir;

		    aValue = bcExecuteIntoValue(bc->v.eval.bc, env, &needCopy);
		    if (needCopy)
			aValue = valueNewScalarFromValue(aValue);
		    else
			valueTransformToScalar(aValue);

		    ir = irNewDynstring(&aValue->v.scalar.scalar, 0);
		    parParseUntil(&ir, 0, bcwNewOutput(env, ow), 1, env, 0);
		}
		break;

	    case BYTECODE_ARITH :
		{
		    int needCopy;
		    dynstring arithValue;

		    aValue = bcExecuteIntoValue(bc->v.arith.bc, env, &needCopy);
		    if (needCopy)
			aValue = valueNewScalarFromValue(aValue);
		    else
			valueTransformToScalar(aValue);

		    arithValue = arithEvalDS(&aValue->v.scalar.scalar, env);

		    OUT_STRING(ow, arithValue.data, arithValue.length);
		}
		break;

	    case BYTECODE_ASSIGNMENT :
		{
		    int needCopy;
		    value *oldValue,
			*newValue;

		    aValue = bcExecuteIntoValue(bc->v.assignment.varName, env, &needCopy);
		    if (needCopy)
			aValue = valueNewScalarFromValue(aValue);
		    else
			valueTransformToScalar(aValue);

		    oldValue = envGetValueForBinding(env, &aValue->v.scalar.scalar);

		    if (oldValue == 0)
		    {
			assert(!bc->v.assignment.modify && bc->v.assignment.subscript == 0);

			newValue = bcExecuteIntoValue(bc->v.assignment.newValue, env, &needCopy);
			if (needCopy)
			    newValue = valueCopy(newValue);

			assert(newValue != 0);
			envAddBinding(globalEnvironment, &aValue->v.scalar.scalar, newValue);
		    }
		    else
		    {
			newValue = bcExecuteIntoValue(bc->v.assignment.newValue, env, &needCopy);

			if (oldValue->type == VALUE_INTERNAL)
			{
			    if (oldValue->v.internal.setFunc != 0)
				oldValue->v.internal.setFunc(newValue, needCopy,
							     bc->v.assignment.subscript, env);
			    else
			    {
				issueError(ERRMAC_SET_INTERNAL, aValue->v.scalar.scalar.data);
				break;
			    }
			}
			else
			{
			    if (needCopy)
				newValue = valueCopy(newValue);

			    if (bc->v.assignment.subscript != 0)
			    {
				bcSubscript *subscript = bc->v.assignment.subscript;

				while (oldValue != 0 && subscript->next != 0)
				{
				    oldValue = followOneSubscript(oldValue, subscript, env);
				    subscript = subscript->next;
				}

				assert(oldValue != 0);

				if (bc->v.assignment.modify)
				{
				    oldValue = followOneSubscript(oldValue, subscript, env);
				    assert(oldValue != 0);
				    valueAssign(oldValue, newValue, 1);
				}
				else
				{
				    value *indexValue;

				    indexValue = bcExecuteIntoValue(subscript->bc, env,
								    &needCopy);
				    if (needCopy)
					indexValue = valueNewScalarFromValue(indexValue);
				    else
					valueTransformToScalar(indexValue);

				    if (subscript->type == BYTECODE_SUBSCRIPT_LIST)
				    {
					int index;

					assert(oldValue->type == VALUE_LIST);

					index = atoi(indexValue->v.scalar.scalar.data);

					assert(index >= 0);

					valueListSetElement(oldValue, index, newValue);
				    }
				    else if (subscript->type == BYTECODE_SUBSCRIPT_HASH)
				    {
					assert(oldValue->type == VALUE_HASH);

					valueHashDefine(oldValue, &indexValue->v.scalar.scalar,
							newValue);
				    }
				    else
					assert(0);
				}
			    }
			    else
			    {
				if (bc->v.assignment.modify)
				    valueAssign(oldValue, newValue, 1);
				else
				{
				    assert(newValue != 0);
				    envModifyBinding(env, &aValue->v.scalar.scalar, newValue);
				}
			    }
			}
		    }
		}
		break;

	    case BYTECODE_MACRO :
		aValue = bcExecuteIntoValue(bc->v.macro.nameOrValue, env, 0);

		if (!(bc->v.macro.flags & BYTECODE_MACRO_SUBVALUE))
		{
		    value *newValue;

		    assert(aValue->type == VALUE_SCALAR);
		    newValue = envGetValueForBinding(env, &aValue->v.scalar.scalar);

		    if (newValue == 0)
		    {
			issueError(ERRMAC_UNBOUND_VARIABLE, aValue->v.scalar.scalar.data);
			break;
		    }
		    else
			aValue = newValue;
		}

		if (aValue->type == VALUE_INTERNAL)
		{
		    if (aValue->v.internal.getFunc != 0)
			aValue->v.internal.getFunc(bc->v.macro.subscript, env, ow);
		    else
		    {
			issueError(ERRMAC_GET_INTERNAL, aValue->v.scalar.scalar.data);
			break;
		    }
		}
		else
		{
		    aValue = followAllSubscripts(aValue, bc->v.macro.subscript, env);

		    if (aValue == 0)
			break;

		    if (bc->v.macro.numArgs >= 0)
		    {
			if (aValue->type == VALUE_BUILT_IN)
			{
			    macroArgument *args =
				(macroArgument*)memXAlloc(sizeof(macroArgument)
							  * bc->v.macro.numArgs);
			    bcArgument *arg = bc->v.macro.arguments;
			    int i;

			    for (i = 0; i < bc->v.macro.numArgs; ++i)
			    {
				if (aValue->v.builtIn.evalParams)
				    args[i].value.value =
					bcExecuteIntoValue(arg->bc, env,
							   &args[i].value.needCopy);
				else
				    args[i].bytecode = arg->bc;

				arg = arg->next;
			    }

			    aValue->v.builtIn.function(bc->v.macro.numArgs, args, env, ow);
			}
			else if (aValue->type == VALUE_LAMBDA)
			{
			    environment *macroEnv = envNew(aValue->v.lambda.env);
			    bcArgument *arg = bc->v.macro.arguments;
			    int i;

			    if (bc->v.macro.numArgs
				< aValue->v.lambda.numParams + aValue->v.lambda.minVarArgs
				|| (bc->v.macro.numArgs
				    > aValue->v.lambda.numParams + aValue->v.lambda.maxVarArgs
				    && aValue->v.lambda.maxVarArgs >= 0))
			    {
				issueError(ERRMAC_WRONG_NUM_ARGS_USER, bc->v.macro.numArgs,
					   aValue->v.lambda.numParams);
				break;
			    }

			    for (i = 0; i < aValue->v.lambda.numParams; ++i)
			    {
				int needCopy;
				value *argValue = bcExecuteIntoValue(arg->bc, env, &needCopy);

				if (needCopy)
				    argValue = valueCopy(argValue);

				assert(argValue != 0);
				envAddBinding(macroEnv, &aValue->v.lambda.paramNames[i],
					      argValue);

				arg = arg->next;
			    }

			    if (aValue->v.lambda.maxVarArgs != 0)
			    {
				value *theList = valueNewList();

				for ( ; i < bc->v.macro.numArgs; ++i)
				{
				    int needCopy;
				    value *argValue =
					bcExecuteIntoValue(arg->bc, env, &needCopy);

				    if (needCopy)
					argValue = valueCopy(argValue);

				    valueListSetElement(theList, i - aValue->v.lambda.numParams,
							argValue);

				    arg = arg->next;
				}

				assert(theList != 0);
				envAddBinding(macroEnv,
					      &aValue->v.lambda.paramNames
					          [aValue->v.lambda.numParams],
					      theList);
			    }

			    bcExecute(aValue->v.lambda.code, macroEnv, ow);
			}
			else
			    assert(0);
		    }
		    else
		    {
			if (bc->v.macro.flags & BYTECODE_MACRO_BYREF)
			{
			    OUT_VALUE_REF(ow, aValue);
			}
			else
			{
			    OUT_VALUE(ow, aValue);
			}
		    }
		}
		break;
	}

	bc = bc->next;
    }
}

value*
bcExecuteIntoValue (bytecode *bc, environment *env, int *needCopy)
{
    outputWriter ow = owNewValue(1);
    value *result;

    bcExecute(bc, env, &ow);

    result = owValueValue(&ow);
    if (needCopy != 0)
	*needCopy = owValueNeedCopy(&ow);

    if (result->type == VALUE_UNDEFINED)
	valueTransformToScalar(result);

    return result;
}

value*
bcExecuteIntoCopiedValue (bytecode *bc, environment *env)
{
    value *theValue;
    int needCopy;

    theValue = bcExecuteIntoValue(bc, env, &needCopy);
    if (needCopy)
	theValue = valueCopy(theValue);

    return theValue;
}

dynstring
bcExecuteIntoDS (bytecode *bc, environment *env)
{
    dynstring result = dsNew();
    outputWriter ow = owNewDynstring(&result);

    bcExecute(bc, env, &ow);

    return result;
}

void
bcExecuteMacro (value *macro, value *argList, environment *env, outputWriter *ow)
{
    int numArgs = valueListLength(argList);

    if (macro->type == VALUE_BUILT_IN)
    {
	macroArgument *args =
	    (macroArgument*)memXAlloc(sizeof(macroArgument) * numArgs);
	int i;

	assert(macro->v.builtIn.evalParams);

	for (i = 0; i < numArgs; ++i)
	{
	    args[i].value.value = valueListGetElement(argList, i);
	    args[i].value.needCopy = 0;
	}

	macro->v.builtIn.function(numArgs, args, env, ow);
    }
    else if (macro->type == VALUE_LAMBDA)
    {
	environment *macroEnv = envNew(macro->v.lambda.env);
	int i;

	if (numArgs < macro->v.lambda.numParams + macro->v.lambda.minVarArgs
	    || (numArgs > macro->v.lambda.numParams + macro->v.lambda.maxVarArgs
		&& macro->v.lambda.maxVarArgs >= 0))
	{
	    issueError(ERRMAC_WRONG_NUM_ARGS_USER, numArgs,
		       macro->v.lambda.numParams);
	    return;
	}

	for (i = 0; i < macro->v.lambda.numParams; ++i)
	    envAddBinding(macroEnv, &macro->v.lambda.paramNames[i],
			  valueListGetElement(argList, i));

	if (macro->v.lambda.maxVarArgs != 0)
	{
	    value *list = valueNewList();

	    for ( ; i < numArgs; ++i)
		valueListSetElement(list, i - macro->v.lambda.numParams,
				    valueListGetElement(argList, i));

	    envAddBinding(macroEnv,
			  &macro->v.lambda.paramNames
			  [macro->v.lambda.numParams],
			  list);
	}

	bcExecute(macro->v.lambda.code, macroEnv, ow);
    }
}

value*
bcExecuteMacroIntoValue (value *macro, value *argList, environment *env, int *needCopy)
{
    outputWriter ow = owNewValue(1);
    value *result;

    bcExecuteMacro(macro, argList, env, &ow);

    result = owValueValue(&ow);
    if (needCopy != 0)
	*needCopy = owValueNeedCopy(&ow);

    if (result->type == VALUE_UNDEFINED)
	valueTransformToScalar(result);

    return result;
}

value*
bcExecuteMacroIntoCopiedValue (value *macro, value *argList, environment *env)
{
    int needCopy;
    value *theValue = bcExecuteMacroIntoValue(macro, argList, env, &needCopy);

    if (needCopy)
	theValue = valueCopy(theValue);

    return theValue;
}

dynstring
bcExecuteMacroIntoDS (value *macro, value *argList, environment *env)
{
    dynstring result = dsNew();
    outputWriter ow = owNewDynstring(&result);

    bcExecuteMacro(macro, argList, env, &ow);

    return result;
}
