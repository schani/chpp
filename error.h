/* -*- c -*- */

/*
 * error.h
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


#ifndef __ERROR_H__
#define __ERROR_H__

void issueWarning (int num, ...);
void issueError (int num, ...);

extern int warningsOccured;
extern int errorsOccured;

#define ERRFRNT_MULTI_OUTPUT           0
#define ERRFRNT_WRONG_OUTPUT           1
#define ERRFRNT_WRONG_INPUT            2

#define WARNCMD_UNUSED_ARGS         1000
#define WARNCMD_UNKNOWN_COMMAND     1001

#define ERRCMD_UNMATCHED_COMMAND    1000
#define ERRCMD_NO_INCLUDE           1001
#define ERRCMD_CLOSE_PAREN_EXPECTED 1002
#define ERRCMD_ONE_ARG              1003
#define ERRCMD_USER_ERROR           1004

#define WARNMAC_USER_WARNING        2000
#define WARNMAC_DUPLICATE_HASH_KEY  2001
#define WARNMAC_UNBOUND_VARIABLE    2002
#define WARNMAC_WRONG_NUM_ARGS      2003
#define WARNMAC_SUSPECT_CHARACTER   2004

#define ERRMAC_UNDEFINED            2000
#define ERRMAC_UNMATCHED_MACROCALL  2001
#define ERRMAC_ZERODIV              2002
#define ERRMAC_ARITH_SYNTAX         2003
#define ERRMAC_SET_INTERNAL         2004
#define ERRMAC_INVALID_REGEX        2005
#define ERRMAC_INVALID_MACRO_ARG    2006
#define ERRMAC_CALL_FAILED          2007
#define ERRMAC_INTERNAL_NOT_ASSOC   2008
#define ERRMAC_KEY_NOT_IN_HASH      2009
#define ERRMAC_REDEFINE_BUILT_IN    2010
#define ERRMAC_USER_ERROR           2011
#define ERRMAC_NULL_INCREMENT       2012
#define ERRMAC_NOT_IMPLEMENTED      2013
#define ERRMAC_INDEX_OUT_OF_BOUNDS  2014
#define ERRMAC_INTERNAL_NOT_ARRAY   2015
#define ERRMAC_SYMBOL_WRONG_TYPE    2016
#define ERRMAC_VALUE_WRONG_TYPE     2017
#define ERRMAC_NOT_LVALUE           2018
#define ERRMAC_WRONG_NUM_ARGS       2019
#define ERRMAC_WRONG_NUM_ARGS_USER  2020
#define ERRMAC_UNBOUND_VARIABLE     2021
#define ERRMAC_GET_INTERNAL         2022
#define ERRMAC_LIST_INDEXED_AS_HASH 2023
#define ERRMAC_UNEXPECTED_CHAR      2024
#define ERRMAC_PREMATURE_END        2025

#endif
