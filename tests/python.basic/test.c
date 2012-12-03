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

/* xcom.ITest.Run implementation for port test_in_s */
xc_result_t
test_in_s_run (
   xc_handle_t importHandle,
   void (* Run_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void (* Run_error) (
      xc_handle_t importHandle,
      xc_result_t error,
      void *user_data
   ),
   void *user_data
) {
   tests_in_s_t *testPtr;
   xc_result_t result;

   testPtr = (tests_in_s_t *) xCOM_GetSwitch (intf_in_sImportHandle);
   printf ("### testPtr=%p\n", testPtr);
   testPtr->check (intf_in_sImportHandle, "test string argument", Run_result, Run_error, user_data);
   return XC_OK;
}

