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

static void
import_dealloc (
   PyObject *objectPtr
) {
   PyObject_Del (objectPtr);
}

static PyMethodDef import_methods [] = {
   { NULL, NULL, 0, NULL }
};

static PyGetSetDef import_getseters [] = {
   { NULL, NULL, NULL, NULL, NULL }
};

static PyMemberDef import_members[] = {
   { "handle", T_INT, offsetof (ImportObject, importHandle), READONLY, NULL },
   { NULL,     0,     0,                                     0,        NULL }
};

PyTypeObject importType = {
    PyVarObject_HEAD_INIT (NULL, 0)
    "pycom.import",        /* tp_name           */
    sizeof (ImportObject), /* tp_size           */
    0,                     /* tp_itemsize       */
                           /* methods           */
    import_dealloc,        /* tp_dealloc        */
    0,                     /* tp_print          */
    0,                     /* tp_getattr        */
    0,                     /* tp_setattr        */
    0,                     /* tp_compare        */
    0,                     /* tp_repr           */
    0,                     /* tp_as_number      */
    0,                     /* tp_as_sequence    */
    0,                     /* tp_as_mapping     */
    0,                     /* tp_hash           */
    0,                     /* tp_call           */
    0,                     /* tp_str            */
    0,                     /* tp_getattro       */
    0,                     /* tp_setattro       */
    0,                     /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT,    /* tp_flags          */
    0,                     /* tp_doc            */
    0,                     /* tp_traverse       */
    0,                     /* tp_clear          */
    0,                     /* tp_richcompare    */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter           */
    0,                     /* tp_iternext       */
    import_methods,        /* tp_methods        */
    import_members,        /* tp_members        */
    import_getseters,      /* tp_getset         */
    0,                     /* tp_base           */
    0,                     /* tp_dict           */
    0,                     /* tp_descr_get      */
    0,                     /* tp_descr_set      */
    0,                     /* tp_dictoffset     */
    0,                     /* tp_init           */
    0,                     /* tp_alloc          */
    0,                     /* tp_new            */
    0,                     /* tp_free           */
    0,                     /* tp_is_gc          */
    0,                     /* tp_bases          */
    0,                     /* tp_mro            */
    0,                     /* tp_cache          */
    0,                     /* tp_subclasses     */
    0,                     /* tp_weaklist       */
    0,                     /* tp_del            */
    0                      /* tp_version_tag    */
};

