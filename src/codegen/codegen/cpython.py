#
# xCOM - a Simple Component Framework
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

import string;
from codegen import match;
from codegen import cgeneric;

class CodeGenerator(cgeneric.CodeGenerator):

   def __init__ (self, name, components, interfaces, opts):
      cgeneric.CodeGenerator.__init__ (self, name, components, interfaces, opts);

   def write_source_includes (self, file):
      file.write ('#include <Python.h>\n');
      cgeneric.CodeGenerator.write_source_includes (self, file);

   def write_component_init_internal (self, comp, file):
      cgeneric.CodeGenerator.write_component_init_internal (self, comp, file);

   def source_write_queued_message_struct_members (self, file):
      cgeneric.CodeGenerator.source_write_queued_message_struct_members (self, file);
      file.write ('   PyObject *oMethodName;\n');
      file.write ('   PyObject *oUserData;\n');
      file.write ('   PyObject *oError;\n');
      file.write ('   PyMethodDef oErrorDef;\n');
      file.write ('   PyObject *oResult;\n');
      file.write ('   PyMethodDef oResultDef;\n');

   def is_imported (self, i):
      for c in self.m_components:
         for p in c.ports():
            if p.provided() == False:
               if p.interface() == i.name() and p.versionMajor() == i.versionMajor() and p.versionMinor() == i.versionMinor():
                   return True;
      return False;

   def native_type_to_python (self, type):
      if type == 'y':
         return 'PyInt_FromLong';
      elif type == 'b':
         return 'PyBool_FromLong';
      elif type == 'n':
         return 'PyInt_FromLong';
      elif type == 'q':
         return 'PyLong_FromUnsignedLong';
      elif type == 'i':
         return 'PyInt_FromLong';
      elif type == 'u':
         return 'PyLong_FromUnsignedLong';
      elif type == 'x':
         return 'PyLong_FromLongLong';
      elif type == 't':
         return 'PyLong_FromUnsignedLongLong';
      elif type == 'd':
         return 'PyFloat_FromDouble';
      elif type == 's':
         return 'PyString_FromString';
      # TODO everything else to default to variant type
      return None;

   def python_to_native_type (self, type):
      if type == 'y':
         return '(uint8_t) PyInt_AsLong';
      elif type == 'b':
         return '(bool) PyInt_AsLong';
      elif type == 'n':
         return '(int16_t) PyInt_AsLong';
      elif type == 'q':
         return '(uint16_t) PyLong_AsUnsignedLong';
      elif type == 'i':
         return 'PyInt_AsLong';
      elif type == 'u':
         return 'PyLong_AsUnsignedLong';
      elif type == 'x':
         return 'PyLong_AsLongLong';
      elif type == 't':
         return 'PyLong_AsUnsignedLongLong';
      elif type == 'd':
         return 'PyFloat_AsDouble';
      elif type == 's':
         return 'PyString_AsString';
      # TODO everything else to default to variant type
      return None;

   def out_arguments (self, m):
      count = 0;
      for a in m.arguments():
         if a.direction() == 'out':
            count = count + 1;
      return count;

   def source_write_free_out_arg (self, a, file):
      if a.type() == 's':
         file.write ('   free (%s);\n'%(self.name_argument(a)));

   def source_write_method_result_wrapper (self, m, file):
      intf = m.parent();

      # Prototype
      file.write ('static void\n');
      file.write ('%s_%s_result (\n'%(self.name_interface(intf),self.name_method(m)));
      file.write ('   xc_handle_t importHandle,\n');
      file.write ('   xc_result_t error,\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('   %s %s,\n'%(self.name_type(a.type(),a.direction()),self.name_argument(a)));
      file.write ('   void *user_data\n');
      file.write (') {\n');

      # Local variables
      file.write ('   PyObject *oContext, *oSuccess, *oUserData, *oImportHandle, *oError, *oResult;\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('   PyObject *py_%s;\n'%(self.name_argument(a)));

      file.write ('   oContext  = user_data;\n');
      file.write ('   PyEval_AcquireLock ();\n');
      file.write ('   PyThreadState_Swap (__component);\n');

      file.write ('   oSuccess = PyTuple_GetItem (oContext, 0);\n');
      file.write ('   oUserData = PyTuple_GetItem (oContext, 2);\n');
      file.write ('   oImportHandle = PyInt_FromLong (importHandle);\n');
      file.write ('   oError = PyInt_FromLong (error);\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('   py_%s = %s (%s);\n'%(
               self.name_argument(a),
	       self.native_type_to_python(a.type()),
               self.name_argument(a)
            ));
            self.source_write_free_out_arg (a, file);

      file.write ('   if (PyCallable_Check (oSuccess)) {\n');
      file.write ('      oResult = PyObject_CallFunctionObjArgs (\n');
      file.write ('         oSuccess,\n');
      file.write ('         oImportHandle,\n');
      file.write ('         oError,\n');
      for a in m.arguments():
         if a.direction() == 'out':
             file.write ('         py_%s,\n'%(self.name_argument(a)));
      file.write ('         oUserData,\n');
      file.write ('         NULL\n');
      file.write ('      );\n');
      file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
      file.write ('   }\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('   Py_DECREF (py_%s);\n'%(self.name_argument(a)));
      file.write ('   Py_DECREF (oContext);\n');
      file.write ('   PyThreadState_Swap (__global);\n');
      file.write ('   PyEval_ReleaseLock ();\n');
      file.write ('}\n\n');

   def source_write_method_wrapper (self, m, file):
      intf = m.parent();

      # Any out arguments?
      oargs = self.out_arguments (m);
      if oargs > 0:
         self.source_write_method_result_wrapper (m, file);

      # Prototype
      file.write ('/* %s: %s */\n'%(intf.name(), m.name()));
      file.write ('static PyObject *\n');
      file.write ('%s_%s (\n'%(self.name_interface(intf),self.name_method(m)));
      file.write ('   PyObject *oSelf,\n');
      file.write ('   PyObject *oArgs\n');
      file.write (') {\n');

      # Local variables
      file.write ('   PyObject *oImportHandle;\n');
      file.write ('   xc_handle_t importHandle;\n');
      file.write ('   %s_t *switchPtr;\n'%(self.name_interface (intf)));
      for a in m.arguments():
         if a.direction() == 'in':
            file.write ('   PyObject *py_%s;\n'%(self.name_argument(a)));
            file.write ('   %s %s;\n'%(self.name_type(a.type(),a.direction()),self.name_argument(a)));
      file.write ('   PyObject *oSuccess;\n');
      file.write ('   PyObject *oFailure;\n');
      file.write ('   PyObject *oUserData;\n');
      file.write ('   PyObject *oTuple;\n');
      file.write ('\n');

      file.write ('   /* Get import handle and switch */\n');
      file.write ('   oImportHandle = PyObject_GetAttrString (oSelf, "__import_handle");\n');
      file.write ('   importHandle  = PyLong_AsUnsignedLong (oImportHandle);\n');
      file.write ('   switchPtr     = (%s_t *) xCOM_GetSwitch (importHandle);\n'%(
         self.name_interface(intf)
      ));
      file.write ('\n');

      file.write ('   /* Get argument values. */\n');
      i = 0;
      for a in m.arguments():
         if a.direction() == 'in':
            file.write ('   py_%s = PyTuple_GetItem (oArgs, %d);\n'%(self.name_argument(a), i));
            file.write ('   %s = %s (py_%s);\n'%(
               self.name_argument(a),
               self.python_to_native_type (a.type()),
               self.name_argument(a)
            ));
            i = i + 1;
      file.write ('   oTuple    = PyTuple_New (3);\n');
      file.write ('   oSuccess  = PyTuple_GetItem (oArgs, %d);\n'%(i));
      file.write ('   PyTuple_SetItem (oTuple, 0, oSuccess);\n');
      i = i + 1;
      file.write ('   oFailure  = PyTuple_GetItem (oArgs, %d);\n'%(i));
      file.write ('   PyTuple_SetItem (oTuple, 1, oFailure);\n');
      i = i + 1;
      file.write ('   oUserData = PyTuple_GetItem (oArgs, %d);\n'%(i));
      file.write ('   PyTuple_SetItem (oTuple, 2, oUserData);\n');
      i = i + 1;
      file.write ('\n');

      file.write ('   /* Call imported method. */\n');
      file.write ('   switchPtr->%s (\n'%(self.name_method(m)));
      file.write ('      importHandle,\n');
      for a in m.arguments():
         if a.direction() == 'in':
            file.write ('      %s,\n'%(self.name_argument(a)));
      if oargs > 0:
         file.write ('      %s_%s_result,\n'%(self.name_interface(intf),self.name_method(m)));
      else:
         file.write ('      native_method_result_to_python,\n');
      file.write ('      native_method_error_to_python,\n');
      file.write ('      oTuple\n');
      file.write ('   );\n');
      file.write ('   Py_RETURN_NONE;\n');
      file.write ('}\n');
      file.write ('\n');

      file.write ('static PyMethodDef %s_%s_def = {\n'%(self.name_interface(intf),self.name_method(m)));
      file.write ('   "%s",\n'%(m.name()));
      file.write ('   %s_%s,\n'%(self.name_interface(intf),self.name_method(m)));
      file.write ('   METH_VARARGS,\n');
      file.write ('   NULL\n');
      file.write ('};\n');
      file.write ('\n');

   def source_write_interface_wrappers (self, intf, file):
      for m in intf.methods():
         self.source_write_method_wrapper (m, file);

   def source_write_wrappers (self, file):
      for i in self.m_interfaces:
         if self.is_imported (i):
            self.source_write_interface_wrappers (i, file);

   def source_write_globals (self, comp, file):
      cgeneric.CodeGenerator.source_write_globals (self, comp, file);
      file.write ('/* Global Python interpreter. */\n');
      file.write ('static PyThreadState *__global;\n');
      file.write ('\n');
      file.write ('/* Python interpreter for this component. */\n');
      file.write ('static PyThreadState *__component;\n');
      file.write ('\n');
      file.write ('/* Python module holding component code. */\n');
      file.write ('static PyObject *__module;\n');
      file.write ('\n');
      file.write ('/* Component handle. */\n');
      file.write ('static xc_handle_t __component_handle;\n');
      file.write ('\n');
      file.write ('/* Whether initialized. */\n');
      file.write ('static bool __python_initialized = false;\n');
      file.write ('\n');

      # FIXME find a better way to add path to python component
      file.write ('static xc_result_t\n');
      file.write ('__python_initialize (\n');
      file.write ('   xc_handle_t componentHandle\n');
      file.write (') {\n');
      file.write ('   xc_result_t result = XC_OK;\n');
      file.write ('   pthread_mutex_lock (&__lock);\n');
      file.write ('   if (__python_initialized == false) {\n');
      file.write ('      Py_Initialize ();\n');
      file.write ('      PyEval_InitThreads ();\n');
      file.write ('      __global = PyThreadState_Get ();\n');
      file.write ('      __component = Py_NewInterpreter ();\n');
      file.write ('      if (__component != NULL) {\n');
      file.write ('         char syspath [1024];\n');
      file.write ('         strcpy (syspath, "import sys\\nsys.path.append(\'");\n');
      file.write ('         strcat (syspath, xCOM_GetComponentBundlePath (componentHandle));\n');
      file.write ('         strcat (syspath, "/Code/python\')\\n");\n');
      file.write ('         PyRun_SimpleString (syspath);\n');
      file.write ('         __module = PyImport_ImportModule ("%s");\n'%(comp.name()));
      file.write ('         if (__module != NULL) {\n');
      file.write ('            __python_initialized = true;\n');
      file.write ('         }\n');
      file.write ('         else result = XC_ERR_NOENT;\n');
      file.write ('      }\n');
      file.write ('      PyEval_ReleaseLock ();\n');
      file.write ('   }\n');
      file.write ('   pthread_mutex_unlock (&__lock);\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');
 
      file.write ('static void\n');
      file.write ('__python_finalize (\n');
      file.write ('   xc_handle_t componentHandle\n');
      file.write (') {\n');
      file.write ('   pthread_mutex_lock (&__lock);\n');
      file.write ('   if (__python_initialized == true) {\n');
      file.write ('      PyEval_AcquireLock ();\n');
      file.write ('      PyThreadState_Swap (__component);\n');
      file.write ('      Py_EndInterpreter (__component);\n');
      file.write ('      __component = NULL;\n');
      file.write ('      PyThreadState_Swap (__global);\n');
      file.write ('      Py_Finalize ();\n');
      file.write ('      PyEval_ReleaseLock ();\n');
      file.write ('      __python_initialized = false;\n');
      file.write ('   }\n');
      file.write ('   pthread_mutex_unlock (&__lock);\n');
      file.write ('}\n\n');

      file.write ('static PyObject *\n');
      file.write ('__python_register_unregister (\n');
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle,\n'); 
      file.write ('   const char *name\n'); 
      file.write (') {\n');
      file.write ('   PyObject *oDict, *oFunc, *result = NULL;\n');
      file.write ('   PyEval_AcquireLock ();\n');
      file.write ('   PyThreadState_Swap (__component);\n');
      file.write ('   oDict = PyModule_GetDict (__module);\n');
      file.write ('   oFunc = PyDict_GetItemString (oDict, name);\n');
      file.write ('   if (PyCallable_Check (oFunc)) {\n');
      file.write ('      result = PyObject_CallFunction (oFunc, "(II)", componentHandle, importHandle);\n');
      file.write ('   }\n');
      file.write ('   else {\n');
      file.write ('      result = NULL;\n');
      file.write ('   }\n');
      file.write ('   if (oFunc != NULL) Py_DECREF (oFunc);\n');
      file.write ('   PyThreadState_Swap (__global);\n');
      file.write ('   PyEval_ReleaseLock ();\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

      file.write ('/* Generic Method Error Python/C callback. */\n');
      file.write ('static PyObject *\n');
      file.write ('python_method_error_to_native (\n');
      file.write ('   PyObject *oSelf,\n');
      file.write ('   PyObject *oArgs\n');
      file.write (') {\n');
      file.write ('   struct __queued_message *header;\n');
      file.write ('   PyObject *oImportHandle;\n');
      file.write ('   xc_handle_t importHandle;\n');
      file.write ('   PyObject *oResult;\n');
      file.write ('   xc_result_t result;\n');
      file.write ('   void (* error_cb) (xc_handle_t, xc_result_t, void *);\n');
      file.write ('   oImportHandle = PyTuple_GetItem (oArgs, 0);\n');
      file.write ('   importHandle  = PyInt_AsUnsignedLongMask (oImportHandle);\n');
      file.write ('   oResult = PyTuple_GetItem (oArgs, 1);\n');
      file.write ('   result  = PyInt_AsLong (oResult);\n');
      file.write ('   header   = PyCObject_AsVoidPtr (oSelf);\n');
      file.write ('   error_cb = header->error;\n');
      file.write ('   error_cb (importHandle, result, header->user_data);\n');
      file.write ('   Py_RETURN_NONE;\n');
      file.write ('}\n\n');

      file.write ('static void\n');
      file.write ('native_method_error_to_python (\n');
      file.write ('   xc_handle_t importHandle,\n');
      file.write ('   xc_result_t error,\n');
      file.write ('   void *user_data\n');
      file.write (') {\n');
      file.write ('   PyObject *oContext, *oFailure, *oUserData, *oImportHandle, *oError, *oResult;\n');
      file.write ('   oContext  = user_data;\n');
      file.write ('   PyEval_AcquireLock ();\n');
      file.write ('   PyThreadState_Swap (__component);\n');
      file.write ('   oFailure = PyTuple_GetItem (oContext, 1);\n');
      file.write ('   oUserData = PyTuple_GetItem (oContext, 2);\n');
      file.write ('   oImportHandle = PyInt_FromLong (importHandle);\n');
      file.write ('   oError = PyInt_FromLong (error);\n');
      file.write ('   if (PyCallable_Check (oFailure)) {\n');
      file.write ('      oResult = PyObject_CallFunctionObjArgs (oFailure, oImportHandle, oError, oUserData, NULL);\n');
      file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
      file.write ('   }\n');
      file.write ('   Py_DECREF (oContext);\n');
      file.write ('   PyThreadState_Swap (__global);\n');
      file.write ('   PyEval_ReleaseLock ();\n');
      file.write ('}\n\n');

      file.write ('static void\n');
      file.write ('native_method_result_to_python (\n');
      file.write ('   xc_handle_t importHandle,\n');
      file.write ('   xc_result_t error,\n');
      file.write ('   void *user_data\n');
      file.write (') {\n');
      file.write ('   PyObject *oContext, *oSuccess, *oUserData, *oImportHandle, *oError, *oResult;\n');
      file.write ('   oContext  = user_data;\n');
      file.write ('   PyEval_AcquireLock ();\n');
      file.write ('   PyThreadState_Swap (__component);\n');
      file.write ('   oSuccess = PyTuple_GetItem (oContext, 0);\n');
      file.write ('   oUserData = PyTuple_GetItem (oContext, 2);\n');
      file.write ('   oImportHandle = PyInt_FromLong (importHandle);\n');
      file.write ('   oError = PyInt_FromLong (error);\n');
      file.write ('   if (PyCallable_Check (oSuccess)) {\n');
      file.write ('      oResult = PyObject_CallFunctionObjArgs (oSuccess, oImportHandle, oError, oUserData, NULL);\n');
      file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
      file.write ('   }\n');
      file.write ('   Py_DECREF (oContext);\n');
      file.write ('   PyThreadState_Swap (__global);\n');
      file.write ('   PyEval_ReleaseLock ();\n');
      file.write ('}\n\n');

      # Write wrappers for imported interfaces
      self.source_write_wrappers (file);

   def queue_arg_type (self, a):
      return 'PyObject *';

   def queue_free_arg_expr (self, a, expr):
      return 'Py_DECREF (%s);'%(expr);

   def source_write_free_message (self, m, file):
      file.write('   Py_DECREF (header->oMethodName);\n');
      file.write('   Py_DECREF (header->oUserData);\n');
      file.write('   Py_DECREF (header->oError);\n');
      file.write('   Py_DECREF (header->oResult);\n');
      cgeneric.CodeGenerator.source_write_free_message (self, m, file);

   def write_queue_copy_args_prologue (self, file):
      file.write ('      PyThreadState_Swap (__component);\n');

   def write_queue_copy_args_epilogue (self, m, file):
      file.write ('      header->oMethodName = PyString_FromString ("%s");\n'%(m.name()));
      file.write ('      header->oUserData = PyCObject_FromVoidPtr (header, NULL);\n');
      file.write ('      if (%s_result != NULL) {\n'%(self.name_method(m)));
      file.write ('         const char *NAME = "%s_result";\n'%(self.name_method(m)));
      file.write ('         header->oResultDef.ml_name = NAME;\n');
      file.write ('         header->oResultDef.ml_meth = __%s_method_result;\n'%(self.name_method(m)));
      file.write ('         header->oResultDef.ml_flags = METH_VARARGS;\n');
      file.write ('         header->oResultDef.ml_doc = NULL;\n');
      file.write ('         PyObject *name = PyString_FromString (NAME);\n');
      file.write ('         header->oResult = PyCFunction_NewEx (&header->oResultDef, header->oUserData, name);\n');
      file.write ('         Py_DECREF(name);\n');
      file.write ('      }\n');
      file.write ('      else {\n');
      file.write ('         Py_INCREF (Py_None);\n');
      file.write ('         header->oResult = Py_None;\n');
      file.write ('      }\n');
      file.write ('      if (%s_error != NULL) {\n'%(self.name_method(m)));
      file.write ('         const char *NAME = "%s_error";\n'%(self.name_method(m)));
      file.write ('         header->oErrorDef.ml_name = NAME;\n');
      file.write ('         header->oErrorDef.ml_meth = python_method_error_to_native;\n');
      file.write ('         header->oErrorDef.ml_flags = METH_VARARGS;\n');
      file.write ('         header->oErrorDef.ml_doc = NULL;\n');
      file.write ('         PyObject *name = PyString_FromString (NAME);\n');
      file.write ('         header->oError = PyCFunction_NewEx (&header->oErrorDef, header->oUserData, name);\n');
      file.write ('         Py_DECREF(name);\n');
      file.write ('      }\n');
      file.write ('      else {\n');
      file.write ('         Py_INCREF (Py_None);\n');
      file.write ('         header->oError = Py_None;\n');
      file.write ('      }\n');
      file.write ('      PyThreadState_Swap (__global);\n');

   def queue_copy_arg_expr (self, a, indent):
      result='';
      if a.type() == 's':
         result += indent + 'if ((result == XC_OK) && (%s != NULL)) {\n'%(self.name_argument(a));
         result += indent + '   message->%s = PyString_FromString (%s);\n'%(self.name_argument(a),self.name_argument(a));
         result += indent + '   if (message->%s == NULL) {\n'%(self.name_argument(a));
         result += indent + '      result = XC_ERR_NOMEM;\n';
         result += indent + '   }\n';
         result += indent + '}\n';
      else:
         result += indent + 'message->%s = %s;\n'%(self.name_argument(a),self.name_argument(a));
      return result;

   # Do not write the C declarations for the provided methods since they
   # would be implemented in Python instead!
   def write_port_decls (self, p, file):
      return None;

   def source_write_call_python_init (self, comp, file, indent):
      if comp.init() == True:
         file.write (indent + '/* Call component init() in Python. */\n');
         file.write (indent + 'result = __python_init_destroy (componentHandle, "init");\n');
      else:
         file.write (indent + 'result = XC_OK;\n');
 
   def source_write_call_user_init (self, comp, file):
      file.write ('   __component_handle = componentHandle;\n');
      self.source_write_call_python_init (comp, file, '   ');

   def source_write_call_user_destroy (self, comp, file):
      if comp.destroy() == True:
         file.write ('   result = __python_init_destroy (componentHandle, "destroy");\n');
      else:
         file.write ('   result = XC_OK;\n');
      file.write ('   __python_finalize (componentHandle);\n');

   def write_queue_method_call (self, p, m, file):
      args = len(m.arguments());
      iargs = 0;
      for a in m.arguments():
         if a.direction() == 'in':
            iargs = iargs + 1;
      file.write ('static void\n');
      file.write ('__call_%s (\n'%(self.name_provided_method(p,m)));
      file.write ('   void *_message\n');
      file.write (') {\n');
      file.write ('   struct __queued_message *header = _message;\n');
      if args > 0:
         file.write ('   struct __%s_message *message = _message;\n'%(self.name_provided_method(p,m)));
      else:
         file.write ('   struct __queued_message *message = _message;\n');
      file.write ('   void (* error_cb) %s = header->error;\n'%(self.proto_method_error(m, '   ')));
      file.write ('   PyObject *object = xCOM_ImportGetSpecific (__component_handle, header->importHandle);\n');
      file.write ('   if (object != NULL) {\n');
      file.write ('      PyObject *oResult;\n');
      file.write ('      PyEval_AcquireLock ();\n');
      file.write ('      PyThreadState_Swap (__component);\n');
      file.write ('      oResult = PyObject_CallMethodObjArgs (\n');
      file.write ('         object,\n');
      file.write ('         header->oMethodName,\n');
      for a in m.arguments():
         if a.direction() == 'in':
            file.write ('         message->%s,\n'%(self.name_argument(a)));
      file.write ('         header->oResult,\n');
      file.write ('         header->oError,\n');
      file.write ('         header->oUserData,\n');
      file.write ('         NULL\n');
      file.write ('      );\n');
      file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
      file.write ('      else {\n');
      file.write ('         if (error_cb != NULL) error_cb (header->importHandle, XC_ERR_INVAL, header->user_data);\n');
      file.write ('      }\n');
      file.write ('      PyThreadState_Swap (__global);\n');
      file.write ('      PyEval_ReleaseLock ();\n');
      file.write ('   }\n');
      file.write ('   header->free (message);\n');
      file.write ('}\n\n');

      # TODO handle out arguments
      file.write ('static PyObject *\n');
      file.write ('__%s_method_result (\n'%(self.name_method(m)));
      file.write ('   PyObject *oSelf,\n');
      file.write ('   PyObject *oArgs\n');
      file.write (') {\n');
      file.write ('   struct __queued_message *header;\n');
      file.write ('   PyObject *oImportHandle;\n');
      file.write ('   xc_handle_t importHandle;\n');
      file.write ('   PyObject *oResult;\n');
      for a in m.arguments():
         file.write ('   PyObject *py_%s;\n'%(self.name_argument(a)));
         file.write ('   %s %s;\n'%(self.name_type(a.type(),a.direction()),self.name_argument(a)));
      file.write ('   xc_result_t result;\n');
      file.write ('   void (* result_cb) %s;\n'%(self.proto_method_result(m, '   ')));

      file.write ('   oImportHandle = PyTuple_GetItem (oArgs, 0);\n');
      file.write ('   importHandle  = PyInt_AsUnsignedLongMask (oImportHandle);\n');
      file.write ('   Py_DECREF (oImportHandle);\n');

      file.write ('   oResult = PyTuple_GetItem (oArgs, 1);\n');
      file.write ('   result  = PyInt_AsLong (oResult);\n');
      file.write ('   Py_DECREF (oResult);\n');

      i = 1;
      for a in m.arguments():
         if a.direction() == 'out':
            i = i + 1;
            file.write ('   py_%s = PyTuple_GetItem (oArgs, %d);\n'%(self.name_argument(a),i));
            file.write ('   %s = %s (py_%s);\n'%(
               self.name_argument(a),
               self.python_to_native_type(a.type()),
               self.name_argument(a)
            ));
            if a.type() == 's':
               file.write ('   %s = strdup (%s);\n'%(self.name_argument(a), self.name_argument(a)));

      file.write ('   header = PyCObject_AsVoidPtr (oSelf);\n');
      file.write ('   result_cb = header->result;\n');
      file.write ('   result_cb (\n');
      file.write ('      importHandle,\n');
      file.write ('      result,\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('      %s,\n'%(self.name_argument(a)));
      file.write ('      header->user_data\n');
      file.write ('   );\n');
      file.write ('   Py_RETURN_NONE;\n');
      file.write ('}\n\n');

   def source_write_init_destroy_prologue (self, component, file):
      file.write ('static xc_result_t\n');
      file.write ('__python_init_destroy (\n');
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   const char *name\n');
      file.write (') {\n');
      file.write ('   xc_result_t result = __python_initialize (componentHandle);\n');
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      PyEval_AcquireLock ();\n');
      file.write ('      PyThreadState_Swap (__component);\n');
      file.write ('      PyObject *oDict = PyModule_GetDict (__module);\n');
      file.write ('      PyObject *oFunc = PyDict_GetItemString (oDict, name);\n');
      file.write ('      if (PyCallable_Check (oFunc)) {\n');
      file.write ('         PyObject *oResult = PyObject_CallFunction (oFunc, "(I)", componentHandle);\n');
      file.write ('         if (oResult != NULL) {\n');
      file.write ('            result = PyInt_AsLong (oResult);\n');
      file.write ('         }\n');
      file.write ('      }\n');
      file.write ('      else {\n');
      file.write ('         result = XC_ERR_INVAL;\n');
      file.write ('      }\n');
      file.write ('      if (oFunc != NULL) Py_DECREF (oFunc);\n');
      file.write ('      PyThreadState_Swap (__global);\n');
      file.write ('      PyEval_ReleaseLock ();\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_fill_import_object (self, p, file):
      file.write ('         PyObject *oFunc;\n');
      file.write ('         oFunc = PyLong_FromUnsignedLong (importHandle);\n');
      file.write ('         PyObject_SetAttrString (oResult, "__import_handle", oFunc);\n');
      intf = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
      for m in intf.methods():
         file.write ('         oFunc = PyCFunction_NewEx (&%s_%s_def, oResult, NULL);\n'%(
            self.name_interface(intf),self.name_method(m)
         ));
         file.write ('         result = PyObject_SetAttrString (oResult, "%s", oFunc);\n'%(m.name()));

   def source_write_port_register (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_register(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result = __python_initialize (componentHandle);\n');
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      PyObject *oResult = __python_register_unregister (\n');
      file.write ('         componentHandle, importHandle, "%s"\n'%(self.name_port_register(port)));
      file.write ('      );\n');
      file.write ('      if (oResult != NULL) {\n');
      if port.provided() == False:
         self.source_write_fill_import_object (port, file);
      file.write ('         result = xCOM_ImportSetSpecific (componentHandle, importHandle, oResult);\n');
      file.write ('         if (result != XC_OK) Py_DECREF (oResult);\n');
      file.write ('      }\n');
      file.write ('      else {\n');
      file.write ('         result = XC_ERR_INVAL;\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   # Unconditionally set a port register method
   def write_provided_port_register (self, p, file):
      file.write ('         %s,\n'%(self.name_port_register(p)));

   # Unconditionally set a port unregister method
   def write_provided_port_unregister (self, p, file):
      file.write ('         %s\n'%(self.name_port_unregister(p)));

   def source_write_port_unregister (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_unregister(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result = __python_initialize (componentHandle);\n');
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      PyObject *oResult;\n');
      if port.unregister() == True:
         file.write ('      oResult = __python_register_unregister (\n');
         file.write ('         componentHandle, importHandle, "%s"\n'%(self.name_port_unregister(port)));
         file.write ('      );\n');
         file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
         file.write ('      else {\n');
         file.write ('         result = XC_ERR_INVAL;\n');
         file.write ('      }\n');
      file.write ('      oResult = xCOM_ImportGetSpecific (componentHandle, importHandle);\n');
      file.write ('      if (oResult != NULL) {\n');
      file.write ('         Py_DECREF (oResult);\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_port_prologue (self, component, port, file):
      self.source_write_port_register (component, port, file);
      self.source_write_port_unregister (component, port, file);

