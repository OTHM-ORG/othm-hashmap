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

#include "MurmurHash2.h"
#include "othm_hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_ENTRIES_TO_HASHBINS 1
#define DEFAULT_HASH_SEED 37

const static int hashmap_primes[] = {
	256 + 27, 512 + 9, 1024 + 9, 2048 + 5, 4096 + 3, 8192 + 27,
	16384 + 43, 32768 + 3, 65536 + 45, 131072 + 29, 262144 + 3,
	524288 + 21, 1048576 + 7, 2097152 + 17, 4194304 + 15,
	8388608 + 9, 16777216 + 43, 33554432 + 35, 67108864 + 15,
	134217728 + 29, 268435456 + 3, 536870912 + 11, 1073741824 + 85, 0
};

static void rehash(struct othm_hashmap *);

/* Copies value of request pointer into the hashbin */
static struct othm_hashentry *new_hashentry(struct othm_request *request,
					    void *storage)
{
	struct othm_hashentry *new_hashentry;

	new_hashentry = malloc(sizeof(struct othm_hashentry));
	new_hashentry->key = request;
	new_hashentry->storage = storage;
	new_hashentry->next = NULL;
	return new_hashentry;
}

/* frees hashbin without freeing request, used in rehashing */
static void hashentry_free(struct othm_hashentry *hashentry)
{
	free(hashentry);
}

/* creates a new hashmap */
struct othm_hashmap *othm_hashmap_new(void)
{
	struct othm_hashmap *new_hashmap;
	struct othm_hashbin *hashbin_ptr;
	int i;

	new_hashmap = malloc(sizeof(struct othm_hashmap));
	new_hashmap->hashbin_num = hashmap_primes[0];
	new_hashmap->hashbins =
		malloc(sizeof(struct othm_hashbin) * hashmap_primes[0]);
	new_hashmap->entries_num = 0;
	new_hashmap->primes_pointer = hashmap_primes;
	hashbin_ptr = new_hashmap->hashbins;
	for (i = 0; i != hashmap_primes[0]; ++i) {
		hashbin_ptr->first = NULL;
		++hashbin_ptr;
	}
	return new_hashmap;
}

/* Avoid uneeded rehashings by setting up the hashmap at creation
   with a point in the sequence */
struct othm_hashmap *othm_hashmap_new_seq(int sequence)
{
	struct othm_hashmap *new_hashmap;
	struct othm_hashbin *hashbin_ptr;
	int i;

	new_hashmap = malloc(sizeof(struct othm_hashmap));
	new_hashmap->hashbin_num = hashmap_primes[sequence];
	new_hashmap->hashbins =
		malloc(sizeof(struct othm_hashbin) *
		       hashmap_primes[sequence]);
	new_hashmap->entries_num = 0;
	new_hashmap->primes_pointer = hashmap_primes + sequence;
	hashbin_ptr = new_hashmap->hashbins;
	for (i = 0; i != hashmap_primes[sequence]; ++i) {
		hashbin_ptr->first = NULL;
		++hashbin_ptr;
	}
	return new_hashmap;
}

/* Final freeing of hashmap */
void othm_hashmap_free(struct othm_hashmap *hashmap)
{
	struct othm_hashbin *current_hashbin;
	unsigned int hashbin_num;
	unsigned int i;

	current_hashbin = hashmap->hashbins;
	hashbin_num = hashmap->hashbin_num;
	for (i = 0; i != hashbin_num; ++i) {
		struct othm_hashentry *current_hashentry;

		current_hashentry = current_hashbin->first;
		while (current_hashentry != NULL) {
			struct othm_hashentry *current_hashentry_holder;

			current_hashentry_holder = current_hashentry;
			current_hashentry = current_hashentry->next;
			hashentry_free(current_hashentry_holder);
		}
		++current_hashbin;
	}
	free(hashmap->hashbins);
	free(hashmap);
}

