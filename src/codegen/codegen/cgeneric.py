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

class CodeGenerator:
   def __init__ (self, name, components, interfaces, opts):
      self.m_name = name;
      self.m_components = components;
      self.m_interfaces = interfaces;
      self.m_options = opts;
   
   def header_file (self):
      return self.m_name + ".h";
   
   def source_file (self):
      return self.m_name + ".c";
   
   def name_identifier (self, name):
      name = string.replace (name, '.', '_');
      name = string.replace (name, '-', '_');
      return name;
   
   def base_name_interface (self, intf):
      name  = string.lower(self.name_identifier(intf.name()));
      return name;
   
   def name_interface (self, intf):
      name  = self.base_name_interface (intf);
      name += "_v" + str(intf.versionMajor()) + "_" + str(intf.versionMinor());
      return name;
   
   def name_method (self, method):
      name = method.name();
      return name;
   
   def name_argument (self, arg):
      name = 'arg_' + arg.name();
      return name;
   
   def proto_arg (self, arg, indent):
      proto = ',\n' + indent + self.name_type(arg.type(),arg.direction()) + ' ';
      proto += '/* ' + arg.direction() + ' */ ';
      proto += self.name_argument(arg);
      return proto;

   def proto_method_result (self, method, indent):
      proto = '(\n';
      proto += indent + '   xc_handle_t importHandle,\n';
      proto += indent + '   xc_result_t result';
      for a in method.arguments():
         if a.direction() == 'out':
            proto += self.proto_arg(a, indent + '   ');
      proto += ',\n   ' + indent + 'void *user_data';
      proto += '\n' + indent + ')';
      return proto;

   def proto_method_error (self, method, indent):
      proto = '(\n';
      proto += indent + '   xc_handle_t importHandle,\n';
      proto += indent + '   xc_result_t error,\n';
      proto += indent + '   void *user_data\n';
      proto += indent + ')';
      return proto;
      
   def proto_method (self, method, indent):
      proto = '(\n' + indent + '   xc_handle_t importHandle';
      for a in method.arguments():
         if a.direction() == 'in':
            proto += self.proto_arg(a, indent + '   ');
      proto += ',\n' + indent + '   void (* %s_result) %s'%(
         self.name_method(method),
         self.proto_method_result(method, indent + '   ')
      );
      proto += ',\n' + indent + '   void (* %s_error) %s'%(
         self.name_method(method),
         self.proto_method_error(method, indent + '   ')
      );
      proto += ',\n   ' + indent + 'void *user_data';
      proto += '\n' + indent + ')';
      return proto;
     
   def name_provided_method (self, port, method):
      name = string.lower(self.base_name_port(port)) + '_' + string.lower(self.name_method(method));
      return name;
   
   def base_name_port (self, port):
      name = self.name_identifier(port.name());
      return name;
   
   def name_port (self, port):
      name = self.base_name_port (port);
      if port.provided() == True:
         name += "Export";
      else:
         name += "Import";
      return name;
   
   def name_port_handle (self, port):
      name = self.name_port (port) + "Handle";
      return name;
   
   def name_port_register (self, port):
      name = string.lower(self.base_name_port(port)) + "_register";
      return name;
   
   def name_port_unregister (self, port):
      name = string.lower(self.base_name_port(port)) + "_unregister";
      return name;
   
   def name_component (self, comp):
      name = self.name_identifier(comp.name())
      return name;
   
   def name_component_init (self, comp):
      name = self.name_component (comp) + "_init";
      return name;
   
   def name_component_destroy (self, comp):
      name = self.name_component (comp) + "_destroy";
      return name;
   
   def name_type (self, type, direction):
      if type == 'y':
         return 'uint8_t';
      elif type == 'b':
         return 'bool';
      elif type == 'n':
         return 'int16_t';
      elif type == 'q':
         return 'uint16_t';
      elif type == 'i':
         return 'int32_t';
      elif type == 'u':
         return 'uint32_t';
      elif type == 'x':
         return 'int64_t';
      elif type == 't':
         return 'uint64_t';
      elif type == 'd':
         return 'double';
      elif type == 's':
         if direction == 'in':
            return 'const char *';
         else:
            return 'char *';
      # TODO everything else to default to variant type
      return None;
      
   def port_flags (self, port):
      flags = '';
      if port.runtime() == False:
         flags = 'XC_PORTF_LOADTIME';
      if flags == '':
         flags = 'XC_PORTF_NONE';
      return flags;
      
   def write (self):
      self.write_header ();
      self.write_source ();
   
   def write_switch_decls (self, file):
      for i in self.m_interfaces:
         if i.references() == 0:
            continue;
         file.write ('\n/* Interface %s version %u.%u */\n'%(i.name(), i.versionMajor(), i.versionMinor()));
         file.write ('typedef struct %s {\n'%(self.name_interface(i)));
         file.write ('   XC_INTERFACE;\n');
         for m in i.methods():
            file.write ('   xc_result_t (* %s) %s;\n'%(self.name_method(m), self.proto_method(m, '   ')));
         file.write ('} %s_t;\n'%(self.name_interface(i)));
         
   def write_switch_aliases (self, file):
      inames = {};
      for i in self.m_interfaces:
         if not i.name() in inames:
            latest = match.latest_interface (self.m_interfaces, i.name());
            if latest is not None:
               file.write ('\n/* %s: default version typedef to %u.%u */\n'%(
                  i.name(),
                  latest.versionMajor(),
                  latest.versionMinor()));
               file.write ('#define %s_NAME "%s"\n'%(string.upper(self.base_name_interface(latest)), latest.name()));
               file.write ('#define %s_VERSION_MAJOR %u\n'%(string.upper(self.base_name_interface(latest)), latest.versionMajor()));
               file.write ('#define %s_VERSION_MINOR %u\n'%(string.upper(self.base_name_interface(latest)), latest.versionMinor()));
               file.write ('typedef %s_t %s_t;\n'%(
                  self.name_interface(latest),
                  self.base_name_interface(latest)));
               inames[i.name()] = latest;
   
   def write_component_decls (self, comp, file):
      if comp.init() == True:
         file.write ('\n/* %s component init(). */\n'%(comp.name()));
         file.write ('extern xc_result_t\n');
         file.write ('%s (\n'%(self.name_component_init(comp)));
         file.write ('   xc_handle_t importHandle\n');
         file.write (');\n');
      if comp.destroy() == True:
         file.write ('\n/* %s component destroy(). */\n'%(comp.name()));
         file.write ('extern xc_result_t\n');
         file.write ('%s (\n'%(self.name_component_destroy(comp)));
         file.write ('   xc_handle_t importHandle\n');
         file.write (');\n');
         
   def write_port_decls (self, p, file):
      if p.register() == True:
         file.write ('\n/* %s port register(). */\n'%(p.name()));
         file.write ('extern xc_result_t\n');
         file.write ('%s (\n'%(self.name_port_register(p)));
         file.write ('   xc_handle_t componentHandle,\n');
         file.write ('   xc_handle_t importHandle\n');
         file.write (');\n');
      if p.unregister() == True:
         file.write ('\n/* %s port unregister(). */\n'%(p.name()));
         file.write ('extern xc_result_t\n');
         file.write ('%s (\n'%(self.name_port_unregister(p)));
         file.write ('   xc_handle_t componentHandle,\n');
         file.write ('   xc_handle_t importHandle\n');
         file.write (');\n');
      i = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
      if p.provided() == True:
         for m in i.methods():
            file.write ('\n/* %s.%s implementation for port %s */\n'%(i.name(),m.name(),p.name()));
            file.write ('extern xc_result_t\n');
            file.write ('%s %s;\n'%(self.name_provided_method(p, m), self.proto_method(m, '')));
      else:
         if p.runtime() == False:
            file.write ('\n/* Handle for the loadtime port %s */\n'%(p.name()));
            file.write ('extern xc_handle_t %s;\n\n'%(self.name_port_handle(p)));
               
   def write_header (self):
      filename = self.header_file();
      hfile = open (filename, "w");
      hfile.write ('/* Generated file, DO NOT EDIT! */\n\n');
      hfile.write ('#ifndef XCOM_CODEGEN_H\n');
      hfile.write ('#define XCOM_CODEGEN_H\n\n');
      hfile.write ('#include <xCOM.h>\n\n');
      hfile.write ('#ifdef __cplusplus\n');
      hfile.write ('extern "C" {\n');
      hfile.write ('#endif\n');
      self.write_switch_decls (hfile);
      self.write_switch_aliases (hfile);
      for c in self.m_components:
         self.write_component_decls (c, hfile);
         for p in c.ports():
            self.write_port_decls (p, hfile);
      hfile.write ('\n#ifdef __cplusplus\n');
      hfile.write ('}\n');
      hfile.write ('#endif\n\n');         
      hfile.write ('#endif /* XCOM_CODEGEN_H */\n\n');
      hfile.close ();
      print '# Wrote ' + filename;
   
   def write_provided_intf (self, file):
      for c in self.m_components:
         for p in c.ports():
            if p.provided() == True:
               file.write ('/* Provided port %s: %s v%u.%u */\n'%(
                  p.name(),
                  p.interface(),
                  p.versionMajor(),
                  p.versionMinor()));
               intf = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
               latest = match.latest_interface (self.m_interfaces, p.interface());
               if intf == latest:
                  name = self.base_name_interface(intf);
               else:
                  name = self.name_interface(intf);
               file.write ('const %s_t %s = {\n'%(name, self.name_port (p)));
               file.write ('   XC_INTERFACE_INIT (\n');
               file.write ('      "%s", /* name */\n'%(intf.name()));
               file.write ('      %u, /* major version */\n'%(intf.versionMajor()));
               file.write ('      %u  /* minor version */\n'%(intf.versionMinor()));
               file.write ('   )');
               for m in intf.methods():
                  file.write (',\n');
                  file.write ('   __queue_%s'%(self.name_provided_method(p,m)));
               file.write ('\n};\n\n');
   
   def write_required_intf (self, file):
      for c in self.m_components:
         for p in c.ports():
            if p.provided() == False:
               file.write ('/* Required port %s: %s v%u.%u */\n'%(
                  p.name(),
                  p.interface(),
                  p.versionMajor(),
                  p.versionMinor()));
               intf = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
               file.write ('const xc_interface_t %s =\n'%(self.name_port (p)));
               file.write ('   XC_INTERFACE_INIT (\n');
               file.write ('      "%s", /* name */\n'%(intf.name()));
               file.write ('      %u, /* major version */\n'%(intf.versionMajor()));
               file.write ('      %u  /* minor version */\n'%(intf.versionMinor()));
               file.write ('   );\n\n');
               if p.runtime() == False:
                  file.write ('/* Handle for the loadtime import from port %s */\n'%(p.name()));
                  file.write ('xc_handle_t %s = XC_INVALID_HANDLE;\n\n'%(self.name_port_handle(p)));

   def write_provided_port_register (self, p, file):
      if p.register() == True:
         file.write ('         %s,\n'%(self.name_port_register(p)));
      else:
         file.write ('         NULL,\n');

   def write_provided_port_unregister (self, p, file):
      if p.unregister() == True:
         file.write ('         %s\n'%(self.name_port_unregister(p)));
      else:
         file.write ('         NULL\n');

   def write_provided_ports_init (self, comp, file):
      for p in comp.ports():
         if p.provided() == False:
            continue;
         i = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
         file.write ('      /* Port %s for provided interface %s. */\n'%(p.name(),i.name()));
         file.write ('      XC_DECLARE_PROVIDED_PORT (\n');
         file.write ('         "%s",\n'%(p.name()));
         file.write ('         (xc_interface_t *) &%s,\n'%(self.name_port(p)));
         file.write ('         sizeof (%s),\n'%(self.name_port(p)));
         file.write ('         NULL,\n'); # FIXME: remove in framework (used to be proto)
         file.write ('         NULL,\n'); # TODO: pass encoded interface spec
         self.write_provided_port_register (p, file);
         self.write_provided_port_unregister (p, file);
         file.write ('      ),\n');
         
   def write_required_ports_init (self, comp, file):
      for p in comp.ports():
         if p.provided() == True:
            continue;
         i = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
         file.write ('      /* Port %s for required interface %s. */\n'%(p.name(),i.name()));
         file.write ('      XC_DECLARE_REQUIRED_PORT (\n');
         file.write ('         "%s", /* name */\n'%(p.name()));
         file.write ('         (xc_interface_t *) &%s, /* interface spec */\n'%(self.name_port(p)));
         if p.runtime() == False:
            file.write ('         &%s, /* handle */\n'%(self.name_port_handle(p)));
         else:
            file.write ('         NULL, /* handle */\n');
         if p.component() is None or p.component() == '':
            file.write ('         NULL, /* component */\n');
         else:
            file.write ('         "%s", /* component */\n'%(p.component()));
         if p.port() is None or p.port() == '':
            file.write ('         NULL, /* port */\n');
         else:
            file.write ('         "%s", /* port */\n'%(p.port()));
         file.write ('         NULL, /* reserved */\n'); # TODO: pass encoded interface spec
         if p.register() == True:
            file.write ('         %s,\n'%(self.name_port_register(p)));
         else:
            file.write ('         NULL,\n');
         if p.unregister() == True:
            file.write ('         %s,\n'%(self.name_port_unregister(p)));
         else:
            file.write ('         NULL,\n');
         file.write ('         %s /* flags */\n'%(self.port_flags(p)));
         file.write ('      ),\n');

   def queue_arg_type (self, a):
      return self.name_type(a.type(),a.direction());

   def queue_free_arg_expr (self, a, expr):
      if a.type() == 's':
         return 'free ((char *) message->%s);'%(self.name_argument(a));
      else:
         return '';

   def write_queue_copy_args_prologue (self, file):
      return None;     

   def write_queue_copy_args_epilogue (self, m, file):
      return None;     

   def queue_copy_arg_expr (self, a, indent):
      result='';
      if a.type() == 's':
         result += indent + 'if ((result == XC_OK) && (%s != NULL)) {\n'%(self.name_argument(a));
         result += indent + '   message->%s = strdup (%s);\n'%(self.name_argument(a),self.name_argument(a));
         result += indent + '   if (message->%s == NULL) {\n'%(self.name_argument(a));
         result += indent + '      result = XC_ERR_NOMEM;\n';
         result += indent + '   }\n';
         result += indent + '}\n';
      else:
         result += indent + 'message->%s = %s;\n'%(self.name_argument(a),self.name_argument(a));
      return result;

   def write_queue_method_call (self, p, m, file):
      args = len(m.arguments());
      file.write('static void\n');
      file.write('__call_%s (\n'%(self.name_provided_method(p,m)));
      file.write('   void *_message\n');
      file.write(') {\n');
      file.write('   struct __queued_message *header = _message;\n');
      if args > 0:
         file.write('   struct __%s_message *message = _message;\n'%(self.name_provided_method(p,m)));
      else:
         file.write('   struct __queued_message *message = _message;\n');
      file.write ('   void (* result_cb) %s = header->result;\n'%(self.proto_method_result(m, '   ')));
      file.write ('   void (* error_cb) %s = header->error;\n'%(self.proto_method_error(m, '   ')));
      file.write('   xc_result_t result = %s (\n'%(self.name_provided_method(p,m)));
      file.write('      header->importHandle');
      for a in m.arguments():
         if a.direction() == 'in':
            file.write(',\n      message->%s'%(self.name_argument(a)));
      file.write(',\n      result_cb');
      file.write(',\n      error_cb');
      file.write(',\n      header->user_data\n');
      file.write('   );\n');
      file.write('   header->free (message);\n');
      file.write('}\n\n');

   def source_write_port_prologue (self, component, port, file):
      return None;

   def source_write_free_message (self, m, file):
      file.write('   free (message);\n');

   def write_queued_message_structs (self, file):
      for c in self.m_components:
         for p in c.ports():
            self.source_write_port_prologue (c, p, file);
            if p.provided() == True:
               intf = match.interface (self.m_interfaces, p.interface(), p.versionMajor(), p.versionMinor());
               for m in intf.methods():
                  args = len(m.arguments());
                  if args > 0:
                     file.write('/* %s: %s() */\n'%(intf.name(), m.name()));
                     file.write('struct __%s_message {\n'%(self.name_provided_method(p,m)));
                     file.write('   struct __queued_message __queued_message;\n');
                     for a in m.arguments():
                        if a.direction() == 'in':
                           file.write('   %s %s;\n'%(self.queue_arg_type(a), self.name_argument(a)));
                     file.write('};\n\n');

                     file.write('static void\n');
                     file.write('__free_%s (\n'%(self.name_provided_method(p,m)));
                     file.write('   void *_message\n');
                     file.write(') {\n');
                     file.write('   struct __%s_message *message = _message;\n'%(self.name_provided_method(p,m)));
                     file.write('   struct __queued_message *header = _message;\n');
                     for a in m.arguments():
                        free_expr = self.queue_free_arg_expr (a, 'message->%s'%(self.name_argument(a)));
                        if free_expr != '':
                           file.write('   %s\n'%(free_expr));
                     self.source_write_free_message (m, file);
                     file.write('}\n\n');

                  self.write_queue_method_call (p, m, file);

                  file.write('static xc_result_t\n');
                  file.write('__queue_%s '%(self.name_provided_method(p,m)));
                  file.write(self.proto_method(m, ''));
                  file.write(' {\n');
                  if args > 0:
                     file.write('   struct __%s_message *message;\n'%(self.name_provided_method(p,m)));
                  else:
                     file.write('   struct __queued_message *message;\n');
                  file.write('   struct __queued_message *header;\n');
                  file.write('   xc_result_t result = XC_OK;\n');
                  file.write('   message = malloc (sizeof (*message));\n');
                  file.write('   if (message != NULL) {\n');
                  file.write('      memset (message, 0, sizeof (*message));\n');
                  file.write('      header = (struct __queued_message *) message;\n');
                  file.write('      header->result = %s_result;\n'%(self.name_method(m)));
                  file.write('      header->error = %s_error;\n'%(self.name_method(m)));
                  file.write('      header->user_data = user_data;\n');
                  file.write('      header->handler = __call_%s;\n'%(self.name_provided_method(p,m)));
                  file.write('      header->importHandle = importHandle;\n');
                  self.write_queue_copy_args_prologue (file);
                  if args > 0:
                     file.write('      header->free = __free_%s;\n'%(self.name_provided_method(p,m)));
                  else:
                     file.write('      header->free = free;\n');
                  for a in m.arguments():
                     file.write(self.queue_copy_arg_expr(a, '      '));
                  self.write_queue_copy_args_epilogue (m, file);
                  file.write('      if (result == XC_OK) {\n');
                  file.write('         pthread_mutex_lock (&__lock);\n');
                  file.write('         XC_CLIST_ADDTAIL (&__queued_messages, message);\n');
                  file.write('         sem_post (&__sem);\n');
                  file.write('         pthread_mutex_unlock (&__lock);\n');
                  file.write('      }\n');
                  file.write('      else {\n');
                  file.write('         __free_%s (message);\n'%(self.name_provided_method(p,m)));
                  file.write('      }\n');
                  file.write('   }\n');
                  file.write('   else {\n');
                  file.write('      result = XC_ERR_NOMEM;\n');
                  file.write('   }\n');
                  file.write('   return result;\n');
                  file.write('}\n\n');

   def source_write_call_user_init (self, comp, file):
      if comp.init() == True:
         file.write ('   result = %s (componentHandle);\n'%(self.name_component_init(comp)));
      else:
         file.write ('   result = XC_OK;\n');

   def source_write_call_user_destroy (self, comp, file):
      if comp.destroy() == True:
         file.write ('   result = %s (componentHandle);\n'%(self.name_component_destroy(comp)));
      else:
         file.write ('   result = XC_OK;\n');

   def write_component_queue_globals (self, comp, file): 
      file.write ('/* mutex to the message queue */\n');
      file.write ('static pthread_mutex_t __lock;\n');
      file.write ('\n');
      file.write ('/* semaphore to wake-up the message queue thread. */\n');
      file.write ('static sem_t __sem;\n');
      file.write ('\n');
      file.write ('/* message queue thread. */\n');
      file.write ('static pthread_t __%s_thread;\n'%(self.name_component(comp)));
      file.write ('\n');
      file.write ('/* boolean for controlling execution of the message queue thread. */\n');
      file.write ('static bool __%s_quit;\n'%(self.name_component(comp)));
      file.write ('\n');

   def source_write_queued_message_struct_members (self, comp, file):
      file.write ('   xc_clist_t node;\n');
      file.write ('   void *method;\n');
      file.write ('   void *result;\n');
      file.write ('   void *error;\n');
      file.write ('   void *user_data;\n');
      file.write ('   void (* free) (void *_message);\n');
      file.write ('   void (* handler) (void *_message);\n');
      file.write ('   xc_handle_t importHandle;\n');

   def source_write_init_destroy_prologue (self, component, file):
      return None;

   def write_component_init_internal (self, comp, file):
      self.write_component_queue_globals (comp, file);

      file.write ('/* struct for queued messages. */\n');
      file.write ('struct __queued_message {\n');
      self.source_write_queued_message_struct_members (comp, file);
      file.write ('};\n');
      file.write ('\n');

      file.write ('/* list of queued messages. */\n');
      file.write ('static xc_clist_t __queued_messages;\n');
      file.write ('\n');
      self.write_queued_message_structs (file);
      file.write ('static void *\n');
      file.write ('__%s_run (\n'%(self.name_component(comp)));
      file.write ('   void *arg\n');
      file.write (') {\n');
      file.write ('   struct __queued_message *message;\n');
      file.write ('   arg = arg;\n');
      file.write ('   while (1) {\n');
      file.write ('      sem_wait (&__sem);\n');
      file.write ('      pthread_mutex_lock (&__lock);\n');
      file.write ('      if (__%s_quit == false) {\n'%(self.name_component(comp)));
      file.write ('         message = XC_CLIST_HEAD (&__queued_messages);\n');
      file.write ('         if (!XC_CLIST_END (&__queued_messages, message)) {\n');
      file.write ('            XC_CLIST_REMOVE (message);\n');
      file.write ('            pthread_mutex_unlock (&__lock);\n');
      file.write ('            message->handler (message);\n');
      file.write ('         }\n');
      file.write ('      }\n');
      file.write ('      else break;\n');
      file.write ('   }\n');
      file.write ('   message = XC_CLIST_HEAD (&__queued_messages);\n');
      file.write ('   while (!XC_CLIST_END (&__queued_messages, message)) {\n');
      file.write ('      struct __queued_message *next = XC_CLIST_NEXT (message);\n');
      file.write ('      XC_CLIST_REMOVE (message);\n');
      file.write ('      message->free (message);\n');
      file.write ('      message = next;\n');
      file.write ('   };\n');
      file.write ('   pthread_mutex_unlock (&__lock);\n');
      file.write ('   return NULL;\n');
      file.write ('}\n');
      file.write ('\n');

      self.source_write_init_destroy_prologue (comp, file);

      file.write ('/* %s component init(). */\n'%(comp.name()));
      file.write ('static xc_result_t\n');
      file.write ('__%s (\n'%(self.name_component_init(comp)));
      file.write ('   xc_handle_t componentHandle\n');
      file.write (') {\n');
      file.write ('   xc_result_t result;\n');
      file.write ('   componentHandle = componentHandle;\n');
      file.write ('\n');
      self.source_write_call_user_init (comp, file);
      file.write ('   if (result == XC_OK) {\n');
      file.write ('      XC_CLIST_INIT (&__queued_messages);\n');
      file.write ('      __%s_quit = false;\n'%(self.name_component(comp)));
      file.write ('      result = sem_init (&__sem, 0, 0);\n');
      file.write ('      if (result == 0) {\n');
      file.write ('         result = pthread_mutex_init (&__lock, NULL);\n');
      file.write ('         if (result == 0) {\n');
      file.write ('            result = pthread_create (\n');
      file.write ('               &__%s_thread,\n'%(self.name_component(comp)));
      file.write ('               NULL,\n');
      file.write ('               __%s_run,\n'%(self.name_component(comp)));
      file.write ('               NULL\n');
      file.write ('            );\n');
      file.write ('            if (result != 0) {\n');
      file.write ('               pthread_mutex_destroy (&__lock);\n');
      file.write ('               sem_destroy (&__sem);\n');
      file.write ('            }\n');
      file.write ('         }\n');
      file.write ('         else {\n');
      file.write ('            sem_destroy (&__sem);\n');
      file.write ('         }\n');
      file.write ('      }\n');
      file.write ('      if (result != 0) {\n');
      self.source_write_call_user_destroy (comp, file);
      file.write ('         result = XC_ERR_NOMEM;\n');
      file.write ('      }\n');
      file.write ('   }\n');
      file.write ('   return result;\n');
      file.write ('}\n');
      file.write ('\n');
         
      file.write ('/* %s component destroy(). */\n'%(comp.name()));
      file.write ('static xc_result_t\n');
      file.write ('__%s (\n'%(self.name_component_destroy(comp)));
      file.write ('   xc_handle_t componentHandle\n');
      file.write (') {\n');
      file.write ('   xc_result_t result;\n');
      file.write ('   componentHandle = componentHandle;\n');
      file.write ('   __%s_quit = true;\n'%(self.name_component(comp)));
      file.write ('   sem_post (&__sem);\n');
      file.write ('   pthread_join (__%s_thread, NULL);\n'%(self.name_component(comp)));
      file.write ('   sem_destroy (&__sem);\n');
      file.write ('   pthread_mutex_destroy (&__lock);\n');
      self.source_write_call_user_destroy (comp, file);
      file.write ('   return result;\n');
      file.write ('}\n');
      file.write ('\n');

   def write_component (self, comp, file):
      file.write ('/* Component %s version %u.%u */\n'%(comp.name(), comp.versionMajor(), comp.versionMinor()));
      file.write ('XC_DECLARE_COMPONENT {\n');
      file.write ('   "%s",\n'%(comp.name()));
      file.write ('   "%s",\n'%(comp.description()));
      file.write ('   %u,\n'%(comp.versionMajor()));
      file.write ('   %u,\n'%(comp.versionMinor()));
      file.write ('   __%s,\n'%(self.name_component_init(comp)));
      file.write ('   __%s,\n'%(self.name_component_destroy(comp)));
      file.write ('   {\n');
      self.write_provided_ports_init(comp, file);
      self.write_required_ports_init(comp, file);
      file.write ('      { NULL, NULL, 0, NULL, NULL, NULL, 0, NULL, NULL, NULL }\n');
      file.write ('   }\n');
      file.write ('};\n\n');

   def write_source_includes (self, file):
      file.write ('#include <xCOM/clist.h>\n');
      file.write ('#include <pthread.h>\n');
      file.write ('#include <semaphore.h>\n');
      file.write ('#include <stdbool.h>\n');
      file.write ('#include <stdlib.h>\n');
      file.write ('#include <string.h>\n');
      file.write ('#include "%s"\n'%(self.header_file()));
 
   def write_source (self):
      filename = self.source_file();
      cfile = open (filename, "w");
      cfile.write ('/* Generated file, DO NOT EDIT! */\n\n');
      self.write_source_includes (cfile);
      cfile.write ('\n');
      for c in self.m_components:
         self.write_component_init_internal (c, cfile);
      self.write_provided_intf (cfile);
      self.write_required_intf (cfile);
      for c in self.m_components:
         self.write_component (c, cfile);
      cfile.close ();
      print '# Wrote ' + filename;
