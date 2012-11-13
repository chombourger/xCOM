/*
 * xCOM - a Simple Component Framework
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#define  TRACE_CLASS_DEFAULT HASHTABLE
#include "internal.h"

#define HASHTABLE_MIN_SIZE 11
#define HASHTABLE_MAX_SIZE 13845163

static const unsigned int primes [] = {
   11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861,
   2777, 4177, 6247, 9371, 14057, 21089, 31627, 47431, 71143,
   106721, 160073, 240101, 360163, 540217, 810343, 1215497,
   1823231, 2734867, 4102283, 6153409, 9230113, 13845163
};

#define NPRIMES (sizeof (primes) / sizeof (primes [0]))

typedef struct hashtable_entry {
   const char * key;
   unsigned int hash;
   void *value;
   struct hashtable_entry *next;
} entry_t;

struct hashtable {
   unsigned int size;
   unsigned int count;
   entry_t **entries;
   hashtable_value_dtor_t value_dtor;
};

static unsigned int
hash_string (
   const char *str
) {
   /* 31 bit hash function */
   const signed char *p = (const signed char *) str;
   unsigned int h = *p;

   if (h) {
      for (p += 1; *p != '\0'; p++) {
         h = (h << 5) - h + *p;
      }
   }
   return h;
}

static inline entry_t *
entry_new (
   const char *key,
   unsigned int hash,
   void *value
) {
   entry_t *entry;
   entry = (entry_t *) malloc (sizeof (*entry));
   if (entry != NULL) {
      entry->key   = key;
      entry->hash  = hash;
      entry->value = value;
      entry->next  = NULL;
   }
   return entry;
}

static inline void
entry_destroy (
   struct hashtable *hashtable,
   entry_t *entry
) {

   TRACE3 (("called with hashtable=%p, entry=%p", hashtable, entry));
   assert (hashtable != NULL);
   assert (entry != NULL);

   if (hashtable->value_dtor) {
      hashtable->value_dtor (entry->value);
   }
   free (entry);
}

/**
 * Create a new hash table.
 * @return an opaque pointer to the created hashtable on success or NULL.
 **/

void *
hashtable_create (
   void
) {
   struct hashtable *hashtable;

   hashtable = (struct hashtable *) malloc (sizeof (*hashtable));
   if (hashtable != NULL) {
      hashtable->value_dtor = NULL;
      hashtable->size       = HASHTABLE_MIN_SIZE;
      hashtable->count      = 0;
      hashtable->entries    = malloc (sizeof (entry_t *) * hashtable->size);
      if (hashtable->entries != NULL) {
         memset (hashtable->entries, 0, sizeof (entry_t *) * hashtable->size);
      }
      else {
         free (hashtable);
         hashtable = NULL;
      }
   }
   return hashtable;
}

void
hashtable_destroy (
   void *_hashtable
) {
   struct hashtable *hashtable = _hashtable;
   entry_t *entry;
   entry_t *next;
   unsigned int i;

   for (i = 0; i < hashtable->size; i++) {
      for (entry = hashtable->entries [i]; entry != NULL; entry = next) {
         next = entry->next;
         entry_destroy (hashtable, entry);
      }
   }

   free (hashtable->entries);
   free (hashtable);
}

int
hashtable_iter (
   void *_hashtable,
   hashtable_iter_t callback,
   void *user_data
) {
   struct hashtable *hashtable = _hashtable;
   entry_t *entry;
   unsigned int i;
   int r = 0;

   for (i = 0; i < hashtable->size; i++) {
      for (entry = hashtable->entries [i]; entry != NULL; entry = entry->next) {
         r = callback (entry->key, entry->value, user_data);
         if (r != 0) goto exit;
      }
   }

exit:
   return r;
}

/**
 * Return the number of entries contained in the hashtable.
 * @param _hashtable opaque pointer to the hashtable
 * @return the number of contained entries.
 **/
unsigned int
hashtable_size (
   void *_hashtable
) {
   struct hashtable *hashtable = _hashtable;
   TRACE2 (("called with hashtable=%p", hashtable));

   assert (hashtable != NULL);

   TRACE3 (("exiting with result=%u", hashtable->count));
   return hashtable->count;
}

static inline entry_t **
lookup (
   struct hashtable *hashtable,
   const char *key,
   unsigned int hash
) {

   entry_t **entry_ptr;

   TRACE3 (("called with hashtable=%p, key='%s', hash=%u", hashtable, key, hash));
   assert (hashtable != NULL);
   assert (key != NULL);

   entry_ptr = &hashtable->entries [hash % hashtable->size];
   while ((*entry_ptr != NULL) &&
          (((*entry_ptr)->hash != hash) || ((strcmp ((*entry_ptr)->key, key)) != 0))) {
      entry_ptr = &(*entry_ptr)->next;
   }

   TRACE3 (("exiting with result=%p", entry_ptr));
   return entry_ptr;
}

static unsigned int
closest_prime (
   unsigned int n
) {

   unsigned int i;

   for (i = 0; i < NPRIMES; i++) {
      if (primes [i] > n) return primes [i];
   }

   return primes [NPRIMES - 1];
}