/* checks to see if hashentry uses request*/
static int check_request_hashentry(struct othm_hashentry *hashentry,
				   struct othm_request *request)
{
	if(hashentry->key->key_type != request->key_type)
		return 0;
	return request->check_key(hashentry->key->data, request->data);
}

/* Adds an element to the hashmap */
int othm_hashmap_add(struct othm_hashmap *hashmap,
		     struct othm_request *request, void *storage)
{
	struct othm_hashentry *hashentry;
	unsigned int row;

	if (hashmap->entries_num / hashmap->hashbin_num >=
	    DEFAULT_ENTRIES_TO_HASHBINS)
		rehash(hashmap);

	row = MurmurHash2(request->data, request->data_size, DEFAULT_HASH_SEED)
		% hashmap->hashbin_num;
        hashentry = hashmap->hashbins[row].first;

	if(hashentry == NULL) {
		hashmap->hashbins[row].first = new_hashentry(request, storage);
		++hashmap->entries_num;
		return OTHM_HASHMAP_ADDED;
	}

	if(check_request_hashentry(hashentry, request))
		return OTHM_HASHMAP_ALREADY_PRESENT;


	while(hashentry->next != NULL) {
		hashentry = hashentry->next;
		if(check_request_hashentry(hashentry, request))
			return OTHM_HASHMAP_ALREADY_PRESENT;
	}
	hashentry->next = new_hashentry(request, storage);
	++hashmap->entries_num;
	return OTHM_HASHMAP_ADDED;
}


/* Gets an element from a hashmap */
void *othm_hashmap_get(struct othm_hashmap *hashmap,
		       struct othm_request *request)
{
	struct othm_hashentry *hashentry;
	unsigned int row;

        row = MurmurHash2(request->data, request->data_size, DEFAULT_HASH_SEED)
		% hashmap->hashbin_num;

	hashentry = hashmap->hashbins[row].first;
	while(hashentry != NULL) {
		if(check_request_hashentry(hashentry, request))
			return hashentry->storage;
		hashentry = hashentry->next;
	}
	return NULL;
}

/* Adds to a bin something already in the hashmap during a rehash */
static void rehash_add(struct othm_hashbin *hashbin,
		       struct othm_hashentry *adding_hashentry)
{
	struct othm_hashentry *current_hashentry;

        current_hashentry = hashbin->first;
	adding_hashentry->next = NULL;
	if(current_hashentry == NULL) {
		hashbin->first = adding_hashentry;
		return;
	}
	while(current_hashentry->next != NULL)
		current_hashentry = current_hashentry->next;
	current_hashentry->next = adding_hashentry;
}

/* Rehashes the hashmap */
static void rehash(struct othm_hashmap *hashmap)
{
	struct othm_hashbin *old_hashbins;
	struct othm_hashbin *new_hashbins;
	struct othm_hashbin *current_hashbin;
	unsigned int old_hashbin_num;
	unsigned int new_hashbin_num;
	unsigned int i;

	old_hashbin_num = hashmap->hashbin_num;
	new_hashbin_num = hashmap->primes_pointer[1];

	old_hashbins = hashmap->hashbins;
	new_hashbins = malloc(sizeof(struct othm_hashbin) *
			      new_hashbin_num);

	current_hashbin = old_hashbins;
	for (i = 0; i != old_hashbin_num; ++i) {
		struct othm_hashentry *current_hashentry;

		current_hashentry = current_hashbin->first;
		while (current_hashentry != NULL) {
			struct othm_hashentry *current_hashentry_holder;
			unsigned int row;

			current_hashentry_holder = current_hashentry;
			current_hashentry = current_hashentry->next;
			row = MurmurHash2(current_hashentry_holder->key->data,
					  current_hashentry_holder->key->data_size,
					  DEFAULT_HASH_SEED) % new_hashbin_num;
			rehash_add(new_hashbins + row, current_hashentry_holder);
		}
		++current_hashbin;
	}
	free(hashmap->hashbins);
	hashmap->hashbins = new_hashbins;
	++hashmap->primes_pointer;
	hashmap->hashbin_num = new_hashbin_num;
}
