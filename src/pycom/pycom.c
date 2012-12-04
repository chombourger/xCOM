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

/** Script for initalizing Python syspath. */
#define SYSPATH_SCRIPT "import sys\nsys.path.append('%s/Code/python')\n"

static PyThreadState *mainStatePtr = NULL;
static unsigned int components = 0;

xc_result_t
pyCOM_Init (
   xc_handle_t componentHandle,
   struct  __xc_component_decl__ *componentDeclPtr
) {

   pycom_context_t *contextPtr;
   xc_result_t result;

   /* Initialize Python GIL */
   PyEval_InitThreads ();

   contextPtr = xCOM_GetSpecific (componentHandle);
   if (contextPtr != NULL) {
      PyEval_ReleaseLock ();
      return XC_OK;
   }

   /* Initialize Python. */
   if (mainStatePtr == NULL) {
      Py_InitializeEx (0);
      mainStatePtr = PyEval_SaveThread ();
   }
   else {
      PyThreadState_Swap (mainStatePtr);
   }

   /* Allocate a context for this component. */
   contextPtr = malloc (sizeof (*contextPtr));
   if (contextPtr != NULL) {
      const char *bundlePathPtr;
      char *scriptPtr = NULL;
      int size;

      contextPtr->componentHandle = componentHandle;
      contextPtr->componentDeclPtr = componentDeclPtr;
      contextPtr->interpreterPtr = NULL;

      /* Allocate and create a small Python script to add the component
       * Python code to the syspath of its Python interpreter. */
      bundlePathPtr = xCOM_GetComponentBundlePath (componentHandle);
      if (bundlePathPtr != NULL) {
         size = snprintf (NULL, 0, SYSPATH_SCRIPT, bundlePathPtr);
         scriptPtr = malloc (size);
         if (scriptPtr != NULL) {
            snprintf (scriptPtr, size, SYSPATH_SCRIPT, bundlePathPtr);
            result = XC_OK;
         }
         else {
            result = XC_ERR_NOMEM;
         } 
      }
      else {
         result = XC_ERR_INVAL;
      }

      /* Create a new Python interpreter and run the syspath initialization
       * script. */
      if (result == XC_OK) {
         contextPtr->initStatePtr = PyThreadState_Swap (NULL);
         contextPtr->interpreterPtr = Py_NewInterpreter ();
         if (contextPtr->interpreterPtr != NULL) {
            PyRun_SimpleString (scriptPtr);
            contextPtr->module = PyImport_ImportModule (componentDeclPtr->name);
            if (contextPtr->module != NULL) {
               result = xCOM_SetSpecific (componentHandle, contextPtr);
            }
            else {
               result = XC_ERR_NOENT;
            }
         }
         else {
            result = XC_ERR_NOMEM;
         }
      }

      /* On error, free anything we would have allocated. Note that the Python
       * syspath initialization script is always freed. */
      free (scriptPtr);
      if (result == XC_OK) {
         components ++;
      }
      else {
         if (contextPtr->interpreterPtr != NULL) {
            Py_EndInterpreter (contextPtr->interpreterPtr); 
         }
         free (contextPtr);
      }
   }
   else {
      result = XC_ERR_NOMEM;
   }

   /* Release Python global lock since acquired by PyEval_InitThreads(). */
   PyThreadState_Swap (mainStatePtr);
   PyEval_ReleaseLock ();
   return result;
}

xc_result_t
pyCOM_Destroy (
   xc_handle_t componentHandle
) {
   pycom_context_t *contextPtr;
   PyThreadState *previousStatePtr;

   PyEval_AcquireLock ();
   contextPtr = xCOM_GetSpecific (componentHandle);
   if (contextPtr == NULL) {
      PyEval_ReleaseLock ();
      return XC_OK;
   }

   xCOM_SetSpecific (componentHandle, NULL);

   /* Switch to and delete component interpreter. */
   previousStatePtr = PyThreadState_Swap (contextPtr->interpreterPtr);
   PyThreadState_Clear (contextPtr->interpreterPtr);
   Py_EndInterpreter (contextPtr->interpreterPtr);


   /* Check if this is the last Python component running? */
   components --;
   if (components == 0) {
      /* Switch to main thread state. */
      PyThreadState_Swap (mainStatePtr);
      /* Yes, shutdown Python. */
      Py_Finalize ();
      mainStatePtr = NULL;
   }
   else {
      /* No, simply release the GIL */
      PyThreadState_Swap (previousStatePtr);
      PyEval_ReleaseLock ();
   }

   return XC_OK;
}

pycom_context_t *
pyCOM_ContextEnter (
   xc_handle_t componentHandle
) {
   pycom_context_t *contextPtr;

   PyEval_AcquireLock ();
   contextPtr = xCOM_GetSpecific (componentHandle);
   if (contextPtr != NULL) {
      contextPtr->previousStatePtr = PyThreadState_Swap (contextPtr->interpreterPtr);
   }
   else {
      PyEval_ReleaseLock ();
   }

   return contextPtr;
}

