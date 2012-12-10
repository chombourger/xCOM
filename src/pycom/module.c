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

/**
  * List of functions exported by the pycom module.
  *
  */
static struct PyMethodDef pycom_functions [] = {
    { "Query", (PyCFunction) pycom_query, METH_VARARGS|METH_KEYWORDS, NULL },
    { NULL,     NULL,                     0,                          NULL }
};

/**
  * pycom module initialization.
  *
  */
PyMODINIT_FUNC
pycom_module_init (
   void
) {
    PyObject *type;
    PyObject *m;

    /* Initialize the "import" type. */
    type = (PyObject *) &importType;
    Py_TYPE (type) = &PyType_Type;
    if (PyType_Ready (&importType) >= 0) {
       /* Initialize the "query" type. */
       type = (PyObject *) &queryType;
       Py_TYPE (type) = &PyType_Type;
       if (PyType_Ready (&queryType) >= 0) {
          /* Initialize the "pycom" module. */
          m = Py_InitModule ("pycom", pycom_functions);
          if (m != NULL) {
             /* Success. */
          }
       }
    }
}

