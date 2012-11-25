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

import sys;

class Port:
   def __init__ (self, parent):
      self.m_name = '';
      self.m_interface = '';
      self.m_provided = True;
      self.m_multiple = False;
      self.m_runtime = False;
      self.m_component = '';
      self.m_port = '';
      self.setParent (parent);

   def name (self):
      return self.m_name;

   def setName (self, s):
      self.m_name = s;

   def interface (self):
      return self.m_interface;

   def setInterface (self, i):
      self.m_interface = i;

   def provided (self):
      return self.m_provided;

   def setProvided (self, p):
      self.m_provided = p;

   def multiple (self):
      return self.m_multiple;

   def setMultiple (self, m):
      self.m_multiple = m;

   def runtime (self):
      return self.m_runtime;

   def setRuntime (self, r):
      self.m_runtime = r;

   def component (self):
      return self.m_component;

   def setComponent (self, c):
      self.m_component = c;

   def port (self):
      return self.m_port;

   def setPort (self, p):
      self.m_port = p;

   def parent (self):
      return self.m_parent;

   def setParent (self, p):
      self.m_parent = p;
      p.addPort (self);

