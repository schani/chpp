/* -*- c -*- */

/*
 * avl.c
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

#include "memory.h"

#include "avl.h"

int
avlCompare (avlTree *pcolCollect, void* key, unsigned int rank,
		     avlNode *pavlnNode)
{
    if (pcolCollect->isSorted)
	return pcolCollect->compare(key, pavlnNode->key);
    if (rank < pavlnNode->rank)
	return -1;
    if (rank == pavlnNode->rank)
	return 0;
    return 1;
}

int
avlBalance (avlNode *pavlnS, int iA, avlNode **phead)
{
    avlNode *pavlnR,
	*pavlnP,
	*pavlnT;

    *phead = 0;
    pavlnT = pavlnS->up;
    pavlnR = AVL_LINK(iA, pavlnS);                      /* R = LINK(a,S) */
    if (pavlnR->balance == 0 || pavlnR->balance == iA)  /* single rotation */
    {
	pavlnR->up = pavlnT;                                 /* UP(R) = T */
	pavlnS->up = pavlnR;                                 /* UP(S) = R */
	if (pavlnT)
	{
	    if (pavlnT->left == pavlnS)
		pavlnT->left = pavlnR;                         /* LEFT(T) = R */
	    else                
		pavlnT->right = pavlnR;                       /* RIGHT(T) = R */
	}
	else
	    *phead = pavlnR;
	if (AVL_LINK(-iA, pavlnR))
	    AVL_LINK(-iA, pavlnR)->up = pavlnS;   /* UP(LINK(-a,R)) = S */
	pavlnP = pavlnR;
	AVL_LINK_SET(iA, pavlnS, AVL_LINK(-iA, pavlnR));
	AVL_LINK_SET(-iA, pavlnR, pavlnS);
	if (iA == 1)
	    pavlnR->rank += pavlnS->rank;
	else
	    pavlnS->rank -= pavlnR->rank;
	if (pavlnR->balance == iA)
	{
	    pavlnS->balance = pavlnR->balance = 0;
	    return iA;
	}
	pavlnR->balance = -iA;
	return 0;
    }

    /* double rotation */
    pavlnP = AVL_LINK(-iA, pavlnR);
    pavlnP->up = pavlnT;                                   /* UP(P) = T */
    pavlnS->up = pavlnR->up = pavlnP;         /* UP(S) = UP(R) = P */
    if (pavlnT)
    {
	if (pavlnT->left == pavlnS)
	    pavlnT->left = pavlnP;                           /* LEFT(T) = R */
	else                
	    pavlnT->right = pavlnP;                         /* RIGHT(T) = R */
    }
    else
	*phead = pavlnP;
    if (AVL_LINK(iA, pavlnP))
	AVL_LINK(iA, pavlnP)->up = pavlnR;       /* UP(LINK(a,P)) = R */
    if (AVL_LINK(-iA, pavlnP))
	AVL_LINK(-iA, pavlnP)->up = pavlnS;     /* UP(LINK(-a,P)) = S */
    AVL_LINK_SET(-iA, pavlnR, AVL_LINK(iA, pavlnP));
    AVL_LINK_SET(iA, pavlnP, pavlnR);
    AVL_LINK_SET(iA, pavlnS, AVL_LINK(-iA, pavlnP));
    AVL_LINK_SET(-iA, pavlnP, pavlnS);
    if (pavlnP->balance == iA)
    {
	pavlnS->balance = -iA;
	pavlnR->balance = 0;
    }
    else
	if (pavlnP->balance == -iA)
	{
	    pavlnS->balance = 0;
	    pavlnR->balance = iA;
	}
	else
	    pavlnS->balance = pavlnR->balance = 0;
    pavlnP->balance = 0;
    if (iA == 1)
    {
	pavlnR->rank -= pavlnP->rank;
	pavlnP->rank += pavlnS->rank;
    }
    else
    {
	pavlnP->rank += pavlnR->rank;
	pavlnS->rank -= pavlnP->rank;
    }
    return -iA;
}

avlTree*
avlNew (void)
{
    avlTree *pcolReturnVar;

    pcolReturnVar = memXAlloc(sizeof(avlTree));
    pcolReturnVar->head = 0;
    pcolReturnVar->numElements = 0;
    pcolReturnVar->isSorted = 0;
    return pcolReturnVar;
}

