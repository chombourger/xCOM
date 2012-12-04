import os;
from tests.python.basic.out_s import Test;

def test_register (componentHandle, importHandle):
   port = Test.Test (componentHandle, importHandle);
   return port;   

