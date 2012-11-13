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

#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <xCOM/types.h>

typedef void (* hashtable_value_dtor_t) (void *value);
typedef int  (* hashtable_iter_t) (
   const char *key,
   void *value,
   void *user_data
);
 
void *
hashtable_create (
   void
);

void
hashtable_destroy (
   void *hashtable
);

unsigned int
hashtable_size (
   void *hashtable
);

xc_result_t
hashtable_lookup (
   void *hashtable,
   const char *key,
   void **valuePtr
);

xc_result_t
hashtable_insert (
   void *hashtable,
   const char *key,
   void *value
);

xc_result_t
hashtable_remove (
   void *hashtable,
   const char *key
);

void
hashtable_set_value_destructor (
   void *hashtable,
   hashtable_value_dtor_t destructor
);

int
hashtable_iter (
   void *hashtable,
   hashtable_iter_t callback,
   void *user_data
);
 
#endif /* HASHTABLE_H */

