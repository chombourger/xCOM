
def global_say_error (importHandle, result, userData):
   print 'python> global error function for IHello.Say() called';
   userData.m_error (importHandle, result, userData.m_user);

def global_say_result (importHandle, result, response, userData):
   print 'python> global result function for IHello.Say() called';
   print 'python> respone: ' + response;
   userData.m_result (importHandle, result, userData.m_user);

class App:
   def __init__ (self, chandle, ihandle, hello):
      print 'python> App constructor';
      self.m_chandle = chandle;
      self.m_ihandle = ihandle;
      self.i_hello   = hello;
      self.m_error   = None;
      self.m_result  = None;
      self.m_user    = None;

   def __del__ (self):
      print 'python> App destructor';

   def Start (self, result, error, user_data):
      print 'python> hello world xcom.IApplication in Python!';
      self.m_result = result;
      self.m_error  = error;
      self.m_user   = user_data;
      if self.i_hello is not None:
         self.i_hello.Say ('hi there', global_say_result, global_say_error, self);
      return 0;