static xc_result_t
resize (
   struct hashtable *hashtable
) {

   entry_t **entries;
   entry_t *entry;
   entry_t *next;
   xc_result_t result;
   unsigned int size;
   unsigned int hash;
   unsigned int i;

   TRACE3 (("called with hashtable=%p", hashtable));
   assert (hashtable != NULL);

   /* Check whether the hashtable needs to be resized. */

   if (((hashtable->size >= 3 * hashtable->count) && (hashtable->size > HASHTABLE_MIN_SIZE)) ||
       ((3 * hashtable->size <= hashtable->count) && (hashtable->size < HASHTABLE_MAX_SIZE))) {

      /* Yes, allocate and initialize a new entry array. */

      size = closest_prime (hashtable->count);
      if (size > HASHTABLE_MAX_SIZE) size = HASHTABLE_MAX_SIZE;
      if (size < HASHTABLE_MIN_SIZE) size = HASHTABLE_MIN_SIZE;
      TRACE4 (("resizing hashtable from %u to %u", hashtable->size, size));
      entries = (entry_t **) malloc (sizeof (*entry) * size);
      if (entries != NULL) {
         memset (entries, 0, sizeof (*entry) * size);
         for (i = 0; i < hashtable->size; i++) {
            for (entry = hashtable->entries [i]; entry != NULL; entry = next) {
               next           = entry->next;
               hash           = entry->hash % size;
               entry->next    = entries [hash];
               entries [hash] = entry; 
            }
         }
         free (hashtable->entries);
         hashtable->entries = entries;
         hashtable->size    = size;
         result             = XC_OK;
      }
      else result = XC_ERR_NOMEM;
   }
   else result = XC_OK;

   TRACE3 (("exiting with result=%d", result));
   return result;
}

/**
 * Lookup a specified key in a hashtable.
 * @param _hashtable opaque pointer to the hashtable to look into.
 * @param key the search key
 * @param value_ptr where to store the value associated with the key
 * @return XC_OK if the key was found, an error code otherwise.
 **/
xc_result_t
hashtable_lookup (
   void *_hashtable,
   const char *key,
   void **value_ptr
) {
   struct hashtable *hashtable = _hashtable;
   entry_t *entry;
   unsigned int hash;
   xc_result_t result;

   TRACE3 (("called with hashtable=%p, key='%s', value_ptr=%p", hashtable, key, value_ptr));
   assert (hashtable != NULL);
   assert (key != NULL);
   assert (value_ptr != NULL);

   /* Compute the hash code of the specified key. */
   hash = hash_string (key);

   /* Search for a matching entry in the hashtable. */
   entry = *lookup (hashtable, key, hash);
   if (entry != NULL) {
      /* One was found, provide value to caller. */
      *value_ptr = entry->value;
      result = XC_OK;
   }
   else {
      result = XC_ERR_NOENT;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
hashtable_insert (
   void *_hashtable,
   const char *key,
   void *value
) {
   struct hashtable *hashtable = _hashtable;
   entry_t **entry_ptr;
   unsigned int hash;
   xc_result_t result;

   TRACE3 (("called with hashtable=%p, key='%s', value=%p", hashtable, key, value));
   assert (hashtable != NULL);
   assert (key != NULL);

   /* Compute the hash code of the specified key. */
   hash = hash_string (key);

   /** Lookup the key in the hashtable. */
   entry_ptr = lookup (hashtable, key, hash);
   assert (entry_ptr != NULL);

   if ((*entry_ptr) == NULL) {
      (*entry_ptr) = entry_new (key, hash, value);
      if ((*entry_ptr) != NULL) {
         hashtable->count ++;
         result = resize (hashtable);
         if (result != XC_OK) {
            entry_destroy (hashtable, (*entry_ptr));
            (*entry_ptr) = NULL;
         }
      }
      else result = XC_ERR_NOMEM;
   }
   else {
      result = XC_ERR_EXIST;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
hashtable_remove (
   void *_hashtable,
   const char *key
) {

   struct hashtable *hashtable = _hashtable;
   entry_t **entry_ptr;
   entry_t *entry;
   unsigned int hash;
   xc_result_t result;

   TRACE3 (("called with hashtable=%p, key='%s'", hashtable, key));
   assert (hashtable != NULL);
   assert (key != NULL);

   /* Compute the hash code of the specified key. */
   hash = hash_string (key);

   /** Lookup the key in the hashtable. */
   entry_ptr = lookup (hashtable, key, hash);
   assert (entry_ptr != NULL);

   if ((*entry_ptr) == NULL) {
      result = XC_ERR_NOENT;
   }
   else {
      entry = (*entry_ptr);
      (*entry_ptr) = entry->next;
      entry_destroy (hashtable, entry);
      assert (hashtable->count > 0);
      hashtable->count --; 
      (void) resize (hashtable);
      result = XC_OK;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
hashtable_set_value_destructor (
   void *_hashtable,
   hashtable_value_dtor_t destructor
) {
   struct hashtable *hashtable = _hashtable;

   TRACE3 (("called with hashtable=%p, destructor=%p", hashtable, destructor));
   assert (hashtable != NULL);

   hashtable->value_dtor = destructor;

   TRACE3 (("exiting"));
}

