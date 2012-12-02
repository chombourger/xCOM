
class App:
   def __init__ (self, chandle, ihandle):
      print 'python> App constructor';
      self.m_chandle = chandle;
      self.m_ihandle = ihandle;

   def __del__ (self):
      print 'python> App destructor';

   def Start (self, result, error, user_data):
      print 'python> hello world xcom.IApplication in Python!';
      if result is not None:
         result (self.m_ihandle, 0, user_data);

