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

#define DEFAULT_ENTRIES_TO_HASHBINS 5
#define DEFAULT_HASH_SEED 37

const static int hashmap_primes[] = {
	256 + 27, 512 + 9, 1024 + 9, 2048 + 5, 4096 + 3, 8192 + 27,
	16384 + 43, 32768 + 3, 65536 + 45, 131072 + 29, 262144 + 3,
	524288 + 21, 1048576 + 7, 2097152 + 17, 4194304 + 15,
	8388608 + 9, 16777216 + 43, 33554432 + 35, 67108864 + 15,
	134217728 + 29, 268435456 + 3, 536870912 + 11, 1073741824 + 85, 0
};

/* Creates a new request */
struct othm_request *othm_request_new(int (*check_key)(void *storage, void *data),
				      void *type, int data_size, void *data)
{
	struct othm_request *request;
	request = malloc(sizeof(struct othm_request));
	request->check_key = check_key;
	request->type = type;
	request->data_size = data_size;
	request->data = data;
	return request;
}

/* Creates a new pair */
struct othm_pair othm_pair_new(void *first, void *second)
{
	struct othm_pair pair;

	pair.first = first;
	pair.second = second;
	return pair;
}

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
	getchar();
	return new_hashentry;
}

/* /\* If request exists saved in some place the hashmap can uses safely */
/*   (such as a hashbin before a rehash) then use this method so */
/*   there is no extra computation *\/ */
/* struct othm_hashbin *copy_hashentry(struct othm_request *request, void *storage) */
/* { */
/* 	struct othm_hashentry *new_hashentry; */

/* 	new_hashentry = malloc(sizeof(struct othm_hashentry)); */
/* 	new_hashentry->key = request; */
/* 	new_hashentry->storage = storage; */
/* 	new_hashentry->next = NULL; */
/* 	return new_hashentry; */
/* } */

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

/* void othm_print_hashmap(struct othm_hashmap *hashmap) */
/* { */
/*   struct othm_hashbin **current_top_hashbin = hashmap->hashbins; */
/*   unsigned int hashbin_num = hashmap->hashbin_num; */
/*   unsigned int i; */
/*   for(i = 0; i != hashbin_num; ++i) { */
/*     struct othm_hashbin *current_hashbin = *current_top_hashbin; */
/*     while(current_hashbin != NULL) { */
/*       struct othm_hashbin *current_hashbin_holder = current_hashbin; */
/*       current_hashbin = current_hashbin->next; */
/*       printf("%s ", current_hashbin_holder->storage->prim_vals.string); */
/*     } */
/*     ++current_top_hashbin; */
/*     printf("\n"); */
/*   } */
/* } */

/* checks to see if hashentry uses request*/
static int check_request_hashentry(struct othm_hashentry *hashentry,
				   struct othm_request *request)
{
	if(hashentry->key->type != request->type)
		return 0;
	return request->check_key(hashentry->key->data, request->data);
}

/* /\* Returns a hashbin given a request, else makes hashbin and returns that *\/ */
/* static void search_add_hashbin(struct othm_hashmap *hashmap, */
/* 			       struct othm_hashbin *hashbin, */
/* 			       struct othm_request *request, void *storage) */
/* { */
/* 	if (check_request_hashbin(top_hashbin->first, request)) */
/* 		top_hashbin->storage = storage; */
/* 	else { */
/* 		struct othm_hashbin *next_hashbin; */

/* 		next_hashbin = top_hashbin; */
/* 		if (top_hashbin->next != NULL) { */
/* 			next_hashbin = top_hashbin->next; */
/* 			int loop = 1; */
/* 			while (loop) { */
/* 				if (check_request_hashbin(next_hashbin, request)) { */
/* 					next_hashbin->storage = storage; */
/* 					loop = 0; */
/* 				} else if (next_hashbin->next == NULL) { */
/* 					next_hashbin->next = */
/* 					    new_hashbin(request, storage); */
/* 					++(hashmap->entries_num); */
/* 					loop = 0; */
/* 				} else */
/* 					next_hashbin = next_hashbin->next; */
/* 			} */
/* 		} else */
/* 			top_hashbin->next = new_hashbin(request, storage); */
/* 	} */
/* } */

/* /\* You should never need to use this so long name is okay... I hate myself for this */
/*    Returns hashbin if it exists, else makes new hashbin and returns it *\/ */
/* static void search_add_top_hashbin_old_request(struct othm_hashmap *hashmap, */
/* 					       struct othm_hashbin *top_hashbin, */
/* 					       struct othm_request *request, */
/* 					       void *storage) */
/* { */
/* 	if (check_request_hashbin(top_hashbin, request)) */
/* 		top_hashbin->storage = storage; */
/* 	else { */
/* 		struct othm_hashbin *next_hashbin; */

/* 		next_hashbin = top_hashbin; */
/* 		if (top_hashbin->next != NULL) { */
/* 			next_hashbin = top_hashbin->next; */
/* 			int loop = 1; */
/* 			while (loop) { */
/* 				if (check_request_hashbin(next_hashbin, request)) { */
/* 					next_hashbin->storage = storage; */
/* 					loop = 0; */
/* 				} else if (next_hashbin->next == NULL) { */
/* 					next_hashbin->next = */
/* 					    copy_hashbin(request, storage); */
/* 					++(hashmap->entries_num); */
/* 					loop = 0; */
/* 				} else */
/* 					next_hashbin = next_hashbin->next; */
/* 			} */
/* 		} else { */
/* 			top_hashbin->next = new_hashbin(request, storage); */
/* 		} */
/* 	} */
/* } */

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
		return 0;
	}

	if(check_request_hashentry(hashentry, request))
		return 1;


	while(hashentry->next != NULL) {
		hashentry = hashentry->next;
		if(check_request_hashentry(hashentry, request))
			return 1;
	}
	hashentry->next = new_hashentry(request, storage);
	++hashmap->entries_num;
	return 0;
	/* if (hashentry == NULL) { */
	/* 	hashmap->hashbins[row].first = new_hashbin(request, storage); */
	/* 	++hashmap->entries_num; */
	/* 	return; */
	/* } */

}

