/* -*- c -*- */

/*
 * value.c
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "dynstring.h"
#include "bytecode.h"

#include "value.h"

static value*
allocValue (void)
{
    value *theValue = (value*)memXAlloc(sizeof(value));

    return theValue;
}

value*
valueNewUndefined (void)
{
    value *theValue = allocValue();

    theValue->type = VALUE_UNDEFINED;

    return theValue;
}

value*
valueNewInternal (internalSet setFunc, internalGet getFunc)
{
    value *theValue = allocValue();

    theValue->type = VALUE_INTERNAL;

    theValue->v.internal.setFunc = setFunc;
    theValue->v.internal.getFunc = getFunc;

    return theValue;
}

value*
valueNewBuiltIn (builtIn function, int evalParams,
		 builtInGlobalEffector globalEffector,
		 builtInEnvironmentor environmentor)
{
    value *theValue = allocValue();

    theValue->type = VALUE_BUILT_IN;

    theValue->v.builtIn.function = function;
    theValue->v.builtIn.evalParams = evalParams;
    theValue->v.builtIn.globalEffector = globalEffector;
    theValue->v.builtIn.environmentor = environmentor;

    return theValue;
}

value*
valueNewScalar (dynstring *scalar)
{
    value *theValue = allocValue();

    theValue->type = VALUE_SCALAR;

    theValue->v.scalar.scalar = dsCopy(scalar);

    return theValue;
}

value*
valueNewScalarFromCString (const char *scalar)
{
    value *theValue = allocValue();

    theValue->type = VALUE_SCALAR;

    theValue->v.scalar.scalar = dsNewFrom(scalar);

    return theValue;
}

value*
valueNewScalarFromBytes (const char *scalar, int length)
{
    value *theValue = allocValue();

    theValue->type = VALUE_SCALAR;

    theValue->v.scalar.scalar = dsNewFromBytes(scalar, length);

    return theValue;
}

value*
valueNewList (void)
{
    value *theValue = allocValue();

    theValue->type = VALUE_LIST;

    theValue->v.list.list = avlNew();

    return theValue;
}

value*
valueNewHash (void)
{
    value *theValue = allocValue();

    theValue->type = VALUE_HASH;

    theValue->v.hash.hash = hash_new(32);

    return theValue;
}

value*
valueNewLambda (int numParams, int minVarArgs, int maxVarArgs,
		dynstring *paramNames, bytecode *code, environment *env,
		int evalParams)
{
    value *theValue = allocValue();
    int i;

    theValue->type = VALUE_LAMBDA;

    theValue->v.lambda.numParams = numParams;
    theValue->v.lambda.minVarArgs = minVarArgs;
    theValue->v.lambda.maxVarArgs = maxVarArgs;

    if (maxVarArgs != 0)
	++numParams;

    theValue->v.lambda.paramNames = (dynstring*)memXAlloc(sizeof(dynstring) * numParams);
    for (i = 0; i < numParams; ++i)
	theValue->v.lambda.paramNames[i] = dsCopy(&paramNames[i]);

    theValue->v.lambda.code = code;
    theValue->v.lambda.env = env;
    theValue->v.lambda.evalParams = evalParams;

    return theValue;
}

value*
valueNewEnvironment (environment *env)
{
    value *theValue = allocValue();

    theValue->type = VALUE_ENVIRONMENT;
    theValue->v.env.env = env;

    return theValue;
}

value*
valueNewBytecode (bytecode *code)
{
    value *theValue = allocValue();

    theValue->type = VALUE_BYTECODE;
    theValue->v.bytecode.code = code;

    return theValue;
}

value*
valueNewWhatsit (void *data)
{
    value *theValue = allocValue();

    theValue->type = VALUE_WHATSIT;
    theValue->v.whatsit.data = data;

    return theValue;
}

value*
valueNewScalarFromValue (value *theValue)
{
    switch (theValue->type)
    {
	case VALUE_UNDEFINED :
	    return valueNewScalarFromCString("");

	case VALUE_INTERNAL :
	    return valueNewScalarFromCString("<internal>");

	case VALUE_BUILT_IN :
	    return valueNewScalarFromCString("<builtIn>");

	case VALUE_SCALAR :
	    return valueCopy(theValue);

	case VALUE_LIST :
	    return valueNewScalarFromCString("<list>");

	case VALUE_HASH :
	    return valueNewScalarFromCString("<hash>");

	case VALUE_LAMBDA :
	    return valueNewScalarFromCString("<lambda>");

	case VALUE_ENVIRONMENT :
	    return valueNewScalarFromCString("<environment>");

	case VALUE_BYTECODE :
	    return valueNewScalarFromCString("<bytecode>");

	case VALUE_WHATSIT :
	    return valueNewScalarFromCString("<whatsit>");
    }

    return 0;
}

value*
valueCopy (value *theValue)
{
    value *theCopy = allocValue();

    theCopy->type = theValue->type;

    switch (theValue->type)
    {
	case VALUE_UNDEFINED :
	case VALUE_INTERNAL :
	case VALUE_BUILT_IN :
	case VALUE_ENVIRONMENT :
	case VALUE_BYTECODE :
	case VALUE_WHATSIT :
	    *theCopy = *theValue;
	    return theCopy;

	case VALUE_SCALAR :
	    theCopy->v.scalar.scalar = dsCopy(&theValue->v.scalar.scalar);
	    return theCopy;

	case VALUE_LIST :
	    {
		avlNode *node;
		int i = 1;

		theCopy->v.list.list = avlNew();

		node = avlFirst(theValue->v.list.list, 1);
		while (node != 0)
		{
		    avlInsert(theCopy->v.list.list, (value*)avlKey(node), i++);
		    node = avlNext(node);
		}
	    }
	    return theCopy;
	    
	case VALUE_HASH :
	    {
		hstate state;
		value *aValue;
		char *aKey;

		theCopy->v.hash.hash = hash_new(hash_count(theValue->v.hash.hash));
		
		state = hash_state(theValue->v.hash.hash);
		while ((aValue = (value*)hash_next(&state, &aKey)) != 0)
		{
		    char *copiedKey = strcpy((char*)memXAllocAtomic(strlen(aKey) + 1), aKey);

		    hash_inject(theCopy->v.hash.hash, copiedKey, aValue);
		}
	    }
	    return theCopy;

	case VALUE_LAMBDA :
	    {
		int i;

		theCopy->v.lambda.numParams = theValue->v.lambda.numParams;
		theCopy->v.lambda.minVarArgs = theValue->v.lambda.minVarArgs;
		theCopy->v.lambda.maxVarArgs = theValue->v.lambda.maxVarArgs;
		theCopy->v.lambda.paramNames =
		    (dynstring*)memXAllocAtomic(sizeof(dynstring)
						* theValue->v.lambda.numParams);
		for (i = 0; i < theValue->v.lambda.numParams; ++i)
		    theCopy->v.lambda.paramNames[i] =
			dsCopy(&theValue->v.lambda.paramNames[i]);
		theCopy->v.lambda.code = theValue->v.lambda.code;
		theCopy->v.lambda.env = theValue->v.lambda.env;
	    }
	    return theCopy;

	default :
	    assert(0);
    }

    return 0;
}

value*
valueCopyDeep (value *theValue)
{
    value *theCopy = allocValue();

    theCopy->type = theValue->type;

    switch (theValue->type)
    {
	case VALUE_UNDEFINED :
	case VALUE_INTERNAL :
	case VALUE_BUILT_IN :
	case VALUE_ENVIRONMENT :
	case VALUE_BYTECODE :
	case VALUE_WHATSIT :
	    *theCopy = *theValue;
	    return theCopy;

	case VALUE_SCALAR :
	    theCopy->v.scalar.scalar = dsCopy(&theValue->v.scalar.scalar);
	    return theCopy;

	case VALUE_LIST :
	    {
		avlNode *node;
		int i = 1;

		theCopy->v.list.list = avlNew();

		node = avlFirst(theValue->v.list.list, 1);
		while (node != 0)
		{
		    avlInsert(theCopy->v.list.list, valueCopyDeep((value*)avlKey(node)), i++);
		    node = avlNext(node);
		}
	    }
	    return theCopy;
	    
	case VALUE_HASH :
	    {
		hstate state;
		value *aValue;
		char *aKey;

		theCopy->v.hash.hash = hash_new(hash_count(theValue->v.hash.hash));
		
		state = hash_state(theValue->v.hash.hash);
		while ((aValue = (value*)hash_next(&state, &aKey)) != 0)
		{
		    value *copiedValue = valueCopyDeep(aValue);
		    char *copiedKey = strcpy((char*)memXAllocAtomic(strlen(aKey) + 1), aKey);

		    hash_inject(theCopy->v.hash.hash, copiedKey, copiedValue);
		}
	    }
	    return theCopy;

	case VALUE_LAMBDA :
	    {
		int i;

		theCopy->v.lambda.numParams = theValue->v.lambda.numParams;
		theCopy->v.lambda.minVarArgs = theValue->v.lambda.minVarArgs;
		theCopy->v.lambda.maxVarArgs = theValue->v.lambda.maxVarArgs;
		theCopy->v.lambda.paramNames =
		    (dynstring*)memXAllocAtomic(sizeof(dynstring)
						* theValue->v.lambda.numParams);
		for (i = 0; i < theValue->v.lambda.numParams; ++i)
		    theCopy->v.lambda.paramNames[i] =
			dsCopy(&theValue->v.lambda.paramNames[i]);
		theCopy->v.lambda.code = theValue->v.lambda.code;
		theCopy->v.lambda.env = theValue->v.lambda.env;
	    }
	    return theCopy;

	default :
	    assert(0);
    }

    return 0;
}

int
valueCompare (value *value1, value *value2)
{
    /* this should do a more sophisticated comparison */

    return strcmp(valueTransformToScalar(value1)->v.scalar.scalar.data,
		  valueTransformToScalar(value2)->v.scalar.scalar.data);
}

