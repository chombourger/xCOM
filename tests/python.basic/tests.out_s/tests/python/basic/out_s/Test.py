
class Test:
   def __init__ (self, chandle, ihandle):
      self.m_chandle = chandle;
      self.m_ihandle = ihandle;

   def check (self, result, error, user_data):
      if result is not None:
         result (self.m_ihandle, 0, "test out string argument", user_data);
         return 0;
      if error is not None:
         error (self.m_ihandle, 1, user_data);
         return 0;

