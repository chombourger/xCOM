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

#ifndef INTERNAL_H
#define INTERNAL_H

#include <Python.h>
#include <structmember.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pyCOM.h>

struct pyCOM_Context {
   xc_handle_t componentHandle;
   struct  __xc_component_decl__ *componentDeclPtr;
   PyThreadState *interpreterPtr;
   PyThreadState *previousStatePtr;
   PyThreadState *initStatePtr;
   PyObject *module;
};

/** The pycom.import object type. */
extern PyTypeObject importType;

/** The pycom.query object type. */
extern PyTypeObject queryType;

/** The native pycom.import structure. */
typedef struct {
   PyObject_HEAD
   xc_handle_t importHandle; /**< Handle of this import. */
} ImportObject;

/** The native pycom.query structure. */
typedef struct {
   PyObject_HEAD
   xc_handle_t queryHandle; /**< Handle of this query. */
   unsigned int matches;    /**< Number of matches. */
} QueryObject;

PyMODINIT_FUNC
pycom_module_init (
   void
);

PyObject *
pycom_query (
   PyObject *self,
   PyObject *args,
   PyObject *kwdict
);

#endif /* INTERNAL_H */

