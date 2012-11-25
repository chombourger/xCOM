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

class Component: 
   def __init__ (self):
      self.m_name = '';
      self.m_vmajor = 0;
      self.m_vminor = 0;
      self.m_ports = [];

   def addPort (self, p):
      self.m_ports.append (p);

   def ports (self):
      return self.m_ports;

   def name (self):
      return self.m_name;

   def versionMajor (self):
      return self.m_vmajor;

   def versionMinor (self):
      return self.m_vminor;

   def setName (self, s):
      self.m_name = s;

   def setVersionMajor (self, v):
      self.m_vmajor = v;

   def setVersionMinor (self, v):
      self.m_vminor = v;