avlTree*
avlNewSorted (int (*compare) (void*,void*))
{
    avlTree *pcolReturnVar;

    pcolReturnVar = memXAlloc(sizeof(avlTree));
    pcolReturnVar->head = 0;
    pcolReturnVar->numElements = 0;
    pcolReturnVar->isSorted = 1;
    pcolReturnVar->compare = compare;
    return pcolReturnVar;
}

void
avlInsert (avlTree *pcolCollect, void* key, unsigned int uiPos)
{
    avlNode *pavlnNode,
	*pavlnS,
	*pavlnP,
	*pavlnR;
    unsigned int uiU,
	uiM;
    int iA,
	iResult;
    int done = 0;

    pcolCollect->numElements++;
    pavlnNode = memXAlloc(sizeof(avlNode));
    pavlnNode->key = key;
    pavlnNode->left = 0;
    pavlnNode->right = 0;
    pavlnNode->balance = 0;
    pavlnNode->rank = 1;
    if (pcolCollect->head == 0)
    {
	pavlnNode->up = 0;
	pcolCollect->head = pavlnNode;
    }
    else
    {
	uiU = uiM = uiPos;
	pavlnS = pavlnP = pcolCollect->head;
	do
	{
	    if (avlCompare(pcolCollect, key, uiM, pavlnP) < 0)
	    { /* move left */
		pavlnP->rank++;
		pavlnR = pavlnP->left;
		if (pavlnR == 0)
		{
		    pavlnP->left = pavlnNode;
		    pavlnNode->up = pavlnP;
		    done = 1;
		}
		else
		    if (pavlnR->balance != 0)
		    {
			pavlnS = pavlnR;
			uiU = uiM;
		    }
		pavlnP = pavlnR;
	    }
	    else
	    { /* move right */
		uiM -= pavlnP->rank;
		pavlnR = pavlnP->right;
		if (pavlnR == 0)
		{
		    pavlnP->right = pavlnNode;
		    pavlnNode->up = pavlnP;
		    done = 1;
		}
		else
		    if (pavlnR->balance != 0)
		    {
			pavlnS = pavlnR;
			uiU = uiM;
		    }
		pavlnP = pavlnR;
	    }
	} while (!done);
	uiM = uiU;
	if (avlCompare(pcolCollect, key, uiM, pavlnS) < 0)
	    pavlnR = pavlnP = pavlnS->left;
	else
	{
	    pavlnR = pavlnP = pavlnS->right;
	    uiM -= pavlnS->rank;
	}
	while (pavlnP != pavlnNode)
	{
	    iResult = avlCompare(pcolCollect, key, uiM, pavlnP);
	    if (iResult < 0)
	    {
		pavlnP->balance = -1;
		pavlnP = pavlnP->left;
	    }
	    else
	    {
		pavlnP->balance = 1;
		uiM -= pavlnP->rank;
		pavlnP = pavlnP->right;
	    }
	}
	/* balancing act */
	if (avlCompare(pcolCollect, key, uiU, pavlnS) < 0)
	    iA = -1;
	else
	    iA = 1;
	if (pavlnS->balance == 0)
	{
	    pavlnS->balance = iA;
	    return;
	}
	if (pavlnS->balance == -iA)
	{
	    pavlnS->balance = 0;
	    return;
	}
	/* we need to rebalance our tree */
	avlBalance(pavlnS, iA, &pavlnR);
	if (pavlnR)
	    pcolCollect->head = pavlnR;
    }
}

void
avlPut (avlTree *pcolCollect, void* key)
{
    avlInsert(pcolCollect, key, pcolCollect->numElements + 1);
}

avlNode*
avlFirst (avlTree *pcolCollect, unsigned int uiPos)
{

    avlNode *pavlnNode;

    if (pcolCollect->head == 0)
	return 0;
    pavlnNode = pcolCollect->head;
    while (uiPos != pavlnNode->rank)
    {
	if (uiPos < pavlnNode->rank)
	    pavlnNode = pavlnNode->left;
	else
	{
	    uiPos -= pavlnNode->rank;
	    pavlnNode = pavlnNode->right;
	}
    }
    return pavlnNode;
}

avlNode*
avlSearch (avlTree *pcolCollect, void* key)
{

    avlNode *pavlnNode,
	*pavlnLast = 0;
    int           iResult;

    if (!pcolCollect->isSorted)
	return 0;
    if (pcolCollect->head == 0)
	return 0;
    pavlnNode = pcolCollect->head;
    iResult = pcolCollect->compare(key, pavlnNode->key);
    while (pavlnNode)
    {
	if (iResult == 0)
	    pavlnLast = pavlnNode;
	if (iResult <= 0)
	    pavlnNode = pavlnNode->left;
	else
	    pavlnNode = pavlnNode->right;
	if (pavlnNode)
	    iResult = pcolCollect->compare(key, pavlnNode->key);
    }
    return pavlnLast;
}

