/* -*- c -*- */

/*
 * output.c
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

#include "memory.h"

#include "output.h"

extern int generateDependencies;

void
dummyOutChar (void *state, char c)
{
}

void
dummyOutString (void *state, const char *str, int length)
{
}

void
dummyOutValue (void *state, value *theValue, int byRef)
{
}

void
dynstringOutChar (void *state, char c)
{
    dsAppendChar((dynstring*)state, c);
}

void
dynstringOutString (void *state, const char *str, int length)
{
    dsAppendString((dynstring*)state, str, length);
}

void
dynstringOutValue (void *state, value *theValue, int byRef)
{
    if (theValue->type == VALUE_SCALAR)
	dsAppendString((dynstring*)state,
		       theValue->v.scalar.scalar.data, theValue->v.scalar.scalar.length);
    else
    {
	value *scalarValue = valueNewScalarFromValue(theValue);

	dsAppendString((dynstring*)state,
		       scalarValue->v.scalar.scalar.data, scalarValue->v.scalar.scalar.length);
    }
}

void
fileOutChar (void *state, char c)
{
    if (!generateDependencies)
	putc(c, (FILE*)state);
}

void
fileOutString (void *state, const char *str, int length)
{
    if (!generateDependencies)
	fwrite(str, length, 1, (FILE*)state);
}

void
fileOutValue (void *state, value *theValue, int byRef)
{
    if (!generateDependencies)
    {
	if (theValue->type == VALUE_SCALAR)
	    fwrite(theValue->v.scalar.scalar.data, theValue->v.scalar.scalar.length,
		   1, (FILE*)state);
	else
	{
	    value *scalarValue = valueNewScalarFromValue(theValue);
	    
	    fwrite(scalarValue->v.scalar.scalar.data, scalarValue->v.scalar.scalar.length,
		   1, (FILE*)state);
	}
    }
}

void
valueOutChar (void *state, char c)
{
    outputStateValue *theState = (outputStateValue*)state;

    if (theState->theValue == 0)
    {
	char string[2] = { c, '\0' };

	theState->theValue = valueNewScalarFromCString(string);
    }
    else
    {
	if (theState->needCopy)
	    theState->theValue = valueCopy(theState->theValue);
	valueTransformToScalar(theState->theValue);
	dsAppendChar(&theState->theValue->v.scalar.scalar, c);
    }
    theState->needCopy = 0;
}

void
valueOutString (void *state, const char *str, int length)
{
    outputStateValue *theState = (outputStateValue*)state;

    if (length > 0)
    {
	if (theState->theValue == 0)
	    theState->theValue = valueNewScalarFromBytes(str, length);
	else
	{
	    if (theState->needCopy)
		theState->theValue = valueCopy(theState->theValue);
	    valueTransformToScalar(theState->theValue);
	    dsAppendString(&theState->theValue->v.scalar.scalar, str, length);
	}
	theState->needCopy = 0;
    }
}

void
valueOutValue (void *state, value *appendedValue, int byRef)
{
    outputStateValue *theState = (outputStateValue*)state;
    value *theValue = theState->theValue;

    if (theState->theValue == 0)
    {
	if (theState->byReference)
	{
	    theState->theValue = appendedValue;
	    theState->needCopy = !byRef;
	}
	else
	    theState->theValue = valueCopy(appendedValue);
    }
    else
    {
	if (theState->needCopy)
	    theState->theValue = valueCopy(theState->theValue);

	if (theState->theValue->type == VALUE_LIST
	    && appendedValue->type == VALUE_LIST)
	    valueListAppendList(theValue, appendedValue, 0);
	else
	{
	    valueTransformToScalar(theState->theValue);

	    if (appendedValue->type != VALUE_SCALAR)
		appendedValue = valueNewScalarFromValue(appendedValue);

	    dsAppendString(&theState->theValue->v.scalar.scalar,
			   appendedValue->v.scalar.scalar.data,
			   appendedValue->v.scalar.scalar.length);
	}

	theState->needCopy = 0;
    }
}

outputWriter
owNewDummy (void)
{
    outputWriter ow;

    ow.outChar = dummyOutChar;
    ow.outString = dummyOutString;
    ow.outValue = dummyOutValue;
    ow.enabled = 1;
    ow.state = 0;

    return ow;
}

outputWriter
owNewDynstring (dynstring *ds)
{
    outputWriter ow;

    ow.outChar = dynstringOutChar;
    ow.outString = dynstringOutString;
    ow.outValue = dynstringOutValue;
    ow.enabled = 1;
    ow.state = ds;

    return ow;
}

outputWriter
owNewFile (FILE *file)
{
    outputWriter ow;

    ow.outChar = fileOutChar;
    ow.outString = fileOutString;
    ow.outValue = fileOutValue;
    ow.enabled = 1;
    ow.state = file;

    return ow;
}

outputWriter
owNewValue (int byReference)
{
    outputWriter ow;
    outputStateValue *state = memXAlloc(sizeof(outputStateValue));

    ow.outChar = valueOutChar;
    ow.outString = valueOutString;
    ow.outValue = valueOutValue;
    ow.enabled = 1;
    ow.state = state;
    state->theValue = 0;
    state->byReference = byReference;
    state->needCopy = 0;

    return ow;
}

value*
owValueValue (outputWriter *ow)
{
    outputStateValue *state = (outputStateValue*)ow->state;

    if (state->theValue == 0)
	return valueNewUndefined();
    return state->theValue;
}

int
owValueNeedCopy (outputWriter *ow)
{
    outputStateValue *state = (outputStateValue*)ow->state;

    return state->needCopy;
}
