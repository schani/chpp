/* -*- c -*- */

/*
 * builtins/builtins.c
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
#include <string.h>
#ifdef _NEXT
#include <libc.h>
#endif

#include "../memory.h"
#include "../error.h"
#include "../value.h"
#include "../main.h"
#include "../depends.h"
#include "../environment.h"
#include "../bytecode.h"

#include "builtins.h"

extern int generateDependencies;

void
assureArgumentIsCopy (macroArgument *arg)
{
    if (arg->value.needCopy)
    {
	arg->value.value = valueCopy(arg->value.value);
	arg->value.needCopy = 0;
    }
}

value*
transformArgumentToScalar (macroArgument *arg)
{
    if (arg->value.needCopy)
    {
	arg->value.value = valueNewScalarFromValue(arg->value.value);
	arg->value.needCopy = 0;
    }
    else
	valueTransformToScalar(arg->value.value);

    return arg->value.value;
}

void
builtInLambda (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring *macroArgs;
    char *pos1;
    int i,
	numMacroArgs,
	minVarArgs = 0,
	maxVarArgs = 0;

    if (!(numArgs >= 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "lambda");
	return;
    }

    macroArgs = (dynstring*)memXAlloc((numArgs - 1) * sizeof(dynstring));
    for (i = 0; i < numArgs - 1; ++i)
	macroArgs[i] = bcExecuteIntoDS(args[i].bytecode, env);

    if (numArgs >= 2 && (pos1 = strchr(macroArgs[numArgs - 2].data, ':')) != 0)
    {
	dynstring numberString;
	char *pos2;

	pos2 = strchr(pos1 + 1, ':');
	assert(pos2 != 0);

	numberString = dsNewFromBytes(pos1 + 1, pos2 - pos1 - 1);
	if (numberString.length > 0)
	    minVarArgs = atoi(numberString.data);
	
	numberString = dsNewFrom(pos2 + 1);
	if (numberString.length == 0)
	    maxVarArgs = -1;
	else
	    maxVarArgs = atoi(numberString.data);

	numMacroArgs = numArgs - 2;

	dsShrinkRear(&macroArgs[numArgs - 2],
		     macroArgs[numArgs - 2].length - (pos1 - macroArgs[numArgs - 2].data));
    }
    else
	numMacroArgs = numArgs - 1;

    OUT_VALUE_REF(ow, valueNewLambda(numMacroArgs, minVarArgs, maxVarArgs, macroArgs, '%',
				     args[numArgs - 1].bytecode, env));
}

environment*
builtInLambdaEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    environment *env;

    if (numArgs == 0)
	return parentEnv;

    env = envNew(parentEnv);
    while (args != 0)
    {
	if (bcIsString(args->bc))
	{
	    dynstring name = bcExecuteIntoDS(args->bc, 0);

	    if (args->next == 0)
	    {
		char *pos = strchr(name.data, ':');

		if (pos != 0)
		    dsShrinkRear(&name, pos - name.data);
	    }

	    if (envGetBinding(env, &name) == 0)
		envAddBinding(env, &name, 0);
	}
	args = args->next;
    }

    return env;
}

void
builtInDefine (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring name;
    char *pos1;
    dynstring *macroArgs;
    int i,
	numMacroArgs,
	minVarArgs = 0,
	maxVarArgs = 0;

    if (!(numArgs >= 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "define");
	return;
    }

    name = bcExecuteIntoDS(args[0].bytecode, env);

    macroArgs = (dynstring*)memXAlloc((numArgs - 2) * sizeof(dynstring));
    for (i = 0; i < numArgs - 2; ++i)
	macroArgs[i] = bcExecuteIntoDS(args[i + 1].bytecode, env);

    if (numArgs >= 3 && (pos1 = strchr(macroArgs[numArgs - 3].data, ':')) != 0)
    {
	dynstring numberString;
	char *pos2;

	pos2 = strchr(pos1 + 1, ':');
	assert(pos2 != 0);

	numberString = dsNewFromBytes(pos1 + 1, pos2 - pos1 - 1);
	if (numberString.length > 0)
	    minVarArgs = atoi(numberString.data);
	
	numberString = dsNewFrom(pos2 + 1);
	if (numberString.length == 0)
	    maxVarArgs = -1;
	else
	    maxVarArgs = atoi(numberString.data);

	numMacroArgs = numArgs - 3;

	dsShrinkRear(&macroArgs[numArgs - 3],
		     macroArgs[numArgs - 3].length - (pos1 - macroArgs[numArgs - 3].data));
    }
    else
	numMacroArgs = numArgs - 2;

    envModifyOrAddBinding(env, &name,
			  valueNewLambda(numMacroArgs, minVarArgs, maxVarArgs,
					 macroArgs, '%', args[numArgs - 1].bytecode, env),
			  globalEnvironment);
}

void
builtInDefineGlobalEffector (int numArgs, bcArgument *args, list *globalEffectList,
			     environment *globalEffects)
{
    int i;

    if (numArgs > 0 && bcIsString(args->bc))
    {
	dynstring name = bcExecuteIntoDS(args->bc, 0);

	envAddBinding(globalEffects, &name, 0);
    }

    for (i = 0; i < numArgs - 1; ++i)
	envAddBindings(globalEffects, (environment*)listNThElementData(globalEffectList, i));
}

environment*
builtInDefineEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    environment *env;

    if (numArgs == 0)
	return parentEnv;

    env = envNew(parentEnv);
    while (args != 0)
    {
	if (bcIsString(args->bc))
	{
	    dynstring name = bcExecuteIntoDS(args->bc, 0);

	    if (numArgs > 1 && args->next == 0)
	    {
		char *pos = strchr(name.data, ':');

		if (pos != 0)
		    dsShrinkRear(&name, name.length - (pos - name.data));
	    }

	    if (envGetBinding(env, &name) == 0)
		envAddBinding(env, &name, 0);
	}
	args = args->next;
    }

    return env;
}

void
builtInLocals (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    environment *newEnv;
    int i;

    if (!(numArgs >= 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "locals");
	return;
    }

    newEnv = envNew(env);

    for (i = 0; i < numArgs - 1; ++i)
    {
	dynstring var;

	var = bcExecuteIntoDS(args[i].bytecode, env);
	envAddBinding(newEnv, &var, valueNewScalarFromCString(""));
    }

    bcExecute(args[numArgs - 1].bytecode, newEnv, ow);
}

environment*
builtInLocalsEnvironmentor (int numArgs, bcArgument *args, environment *parentEnv)
{
    environment *env;

    if (numArgs == 0)
	return parentEnv;

    env = envNew(parentEnv);
    while (args != 0)
    {
	if (bcIsString(args->bc))
	{
	    dynstring name = bcExecuteIntoDS(args->bc, 0);

	    if (envGetBinding(env, &name) == 0)
		envAddBinding(env, &name, 0);
	}
	args = args->next;
    }

    return env;
}

void
builtInApply (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "apply");
	return;
    }

    if (args[0].value.value->type != VALUE_BUILT_IN && args[0].value.value->type != VALUE_LAMBDA)
    {
	issueError(ERRMAC_VALUE_WRONG_TYPE, cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_LAMBDA));
	return;
    }

    if (args[1].value.value->type != VALUE_LIST)
    {
	issueError(ERRMAC_VALUE_WRONG_TYPE, cStringForValueType(args[1].value.value->type),
		   cStringForValueType(VALUE_LIST));
	return;
    }

    assureArgumentIsCopy(&args[1]);

    bcExecuteMacro(args[0].value.value, args[1].value.value, env, ow);
}

void
builtInBound (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "bound");
	return;
    }

    transformArgumentToScalar(&args[0]);
    if (envGetBinding(env, &args[0].value.value->v.scalar.scalar) != 0)
    {
	OUT_CHAR(ow, '1');
    }
    else
    {
	OUT_CHAR(ow, '0');
    }
}

void
builtInVoid (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "void");
	return;
    }
}

void
builtInOutputenable (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "outputenable");
	return;
    }

    toplevelOutputWriter.enabled = valueBoolValue(args[0].value.value);
}

void
builtInWarning (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "warning");
	return;
    }

    transformArgumentToScalar(&args[0]);
    issueWarning(WARNMAC_USER_WARNING, args[0].value.value->v.scalar.scalar.data);
}

void
builtInError (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "error");
	return;
    }

    transformArgumentToScalar(&args[0]);
    issueError(ERRMAC_USER_ERROR, args[0].value.value->v.scalar.scalar.data);
}

void
builtInRandom (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int randomNumber;
    char randString[64];

    if (!(numArgs == 0 || numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "random");
	return;
    }

    randomNumber = random();

    if (numArgs == 1)
    {
	transformArgumentToScalar(&args[0]);
	randomNumber %= atoi(args[0].value.value->v.scalar.scalar.data);
    }

    OUT_STRING(ow, randString, sprintf(randString, "%d", randomNumber));
}

void
builtInDepend (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1 || numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "depend");
	return;
    }

    if (generateDependencies)
    {
	transformArgumentToScalar(&args[0]);
	if (numArgs == 1)
	    addDependency(0, &args[0].value.value->v.scalar.scalar);
	else
	{
	    transformArgumentToScalar(&args[1]);
	    addDependency(&args[1].value.value->v.scalar.scalar,
			  &args[0].value.value->v.scalar.scalar);
	}
    }
}

void
registerBuiltIn (const char *name, builtIn func, int evalParams,
		 builtInGlobalEffector globalEffector, builtInEnvironmentor environmentor)
{
    dynstring nameString = dsNewFrom(name);

    envAddBinding(0, &nameString, valueNewBuiltIn(func, evalParams,
						  globalEffector, environmentor));
}

void
registerBuiltIns (void)
{
    registerBuiltIn("lambda", builtInLambda, 0, 0, builtInLambdaEnvironmentor);
    registerBuiltIn("locals", builtInLocals, 0, 0, builtInLocalsEnvironmentor);
    registerBuiltIn("define", builtInDefine, 0,
		    builtInDefineGlobalEffector, builtInDefineEnvironmentor);
    registerBuiltIn("apply", builtInApply, 1, 0, 0);
    registerBuiltIn("bound", builtInBound, 1, 0, 0);
    registerBuiltIn("void", builtInVoid, 1, 0, 0);
    registerBuiltIn("outputenable", builtInOutputenable, 1, 0, 0);
    registerBuiltIn("warning", builtInWarning, 1, 0, 0);
    registerBuiltIn("error", builtInError, 1, 0, 0);
    registerBuiltIn("random", builtInRandom, 1, 0, 0);
    registerBuiltIn("depend", builtInDepend, 1, 0, 0);

    registerValues();
    registerStringOps();
    registerFlowCtl();
    registerFileOps();
    registerArrayOps();
    registerHashOps();
    registerWWWBuiltIns();
    registerDatabaseBuiltIns();
}