void
pyCOM_ContextLeave (
   pycom_context_t *contextPtr
) {
   /* Restore previous thread state. */
   if (contextPtr != NULL) {
      PyThreadState_Swap (contextPtr->previousStatePtr);
      PyEval_ReleaseLock ();
   }
}

static xc_result_t
pycom_invoke_init_destroy (
   xc_handle_t componentHandle,
   const char *name
) {
   pycom_context_t *contextPtr;
   PyObject *oDict, *oFunc;
   xc_result_t result;

   assert (name != NULL);

   contextPtr = pyCOM_ContextEnter (componentHandle);
   if (contextPtr == NULL) {
      return XC_ERR_INVAL;
   }

   /* Get Python init/destroy method.
    * oDict and oFunc are borrow references, no Py_DECREF required. */
   oDict = PyModule_GetDict (contextPtr->module);
   oFunc = PyDict_GetItemString (oDict, name);
   if (PyCallable_Check (oFunc)) {
      PyObject *oResult = PyObject_CallFunction (oFunc, "(I)", componentHandle);
      if (oResult != NULL) {
         result = PyInt_AsLong (oResult);
      }
      else {
         result = XC_ERR_INVAL;
      }
   }
   else {
      result = XC_ERR_NOENT;
   }

   /* Leave component context and return result. */
   pyCOM_ContextLeave (contextPtr);
   return result;
}

xc_result_t
pyCOM_InvokeInit (
   xc_handle_t componentHandle
) {
   return pycom_invoke_init_destroy (componentHandle, "init");
}

xc_result_t
pyCOM_InvokeDestroy (
   xc_handle_t componentHandle
) {
   return pycom_invoke_init_destroy (componentHandle, "destroy");
}

static PyObject *
pycom_invoke_register_unregister (
   pycom_context_t *contextPtr,
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   const char *name
) {
   PyObject *oDict, *oFunc, *oResult = NULL;

   assert (contextPtr != NULL);
   assert (name != NULL);

   /* Get Python init/destroy method.
    * oDict and oFunc are borrow references, no Py_DECREF required. */
   oDict = PyModule_GetDict (contextPtr->module); 
   oFunc = PyDict_GetItemString (oDict, name);
 
   if (PyCallable_Check (oFunc)) {
      oResult = PyObject_CallFunction (oFunc, "(II)", componentHandle, importHandle);
   }

   /* Store import handle in the returned object. */
   if ((oResult != NULL) && (oResult != Py_None)) {
      PyObject *oImportHandle = PyLong_FromUnsignedLong (importHandle);
      PyObject_SetAttrString (oResult, "__import_handle", oImportHandle);
   }

   return oResult;
}

xc_result_t
pyCOM_InvokeRegister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   const char *registerName
) {
   pycom_context_t *contextPtr;
   PyObject *oResult;
   xc_result_t result;

   contextPtr = pyCOM_ContextEnter (componentHandle);
   if (contextPtr == NULL) return XC_ERR_NOENT;

   /* Call register method. */
   oResult = pycom_invoke_register_unregister (
      contextPtr, componentHandle, importHandle, registerName
   );

   /* Store returned Python object in the xCOM import. */
   if (oResult != NULL) {
      result = xCOM_ImportSetSpecific (componentHandle, importHandle, oResult);
      if (result != XC_OK) Py_DECREF (oResult);
   }
   else {
      result = XC_ERR_INVAL;
   }

   /* Leave component context and return result. */
   pyCOM_ContextLeave (contextPtr);
   return result;
}

xc_result_t
pyCOM_InvokeUnRegister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   const char *unRegisterName
) {
   pycom_context_t *contextPtr;
   PyObject *oResult;
   xc_result_t result;

   contextPtr = pyCOM_ContextEnter (componentHandle);
   if (contextPtr == NULL) return XC_ERR_NOENT;

   /* Call unregister method. */
   if (unRegisterName != NULL) {
      oResult = pycom_invoke_register_unregister (
         contextPtr, componentHandle, importHandle, unRegisterName
      );
      if (oResult != NULL) Py_DECREF (oResult);
   }

   /* Get the Python object that we got from register() and stored in the import. */
   oResult = xCOM_ImportGetSpecific (componentHandle, importHandle);
   if (oResult != NULL) Py_DECREF (oResult);

   /* Leave component context and return result. */
   pyCOM_ContextLeave (contextPtr);
   return result;
}

PyObject *
pyCOM_HandlePythonMethodErrorToNative (
   PyObject *oSelf,
   PyObject *oArgs
) {
   pyCOM_queued_call_t *callPtr;
   PyObject *oImportHandle;
   xc_handle_t importHandle;
   PyObject *oResult;
   xc_result_t result;
   void (* error_cb) (xc_handle_t, xc_result_t, void *);

   /* Get import handle from Python. */
   oImportHandle = PyTuple_GetItem (oArgs, 0);
   importHandle  = PyInt_AsUnsignedLongMask (oImportHandle);

   /* Get result code from Python. */
   oResult = PyTuple_GetItem (oArgs, 1);
   result  = PyInt_AsLong (oResult);

   /* Get call instance and error callback. */
   callPtr  = PyCObject_AsVoidPtr (oSelf);
   error_cb = callPtr->error;

   /* Call the error callback. */
   error_cb (importHandle, result, callPtr->user_data);
   Py_RETURN_NONE;
}

