/* -*- c -*- */

/*
 * avl.h
 *
 * chpp
 *
 * Copyright (C) 1994-1998 Mark Probst
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

#ifndef __AVL_H__
#define __AVL_H__

typedef struct _avlNode
{
  void *key;
  int balance;
  unsigned int rank;
  struct _avlNode *up;
  struct _avlNode *left;
  struct _avlNode *right;
} avlNode;

typedef struct
{
  avlNode *head;
  unsigned int numElements;
  int isSorted;
  int (*compare) (void*, void*);
} avlTree;

#define AVL_LINK(a,N)            (((a) == -1) ? (N)->left : (N)->right)
#define AVL_LINK_SET(s,P,V)      ((((s) == -1) ? (P)->left : (P)->right) = (V))

int avlCompare (avlTree*, void*, unsigned int, avlNode*);
int avlBalance (avlNode*, int, avlNode**);
avlTree* avlNew (void);
avlTree* avlNewSorted (int (*) (void*, void*));
void avlInsert (avlTree*, void*, unsigned int);
void avlPut (avlTree*, void*);
avlNode* avlFirst (avlTree*, unsigned int);
avlNode* avlSearch (avlTree*, void*);
unsigned int avlGetPos (avlTree*, void*);
avlNode* avlNext (avlNode*);
avlNode* avlPrevious (avlNode*);
void avlDeleteNode (avlTree*, avlNode*, int);
void* avlDelete (avlTree*, unsigned int);
void* avlKey (avlNode*);
void* avlGet (avlTree*, unsigned int);
unsigned int avlNumElements (avlTree*);
int avlIsEmpty (avlTree*);
int avlBalanced (avlNode*, unsigned int*, unsigned int*);

#endif
