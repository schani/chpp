/* -*- c -*- */

/*
 * dynstring.h
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

#ifndef __DYNSTRING_H__
#define __DYNSTRING_H__

typedef struct _dynstring
{
    char *data;
    int length;
    int allocated;
} dynstring;

void initDynstring (void);

dynstring dsNew (void);
dynstring dsNewFrom (const char *s);
dynstring dsNewFromBytes (const char *s, int num);
dynstring dsEmpty (void);

dynstring dsCopy (dynstring *ds);

void dsAppendString (dynstring *ds, const char *s, int num);
void dsAppendChar (dynstring *ds, char c);

void dsShrinkFront (dynstring *ds, int num);
void dsShrinkRear (dynstring *ds, int num);

void dsRemoveOuterWS (dynstring *ds);
void dsRemoveFrontWS (dynstring *ds);
void dsRemoveRearWS (dynstring *ds);

dynstring dsSplitWS (dynstring *ds);

#endif
