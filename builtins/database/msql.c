/* -*- c -*- */

/*
 * builtins/database/msql.c
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

#include "../../config.h"

#ifdef HAVE_LIBMSQL

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <msql.h>

#include "../../error.h"
#include "../../environment.h"
#include "../../value.h"
#include "../../output.h"
#include "../builtins.h"

void
builtIn_sqlMsqlConnect (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int socket;
    char socketString[64];
    value *result;

    if (!(numArgs == 4))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlConnect");
	return;
    }

    transformArgumentToScalar(&args[0]);

    if (strcmp(args[0].value.value->v.scalar.scalar.data, "localhost") == 0)
	socket = msqlConnect(0);
    else
	socket = msqlConnect(args[0].value.value->v.scalar.scalar.data);

    if (socket == -1)
    {
	OUT_CHAR(ow, '0');
	return;
    }

    transformArgumentToScalar(&args[2]);

    if (msqlSelectDB(socket, args[2].value.value->v.scalar.scalar.data) == -1)
    {
	msqlClose(socket);
	OUT_CHAR(ow, '0');
	return;
    }

    result = valueNewList();
    sprintf(socketString, "%d", socket);
    valueListSetElement(result, 0, valueNewScalarFromCString(socketString));
    OUT_VALUE_REF(ow, result);
}

void
builtIn_sqlMsqlClose (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *aValue;
    int socket;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlClose");
	return;
    }

    assert(args[0].value.value->type == VALUE_LIST);
    aValue = valueListGetElement(args[0].value.value, 0);
    assert(aValue->type == VALUE_SCALAR);
    socket = atoi(aValue->v.scalar.scalar.data);

    msqlClose(socket);
}

void
builtIn_sqlMsqlDatabaseInfo (int numArgs, macroArgument *args,
			     environment *env, outputWriter *ow)
{
    value *hash = valueNewHash();
    dynstring key;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlDatabaseInfo");
	return;
    }

    key = dsNewFrom("timeformat");
    valueHashDefine(hash, &key, valueNewScalarFromCString("$H:$M:$S"));

    key = dsNewFrom("dateformat");
    valueHashDefine(hash, &key, valueNewScalarFromCString("$d-$b-$Y"));

    key = dsNewFrom("datetimeformat");
    valueHashDefine(hash, &key, valueNewScalarFromCString(""));

    OUT_VALUE(ow, hash);
}

void
builtIn_sqlMsqlQuery (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *aValue,
	*resultHash,
	*rowList,
	*colInfoHash,
	*columnNameList;
    int socket;
    m_result *queryResult;
    m_field *field;
    m_row row;
    dynstring ds;
    int i,
	numColumns = 0;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlQuery");
	return;
    }

    assert(args[0].value.value->type == VALUE_LIST);
    aValue = valueListGetElement(args[0].value.value, 0);
    assert(aValue->type == VALUE_SCALAR);
    socket = atoi(aValue->v.scalar.scalar.data);

    transformArgumentToScalar(&args[1]);

    if (msqlQuery(socket, args[1].value.value->v.scalar.scalar.data) == -1)
    {
	OUT_VALUE_REF(ow, valueNewHash());
	return;
    }

    queryResult = msqlStoreResult();
    assert(queryResult != 0);

    resultHash = valueNewHash();
    rowList = valueNewList();
    ds = dsNewFrom("rows"); valueHashDefine(resultHash, &ds, rowList);
    colInfoHash = valueNewHash();
    ds = dsNewFrom("colinfo"); valueHashDefine(resultHash, &ds, colInfoHash);
    columnNameList = valueNewList();
    ds = dsNewFrom("colnames"); valueHashDefine(resultHash, &ds, columnNameList);

    for (field = msqlFetchField(queryResult); field != 0; field = msqlFetchField(queryResult))
    {
	static char *types[] = { "illegal", "int", "char", "float", "int", "null",
				 "text", "date", "int", "float", "time" };

	value *infoHash = valueNewHash();
	dynstring ds2;

	ds = dsNewFrom(field->name);
	valueListSetElement(columnNameList, numColumns++, valueNewScalar(&ds));

	ds2 = dsNewFrom("type");
	valueHashDefine(infoHash, &ds2, valueNewScalarFromCString(types[field->type]));

	valueHashDefine(colInfoHash, &ds, infoHash);
    }

    for (row = msqlFetchRow(queryResult), i = 0; row != 0; row = msqlFetchRow(queryResult))
    {
	int j;
	value *hash = valueNewHash();

	for (j = 0; j < numColumns; ++j)
	{
	    if (row[j] == 0)
		ds = dsEmpty();
	    else
		ds = dsNewFrom(row[j]);
	    valueHashDefine(hash, &valueListGetElement(columnNameList, j)->v.scalar.scalar,
			    valueNewScalar(&ds));
	}

	valueListSetElement(rowList, i++, hash);
    }

    msqlFreeResult(queryResult);

    OUT_VALUE_REF(ow, resultHash);
}

void
builtIn_sqlMsqlUpdate (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char numberString[64];
    value *aValue;
    int socket,
	result;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlUpdate");
	return;
    }

    assert(args[0].value.value->type == VALUE_LIST);
    aValue = valueListGetElement(args[0].value.value, 0);
    assert(aValue->type == VALUE_SCALAR);
    socket = atoi(aValue->v.scalar.scalar.data);

    transformArgumentToScalar(&args[1]);

    result = msqlQuery(socket, args[1].value.value->v.scalar.scalar.data);

    OUT_STRING(ow, numberString, sprintf(numberString, "%d", result));
}

void
builtIn_sqlMsqlError (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMsqlError");
	return;
    }

    OUT_STRING(ow, msqlErrMsg, strlen(msqlErrMsg));
}

void
registerDatabaseMsqlBuiltIns (void)
{
    registerBuiltIn("_sqlMsqlConnect", builtIn_sqlMsqlConnect, 1, 0, 0);
    registerBuiltIn("_sqlMsqlClose", builtIn_sqlMsqlClose, 1, 0, 0);
    registerBuiltIn("_sqlMsqlDatabaseInfo", builtIn_sqlMsqlDatabaseInfo, 1, 0, 0);
    registerBuiltIn("_sqlMsqlQuery", builtIn_sqlMsqlQuery, 1, 0, 0);
    registerBuiltIn("_sqlMsqlUpdate", builtIn_sqlMsqlUpdate, 1, 0, 0);
    registerBuiltIn("_sqlMsqlError", builtIn_sqlMsqlError, 1, 0, 0);
}

#endif
