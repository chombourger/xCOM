import os;
from tests.python.basic.in_y import Test;

def test_register (componentHandle, importHandle):
   port = Test.Test (componentHandle, importHandle);
   return port;   

