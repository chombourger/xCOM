import os;
from tests.python.basic.in_b import Test;

def test_register (componentHandle, importHandle):
   port = Test.Test (componentHandle, importHandle);
   return port;   

