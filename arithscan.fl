%option noyywrap

%{
/* -*- c -*- */

/*
 * arithscan.fl
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

#include "arith.h"
#include "arithparse.h"
%}

%%

"||"        return T_OR;
"&&"        return T_AND;
"=="        return T_EQ;
"!="        return T_NE;
"<="        return T_LE;
">="        return T_GE;

[a-zA-Z_][a-zA-Z0-9_]* { yylval.type = NUMBER_STRING; yylval.val.stringVal = dsNewFrom(yytext); return T_IDENT; }

[0-9]+      { yylval.type = NUMBER_INT; yylval.val.intVal = atoi(yytext); yylval.val.doubleVal = yylval.val.intVal; return T_NUM; }
[0-9]*\.[0-9]+ { yylval.type = NUMBER_DOUBLE; yylval.val.doubleVal = atof(yytext); yylval.val.intVal = yylval.val.doubleVal; return T_NUM; }

[ \t\n]+    ;

.           return yytext[0];

%%

static YY_BUFFER_STATE bufferState;

void
arithScanFromDS (dynstring *ds)
{
    bufferState = yy_scan_string(ds->data);
}

void
arithEndScanning (void)
{
    yy_delete_buffer(bufferState);
}
