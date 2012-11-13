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

#ifndef HANDLEDIR_H
#define HANDLEDIR_H

#ifdef __cplusplus
extern "C" {
#endif

#define HANDLE_DIR_POOL_SIZE 0x1000

/**
 *
 * Allocate and initialize a handle directory
 *
 * @return pointer to the created directory, or NULL for failure.
 *
 */
void *
handle_dir_new (
   void
);

/**
 *
 * Frees a handle directory. No reference to the directory can be
 * made after that call returns.
 *
 * @param hDirPtr Pointer to handle directory to be freed.
 *
 */
void
handle_dir_destroy (
   void *hDirPtr
);


/**
 *
 * Push an object to a directory and get a handle for it.
 *
 * @param hDirPtr Pointer to handle directory to allocate from
 * @param objPtr Opaque pointer to object to be associated with the created handle
 *
 * @return a handle number (>= 1) or 0 for error.
 *
 */
xc_handle_t
handle_dir_push (
   void *hDirPtr, 
   void *objPtr
);

/**
 *
 * Remove a handle from the directory.
 *
 * @param hDirPtr Pointer to handle directory to allocate from
 * @param handle Handle number to be released
 *
 */
void
handle_dir_remove (
   void        *hDirPtr, 
   xc_handle_t  handle
);

/**
 *
 * Return opaque pointer to object associated with handle
 *
 * @param hDirPtr pointer to handle directory
 * @param handle handle number
 *
 * @return pointer to the object with that handle or NULL if not found.
 *
 */
void *
handle_dir_get (
   void        *hDirPtr, 
   xc_handle_t  handle
);

#endif /* HANDLEDIR_H */