int
valueAreSame (value *value1, value *value2)
{
    return value1 == value2;
}

int
valueAreEqual (value *value1, value *value2)
{
    if (valueAreSame(value1, value2))
	return 1;

    if (value1->type != value2->type)
	return 0;

    switch (value1->type)
    {
	case VALUE_INTERNAL :
	    return value1->v.internal.setFunc == value2->v.internal.setFunc
		&& value1->v.internal.getFunc == value2->v.internal.getFunc;

	case VALUE_BUILT_IN :
	    return value1->v.builtIn.function == value2->v.builtIn.function;

	case VALUE_SCALAR :
	    if (value1->v.scalar.scalar.length != value2->v.scalar.scalar.length)
		return 0;

	    return memcmp(value1->v.scalar.scalar.data, value2->v.scalar.scalar.data,
			  value1->v.scalar.scalar.length) == 0;

	case VALUE_LIST :
	    {
		int length,
		    i;

		length = valueListLength(value1);

		if (length != valueListLength(value2))
		    return 0;

		for (i = 0; i < length; ++i)
		    if (!valueAreEqual(valueListGetElement(value1, i),
				       valueListGetElement(value2, i)))
			return 0;

		return 1;
	    }

	case VALUE_HASH :
	    {
		hstate state;
		value *aValue;
		char *aKey;

		if (valueHashCount(value1) != valueHashCount(value2))
		    return 0;

		state = hash_state(value1->v.hash.hash);
		while ((aValue = (value*)hash_next(&state, &aKey)) != 0)
		{
		    value *otherValue = (value*)hash_lookup(value2->v.hash.hash, aKey);

		    if (otherValue == 0 || !valueAreEqual(aValue, otherValue))
			return 0;
		}

		return 1;
	    }

	case VALUE_LAMBDA :
	    return value1->v.lambda.code == value2->v.lambda.code;

	case VALUE_ENVIRONMENT :
	    return value1->v.env.env == value2->v.env.env;

	case VALUE_BYTECODE :
	    return value1->v.bytecode.code == value2->v.bytecode.code;

	case VALUE_WHATSIT :
	    return value1->v.whatsit.data == value2->v.whatsit.data;

	default :
	    assert(0);
    }

    return 0;
}

