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

class Argument:
   DIRECTION_IN  = 'in';
   DIRECTION_OUT = 'out';

   def __init__ (self, parent):
      self.m_name = '';
      self.m_type = '';
      self.m_direction = DIRECTION_IN;
      self.setParent (parent);

   def name (self):
      return self.m_name;

   def setName (self, s):
      self.m_name = s;

   def type (self):
      return self.m_type;

   def setType (self, t):
      self.m_type = t;

   def direction (self):
      return self.m_direction;

   def setDirection (self, d):
      self.m_direction = d;

   def parent (self):
      return self.m_parent;

   def setParent (self, p):
      self.m_parent = p;
      p.addArgument (self);
