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
      self.m_descr = '';
      self.m_ports = [];
      self.m_init = True;
      self.m_destroy = True;

   def addPort (self, p):
      self.m_ports.append (p);

   def ports (self):
      return self.m_ports;

   def name (self):
      return self.m_name;

   def description (self):
      return self.m_descr;
   
   def versionMajor (self):
      return self.m_vmajor;

   def versionMinor (self):
      return self.m_vminor;

   def setName (self, s):
      self.m_name = s;

   def setDescription (self, d):
      self.m_descr = d;
      
   def setVersionMajor (self, v):
      self.m_vmajor = int(v);

   def setVersionMinor (self, v):
      self.m_vminor = int(v);

   def setInit (self, v):
      self.m_init = v;
      
   def init (self):
      return self.m_init;
   
   def setDestroy (self, v):
      self.m_destroy = v;
      
   def destroy (self):
      return self.m_destroy;
   