/* -*- c -*- */

/*
 * input.h
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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "dynstring.h"

typedef struct
{
    int (*nextChar) (void*);
    int (*isAtEnd) (void*);
    void *state;
    int thisChar;
} inputReader;

#define BUFFER_LENGTH    8192

typedef struct
{
    char inputBuffer[BUFFER_LENGTH];
    int bufferUsed;
    int bufferPos;
} statePreprocessor;

typedef struct
{
    dynstring *ds;
    int pos;
} stateDynstring;

#define NEXT_CHAR(ir)      ((ir)->thisChar = (ir)->nextChar((ir)->state))
#define THIS_CHAR(ir)      ((ir)->thisChar)
#define IS_AT_END(ir)      ((ir)->isAtEnd((ir)->state))

inputReader irNewPreprocessor (void);
const char* irPreprocessorFileName (inputReader *ir);
int irPreprocessorLineNumber (inputReader *ir);

inputReader irNewDynstring (dynstring *ds, int pos);

#endif
