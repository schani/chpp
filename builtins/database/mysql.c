/* -*- c -*- */

/*
 * builtins/database/mysql.c
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

#ifdef HAVE_LIBMYSQLCLIENT

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <mysql/mysql.h>

#include "../../memory.h"
#include "../../error.h"
#include "../../environment.h"
#include "../../value.h"
#include "../../output.h"
#include "../builtins.h"

static const char*
typeNameForMysqlType (int type)
{
    switch (type)
    {
	case FIELD_TYPE_DECIMAL : return "decimal";
	case FIELD_TYPE_TINY : return "tinyint";
	case FIELD_TYPE_SHORT : return "smallint";
	case FIELD_TYPE_LONG : return "long";
	case FIELD_TYPE_FLOAT : return "float";
	case FIELD_TYPE_DOUBLE : return "double";
	case FIELD_TYPE_NULL : return "null";
	case FIELD_TYPE_TIMESTAMP : return "timestamp";
	case FIELD_TYPE_LONGLONG : return "bigint";
	case FIELD_TYPE_INT24 : return "mediumint";
	case FIELD_TYPE_DATE : return "date";
	case FIELD_TYPE_TIME : return "time";
	case FIELD_TYPE_DATETIME : return "datetime";
	case FIELD_TYPE_ENUM : return "enum";
	case FIELD_TYPE_SET : return "set";
	case FIELD_TYPE_TINY_BLOB : return "tinyblob";
	case FIELD_TYPE_MEDIUM_BLOB : return "mediumblob";
	case FIELD_TYPE_LONG_BLOB : return "longblob";
	case FIELD_TYPE_BLOB : return "blob";
	case FIELD_TYPE_VAR_STRING : return "varchar";
	case FIELD_TYPE_STRING : return "char";
    }

    return 0;
}

void
builtIn_sqlMysqlConnect (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *user,
	*password;
    MYSQL *mysql;

    if (!(numArgs == 4))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlConnect");
	return;
    }

    mysql = (MYSQL*)memXAlloc(sizeof(MYSQL));

    transformArgumentToScalar(&args[0]);
    transformArgumentToScalar(&args[2]);

    if (args[3].value.value->type != VALUE_HASH)
    {

	issueError(ERRMAC_VALUE_WRONG_TYPE,
		   cStringForValueType(args[0].value.value->type),
		   cStringForValueType(VALUE_HASH));
	OUT_CHAR(ow, '0');
	return;
    }

    user = valueHashLookupCString(args[3].value.value, "user");
    if (user == 0)
	user = valueNewScalarFromCString("");
    else
	user = valueNewScalarFromValue(user);

    password = valueHashLookupCString(args[3].value.value, "password");
    if (password == 0)
	password = valueNewScalarFromCString("");
    else
	password = valueNewScalarFromValue(password);

    if (mysql_connect(mysql,
		      args[0].value.value->v.scalar.scalar.data,
		      user->v.scalar.scalar.data,
		      password->v.scalar.scalar.data) == 0)
    {
	OUT_CHAR(ow, '0');
	return;
    }

    if (mysql_select_db(mysql, args[2].value.value->v.scalar.scalar.data) == -1)
    {
	mysql_close(mysql);
	OUT_CHAR(ow, '0');
	return;
    }

    OUT_VALUE_REF(ow, valueNewWhatsit(mysql));
}

void
builtIn_sqlMysqlClose (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    MYSQL *mysql;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlClose");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    mysql = (MYSQL*)args[0].value.value->v.whatsit.data;

    mysql_close(mysql);
}

void
builtIn_sqlMysqlDatabaseInfo (int numArgs, macroArgument *args,
			      environment *env, outputWriter *ow)
{
    value *hash = valueNewHash();
    dynstring key;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlDatabaseInfo");
	return;
    }

    valueHashDefineCString(hash, "timeformat", valueNewScalarFromCString("$H:$M:$S"));
    valueHashDefineCString(hash, "dateformat", valueNewScalarFromCString("$Y-$m-$d"));
    valueHashDefineCString(hash, "datetimeformat", valueNewScalarFromCString("$Y-$m-$d $H:$M:$S"));

    OUT_VALUE(ow, hash);
}

void
builtIn_sqlMysqlQuery (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    value *resultHash,
	*rowList,
	*colInfoHash,
	*columnNameList;
    dynstring ds;
    int i,
	numColumns = 0;
    MYSQL *mysql;
    MYSQL_RES *queryResult;
    MYSQL_FIELD *field;
    MYSQL_ROW row;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlQuery");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    mysql = (MYSQL*)args[0].value.value->v.whatsit.data;

    transformArgumentToScalar(&args[1]);

    if (mysql_query(mysql, args[1].value.value->v.scalar.scalar.data) == -1)
    {
	OUT_VALUE_REF(ow, valueNewHash());
	return;
    }

    queryResult = mysql_use_result(mysql);
    assert(queryResult != 0);

    resultHash = valueNewHash();
    rowList = valueNewList();
    ds = dsNewFrom("rows"); valueHashDefine(resultHash, &ds, rowList);
    colInfoHash = valueNewHash();
    ds = dsNewFrom("colinfo"); valueHashDefine(resultHash, &ds, colInfoHash);
    columnNameList = valueNewList();
    ds = dsNewFrom("colnames"); valueHashDefine(resultHash, &ds, columnNameList);

    for (field = mysql_fetch_field(queryResult);
	 field != 0; 
	 field = mysql_fetch_field(queryResult))
    {
	value *infoHash = valueNewHash();
	dynstring ds2;

	ds = dsNewFrom(field->name);
	valueListSetElement(columnNameList, numColumns++, valueNewScalar(&ds));

	ds2 = dsNewFrom("type");
	valueHashDefine(infoHash, &ds2,
			valueNewScalarFromCString(typeNameForMysqlType(field->type)));

	valueHashDefine(colInfoHash, &ds, infoHash);
    }

    for (row = mysql_fetch_row(queryResult), i = 0; row != 0; row = mysql_fetch_row(queryResult))
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

    mysql_free_result(queryResult);

    OUT_VALUE_REF(ow, resultHash);
}

void
builtIn_sqlMysqlUpdate (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char numberString[64];
    MYSQL *mysql;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlUpdate");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    mysql = (MYSQL*)args[0].value.value->v.whatsit.data;

    transformArgumentToScalar(&args[1]);

    if (mysql_query(mysql, args[1].value.value->v.scalar.scalar.data) == -1)
    {
	OUT_CHAR(ow, '0');
	return;
    }

    OUT_STRING(ow, numberString, sprintf(numberString, "%ld", mysql_affected_rows(mysql)));
}

void
builtIn_sqlMysqlError (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    const char *errorMsg;
    MYSQL *mysql;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlMysqlError");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    mysql = (MYSQL*)args[0].value.value->v.whatsit.data;

    errorMsg = mysql_error(mysql);
    OUT_STRING(ow, errorMsg, strlen(errorMsg));
}

void
registerDatabaseMysqlBuiltIns (void)
{
    registerBuiltIn("_sqlMysqlConnect", builtIn_sqlMysqlConnect, 1, 0, 0);
    registerBuiltIn("_sqlMysqlClose", builtIn_sqlMysqlClose, 1, 0, 0);
    registerBuiltIn("_sqlMysqlDatabaseInfo", builtIn_sqlMysqlDatabaseInfo, 1, 0, 0);
    registerBuiltIn("_sqlMysqlQuery", builtIn_sqlMysqlQuery, 1, 0, 0);
    registerBuiltIn("_sqlMysqlUpdate", builtIn_sqlMysqlUpdate, 1, 0, 0);
    registerBuiltIn("_sqlMysqlError", builtIn_sqlMysqlError, 1, 0, 0);
}

#endif
