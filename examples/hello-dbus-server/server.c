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

#include <xCOM.h>
#include <assert.h>
#include <stdio.h>

#include "component.h"
#include "hello-dbus.h"

static guint bus = 0;
static Hello *hello;
static examples_ihello_t *helloImpl = NULL;

static gboolean
on_handle_say (
   Hello *hello,
   GDBusMethodInvocation *invocation,
   const gchar *greeting,
   gpointer user_data
) {

   /* Call the actual IHello.Say() method. */
   helloImpl->Say (HelloImportHandle, greeting, NULL, NULL, NULL);

   /* Finish the D-Bus method call. */
   hello_complete_say (hello, invocation);
   return TRUE;
}

static void
on_bus_acquired (
   GDBusConnection *connection,
   const gchar *name,
   gpointer user_data) {

   GError *error;
   gboolean result;

   fprintf (stderr, "TODO on_bus_acquired\n");
   hello = hello_skeleton_new ();
   g_signal_connect (
      hello,
      "handle-say",
      G_CALLBACK (on_handle_say),
      NULL
   );

   error  = NULL;
   result = g_dbus_interface_skeleton_export (
      G_DBUS_INTERFACE_SKELETON (hello),
      connection,
      "/xcom/examples/hello",
      &error
   );
   if (!result) {
   }
}

static void
on_name_acquired (
   GDBusConnection *connection,
   const gchar *name,
   gpointer user_data) {
   fprintf (stderr, "acquired the name '%s'\n", name);
}

static void
on_name_lost (
   GDBusConnection *connection,
   const gchar *name,
   gpointer user_data) {
   fprintf (stderr, "lost the name '%s'\n", name);
}

/* examples.hello-dbus-server component init(). */
xc_result_t
examples_hello_dbus_server_init (
   xc_handle_t importHandle
) {
   helloImpl = (examples_ihello_t *) xCOM_GetSwitch (HelloImportHandle);
   assert (helloImpl != NULL);
   g_type_init ();
   return XC_OK;
}

/* xcom.IService.Start implementation for port Service */
xc_result_t
service_start (
   xc_handle_t importHandle,
   void (* Start_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void (* Start_error) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void *user_data
) {
   xc_result_t result;

   bus = g_bus_own_name (
      G_BUS_TYPE_SESSION,
      "xcom.examples",
      G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT
      | G_BUS_NAME_OWNER_FLAGS_REPLACE,
      on_bus_acquired,
      on_name_acquired,
      on_name_lost,
      NULL,
      NULL
   );

   if (bus != 0) {
      result = XC_OK;
      if (Start_result != NULL) {
         Start_result (importHandle, result, user_data);
      }
   }
   else {
      result = XC_ERR_NOMEM;
      if (Start_error != NULL) {
         Start_error (importHandle, result, user_data);
      }
   }

   return result;
}

/* xcom.IService.Stop implementation for port Service */
xc_result_t
service_stop (
   xc_handle_t importHandle,
   void (* Stop_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void (* Stop_error) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void *user_data
) {

   if (bus != 0) {
      g_bus_unown_name (bus);
   }

   if (Stop_result != NULL) {
      Stop_result (importHandle, XC_OK, user_data);
   }
   return XC_OK;
}

/* Service port register(). */
xc_result_t
service_register (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   printf ("service_register %u %u\n", componentHandle, importHandle);
   return XC_OK;
}

/* Service port unregister(). */
xc_result_t
service_unregister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   printf ("service_unregister %u %u\n", componentHandle, importHandle);
   return XC_OK;
}

/* Hello port register(). */
xc_result_t
hello_register (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   printf ("hello_register %u %u\n", componentHandle, importHandle);
   return XC_OK;
}

/* Hello port unregister(). */
xc_result_t
hello_unregister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   printf ("hello_unregister %u %u\n", componentHandle, importHandle);
   return XC_OK;
}
