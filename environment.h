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

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "dynstring.h"

struct _value;

typedef struct _binding
{
    dynstring name;
    struct _value *value;

    struct _binding *next;
} binding;

typedef struct _environment
{
    binding *firstBinding;

    struct _environment *parent;
} environment;

extern environment *globalEnvironment;

void initEnvironments (void);

environment* envNew (environment *parent);
void envAddBinding (environment *env, dynstring *name, struct _value *theValue);
void envModifyBinding (environment *env, dynstring *name, struct _value *theValue);
void envModifyOrAddBinding (environment *env, dynstring *name, struct _value *theValue,
			    environment *defEnv);
binding* envGetBinding (environment *env, dynstring *name);
struct _value* envGetValueForBinding (environment *env, dynstring *name);

void envAddBindings (environment *env, environment *additions);

environment* envTop (environment *env);

environment* envNewUnion (environment *env1, environment *env2);
environment* envNewIntersection (environment *env1, environment *env2);

#endif
