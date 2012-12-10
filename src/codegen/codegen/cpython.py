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
      file.write ('#include <pyCOM.h>\n');
      cgeneric.CodeGenerator.write_source_includes (self, file);

   def name_queued_message_struct (self):
      return 'pyCOM_queued_call_t';

   def write_component_init_internal (self, comp, file):
      cgeneric.CodeGenerator.write_component_init_internal (self, comp, file);

   def source_write_global_decls (self, file):
      return None;

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
      file.write ('   xc_handle_t componentHandle;\n');
      file.write ('   pycom_context_t *contextPtr;\n');
      for a in m.arguments():
         if a.direction() == 'out':
            file.write ('   PyObject *py_%s;\n'%(self.name_argument(a)));

      file.write ('\n');
      file.write ('   componentHandle = xCOM_ImportGetClient (importHandle);\n');
      file.write ('   contextPtr = pyCOM_ContextEnter (componentHandle);\n');
      file.write ('   if (contextPtr == NULL) return;\n\n');

      file.write ('   oContext = user_data;\n');
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
      file.write ('   Py_DECREF (oContext);\n\n');

      file.write ('   pyCOM_ContextLeave (contextPtr);\n');
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
         file.write ('      pyCOM_HandleNativeMethodResultToPython,\n');
      file.write ('      pyCOM_HandleNativeMethodErrorToPython,\n');
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
      file.write ('/* Component handle. */\n');
      file.write ('static xc_handle_t __component_handle;\n');
      file.write ('\n');
      file.write ('/* Forward declaration. */\n');
      file.write ('struct __xc_component_decl__ XC_DECLARED_COMPONENT_SYM;\n');
      file.write ('\n');

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

   def source_write_init_queued_call (self, m, file):
      oargs = self.out_arguments (m);
      if oargs > 0:
         result_func = '__' + self.name_method(m) + '_method_result';
      else:
         result_func = 'pyCOM_HandlePythonMethodResultToNative';

      file.write ('      pycom_context_t *contextPtr;\n');
      file.write ('      header->result = %s_result;\n'%(self.name_method(m)));
      file.write ('      header->error  = %s_error;\n'%(self.name_method(m)));
      file.write ('      contextPtr = pyCOM_ContextEnter (__component_handle);\n');
      file.write ('      pyCOM_InitQueuedCall (\n');
      file.write ('         contextPtr, header, "%s", importHandle, %s, %s_error, user_data\n'%(
         m.name(), result_func, self.name_method(m)
      )); 
      file.write ('      );\n');

   def write_queue_copy_args_epilogue (self, m, file):
      file.write ('      pyCOM_ContextLeave (contextPtr);\n');
      file.write ('\n');

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
         result += indent + 'message->%s = %s (%s);\n'%(
            self.name_argument(a),
            self.native_type_to_python(a.type()),
            self.name_argument(a)
         );
      return result;

   # Do not write the C declarations for the provided methods since they
   # would be implemented in Python instead!
   def write_port_decls (self, p, file):
      return None;

   def source_write_call_python_init (self, comp, file, indent):
      if comp.init() == True:
         file.write (indent + '/* Call component init() in Python. */\n');
         file.write (indent + 'result = pyCOM_InvokeInit (componentHandle);\n');
      else:
         file.write (indent + 'result = XC_OK;\n');
 
   def source_write_call_user_init (self, comp, file):
      file.write ('   __component_handle = componentHandle;\n');
      self.source_write_call_python_init (comp, file, '   ');

   def source_write_call_user_destroy (self, comp, file):
      if comp.destroy() == True:
         file.write ('   result = pyCOM_InvokeDestroy (componentHandle);\n');
      else:
         file.write ('   result = XC_OK;\n');
      file.write ('   pyCOM_Destroy (componentHandle);\n');

   def write_queue_method_call (self, p, m, file):
      args = len(m.arguments());
      oargs = self.out_arguments (m);
      iargs = 0;
      for a in m.arguments():
         if a.direction() == 'in':
            iargs = iargs + 1;
      file.write ('static void\n');
      file.write ('__call_%s (\n'%(self.name_provided_method(p,m)));
      file.write ('   void *_message\n');
      file.write (') {\n');
      file.write ('   %s *callPtr = _message;\n'%(self.name_queued_message_struct()));
      if args > 0:
         file.write ('   struct __%s_message *message = _message;\n'%(self.name_provided_method(p,m)));
      else:
         file.write ('   %s *message = _message;\n'%(self.name_queued_message_struct()));
      file.write ('   void (* error_cb) %s = callPtr->error;\n\n'%(self.proto_method_error(m, '   ')));
      file.write ('   xc_handle_t componentHandle;\n');
      file.write ('   pycom_context_t *contextPtr;\n');
      file.write ('   PyObject *oResult;\n\n');

      file.write ('   componentHandle = xCOM_ImportGetServer (callPtr->importHandle);\n');
      file.write ('   contextPtr = pyCOM_ContextEnter (componentHandle);\n');
      file.write ('   if (contextPtr == NULL) {\n');
      file.write ('      if (error_cb != NULL) error_cb (callPtr->importHandle, XC_ERR_INVAL, callPtr->user_data);\n');
      file.write ('      return;\n');
      file.write ('   }\n\n');

      file.write ('   PyObject *object = xCOM_ImportGetSpecific (__component_handle, callPtr->importHandle);\n');
      file.write ('   if (object != NULL) {\n');
      file.write ('      PyErr_Clear ();\n');
      file.write ('      oResult = PyObject_CallMethodObjArgs (\n');
      file.write ('         object,\n');
      file.write ('         callPtr->oMethodName,\n');
      for a in m.arguments():
         if a.direction() == 'in':
            file.write ('         message->%s,\n'%(self.name_argument(a)));
      file.write ('         callPtr->oResult,\n');
      file.write ('         callPtr->oError,\n');
      file.write ('         callPtr->oUserData,\n');
      file.write ('         NULL\n');
      file.write ('      );\n');
      file.write ('      if (oResult != NULL) Py_DECREF (oResult);\n');
      file.write ('      else {\n');
      file.write ('         if (PyErr_Occurred ()) PyErr_Print ();\n');
      file.write ('         if (error_cb != NULL) error_cb (callPtr->importHandle, XC_ERR_INVAL, callPtr->user_data);\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   pyCOM_ContextLeave (contextPtr);\n');
      file.write ('   callPtr->free (message);\n');
      file.write ('}\n\n');

      # TODO handle out arguments
      if oargs == 0:
         return None;

      file.write ('static PyObject *\n');
      file.write ('__%s_method_result (\n'%(self.name_method(m)));
      file.write ('   PyObject *oSelf,\n');
      file.write ('   PyObject *oArgs\n');
      file.write (') {\n');
      file.write ('   %s *header;\n'%(self.name_queued_message_struct()));
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

   # Unconditionally set a port register method
   def write_provided_port_register (self, p, file):
      file.write ('         %s,\n'%(self.name_port_register(p)));

   # Unconditionally set a port unregister method
   def write_provided_port_unregister (self, p, file):
      file.write ('         %s\n'%(self.name_port_unregister(p)));

   def source_write_port_register (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_register(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result = pyCOM_Init (componentHandle, &XC_DECLARED_COMPONENT_SYM);\n');
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      result = pyCOM_InvokeRegister (\n');
      file.write ('         componentHandle, importHandle, "%s"\n'%(self.name_port_register(port)));
      file.write ('      );\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_port_unregister (self, component, port, file):
      file.write ('static xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_unregister(port)));
      file.write ('   xc_handle_t componentHandle,\n'); 
      file.write ('   xc_handle_t importHandle\n'); 
      file.write (') {\n');
      file.write ('   xc_result_t result = pyCOM_Init (componentHandle, &XC_DECLARED_COMPONENT_SYM);\n');
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      result = pyCOM_InvokeUnRegister (\n');
      file.write ('         componentHandle, importHandle, ');
      if port.unregister() == True:
         file.write ('"%s"'%(self.name_port_register(port)));
      else:
         file.write ('NULL');
      file.write ('\n      );\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n\n');

   def source_write_port_prologue (self, component, port, file):
      self.source_write_port_register (component, port, file);
      self.source_write_port_unregister (component, port, file);

