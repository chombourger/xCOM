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
      file.write ('/* Generic Method Error Python/C callback. */\n');
      file.write ('static PyObject *\n');
      file.write ('method_error (\n');
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

   def source_write_queued_message_struct_members (self, comp, file):
      cgeneric.CodeGenerator.source_write_queued_message_struct_members (self, comp, file);
      file.write ('   PyObject *oMethodName;\n');
      file.write ('   PyObject *oUserData;\n');
      file.write ('   PyObject *oError;\n');
      file.write ('   PyMethodDef oErrorDef;\n');
      file.write ('   PyObject *oResult;\n');
      file.write ('   PyMethodDef oResultDef;\n');

   def write_component_queue_globals (self, comp, file):
      cgeneric.CodeGenerator.write_component_queue_globals (self, comp, file);
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
      file.write ('static PyObject *\n');
      file.write ('method_error (\n');
      file.write ('   PyObject *oSelf,\n');
      file.write ('   PyObject *oArgs\n');
      file.write (');\n\n');

      file.write ('static PyObject *\n');
      file.write ('__python_register_unregister (\n');
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle,\n'); 
      file.write ('   const char *name\n'); 
      file.write (') {\n');
      file.write ('   PyObject *result = NULL;\n');
      file.write ('   PyObject *oDict = PyModule_GetDict (__module);\n');
      file.write ('   PyObject *oFunc = PyDict_GetItemString (oDict, name);\n');
      file.write ('   if (PyCallable_Check (oFunc)) {\n');
      file.write ('      result = PyObject_CallFunction (oFunc, "(II)", componentHandle, importHandle);\n');
      file.write ('   }\n');
      file.write ('   else {\n');
      file.write ('      PyErr_Print ();\n');
      file.write ('      result = NULL;\n');
      file.write ('   }\n');
      file.write ('   if (oFunc != NULL) Py_DECREF (oFunc);\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

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
      file.write ('         header->oErrorDef.ml_meth = method_error;\n');
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

   def write_port_decls (self, p, file):
      return None;

   def source_write_call_python_init (self, comp, file, indent):
      if comp.init() == True:
         file.write (indent + '/* Call component init() in Python. */\n');
         file.write (indent + 'result = __python_init_destroy (componentHandle, "init");\n');
      else:
         file.write (indent + 'result = XC_OK;\n');
 
   # FIXME find a better way to add path to python component
   def source_write_call_user_init (self, comp, file):
      file.write ('   __component_handle = componentHandle;\n');
      file.write ('   Py_Initialize ();\n');
      file.write ('   PyEval_InitThreads ();\n');
      file.write ('   __global = PyThreadState_Get ();\n');
      file.write ('   __component = Py_NewInterpreter ();\n');
      file.write ('   if (__component != NULL) {\n');
      file.write ('      char syspath [1024];\n');
      file.write ('      strcpy (syspath, "import sys\\nsys.path.append(\'");\n');
      file.write ('      strcat (syspath, xCOM_GetComponentBundlePath (componentHandle));\n');
      file.write ('      strcat (syspath, "/Code/python\')\\n");\n');
      file.write ('      PyRun_SimpleString (syspath);\n');
      file.write ('      __module = PyImport_ImportModule ("%s");\n'%(comp.name()));
      file.write ('      if (__module != NULL) {\n');
      self.source_write_call_python_init (comp, file, '         ');
      file.write ('      }\n');
      file.write ('      else {\n');
      file.write ('         Py_EndInterpreter (__component);\n');
      file.write ('         result = XC_ERR_NOENT;\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   else {\n');
      file.write ('      result = XC_ERR_NOMEM;\n');
      file.write ('   }\n');
      file.write ('   PyThreadState_Swap (__global);\n');

   def source_write_call_user_destroy (self, comp, file):
      if comp.destroy() == True:
         file.write ('   result = __python_init_destroy (componentHandle, "destroy");\n');
      else:
         file.write ('   result = XC_OK;\n');
      file.write ('PyThreadState_Swap (__component);\n');
      file.write ('Py_EndInterpreter (__component);\n');
      file.write ('__component = NULL;\n');
      file.write ('PyThreadState_Swap (__global);\n');
      file.write ('Py_Finalize ();\n');

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
      file.write ('         PyErr_Print ();\n');
      file.write ('         if (error_cb != NULL) error_cb (header->importHandle, XC_ERR_INVAL, header->user_data);\n');
      file.write ('      }\n');
      file.write ('      PyThreadState_Swap (__global);\n');
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
      file.write ('   xc_result_t result;\n');
      file.write ('   void (* result_cb) %s;\n'%(self.proto_method_result(m, '   ')));
      file.write ('   oImportHandle = PyTuple_GetItem (oArgs, 0);\n');
      file.write ('   importHandle  = PyInt_AsUnsignedLongMask (oImportHandle);\n');
      file.write ('   oResult = PyTuple_GetItem (oArgs, 1);\n');
      file.write ('   result  = PyInt_AsLong (oResult);\n');
      file.write ('   header   = PyCObject_AsVoidPtr (oSelf);\n');
      file.write ('   result_cb = header->result;\n');
      file.write ('   result_cb (importHandle, result, header->user_data);\n');
      file.write ('   Py_RETURN_NONE;\n');
      file.write ('}\n\n');

   def source_write_init_destroy_prologue (self, component, file):
      file.write ('static xc_result_t\n');
      file.write ('__python_init_destroy (\n');
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   const char *name\n');
      file.write (') {\n');
      file.write ('   xc_result_t result = XC_OK;\n');
      file.write ('   PyObject *oDict = PyModule_GetDict (__module);\n');
      file.write ('   PyObject *oFunc = PyDict_GetItemString (oDict, name);\n');
      file.write ('   if (PyCallable_Check (oFunc)) {\n');
      file.write ('      PyObject *oResult = PyObject_CallFunction (oFunc, "(I)", componentHandle);\n');
      file.write ('      if (oResult != NULL) {\n');
      file.write ('         result = PyInt_AsLong (oResult);\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   else {\n');
      file.write ('      PyErr_Print ();\n');
      file.write ('      result = XC_ERR_INVAL;\n');
      file.write ('   }\n');
      file.write ('   if (oFunc != NULL) Py_DECREF (oFunc);\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_port_register (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_register(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result;\n');
      file.write ('   PyObject *oResult = __python_register_unregister (\n');
      file.write ('      componentHandle, importHandle, "%s"\n'%(self.name_port_register(port)));
      file.write ('   );\n');
      file.write ('   if (oResult != NULL) {\n');
      file.write ('      result = xCOM_ImportSetSpecific (componentHandle, importHandle, oResult);\n');
      file.write ('      if (result != XC_OK) Py_DECREF (oResult);\n');
      file.write ('   }\n');
      file.write ('   else {\n');
      file.write ('      PyErr_Print ();\n');
      file.write ('      result = XC_ERR_INVAL;\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   # Unconditionally set a port unregister method
   def write_provided_port_unregister (self, p, file):
      file.write ('         %s\n'%(self.name_port_unregister(p)));

   def source_write_port_unregister (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_unregister(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result = XC_OK;\n');
      file.write ('   PyObject *oResult;\n');
      if port.unregister() == True:
         file.write ('   oResult = __python_register_unregister (\n');
         file.write ('      componentHandle, importHandle, "%s"\n'%(self.name_port_unregister(port)));
         file.write ('   );\n');
         file.write ('   if (oResult != NULL) Py_DECREF (oResult);\n');
         file.write ('   else {\n');
         file.write ('      PyErr_Print ();\n');
         file.write ('      result = XC_ERR_INVAL;\n');
         file.write ('   }\n');
      file.write ('   oResult = xCOM_ImportGetSpecific (componentHandle, importHandle);\n');
      file.write ('   if (oResult != NULL) {\n');
      file.write ('      Py_DECREF (oResult);\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_port_prologue (self, component, port, file):
      if port.provided() == True:
         if port.register() == True:
            self.source_write_port_register (component, port, file);
         self.source_write_port_unregister (component, port, file);

