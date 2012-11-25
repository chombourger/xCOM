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

def interface (interfaces, name, vmaj, vmin):
   result = None;
   for i in interfaces:
      if i.name() != name:
         continue;
      if vmaj is not None and vmaj != i.versionMajor ():
         continue;
      if vmin is not None and vmin != i.versionMinor ():
         continue;
      result = i;
      break;
   return result;