unsigned int
avlGetPos (avlTree *pcolCollect, void* key)
{

    avlNode *pavlnNode;
    unsigned int          uiReturnVar = 1,
	uiLastPos   = 0;
    int           iResult;

    if (!pcolCollect->isSorted)
	return 0;
    if (pcolCollect->head == 0)
	return 0;
    pavlnNode = pcolCollect->head;
    iResult = pcolCollect->compare(key, pavlnNode->key);
    while (pavlnNode)
    {
	if (iResult == 0)
	    uiLastPos = uiReturnVar;
	if (iResult <= 0)
	    pavlnNode = pavlnNode->left;
	else
	{
	    uiReturnVar += pavlnNode->rank;
	    pavlnNode = pavlnNode->right;
	}
	if (pavlnNode)
	    iResult = pcolCollect->compare(key, pavlnNode->key);
    }
    return uiLastPos;
}

avlNode*
avlNext (avlNode *pavlnNode)
{
    if (pavlnNode->right)
    {
	pavlnNode = pavlnNode->right;
	while (pavlnNode->left)
	    pavlnNode = pavlnNode->left;
	return pavlnNode;
    }
    do
    {
	if (pavlnNode->up)
	{
	    if (pavlnNode->up->left == pavlnNode)
		return pavlnNode->up;
	    else
		pavlnNode = pavlnNode->up;
	}
	else
	    return 0;
    } while (1);
}

avlNode*
avlPrevious (avlNode *pavlnNode)
{
    if (pavlnNode->left)
    {
	pavlnNode = pavlnNode->left;
	while (pavlnNode->right)
	    pavlnNode = pavlnNode->right;
	return pavlnNode;
    }
    do
    {
	if (pavlnNode->up)
	{
	    if (pavlnNode->up->right == pavlnNode)
		return pavlnNode->up;
	    else
		pavlnNode = pavlnNode->up;
	}
	else
	    return 0;
    } while (1);
}

void
avlDeleteNode (avlTree *pcolCollect,
			 avlNode *pavlnNode, int iA)
{
    int balancing = 1;
    avlNode *pavlnTemp;

    if (!pavlnNode->up)                            /* deleting the root */
    {
	pcolCollect->head = AVL_LINK(iA, pavlnNode);
	if (pcolCollect->head)
	    pcolCollect->head->up = 0;
	memFree(pavlnNode);
	return;
    }
    pavlnTemp = pavlnNode;
    if (pavlnNode->up->left == pavlnNode)
    {
	pavlnNode->up->left = AVL_LINK(iA, pavlnNode);
	iA = -1;
    }
    else
    {
	pavlnNode->up->right = AVL_LINK(iA, pavlnNode);
	iA = 1;
    }
    pavlnNode = pavlnNode->up;
    if (AVL_LINK(iA, pavlnNode))
	AVL_LINK(iA, pavlnNode)->up = pavlnNode;
    memFree(pavlnTemp);
    do
    {
	if (iA == -1)
	    pavlnNode->rank--;                                  /* adjust rank */
	if (balancing)
	{                                                    /* adjust balance */
	    if (pavlnNode->balance == 0)
	    {
		pavlnNode->balance = -iA;
		balancing = 0;
	    }
	    else
		if (pavlnNode->balance == iA)
		    pavlnNode->balance = 0;
		else
		{
		    /* rebalancing is necessary */
		    if (!avlBalance(pavlnNode, -iA, &pavlnTemp))
			balancing = 0;
		    if (pavlnTemp)
			pcolCollect->head = pavlnTemp;
		    pavlnNode = pavlnNode->up;
		}
	}
	if (pavlnNode->up)                             /* go up one level */
	{
	    if (pavlnNode->up->left == pavlnNode)
		iA = -1;
	    else
		iA = 1;
	    pavlnNode = pavlnNode->up;
	}
	else
	    pavlnNode = 0;
    } while (pavlnNode);
}

