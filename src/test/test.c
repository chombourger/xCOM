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
#include <xCOM/IApplication.h>
#include <xCOM/ITest.h>

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

/** Our component handle. */
xc_handle_t componentHandle = XC_INVALID_HANDLE;

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

   xc_handle_t queryHandle;
   xc_handle_t importHandle;
   unsigned int i, testCount=0;
   unsigned int testLoaded=0, testPassed=0;
   xc_itest_t *testImpl;
   xc_result_t result, testResult;

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
               testResult = testImpl->run ();
               if (testResult == XC_OK) {
                  /* Test passed. */
                  testPassed ++;
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
test_run (
   void
) {
   printf (
      "\n"
      "xCOM Test Driver (built with " XC_ITEST_NAME " interface version %u.%u)\n"
      "\n"
      ,
      XC_ITEST_VERSION_MAJOR,
      XC_ITEST_VERSION_MINOR
   );
   while (main_menu () == true);
   return XC_OK;
}

/** Our exported xcom.IApplication interface. */
const xc_iapplication_t applicationImpl = {
   XC_INTERFACE_INIT (
      XC_IAPPLICATION_NAME,
      XC_IAPPLICATION_VERSION_MAJOR,
      XC_IAPPLICATION_VERSION_MINOR
   ),
   test_run
};

/** Component init() method. */
xc_result_t
test_init (
   xc_handle_t myComponentHandle
) {
   componentHandle = myComponentHandle;
   return XC_OK;
}

/** The imported xcom.ITest interface. */
const xc_interface_t testImport =
   XC_INTERFACE_INIT (
      XC_ITEST_NAME,
      XC_ITEST_VERSION_MAJOR,
      XC_ITEST_VERSION_MINOR
   );

/** Our component declaration. */
XC_DECLARE_COMPONENT {
   "xcom.test", "xCOM Test Driver", 1, 0,
   test_init, /* init() */
   NULL,      /* destroy() */
   {
      /* Provided ports. */
      XC_DECLARE_PROVIDED_PORT (
         "Application",                       /* Port name */
         (xc_interface_t *) &applicationImpl, /* Export switch */
         sizeof (applicationImpl),            /* Size of the export switch */
         NULL,                                /* Protocol */
         NULL,                                /* Reserved */
         NULL,                                /* register() */
         NULL                                 /* unregister() */
      ),
      /* Required ports. */
      XC_DECLARE_REQUIRED_PORT (
         "Tests",                             /* Port name */
         (xc_interface_t *) &testImport,      /* Interface spec */
         NULL,                                /* Import handle */
         NULL,                                /* Queried component name */
         NULL,                                /* Queried protocol */
         NULL,                                /* Reserved */
         XC_PORTF_NONE                        /* Port flags */
      ),
      NULL
   }
};

