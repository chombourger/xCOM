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

#define  TRACE_CLASS_DEFAULT COMPONENT
#include "internal.h"

/** Directory of known xCOM components. */
static void *directory = NULL;

/** List of known xCOM component bundles. */
static xc_clist_t bundles;

/** Handle of the special anonymous component. */
static xc_handle_t anonymousHandle = XC_INVALID_HANDLE;

/** Pointer to the anonymous component. */
static component_t *anonymousPtr = NULL;

/** Name of the special anonymous component. */
static const char *anonymousName = "<anonymous>";

/** Lock for the component module. */
static pthread_mutex_t lock;

#define IS_EXEC(m) (((m) & S_IXUSR) || ((m) & S_IXGRP) || ((m) & S_IXOTH))

xc_result_t
component_module_init (
   void
) {
   pthread_mutexattr_t attr;
   xc_result_t result = XC_OK;

   TRACE3 (("called"));

   pthread_mutexattr_init (&attr);
   pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init (&lock, &attr);

   XC_CLIST_INIT (&bundles);
   directory = handle_dir_new ();
   if (directory != NULL) {
      anonymousPtr = component_new (NULL);
      if (anonymousPtr != NULL) {
         component_set_name (anonymousPtr, anonymousName);
         anonymousPtr->path = (char *) anonymousName;
         anonymousHandle = anonymousPtr->id;
      }
      else {
         TRACE1 (("failed to create anonymous component!"));
         result = XC_ERR_NOMEM;
      }
   }
   else {
      TRACE1 (("failed to create component directory!"));
      result = XC_ERR_NOMEM;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
component_module_destroy (
   void
) {
   bundle_t *bundlePtr;

   TRACE3 (("called"));
   pthread_mutex_lock (&lock);

   /* Free all component bundles. */
   bundlePtr = XC_CLIST_HEAD (&bundles);
   while (!XC_CLIST_END (&bundles, bundlePtr)) {
      bundle_t *nextPtr = XC_CLIST_NEXT (bundlePtr);
      XC_CLIST_REMOVE (bundlePtr);
      bundle_free (bundlePtr);
      bundlePtr = nextPtr;
   }

   /* Free anonymous component. */
   component_free (anonymousPtr);
   anonymousPtr = NULL;

   if (directory != NULL) {
      handle_dir_destroy (directory);
      directory = NULL;
   }

   pthread_mutex_destroy (&lock);
   TRACE3 (("exiting"));
}

bundle_t *
bundle_new (
   const char *path
) {
   bundle_t *bundlePtr = NULL;
   int dirfd;

   TRACE3 (("called with path='%s'", path));
   assert (path != NULL);

   dirfd = open (path, O_RDONLY);
   if (dirfd >= 0) {
      bundlePtr = (bundle_t *) malloc (sizeof (*bundlePtr));
      if (bundlePtr != NULL) {
         bundlePtr->dirfd = dirfd;
         bundlePtr->path = strdup (path);
         if (bundlePtr->path != NULL) {
            XC_CLIST_INIT (&bundlePtr->components);
         }
         else {
            TRACE1 (("strdup() for the bundle path failed!"));
            free (bundlePtr);
            close (dirfd);
            bundlePtr = NULL;
         }
      }
   }

   TRACE3 (("exiting with result=%p", bundlePtr));
   return bundlePtr;
}

component_t *
component_new (
   bundle_t *bundlePtr
) {
   component_t *componentPtr;

   TRACE3 (("called with bundlePtr=%p", bundlePtr));

   componentPtr = (component_t *) malloc (sizeof (*componentPtr));
   if (componentPtr != NULL) {
      componentPtr->name = NULL;
      componentPtr->descr = NULL;
      componentPtr->bundlePtr = bundlePtr;
      componentPtr->handle = NULL;
      componentPtr->state = COMP_STATE_NULL;
      componentPtr->references = 0;
      componentPtr->portsCount = 0;
      componentPtr->ports = NULL;
      componentPtr->userData = NULL;
      XC_CLIST_INIT (&componentPtr->imports);
      componentPtr->id = handle_dir_push (directory, componentPtr);
      if (componentPtr->id != XC_INVALID_HANDLE) {
         TRACE4 (("registered component #%u", componentPtr->id));
         pthread_mutexattr_t attr;
         pthread_mutexattr_init (&attr);
         pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
         pthread_mutex_init (&componentPtr->lock, &attr);
      }
      else {
         TRACE1 (("failed to obtain a component handle!"));
         component_free (componentPtr);
         componentPtr = NULL;
      }
   }

   TRACE3 (("exiting with result=%p", componentPtr));
   return componentPtr;
}

void
component_free (
   component_t *componentPtr
) {

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);

   component_destroy (componentPtr);

   if (componentPtr->id != XC_INVALID_HANDLE) {
      handle_dir_remove (directory, componentPtr->id);
   }

   pthread_mutex_destroy (&componentPtr->lock);
   if (componentPtr->path != anonymousName) {
      free (componentPtr->path);
   }
   component_free_ports (componentPtr);
   component_set_name (componentPtr, NULL);
   component_set_descr (componentPtr, NULL);
   free (componentPtr);

   TRACE3 (("exiting"));
}

void
component_free_ports (
   component_t *componentPtr
) {
   port_t *portPtr;
   unsigned int i;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);

   for (i=0; i<componentPtr->portsCount; i++) {
      portPtr = componentPtr->ports[i];
      if (portPtr != NULL) {
         port_free (portPtr);
      }
   }

   free (componentPtr->ports);
   componentPtr->portsCount = 0;
   componentPtr->ports = NULL;

   TRACE3 (("exiting"));
}

