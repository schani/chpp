/* -*- c -*- */

/*
 * internals.h
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

#ifndef __INTERNALS_H__
#define __INTERNALS_H__

struct _value;
struct _outputWriter;
struct _bcSubscript;
struct _environment;

extern char *metaChar;
extern char quoteChar;

typedef void (*internalSet) (struct _value *theValue, int needCopy,
			     struct _bcSubscript *subscripts,
			     struct _environment *env);
typedef void (*internalGet) (struct _bcSubscript *subscripts,
			     struct _environment *env,
			     struct _outputWriter *ow);

void registerInternals (void);

#endif
