/* -*- c -*- */

/*
 * precompile.c
 *
 * chpp
 *
 * Copyright (C) 1999 Mark Probst
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

#include <stdio.h>
#include <assert.h>

#include "byteorder.h"
#include "dynstring.h"
#include "bytecode.h"

#include "precompile.h"

static void
out_char (FILE *out, char c)
{
    assert(fwrite(&c, 1, 1, out) == 1);
}

static void
out_int (FILE *out, int i)
{
    unsigned4 u4 = bo_host_to_canonical_4((unsigned4)i);

    assert(fwrite(&u4, 4, 1, out) == 1);
}

static void
out_dynstring (FILE *out, dynstring *ds)
{
    out_int(out, ds->length);
    assert(fwrite(ds->data, 1, ds->length, out) == ds->length);
}

static void
out_subscripts (FILE *out, bcSubscript *subscript)
{
    if (subscript == 0)
	out_char(out, 0);
    else
    {
	out_char(out, (char)subscript->type);
	prcpWriteBytecode(out, subscript->bc);
	out_subscripts(out, subscript->next);
    }
}

static void
out_arguments (FILE *out, bcArgument *argument)
{
    if (argument == 0)
	prcpWriteBytecode(out, 0);
    else
    {
	prcpWriteBytecode(out, argument->bc);
	out_arguments(out, argument->next);
    }
}

void
prcpWriteBytecode (FILE *out, bytecode *bc)
{
    if (bc == 0)
	out_char(out, 0);
    else
    {
	out_char(out, (char)bc->type);

	switch (bc->type)
	{
	    case BYTECODE_STRING :
	    case BYTECODE_QUOTED_STRING :
		out_dynstring(out, &bc->v.string.string);
		break;

	    case BYTECODE_EVAL :
		prcpWriteBytecode(out, bc->v.eval.bc);
		break;

	    case BYTECODE_ARITH :
		prcpWriteBytecode(out, bc->v.arith.bc);
		break;

	    case BYTECODE_ASSIGNMENT :
		if (bc->v.assignment.modify)
		    out_char(out, 1);
		else
		    out_char(out, 0);
		prcpWriteBytecode(out, bc->v.assignment.varName);
		out_subscripts(out, bc->v.assignment.subscript);
		prcpWriteBytecode(out, bc->v.assignment.newValue);
		break;

	    case BYTECODE_MACRO :
		out_char(out, (char)bc->v.macro.flags);
		prcpWriteBytecode(out, bc->v.macro.nameOrValue);
		out_subscripts(out, bc->v.macro.subscript);
		out_arguments(out, bc->v.macro.arguments);
		break;
	}

	prcpWriteBytecode(out, bc->next);
    }
}
