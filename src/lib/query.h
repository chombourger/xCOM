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

#ifndef QUERY_H
#define QUERY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Lock a query object.
  *
  * @param q pointer to the query object.
  *
  */
#define query_lock(q) pthread_mutex_lock (&(q)->lock)

/**
  * Unlock a query object.
  *
  * @param q pointer to the query object.
  *
  */
#define query_unlock(q) pthread_mutex_unlock (&(q)->lock)
 
typedef struct {
   xc_handle_t queryHandle;         /**< Our very own query handle */
   xc_handle_t componentHandle;     /**< Handle of the component issuing the query. */
   component_t *componentPtr;       /**< Pointer to the component issuing the query. */
   xc_interface_t queriedInterface; /**< Queried interface description. */
   char *queriedComponent;          /**< Component name if query against a specific server. */
   char *queriedPort;               /**< Port name if a specific one requested. */
   unsigned int queryFlags;         /**< Combination of XC_QUERYF_ flags. */
   xc_clist_t imports;              /**< List of imports. */
   import_t *firstImportPtr;        /**< First matching import from this query. */
   import_t *lastImportPtr;         /**< Last matching import from this query. */
   import_t *importPtr;             /**< Current import. */
} query_t;

xc_result_t
query_init (
   void
);

void
query_destroy (
   void
);

/**
  * Create a new query.
  *
  * @param componentHandle handle of the component issuing the query.
  * @param interface name of the queried interface.
  * @param interfaceMajorVersion major version number to be matched.
  * @param interfaceMinorVersion minor version number to be matched.
  * @param queriedComponentName name of the component to provide the interface
  * @param queriedPortName targeted server component(s) port to be matched
  * @param queryFlags query flags
  * @param queryHandlePtr pointer to where the created query handle should be stored.
  * @param matchCountPtr pointer to where the number of matches should be stored.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
query_new (
   xc_handle_t componentHandle,
   const char *interfaceName,
   unsigned int interfaceMajorVersion,
   unsigned int interfaceMinorVersion,
   const char *queriedComponentName,
   const char *queriedPortName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
);

/**
  * Free a previously created query.
  *
  * @param queryPtr pointer to the query to be destroyed.
  *
  */
void
query_free (
   query_t *queryPtr
);

xc_result_t
query_add_import (
   query_t *queryPtr,
   import_t *importPtr
);

xc_result_t
query_unlink_import (
   xc_handle_t queryHandle,
   import_t *importPtr
);

#ifdef __cplusplus
}
#endif

#endif /* QUERY_H */

