/* -*- c -*- */

/*
 * depends.c
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

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

#include "depends.h"

extern dynstring outputFileName;

dependencyTarget *firstTarget = 0,
    *lastTarget = 0;

void
addDependency (dynstring *targetName, dynstring *dependencyName)
{
    dependencyTarget *target;
    dependencyEntry *entry;

    if (targetName == 0)
	targetName = &outputFileName;

    for (target = firstTarget; target != 0; target = target->next)
	if (strcmp(targetName->data, target->name.data) == 0)
	    break;

    if (target == 0)
    {
	target = (dependencyTarget*)memXAlloc(sizeof(dependencyTarget));
	if (lastTarget != 0)
	    lastTarget->next = target;
	else
	    firstTarget = target;
	lastTarget = target;
	
	target->name = dsCopy(targetName);
	target->firstEntry = 0;
	target->next = 0;
    }

    entry = (dependencyEntry*)memXAlloc(sizeof(dependencyEntry));
    entry->name = dsCopy(dependencyName);
    entry->next = target->firstEntry;
    target->firstEntry = entry;
}

void
outputDependencies (void)
{
    dependencyTarget *target;

    for (target = firstTarget; target != 0; target = target->next)
    {
	dependencyEntry *entry;

	printf("%s : ", target->name.data);

	for (entry = target->firstEntry; entry != 0; entry = entry->next)
	    printf("%s ", entry->name.data);
	printf("\n\n");
    }
}
