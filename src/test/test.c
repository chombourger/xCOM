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

#include <xCOM.h>

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

//#ifdef HAVE_SEMAPHORE_H
#include <semaphore.h>
//#endif

#include "component.h"

/** Our component handle. */
xc_handle_t componentHandle = XC_INVALID_HANDLE;

struct TestData {
   sem_t waitSem;
   xc_result_t testResult;
};

static void
test_run_result (
   xc_handle_t importHandle,
   xc_result_t result,
   void *user_data
) {
   struct TestData *test = user_data;
   importHandle = importHandle;
   test->testResult = result;
   sem_post (&test->waitSem);
}

/**
  * Sequentially run all the test components implementing the xcom.ITest
  * interface.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
run_all_single (
   void
) {
   struct TestData testData;
   xc_handle_t queryHandle;
   xc_handle_t importHandle;
   unsigned int i, testCount=0;
   unsigned int testLoaded=0, testPassed=0;
   xcom_itest_t *testImpl;
   xc_result_t result;

   sem_init (&testData.waitSem, 0, 0);

   /* System-wide query for the xcom.ITest interface. */
   result = xCOM_QueryPort (
      componentHandle,
      "Tests",
      XC_QUERYF_NONE,
      &queryHandle,
      &testCount
   );

   /* (Try to) import each match. */
   if (result == XC_OK) {
      for (i=0; i<testCount; i++) {
         result = xCOM_QueryNext (queryHandle, &importHandle);
         if (result == XC_OK) {
            result = xCOM_Import (importHandle, (xc_interface_t **) &testImpl);
            if (result == XC_OK) {
               /* Test loaded, count and run it. */
               testLoaded ++;
               result = testImpl->Run (importHandle, test_run_result, &testData);
               if (result == XC_OK) {
                  sem_wait (&testData.waitSem);
                  if (testData.testResult == XC_OK) {
                     /* Test passed. */
                     testPassed ++;
                  }
               }
               /* Unload before proceeding with the next test. */
               xCOM_UnImport (importHandle);
            }
         }
     }
     /* Tried all xcom.ITest tests. Close query. */
     xCOM_QueryFinish (queryHandle);
   }

   /* Print test suite result summary. */
   if (testCount > 0) {
      printf ("\n%u tests: %u loaded, %u passed\n\n", testCount, testLoaded, testPassed);
   }
   else {
      fprintf (stderr, "no tests found!\n");
      result = XC_ERR_NOENT;
   }

   sem_destroy (&testData.waitSem);
   return result;
}

/**
  * Print the main menu and prompt user for a choice.
  *
  * @return true if the menu should be recalled, false otherwise.
  *
  */
static bool
main_menu (
   void
) {

   bool result = true;
   bool cont = false;
   int c;

   printf (
      "[1] Run all tests (single-threaded)\n"
      "[0] Quit\n"
      "\n"
      "Your choice: "
   );

   do {
      cont = false;
      c = fgetc (stdin);
      switch (c) {
         /* Quit */
         case '0':
            result = false;
            break;
         /* Run all tests (single-threaded) */
         case '1':
            run_all_single ();
            break;
         /* Characters to be simply ignored when received. */
         case '\n':
         case '\r':
            cont = true;
            break;
         /* Print the prompt again if we did get a valid choice. */
         default:
            printf ("Your choice: ");
            cont = true;
            break;
      }
   } while (cont == true);

   return result;
}

/**
  * xcom.IApplication entry point. 
  *
  * @return XC_OK
  *
  */
xc_result_t
application_start (
   xc_handle_t importHandle,
   void (* Start_result) (
      xc_handle_t importHandle,
      xc_result_t result,
      void *user_data
   ),
   void *user_data
) {
   printf (
      "\n"
      "xCOM Test Driver\n"
      "\n"
   );
   while (main_menu () == true);
   if (Start_result != NULL) {
      Start_result (importHandle, XC_OK, user_data);
   }
   return XC_OK;
}

/** Component init() method. */
xc_result_t
xcom_test_init (
   xc_handle_t myComponentHandle
) {
   componentHandle = myComponentHandle;
   return XC_OK;
}

