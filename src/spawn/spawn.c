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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <getopt.h>

#include <xCOM.h>
#include <xCOM/semaphore.h>

#include "component.h"

static void
service_result (
   xc_handle_t importHandle,
   xc_result_t result,
   void *user_data
) {
   xc_sem_t *waitSem = user_data;
   importHandle = importHandle;
   result = result;
   xc_sem_signal (waitSem);
}

static void
service_error (
   xc_handle_t importHandle,
   xc_result_t result,
   void *user_data
) {
   xc_sem_t *waitSem = user_data;
   importHandle = importHandle;
   fprintf (stderr, "warning: service failed to start/stop (%d)\n", result);
   xc_sem_signal (waitSem);
}

static void
application_start_result (
   xc_handle_t importHandle,
   xc_result_t result,
   void *user_data
) {
   importHandle = importHandle;
   user_data = user_data;
   printf ("application exited with result=%d\n", result);
   xCOM_Quit ();
}

int
main (
   int    argc,
   char **argv
) {

   xc_handle_t queryHandle;
   xc_handle_t *serviceHandles = NULL;
   xc_handle_t applicationHandle;
   const char *appName = NULL;
   bool appImported = false;
   unsigned int i, servicesCount=0, appCount=0;
   xcom_iapplication_t *appImpl;
   xcom_iservice_t **serviceImpls = NULL;
   unsigned int flags = XC_LOADF_NONE;
   xc_sem_t serviceSem;
   xc_result_t result;

   while (1) {
      int option_index = 0, c;
      static struct option long_options[] = {
         { "ignore-caches", no_argument, 0, 'i' },
         { 0, 0, 0, 0 }
      };

      c = getopt_long (argc, argv, "i", long_options, &option_index);
      if (c == -1) {
         break;
      }

      switch (c) {
         /* -i, --ignore-caches */
         case 'i':
            flags |= XC_LOADF_IGNORE_CACHES;
            break;
         /* Unknown option. */
         case '?':
            exit (1);
      }
   }

   result = xCOM_Init ();
   if (result == XC_OK) {
      /* Load bundles provided on the command line. */
      for (i=optind; i<(unsigned int)argc; i++) {
         result = xCOM_LoadComponentBundle (argv[i], flags);
         if ((result == XC_ERR_NOENT) && (i==(unsigned int)(argc-1))) {
            appName = argv[i];
            result = XC_OK;
         }
         else if (result != XC_OK) {
            fprintf (stderr, "%s: failed to load component bundle (%d)!\n", argv[i], result);
            break;
         }
      }

      /* Query and load components implementing xcom.IService. */
      if (result == XC_OK) {
         xc_sem_init (&serviceSem, 0);
         result = xCOM_QueryInterface (
            XC_INVALID_HANDLE,
            XCOM_ISERVICE_NAME,
            XCOM_ISERVICE_VERSION_MAJOR,
            XCOM_ISERVICE_VERSION_MINOR,
            NULL,
            NULL,
            XC_QUERYF_NONE,
            &queryHandle,
            &servicesCount
         );
         if (result == XC_OK) {
            if (servicesCount > 0) {
               serviceHandles = (xc_handle_t *) malloc (sizeof (*serviceHandles) * servicesCount);
               serviceImpls = (xcom_iservice_t **) malloc (sizeof (*serviceImpls) * servicesCount);
               if ((serviceHandles != NULL) && (serviceImpls != NULL)) {
                  for (i=0; i<servicesCount; i++) {
                     xc_handle_t serviceHandle;
                     serviceHandles[i] = XC_INVALID_HANDLE;
                     result = xCOM_QueryNext (queryHandle, &serviceHandle);
                     if (result == XC_OK) {
                        result = xCOM_Import (serviceHandle, (xc_interface_t **) &serviceImpls[i]);
                        if (result == XC_OK) {
                           serviceHandles[i] = serviceHandle;
                           result = serviceImpls[i]->Start (serviceHandles[i], service_result, service_error, &serviceSem);
                           if (result == XC_OK) {
                              xc_sem_wait (&serviceSem);
                           }
                        }
                        else {
                           /* FIXME print the name of the service we failed to load. */
                           fprintf (stderr, "warning: failed to load service\n");
                           result = XC_OK;
                        }
                     }
                  }
               }
               else {
                  /* One of serviceHandles/serviceImpls may not be NULL! */
                  free (serviceHandles); serviceHandles=NULL;
                  free (serviceImpls); serviceImpls=NULL;

                  fprintf (stderr, "%s: out of memory!\n", argv [0]);
                  result = XC_ERR_NOMEM;
               }
            }
            xCOM_QueryFinish (queryHandle);
         }
      }

      /* Query and load a component implementing xcom.IApplication. */
      if (result == XC_OK) {
         result = xCOM_QueryInterface (
            XC_INVALID_HANDLE,
            XCOM_IAPPLICATION_NAME,
            XCOM_IAPPLICATION_VERSION_MAJOR,
            XCOM_IAPPLICATION_VERSION_MINOR,
            appName,
            NULL,
            XC_QUERYF_NONE,
            &queryHandle,
            &appCount
         );

         if (result == XC_OK) {
            if ((appCount == 0) && (appName != NULL)) {
               fprintf (stderr, "%s: %s: application not found!\n", argv[0], appName);
               result = XC_ERR_NOENT;
            }
            for (i=0; i<appCount; i++) {
               result = xCOM_QueryNext (queryHandle, &applicationHandle);
               if (result == XC_OK) {
                  result = xCOM_Import (applicationHandle, (xc_interface_t **) &appImpl);
                  if (result == XC_OK) {
                     (void) appImpl->Start (applicationHandle, application_start_result, application_start_result, NULL);
                     appImported = true;
                     break;
                  }
               }
            }
            xCOM_QueryFinish (queryHandle);
         }
      }

      if (result == XC_OK) {
         xCOM_Exec ();
      }

      /* Wait for the application thread to return. */
      if (appImported == true) {
         (void) xCOM_UnImport (applicationHandle);
      }

      /* Stop and unimport services. */
      for (i=0; i<servicesCount; i++) {
         if (serviceHandles[i] != XC_INVALID_HANDLE) {
            result = serviceImpls[i]->Stop (serviceHandles[i], service_result, service_error, &serviceSem);
            if (result == XC_OK) {
               xc_sem_wait (&serviceSem);
            }
            (void) xCOM_UnImport (serviceHandles[i]);
         }
      }
      free (serviceHandles); serviceHandles=NULL;
      free (serviceImpls); serviceImpls=NULL;
      xc_sem_destroy (&serviceSem);

      xCOM_Destroy ();
   }

   return (int) result;
}

