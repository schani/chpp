/* -*- c -*- */

/*
 * bcoutput.c
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

#include "memory.h"

#include "bcoutput.h"

void
bcwBytecodeOutChar (void *theState, char c)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    dynstring empty = dsEmpty();

    if (state->last == 0)
	state->first = state->last = bcNewString(&empty);
    else if (state->last->type != BYTECODE_STRING && state->last->type != BYTECODE_QUOTED_STRING)
	state->last = state->last->next = bcNewString(&empty);

    dsAppendChar(&state->last->v.string.string, c);
}

void
bcwBytecodeOutString (void *theState, const char *string, int length)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    dynstring empty = dsEmpty();

    if (state->last == 0)
	state->first = state->last = bcNewString(&empty);
    else if (state->last->type != BYTECODE_STRING && state->last->type != BYTECODE_QUOTED_STRING)
	state->last = state->last->next = bcNewString(&empty);

    dsAppendString(&state->last->v.string.string, string, length);
}

void
bcwBytecodeOutAssignment (void *theState, int modify, bytecode *varName, bcSubscript *subscript,
			  bytecode *newValue)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    bytecode *bc = bcNewAssignment(modify, varName, subscript, newValue);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwBytecodeOutMacro (void *theState, int flags, bytecode *nameOrValue, bcSubscript *subscript,
		     int numArgs, bcArgument *args)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    bytecode *bc = bcNewMacro(flags, nameOrValue, subscript, numArgs, args);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwBytecodeOutBeginQuote (void *theState)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    dynstring empty = dsEmpty();
    bytecode *bc = bcNewQuotedString(&empty);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwBytecodeOutEndQuote (void *theState)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    dynstring empty = dsEmpty();
    bytecode *bc = bcNewString(&empty);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwBytecodeOutArithmetic (void *theState, bytecode *arg)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    bytecode *bc = bcNewArithmetic(arg);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwBytecodeOutEvaluation (void *theState, bytecode *arg)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)theState;
    bytecode *bc = bcNewEvaluation(arg);

    if (state->last == 0)
	state->first = state->last = bc;
    else
	state->last = state->last->next = bc;
}

void
bcwOutputOutChar (void *theState, char c)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    OUT_CHAR(state->ow, c);
}

void
bcwOutputOutString (void *theState, const char *string, int length)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    OUT_STRING(state->ow, string, length);
}

void
bcwOutputOutAssignment (void *theState, int modify, bytecode *varName, bcSubscript *subscript,
			bytecode *newValue)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    bcExecute(bcNewAssignment(modify, varName, subscript, newValue), state->env, state->ow);
}

void
bcwOutputOutMacro (void *theState, int flags, bytecode *nameOrValue, bcSubscript *subscript,
		   int numArgs, bcArgument *args)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    bcExecute(bcNewMacro(flags, nameOrValue, subscript, numArgs, args), state->env, state->ow);
}

void
bcwOutputOutBeginQuote (void *theState)
{
}

void
bcwOutputOutEndQuote (void *theState)
{
}

void
bcwOutputOutArithmetic (void *theState, bytecode *bc)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    bcExecute(bcNewArithmetic(bc), state->env, state->ow);
}

void
bcwOutputOutEvaluation (void *theState, bytecode *bc)
{
    bytecodeStateOutput *state = (bytecodeStateOutput*)theState;

    bcExecute(bcNewEvaluation(bc), state->env, state->ow);
}

bytecodeWriter*
bcwNewBytecode (void)
{
    bytecodeWriter *bcw = (bytecodeWriter*)memXAlloc(sizeof(bytecodeWriter));
    bytecodeStateBytecode *state =
	(bytecodeStateBytecode*)memXAlloc(sizeof(bytecodeStateBytecode));

    state->first = state->last = 0;

    bcw->state = state;
    bcw->outChar = bcwBytecodeOutChar;
    bcw->outString = bcwBytecodeOutString;
    bcw->outAssignment = bcwBytecodeOutAssignment;
    bcw->outMacro = bcwBytecodeOutMacro;
    bcw->outBeginQuote = bcwBytecodeOutBeginQuote;
    bcw->outEndQuote = bcwBytecodeOutEndQuote;
    bcw->outArithmetic = bcwBytecodeOutArithmetic;
    bcw->outEvaluation = bcwBytecodeOutEvaluation;

    return bcw;
}

bytecode*
bcwBytecodeBytecode (bytecodeWriter *bcw)
{
    bytecodeStateBytecode *state = (bytecodeStateBytecode*)bcw->state;

    return state->first;
}

bytecodeWriter*
bcwNewOutput (environment *env, outputWriter *ow)
{
    bytecodeWriter *bcw = (bytecodeWriter*)memXAlloc(sizeof(bytecodeWriter));
    bytecodeStateOutput *state =
	(bytecodeStateOutput*)memXAlloc(sizeof(bytecodeStateOutput));

    state->env = env;
    state->ow = ow;

    bcw->state = state;
    bcw->outChar = bcwOutputOutChar;
    bcw->outString = bcwOutputOutString;
    bcw->outAssignment = bcwOutputOutAssignment;
    bcw->outMacro = bcwOutputOutMacro;
    bcw->outBeginQuote = bcwOutputOutBeginQuote;
    bcw->outEndQuote = bcwOutputOutEndQuote;
    bcw->outArithmetic = bcwOutputOutArithmetic;
    bcw->outEvaluation = bcwOutputOutEvaluation;

    return bcw;
}
