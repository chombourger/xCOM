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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "component.h"

struct TestContext {
   xc_handle_t importHandle;
   void (* Run_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   );
   void (* Run_error) (
      xc_handle_t importHandle,
      xc_result_t error,
      void *user_data
   );
   void *user_data;
};

void test_result (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
) {
   struct TestContext *contextPtr = user_data;
   void (* Run_result) (
      xc_handle_t importHandle,
      xc_result_t error,
      void *user_data
   );

   importHandle = contextPtr->importHandle;
   Run_result   = contextPtr->Run_result;
   user_data    = contextPtr->user_data;
   free (contextPtr);

   Run_result (importHandle, error, user_data);
}

void test_error (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
) {
   struct TestContext *contextPtr = user_data;
   void (* Run_error) (
      xc_handle_t importHandle,
      xc_result_t error,
      void *user_data
   );

   importHandle = contextPtr->importHandle;
   Run_error    = contextPtr->Run_error;
   user_data    = contextPtr->user_data;
   free (contextPtr);

   Run_error (importHandle, error, user_data);
}

static struct TestContext *
create_test_context (
   xc_handle_t importHandle,
   void *Run_result,
   void *Run_error,
   void *user_data
) {
   struct TestContext *contextPtr;

   contextPtr = malloc (sizeof (*contextPtr));
   if (contextPtr != NULL) {
      contextPtr->importHandle = importHandle;
      contextPtr->Run_result   = Run_result;
      contextPtr->Run_error    = Run_error;
      contextPtr->user_data    = user_data;
   }
   return contextPtr;
}
 
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
   struct TestContext *contextPtr;
   tests_in_s_t *testPtr;
   xc_result_t result = XC_OK;

   contextPtr = create_test_context (importHandle, Run_result, Run_error, user_data);
   if (contextPtr != NULL) {
      testPtr = (tests_in_s_t *) xCOM_GetSwitch (intf_in_sImportHandle);
      testPtr->check (intf_in_sImportHandle, "test string argument", test_result, test_error, contextPtr);
   }
   else result = XC_ERR_NOMEM;
   return result;
}

void test_out_s_result (
   xc_handle_t importHandle,
   xc_result_t error,
   char *data,
   void *user_data
) {
   struct TestContext *contextPtr = user_data;
   void (* Run_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   );
   void (* Run_error) (
      xc_handle_t importHandle,
      xc_result_t error,
      void *user_data
   );

   importHandle = contextPtr->importHandle;
   Run_result   = contextPtr->Run_result;
   Run_error    = contextPtr->Run_error;
   user_data    = contextPtr->user_data;
   free (contextPtr);

   if ((data != NULL) && (strcmp (data, "test out string argument") == 0)) {
      free (data);
      Run_result (importHandle, XC_OK, user_data);
   }
   else {
      free (data);
      Run_error (importHandle, XC_ERR_INVAL, user_data);
   }
}

/* xcom.ITest.Run implementation for port test_out_s */
xc_result_t
test_out_s_run (
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
   struct TestContext *contextPtr;
   tests_out_s_t *testPtr;
   xc_result_t result = XC_OK;

   contextPtr = create_test_context (importHandle, Run_result, Run_error, user_data);
   if (contextPtr != NULL) {
      testPtr = (tests_out_s_t *) xCOM_GetSwitch (intf_out_sImportHandle);
      testPtr->check (intf_out_sImportHandle, test_out_s_result, test_error, contextPtr);
   }
   else result = XC_ERR_NOMEM;
   return result;
}

/* xcom.ITest.Run implementation for port test_in_y */
xc_result_t
test_in_y_run (
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
   struct TestContext *contextPtr;
   tests_in_y_t *testPtr;
   xc_result_t result = XC_OK;

   contextPtr = create_test_context (importHandle, Run_result, Run_error, user_data);
   if (contextPtr != NULL) {
      testPtr = (tests_in_y_t *) xCOM_GetSwitch (intf_in_yImportHandle);
      testPtr->check (intf_in_yImportHandle, 0xabcd, test_result, test_error, contextPtr);
   }
   else result = XC_ERR_NOMEM;
   return result;
}

/* xcom.ITest.Run implementation for port test_in_b */
xc_result_t
test_in_b_run (
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
   struct TestContext *contextPtr;
   tests_in_b_t *testPtr;
   xc_result_t result = XC_OK;

   contextPtr = create_test_context (importHandle, Run_result, Run_error, user_data);
   if (contextPtr != NULL) {
      testPtr = (tests_in_b_t *) xCOM_GetSwitch (intf_in_bImportHandle);
      testPtr->check (intf_in_bImportHandle, true, test_result, test_error, contextPtr);
   }
   else result = XC_ERR_NOMEM;
   return result;
}

