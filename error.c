/* -*- c -*- */

/*
 * error.c
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

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include "dynstring.h"
#include "main.h"
#include "error.h"

#include "error.h"

char *frontendWarnings[] = 
    {
    };

char *frontendErrors[] =
    {
	"only one output file is allowed",
	"could not open output file '%s': %s",
	"could not open input file '%s': %s"
    };

char *commandWarnings[] =
    {
	"command %s should not have arguments",
	"unknown command %s"
    };

char *commandErrors[] =
    {
	"unmatched %s",
	"could not open include file: %s",
	"expected ')' .. found '\\n' or '\\0'",
	"%s expects only one argument",
	"%s"
    };

char *macroWarnings[] =
    {
	"%s",
	"key '%s' already defined in hash",
	"possibly unbound variable '%s'",
	"possibly wrong number (%d) of arguments for macro %s",
	"suspect character '%c'"
    };

char *macroErrors[] =
    {
	"symbol '%s' not defined",
	"unmatched '<'",
	"division by zero",
	"malformed arithmetic expression '%s'",
	"cannot set internal variable '%s'",
	"invalid regular expression '%s'",
	"invalid argument to macro '%s': '%s'",
	"call to '%s' failed: '%s'",
	"'%s' is an internal, not a hash",
	"key '%s' is not contained in hash",
	"cannot redefine built-in '%s'",
	"%s",
	"increment in for-loop cannot be zero",
	"feature not yet implemented: %s",
	"list index %d out of bounds",
	"'%s' is an internal, not an array",
	"'%s' is of type %s instead of expected type %s",
	"value is of type %s instead of expected type %s",
	"value is not an lvalue",
	"wrong number of arguments for macro '%s'",
	"user-defined macro called with %d instead of expected %d arguments",
	"unbound variable '%s'",
	"cannot get internal variable '%s'",
	"list indexed as hash with index '%s'",
	"unexpected character '%c': expected '%c'",
	"premature end of file or string"
    };

int errorsOccured = 0,
    warningsOccured = 0;

static char printfString[1024];

void
issueWarningInLine (const char *filename, int lineNumber, int num, ...)
{
    va_list ap;
    char *warningString = 0;

    assert(num < 3000);

    if (num < 1000)
	warningString = frontendWarnings[num];
    else if (num < 2000)
	warningString = commandWarnings[num - 1000];
    else if (num < 3000)
	warningString = macroWarnings[num - 2000];

    if (num < 1000)
	sprintf(printfString, "%s: warning: %s\n", executableName, warningString);
    else
	sprintf(printfString, "%s:%d: warning: %s\n", filename, lineNumber,
		warningString);
    va_start(ap, num);
    vfprintf(stderr, printfString, ap);
    va_end(ap);

    ++warningsOccured;
}

void
issueErrorInLine (const char *filename, int lineNumber, int num, ...)
{
    va_list ap;
    char *errorString = 0;

    assert(num < 3000);

    if (num < 1000)
	errorString = frontendErrors[num];
    else if (num < 2000)
	errorString = commandErrors[num - 1000];
    else if (num < 3000)
	errorString = macroErrors[num - 2000];

    if (num < 1000)
	sprintf(printfString, "%s: %s\n", executableName, errorString);
    else
	sprintf(printfString, "%s:%d: %s\n", filename, lineNumber, errorString);
    va_start(ap, num);
    vfprintf(stderr, printfString, ap);
    va_end(ap);

    ++errorsOccured;
}

void
issueWarning (int num, ...)
{
    va_list ap;
    char *warningString = 0;
    const char *filename = irPreprocessorFileName(&toplevelInputReader);
    int lineNumber = irPreprocessorLineNumber(&toplevelInputReader);

    assert(num < 3000);

    if (num < 1000)
	warningString = frontendWarnings[num];
    else if (num < 2000)
	warningString = commandWarnings[num - 1000];
    else if (num < 3000)
	warningString = macroWarnings[num - 2000];

    if (num < 1000)
	sprintf(printfString, "%s: warning: %s\n", executableName, warningString);
    else
	sprintf(printfString, "%s:%d: warning: %s\n", filename, lineNumber,
		warningString);
    va_start(ap, num);
    vfprintf(stderr, printfString, ap);
    va_end(ap);

    ++warningsOccured;
}

void
issueError (int num, ...)
{
    va_list ap;
    char *errorString = 0;
    const char *filename = irPreprocessorFileName(&toplevelInputReader);
    int lineNumber = irPreprocessorLineNumber(&toplevelInputReader);

    assert(num < 3000);

    if (num < 1000)
	errorString = frontendErrors[num];
    else if (num < 2000)
	errorString = commandErrors[num - 1000];
    else if (num < 3000)
	errorString = macroErrors[num - 2000];

    if (num < 1000)
	sprintf(printfString, "%s: %s\n", executableName, errorString);
    else
	sprintf(printfString, "%s:%d: %s\n", filename, lineNumber, errorString);
    va_start(ap, num);
    vfprintf(stderr, printfString, ap);
    va_end(ap);

    ++errorsOccured;
}
