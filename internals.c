/* -*- c -*- */

/*
 * internals.c
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

#include "dynstring.h"
#include "error.h"
#include "main.h"
#include "bytecode.h"
#include "value.h"
#include "environment.h"
#include "builtins/builtins.h"
#include "output.h"

extern int generateDependencies;
extern dynstring mainFileName;

char metaChar = '%';
char quoteChar = '\\';

extern char filler_cmdChar;

extern char **environ;

void
internalSetMetaChar (value *val, int needCopy, bcSubscript *subscripts, environment *env)
{
    assert(subscripts == 0);
    assert(val->type == VALUE_SCALAR && val->v.scalar.scalar.length == 1);

    metaChar = val->v.scalar.scalar.data[0];
}

void
internalGetMetaChar (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    OUT_CHAR(ow, metaChar);
}

void
internalSetCommandChar (value *val, int needCopy, bcSubscript *subscripts, environment *env)
{
    assert(subscripts == 0);
    assert(val->type == VALUE_SCALAR && val->v.scalar.scalar.length == 1);

    filler_cmdChar = val->v.scalar.scalar.data[0];
}

void
internalGetCommandChar (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    OUT_CHAR(ow, filler_cmdChar);
}

void
internalSetQuoteChar (value *val, int needCopy, bcSubscript *subscripts, environment *env)
{
    assert(subscripts == 0);
    assert(val->type == VALUE_SCALAR && val->v.scalar.scalar.length == 1);

    quoteChar = val->v.scalar.scalar.data[0];
}

void
internalGetQuoteChar (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    OUT_CHAR(ow, quoteChar);
}

void
internalSetOutputenabled (value *val, int needCopy, bcSubscript *subscripts, environment *env)
{
    assert(subscripts == 0);

    toplevelOutputWriter.enabled = valueBoolValue(val);
}

void
internalGetOutputenabled (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    if (toplevelOutputWriter.enabled)
    {
	OUT_CHAR(ow, '1');
    }
    else
    {
	OUT_CHAR(ow, '0');
    }
}

void
internalGetDependencing (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    if (generateDependencies)
    {
	OUT_CHAR(ow, '1');
    }
    else
    {
	OUT_CHAR(ow, '0');
    }
}

void
internalGetMainfilename (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    assert(subscripts == 0);

    OUT_STRING(ow, mainFileName.data, mainFileName.length);
}

void
internalGetEnv (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    if (subscripts == 0)
    {
	int i;
	value *theHash = valueNewHash();

	for (i = 0; environ[i] != 0; ++i)
	{
	    char *pos = strchr(environ[i], '=');
	    dynstring name = dsNewFromBytes(environ[i], pos - environ[i]);

	    valueHashDefine(theHash, &name, valueNewScalarFromCString(pos + 1));
	}

	OUT_VALUE_REF(ow, theHash);
    }
    else
    {
	dynstring index;
	char *result;

	assert(subscripts->type == BYTECODE_SUBSCRIPT_HASH
	       && subscripts->next == 0);

	index = bcExecuteIntoDS(subscripts->bc, env);

	result = getenv(index.data);

	if (result == 0)
	    issueError(ERRMAC_KEY_NOT_IN_HASH, index.data);
	else
	    OUT_VALUE_REF(ow, valueNewScalarFromCString(result));
    }
}

void
internalGetMan (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    static char *manString =
	"  O  \n"
	"  |  \n"
	" --- \n"
	"/ | \\\n"
	"  |  \n"
	" /'\\ \n"
	"/   \\\n";

    OUT_STRING(ow, manString, strlen(manString));
}

void
internalGetWoman (bcSubscript *subscripts, environment *env, outputWriter *ow)
{
    static char *womanString =
	"  O  \n"
	"  |  \n"
	" --- \n"
	"/o|o\\\n"
	"  |  \n"
	" / \\ \n"
	"/   \\\n";

    OUT_STRING(ow, womanString, strlen(womanString));
}

void
registerInternal (const char *name, internalSet setFunc, internalGet getFunc)
{
    dynstring nameString = dsNewFrom(name);

    envAddBinding(0, &nameString, valueNewInternal(setFunc, getFunc));
}

void
registerInternals (void)
{
    registerInternal("metachar", internalSetMetaChar, internalGetMetaChar);
    /* registerInternal("commandchar", internalSetCommandChar, internalGetCommandChar); */
    registerInternal("quotechar", internalSetQuoteChar, internalGetQuoteChar);
    registerInternal("outputenabled", internalSetOutputenabled, internalGetOutputenabled);
    registerInternal("dependencing", 0, internalGetDependencing);
    registerInternal("mainfilename", 0, internalGetMainfilename);
    registerInternal("env", 0, internalGetEnv);
    registerInternal("man", 0, internalGetMan);
    registerInternal("woman", 0, internalGetWoman);
}
