/* -*- c -*- */

/*
 * builtins/database/adabas.c
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

#define export
#define FAR
#define CALLBACK
#define EXPORT
typedef void* HWND;

#include <assert.h>

#include <sql.h>
#include <sqlext.h>

#include "../../error.h"
#include "../../environment.h"
#include "../../value.h"
#include "../../output.h"
#include "../../memory.h"
#include "../builtins.h"

typedef struct
{
    HENV henv;
    HDBC hdbc;
    HSTMT hstmt;
} odbcWhatsit;

typedef struct
{
    SDWORD displaySize;
    SDWORD type;
} columnInfo;

static const char*
typeNameForSQLType (SDWORD type)
{
    switch (type)
    {
	
    }
}

void
builtIn_sqlAdabasConnect (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    HENV henv;
    HDBC hdbc;
    RETCODE rc;
    dynstring ds;
    value *user,
	*password;

    if (!(numArgs == 4))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlAdabasConnect");
	return;
    }

    if (args[3].value.value->type != VALUE_HASH)
    {
	transformArgumentToScalar(&args[3]);
	issueError(ERRMAC_INVALID_MACRO_ARG, "_sqlAdabasConnect",
		   args[3].value.value->v.scalar.scalar.data);
	return;
    }

    transformArgumentToScalar(&args[2]);

    ds = dsNewFrom("user");
    user = valueHashLookup(args[3].value.value, &ds);
    if (user->type != VALUE_SCALAR)
	user = valueNewScalarFromValue(user);

    ds = dsNewFrom("password");
    password = valueHashLookup(args[3].value.value, &ds);
    if (password->type != VALUE_SCALAR)
	password = valueNewScalarFromValue(password);


    SQLAllocEnv(&henv);
    SQLAllocConnect(henv, &hdbc);
    rc = SQLConnect(hdbc, args[2].value.value->v.scalar.scalar.data, SQL_NTS,
		    user->v.scalar.scalar.data, SQL_NTS,
		    password->v.scalar.scalar.data, SQL_NTS);

    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
	odbcWhatsit *whatsit = (odbcWhatsit*)memXAlloc(sizeof(odbcWhatsit));

	whatsit->henv = henv;
	whatsit->hdbc = hdbc;
	SQLAllocStmt(whatsit->hdbc, &whatsit->hstmt);

	OUT_VALUE_REF(ow, valueNewWhatsit(whatsit));
    }
    else
    {
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);

	OUT_CHAR(ow, '0');
    }
}

void
builtIn_sqlAdabasClose (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    odbcWhatsit *whatsit;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlAdabasClose");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    whatsit = (odbcWhatsit*)valueWhatsitData(args[0].value.value);

    SQLFreeStmt(whatsit->hstmt, SQL_DROP);
    SQLDisconnect(whatsit->hdbc);
    SQLFreeConnect(whatsit->hdbc);
    SQLFreeEnv(whatsit->henv);
}

void
builtIn_sqlAdabasQuery (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    odbcWhatsit *whatsit;
    RETCODE rc;
    SWORD numCols;
    int i;
    columnInfo *colInfos;
    value *resultHash,
	*rowList,
	*colInfoHash,
	*columnNameList;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "_sqlAdabasQuery");
	return;
    }

    assert(args[0].value.value->type == VALUE_WHATSIT);
    whatsit = (odbcWhatsit*)valueWhatsitData(args[0].value.value);

    transformArgumentToScalar(&args[1]);

    rc = SQLExecDirect(whatsit->hstmt, args[1].value.value->v.scalar.scalar.data, SQL_NTS);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
    {
	OUT_VALUE_REF(ow, valueNewHash());
	return;
    }

    rc = SQLNumResultCols(whatsit->hstmt, &numCols);
    if ((rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) || numCols == 0)
    {
	OUT_VALUE_REF(ow, valueNewHash());
	return;
    }

    resultHash = valueNewHash();
    rowList = valueNewList();
    ds = dsNewFrom("rows"); valueHashDefine(resultHash, &ds, rowList);
    colInfoHash = valueNewHash();
    ds = dsNewFrom("colinfo"); valueHashDefine(resultHash, &ds, colInfoHash);

    colInfos = (columnInfo*)memXAlloc(sizeof(columnInfo) * numCols);

    columnNameList = valueNewList();

    for (i = 1; i <= numCols; ++i)
    {
	char colNameString[128];
	SWORD nameLength;
	ds colName,
	    ds2;

	rc = SQLColAttributes(whatsit->hstmt, i, SQL_COLUMN_DISPLAY_SIZE, 0, 0, 0,
			      &colInfos[i].displaySize);
	assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	rc = SQLColAttributes(whatsit->hstmt, i, SQL_COLUMN_TYPE, 0, 0, 0,
			      &colInfos[i].type);
	assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	rc = SQLColAttributes(whatsit->hstmt, i, SQL_COLUMN_NAME,
			      &colNameString, 128, &nameLength, 0);
	assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

	colName = dsNewFrom(colNameString);
	valueListSetElement(columnNameList, i - 1, valueNewScalar(&ds));
	
	ds2 = dsNewFrom("type");
	valueHashDefine(infoHash, &ds2,
			valueNewScalarFromCString(typeNameForSQLType(colInfos[i].type)));
    }

    
}

void
registerDatabaseAdabasBuiltIns (void)
{
    registerBuiltIn("_sqlAdabasConnect", builtIn_sqlAdabasConnect, 1, 0, 0);
    registerBuiltIn("_sqlAdabasClose", builtIn_sqlAdabasClose, 1, 0, 0);
}
