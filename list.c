/* -*- c -*- */

/*
 * list.c
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

#include <string.h>

#include "memory.h"

#include "list.h"

static listElement*
allocListElement (list *lst)
{
    return (listElement*)memXAlloc(lst->elementAllocSize);
}

list*
listNewList (list *lst, int elementSize)
{
    lst->first = lst->last = 0;
    lst->elementSize = elementSize;
    if (elementSize <= sizeof(int))
	lst->elementAllocSize = sizeof(listElement);
    else
	lst->elementAllocSize = sizeof(listElement) + elementSize - sizeof(int);
    lst->length = 0;
    lst->pointerType = 0;

    return lst;
}

list*
listNewPointerList (list *lst)
{
    lst->first = lst->last = 0;
    lst->elementSize = sizeof(void*);
    if (lst->elementSize <= sizeof(int))
	lst->elementAllocSize = sizeof(listElement);
    else
	lst->elementAllocSize = sizeof(listElement) + lst->elementSize - sizeof(int);
    lst->length = 0;
    lst->pointerType = 1;

    return lst;
}

int
listLength (list *lst)
{
    return lst->length;
}

listElement*
listAppend (list *lst, void *data)
{
    listElement *elem = allocListElement(lst);

    listSetElementData(lst, elem, data);

    elem->prev = lst->last;
    elem->next = 0;

    if (lst->last == 0)
	lst->first = lst->last = elem;
    else
	lst->last = lst->last->next = elem;

    ++lst->length;

    return elem;
}

listElement*
listPrepend (list *lst, void *data)
{
    listElement *elem = allocListElement(lst);

    listSetElementData(lst, elem, data);

    elem->prev = 0;
    elem->next = lst->first;

    if (lst->first == 0)
	lst->first = lst->last = elem;
    else
	lst->first = lst->first->prev = elem;

    ++lst->length;

    return elem;
}

listElement*
listFirstElement (list *lst)
{
    return lst->first;
}

listElement*
listLastElement (list *lst)
{
    return lst->last;
}

listElement*
listNThElement (list *lst, int n)
{
    listElement *elem;
    int i;

    if (n < 0 || n >= lst->length)
	return 0;

    elem = lst->first;
    for (i = 0; i < n; ++i)
	elem = elem->next;

    return elem;
}

listElement*
listNextElement (listElement *elem)
{
    return elem->next;
}

listElement*
listPreviousElement (listElement *elem)
{
    return elem->prev;
}

listElement*
listInsertBeforeElement (list *lst, listElement *elem, void *data)
{
    listElement *newElem = allocListElement(lst);

    listSetElementData(lst, newElem, data);

    newElem->prev = elem->prev;
    newElem->next = elem;

    if (elem->prev != 0)
	elem->prev->next = newElem;
    else
	lst->first = newElem;

    elem->prev = newElem;

    ++lst->length;

    return newElem;
}

listElement*
listInsertAfterElement (list *lst, listElement *elem, void *data)
{
    listElement *newElem = allocListElement(lst);

    listSetElementData(lst, newElem, data);

    newElem->prev = elem;
    newElem->next = elem->next;

    if (elem->next != 0)
	elem->next->prev = newElem;
    else
	lst->last = newElem;

    elem->next = newElem;

    ++lst->length;

    return newElem;
}

void
listRemoveElement (list *lst, listElement *elem)
{
    if (elem->prev != 0)
	elem->prev->next = elem->next;
    else
	lst->first = elem->next;

    if (elem->next != 0)
	elem->next->prev = elem->prev;
    else
	lst->last = elem->prev;

    --lst->length;
}

void*
listElementData (list *lst, listElement *elem)
{
    if (lst->pointerType)
	return *(void**)&elem->data;
    else
	return (void*)&elem->data;
}

void*
listNThElementData (list *lst, int n)
{
    return listElementData(lst, listNThElement(lst, n));
}

void
listSetElementData (list *lst, listElement *elem, void *data)
{
    if (lst->pointerType)
	memcpy(&elem->data, &data, lst->elementSize);
    else
	memcpy(&elem->data, data, lst->elementSize);
}
