/* -*- c -*- */

/*
 * list.h
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

#ifndef __LIST_H__
#define __LIST_H__

typedef struct _listElement
{
    struct _listElement *prev;
    struct _listElement *next;

    int data[1];
} listElement;

typedef struct
{
    listElement *first;
    listElement *last;

    int pointerType;
    int elementSize;
    int elementAllocSize;
    int length;
} list;

list* listNewList (list *lst, int elementSize);
list* listNewPointerList (list *lst);

int listLength (list *lst);

listElement* listAppend (list *lst, void *data);
listElement* listPrepend (list *lst, void *data);

listElement* listFirstElement (list *lst);
listElement* listLastElement (list *lst);
listElement* listNThElement (list *lst, int n);
listElement* listNextElement (listElement *elem);
listElement* listPreviousElement (listElement *elem);

listElement* listInsertBeforeElement (list *lst, listElement *elem, void *data);
listElement* listInsertAfterElement (list *lst, listElement *elem, void *data);
void listRemoveElement (list *lst, listElement *elem);

void* listElementData (list *lst, listElement *elem);
void* listNThElementData (list *lst, int n);
void listSetElementData (list *lst, listElement *elem, void *data);

#endif
