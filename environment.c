/* -*- c -*- */

/*
 * environment.h
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

#include <assert.h>

#include "memory.h"
#include "value.h"

#include "environment.h"

environment *globalEnvironment = 0;

void
initEnvironments (void)
{
    globalEnvironment = envNew(0);
}

environment*
envNew (environment *parent)
{
    environment *env = (environment*)memXAlloc(sizeof(environment));

    env->firstBinding = 0;
    env->parent = parent;

    return env;
}

void
envAddBinding (environment *env, dynstring *name, value *theValue)
{
    binding *theBinding = (binding*)memXAlloc(sizeof(binding));

    if (env == 0)
	env = globalEnvironment;

    theBinding->name = dsCopy(name);
    theBinding->value = theValue;
    theBinding->next = env->firstBinding;
    env->firstBinding = theBinding;
}

void
envModifyBinding (environment *env, dynstring *name, struct _value *theValue)
{
    binding *aBinding;

    assert(theValue != 0);

    while (env != 0)
    {
	for (aBinding = env->firstBinding; aBinding != 0; aBinding = aBinding->next)
	    if (strcmp(name->data, aBinding->name.data) == 0)
	    {
		aBinding->value = theValue;
		return;
	    }
	env = env->parent;
    }

    assert(0);
}

void
envModifyOrAddBinding (environment *env, dynstring *name, struct _value *theValue,
		       environment *defEnv)
{
    binding *aBinding;

    if (theValue == 0)
	theValue = valueNewScalarFromCString("<heinz>"); /* FIXME: can this be true? */

    while (env != 0)
    {
	for (aBinding = env->firstBinding; aBinding != 0; aBinding = aBinding->next)
	    if (strcmp(name->data, aBinding->name.data) == 0)
	    {
		aBinding->value = theValue;
		return;
	    }
	env = env->parent;
    }

    aBinding = (binding*)memXAlloc(sizeof(binding));

    aBinding->name = dsCopy(name);
    aBinding->value = theValue;
    aBinding->next = defEnv->firstBinding;
    defEnv->firstBinding = aBinding;
}

value*
envGetValueForBinding (environment *env, dynstring *name)
{
    binding *aBinding;

    while (env != 0)
    {
	for (aBinding = env->firstBinding; aBinding != 0; aBinding = aBinding->next)
	    if (strcmp(name->data, aBinding->name.data) == 0)
		return aBinding->value;
	env = env->parent;
    }

    return 0;
}

binding*
envGetBinding (environment *env, dynstring *name)
{
    binding *aBinding;

    while (env != 0)
    {
	for (aBinding = env->firstBinding; aBinding != 0; aBinding = aBinding->next)
	    if (strcmp(name->data, aBinding->name.data) == 0)
		return aBinding;
	env = env->parent;
    }

    return 0;
}

void
envAddBindings (environment *env, environment *additions)
{
    binding *aBinding = additions->firstBinding;

    while (aBinding != 0)
    {
	if (envGetBinding(env, &aBinding->name) == 0)
	    envAddBinding(env, &aBinding->name, aBinding->value);
	aBinding = aBinding->next;
    }
}

environment*
envTop (environment *env)
{
    while (env->parent != 0)
	env = env->parent;

    return env;
}

environment*
envNewUnion (environment *env1, environment *env2)
{
    environment *result = envNew(0);
    binding *aBinding = env1->firstBinding;

    while (aBinding != 0)
    {
	envAddBinding(result, &aBinding->name, aBinding->value);
	aBinding = aBinding->next;
    }

    envAddBindings(result, env2);

    return result;
}

environment*
envNewIntersection (environment *env1, environment *env2)
{
    environment *result = envNew(0),
	*env2Parent = env2->parent;
    binding *aBinding = env1->firstBinding;

    env2->parent = 0;

    while (aBinding != 0)
    {
	if (envGetBinding(env2, &aBinding->name))
	    envAddBinding(result, &aBinding->name, aBinding->value);
	aBinding = aBinding->next;
    }

    env2->parent = env2Parent;

    return result;
}
