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
import xml.parsers.expat;
from codegen import component;
from codegen import port;
from codegen import interface;
from codegen import method;
from codegen import argument;

class XmlParser:
   STATE_TOPLEVEL  = 'toplevel';
   STATE_COMPONENT = 'component';
   STATE_PORT      = 'port';
   STATE_INTERFACE = 'interface';
   STATE_METHOD    = 'method';
   STATE_ARGUMENT  = 'arg';
   STATE_DISCARD   = 'discard';

   def __init__ (self, file, xmlData):
      self.m_file = file;
      self.m_parser = xml.parsers.expat.ParserCreate ();
      self.m_parser.StartElementHandler = self.handleStartElement;
      self.m_parser.EndElementHandler = self.handleEndElement;
      self.m_components = [];
      self.m_interfaces = [];
      self.m_stack = [];
      self.m_stateStack = [];
      self.pushState (XmlParser.STATE_TOPLEVEL);
      self.m_parser.Parse (xmlData);

   def components (self):
      return self.m_components;

   def interfaces (self):
      return self.m_interfaces;

   def warn (self, w):
      print self.m_file + ':' + str(self.m_parser.CurrentLineNumber) + ': warning: ' + w;

   def pushState (self, s):
      self.m_stateStack.append (s);

   def popState (self):
      s = self.m_stateStack.pop ();

   def state (self):
      depth = len (self.m_stateStack);
      s = self.m_stateStack[len (self.m_stateStack) - 1];
      return s;

   def handleStartElement (self, name, attrs):
      state = self.state ();
      handled = False;
      if state == XmlParser.STATE_TOPLEVEL:
         if name == XmlParser.STATE_INTERFACE:
            handled = self.handleStartInterface (name, attrs);
         elif name == XmlParser.STATE_COMPONENT:
            handled = self.handleStartComponent (name, attrs);
         else:
            self.warn ('unexpected <' + name + '> tag!');
      elif state == XmlParser.STATE_COMPONENT:
         if name == XmlParser.STATE_PORT:
             handled = self.handleStartPort (name, attrs);
      elif state == XmlParser.STATE_INTERFACE:
         if name == XmlParser.STATE_METHOD:
            handled = self.handleStartMethod (name, attrs);
      elif state == XmlParser.STATE_METHOD:
         if name == XmlParser.STATE_ARGUMENT:
            handled = self.handleStartArgument (name, attrs);

      if handled == False:
         self.pushState (XmlParser.STATE_DISCARD);

   def handleEndElement (self, name):
      state = self.state ();
      if state == XmlParser.STATE_COMPONENT:
         if name == XmlParser.STATE_COMPONENT:
            self.handleEndComponent ();
      elif state == XmlParser.STATE_PORT:
         if name == XmlParser.STATE_PORT:
            self.handleEndPort ();
      elif state == XmlParser.STATE_INTERFACE:
         if name == XmlParser.STATE_INTERFACE:
            self.handleEndInterface ();
      elif state == XmlParser.STATE_METHOD:
         if name == XmlParser.STATE_METHOD:
            self.handleEndMethod ();
      elif state == XmlParser.STATE_ARGUMENT:
         if name == XmlParser.STATE_ARGUMENT:
            self.handleEndArgument ();
      elif state == XmlParser.STATE_DISCARD:
         self.popState ();

   def handleStartComponent (self, name, attrs):
      comp = component.Component ();
      if attrs.has_key ('name'):
         comp.setName (attrs['name']);
      if attrs.has_key ('version-major'):
         comp.setVersionMajor (attrs['version-major']);
      if attrs.has_key ('version-minor'):
         comp.setVersionMinor (attrs['version-minor']);
      self.m_stack.append (comp);
      self.pushState (XmlParser.STATE_COMPONENT);
      return True;

   def handleStartPort (self, name, attrs):
      parent = self.m_stack[len(self.m_stack) - 1];
      prt = port.Port (parent);
      if attrs.has_key ('name'):
         prt.setName (attrs['name']);
      if attrs.has_key ('interface'):
         prt.setInterface (attrs['interface']);
      if attrs.has_key ('component'):
         prt.setComponent (attrs['component']);
      if attrs.has_key ('port'):
         prt.setComponent (attrs['port']);
      if attrs.has_key ('version-major'):
         prt.setVersionMajor (attrs['version-major']);
      if attrs.has_key ('version-minor'):
         prt.setVersionMinor (attrs['version-minor']);
      if attrs.has_key ('provided'):
         provided = attrs['provided'];
         if provided == "true":
            prt.setProvided (True);
         elif provided == "false":
            prt.setProvided (False);
         else:
            self.warn ("'" + provided + "' is an invalid value for provided attribute (true/false expected)");
      if attrs.has_key ('required'):
         required = attrs['required'];
         if required == "true":
            prt.setProvided (True);
         elif required == "false":
            prt.setProvided (False);
         else:
            self.warn ("'" + required + "' is an invalid value for required attribute (true/false expected)");
      if attrs.has_key ('multiple'):
         multiple = attrs['multiple'];
         if multiple == "true":
            prt.setMultiple (True);
         elif multiple == "false":
            prt.setMultiple (False);
         else:
            self.warn ("'" + multiple + "' is an invalid value for multiple attribute (true/false expected)");
      if attrs.has_key ('runtime'):
         runtime = attrs['runtime'];
         if runtime == "true":
            prt.setRuntime (True);
         elif runtime== "false":
            prt.setRuntime (False);
         else:
            self.warn ("'" + runtime + "' is an invalid value for runtime attribute (true/false expected)");
      self.m_stack.append (prt);
      self.pushState (XmlParser.STATE_PORT);
      return True;

   def handleStartInterface (self, name, attrs):
      intf = interface.Interface ();
      if attrs.has_key ('name'):
         intf.setName (attrs['name']);
      if attrs.has_key ('version-major'):
         intf.setVersionMajor (attrs['version-major']);
      if attrs.has_key ('version-minor'):
         intf.setVersionMinor (attrs['version-minor']);
      self.m_stack.append (intf);
      self.pushState (XmlParser.STATE_INTERFACE);
      return True;

   def handleStartMethod (self, name, attrs):
      parent = self.m_stack[len(self.m_stack) - 1];
      mthd = method.Method (parent);
      if attrs.has_key ('name'):
         mthd.setName (attrs['name']);
      self.m_stack.append (mthd);
      self.pushState (XmlParser.STATE_METHOD);
      return True;

   def handleStartArgument (self, name, attrs):
      parent = self.m_stack[len(self.m_stack) - 1];
      arg = argument.Argument (parent);
      if attrs.has_key ('name'):
         arg.setName (attrs['name']);
      if attrs.has_key ('type'):
         arg.setType (attrs['type']);
      if attrs.has_key ('direction'):
         arg.setDirection (attrs['direction']);
      self.m_stack.append (arg);
      self.pushState (XmlParser.STATE_ARGUMENT);
      return True;

   def handleEndComponent (self):
      comp = self.m_stack.pop ();
      self.m_components.append (comp);
      self.popState ();

   def handleEndPort (self):
      prt = self.m_stack.pop ();
      self.popState ();

   def handleEndInterface (self):
      intf = self.m_stack.pop ();
      self.m_interfaces.append (intf);
      self.popState ();

   def handleEndMethod (self):
      mthd = self.m_stack.pop ();
      self.popState ();

   def handleEndArgument (self):
      arg = self.m_stack.pop ();
      self.popState ();
 
def parse (file, xmlData):
   parser = XmlParser (file, xmlData);
   return parser;