value*
valueAssign (value *dest, value *src, int share)
{
    if (share)
	*dest = *src;
    else
    {
	value *copy = valueCopy(src);

	*dest = *copy;
    }

    return dest;
}

int
valueBoolValue (value *theValue)
{
    if (theValue->type == VALUE_SCALAR)
    {
	if (strcmp(theValue->v.scalar.scalar.data, "0") == 0)
	    return 0;
	else
	    return theValue->v.scalar.scalar.length != 0;
    }
    else if (theValue->type == VALUE_LIST)
	return valueListLength(theValue) != 0;
    else if (theValue->type == VALUE_HASH)
	return valueHashCount(theValue) != 0;
    else
	return 1;
}

int
valueHashCount (value *theValue)
{
    assert(theValue->type == VALUE_HASH);

    return hash_count(theValue->v.hash.hash);
}

void
valueHashDefine (value *theValue, dynstring *key, value *defValue)
{
    value *hashEntry;

    assert(theValue->type == VALUE_HASH);

    hashEntry = (value*)hash_lookup(theValue->v.hash.hash, key->data);
    hash_insert(theValue->v.hash.hash, key->data, defValue);
}

void
valueHashDefineCString (value *theValue, const char *key, value *defValue)
{
    value *hashEntry;

    assert(theValue->type == VALUE_HASH);

    hashEntry = (value*)hash_lookup(theValue->v.hash.hash, key);
    hash_insert(theValue->v.hash.hash, key, defValue);
}

