/* -*- c -*- */

/*
 * bcoutput.h
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

#ifndef __BCOUTPUT_H__
#define __BCOUTPUT_H__

#include "environment.h"
#include "output.h"
#include "bytecode.h"

typedef struct
{
    void *state;

    void (*outChar) (void*, char);
    void (*outString) (void*, const char*, int);
    void (*outAssignment) (void*, int, bytecode*, bcSubscript*, bytecode*);
    void (*outMacro) (void*, int, bytecode*, bcSubscript*, int, bcArgument*);
    void (*outBeginQuote) (void*);
    void (*outEndQuote) (void*);
    void (*outArithmetic) (void*, bytecode*);
    void (*outEvaluation) (void*, bytecode*);
} bytecodeWriter;

typedef struct
{
    bytecode *first;
    bytecode *last;
} bytecodeStateBytecode;

typedef struct
{
    environment *env;
    outputWriter *ow;
} bytecodeStateOutput;

#define BCW_OUT_CHAR(bcw,c)             ((bcw)->outChar((bcw)->state,(c)))
#define BCW_OUT_STRING(bcw,s,l)         ((bcw)->outString((bcw)->state,(s),(l)))
#define BCW_OUT_ASSIGNMENT(bcw,m,v,s,n) ((bcw)->outAssignment((bcw)->state,(m),(v),(s),(n)))
#define BCW_OUT_MACRO(bcw,f,n,s,c,a)    ((bcw)->outMacro((bcw)->state,(f),(n),(s),(c),(a)))
#define BCW_OUT_BEGIN_QUOTE(bcw)        ((bcw)->outBeginQuote((bcw)->state))
#define BCW_OUT_END_QUOTE(bcw)          ((bcw)->outEndQuote((bcw)->state))
#define BCW_OUT_ARITHMETIC(bcw,b)       ((bcw)->outArithmetic((bcw)->state,(b)))
#define BCW_OUT_EVALUATION(bcw,b)       ((bcw)->outEvaluation((bcw)->state,(b)))

bytecodeWriter* bcwNewBytecode (void);
bytecode* bcwBytecodeBytecode (bytecodeWriter *bcw);

bytecodeWriter* bcwNewOutput (environment *env, outputWriter *ow);

#endif
