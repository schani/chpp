/* -*- c -*- */

/*
 * output.h
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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <stdio.h>

#include "dynstring.h"
#include "value.h"

typedef struct _outputWriter
{
    void (*outChar) (void*, char);
    void (*outString) (void*, const char*, int);
    void (*outValue) (void*, value*, int);
    int enabled;
    void *state;
} outputWriter;

typedef struct
{
    value *theValue;
    int byReference;
    int needCopy;
} outputStateValue;

outputWriter owNewDummy (void);

outputWriter owNewDynstring (dynstring *ds);

outputWriter owNewFile (FILE *file);

outputWriter owNewValue (int byReference);
value* owValueValue (outputWriter *ow);
int owValueNeedCopy (outputWriter *ow);

#define OUT_CHAR(ow,c)         { if ((ow)->enabled) ((ow)->outChar((ow)->state,(c))); }
#define OUT_STRING(ow,s,l)     { if ((ow)->enabled) ((ow)->outString((ow)->state,(s),(l))); }
#define OUT_VALUE(ow,v)        { if ((ow)->enabled) ((ow)->outValue((ow)->state,(v),0)); }
#define OUT_VALUE_REF(ow,v)    { if ((ow)->enabled) ((ow)->outValue((ow)->state,(v),1)); }

#endif
