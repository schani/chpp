/* -*- c -*- */

/*
 * arith.h
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

#include "dynstring.h"

struct _environment;

#define NUMBER_INT       1
#define NUMBER_DOUBLE    2
#define NUMBER_STRING    3

typedef struct
{
    int type;
    struct
    {
	int intVal;
	double doubleVal;
	dynstring stringVal;
    } val;
} arithNumber;

#define YYSTYPE arithNumber

void arithScanFromDS (dynstring *ds);
void arithEndScanning (void);

dynstring arithEvalDS (dynstring *ds, struct _environment *env);
