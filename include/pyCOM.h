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

#ifndef PYCOM_H
#define PYCOM_H

/** @file */

#include <xCOM.h>
#include <xCOM/clist.h>

#ifdef __PYCOM__

#ifdef HAVE_VISIBILITY
#define PYCOM_EXPORT __attribute__((visibility("default")))
#endif

#endif /* __PYCOM__ */

#ifndef PYCOM_EXPORT
#define PYCOM_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct pyCOM_Context;
typedef struct pyCOM_Context pycom_context_t;

struct pyCOM_QueuedCall {
   xc_clist_t node;
   void *method;
   void *result;
   void *error;
   void *user_data;
   void (* free) (void *_message);
   void (* handler) (void *_message);
   xc_handle_t importHandle;
   PyObject *oMethodName;
   PyObject *oUserData;
   PyObject *oError;
   PyMethodDef oErrorDef;
   PyObject *oResult;
   PyMethodDef oResultDef;
};
typedef struct pyCOM_QueuedCall pyCOM_queued_call_t;

xc_result_t
pyCOM_Init (
   xc_handle_t componentHandle,
   struct __xc_component_decl__ *componentDeclPtr
)
PYCOM_EXPORT;

xc_result_t
pyCOM_Destroy (
   xc_handle_t componentHandle
)
PYCOM_EXPORT;

pycom_context_t *
pyCOM_ContextEnter (
   xc_handle_t componentHandle
)
PYCOM_EXPORT;

void
pyCOM_ContextLeave (
   pycom_context_t *contextPtr
)
PYCOM_EXPORT;

xc_result_t
pyCOM_InvokeInit (
   xc_handle_t componentHandle
)
PYCOM_EXPORT;

xc_result_t
pyCOM_InvokeDestroy (
   xc_handle_t componentHandle
)
PYCOM_EXPORT;

xc_result_t
pyCOM_InvokeRegister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   const char *registerName
)
PYCOM_EXPORT;

xc_result_t
pyCOM_InvokeUnRegister (
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   const char *unRegisterName
)
PYCOM_EXPORT;

PyObject *
pyCOM_HandlePythonMethodErrorToNative (
   PyObject *oSelf,
   PyObject *oArgs
)
PYCOM_EXPORT;

PyObject *
pyCOM_HandlePythonMethodResultToNative (
   PyObject *oSelf,
   PyObject *oArgs
)
PYCOM_EXPORT;

void
pyCOM_HandleNativeMethodErrorToPython (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
)
PYCOM_EXPORT;

void
pyCOM_HandleNativeMethodResultToPython (
   xc_handle_t importHandle,
   xc_result_t error,
   void *user_data
)
PYCOM_EXPORT;

void
pyCOM_InitQueuedCall (
   pycom_context_t *contextPtr,
   pyCOM_queued_call_t *callPtr,
   const char *name,
   xc_handle_t importHandle,
   void *resultFunc,
   void *errorFunc,
   void *userData
) 
PYCOM_EXPORT;

#ifdef __cplusplus
}
#endif

#endif /* PYCOM_H */

