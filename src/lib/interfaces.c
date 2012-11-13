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

#define  TRACE_CLASS_DEFAULT INTERFACES
#include "internal.h"

bool
interfaces_match (
   const xc_interface_t *interface1Ptr,
   const char *proto1,
   void *reserved1,
   const xc_interface_t *interface2Ptr,
   const char *proto2,
   void *reserved2,
   int flags
) {
   bool result = true;

   TRACE3 ((
      "called with interface1Ptr=%p, proto1='%s', reserved1=%p "
      "interface2Ptr=%p, proto2='%s', reserved2=%p",
      interface1Ptr, proto1, reserved1,
      interface2Ptr, proto2, reserved2
   ));
   assert (interface1Ptr != NULL);
   assert (interface2Ptr != NULL);

   /* Match interface names. */
   if (strcmp (interface1Ptr->name, interface2Ptr->name) != 0) {
      TRACE4 (("names do not match ('%s' vs '%s')", interface1Ptr->name, interface2Ptr->name));
      result = false;
   }

   /* Match protocols. */
   if ((result == XC_OK) && (proto1 != NULL) && (proto2 != NULL)) {
      if (strcmp (proto1, proto2) != 0) {
         TRACE4 (("protocols do not match ('%s' vs '%s')", proto1, proto2));
         result = false;
      }
   }

   /* Version check */
   if (result == true) {

      TRACE4 ((
         "versions: %u.%u vs %u.%u",
         interface1Ptr->vmajor, interface1Ptr->vminor,
         interface2Ptr->vmajor, interface2Ptr->vminor
      ));

      if (interface1Ptr->vmajor != interface2Ptr->vmajor) {
         TRACE4 ((
            "major version numbers do not match (%u.%u vs %u.%u)",
            interface1Ptr->vmajor, interface1Ptr->vminor,
            interface2Ptr->vmajor, interface2Ptr->vminor
         ));
         result = false;
      }
      else if (flags & MATCHF_SAME_VERSION) {
         if (interface1Ptr->vminor != interface2Ptr->vminor) {
            TRACE4 ((
               "versions do not match (%u.%u vs %u.%u)",
               interface1Ptr->vmajor, interface1Ptr->vminor,
               interface2Ptr->vmajor, interface2Ptr->vminor
            ));
            result = false;
         }
      }
      else if (interface2Ptr->vminor > interface1Ptr->vminor) {
         TRACE4 ((
            "versions do not match (%u.%u vs %u.%u)",
            interface1Ptr->vmajor, interface1Ptr->vminor,
            interface2Ptr->vmajor, interface2Ptr->vminor
         ));
         result = false;
      }
   }
 
   TRACE3 (("exiting with result=%s", (result) ? "true" : "false"));
   return result;
}

