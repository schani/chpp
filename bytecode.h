/* -*- c -*- */

/*
 * bytecode.h
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

#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "environment.h"
#include "output.h"
#include "value.h"

#define BYTECODE_STRING           1
#define BYTECODE_QUOTED_STRING    2
#define BYTECODE_EVAL             3
#define BYTECODE_ARITH            4
#define BYTECODE_ASSIGNMENT       5
#define BYTECODE_MACRO            6

#define BYTECODE_MACRO_SUBVALUE   1
#define BYTECODE_MACRO_BYREF      2

#define BYTECODE_SUBSCRIPT_LIST   1
#define BYTECODE_SUBSCRIPT_HASH   2

struct _bytecode;

typedef struct _bcSubscript
{
    int type;
    struct _bytecode *bc;

    struct _bcSubscript *next;
} bcSubscript;

typedef struct _bcArgument
{
    struct _bytecode *bc;

    struct _bcArgument *next;
} bcArgument;

typedef struct _bytecode
{
    int type;

    union
    {
	struct
	{
	    dynstring string;
	} string;

	struct
	{
	    struct _bytecode *bc;
	} eval;

	struct
	{
	    struct _bytecode *bc;
	} arith;

	struct
	{
	    int modify;
	    struct _bytecode *varName;
	    bcSubscript *subscript;
	    struct _bytecode *newValue;
	} assignment;

	struct
	{
	    int flags;
	    struct _bytecode *nameOrValue;
	    bcSubscript *subscript;
	    int numArgs;
	    bcArgument *arguments;
	} macro;
    } v;

    struct _bytecode *next;
} bytecode;

bytecode* bcNewString (dynstring *str);
bytecode* bcNewQuotedString (dynstring *str);
bytecode* bcNewEmpty (void);
bytecode* bcNewAssignment (int modify, bytecode *varName, bcSubscript *subscript,
			   bytecode *newValue);
bytecode* bcNewMacro (int flags, bytecode *nameOrValue, bcSubscript *subscript,
		      int numArgs, bcArgument *args);
bytecode* bcNewArithmetic (bytecode *bc);
bytecode* bcNewEvaluation (bytecode *bc);

bytecode* bcRemoveOuterWS (bytecode *bc);

int bcIsString (bytecode *bc);

void bcExecute (bytecode *bc, environment *env, outputWriter *ow);

value* bcExecuteIntoValue (bytecode *bc, environment *env, int *needCopy);
value* bcExecuteIntoCopiedValue (bytecode *bc, environment *env);
dynstring bcExecuteIntoDS (bytecode *bc, environment *env);

void bcExecuteMacro (value *macro, value *argList, environment *env, outputWriter *ow);
value* bcExecuteMacroIntoValue (value *macro, value *argList, environment *env, int *needCopy);
value* bcExecuteMacroIntoCopiedValue (value *macro, value *argList, environment *env);
dynstring bcExecuteMacroIntoDS (value *macro, value *argList, environment *env);

#endif
