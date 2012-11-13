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
#include <examples/IHello.h>

#include <stdio.h>

xc_result_t
hello_say (
   const char *greeting
) {
   fprintf (stderr, "say('%s') called!\n", greeting);
   return XC_OK;
}

const examples_ihello_t hello = {
   XC_INTERFACE_INIT (
      EXAMPLES_IHELLO_NAME,
      EXAMPLES_IHELLO_VERSION_MAJOR,
      EXAMPLES_IHELLO_VERSION_MINOR
   ),
   hello_say
};

xc_result_t
hello_register (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   fprintf (stderr, "hello: register called!\n");
   return XC_OK;
}

xc_result_t
hello_unregister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
) {
   fprintf (stderr, "hello: unregister called!\n");
   return XC_OK;
}

xc_result_t
hello_init (
   xc_handle_t handle
) {
   fprintf (stderr, "hello: init called!\n");
   return XC_OK;
}

xc_result_t
hello_destroy (
   xc_handle_t handle
) {
   fprintf (stderr, "hello: destroy called!\n");
   return XC_OK;
}

XC_DECLARE_COMPONENT {
   "hello", "xCOM flavour of helloworld", 1, 0,
   hello_init,
   hello_destroy,
   {
      XC_DECLARE_PROVIDED_PORT (
         "Hello",
         (xc_interface_t *) &hello,
         sizeof (hello),
         NULL,
         NULL,
         hello_register,
         hello_unregister
      ),
      NULL
   }
};

