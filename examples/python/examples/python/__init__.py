import os;
from examples.python import App;

ports = {};

def init (componentHandle):
   print 'python> init(' + str(componentHandle) + ')';
   return 0;

def destroy (componentHandle):
   print 'python> destroy (' + str(componentHandle) + ')';
   return 0;

def app_register (componentHandle, importHandle):
   print 'python> register (' + str(componentHandle) + ',' + str(importHandle) + ')';
   port = App.App (componentHandle, importHandle);
   ports[importHandle] = port;
   return port;   

def app_unregister (componentHandle, importHandle):
   print 'python> unregister (' + str(componentHandle) + ',' + str(importHandle) + ')';
   if ports.has_key(importHandle):
      del ports[importHandle];