/* /\* Adds an element to the hashmap without allocating a new request. Used in rehashes *\/ */
/* static void readd_to_hashmap(struct othm_hashmap *hashmap, */
/* 			     struct othm_request *request, void *storage) */
/* { */
/* 	struct othm_hashbin *top_hashbin; */
/* 	unsigned int row; */

/* 	if (hashmap->entries_num / hashmap->hashbin_num >= */
/* 	    DEFAULT_ENTRIES_TO_HASHBINS) */
/* 		rehash(hashmap); */
/* 	row = MurmurHash2(request->data, request->data_size, DEFAULT_HASH_SEED) */
/* 		% hashmap->hashbin_num; */
/*         top_hashbin = hashmap->hashbins[row]; */
/* 	if (top_hashbin == NULL) { */
/* 		hashmap->hashbins[row] = copy_hashbin(request, storage); */
/* 		++(hashmap->entries_num); */
/* 		return; */
/* 	} */

/* 	search_add_top_hashbin_old_request(hashmap, top_hashbin, request, */
/* 					   storage); */
/* } */

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
	/* if (top_hashbin == NULL) */
	/* 	return NULL; */
	/* if (check_request_hashbin(top_hashbin, request)) */
	/* 	return top_hashbin->storage; */

        /* next_hashbin = top_hashbin->next; */
	/* while (next_hashbin != NULL) { */
	/* 	if (check_request_hashbin(next_hashbin, request)) */
	/* 		return next_hashbin->storage; */
	/* 	next_hashbin = next_hashbin->next; */
	/* } */
}

/* struct othm_pair othm_hashmap_remove(struct othm_hashmap *hashmap, */
/* 				     struct othm_request *request) */
/* { */
/* 	struct othm_hashbin *previous_hashentry; */
/* 	struct othm_hashbin *next_hashentry; */
/* 	struct othm_pair removed; */
/* 	unsigned int row; */

/*         row = MurmurHash2(request->data, request->data_size, DEFAULT_HASH_SEED) */
/* 		% hashmap->hashbin_num; */
/* 	top_hashbin = hashmap->hashbins[row]; */
/* 	if (top_hashbin == NULL) */
/* 		return othm_pair_new(NULL, NULL); */
/* 	if (check_request_hashbin(top_hashbin, request)) { */
/* 		removed = othm_pair_new(top_hashbin->key, */
/* 					top_hashbin->storage); */
/* 		if (top_hashbin->next != NULL) { */
/* 			*top_hashbin = *top_hashbin->next; */
/* 			hashbin_free(top_hashbin->next); */
/* 		} else { */
/* 			hashbin_free(top_hashbin); */
/* 			hashmap->hashbins[row] = NULL; */
/* 		} */
/* 		return removed; */
/* 	} */

/* 	previous_hashbin = top_hashbin; */
/*         next_hashbin = top_hashbin->next; */
/* 	while (next_hashbin != NULL) { */
/* 		if (check_request_hashbin(next_hashbin, request)) { */
/* 			removed = othm_pair_new(next_hashbin->key, */
/* 						next_hashbin->storage); */
/* 			previous_hashbin->next = next_hashbin->next; */
/* 			hashbin_free(next_hashbin); */
/* 			return removed; */
/* 		} */
/* 		previous_hashbin = next_hashbin; */
/* 		next_hashbin = next_hashbin->next; */
/* 	} */
/* 	return othm_pair_new(NULL, NULL); */
/* } */

/* Rehashes the hashmap */
static void rehash(struct othm_hashmap *hashmap)
{
	/* struct othm_hashbin **old_hashbins; */
	/* struct othm_hashbin **current_top_hashbin; */
	/* struct othm_hashbin **rehashed_hashbins; */
	/* unsigned int old_hashbin_num; */
	/* unsigned int new_hashbin_num; */
	/* unsigned int i; */

	/* old_hashbin_num = hashmap->hashbin_num; */
	/* old_hashbins = hashmap->hashbins; */
	/* hashmap->entries_num = 0; */
	/* ++(hashmap->primes_pointer); */
        /* new_hashbin_num = *(hashmap->primes_pointer); */
	/* hashmap->hashbin_num = new_hashbin_num; */
	/* rehashed_hashbins = */
	/*     malloc(sizeof(struct othm_hashbin *) * new_hashbin_num); */
	/* hashmap->hashbins = rehashed_hashbins; */

	/* for (i = 0; i != new_hashbin_num; ++i) { */
	/* 	*rehashed_hashbins = NULL; */
	/* 	++rehashed_hashbins; */
	/* } */

        /* current_top_hashbin = old_hashbins; */
	/* for (i = 0; i != old_hashbin_num; ++i) { */
	/* 	struct othm_hashbin *current_hashbin; */

	/* 	current_hashbin = *current_top_hashbin; */
	/* 	while (current_hashbin != NULL) { */
	/* 		readd_to_hashmap(hashmap, */
	/* 				 current_hashbin->key, */
	/* 				 current_hashbin->storage); */
	/* 		struct othm_hashbin *current_hashbin_holder; */

	/* 		current_hashbin_holder = current_hashbin; */
	/* 		current_hashbin = current_hashbin->next; */
	/* 		hashbin_free(current_hashbin_holder); */
	/* 	} */
	/* 	++current_top_hashbin; */
	/* } */
	/* free(old_hashbins); */
}
