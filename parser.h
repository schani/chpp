/* -*- c -*- */

/*
 * parser.h
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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "input.h"
#include "bytecode.h"
#include "bcoutput.h"

void parParseUntil (inputReader *ir, const char *delimiters, bytecodeWriter *bcw, int consume,
		    environment *env, environment *globalEffects);

bytecode* parParseIntoBCUntil (inputReader *ir, const char *delimiters, int consume,
			       environment *env, environment *globalEffects);

#endif
