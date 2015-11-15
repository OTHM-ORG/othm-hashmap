/*    This file is part of Othm_hashmap.
 *
 *    Othm_hashmap is free software: you can redistribute it and/or modify
 *    it under the terms of the Lesser GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Othm_hashmap is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Lesser GNU General Public License for more details.
 *
 *    You should have received a copy of the Lesser GNU General Public License
 *    along with Ruspma.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OTHM_HASHMAP_H
#define OTHM_HASHMAP_H

#include <othm_base.h>

enum { OTHM_HASHMAP_ALREADY_PRESENT, OTHM_HASHMAP_ADDED };

struct othm_hashentry {
	struct othm_request *key;
	void *storage;
	struct othm_hashentry *next;
};

struct othm_hashbin {
	struct othm_hashentry *first;
};

struct othm_hashmap {
	unsigned int hashbin_num;
	int entries_num;
	const int *primes_pointer;
	struct othm_hashbin *hashbins;
};

#define OTHMHASHMAP(HASHMAP) ((struct othm_hashmap *) (HASHMAP))

struct othm_hashmap *othm_hashmap_new_seq(struct othm_hashmap *(*gen)(void),
					  int seq);

struct othm_hashmap *othm_hashmap_new(struct othm_hashmap *(*gen)(void));

void othm_hashmap_free(void (*map_free)(struct othm_hashmap *map),
		       struct othm_hashmap *hashmap);

int othm_hashmap_add(struct othm_hashmap *hashmap,
		     struct othm_request *request,
		     void *storage);

void *othm_hashmap_get(struct othm_hashmap *hashmap,
		       struct othm_request *request);

#endif