xc_result_t
component_load (
   component_t *componentPtr
) {
   struct __xc_component_decl__ *declPtr;
   void *handle;
   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);

   if (componentPtr->state < COMP_STATE_LOADED) {
      assert (componentPtr->handle == NULL); /* stale handle */
      assert (componentPtr->path != NULL);

      /* Load shared library. */
      handle = dlopen (componentPtr->path, RTLD_LAZY);
      if (handle != NULL) {
         /* Log dlopen() result and clear any error. */
         componentPtr->handle = handle;
         componentPtr->state = COMP_STATE_LOADED;
         TRACE4 (("dlopen('%s') returned %p", componentPtr->path, handle));
         dlerror ();

         /* Get component declaration */
         declPtr = dlsym (handle, STRINGIFY (XC_DECLARED_COMPONENT_SYM));
         TRACE4 (("dlsym('%s') returned %p", STRINGIFY (XC_DECLARED_COMPONENT_SYM), declPtr));
         if (declPtr != NULL) {
            unsigned int i;
            componentPtr->init = declPtr->init;
            componentPtr->destroy = declPtr->destroy;
            TRACE4 (("%s: init at %p", componentPtr->path, componentPtr->init));
            TRACE4 (("%s: destroy at %p", componentPtr->path, componentPtr->destroy));
            for (i=0; i<componentPtr->portsCount; i++) {
               port_enable (componentPtr->ports[i], &declPtr->ports[i]);
            }
         }
         else {
            TRACE1 (("%s: %s", dlerror ()));
            result = XC_ERR_INVAL;
         }
      }
      else {
         TRACE1 (("%s: %s", dlerror ()));
         result = XC_ERR_INVAL;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_unload (
   component_t *componentPtr
) {
   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   if (componentPtr->state >= COMP_STATE_LOADED) {
      unsigned int i;
      for (i=0; i<componentPtr->portsCount; i++) {
         port_disable (componentPtr->ports[i]);
      }
      componentPtr->init = NULL;
      componentPtr->destroy = NULL;
      if (componentPtr->handle != NULL) {
         dlclose (componentPtr->handle);
         componentPtr->handle = NULL;
      }
      componentPtr->state = COMP_STATE_NULL;
      componentPtr->userData = NULL;
   }
   
   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

component_t *
component_introspect (
   bundle_t *bundlePtr,
   const char *filename
) {
   struct __xc_component_decl__ *declPtr;
   struct __xc_port_decl__ *portDeclPtr;
   component_t *componentPtr = NULL;
   unsigned int i;
   unsigned int ports;
   port_t *portPtr;
   char *libPath;
   void *handle;

   TRACE3 (("called with bundlePtr=%p, filename='%s'", bundlePtr, filename));
   assert (bundlePtr != NULL);
   assert (bundlePtr->path != NULL);
   assert (filename != NULL);

   libPath = (char *) malloc (PATH_MAX);
   if (libPath != NULL) {
      sprintf (libPath, "%s/" CODE_FOLDER "/" XC_HOST "/%s", bundlePtr->path, filename);
      handle = dlopen (libPath, RTLD_LAZY);
      if (handle != NULL) {
         /* Log dlopen() result and clear any error. */
         TRACE4 (("dlopen('%s') returned %p", libPath, handle));
         dlerror ();

         declPtr = dlsym (handle, STRINGIFY (XC_DECLARED_COMPONENT_SYM));
         TRACE4 (("dlsym('%s') returned %p", STRINGIFY (XC_DECLARED_COMPONENT_SYM), declPtr));
         if (declPtr != NULL) {
            componentPtr = component_new (bundlePtr);
            if (componentPtr != NULL) {
               componentPtr->path = libPath;
               component_set_name (componentPtr, declPtr->name);
               component_set_descr (componentPtr, declPtr->descr);
               componentPtr->vmajor = declPtr->vmaj;
               componentPtr->vminor = declPtr->vmin;
               TRACE4 (("%s: named as '%s'", libPath, componentPtr->name));

               /* Count number of ports declared. */
               ports = 0;
               for (portDeclPtr = &declPtr->ports[0]; portDeclPtr->name != NULL; portDeclPtr ++) {
                  ports ++;
               }

               componentPtr->ports = malloc (sizeof (port_t *) * ports);
               if (componentPtr->ports != NULL) {
                  componentPtr->portsCount = ports;
                  memset (componentPtr->ports, 0, sizeof (port_t *) * ports);
                  for (i=0; i<ports; i++) {
                     portDeclPtr = &declPtr->ports[i];
                     TRACE4 (("%s: port '%s'", libPath, portDeclPtr->name));
                     portPtr = port_new (componentPtr->id, portDeclPtr);
                     if (portPtr != NULL) {
                        componentPtr->ports[i] = portPtr;
                     }
                     else {
                        component_free (componentPtr);
                        componentPtr = NULL;
                        break;
                     }
                  }
               }
               libPath = NULL;
               /* Dump component meta-data to the cache. */
               (void) cache_create (componentPtr, filename);
            }
         }
      }
      else {
          TRACE1 (("dlopen('%s') failed: %s", libPath, dlerror ()));
      }
      free (libPath);
   }
   else {
      TRACE1 (("Out of memory!"));
   }

   TRACE3 (("exiting with result=%p", componentPtr));
   return componentPtr;
}

xc_result_t
bundle_load_components (
   bundle_t *bundlePtr,
   unsigned int flags
) {
   xc_result_t result;
   struct stat st;
   char *codePath;
   DIR *dir;
   struct dirent *ent;
   component_t *componentPtr;
   int fd;

   TRACE3 (("called with bundlePtr=%p, flags=0x%x", bundlePtr, flags));
   assert (bundlePtr != NULL);

   /* Construct path to component code. */
   TRACE4 (("XC_HOST is '%s'", XC_HOST));
   codePath = (char *) malloc (PATH_MAX);

   if (codePath != NULL) {
      sprintf (codePath, "%s/%s/%s", bundlePtr->path, CODE_FOLDER, XC_HOST);
      TRACE4 (("trying to load component code from '%s'", codePath));

      /* Open the Code directory for the built target (XC_HOST). */
      dir = opendir (codePath);
      if (dir != NULL) {
         fd = dirfd (dir);
         assert (fd >= 0);

         result = XC_ERR_INVAL;
         while ((ent = readdir (dir)) != NULL) {
            result = fstatat (fd, ent->d_name, &st, AT_SYMLINK_NOFOLLOW);
            if ((result == 0) && (S_ISREG (st.st_mode)) && (IS_EXEC (st.st_mode))) {
               TRACE4 (("loading '%s'", ent->d_name));
               componentPtr = NULL;
               if ((flags & XC_LOADF_IGNORE_CACHES) == 0) {
                  componentPtr = cache_open (bundlePtr, ent->d_name);
               }
               if (componentPtr == NULL) {
                  componentPtr = component_introspect (bundlePtr, ent->d_name);
               }
               if (componentPtr != NULL) {
                  XC_CLIST_ADDTAIL (&bundlePtr->components, componentPtr);
                  result = XC_OK;
               }
               else {
                  result = XC_ERR_INVAL;
               }
            }
         }
         closedir (dir);

         if (!XC_CLIST_EMPTY (&bundlePtr->components)) {
            result = XC_OK;
         }
         else {
            TRACE1 (("no code was loaded!"));
            result = XC_ERR_INVAL;
         }
      }
      else {
         TRACE1 (("failed to open directory '%s'!", codePath));
         result = XC_ERR_NODIR;
      }

      free (codePath);
   }
   else {
      TRACE1 (("failed to allocate memory!"));
      result = XC_ERR_NOMEM;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;   
}

xc_result_t
bundle_unload_components (
   bundle_t *bundlePtr
) {
   component_t *componentPtr, *nextPtr;
   xc_result_t result = XC_OK;

   TRACE3 (("called with bundlePtr=%p", bundlePtr));
   assert (bundlePtr != NULL);

   componentPtr = XC_CLIST_HEAD (&bundlePtr->components);
   while (!XC_CLIST_END (&bundlePtr->components, componentPtr)) {
      nextPtr = XC_CLIST_NEXT (componentPtr);
      XC_CLIST_REMOVE (componentPtr);
      component_free (componentPtr);
      componentPtr = nextPtr;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

component_t *
component_ref (
   xc_handle_t componentHandle
) {
   component_t *componentPtr;

   TRACE3 (("called with componentHandle=%u", componentHandle));

   pthread_mutex_lock (&lock);
   componentPtr = (component_t *) handle_dir_get (directory, componentHandle);
   if (componentPtr != NULL) {
      componentPtr->references ++;
      TRACE4 (("%s: %u references", componentPtr->path, componentPtr->references));
   }
   pthread_mutex_unlock (&lock);

   TRACE3 (("exiting with result=%p", componentPtr));
   return componentPtr;
}

void
component_unref (
   component_t *componentPtr
) {

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&lock);
   assert (componentPtr->references > 0);

   componentPtr->references --;
   TRACE4 (("%s: %u references", componentPtr->path, componentPtr->references));

   if (componentPtr->references == 0) {
      component_destroy (componentPtr);
   }

   pthread_mutex_unlock (&lock);
   TRACE3 (("exiting"));
}

void
bundle_free (
   bundle_t *bundlePtr
) {
   component_t *componentPtr;

   TRACE3 (("called with bundlePtr=%p", bundlePtr));
   assert (bundlePtr != NULL);

   /* Unload components of this bundle. */
   componentPtr = XC_CLIST_HEAD (&bundlePtr->components);
   while (!XC_CLIST_END (&bundlePtr->components, componentPtr)) {
      component_t *nextPtr = XC_CLIST_NEXT (componentPtr);
      XC_CLIST_REMOVE (componentPtr);
      component_free (componentPtr);
      componentPtr = nextPtr;
   }

   /* Unregister bundle. */
   XC_CLIST_REMOVE (bundlePtr);

   /* Free-up remaining resources. */
   bundle_unload_components (bundlePtr);
   free (bundlePtr->path);
   close (bundlePtr->dirfd);
   free (bundlePtr);

   TRACE3 (("exiting"));
}

void
component_add_import (
   component_t *componentPtr,
   import_t *importPtr
) {

   TRACE3 (("called with componentPtr=%p, importPtr=%p", componentPtr, importPtr));
   assert (componentPtr != NULL);
   assert (importPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   XC_CLIST_ADDTAIL (&componentPtr->imports, importPtr);

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting"));
}

void
component_remove_import (
   component_t *componentPtr,
   import_t *importPtr
) {

   TRACE3 (("called with componentPtr=%p, importPtr=%p", componentPtr, importPtr));
   assert (componentPtr != NULL);
   assert (importPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   XC_CLIST_REMOVE (importPtr);

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting"));
}


xc_handle_t
component_get_anonymous (
   void
) {
   xc_handle_t result;

   TRACE3 (("called"));

   pthread_mutex_lock (&lock);
   result = anonymousHandle;
   pthread_mutex_unlock (&lock);

   TRACE3 (("exiting with result=%u", result));
   return result;
}

xc_result_t
component_loadtime_imports (
   component_t *componentPtr
) {

   unsigned int i;
   port_t *portPtr;
   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);

   for (i=0; i<componentPtr->portsCount; i++) {
      portPtr = componentPtr->ports[i];
      if (XC_PORTF_IS_LOADTIME (portPtr->flags)) {
         TRACE4 ((
            "%s: %s: loadtime import of %s (%u.%u)",
            componentPtr->name, portPtr->name,
            portPtr->interfaceSpec.name,
            portPtr->interfaceSpec.vmajor,
            portPtr->interfaceSpec.vminor
         ));
         result = component_loadtime_import (componentPtr, portPtr);
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_loadtime_import (
   component_t *componentPtr,
   port_t *portPtr
) {
   xc_handle_t queryHandle;
   xc_handle_t importHandle;
   unsigned int matchCount;
   unsigned int i;
   xc_result_t result;

   TRACE3 (("called with componentPtr=%p, portPtr=%p", componentPtr, portPtr));
   assert (componentPtr != NULL);
   assert (portPtr != NULL);

   if (portPtr->importHandlePtr != NULL) {
      *portPtr->importHandlePtr = XC_INVALID_HANDLE;
   }

   result = query_new (
      componentPtr->id,
      portPtr->name,
      portPtr->interfaceSpec.name,
      portPtr->interfaceSpec.vmajor,
      portPtr->interfaceSpec.vminor,
      portPtr->componentName,
      portPtr->port,
      XC_QUERYF_NONE,
      &queryHandle,
      &matchCount
   );

   /* Query succeeded but no results? */
   if ((result == XC_OK) && (matchCount == 0)) {
      /* Set error, may be cleared later if the import is optional. */
      result = XC_ERR_NOENT;
   }

   if (result == XC_OK) {
      for (i=0; i<matchCount; i++) {
         result = xCOM_QueryNext (queryHandle, &importHandle);
         if (result == XC_OK) {
            result = xCOM_Import (importHandle, NULL);
            if (result == XC_OK) {
               if ((portPtr->importHandlePtr != NULL) &&
                   (*portPtr->importHandlePtr == XC_INVALID_HANDLE)) {
                  TRACE4 (("set import handle %u at %p", importHandle, portPtr->importHandlePtr));
                  *portPtr->importHandlePtr = importHandle;
               }
               /* If this is not a multiple import, we can then stop with the first successful import. */
               if (!XC_PORTF_IS_MULTIPLE (portPtr->flags)) {
                  break;
               }
            }
         }
      }
      (void) xCOM_QueryFinish (queryHandle);
   }

   /* Do not throw an error if the import was optional. */
   if ((result != XC_OK) && (XC_PORTF_IS_OPTIONAL (portPtr->flags))) {
      TRACE2 (("%s: optional import from port '%s' failed", componentPtr->name, portPtr->name));
      result = XC_OK;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_close_imports (
   component_t *componentPtr
) {

   import_t *importPtr;
   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   importPtr = XC_CLIST_HEAD (&componentPtr->imports);
   while (!XC_CLIST_END (&componentPtr->imports, importPtr)) {
      import_t *nextPtr = XC_CLIST_NEXT (importPtr);
      XC_CLIST_REMOVE (importPtr);
      import_free (importPtr);
      importPtr = nextPtr;
   }

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_init (
   component_t *componentPtr
) {

   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);

   pthread_mutex_lock (&componentPtr->lock);

   component_load (componentPtr);

   if (componentPtr->state < COMP_STATE_INITIALIZED) {

      /* Process its loadtime imports. */
      result = component_loadtime_imports (componentPtr);

      /* Call its init() method. */
      if ((result == XC_OK) && (componentPtr->init != NULL)) {
         TRACE2 (("init'ing '%s'", componentPtr->path));
         result = componentPtr->init (componentPtr->id);
      }
   }

   if (result == XC_OK) {
      componentPtr->state = COMP_STATE_INITIALIZED;
      printf (
         "%s component version %u.%u (%s)\n",
         componentPtr->name,
         componentPtr->vmajor, componentPtr->vminor,
         componentPtr->descr
      );
   }
   else {
      TRACE1 ((
         "initialization failed (%d), closing imports of %s",
         result, componentPtr->name
      ));
      component_close_imports (componentPtr);
   }
 
   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_destroy (
   component_t *componentPtr
) {

   bool initialized = false;
   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p", componentPtr));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   if (componentPtr->state == COMP_STATE_INITIALIZED) {
      /* Set to unloading state and remember that component was initialized. */
      componentPtr->state = COMP_STATE_UNLOADING;
      initialized = true;

      /* Close imports done by this component. */
      (void) component_close_imports (componentPtr);

      if (componentPtr->destroy) {
         (void) componentPtr->destroy (componentPtr->id);
      }
   }

   componentPtr-> state = COMP_STATE_LOADED;
   (void) component_unload (componentPtr);

   if (initialized == true) {
      printf (
         "%s version %u.%u component unloaded.\n",
         componentPtr->name,
         componentPtr->vmajor, componentPtr->vminor
      );
   }
 
   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_register (
   component_t *componentPtr,
   port_t *portPtr,
   xc_handle_t importHandle
) {

   xc_result_t result = XC_OK;

   TRACE3 ((
      "called with componentPtr=%p, portPtr=%p, importHandle=%u",
      componentPtr, portPtr, importHandle
   ));
   assert (componentPtr != NULL);
   assert (portPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   /* Make sure the component is loaded. */
   result = component_load (componentPtr);

   /* Call port's register() method. */
   if ((result == XC_OK) && (portPtr->onRegister != NULL)) {
      result = portPtr->onRegister (componentPtr->id, importHandle);
   }

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_unregister (
   component_t *componentPtr,
   port_t *portPtr,
   xc_handle_t importHandle
) {

   xc_result_t result = XC_OK;

   TRACE3 ((
      "called with componentPtr=%p, portPtr=%p, importHandle=%u",
      componentPtr, portPtr, importHandle
   ));
   assert (componentPtr != NULL);
   assert (portPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   /* Call port's unregister() method. */
   if (portPtr->onUnRegister != NULL) {
      result = portPtr->onUnRegister (componentPtr->id, importHandle);
   }

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_set_name (
   component_t *componentPtr,
   const char *name
) {

   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p, name='%s'", componentPtr, name));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   if (name != NULL) {
      componentPtr->name = strdup (name);
      if (componentPtr->name == NULL) {
         TRACE1 (("Out of memory!"));
         result = XC_ERR_NOMEM;
      }
   }
   else {
      free (componentPtr->name);
      componentPtr->name = NULL;
   }

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
component_set_descr (
   component_t *componentPtr,
   const char *descr
) {

   xc_result_t result = XC_OK;

   TRACE3 (("called with componentPtr=%p, descr='%s'", componentPtr, descr));
   assert (componentPtr != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   if (descr != NULL) {
      componentPtr->descr = strdup (descr);
      if (componentPtr->descr == NULL) {
         TRACE1 (("Out of memory!"));
         result = XC_ERR_NOMEM;
      }
   }
   else {
      free (componentPtr->descr);
      componentPtr->descr = NULL;
   }

   pthread_mutex_unlock (&componentPtr->lock);
   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xCOM_LoadComponentBundle (
   const char *path,
   unsigned int flags
) {
   xc_result_t result = XC_OK;
   bundle_t *bundlePtr = NULL;
   struct stat st;

   TRACE3 (("called with path='%s', flags=0x%x", path, flags));

   if (path != NULL) {
      /* Check that the specified path exists. */
      if ((stat (path, &st)) == 0) {
         /* Path shall be a directory. */
         if (S_ISDIR (st.st_mode)) {
            /* Register this component. */
            pthread_mutex_lock (&lock);
            bundlePtr = bundle_new (path);
            if (bundlePtr != NULL) {
               XC_CLIST_ADDTAIL (&bundles, bundlePtr);
               result = bundle_load_components (bundlePtr, flags);
            }
            pthread_mutex_unlock (&lock);
         }
         else {
            TRACE1 (("%s: not a directory!", path));
            result = XC_ERR_NODIR;
         }
      }
      else {
         int err = errno;
         TRACE1 (("stat() of '%s' failed (%d)!", path, err));
         result = XC_ERR_NOENT;
      }
   }
   else {
      TRACE1 (("path cannot be NULL!"));
      result = XC_ERR_INVAL;
   }

   /* Clean-up on error. */
   if (result != XC_OK) {
      if (bundlePtr != NULL) {
         pthread_mutex_lock (&lock);
         bundle_free (bundlePtr);
         pthread_mutex_unlock (&lock);
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

port_t *
component_ref_port (
   component_t *componentPtr,
   const char *name
) {
   port_t *portPtr = NULL;
   unsigned int i;

   TRACE3 (("called with componentPtr=%p, name='%s'", componentPtr, name));
   assert (componentPtr != NULL);
   assert (name != NULL);
   pthread_mutex_lock (&componentPtr->lock);

   for (i=0; i<componentPtr->portsCount; i++) {
      if (strcmp (componentPtr->ports[i]->name, name) == 0) {
         portPtr = componentPtr->ports[i];
         break;
      }
   }

   if (portPtr == NULL) {
      TRACE2 (("%s: port '%s' was not found!", componentPtr->name, name));
      pthread_mutex_unlock (&componentPtr->lock);
   }

   TRACE3 (("exiting with result=%p", portPtr));
   return portPtr;
}

void
component_unref_port (
   component_t *componentPtr,
   port_t *portPtr
) {

   TRACE3 (("called with componentPtr=%p, portPtr=%p", componentPtr, portPtr));
   assert (componentPtr != NULL);
   assert (portPtr != NULL);

   pthread_mutex_unlock (&componentPtr->lock);

   TRACE3 (("exiting"));
}

const char *
xCOM_GetComponentBundlePath (
   xc_handle_t componentHandle
) {
   component_t *componentPtr;
   const char *result = NULL;

   TRACE3 (("called with componentHandle=%u", componentHandle));

   componentPtr = component_ref (componentHandle);
   if (componentPtr != NULL) {
      result = componentPtr->bundlePtr->path;
      component_unref (componentPtr);
   }

   TRACE3 (("exiting with result='%s'", result));
   return result;
}

xc_result_t
xCOM_SetSpecific (
   xc_handle_t componentHandle,
   void *user_data
) {
   component_t *componentPtr;
   xc_result_t result;

   TRACE3 (("called with componentHandle=%u", componentHandle));

   componentPtr = component_ref (componentHandle);
   if (componentPtr != NULL) {
      componentPtr->userData = user_data;
      component_unref (componentPtr);
      result = XC_OK;
   }
   else {
      TRACE1 (("invalid component handle %u", componentHandle));
      result = XC_ERR_NOENT;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void *
xCOM_GetSpecific (
   xc_handle_t componentHandle
) {
   component_t *componentPtr;
   void *result;

   TRACE3 (("called with componentHandle=%u", componentHandle));

   componentPtr = component_ref (componentHandle);
   if (componentPtr != NULL) {
      result = componentPtr->userData;
      component_unref (componentPtr);
   }
   else {
      TRACE1 (("invalid component handle %u", componentHandle));
      result = NULL;
   }

   TRACE3 (("exiting with result=%p", result));
   return result;
}

