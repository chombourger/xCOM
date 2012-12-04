
class Test:
   def __init__ (self, chandle, ihandle):
      self.m_chandle = chandle;
      self.m_ihandle = ihandle;

   def check (self, data, result, error, user_data):
      if data == True:
         if result is not None:
            result (self.m_ihandle, 0, user_data);
            return 0;
      else:
         if error is not None:
            error (self.m_ihandle, 1, user_data);
            return 0;

