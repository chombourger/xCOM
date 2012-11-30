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

#define  TRACE_CLASS_DEFAULT QUERY
#include "internal.h"

/** Directory of running queries. */
static void *queryHandles = NULL;

/** Lock for the query sub-system. */
static pthread_mutex_t lock;

xc_result_t
query_init (
   void
) {
   xc_result_t result;

   TRACE3 (("called"));
   assert (queryHandles == NULL);

   queryHandles = handle_dir_new ();
   if (queryHandles != NULL) {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init (&attr);
      pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init (&lock, &attr);
      result = XC_OK;
   }
   else {
      TRACE1 (("Out of memory!"));
      result = XC_ERR_NOMEM;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

bool
query_match_provider (
   query_t *queryPtr,
   port_t *providerPtr
) {
   component_t *componentPtr;
   bool result = true;

   TRACE3 (("called with queryPtr=%p, providerPtr=%p", queryPtr, providerPtr));
   assert (queryPtr != NULL);
   assert (providerPtr != NULL);

   /* If a specific component is queried, make first sure that the provider component matches! */
   if (queryPtr->queriedComponent != NULL) {
      result = false;
      componentPtr = component_ref (providerPtr->componentHandle);
      if (componentPtr != NULL) {
         if (strcmp (queryPtr->queriedComponent, componentPtr->name) == 0) {
            result = true;
         }
         else {
            TRACE2 ((
               "component names do not match (%s vs %s)",
               queryPtr->queriedComponent, componentPtr->name
            ));
         }
         component_unref (componentPtr);
      }
   }

   /* Now match the actual interfaces if the provider wasn't already ruled out. */
   if (result == true) { 
      result = interfaces_match (
         &providerPtr->interfaceSpec,
         providerPtr->port,
         NULL,
         &queryPtr->queriedInterface,
         queryPtr->queriedPort,
         NULL,
         MATCHF_NONE
      );
   }
 
   TRACE3 (("exiting with result=%s", (result) ? "true" : "false"));
   return result;
}

xc_result_t
query_get_candidates (
   query_t *queryPtr,
   unsigned int *matchCountPtr
) {
   struct ProvidedPortsList *listPtr;
   port_t *providerPtr;
   import_t *importPtr;
   unsigned int count = 0;
   xc_result_t result = XC_OK;

   TRACE3 (("called"));
   assert (queryPtr != NULL);

   /* Get all the providers of the queried interface. */
   listPtr = provided_ports_ref (queryPtr->queriedInterface.name);
   if (listPtr != NULL) {
      /* Check matching providers and add them as candidates. */
      providerPtr = (port_t *) XC_CLIST_HEAD (&listPtr->ports);
      while (!XC_CLIST_END (&listPtr->ports, providerPtr)) {
         if (query_match_provider (queryPtr, providerPtr)) {
            importPtr = import_new (
               queryPtr->queryHandle,
               queryPtr->componentHandle,
               queryPtr->clientPortName,
               providerPtr
            );
            if (importPtr != NULL) {
               result = query_add_import (queryPtr, importPtr);
               if (result == XC_OK) {
                  count ++;
               }
               else {
                  TRACE1 (("failed to add import to query %u (%d)", queryPtr->queryHandle, result));
                  import_free (importPtr);
                  break;
               }
            }
            else {
               result = XC_ERR_NOMEM;
               break;
            }
         }
         providerPtr = XC_CLIST_NEXT (providerPtr);
      }
      provided_ports_unref (listPtr);
   }

   if (result == XC_OK) {
      TRACE4 (("%u providers of '%s' found!", count, queryPtr->queriedInterface.name));
      queryPtr->importPtr = queryPtr->firstImportPtr;
      assert ((count == 0) || (queryPtr->importPtr != NULL));
      if (matchCountPtr != NULL) {
         *matchCountPtr = count;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
query_destroy (
   void
) {

   TRACE3 (("called"));

   if (queryHandles != NULL) {
      handle_dir_destroy (queryHandles);
      queryHandles = NULL;
      pthread_mutex_destroy (&lock);
   }

   TRACE3 (("exiting"));
}

xc_result_t
query_new (
   xc_handle_t componentHandle,
   const char *portName,
   const char *interfaceName,
   unsigned int interfaceMajorVersion,
   unsigned int interfaceMinorVersion,
   const char *queriedComponentName,
   const char *queriedPortName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
) {

   query_t *queryPtr;
   xc_result_t result = XC_OK;

   TRACE3 ((
      "called with componentHandle=%u, portName='%s', interfaceName='%s', "
      "interfaceMajorVersion=%u, interfaceMinorVersion=%u, queriedComponentName='%s', "
      "queriedPortName='%s', queryFlags=0x%x, queryHandlePtr=%p, matchCountPtr=%p",
      componentHandle, portName, interfaceName, interfaceMajorVersion, interfaceMinorVersion,
      queriedComponentName, queriedPortName, queryFlags, queryHandlePtr, matchCountPtr
   ));
   assert (interfaceName != NULL);
   assert (queryHandlePtr != NULL);
   pthread_mutex_lock (&lock);

   queryPtr = (query_t *) malloc (sizeof (*queryPtr));
   if (queryPtr != NULL) {
      /* Initialize things to be allocated to sane values. */
      queryPtr->queryHandle = XC_INVALID_HANDLE;
      queryPtr->queriedComponent = NULL;
      queryPtr->queriedPort = NULL;
      queryPtr->componentPtr = NULL;
      queryPtr->firstImportPtr = NULL;
      queryPtr->lastImportPtr = NULL;
      queryPtr->importPtr = NULL;
      queryPtr->clientPortName = NULL;

      XC_CLIST_INIT (&queryPtr->imports);
      queryPtr->componentHandle = componentHandle;
      queryPtr->queriedInterface.vmajor = interfaceMajorVersion;
      queryPtr->queriedInterface.vminor = interfaceMinorVersion;

      /* Resolve component handle */
      if (componentHandle != XC_INVALID_HANDLE) {
         queryPtr->componentPtr = component_ref (componentHandle);
         if (queryPtr->componentPtr == NULL) {
            TRACE1 (("invalid component handle %u", componentHandle));
            result = XC_ERR_NOENT;
         }
      }

      /* Copy client port name. */
      if (portName != NULL) {
         queryPtr->clientPortName = strdup (portName);
         if (queryPtr->clientPortName == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      /* Copy interface name. */
      if (result == XC_OK) {
         queryPtr->queriedInterface.name = strdup (interfaceName);
         if (queryPtr->queriedInterface.name == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      /* Copy queried component name if specified. */
      if ((result == XC_OK) && (queriedComponentName != NULL)) {
         queryPtr->queriedComponent = strdup (queriedComponentName);
         if (queryPtr->queriedComponent == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      /* Copy queried port name if specified. */
      if ((result == XC_OK) && (queriedPortName != NULL)) {
         queryPtr->queriedPort = strdup (queriedPortName);
         if (queryPtr->queriedPort == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      /* Get a handle for this query. */
      if (result == XC_OK) {
         queryPtr->queryHandle = handle_dir_push (queryHandles, queryPtr);
      }
      else {
         TRACE1 (("Out of memory!"));
         result = XC_ERR_NOMEM;
      }

      if (result == XC_OK) {
         result = query_get_candidates (queryPtr, matchCountPtr);
      }

      if (result == XC_OK) {
         *queryHandlePtr = queryPtr->queryHandle;
      }
      else {
         query_free (queryPtr);
      }
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
query_free (
   query_t *queryPtr
) {

   TRACE3 (("called with queryPtr=%p", queryPtr));
   assert (queryPtr != NULL);

   /* Free imports that were not xCOM_Import'ed. */
   if (queryPtr->firstImportPtr != NULL) {
      import_free_unused (queryPtr->firstImportPtr);
   }

   /* Free query handle. */
   if (queryPtr->queryHandle != XC_INVALID_HANDLE) {
      handle_dir_remove (queryHandles, queryPtr->queryHandle);
   }

   free (queryPtr->queriedPort);
   free (queryPtr->queriedComponent);
   free ((void *) queryPtr->queriedInterface.name);
   free (queryPtr->clientPortName);

   if (queryPtr->componentPtr != NULL) {
      component_unref (queryPtr->componentPtr);
   }

   free (queryPtr);

   TRACE3 (("exiting"));
}

xc_result_t
xCOM_QueryInterface (
   xc_handle_t componentHandle,
   const char *interfaceName,
   unsigned int interfaceMajorVersion,
   unsigned int interfaceMinorVersion,
   const char *queriedComponentName,
   const char *queriedPortName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
) {

   xc_result_t result;

   TRACE3 ((
      "called with componentHandle=%u, interfaceName='%s', interfaceMajorVersion=%u, "
      "interfaceMinorVersion=%u, queriedComponentName='%s', queriedPortName='%s', "
      "queryFlags=0x%x, queryHandlePtr=%p, matchCountPtr=%p", componentHandle,
      interfaceName, interfaceMajorVersion, interfaceMinorVersion, queriedComponentName,
      queriedPortName, queryFlags, queryHandlePtr, matchCountPtr
   ));

   if (componentHandle == XC_INVALID_HANDLE) {
      componentHandle = component_get_anonymous ();
   }

   if ((componentHandle == XC_INVALID_HANDLE) ||
       (interfaceName == NULL) ||
       (queryHandlePtr == NULL)) {
      TRACE1 (("called with invalid parameters!"));
      result = XC_ERR_INVAL;
   }
   else {
      result = query_new (
         componentHandle, NULL, interfaceName, interfaceMajorVersion, interfaceMinorVersion,
         queriedComponentName, queriedPortName, queryFlags, queryHandlePtr, matchCountPtr
      );
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
query_next (
   query_t *queryPtr,
   xc_handle_t *importHandlePtr
) {
   import_t *importPtr;
   xc_result_t result;

   TRACE3 (("called with queryPtr=%p, importHandlePtr=%p", queryPtr, importHandlePtr));
   assert (queryPtr != NULL);
   pthread_mutex_lock (&lock);

   importPtr = queryPtr->importPtr;
   if (importPtr != NULL) {
      TRACE4 (("%u is the next import for query %u", importPtr->importHandle, queryPtr->queryHandle));
      if (importHandlePtr != NULL) {
         *importHandlePtr = importPtr->importHandle;
      }
      queryPtr->importPtr = import_get_next (importPtr);
      result = XC_OK;
   }
   else {
      TRACE2 (("no more imports for query %u", queryPtr->queryHandle));
      result = XC_ERR_NOENT;
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xCOM_QueryNext (
   xc_handle_t queryHandle,
   xc_handle_t *importHandlePtr
) {

   query_t *queryPtr;
   xc_result_t result;

   TRACE3 (("called with queryHandle=%u, importHandlePtr=%p", queryHandle, importHandlePtr));
   pthread_mutex_lock (&lock);

   queryPtr = (query_t *) handle_dir_get (queryHandles, queryHandle);
   if (queryPtr != NULL) {
      result = query_next (queryPtr, importHandlePtr);
   }
   else {
      result = XC_ERR_INVAL;
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
} 

xc_result_t
xCOM_QueryFinish (
   xc_handle_t queryHandle
) {

   query_t *queryPtr;
   xc_result_t result;

   TRACE3 (("called with queryHandle=%u", queryHandle));
   pthread_mutex_lock (&lock);

   queryPtr = (query_t *) handle_dir_get (queryHandles, queryHandle);
   if (queryPtr != NULL) {
      query_free (queryPtr);
      result = XC_OK;
   }
   else {
      result = XC_ERR_INVAL;
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xCOM_QueryPort (
   xc_handle_t componentHandle,
   const char *portName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
) {
   component_t *componentPtr;
   port_t *portPtr;
   xc_result_t result;

   TRACE3 ((
      "called with componentHandle=%u, portName='%s', queryFlags=%u, "
      "queryHandlePtr=%p, matchCountPtr=%p", componentHandle, portName,
      queryFlags, queryHandlePtr, matchCountPtr
   ));

   if ((componentHandle == XC_INVALID_HANDLE) || (portName == NULL)) {
      TRACE1 (("Invalid arguments!"));
      result = XC_ERR_INVAL;
   }
   else {

      componentPtr = component_ref (componentHandle);
      if (componentPtr != NULL) {
         portPtr = component_ref_port (componentPtr, portName);
         if (portPtr != NULL) {
            if (XC_PORTF_IS_REQUIRED (portPtr->flags) && XC_PORTF_IS_RUNTIME (portPtr->flags)) {
               result = query_new (
                  componentHandle,
                  portPtr->name,
                  portPtr->interfaceSpec.name,
                  portPtr->interfaceSpec.vmajor,
                  portPtr->interfaceSpec.vminor,
                  portPtr->componentName,
                  portPtr->port,
                  queryFlags,
                  queryHandlePtr,
                  matchCountPtr
               );
            }
            else {
               TRACE1 (("port '%s' of '%s' is not a runtime import!", portPtr->name, componentPtr->name));
               result = XC_ERR_INVAL;
            }
            component_unref_port (componentPtr, portPtr);
         }
         else {
            TRACE1 (("no port named '%s' in %s", portName, componentPtr->name));
            result = XC_ERR_NOENT;
         } 
         component_unref (componentPtr);
      }
      else {
         TRACE1 (("Invalid component handle %u!", componentHandle));
         result = XC_ERR_NOENT;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
query_add_import (
   query_t *queryPtr,
   import_t *importPtr
) {
   xc_result_t result;

   TRACE3 (("called with queryPtr=%p, importPtr=%p", queryPtr, importPtr));
   assert (queryPtr != NULL);
   assert (importPtr != NULL);

   /* Either link this import to the query if this is the first
    * one, otherwise to the previously created import. */
   if (queryPtr->lastImportPtr == NULL) {
      assert (queryPtr->firstImportPtr == NULL);
      TRACE4 (("adding first import to query %u", queryPtr->queryHandle));
      queryPtr->firstImportPtr = importPtr;
      queryPtr->lastImportPtr = importPtr;
   }
   else {
      /* TODO check if server component is already imported by this client
       * in which case, the import would be added to the front. */
      TRACE4 (("adding one more import to query %u", queryPtr->queryHandle));
      import_set_next (queryPtr->lastImportPtr, importPtr);
      queryPtr->lastImportPtr = importPtr;
   }

   /* Add import to client component */
   component_add_import (queryPtr->componentPtr, importPtr);
   result = XC_OK;

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
query_unlink_import (
   xc_handle_t queryHandle,
   import_t *importPtr
) {
   query_t *queryPtr;
   xc_result_t result;

   TRACE3 (("called with queryHandle=%u, importPtr=%p"));
   assert (queryHandle != XC_INVALID_HANDLE);
   assert (importPtr != NULL);
   pthread_mutex_lock (&lock);

   queryPtr = (query_t *) handle_dir_get (queryHandles, queryHandle);
   if (queryPtr != NULL) {
      if (queryPtr->firstImportPtr == importPtr) {
         queryPtr->firstImportPtr = import_get_next (importPtr);
      }
      if (queryPtr->lastImportPtr == importPtr) {
         queryPtr->lastImportPtr = NULL;
      }
   }
   else {
      TRACE1 (("invalid query handle %u", queryHandle));
      result = XC_ERR_NOENT;
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

