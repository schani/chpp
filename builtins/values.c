/* -*- c -*- */

/*
 * builtins/value.c
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
#include <string.h>

#include "../value.h"
#include "../output.h"
#include "../error.h"
#include "../internals.h"
#include "../environment.h"
#include "../bytecode.h"

#include "builtins.h"

void
builtInList (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *theList;
    int i;

    theList = valueNewList();

    for (i = 0; i < numArgs; ++i)
    {
	assureArgumentIsCopy(&args[i]);
	valueListSetElement(theList, i, args[i].value.value);
    }

    OUT_VALUE_REF(ow, theList);
}

void
builtInHash (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *theHash;
    int i;

    if (!(numArgs == 1 || numArgs % 2 == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "hash");
	return;
    }

    if (numArgs == 1)
    {
	if (transformArgumentToScalar(&args[0])->v.scalar.scalar.length > 0)
	{
	    issueError(ERRMAC_WRONG_NUM_ARGS, "hash");
	    return;
	}
    }

    theHash = valueNewHash();

    for (i = 0; i < numArgs / 2; ++i)
    {
	transformArgumentToScalar(&args[i * 2]);
	if (valueHashLookup(theHash, &args[i * 2].value.value->v.scalar.scalar) != 0)
	    issueWarning(WARNMAC_DUPLICATE_HASH_KEY,
			 args[i * 2].value.value->v.scalar.scalar.data);
	assureArgumentIsCopy(&args[i * 2 + 1]);
	valueHashDefine(theHash,
			&args[i * 2].value.value->v.scalar.scalar,
			args[i * 2 + 1].value.value);
    }

    OUT_VALUE_REF(ow, theHash);
}

void
builtInSame (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "same");
	return;
    }

    if (args[0].value.needCopy || args[1].value.needCopy)
    {
	OUT_CHAR(ow, '0');
    }
    else
    {
	if (valueAreSame(args[0].value.value, args[1].value.value))
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
builtInEqual (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "equal");
	return;
    }

    if (valueAreEqual(args[0].value.value, args[1].value.value))
    {
	OUT_CHAR(ow, '1');
    }
    else
    {
	OUT_CHAR(ow, '0');
    }
}

void
builtInTypeof (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    const char *type;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "typeof");
	return;
    }

    type = cStringForValueType(args[0].value.value->type);

    OUT_STRING(ow, type, strlen(type));
}

void
encodeDynstring (dynstring *ds, outputWriter *ow)
{
    int i;

    OUT_CHAR(ow, metaChar);
    OUT_CHAR(ow, '\'');
    for (i = 0; i < ds->length; ++i)
    {
	char c = ds->data[i];

	if (c == '\'' || c == quoteChar)
	    OUT_CHAR(ow, quoteChar);
	OUT_CHAR(ow, c);
    }
    OUT_CHAR(ow, '\'');
}

void encodeBytecode (bytecode *bc, outputWriter *ow);

void
encodeSubscripts (bcSubscript *subscript, outputWriter *ow)
{
    while (subscript != 0)
    {
	if (subscript->type == BYTECODE_SUBSCRIPT_LIST)
	{
	    OUT_CHAR(ow, '[');
	}
	else
	{
	    OUT_CHAR(ow, '{');
	}
	encodeBytecode(subscript->bc, ow);
	if (subscript->type == BYTECODE_SUBSCRIPT_LIST)
	{
	    OUT_CHAR(ow, ']');
	}
	else
	{
	    OUT_CHAR(ow, '}');
	}

	subscript = subscript->next;
    }
}

void
encodeBytecode (bytecode *bc, outputWriter *ow)
{
    while (bc != 0)
    {
	switch (bc->type)
	{
	    case BYTECODE_STRING :
	    case BYTECODE_QUOTED_STRING :
		encodeDynstring(&bc->v.string.string, ow);
		break;

	    case BYTECODE_EVAL :
		OUT_CHAR(ow, metaChar);
		OUT_CHAR(ow, '{');
		encodeBytecode(bc->v.eval.bc, ow);
		OUT_CHAR(ow, '}');
		break;

	    case BYTECODE_ARITH :
		OUT_CHAR(ow, metaChar);
		OUT_CHAR(ow, '[');
		encodeBytecode(bc->v.arith.bc, ow);
		OUT_CHAR(ow, ']');
		break;

	    case BYTECODE_ASSIGNMENT :
		{
		    bcSubscript *subscript;

		    OUT_CHAR(ow, metaChar);
		    OUT_CHAR(ow, '<');
		    if (bc->v.assignment.modify)
			OUT_CHAR(ow, '&');
		    encodeBytecode(bc->v.assignment.varName, ow);
		    encodeSubscripts(bc->v.assignment.subscript, ow);
		    OUT_CHAR(ow, '=');
		    encodeBytecode(bc->v.assignment.newValue, ow);
		    OUT_CHAR(ow, '>');
		}
		break;

	    case BYTECODE_MACRO :
		{
		    bcSubscript *subscript;

		    OUT_CHAR(ow, metaChar);
		    OUT_CHAR(ow, '<');
		    if (bc->v.macro.flags & BYTECODE_MACRO_BYREF)
			OUT_CHAR(ow, '&');
		    if (bc->v.macro.flags & BYTECODE_MACRO_SUBVALUE)
			OUT_CHAR(ow, '(');
		    encodeBytecode(bc->v.macro.nameOrValue, ow);
		    if (bc->v.macro.flags & BYTECODE_MACRO_SUBVALUE)
			OUT_CHAR(ow, ')');
		    encodeSubscripts(bc->v.macro.subscript, ow);
		    if (bc->v.macro.numArgs >= 0)
		    {
			bcArgument *argument;

			OUT_CHAR(ow, '(');
			for (argument = bc->v.macro.arguments; argument != 0; argument = argument->next)
			{
			    encodeBytecode(argument->bc, ow);
			    if (argument->next != 0)
			    {
				OUT_CHAR(ow, ',');
			    }
			}
			OUT_CHAR(ow, ')');
		    }
		    OUT_CHAR(ow, '>');
		}
		break;
	}

	bc = bc->next;
    }
}

void
encodeValue (value *theValue, outputWriter *ow)
{
    assert(theValue != 0);

    switch (theValue->type)
    {
	case VALUE_UNDEFINED :
	    return;

	case VALUE_INTERNAL :
	    OUT_STRING(ow, "<internal>", 10);
	    break;

	case VALUE_BUILT_IN :
	    OUT_STRING(ow, "<builtIn>", 9);
	    break;

	case VALUE_SCALAR :
	    encodeDynstring(&theValue->v.scalar.scalar, ow);
	    break;

	case VALUE_LIST :
	    {
		int i;

		OUT_CHAR(ow, metaChar);
		OUT_STRING(ow, "list(", 5);
		if (valueListLength(theValue) > 0)
		{
		    encodeValue(valueListGetElement(theValue, 0), ow);
		    for (i = 1; i < valueListLength(theValue); ++i)
		    {
			OUT_CHAR(ow, ',');
			encodeValue(valueListGetElement(theValue, i), ow);
		    }
		}
		OUT_CHAR(ow, ')');
	    }
	    break;

	case VALUE_HASH :
	    {
		value *aValue,
		    *keyValue;
		char *aKey;
		hstate state;

		OUT_CHAR(ow, metaChar);
		OUT_STRING(ow, "hash(", 5);
		state = hash_state(theValue->v.hash.hash);
		if ((aValue = (value*)hash_next(&state, &aKey)) != 0)
		{
		    keyValue = valueNewScalarFromCString(aKey);
		    encodeValue(keyValue, ow);
		    OUT_CHAR(ow, ',');
		    encodeValue(aValue, ow);
		    while ((aValue = (value*)hash_next(&state, &aKey)) != 0)
		    {
			OUT_CHAR(ow, ',');
			keyValue = valueNewScalarFromCString(aKey);
			encodeValue(keyValue, ow);
			OUT_CHAR(ow, ',');
			encodeValue(aValue, ow);
		    }
		}
		OUT_CHAR(ow, ')');
	    }
	    break;

	case VALUE_LAMBDA :
	    {
		int i;

		OUT_CHAR(ow, metaChar);
		OUT_STRING(ow, "lambda(", 7);
		for (i = 0; i < theValue->v.lambda.numParams; ++i)
		{
		    encodeDynstring(&theValue->v.lambda.paramNames[i], ow);
		    OUT_CHAR(ow, ',');
		}
		encodeBytecode(theValue->v.lambda.code, ow);
		OUT_CHAR(ow, ')');
	    }
	    break;

	case VALUE_ENVIRONMENT :
	    OUT_STRING(ow, "<environment>", 13);
	    break;

	case VALUE_BYTECODE :
	    OUT_STRING(ow, "<bytecode>", 10);
	    break;

	case VALUE_WHATSIT :
	    OUT_STRING(ow, "<whatsit>", 9);
	    break;

	default :
	    assert(0);
    }
}

void
builtInEncode (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "encode");
	return;
    }

    encodeValue(args[0].value.value, ow);
}

void
registerValues (void)
{
    registerBuiltIn("list", builtInList, 1, 0, 0);
    registerBuiltIn("hash", builtInHash, 1, 0, 0);
    registerBuiltIn("same", builtInSame, 1, 0, 0);
    registerBuiltIn("equal", builtInEqual, 1, 0, 0);
    registerBuiltIn("typeof", builtInTypeof, 1, 0, 0);
    registerBuiltIn("encode", builtInEncode, 1, 0, 0);
}
