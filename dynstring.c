/* -*- c -*- */

/*
 * dynstring.c
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include "memory.h"

#include "dynstring.h"

#define ASSURE_LENGTH(ds,s)       if ((s) >= (ds)->allocated) \
                                      (ds)->data = reallocString((ds)->data, (s) + 1, \
                                                                 &(ds)->allocated)

#define DBG(s)           

char*
allocString (int minSize, int *exponent)
{
    *exponent = minSize;

    return memXAllocAtomic(minSize);
}

char*
reallocString (char *string, int minSize, int *exponent)
{
    *exponent = minSize * 2;

    return memXRealloc(string, *exponent);
}

dynstring
dsNew (void)
{
    dynstring ds;

    ds.data = allocString(1, &ds.allocated);
    ds.data[0] = '\0';
    ds.length = 0;

    return ds;
}

dynstring
dsNewFrom (const char *s)
{
    dynstring ds;
    int length = strlen(s);

    ds.data = allocString(length + 1, &ds.allocated);
    strcpy(ds.data, s);
    ds.length = length;

    return ds;
}

dynstring
dsNewFromBytes (const char *s, int num)
{
    dynstring ds;

    ds.data = allocString(num + 1, &ds.allocated);
    memcpy(ds.data, s, num);
    ds.data[num] = 0;
    ds.length = num;

    return ds;
}

dynstring
dsEmpty (void)
{
    static dynstring ds;

    if (ds.data == 0)
	ds = dsNew();

    return ds;
}

dynstring
dsCopy (dynstring *dsOrig)
{
    dynstring ds;

    assert(dsOrig->data != 0);

    ds.data = allocString(dsOrig->length + 1, &ds.allocated);
    strcpy(ds.data, dsOrig->data);
    ds.length = dsOrig->length;

    return ds;
}

void
dsAppendString (dynstring *ds, const char *s, int num)
{
    assert(ds->data != 0);

    ASSURE_LENGTH(ds, ds->length + num);

    strncpy(ds->data + ds->length, s, num);
    ds->length += num;
    ds->data[ds->length] = '\0';
}

void
dsAppendChar (dynstring *ds, char c)
{
    assert(ds->data != 0);

    ASSURE_LENGTH(ds, ds->length + 1);

    ds->data[ds->length] = c;
    ds->data[++ds->length] = '\0';
}

void
dsShrinkFront (dynstring *ds, int num)
{
    assert(ds->data != 0);

    if (num > ds->length)
	num = ds->length;

    memmove(ds->data, ds->data + num, ds->length + 1 - num);
    ds->length -= num;
}

void
dsShrinkRear (dynstring *ds, int num)
{
    assert(ds->data != 0);

    if (num > ds->length)
	num = ds->length;

    ds->length -= num;
    ds->data[ds->length] = '\0';
}

void
dsSplitPath (dynstring *path, dynstring *dir, dynstring *file)
{
    char *lastSlash = strrchr(path->data, '/');

    if (lastSlash != 0)
    {
	*dir = dsNewFromBytes(path->data, lastSlash - path->data);
	*file = dsNewFrom(lastSlash + 1);
    }
    else
    {
	*dir = dsNewFrom("");
	*file = dsCopy(path);
    }
}

void
dsRemoveRearWS (dynstring *ds)
{
    int i = ds->length;

    assert(ds->data != 0);

    if( !ds->length ) return;

    while( (i > 0) && isspace((int)(ds->data[i-1])) )
    {
	ds->data[i-1] = 0;
	i--;
	ds->length--;
    }
}

void
dsRemoveFrontWS (dynstring *ds) 
{
    int i = 0;

    while( isspace((int)(ds->data[i])) ) {
	i++;
    }

    if( i ) dsShrinkFront( ds, i );
}

void
dsRemoveOuterWS (dynstring *ds) 
{
    dsRemoveRearWS( ds );
    dsRemoveFrontWS( ds );
}

dynstring
dsSplitWS (dynstring *ds) 
{
    dynstring ds2;
    int i = 0;

    assert(ds->data != 0);

    while( i < (ds->length-1) && ds->data[i] != ' ' && ds->data[i] != '\t' ) i++;
    if( i == ds->length - 1 ) return ds2 = dsNewFrom( "" );

    ds2 = dsNewFrom( &ds->data[i+1] );
    dsShrinkRear( ds, ds->length - i );

    return ds2;
}
