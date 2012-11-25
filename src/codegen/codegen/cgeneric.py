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

class CodeGenerator:
   def __init__ (self, name, components, interfaces, opts):
      self.m_name = name;
      self.m_components = components;
      self.m_interfaces = interfaces;
      self.m_options = opts;
   
   def name_interface (self, intf):
      name  = string.replace (intf.name(), ".", "_");
      name += "_v" + str(intf.versionMajor()) + "_" + str(intf.versionMinor());
      return name;
   
   def write (self):
      self.write_header ();
      self.write_source ();
      
   def write_header (self):
      filename = self.m_name + ".h";
      h = open (filename, "w");
      h.write ('/* Generated file, DO NOT EDIT! */\n\n');
      h.write ('#ifndef XCOM_CODEGEN_H\n');
      h.write ('#define XCOM_CODEGEN_H\n\n');
      for i in self.m_interfaces:
         if i.references() == 0:
            continue;
         h.write ('/* Interface %s version %u.%u */\n'%(i.name(), i.versionMajor(), i.versionMinor()));
         h.write ('typedef struct %s {\n'%(self.name_interface(i)));
         h.write ('   XC_INTERFACE;\n');
         h.write ('} %s_t;\n'%(self.name_interface(i)));
         h.write ('\n');
      h.write ('#endif /* XCOM_CODEGEN_H */\n\n');
      h.close ();
      
   def write_source (self):
      filename = self.m_name + ".c";
      c = open (filename, "w");
      c.write ('/* Generated file, DO NOT EDIT! */\n');
      c.close ();
