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
#include <stdio.h>
#include <string.h>

#include "component.h"

/* examples.IHello.Say implementation for port Hello */
xc_result_t
hello_say (
   xc_handle_t importHandle,
   const char * /* in */ arg_greeting,
   void (* Say_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      char *response,
      void *user_data
   ),
   void (* Say_error) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void *user_data
) {
   printf ("you said: '%s'\n", arg_greeting);
   if (Say_result != NULL) {
      char *response = strdup ("how are you?");
      Say_result (importHandle, XC_OK, response, user_data);
   }
   return XC_OK;
}

