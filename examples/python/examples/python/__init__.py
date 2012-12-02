import os;
from examples.python import App;

class Import:
   def __init__ (self, chandle, ihandle):
      self.chandle = chandle;
      self.ihandle = ihandle;


ports = {};
hello = None;

def init (componentHandle):
   print 'python> init(' + str(componentHandle) + ')';
   return 0;

def destroy (componentHandle):
   print 'python> destroy (' + str(componentHandle) + ')';
   return 0;

def app_register (componentHandle, importHandle):
   print 'python> app register (' + str(componentHandle) + ',' + str(importHandle) + ')';
   print 'python> hello ' + str (hello);
   port = App.App (componentHandle, importHandle, hello);
   ports[importHandle] = port;
   return port;   

def app_unregister (componentHandle, importHandle):
   print 'python> app unregister (' + str(componentHandle) + ',' + str(importHandle) + ')';
   if ports.has_key(importHandle):
      del ports[importHandle];

def hello_register (componentHandle, importHandle):
   print 'python> hello register (' + str(componentHandle) + ',' + str(importHandle) + ')';
   global hello, ports;
   port = Import (componentHandle, importHandle);
   ports[importHandle] = port;
   hello = port;
   print 'python> exit hello_register';
   return port;   

def hello_unregister (componentHandle, importHandle):
   print 'python> hello unregister (' + str(componentHandle) + ',' + str(importHandle) + ')';
   if ports.has_key(importHandle):
      del ports[importHandle];

