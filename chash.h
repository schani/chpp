/* -*- c -*- */

/*
 * chash.h
 *
 * chpp
 *
 * Copyright (C) 1997-1998 Herbert Poetzl
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

#ifndef __CHASH_H__
#define __CHASH_H__

struct 	hash_entry {
	long	hashcode;
	char  	*key;
	void 	*value;
	};	

struct 	hash_control {
	int 	sizelog;
	long	mask;
	long	entries;
	long 	memsize;
	struct 	hash_entry *data;
	struct 	hash_entry *wall;
	};


typedef	struct hash_control *hash;

struct	hash_state {
	hash	table;
	long	hashpos;
	struct	{
		unsigned int active:1;
		} sflags;
	};

typedef	struct hash_state   hstate;


hash	hash_new(long capacity);
void	hash_free(hash table);
void	hash_empty(hash table);
int	hash_insert(hash table, const char *key, void *value);
int	hash_inject(hash table, const char *key, void *value);
void *	hash_replace(hash table, const char *key, void *value);
void *	hash_delete(hash table, const char *key);
void *	hash_lookup(hash table, const char *key);

hstate	hash_state(hash table);
void *	hash_next(hstate *state, char **key);
int	hash_count(hash table);

#endif
