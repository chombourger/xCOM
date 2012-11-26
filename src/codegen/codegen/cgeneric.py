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
      if arg.direction == 'out':
         proto += '*';
      proto += self.name_argument(arg);
      return proto;
      
   def proto_method (self, method, indent):
      proto = '(\n' + indent + '   xc_handle_t importHandle';
      for a in method.arguments():
         proto += self.proto_arg(a, indent + '   ');
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
      file.write ('\n/* %s component init(). */\n'%(comp.name()));
      file.write ('extern xc_result_t\n');
      file.write ('%s (\n'%(self.name_component_init(comp)));
      file.write ('   xc_handle_t importHandle\n');
      file.write (');\n');
      file.write ('\n/* %s component destroy(). */\n'%(comp.name()));
      file.write ('extern xc_result_t\n');
      file.write ('%s (\n'%(self.name_component_destroy(comp)));
      file.write ('   xc_handle_t importHandle\n');
      file.write (');\n');
      
   def write_required_port_decls (self, port, file):
      if port.runtime() == False:
         file.write ('\n/* Handle for the loadtime port %s */\n'%(port.name()));
         file.write ('extern xc_handle_t %s;\n\n'%(self.name_port_handle(port)));
         
   def write_provided_port_decls (self, port, file):
      file.write ('\n/* %s port register(). */\n'%(port.name()));
      file.write ('extern xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_register(port)));
      file.write ('   xc_handle_t componentHandle,\n');
      file.write ('   xc_handle_t importHandle\n');
      file.write (');\n');
      file.write ('\n/* %s port unregister(). */\n'%(port.name()));
      file.write ('extern xc_result_t\n');
      file.write ('%s (\n'%(self.name_port_unregister(port)));
      file.write ('   xc_handle_t componentHandle,\n');
      file.write ('   xc_handle_t importHandle\n');
      file.write (');\n');
      i = match.interface (self.m_interfaces, port.interface(), port.versionMajor(), port.versionMinor());
      for m in i.methods():
         file.write ('\n/* %s.%s implementation for port %s */\n'%(i.name(),m.name(),port.name()));
         file.write ('extern xc_result_t\n');
         file.write ('%s %s;\n'%(self.name_provided_method(port, m), self.proto_method(m, '')));
               
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
            if p.provided() == True:
               self.write_provided_port_decls (p, hfile);
            else:
               self.write_required_port_decls(p, hfile);
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
                  file.write ('   %s'%(self.name_provided_method(p,m)));
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
         file.write ('         %s,\n'%(self.name_port_register(p)));
         file.write ('         %s\n'%(self.name_port_unregister(p)));
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
         file.write ('         %s /* flags */\n'%(self.port_flags(p)));
         file.write ('      ),\n');
         
   def write_component (self, comp, file):
      file.write ('/* Component %s version %u.%u */\n'%(comp.name(), comp.versionMajor(), comp.versionMinor()));
      file.write ('XC_DECLARE_COMPONENT {\n');
      file.write ('   "%s",\n'%(comp.name()));
      file.write ('   "%s",\n'%(comp.description()));
      file.write ('   %u,\n'%(comp.versionMajor()));
      file.write ('   %u,\n'%(comp.versionMinor()));
      file.write ('   %s,\n'%(self.name_component_init(comp)));
      file.write ('   %s,\n'%(self.name_component_destroy(comp)));
      file.write ('   {\n');
      self.write_provided_ports_init(comp, file);
      self.write_required_ports_init(comp, file);
      file.write ('      { NULL, NULL, 0, NULL, NULL, NULL, 0, NULL, NULL, NULL }\n');
      file.write ('   }\n');
      file.write ('};\n\n');
                  
   def write_source (self):
      filename = self.source_file();
      cfile = open (filename, "w");
      cfile.write ('/* Generated file, DO NOT EDIT! */\n\n');
      cfile.write ('#include "%s"\n\n'%(self.header_file()));
      self.write_provided_intf (cfile);
      self.write_required_intf (cfile);
      for c in self.m_components:
         self.write_component (c, cfile);
      cfile.close ();
      print '# Wrote ' + filename;
