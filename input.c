/* -*- c -*- */

/*
 * input.c
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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "recorder.h"

#include "input.h"

int fillBuffer (char *buffer, int max);

int
preprocessorNextChar (void *info)
{
    statePreprocessor *state = (statePreprocessor*)info;

    assert(state->bufferPos < state->bufferUsed);

    if (++state->bufferPos == state->bufferUsed)
    {
	do
	{
	    state->bufferUsed = fillBuffer(state->inputBuffer, BUFFER_LENGTH);
	} while (state->bufferUsed == 0);
	if (state->bufferUsed == -1)
	    state->bufferUsed = 0;
	state->bufferPos = 0;
    }

    if (state->bufferPos == state->bufferUsed)
	return EOF;

    while (state->inputBuffer[state->bufferPos] == '\\')
    {
	if (state->bufferPos + 2 >= state->bufferUsed)
	{
	    if (state->bufferPos + 1 >= state->bufferUsed
		|| state->inputBuffer[state->bufferPos + 1] == '\\')
	    {
		memmove(state->inputBuffer, state->inputBuffer + state->bufferPos,
			state->bufferUsed - state->bufferPos);
		state->bufferUsed -= state->bufferPos;

		do
		{
		    int result = fillBuffer(state->inputBuffer + state->bufferUsed,
					    BUFFER_LENGTH - state->bufferUsed);
		    if (result == -1)
			break;
		} while (state->bufferUsed < 3);
	    }
	}

	if (state->inputBuffer[state->bufferPos + 1] == '\n')
	{
	    state->bufferPos += 2;

	    do
	    {
		if (state->bufferPos >= state->bufferUsed)
		{
		    state->bufferPos = 0;
		    do
		    {
			state->bufferUsed = fillBuffer(state->inputBuffer, BUFFER_LENGTH);
		    } while (state->bufferUsed == 0);
		    if (state->bufferUsed == -1)
			state->bufferUsed = 0;
		}
		if (state->bufferPos == state->bufferUsed)
		    return EOF;
		while (state->bufferPos < state->bufferUsed)
		{
		    if (state->inputBuffer[state->bufferPos] == ' '
			|| state->inputBuffer[state->bufferPos] == '\t')
			++state->bufferPos;
		    else
			break;
		}
	    } while (state->bufferPos >= state->bufferUsed);
	}
	else if (state->inputBuffer[state->bufferPos + 1] == '\\'
		 && state->inputBuffer[state->bufferPos + 2] == '\n')
	    ++state->bufferPos;
	else
	    break;
    }

    return state->inputBuffer[state->bufferPos];
}

int
preprocessorIsAtEnd (void *info)
{
    statePreprocessor *state = (statePreprocessor*)info;

    return state->bufferPos == state->bufferUsed;
}

int
dynstringNextChar (void *info)
{
    stateDynstring *state = (stateDynstring*)info;

    assert(state->pos <= state->ds->length);

    if (++state->pos == state->ds->length)
	return EOF;

    return state->ds->data[state->pos];
}

int
dynstringIsAtEnd (void *info)
{
    stateDynstring *state = (stateDynstring*)info;

    return state->pos == state->ds->length;
}

inputReader
irNewPreprocessor (void)
{
    inputReader ir;
    statePreprocessor *state = (statePreprocessor*)memXAlloc(sizeof(statePreprocessor));

    ir.nextChar = preprocessorNextChar;
    ir.isAtEnd = preprocessorIsAtEnd;

    state->bufferUsed = 0;
    state->bufferPos = -1;

    ir.state = state;

    return ir;
}

const char*
irPreprocessorFileName (inputReader *ir)
{
    statePreprocessor *state = (statePreprocessor*)ir->state;

    return unRecordFileName(state->bufferPos);
}

int
irPreprocessorLineNumber (inputReader *ir)
{
    statePreprocessor *state = (statePreprocessor*)ir->state;

    return unRecordLineNumber(state->bufferPos);
}

inputReader
irNewDynstring (dynstring *ds, int pos)
{
    inputReader ir;
    stateDynstring *state = (stateDynstring*)memXAlloc(sizeof(stateDynstring));

    ir.nextChar = dynstringNextChar;
    ir.isAtEnd = dynstringIsAtEnd;

    state->ds = ds;
    state->pos = pos - 1;

    ir.state = state;

    return ir;
}
