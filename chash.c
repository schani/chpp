/* -*- c -*- */

/*
 * chash.c
 *
 * chpp
 *
 * Copyright (C) 1995-1997 Herbert Poetzl
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
#include <stdlib.h>

#include "memory.h"

#include "chash.h"

#ifndef TRUE
#define	TRUE 	(0==0)
#endif
#ifndef FALSE
#define	FALSE	(1==0)
#endif

#define	DELETED	((char *)-1)


char   *_copy_string(const char *string)
{
	char *copy = NULL;
	if (string) {
	    int len = strlen(string);
	    copy = (char *)memXAllocAtomic(len+1);
	    if (copy) {
	    	memcpy(copy, string, len+1);
	    }
	}
	return copy;
}


long	_hashcode_ascii(const char *key)
{  
	long h, c;
	h = 0;
	while ((c = *key++)) 
	{
	    h += c + (c<<5) + (h<<3);
	}
	return h;
}


struct 	hash_entry *_hash_find(hash table, const char *key)
{
	long pos, hashcode;
	struct hash_entry *place, *start;
	hashcode = _hashcode_ascii(key);
	pos = (table->mask & hashcode);
	place = &(table->data[pos]);
	start = place;
	while (place->key != NULL) {
	    if (hashcode == place->hashcode) {
		if (place->key != DELETED) {
		    if (strcmp(key, place->key) == 0) {
			return place;
		    }
		}
	    }
	    place++;
	    if (place == table->wall) place = table->data;
	    if (place == start) break;
	}
	return NULL;
}


void	_hash_place(hash table, struct hash_entry *entry, int check)
{
	long pos;
	struct hash_entry *place;
	pos = (table->mask & entry->hashcode);
	place = &(table->data[pos]);
	while (place->key != NULL) {
	    if (place->key == DELETED) break;
	    if (check) {
	    	if (entry->hashcode == place->hashcode) {
		    if (strcmp(entry->key, place->key) == 0) {
			place->value = entry->value;
			return;
		    }
		}
	    }
	    place++;
	    if (place == table->wall) place = table->data;
	}
	table->entries++;
	memcpy(place, entry, sizeof(struct hash_entry));
	return;
}

int	_hash_grow(hash table)
{
	struct hash_control *new;
	new = hash_new(2<<table->sizelog);
	if (new) {
	    long i,n;
	    for (i=0, n=1<<table->sizelog; i<n; i++) {
		if (table->data[i].key != NULL)
		    _hash_place(new, &(table->data[i]), FALSE); 
	    }
	    memFree(table->data);
	    memcpy(table, new, sizeof(struct hash_control));
	    memFree(new);			
	    return (TRUE);
	}
	return (FALSE);
}

int	_hash_cramped(hash table)
{
	long bound;
	bound = 101*(1 << table->sizelog)/143;
	if (table->entries > bound) return (TRUE);	
	return (FALSE);
}


hash	hash_new(long capacity)
{
	struct hash_control *new;
	long n,s;
	new = (struct hash_control *)
	    memXAlloc(sizeof(struct hash_control));
	if (new) {
	    for (n=0,s=1; s<capacity; n++,s <<= 1);
	    new->sizelog = n;
	    new->mask = s-1;
	    new->memsize = sizeof(struct hash_entry)*s;
	    new->data = (struct hash_entry *)
		memXAlloc(new->memsize);
	    if (new->data) {
	    	new->wall = new->data+s;
		hash_empty(new);
	    }
	    else {
	    	memFree(new);
		new = NULL;
	    }
	}	
	return (hash)new;
}

void	hash_free(hash table)
{
	if (table) {
	    memFree(table->data);
	    memFree(table);
	}
	return;
}

void	hash_empty(hash table)
{
	table->entries = 0;
	memset(table->data, 0, table->memsize);
	return;
}


int	hash_inject(hash table, const char *key, void *value)
{
	if (table && key) {
	    struct hash_entry place;
	    if (_hash_cramped(table)) {
		_hash_grow(table);
	    }
	    place.key = (char *)key;
	    place.value = value;
	    place.hashcode = _hashcode_ascii(key);
	    _hash_place(table, &place, (FALSE));
	    return (TRUE);
	}
	return (FALSE);
}

int	hash_insert(hash table, const char *key, void *value)
{
	if (table && key) {
	    struct hash_entry place;
	    if (_hash_cramped(table)) {
		_hash_grow(table);
	    }
	    place.key = _copy_string(key);
	    place.value = value;
	    place.hashcode = _hashcode_ascii(key);
	    _hash_place(table, &place, (TRUE));
	    return (TRUE);
	}
	return (FALSE);
}


void *	hash_replace(hash table, const char *key, void *value)
{
	void *oldvalue;
	struct hash_entry *entry;
	if (key) {
	    entry = _hash_find(table, key);
	    if (entry) {
	    	oldvalue = entry->value;
		entry->value = value;
		return oldvalue;
	    }
	}
	return NULL;	
}

void *	hash_delete(hash table, const char *key)
{
	void *oldvalue = NULL;
	struct hash_entry *entry;
	if (key) {
	    entry = _hash_find(table, key);
	    if (entry) {
		oldvalue = entry->value;
		memFree(entry->key);
		entry->key = DELETED;
		table->entries--;
	    }
	}
	return oldvalue;
}


void *	hash_lookup(hash table, const char *key)
{
	struct hash_entry *entry;
	if (key) {
	    entry = _hash_find(table, key);
	    if (entry) {
		return entry->value;
	    }
	}
	return NULL;
}


hstate	hash_state(hash table)
{
	hstate state;
	state.hashpos = 0;
	state.table = table;
	state.sflags.active = TRUE;
	return state;
}

void *	hash_next(hstate *state, char **key)
{
	long pos;
	char *lkey;
	void *value = NULL;
	if (state) {
	    hash table = state->table;
	    if (state->sflags.active) { 
		while ((pos = state->hashpos++) 
		    < (1 << table->sizelog)) {
		    lkey = table->data[pos].key;
		    if ((lkey != DELETED) && (lkey)) {
			value = table->data[pos].value;
			if (key) *key = lkey;
			break;
		    } 
		}
		if (pos == (1 << table->sizelog)) 
		    state->sflags.active = FALSE; 
	    }
	}
	return value;
}

int	hash_count(hash table)
{
	if (table) return table->entries;
	return 0;
}