PyObject *
pyCOM_HandlePythonMethodResultToNative (
   PyObject *oSelf,
   PyObject *oArgs
) {
   pyCOM_queued_call_t *callPtr;
   PyObject *oImportHandle;
   xc_handle_t importHandle;
   PyObject *oResult;
   xc_result_t result;
   void (* result_cb) (xc_handle_t, xc_result_t, void *);

   /* Get import handle from Python. */
   oImportHandle = PyTuple_GetItem (oArgs, 0);
   importHandle  = PyInt_AsUnsignedLongMask (oImportHandle);

   /* Get result code from Python. */
   oResult = PyTuple_GetItem (oArgs, 1);
   result  = PyInt_AsLong (oResult);

   /* Get call instance and result callback. */
   callPtr  = PyCObject_AsVoidPtr (oSelf);
   result_cb = callPtr->result;

   /* Call the result callback. */
   result_cb (importHandle, result, callPtr->user_data);
   Py_RETURN_NONE;
}

void
pyCOM_HandleNativeMethodErrorToPython (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
) {
   PyObject *oContext, *oFailure, *oUserData, *oImportHandle, *oError, *oResult;
   pycom_context_t *contextPtr;
   xc_handle_t componentHandle;

   componentHandle = xCOM_ImportGetClient (importHandle);
   contextPtr = pyCOM_ContextEnter (componentHandle);
   if (contextPtr == NULL) return;

   oContext      = user_data;
   oFailure      = PyTuple_GetItem (oContext, 1);
   oUserData     = PyTuple_GetItem (oContext, 2);
   oImportHandle = PyInt_FromLong (importHandle);
   oError        = PyInt_FromLong (error);

   if (PyCallable_Check (oFailure)) {
      oResult = PyObject_CallFunctionObjArgs (oFailure, oImportHandle, oError, oUserData, NULL);
      if (oResult != NULL) Py_DECREF (oResult);
   }
   Py_DECREF (oContext);

   pyCOM_ContextLeave (contextPtr);
}

void
pyCOM_HandleNativeMethodResultToPython (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
) {
   PyObject *oContext, *oSuccess, *oUserData, *oImportHandle, *oError, *oResult;
   pycom_context_t *contextPtr;
   xc_handle_t componentHandle;

   componentHandle = xCOM_ImportGetClient (importHandle);
   contextPtr = pyCOM_ContextEnter (componentHandle);
   if (contextPtr == NULL) return;

   oContext      = user_data;
   oSuccess      = PyTuple_GetItem (oContext, 0);
   oUserData     = PyTuple_GetItem (oContext, 2);
   oImportHandle = PyInt_FromLong (importHandle);
   oError        = PyInt_FromLong (error);

   if (PyCallable_Check (oSuccess)) {
      oResult = PyObject_CallFunctionObjArgs (oSuccess, oImportHandle, oError, oUserData, NULL);
      if (oResult != NULL) Py_DECREF (oResult);
   }
   Py_DECREF (oContext);

   pyCOM_ContextLeave (contextPtr);
}

void
pyCOM_InitQueuedCall (
   pycom_context_t *contextPtr,
   pyCOM_queued_call_t *callPtr,
   const char *name,
   xc_handle_t importHandle,
   void *resultFunc,
   void *errorFunc,
   void *userData
) {

   contextPtr = contextPtr;

   callPtr->user_data    = userData;
   callPtr->importHandle = importHandle;
   callPtr->oUserData    = PyCObject_FromVoidPtr (callPtr, NULL);
   callPtr->oMethodName  = PyString_FromString (name);

   if (resultFunc != NULL) {
      callPtr->oResultDef.ml_name  = name;
      callPtr->oResultDef.ml_meth  = resultFunc;
      callPtr->oResultDef.ml_flags = METH_VARARGS;
      callPtr->oResultDef.ml_doc   = NULL;

      callPtr->oResult = PyCFunction_NewEx (&callPtr->oResultDef, callPtr->oUserData, NULL);
   }
   else {
      Py_INCREF (Py_None);
      callPtr->oResult = Py_None;
   }

   if (errorFunc != NULL) {
      callPtr->oErrorDef.ml_name  = name;
      callPtr->oErrorDef.ml_meth  = resultFunc;
      callPtr->oErrorDef.ml_flags = METH_VARARGS;
      callPtr->oErrorDef.ml_doc   = NULL;

      callPtr->oError = PyCFunction_NewEx (&callPtr->oErrorDef, callPtr->oUserData, NULL);
   }
   else {
      Py_INCREF (Py_None);
      callPtr->oError = Py_None;
   }
}

