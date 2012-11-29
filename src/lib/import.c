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

#define  TRACE_CLASS_DEFAULT IMPORT
#include "internal.h"

/** Directory of running queries. */
static void *importHandles = NULL;

#if defined (HAVE_PTHREAD_H)

/** Lock for the import sub-system. */
static pthread_mutex_t lock;

#define import_module_lock_init() do {                         \
   pthread_mutexattr_t attr;                                   \
   pthread_mutexattr_init (&attr);                             \
   pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE); \
   pthread_mutex_init (&lock, &attr);                          \
} while (0)

#define import_module_lock() pthread_mutex_lock(&lock)
#define import_module_unlock() pthread_mutex_unlock(&lock)

#else /* !HAVE_PTHREAD_H */
#error "create lock macros for your system!"
#endif

xc_result_t
import_init (
   void
) {
   xc_result_t result;

   TRACE3 (("called"));
   assert (importHandles == NULL);

   importHandles = handle_dir_new ();
   if (importHandles != NULL) {
      import_module_lock_init ();
      result = XC_OK;
   }
   else {
      TRACE1 (("Out of memory!"));
      result = XC_ERR_NOMEM;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
import_destroy (
   void
) {

   TRACE3 (("called"));

   if (importHandles != NULL) {
      handle_dir_destroy (importHandles);
      importHandles = NULL;
      pthread_mutex_destroy (&lock);
   }

   TRACE3 (("exiting"));
}

import_t *
import_new (
   xc_handle_t queryHandle,
   xc_handle_t clientHandle,
   const char *clientPortName,
   port_t *providerPtr
) {

   import_t *importPtr;
   xc_result_t result = XC_OK;

   TRACE3 ((
      "called with queryHandle=%u, clientHandle=%u, clientPortName='%s', providerPtr=%p",
      queryHandle, clientHandle, clientPortName, providerPtr
   ));
   assert (providerPtr != NULL);

   importPtr = (import_t *) malloc (sizeof (*importPtr));
   if (importPtr != NULL) {
      importPtr->state = IMPORT_STATE_QUERIED;
      importPtr->queryHandle = queryHandle;
      importPtr->nextImportPtr = NULL;
      importPtr->previousImportPtr = NULL;
      importPtr->importHandle = XC_INVALID_HANDLE;
      importPtr->serverPortPtr = providerPtr;
      importPtr->serverHandle = providerPtr->componentHandle;
      importPtr->serverComponentPtr = NULL;
      importPtr->clientHandle = clientHandle;
      importPtr->portName = NULL;
      importPtr->interfacePtr = NULL;
      importPtr->clientPortName = NULL;

      if (providerPtr->port != NULL) {
         importPtr->portName = strdup (providerPtr->port);
         if (importPtr->portName == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      if ((result == XC_OK) && (clientPortName != NULL)) {
         importPtr->clientPortName = strdup (clientPortName);
         if (importPtr->clientPortName == NULL) {
            TRACE1 (("Out of memory!"));
            result = XC_ERR_NOMEM;
         }
      }

      /** Get a reference to the server component. */
      if ((result == XC_OK) && (importPtr->serverHandle != XC_INVALID_HANDLE)) {
         importPtr->serverComponentPtr = component_ref (importPtr->serverHandle);
         if (importPtr->serverComponentPtr == NULL) {
            TRACE1 (("Invalid server component handle #%u", importPtr->serverHandle));
            result = XC_ERR_NOENT;
         }
      }

      /** Get a handle for this new import. */ 
      if (result == XC_OK) {
         import_module_lock ();
         importPtr->importHandle = handle_dir_push (importHandles, importPtr);
         if (importPtr->importHandle == XC_INVALID_HANDLE) {
            TRACE1 (("Out of memory or handles!"));
            result = XC_ERR_NOMEM;
         }
         import_module_unlock ();
      }

      if (result != XC_OK) {
         import_free (importPtr);
         importPtr = NULL;
      }
   }
   else {
      TRACE1 (("Out of memory!"));
   }

   TRACE3 (("exiting with result=%p", importPtr));
   return importPtr;
}

static void
unregister_from_server (
   import_t *importPtr
) {

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   assert (importPtr->state == IMPORT_STATE_REGISTERED_TO_SERVER);

   (void) component_unregister (
      importPtr->serverComponentPtr,
      importPtr->serverPortPtr,
      importPtr->importHandle
   );
   importPtr->state = IMPORT_STATE_UNREGISTERED;

   TRACE3 (("exiting"));
}

static void
unregister_from_client (
   import_t *importPtr
) {

   component_t *clientComponentPtr;
   port_t *clientPortPtr;
   xc_result_t result;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   assert (importPtr->state == IMPORT_STATE_REGISTERED_TO_CLIENT);

   if (importPtr->clientPortName != NULL) {
      /* Register import to client component. */
      clientComponentPtr = component_ref (importPtr->clientHandle);
      if (clientComponentPtr != NULL) {
         clientPortPtr = component_ref_port (clientComponentPtr, importPtr->clientPortName);
         if (clientPortPtr != NULL) {
            result = component_unregister (
               clientComponentPtr,
               clientPortPtr,
               importPtr->importHandle
            );
            component_unref_port (clientComponentPtr, clientPortPtr);
            clientPortPtr = NULL;
         }
         else {
            TRACE1 (("lookup of port '%s' failed", importPtr->clientPortName));
            result = XC_ERR_NOENT;
         }
         component_unref (clientComponentPtr);
         clientComponentPtr = NULL;
      }
      else {
         TRACE1 (("lookup of client component #%u failed", importPtr->clientHandle));
         result = XC_ERR_NOENT;
      }
   }
   else {
      result = XC_OK;
   }

   if (result == XC_OK) {
      importPtr->state = IMPORT_STATE_REGISTERED_TO_SERVER;
   }

   TRACE3 (("exiting"));
}

static void
import_unregister (
   import_t *importPtr
) {

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);

   if (importPtr->state == IMPORT_STATE_REGISTERED_TO_CLIENT) {
      unregister_from_client (importPtr);
   }

   if (importPtr->state == IMPORT_STATE_REGISTERED_TO_SERVER) {
      unregister_from_server (importPtr);
   }

   TRACE3 (("exiting"));
}

void
import_free (
   import_t *importPtr
) {

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   import_module_lock ();

   if (importPtr->state == IMPORT_STATE_OPENED) {
      TRACE4 (("freeing switch %p", importPtr->interfacePtr));
      free (importPtr->interfacePtr);
      importPtr->interfacePtr = NULL;
      importPtr->state = IMPORT_STATE_REGISTERED_TO_CLIENT;
   }

   import_unregister (importPtr);

   if (importPtr->importHandle != XC_INVALID_HANDLE) {
      TRACE4 (("deleting import handle #%u", importPtr->importHandle));
      handle_dir_remove (importHandles, importPtr->importHandle);
   }

   if (importPtr->serverComponentPtr != NULL) {
      component_unref (importPtr->serverComponentPtr);
      importPtr->serverComponentPtr = NULL;
   }

   free (importPtr->clientPortName);
   free (importPtr->portName);
   free (importPtr);

   import_module_unlock ();
   TRACE3 (("exiting"));
}

static xc_result_t
register_to_server (
   import_t *importPtr
) {

   xc_result_t result;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   assert (importPtr->state == IMPORT_STATE_UNREGISTERED);

   /* Register import to server component. */
   result = component_register (
      importPtr->serverComponentPtr,
      importPtr->serverPortPtr,
      importPtr->importHandle
   );
   if (result == XC_OK) {
      importPtr->state = IMPORT_STATE_REGISTERED_TO_SERVER;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

static xc_result_t
register_to_client (
   import_t *importPtr
) {

   component_t *clientComponentPtr;
   port_t *clientPortPtr;
   xc_result_t result;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   assert (importPtr->state == IMPORT_STATE_REGISTERED_TO_SERVER);

   if (importPtr->clientPortName != NULL) {
      /* Register import to client component. */
      TRACE4 ((
         "need to register() on component #%u port '%s'",
          importPtr->clientHandle, importPtr->clientPortName
      ));
      clientComponentPtr = component_ref (importPtr->clientHandle);
      if (clientComponentPtr != NULL) {
         clientPortPtr = component_ref_port (clientComponentPtr, importPtr->clientPortName);
         if (clientPortPtr != NULL) {
            result = component_register (
               clientComponentPtr,
               clientPortPtr,
               importPtr->importHandle
            );
            TRACE4 ((
               "register() on component #%u port '%s' returned %d",
               importPtr->clientHandle, importPtr->clientPortName, result
            ));
            component_unref_port (clientComponentPtr, clientPortPtr);
            clientPortPtr = NULL;
         }
         else {
            TRACE1 (("lookup of port '%s' failed", importPtr->clientPortName));
            result = XC_ERR_NOENT;
         }
         component_unref (clientComponentPtr);
         clientComponentPtr = NULL;
      }
      else {
         TRACE1 (("lookup of client component #%u failed", importPtr->clientHandle));
         result = XC_ERR_NOENT;
      }
   }
   else {
      result = XC_OK;
   }

   if (result == XC_OK) {
      importPtr->state = IMPORT_STATE_REGISTERED_TO_CLIENT;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

static xc_result_t
import_register (
   import_t *importPtr
) {

   xc_result_t result;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   assert (importPtr->state == IMPORT_STATE_UNREGISTERED);

   /* Register import to server component. */
   result = register_to_server (importPtr);
   if (result == XC_OK) {
      /* Register import to client component. */
      result = register_to_client (importPtr);
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}
 
xc_result_t
import_open (
   xc_handle_t importHandle,
   xc_interface_t **interfacePtr
) {

   import_t *importPtr;
   xc_result_t result;

   TRACE3 (("called with importHandle=%u, interfacePtr=%p", importHandle, interfacePtr));
   import_module_lock ();

   importPtr = handle_dir_get (importHandles, importHandle);
   if (importPtr != NULL) {
      if (importPtr->state == IMPORT_STATE_QUERIED) {

         /* Detach import from its query. */
         assert (importPtr->queryHandle != XC_INVALID_HANDLE);
         TRACE4 (("detaching import %u from query %u", importHandle, importPtr->queryHandle));
         query_unlink_import (importPtr->queryHandle, importPtr);
         importPtr->queryHandle = XC_INVALID_HANDLE;
         importPtr->state = IMPORT_STATE_UNREGISTERED;
     }

     if (importPtr->state == IMPORT_STATE_UNREGISTERED) {

         /* Register import. */
         result = import_register (importPtr);
         if (result == XC_OK) {
            /* Allocate and provide client switch. */
            importPtr->interfacePtr = malloc (importPtr->serverPortPtr->interfaceSize);
            if (importPtr->interfacePtr != NULL) {
               memcpy (importPtr->interfacePtr, importPtr->serverPortPtr->interfacePtr, importPtr->serverPortPtr->interfaceSize);
               importPtr->state = IMPORT_STATE_OPENED;
               if (interfacePtr != NULL) {
                  *interfacePtr = importPtr->interfacePtr;
               }
            }
            else {
               TRACE1 (("Out of memory!"));
               result = XC_ERR_NOMEM;
            }
         }

         if (result == XC_OK) {
            result = component_init (importPtr->serverComponentPtr);
         }

         if (result != XC_OK) {
            TRACE1 (("failed to open import #%u, deleting.", importHandle));
            import_close (importPtr);
            importPtr = NULL;
         }
      }
      else {
         TRACE1 (("import %u already imported!", importHandle));
         result = XC_ERR_BUSY;
      }
   }
   else {
      TRACE1 (("import #%u was not found!", importHandle));
      result = XC_ERR_NOENT;
   }

   import_module_unlock ();
   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
import_close (
   import_t *importPtr
) {

   component_t *clientComponentPtr;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);

   clientComponentPtr = component_ref (importPtr->clientHandle);
   if (clientComponentPtr != NULL) {
      component_remove_import (clientComponentPtr, importPtr);
      component_unref (clientComponentPtr);
   }
   import_free (importPtr);

   TRACE3 (("exiting"));
}

xc_result_t
xCOM_Import (
   xc_handle_t importHandle,
   xc_interface_t **interfacePtr
) {

   xc_result_t result;

   TRACE3 (("called with importHandle=%u, interfacePtr=%p", importHandle, interfacePtr));

   if (importHandle == XC_INVALID_HANDLE) {
      TRACE1 (("Invalid parameters!"));
      result = XC_ERR_INVAL;
   }
   else {
      result = import_open (importHandle, interfacePtr);
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xCOM_UnImport (
   xc_handle_t importHandle
) {

   import_t *importPtr;
   xc_result_t result;

   TRACE3 (("called with importHandle=%u", importHandle));

   if (importHandle == XC_INVALID_HANDLE) {
      TRACE1 (("Invalid parameters!"));
      result = XC_ERR_INVAL;
   }
   else {
      import_module_lock ();
      importPtr = handle_dir_get (importHandles, importHandle);
      if (importPtr != NULL) {
         import_close (importPtr);
         result = XC_OK;
      }
      else {
         TRACE1 (("Invalid import handle (%u)!", importHandle));
         result = XC_ERR_NOENT;
      }
      import_module_unlock ();
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
import_set_next (
   import_t *importPtr,
   import_t *nextImportPtr
) {

   TRACE3 (("called with importPtr=%p, nextImportPtr=%p", importPtr, nextImportPtr));
   assert (importPtr != NULL);
   assert (nextImportPtr != NULL);
   assert (importPtr->nextImportPtr == NULL);
   assert (nextImportPtr->previousImportPtr == NULL);
   import_module_lock ();

   /* Now do the requested linkage. */
   importPtr->nextImportPtr = nextImportPtr;
   nextImportPtr->previousImportPtr = importPtr;

   import_module_unlock ();
   TRACE3 (("exiting"));
}

void
import_free_unused (
   import_t *importPtr
) {
   import_t *currentImportPtr;

   TRACE3 (("called with importPtr=%p", importPtr));
   assert (importPtr != NULL);
   import_module_lock ();

   /* Free any unused imports from the previous list. */
   currentImportPtr = importPtr->previousImportPtr;
   while (currentImportPtr != NULL) {
      import_t *previousImportPtr = currentImportPtr->previousImportPtr;
      if (currentImportPtr->state == IMPORT_STATE_QUERIED) {
         import_free (currentImportPtr);
      }
      currentImportPtr = previousImportPtr;
   }

   /* Free any unused imports from the next list (including initial import if unused). */
   currentImportPtr = importPtr;
   while (currentImportPtr != NULL) {
      import_t *nextImportPtr = currentImportPtr->nextImportPtr;
      if (currentImportPtr->state == IMPORT_STATE_QUERIED) {
         import_free (currentImportPtr);
      }
      currentImportPtr = nextImportPtr;
   }

   import_module_unlock ();
   TRACE3 (("exiting"));
}

xc_interface_t *
xCOM_GetSwitch (
   xc_handle_t importHandle
) {
   import_t *importPtr;
   xc_interface_t *switchPtr = NULL;

   TRACE3 (("called with importHandle=%u", importHandle));
   import_module_lock ();

   importPtr = handle_dir_get (importHandles, importHandle);
   if ((importPtr != NULL) && (importPtr->state = IMPORT_STATE_OPENED)) {
      switchPtr = importPtr->interfacePtr;
   }
   else {
      TRACE1 ((
         "invalid import handle (%u) or state (%d)!",
         importHandle, (importPtr != NULL) ? (int) importPtr->state : -1
      ));
   }

   import_module_unlock ();
   TRACE3 (("exiting with result=%p", switchPtr));
   return switchPtr;
}

xc_handle_t
xCOM_GetNextImport (
   xc_handle_t importHandle
) {
   import_t *importPtr;
   xc_handle_t nextHandle = XC_INVALID_HANDLE;

   TRACE3 (("called with importHandle=%u", importHandle));
   import_module_lock ();

   importPtr = handle_dir_get (importHandles, importHandle);
   if ((importPtr != NULL) && (importPtr->state = IMPORT_STATE_OPENED)) {
      importPtr = importPtr->nextImportPtr;
      if (importPtr != NULL) {
         nextHandle = importPtr->importHandle;
      }
   }

   import_module_unlock ();
   TRACE3 (("exiting with result=%u", nextHandle));
   return nextHandle;
}

