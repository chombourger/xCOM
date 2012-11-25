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

class Method:
   def __init__ (self, parent):
      self.m_name = '';
      self.m_args = [];
      self.setParent (parent);

   def addArgument (self, a):
      self.m_args.append (a);

   def arguments (self):
      return self.m_args;

   def name (self):
      return self.m_name;

   def setName (self, s):
      self.m_name = s;

   def parent (self):
      return self.m_parent;

   def setParent (self, p):
      self.m_parent = p;
      p.addMethod (self);