value*
valueHashLookup (value *theValue, dynstring *key)
{
    assert(theValue->type == VALUE_HASH);

    return (value*)hash_lookup(theValue->v.hash.hash, key->data);
}

value*
valueHashLookupCString (value *theValue, const char *key)
{
    assert(theValue->type == VALUE_HASH);

    return (value*)hash_lookup(theValue->v.hash.hash, key);
}

void
valueHashDelete (value *theValue, dynstring *key)
{
    assert(theValue->type == VALUE_HASH);

    hash_delete(theValue->v.hash.hash, key->data);
}

int
valueListLength (value *theValue)
{
    assert(theValue->type == VALUE_LIST);

    return avlNumElements(theValue->v.list.list);
}

value*
valueListGetElement (value *theValue, int index)
{
    assert(theValue->type == VALUE_LIST);
    assert(index >= 0 && index < (int)avlNumElements(theValue->v.list.list));

    return (value*)avlGet(theValue->v.list.list, index + 1);
}

void
valueListSetElement (value *theValue, int index, value *defValue)
{
    assert(theValue->type == VALUE_LIST);
    assert(index >= 0);

    if (index >= (int)avlNumElements(theValue->v.list.list))
    {
	int i = index - avlNumElements(theValue->v.list.list);

	for (; i > 0; --i)
	    avlInsert(theValue->v.list.list, valueNewScalarFromCString(""),
		      avlNumElements(theValue->v.list.list));

	avlInsert(theValue->v.list.list, defValue, index);
    }
    else
    {
	value *listEntry;

	listEntry = (value*)avlGet(theValue->v.list.list, index + 1);
	avlDelete(theValue->v.list.list, index + 1);
	avlInsert(theValue->v.list.list, defValue, index);
    }
}

void
valueListInsertElement (value *theValue, int index, value *defValue)
{
    assert(theValue->type == VALUE_LIST);
    assert(index >= 0);

    if (index >= (int)avlNumElements(theValue->v.list.list))
	valueListSetElement(theValue, index, defValue);
    else
	avlInsert(theValue->v.list.list, defValue, index);
}

void
valueListDeleteElement (value *theValue, int index)
{
    value *element;

    assert(theValue->type == VALUE_LIST);
    assert(index >= 0 && index < (int)avlNumElements(theValue->v.list.list));

    element = avlGet(theValue->v.list.list, index + 1);
    avlDelete(theValue->v.list.list, index + 1);
}

void
valueListAppendList (value *list, value *appended, int destroy)
{
    int index = avlNumElements(list->v.list.list);
    avlNode *node = avlFirst(appended->v.list.list, 1);

    while (node != 0)
    {
	value *theValue;

	if (destroy)
	    theValue = (value*)avlKey(node);
	else
	    theValue = valueCopy((value*)avlKey(node));

	avlInsert(list->v.list.list, theValue, ++index);

	node = avlNext(node);
    }
}

void
valueListClear (value *theValue)
{
    assert(theValue->type == VALUE_LIST);

    theValue->v.list.list = avlNew();
}

void*
valueWhatsitData (value *theValue)
{
    return theValue->v.whatsit.data;
}

value*
valueTransformToScalar (value *theValue)
{
    if (theValue->type != VALUE_SCALAR)
    {
	value *scalarValue = valueNewScalarFromValue(theValue);

	*theValue = *scalarValue;
    }

    return theValue;
}

const char*
cStringForValueType (int type)
{
    static char *valueTypes[] =
        {
	    "undefined", 
	    "internal",
	    "built-in",
	    "scalar",
	    "list",
	    "hash",
	    "lambda",
	    "environment",
	    "bytecode",
	    "whatsit"
	};

    return valueTypes[type];
}
