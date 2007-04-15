/* -*- c -*- */

/*
 * builtins/time.c
 *
 * chpp
 *
 * Copyright (C) 1997-2007 Mark Probst
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
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "../error.h"
#include "../output.h"
#include "../bytecode.h"
#include "../environment.h"

#include "builtins.h"

void
builtInTimeUNIX (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct timeval tv;
    char buf[32];
    int length;

    if (!(numArgs == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "timeUNIX");
	return;
    }

    assert(gettimeofday(&tv, 0) == 0);

    length = sprintf(buf, "%ld", tv.tv_sec);

    OUT_STRING(ow, buf, length);
}

void
builtInTimeUNIX2Hash (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct tm *time;
    dynstring key;
    char buf[32];
    value *hash;
    time_t unix_time;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "timeUNIX2Hash");
	return;
    }

    hash = valueNewHash();

    unix_time = atol(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);
    time = localtime(&unix_time);

    sprintf(buf, "%d", time->tm_sec);
    valueHashDefineCString(hash, "second", valueNewScalarFromCString(buf));

    sprintf(buf, "%d", time->tm_min);
    valueHashDefineCString(hash, "minute", valueNewScalarFromCString(buf));

    sprintf(buf, "%d", time->tm_hour);
    valueHashDefineCString(hash, "hour", valueNewScalarFromCString(buf));

    sprintf(buf, "%d", time->tm_mday);
    valueHashDefineCString(hash, "day", valueNewScalarFromCString(buf));

    sprintf(buf, "%d", time->tm_mon + 1);
    valueHashDefineCString(hash, "month", valueNewScalarFromCString(buf));

    sprintf(buf, "%d", time->tm_year + 1900);
    valueHashDefineCString(hash, "year", valueNewScalarFromCString(buf));

    OUT_VALUE_REF(ow, hash);
}

void
builtInTimeHash2UNIX (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *hash;
    struct tm time;
    char buf[32];
    int length;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "timeHash2UNIX");
	return;
    }

    hash = args[0].value.value;

    if (hash->type != VALUE_HASH)
    {
	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(hash->type),
		   cStringForValueType(VALUE_HASH));
	return;
    }

    time.tm_sec = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "second"))->v.scalar.scalar.data);
    time.tm_min = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "minute"))->v.scalar.scalar.data);
    time.tm_hour = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "hour"))->v.scalar.scalar.data);
    time.tm_mday = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "day"))->v.scalar.scalar.data);
    time.tm_mon = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "month"))->v.scalar.scalar.data) - 1;
    time.tm_year = atoi(valueNewScalarFromValue(valueHashLookupCString(hash, "year"))->v.scalar.scalar.data) - 1900;
    time.tm_isdst = -1;

    length = sprintf(buf, "%ld", mktime(&time));

    OUT_STRING(ow, buf, length);
}

void
registerTimeOps (void)
{
    registerBuiltIn("timeUNIX", builtInTimeUNIX, 1, 0, 0);
    registerBuiltIn("timeUNIX2Hash", builtInTimeUNIX2Hash, 1, 0, 0);
    registerBuiltIn("timeHash2UNIX", builtInTimeHash2UNIX, 1, 0, 0);
}
