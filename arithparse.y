%{
/* -*- c -*- */

/*
 * arithparse.y
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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "error.h"
#include "environment.h"
#include "value.h"

#include "arith.h"

#define YYSTYPE arithNumber

static arithNumber arithResult;
static environment *arithEnv;

%}

%left T_OR
%left T_AND
%left '|'
%left '^'
%left '&'
%left T_EQ T_NE
%left '<' '>' T_LE T_GE
%left '+' '-'
%left '*' '/' '%'
%left '!' '~' NEG

%token T_NUM T_IDENT

%%

start :   expr { arithResult = $1; }
        ;

expr :   expr '+' expr
{
    if ($1.type == NUMBER_INT && $3.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = $1.val.intVal + $3.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = $1.val.doubleVal + $3.val.doubleVal;
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | expr '-' expr
{
    if ($1.type == NUMBER_INT && $3.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = $1.val.intVal - $3.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = $1.val.doubleVal - $3.val.doubleVal;
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | expr '*' expr
{
    if ($1.type == NUMBER_INT && $3.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = $1.val.intVal * $3.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = $1.val.doubleVal * $3.val.doubleVal;
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | expr '/' expr
{
    if ($3.val.doubleVal == 0.0)
    {
	issueError(ERRMAC_ZERODIV, 0, 0, 0);
	$$.type = NUMBER_INT;
	$$.val.intVal = 0;
	$$.val.doubleVal = 0.0;
    }
    else if ($1.type == NUMBER_INT && $3.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = $1.val.intVal / $3.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = $1.val.doubleVal / $3.val.doubleVal;
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | expr '%' expr
{
    if ($3.val.doubleVal == 0.0)
    {
	issueError(ERRMAC_ZERODIV, 0, 0, 0);
	$$.type = NUMBER_INT;
	$$.val.intVal = 0;
	$$.val.doubleVal = 0.0;
    }
    else if ($1.type == NUMBER_INT && $3.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = $1.val.intVal % $3.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = fmod($1.val.doubleVal, $3.val.doubleVal);
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | expr '<' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal < $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr '>' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal > $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_LE expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal <= $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_GE expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal >= $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_EQ expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal == $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_NE expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal != $3.val.doubleVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr '|' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.intVal | $3.val.intVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr '&' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.intVal & $3.val.intVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr '^' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.intVal ^ $3.val.intVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_AND expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal != 0.0 && $3.val.doubleVal != 0;
    $$.val.doubleVal = $$.val.intVal;
}
       | expr T_OR expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $1.val.doubleVal != 0.0 || $3.val.doubleVal != 0;
    $$.val.doubleVal = $$.val.intVal;
}
       | '-' expr %prec NEG
{
    if ($2.type == NUMBER_INT)
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = -$2.val.intVal;
	$$.val.doubleVal = $$.val.intVal;
    }
    else
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = -$2.val.doubleVal;
	$$.val.intVal = $$.val.doubleVal;
    }
}
       | '!' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = $2.val.doubleVal == 0.0;
    $$.val.doubleVal = $$.val.intVal;
}
       | '~' expr
{
    $$.type = NUMBER_INT;
    $$.val.intVal = ~$2.val.intVal;
    $$.val.doubleVal = $$.val.intVal;
}
       | term { $$ = $1; }
       ;

term :   T_NUM { $$ = $1; }
       | T_IDENT
{
    value *theValue = envGetValueForBinding(arithEnv, &$1.val.stringVal);

    if (theValue == 0)
    {
	issueError(ERRMAC_UNBOUND_VARIABLE, $1.val.stringVal.data);
	YYABORT;
    }

    theValue = valueNewScalarFromValue(theValue);

    if (strchr(theValue->v.scalar.scalar.data, '.'))
    {
	$$.type = NUMBER_DOUBLE;
	$$.val.doubleVal = atof(theValue->v.scalar.scalar.data);
	$$.val.intVal = $$.val.doubleVal;
    }
    else
    {
	$$.type = NUMBER_INT;
	$$.val.intVal = atoi(theValue->v.scalar.scalar.data);
	$$.val.doubleVal = $$.val.intVal;
    }
}
       | '(' expr ')' { $$ = $2; }
       ;

%%

int
yyerror (char *s)
{
    return 0;
}

dynstring
arithEvalDS (dynstring *dsExpr, environment *env)
{
    static char number[64];
    dynstring ds;

    /*    fprintf(stderr, "evaling '%s'\n", dsExpr->data);*/

    arithScanFromDS(dsExpr);
    arithEnv = env;
    
    if (yyparse() == 0)
    {
	if (arithResult.type == NUMBER_INT)
	    sprintf(number, "%d", arithResult.val.intVal);
	else
	    sprintf(number, "%f", arithResult.val.doubleVal);
	ds = dsNewFrom(number);
    }
    else
    {
	issueError(ERRMAC_ARITH_SYNTAX, dsExpr->data, 0, 0);
	ds = dsNew();
    }

    arithEnv = 0;
    arithEndScanning();

    return ds;
}
