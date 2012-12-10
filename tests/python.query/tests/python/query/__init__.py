import os;
from tests.python.query import Test;

def test_register (componentHandle, importHandle):
   port = Test.Test (componentHandle, importHandle);
   return port;   

