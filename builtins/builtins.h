/* -*- c -*- */

/*
 * builtins/builtins.h
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

#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include "../list.h"

struct _value;
struct _outputWriter;
struct _dynstring;
struct _bytecode;
struct _environment;
struct _bcArgument;

#define BYTECODE_EMPTY(bc)  ((bc) == 0 || ((bc)->type == BYTECODE_STRING && (bc)->v.string.string.length == 0))

typedef union
{
    struct
    {
	struct _value *value;
	int needCopy;
    } value;

    struct _bytecode *bytecode;
} macroArgument;

void removeWSAndEvalValue (struct _value *val, struct _outputWriter *ow);
struct _value* removeWSAndEvalValueIntoValue (struct _value *val);
struct _dynstring removeWSAndEvalValueIntoDS (struct _value *val);
struct _value* transformArgumentToScalar (macroArgument *arg);
void assureArgumentIsCopy (macroArgument *arg);

void registerBuiltIns (void);

void registerFileOps (void);
void registerStringOps (void);
void registerFlowCtl (void);
void registerArrayOps (void);
void registerHashOps (void);
void registerValues (void);
void registerWWWBuiltIns (void);
void registerDatabaseBuiltIns (void);

typedef void (*builtIn) (int numArgs, macroArgument *args, struct _environment *env,
			 struct _outputWriter *ow);
typedef void (*builtInGlobalEffector) (int numArgs, struct _bcArgument *args,
				       list *globalEffectList,
				       struct _environment *globalEffects);
typedef struct _environment* (*builtInEnvironmentor) (int numArgs, struct _bcArgument *args,
						      struct _environment *parentEnv);

void registerBuiltIn (const char *name, builtIn func, int evalParams,
		      builtInGlobalEffector globalEffector, builtInEnvironmentor environmentor);

#endif
