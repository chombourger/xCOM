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

#define  TRACE_CLASS_DEFAULT PORTS
#include "internal.h"

/** Hash of ProvidedPortsList with interface name as the key. */
static void *providersHash = NULL;

/** Lock to protect providersHash from concurrent accesses. */
static pthread_mutex_t lock;

xc_result_t
ports_module_init (
   void
) {
   xc_result_t result;

   TRACE3 (("called"));

   providersHash = hashtable_create ();
   if (providersHash != NULL) {
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

void
ports_module_destroy (
   void
) {
   TRACE3 (("called"));

   if (providersHash != NULL) {
      hashtable_destroy (providersHash);
      providersHash = NULL;
      pthread_mutex_destroy (&lock);
   }

   TRACE3 (("exiting"));
}

port_t *
port_new (
   xc_handle_t componentHandle,
   struct __xc_port_decl__ *portDeclPtr
) {
   port_t *portPtr;
   xc_result_t result;

   TRACE3 (("called with componentHandle=%u, portDeclPtr=%p", componentHandle, portDeclPtr));
   assert (componentHandle != XC_INVALID_HANDLE);
   assert (portDeclPtr != NULL);

   portPtr = (port_t *) malloc (sizeof (*portPtr));
   if (portPtr != NULL) {
      portPtr->componentHandle = componentHandle;
      portPtr->interfaceSize = portDeclPtr->interfaceSize;
      portPtr->reserved = portDeclPtr->reserved;
      portPtr->flags = portDeclPtr->flags;
      portPtr->interfaceSpec.name = NULL;
      portPtr->interfaceSpec.vmajor = portDeclPtr->interfacePtr->vmajor;
      portPtr->interfaceSpec.vminor = portDeclPtr->interfacePtr->vminor;
      portPtr->name = NULL;
      portPtr->componentPtr = NULL;
      portPtr->componentName = NULL;
      portPtr->interfacePtr = NULL;
      portPtr->port = NULL;
      portPtr->onRegister = NULL;
      portPtr->onUnRegister = NULL;
      portPtr->importHandlePtr = NULL;

      if (portDeclPtr->name != NULL) {
         portPtr->name = strdup (portDeclPtr->name);
         if (portPtr->name == NULL) {
            port_free (portPtr);
            portPtr = NULL;
         }
      }

      if (portDeclPtr->interfacePtr->name != NULL) {
         portPtr->interfaceSpec.name = strdup (portDeclPtr->interfacePtr->name);
         if (portPtr->interfaceSpec.name == NULL) {
            port_free (portPtr);
            portPtr = NULL;
         }
      }

      if (portDeclPtr->component != NULL) {
         portDeclPtr->component = strdup (portDeclPtr->component);
         if (portDeclPtr->component == NULL) {
            port_free (portPtr);
            portPtr = NULL;
         }
      }

      if (portDeclPtr->port != NULL) {
         portPtr->port = strdup (portDeclPtr->port);
         if (portPtr->port == NULL) {
            port_free (portPtr);
            portPtr = NULL;
         }
      }

      /* Put provided ports into our providedPortsHash. */
      if ((portPtr != NULL) && (XC_PORTF_IS_PROVIDED (portPtr->flags))) {
         result = provided_ports_register (portPtr);
         if (result != XC_OK) {
            port_free (portPtr);
            portPtr = NULL;
         }
      }
   }
 
   TRACE3 (("exiting with result=%p", portPtr));
   return portPtr;
}

xc_result_t
port_enable (
   port_t *portPtr,
   struct __xc_port_decl__ *portDeclPtr
) {

   xc_result_t result = XC_OK;

   TRACE3 (("called with portPtr=%p, portDeclPtr=%p", portPtr, portDeclPtr));
   assert (portPtr != NULL);
   assert (portDeclPtr != NULL);

   portPtr->interfacePtr = portDeclPtr->interfacePtr;
   portPtr->onRegister = portDeclPtr->onRegister;
   portPtr->onUnRegister = portDeclPtr->onUnRegister;
   portPtr->importHandlePtr = portDeclPtr->importHandlePtr;

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
port_disable (
   port_t *portPtr
) {

   xc_result_t result = XC_OK;

   TRACE3 (("called with portPtr=%p", portPtr));
   assert (portPtr != NULL);

   portPtr->interfacePtr = NULL;
   portPtr->onRegister = NULL;
   portPtr->onUnRegister = NULL;
   portPtr->importHandlePtr = NULL;

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
port_free (
   port_t *portPtr
) {

   struct ProvidedPortsList *listPtr;

   TRACE3 (("called with portPtr=%p", portPtr));
   assert (portPtr != NULL);
   pthread_mutex_lock (&lock);

   /* Is this port a "provided interface" port. */
   if (XC_PORTF_IS_PROVIDED (portPtr->flags)) {
      /* Yes, remove it from the provided ports list/hash. */
      listPtr = provided_ports_ref (portPtr->interfaceSpec.name);
      if (listPtr != NULL) {
         XC_CLIST_REMOVE (portPtr);
         /* No more providers of this interface, remove the hash entry. */
         if (XC_CLIST_EMPTY (&listPtr->ports)) {
            TRACE4 (("no more ports providing %s, deleting from hash", listPtr->interface));
            free (listPtr->interface);
            pthread_mutex_destroy (&listPtr->lock);
            free (listPtr);
         }
         else {
            /* Some ports still provide this interface, just release the providers list. */
            provided_ports_unref (listPtr);
         }
      }
   }

   /* Detach from our parent component. */
   if (portPtr->componentPtr != NULL) {
      component_unref (portPtr->componentPtr);
   }

   /* Free remaining memory. */
   free (portPtr->componentName);
   free ((char *)portPtr->interfaceSpec.name);
   free (portPtr->port);
   free (portPtr->name);
   free (portPtr);

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting"));
}

struct ProvidedPortsList *
provided_ports_ref (
   const char *name
) {
   struct ProvidedPortsList *listPtr = NULL;
   xc_result_t result;

   TRACE3 (("called with name='%s'", name));
   assert (name != NULL);
   pthread_mutex_lock (&lock);

   result = hashtable_lookup (providersHash, name, (void **) &listPtr);
   if (result == XC_OK) {
      assert (listPtr != NULL);
      pthread_mutex_lock (&listPtr->lock);
   }
   else {
      assert (listPtr == NULL);
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%p", listPtr));
   return listPtr;
}

void
provided_ports_unref (
  struct ProvidedPortsList *listPtr
) {

   TRACE3 (("called with listPtr=%p", listPtr));
   assert (listPtr != NULL);

   pthread_mutex_unlock (&listPtr->lock);

   TRACE3 (("exiting"));
}

xc_result_t
provided_ports_register (
   port_t *portPtr
) {
   struct ProvidedPortsList *listPtr;
   const char *name;
   xc_result_t result = XC_OK;

   TRACE3 (("called with portPtr=%p", portPtr));
   assert (portPtr != NULL);
   assert ((portPtr->flags & XC_PORTF_PROVIDED) != 0);

   pthread_mutex_lock (&lock);

   /* Was a provider for this interface already registered? */
   name = portPtr->interfaceSpec.name;
   listPtr = provided_ports_ref (name);
   if (listPtr == NULL) {
      listPtr = (struct ProvidedPortsList *) malloc (sizeof (*listPtr));
      if (listPtr != NULL) {
         pthread_mutex_init (&listPtr->lock, NULL);
         pthread_mutex_lock (&listPtr->lock);
         XC_CLIST_INIT (&listPtr->ports);
         listPtr->interface = strdup (name); // needed?
         if (listPtr->interface != NULL) {
            result = hashtable_insert (providersHash, name, listPtr);
         }
         if (result != XC_OK) {
            free (listPtr->interface);
            pthread_mutex_destroy (&listPtr->lock);
            free (listPtr);
         }
      }
   }

   if (result == XC_OK) {
      assert (listPtr != NULL);
      XC_CLIST_ADDTAIL (&listPtr->ports, portPtr);
      provided_ports_unref (listPtr);
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

