/* -*- c -*- */

/*
 * value.h
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

#ifndef __VALUE_H__
#define __VALUE_H__

#include "chash.h"
#include "avl.h"

#include "dynstring.h"
#include "internals.h"
#include "builtins/builtins.h"

struct _bytecode;
struct _environment;

#define VALUE_UNDEFINED              0
#define VALUE_INTERNAL               1
#define VALUE_BUILT_IN               2
#define VALUE_SCALAR                 3
#define VALUE_LIST                   4
#define VALUE_HASH                   5
#define VALUE_LAMBDA                 6
#define VALUE_WHATSIT                7

typedef struct _value
{
    int type;

    union
    {
	struct
	{
	    internalSet setFunc;
	    internalGet getFunc;
	} internal;

	struct
	{
	    builtIn function;
	    int evalParams;
	    builtInGlobalEffector globalEffector;
	    builtInEnvironmentor environmentor;
	} builtIn;

	struct
	{
	    dynstring scalar;
	} scalar;

	struct
	{
	    avlTree *list;
	} list;

	struct
	{
	    hash hash;
	} hash;

	struct
	{
	    int numParams;
	    int minVarArgs;
	    int maxVarArgs;
	    dynstring *paramNames;
	    char metaChar;
	    struct _bytecode *code;
	    struct _environment *env;
	} lambda;

	struct
	{
	    void *data;
	} whatsit;
    } v;
} value;

value* valueNewUndefined (void);
value* valueNewInternal (internalSet setFunc, internalGet getFunc);
value* valueNewBuiltIn (builtIn function, int evalParams,
			builtInGlobalEffector globalEffector,
			builtInEnvironmentor environmentor);
value* valueNewScalar (dynstring *scalar);
value* valueNewScalarFromCString (const char *scalar);
value* valueNewScalarFromBytes (const char *scalar, int length);
value* valueNewList (void);
value* valueNewHash (void);
value* valueNewLambda (int numParams, int minVarArgs, int maxVarArgs,
		       dynstring *paramNames, char metaChar,
		       struct _bytecode *code, struct _environment *env);
value* valueNewWhatsit (void *data);

value* valueNewScalarFromValue (value *theValue);

value* valueCopy (value *theValue);
value* valueCopyDeep (value *theValue);

int valueCompare (value *value1, value *value2);
int valueAreSame (value *value1, value *value2);
int valueAreEqual (value *value1, value *value2);

value* valueAssign (value *dest, value *src, int share);

int valueBoolValue (value *theValue);

int valueHashCount (value *theValue);
void valueHashDefine (value *aValue, dynstring *key, value *defValue);
value* valueHashLookup (value *value, dynstring *key);
value* valueHashLookupCString (value *value, const char *key);

int valueListLength (value *value);
value* valueListGetElement (value *value, int index);
void valueListSetElement (value *theValue, int index, value *defValue);
void valueListInsertElement (value *theValue, int index, value *defValue);
void valueListDeleteElement (value *theValue, int index);
void valueListAppendList (value *list, value *appended, int destroy);
void valueListClear (value *theValue);

void* valueWhatsitData (value *theValue);

value* valueTransformToScalar (value *theValue);

const char* cStringForValueType (int type);

#endif
