/*    This file is part of Othm.
 *
 *    Othm is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Ruspma is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Ruspma.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OTHM_HASHMAP_H
#define OTHM_HASHMAP_H

struct othm_request {
	void *type;
	int data_size;
	void *data;
	int (*check_key)(void *storage, void *data);
};

struct othm_hashbin {
	struct othm_request *key;
	void *storage;
	struct othm_hashbin *next;
};

struct othm_hashmap {
	unsigned int hashbin_num;
	int entries_num;
	int *Primes_pointer;
	struct othm_hashbin **hashbins;
};

#define OTHMHASHMAP(HASHMAP) ((struct othm_hashmap *) (HASHMAP))
#define OTHMREQUEST(REQUEST) ((struct othm_request *) (REQUEST))

struct othm_request *othm_request_new(int (*check_key)(void *storage, void *data),
				      void *type, int data_size, void *data);
struct othm_hashmap *othm_hashmap_new_seq(int);

struct othm_hashmap *othm_hashmap_new(void);

void othm_hashmap_free(struct othm_hashmap *hashmap);

/* void othm_print_hashmap(struct othm_hashmap *hashmap); */

void othm_hashmap_add(struct othm_hashmap *hashmap,
		      struct othm_request *request, void *storage);

void *othm_hashmap_get(struct othm_hashmap *hashmap, struct othm_request *request);

#endif
