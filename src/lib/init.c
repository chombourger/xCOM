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

#include "internal.h"

/** Track the number of xCOM_Init() calls. */
static unsigned int initialized = 0;

/** Global xCOM lock. */
static pthread_mutex_t globalLock = PTHREAD_MUTEX_INITIALIZER;

#ifdef GLIB_ENABLED
static GMainLoop *mainLoop = NULL;
#else
static sem_t runSem;
#endif

void
xcom_lock (
   void
) {
   TRACE3 (("called"));
   pthread_mutex_lock (&globalLock);
   TRACE3 (("exiting"));
}

void
xcom_unlock (
   void
) {
   TRACE3 (("called"));
   pthread_mutex_unlock (&globalLock);
   TRACE3 (("exiting"));
}

xc_result_t
xCOM_Init (
   void
) {
   xc_result_t result = XC_OK;

   TRACE3 (("called"));

   xcom_lock ();

   if (initialized == 0) {
      trace_init ();

#ifdef GLIB_ENABLED
      mainLoop = g_main_loop_new (NULL, FALSE);
      if (mainLoop == NULL) result = XC_ERR_NOMEM;
#else
      sem_init (&runSem, 0, 0);
#endif

      /* Initialize internal sub-modules. */
      if ((result != XC_OK) ||
          ((result = ports_module_init ()) != XC_OK) ||
          ((result = component_module_init ()) != XC_OK) ||
          ((result = query_init ()) != XC_OK) ||
          ((result = import_init ()) != XC_OK)) {
         TRACE4 (("failed to initialize sub-modules, clean-up needed."));
         component_module_destroy ();
         ports_module_destroy ();
         import_destroy ();
         query_destroy ();
      }
   }

   if (result == XC_OK) {
      initialized ++;
      TRACE4 (("initialized %u time(s)", initialized));
   }
   else {
      TRACE1 (("initialization failed (%d)!", result));
   }

   xcom_unlock ();

   TRACE3 (("exiting with result=%d", result));
   return result;
}

void
xCOM_Destroy (
   void
) {

   TRACE3 (("called"));

   xcom_lock ();
   assert (initialized > 0);
   initialized --;

   if (initialized == 0) {
      component_module_destroy ();
      ports_module_destroy ();
      import_destroy ();
      query_destroy ();
#ifdef GLIB_ENABLED
      g_main_loop_unref (mainLoop);
#else
      sem_destroy (&runSem);
#endif
   }

   xcom_unlock ();
   TRACE3 (("exiting"));
}

xc_result_t
xCOM_Exec (
   void
) {
   xc_result_t result;

   TRACE3 (("called"));

#ifdef GLIB_ENABLED
   TRACE4 (("execute glib main loop"));
   g_main_loop_run (mainLoop);
   result = XC_OK;
#else
   do {
      result = sem_wait (&runSem);
   } while ((result == -1) && (errno = EINTR));

   if (result != XC_OK) {
      result = XC_ERR_INVAL;
   }
#endif

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xCOM_Quit (
   void
) {
   xc_result_t result;

   TRACE3 (("called"));

#ifdef GLIB_ENABLED
   TRACE4 (("quitting glib main loop"));
   g_main_loop_quit (mainLoop);
   result = XC_OK;
#else
   result = sem_post (&runSem);
#endif

   if (result != XC_OK) {
      result = XC_ERR_INVAL;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