void*
avlDelete (avlTree *pcolCollect, unsigned int uiPos)
{
    avlNode *pavlnNode,
	*pavlnNext;
    void *key,
	*ulReturnVar;

    pcolCollect->numElements--;
    pavlnNode = avlFirst(pcolCollect, uiPos);
    ulReturnVar = pavlnNode->key;
    pavlnNext = pavlnNode->right;
    if (pavlnNext)
    {
	for (; pavlnNext->left; pavlnNext = pavlnNext->left)
	    ;
	key = pavlnNext->key;
	avlDeleteNode(pcolCollect, pavlnNext, 1);
	pavlnNode->key = key;
    }
    else
	avlDeleteNode(pcolCollect, pavlnNode, -1);
    return ulReturnVar;
}

void*
avlKey (avlNode *pavlnNode)
{
    return pavlnNode->key;
}

void*
avlGet (avlTree *pcolCollect, unsigned int uiPos)
{
    return avlFirst(pcolCollect, uiPos)->key;
}

unsigned int
avlNumElements (avlTree *pcolCollect)
{
    return pcolCollect->numElements;
}

int
avlIsEmpty (avlTree *pcolCollect)
{
    return avlNumElements(pcolCollect) == 0;
}

int
avlBalanced (avlNode *pavlnNode, unsigned int *puiHeight,
		      unsigned int *pnumElements)
{
    unsigned int uiHeightL,
	uiHeightR,
	numElementsR;

    if (!pavlnNode)
    {
	*puiHeight = 0;
	*pnumElements = 0;
	return 1;
    }
    if (pavlnNode->left)
    {
	if (pavlnNode->left->up != pavlnNode)
	    return 0;
	if (!avlBalanced(pavlnNode->left, &uiHeightL, pnumElements))
	    return 0;
	if (pavlnNode->rank != *pnumElements + 1)
	    return 0;
    }
    else
    {
	if (pavlnNode->rank != 1)
	    return 0;
	uiHeightL = 0;
	*pnumElements = 0;
    }
    if (pavlnNode->right)
    {
	if (pavlnNode->right->up != pavlnNode)
	    return 0;
	if (!avlBalanced(pavlnNode->right, &uiHeightR, &numElementsR))
	    return 0;
    }
    else
    {
	numElementsR = 0;
	uiHeightR = 0;
    }
    if ((int)uiHeightR - (int)uiHeightL != pavlnNode->balance)
	return 0;
    if (uiHeightR > uiHeightL)
	*puiHeight = uiHeightR + 1;
    else
	*puiHeight = uiHeightL + 1;
    *pnumElements += numElementsR + 1;
    return 1;
}

#ifdef _AVL_TEST

#include <ctype.h>
#include <string.h>
#include <stdio.h>

int
compare_strings (void* ulString1, void* ulString2)
{
    return strcmp((const char*)ulString1, (const char*)ulString2);
}

void
main (void)
{

    avlTree  *pcolCollect;
    avlNode *pavlnCounter;
    unsigned int uiHeight,
	uiCounter,
	uiIndex,
	numElements;
    FILE *pfileFile;
    char acBuffer[128],
	*pcString;

    pcolCollect = avlNewSorted(compare_strings);
    /*  pcolCollect = avlNew();*/
    pfileFile = fopen("collect.c", "r");
    uiCounter = 0;
    do
    {
	if (fgets(acBuffer, 128, pfileFile))
	{
	    for (uiIndex = 0; isspace(acBuffer[uiIndex]); uiIndex++)
		;
	    pcString = malloc(strlen(acBuffer + uiIndex) + 1);
	    strcpy(pcString, acBuffer + uiIndex);
	    avlPut(pcolCollect, (void*)pcString);
	    /*      uiCounter++;
		    if (!avlBalanced(pcolCollect->head, &uiHeight, &numElements))
		    {
		    printf("Error: Tree no longer balanced after %uth insertion\n", uiCounter);
		    _getch();
		    return;
		    }*/
	}
	else
	    break;
    } while (1);
    fclose(pfileFile);

    for (uiCounter = 0; uiCounter < 11; uiCounter++)
    {
	avlDelete(pcolCollect, 13);
	if (!avlBalanced(pcolCollect->head, &uiHeight, &numElements))
	{
	    printf("Error: Tree no longer balanced!\n");
	    return;
	}
	printf("Height: %u\n", uiHeight);
    }

    pavlnCounter = avlFirst(pcolCollect, 1);
    uiCounter = 0;
    for (; pavlnCounter; pavlnCounter = avlNext(pavlnCounter))
    {
	if (!pavlnCounter)
	    break;
	/*    if (uiCounter++ == 24)
	      break; */
	printf("%s", (char*)avlKey(pavlnCounter));
    }

    avlFree(pcolCollect);
}

#endif
