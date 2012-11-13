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

#ifndef IMPORT_H
#define IMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

struct Import {
   xc_clist_t node;                 /**< List node for adding an import to its client component. */
   xc_handle_t importHandle;        /**< Import handle. */
   import_t *nextImportPtr;         /**< Pointer to the next import (i.e. created from the same query). */
   import_t *previousImportPtr;     /**< Pointer to the previous import (i.e. created from the same query). */
   import_state_t state;            /**< Import state. */
   xc_handle_t queryHandle;         /**< Query handle. */
   xc_handle_t serverHandle;        /**< Server component handle. */
   component_t *serverComponentPtr; /**< Pointer to server component. */
   xc_handle_t clientHandle;        /**< Client component handle. */
   port_t *serverPortPtr;
   xc_interface_t *interfacePtr;
   char *clientProto;
};

xc_result_t
import_init (
   void
);

void
import_destroy (
   void
);

import_t *
import_new (
   xc_handle_t queryHandle,
   xc_handle_t clientHandle,
   port_t *providerPtr
);

void
import_free (
   import_t *importPtr
);

/**
  * Link an import with another one. This is used when a query is performed
  * to link the imports created for matching exports.
  *
  * @param f_pImport pointer to the import to be linked with another one
  * @param f_pNext pointer to the import being referenced by the other.
  *
  */
void
import_set_next (
   import_t *importPtr, 
   import_t *nextImportPtr
);

/**
  * Get the next sibling import.
  *
  * @param importPtr pointer to the current import
  * @return a pointer to the next sibling import or NULL.
  *
  */
#define import_get_next(importPtr) (importPtr->nextImportPtr)

xc_result_t
import_open (
   xc_handle_t importHandle,
   xc_interface_t **interfacePtr
);

void
import_free_unused (
   import_t *importPtr
);

#ifdef __cplusplus
}
#endif

#endif /* IMPORT_H */

