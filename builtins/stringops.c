/* -*- c -*- */

/*
 * builtins/stringops.c
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
#include <ctype.h>
#include <string.h>

#include "../regex.h"
#include "../error.h"
#include "../environment.h"
#include "../bytecode.h"
#include "../output.h"

#include "builtins.h"

#define CHPP_SYNTAX_BITS    (RE_BACKSLASH_ESCAPE_IN_LISTS | RE_CHAR_CLASSES | RE_INTERVALS \
                           | RE_NEWLINE_ALT | RE_NO_BK_BRACES | RE_NO_BK_PARENS \
                           | RE_NO_BK_VBAR | RE_UNMATCHED_RIGHT_PAREN_ORD)

static char caseFoldTable[256];

static value*
listFromRegisters (char *string,
		   struct re_pattern_buffer *patternBuffer, struct re_registers *registers)
{
    value *theList = valueNewList();
    int i;

    for (i = 0; i <= (int)(patternBuffer->re_nsub); ++i)
    {
	dynstring regContent;

	if (registers->start[i] >= 0)
	    regContent =
		dsNewFromBytes(string + registers->start[i],
			       registers->end[i] - registers->start[i]);
	else
	    regContent = dsNew();

	valueListSetElement(theList, i, valueNewScalar(&regContent));
    }

    return theList;
}

void
builtInMatch (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    const char *error;
    struct re_pattern_buffer patternBuffer;
    struct re_registers registers;
    int result;
    char resultString[64];

    if (!(numArgs == 2 || numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "smatch");
	return;
    }

    if (numArgs == 3)
    {
	assureArgumentIsCopy(&args[2]);
	if (args[2].value.value->type != VALUE_LIST)
	{
	    valueTransformToScalar(args[2].value.value);
	    issueError(ERRMAC_INVALID_MACRO_ARG, "smatch",
		       args[2].value.value->v.scalar.scalar.data);
	    return;
	}
    }

    patternBuffer.translate = 0;
    patternBuffer.fastmap = 0;
    patternBuffer.buffer = 0;
    patternBuffer.allocated = 0;
    patternBuffer.regs_allocated = REGS_UNALLOCATED;

    transformArgumentToScalar(&args[0]);
    error = re_compile_pattern(args[0].value.value->v.scalar.scalar.data,
			       args[0].value.value->v.scalar.scalar.length,
			       &patternBuffer);
    if (error != 0)
    {
	issueError(ERRMAC_INVALID_REGEX, error);
	return;
    }

    transformArgumentToScalar(&args[1]);
    result = re_search(&patternBuffer,
		       args[1].value.value->v.scalar.scalar.data,
		       args[1].value.value->v.scalar.scalar.length, 0,
		       args[1].value.value->v.scalar.scalar.length,
		       &registers);

    if (numArgs == 3)
    {
	if (result >= 0)
	    valueAssign(args[2].value.value,
			listFromRegisters(args[1].value.value->v.scalar.scalar.data,
					  &patternBuffer, &registers), 1);
	else
	    valueListClear(args[2].value.value);
    }

    if (patternBuffer.regs_allocated != REGS_UNALLOCATED)
    {
	free(registers.start);
	free(registers.end);
    }
    regfree(&patternBuffer);

    OUT_STRING(ow, resultString, sprintf(resultString, "%d", result));
}

void
builtInSplit (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct re_pattern_buffer patternBuffer;
    struct re_registers registers;
    const char *error;
    int pos,
	length,
	matchPos,
	elements = 0;
    value *result = valueNewList(),
	*lambdaArgs,
	*frontRegs,
	*rearRegs;

    if (!(numArgs == 2 || numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "ssplit");
	return;
    }

    if (numArgs == 3 && args[2].value.value->type != VALUE_LAMBDA)
    {
	transformArgumentToScalar(&args[2]);
	issueError(ERRMAC_INVALID_MACRO_ARG, "ssplit",
		   args[2].value.value->v.scalar.scalar.data);
	return;
    }

    transformArgumentToScalar(&args[0]);
    transformArgumentToScalar(&args[1]);

    if (numArgs == 3)
    {
	lambdaArgs = valueNewList();
	frontRegs = valueNewList();
    }

    patternBuffer.translate = 0;
    patternBuffer.fastmap = 0;
    patternBuffer.buffer = 0;
    patternBuffer.allocated = 0;

    error = re_compile_pattern(args[0].value.value->v.scalar.scalar.data,
			       args[0].value.value->v.scalar.scalar.length, &patternBuffer);
    if (error != 0)
    {
	issueError(ERRMAC_INVALID_REGEX, error);
	return;
    }

    pos = 0;
    length = args[1].value.value->v.scalar.scalar.length;

    do
    {
	dynstring ds;
	value *string,
	    *inserted;
	int elementLength;

	matchPos = re_search(&patternBuffer, args[1].value.value->v.scalar.scalar.data + pos,
			     length, 0, length, &registers);
	if (matchPos >= 0)
	{
	    elementLength = matchPos;
	    if (numArgs == 3)
		rearRegs = listFromRegisters(args[1].value.value->v.scalar.scalar.data + pos,
					     &patternBuffer, &registers);
	}
	else
	{
	    elementLength = length;
	    if (numArgs == 3)
		rearRegs = valueNewList();
	}

	ds = dsNewFromBytes(args[1].value.value->v.scalar.scalar.data + pos, elementLength);
	string = valueNewScalar(&ds);

	if (numArgs == 3)
	{
	    valueListSetElement(lambdaArgs, 0, frontRegs);
	    valueListSetElement(lambdaArgs, 1, string);
	    valueListSetElement(lambdaArgs, 2, rearRegs);
	    inserted = bcExecuteMacroIntoCopiedValue(args[2].value.value, lambdaArgs, env);
	}
	else
	    inserted = string;

	valueListSetElement(result, elements++, inserted);

	pos += elementLength + registers.end[0] - registers.start[0];
	length -= elementLength + registers.end[0] - registers.start[0];

	if (numArgs == 3)
	    frontRegs = rearRegs;
    } while (matchPos >= 0);

    if (patternBuffer.regs_allocated != REGS_UNALLOCATED)
    {
	free(registers.start);
	free(registers.end);
    }
    regfree(&patternBuffer);

    OUT_VALUE(ow, result);
}

void
builtInStokenize (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct re_pattern_buffer patternBuffer;
    struct re_registers registers;
    const char *error;
    int pos,
	length,
	matchPos,
	elements = 0;
    value *result = valueNewList(),
	*lambdaArgs;

    if (!(numArgs == 2 || numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "stokenize");
	return;
    }

    if (numArgs == 3 && args[2].value.value->type != VALUE_LAMBDA)
    {
	transformArgumentToScalar(&args[2]);
	issueError(ERRMAC_INVALID_MACRO_ARG, "stokenize",
		   args[2].value.value->v.scalar.scalar.data);
	return;
    }

    transformArgumentToScalar(&args[0]);
    transformArgumentToScalar(&args[1]);

    if (numArgs == 3)
	lambdaArgs = valueNewList();

    patternBuffer.translate = 0;
    patternBuffer.fastmap = 0;
    patternBuffer.buffer = 0;
    patternBuffer.allocated = 0;

    error = re_compile_pattern(args[0].value.value->v.scalar.scalar.data,
			       args[0].value.value->v.scalar.scalar.length, &patternBuffer);
    if (error != 0)
    {
	issueError(ERRMAC_INVALID_REGEX, error);
	return;
    }

    pos = 0;
    length = args[1].value.value->v.scalar.scalar.length;

    do
    {
	dynstring ds;
	value *inserted;

	matchPos = re_search(&patternBuffer, args[1].value.value->v.scalar.scalar.data + pos,
			     length, 0, length, &registers);
	if (matchPos >= 0)
	{
	    if (numArgs == 3)
	    {
		valueListSetElement(lambdaArgs, 0,
				    listFromRegisters(args[1].value.value->v.scalar.scalar.data
						      + pos, &patternBuffer, &registers));
		inserted = bcExecuteMacroIntoCopiedValue(args[2].value.value, lambdaArgs, env);
	    }
	    else
	    {
		ds = dsNewFromBytes(args[1].value.value->v.scalar.scalar.data + pos + matchPos,
				    registers.end[0] - registers.start[0]);
		inserted = valueNewScalar(&ds);
	    }

	    valueListSetElement(result, elements++, inserted);
	}

	pos += matchPos + registers.end[0] - registers.start[0];
	length -= matchPos + registers.end[0] - registers.start[0];
    } while (matchPos >= 0);

    if (patternBuffer.regs_allocated != REGS_UNALLOCATED)
    {
	free(registers.start);
	free(registers.end);
    }
    regfree(&patternBuffer);

    OUT_VALUE(ow, result);
}

void
builtInGsub (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct re_pattern_buffer patternBuffer;
    struct re_registers registers;
    const char *error;
    int start = 0,
	result;
    value *lambda;

    if (!(numArgs == 3 || numArgs == 4))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "sgsub");
	return;
    }

    patternBuffer.translate = 0;
    if (numArgs == 4)
    {
	transformArgumentToScalar(&args[3]);
	if (strchr(args[3].value.value->v.scalar.scalar.data, 'i') != 0)
	    patternBuffer.translate = caseFoldTable;
    }

    patternBuffer.fastmap = 0;
    patternBuffer.buffer = 0;
    patternBuffer.allocated = 0;
    patternBuffer.regs_allocated = REGS_UNALLOCATED;

    transformArgumentToScalar(&args[0]);
    error = re_compile_pattern(args[0].value.value->v.scalar.scalar.data,
			       args[0].value.value->v.scalar.scalar.length,
			       &patternBuffer);
    if (error != 0)
    {
	issueError(ERRMAC_INVALID_REGEX, error);
	return;
    }

    if (args[2].value.value->type == VALUE_LAMBDA || args[2].value.value->type == VALUE_BUILT_IN)
	lambda = args[2].value.value;
    else
    {
	lambda = 0;
	transformArgumentToScalar(&args[2]);
    }

    transformArgumentToScalar(&args[1]);

    do
    {
	result = re_search(&patternBuffer,
			   args[1].value.value->v.scalar.scalar.data,
			   args[1].value.value->v.scalar.scalar.length,
			   start, args[1].value.value->v.scalar.scalar.length - start,
			   &registers);

	if (result >= 0)
	{
	    OUT_STRING(ow, args[1].value.value->v.scalar.scalar.data + start, result - start);
	    if (lambda != 0)
	    {
		value *list = valueNewList(),
		    *lambdaArgs = valueNewList();
		int i;
		dynstring resultString;

		valueListSetElement(lambdaArgs, 0, list);

		for (i = 0; i <= patternBuffer.re_nsub; ++i)
		{
		    dynstring regContent;

		    if (registers.start[i] >= 0)
			regContent =
			    dsNewFromBytes(args[1].value.value->v.scalar.scalar.data
					   + registers.start[i],
					   registers.end[i] - registers.start[i]);
		    else
			regContent = dsNew();

		    valueListSetElement(list, i, valueNewScalar(&regContent));
		}

		valueListSetElement(lambdaArgs, 0, list);

		resultString = bcExecuteMacroIntoDS(lambda, lambdaArgs, env);
		OUT_STRING(ow, resultString.data, resultString.length);
	    }
	    else
	    {
		OUT_STRING(ow, args[2].value.value->v.scalar.scalar.data,
			   args[2].value.value->v.scalar.scalar.length);
	    }
	    start = registers.end[0];
	}
    } while (result >= 0);

    OUT_STRING(ow, args[1].value.value->v.scalar.scalar.data + start,
	       args[1].value.value->v.scalar.scalar.length - start);

    if (patternBuffer.regs_allocated != REGS_UNALLOCATED)
    {
	free(registers.start);
	free(registers.end);
    }
    patternBuffer.translate = 0;
    regfree(&patternBuffer);
}

void
builtInRemovews (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "sremovews");
	return;
    }

    dsRemoveOuterWS(&transformArgumentToScalar(&args[0])->v.scalar.scalar);

    OUT_STRING(ow, args[0].value.value->v.scalar.scalar.data,
	       args[0].value.value->v.scalar.scalar.length);
}

void
builtInStrlength (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char lengthString[64];

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "slength");
	return;
    }

    OUT_STRING(ow, lengthString,
	       sprintf(lengthString, "%d",
		       transformArgumentToScalar(&args[0])->v.scalar.scalar.length));
}

void
builtInSubstring (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring *string;
    int start,
	length;

    if (!(numArgs == 2 || numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "ssub");
	return;
    }

    string = &transformArgumentToScalar(&args[0])->v.scalar.scalar;

    if (numArgs == 2)
    {
	start = atoi(transformArgumentToScalar(&args[1])->v.scalar.scalar.data);

	if (start >= 0)
	{
	    if (start > string->length)
		start = string->length;
	    length = string->length - start;
	}
	else
	{
	    length = -start;
	    start = string->length + start;
	}
    }
    else
    {
	start = atoi(transformArgumentToScalar(&args[1])->v.scalar.scalar.data);
	length = atoi(transformArgumentToScalar(&args[2])->v.scalar.scalar.data);

	if (start < 0)
	    start = 0;
	else if (start > string->length)
	    start = string->length;

	if (length < 0)
	    length = -length - start;

	if (start + length > string->length)
	    length = string->length - start;

	if (length < 0)
	    length = 0;
    }

    OUT_STRING(ow, string->data + start, length);
}

void
builtInStrcmp (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char resultString[64];

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "scmp");
	return;
    }

    OUT_STRING(ow, resultString,
	       sprintf(resultString, "%d",
		       strcmp(transformArgumentToScalar(&args[0])->v.scalar.scalar.data,
			      transformArgumentToScalar(&args[1])->v.scalar.scalar.data)));
}

void
builtInChr (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char c;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "schr");
	return;
    }

    c = atoi(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);
    OUT_CHAR(ow, c);
}

void
builtInSrange (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    if (!(numArgs % 2 == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "srange");
	return;
    }

    for (i = 0; i < numArgs; i += 2)
    {
	int c1,
	    c2;

	transformArgumentToScalar(&args[i]);
	transformArgumentToScalar(&args[i + 1]);

	if (args[i].value.value->v.scalar.scalar.length != 1)
	{
	    issueError(ERRMAC_INVALID_MACRO_ARG, "srange",
		       args[i].value.value->v.scalar.scalar.data);
	    continue;
	}
	if (args[i + 1].value.value->v.scalar.scalar.length != 1)
	{
	    issueError(ERRMAC_INVALID_MACRO_ARG, "srange",
		       args[i + 1].value.value->v.scalar.scalar.data);
	    continue;
	}
	
	c1 = args[i].value.value->v.scalar.scalar.data[0];
	c2 = args[i + 1].value.value->v.scalar.scalar.data[0];

	for (; c1 <= c2; ++c1)
	    OUT_CHAR(ow, c1);
    }
}

void
builtInSmap (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char map[256];
    int i;

    if (!(numArgs == 3))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "smap");
	return;
    }

    transformArgumentToScalar(&args[0]);
    transformArgumentToScalar(&args[1]);
    transformArgumentToScalar(&args[2]);

    if (args[0].value.value->v.scalar.scalar.length !=
	args[1].value.value->v.scalar.scalar.length)
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "smap",
		   args[1].value.value->v.scalar.scalar.data);
	return;
    }

    for (i = 0; i < 256; ++i)
	map[i] = i;
    for (i = 0; i < args[0].value.value->v.scalar.scalar.length; ++i)
	map[args[0].value.value->v.scalar.scalar.data[i]] =
	    args[1].value.value->v.scalar.scalar.data[i];

    for (i = 0; i < args[2].value.value->v.scalar.scalar.length; ++i)
	OUT_CHAR(ow, map[args[2].value.value->v.scalar.scalar.data[i]]);
}

void
builtInNumber (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    long number,
	base;
    char numberString[32];
    int index = 0,
	isNegative = 0;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "snumber");
	return;
    }

    transformArgumentToScalar(&args[0]);
    transformArgumentToScalar(&args[1]);

    number = atol(args[0].value.value->v.scalar.scalar.data);
    base = atol(args[1].value.value->v.scalar.scalar.data);

    assert(base > 1 && base <= 36);

    if (number < 0)
    {
	isNegative = 1;
	number = -number;
    }

    do
    {
	long digit = number % base;

	if (digit < 10)
	    numberString[index++] = '0' + digit;
	else
	    numberString[index++] = 'a' + digit - 10;
	
	number = number / base;
    } while (number != 0);

    if (isNegative)
	OUT_CHAR(ow, '-');
    for (--index; index >= 0; --index)
	OUT_CHAR(ow, numberString[index]);
}

void
builtInHexencode (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "shexencode");
	return;
    }

    transformArgumentToScalar(&args[0]);

    for (i = 0; i < args[0].value.value->v.scalar.scalar.length; ++i)
    {
	int c = args[0].value.value->v.scalar.scalar.data[i];

	OUT_CHAR(ow, "0123456789ABCDEF"[(c >> 4) & 0xf]);
	OUT_CHAR(ow, "0123456789ABCDEF"[c & 0xf]);
    }
}

void
builtInHexdecode (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int i;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "shexdecode");
	return;
    }

    transformArgumentToScalar(&args[0]);

    if (args[0].value.value->v.scalar.scalar.length % 2 != 0)
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "shexdecode",
		   args[0].value.value->v.scalar.scalar.data);
	return;
    }

    for (i = 0; i < args[0].value.value->v.scalar.scalar.length; )
    {
	int c,
	    digit = args[0].value.value->v.scalar.scalar.data[i++];

	if (digit >= '0' && digit <= '9')
	    digit = digit - '0';
	else if (digit >= 'A' && digit <= 'F')
	    digit = digit - 'A' + 10;
	else
	{
	    issueError(ERRMAC_INVALID_MACRO_ARG, "shexdecode",
		       args[0].value.value->v.scalar.scalar.data);
	    return;
	}

	c = digit << 4;

	digit = args[0].value.value->v.scalar.scalar.data[i++];

	if (digit >= '0' && digit <= '9')
	    digit = digit - '0';
	else if (digit >= 'A' && digit <= 'F')
	    digit = digit - 'A' + 10;
	else
	{
	    issueError(ERRMAC_INVALID_MACRO_ARG, "shexdecode",
		       args[0].value.value->v.scalar.scalar.data);
	    return;
	}

	c |= digit;

	OUT_CHAR(ow, c);
    }
}

void
registerStringOps (void)
{
    int i;

    for (i = 0; i < 256; ++i)
	caseFoldTable[i] = toupper(i);

    registerBuiltIn("smatch", builtInMatch, 1, 0, 0);
    registerBuiltIn("ssplit", builtInSplit, 1, 0, 0);
    registerBuiltIn("stokenize", builtInStokenize, 1, 0, 0);
    registerBuiltIn("sgsub", builtInGsub, 1, 0, 0);
    registerBuiltIn("sremovews", builtInRemovews, 1, 0, 0);
    registerBuiltIn("slength", builtInStrlength, 1, 0, 0);
    registerBuiltIn("ssub", builtInSubstring, 1, 0, 0);
    registerBuiltIn("scmp", builtInStrcmp, 1, 0, 0);
    registerBuiltIn("schr", builtInChr, 1, 0, 0);
    registerBuiltIn("srange", builtInSrange, 1, 0, 0);
    registerBuiltIn("smap", builtInSmap, 1, 0, 0);
    registerBuiltIn("snumber", builtInNumber, 1, 0, 0);
    registerBuiltIn("shexencode", builtInHexencode, 1, 0, 0);
    registerBuiltIn("shexdecode", builtInHexdecode, 1, 0, 0);

    re_syntax_options = CHPP_SYNTAX_BITS;
}
