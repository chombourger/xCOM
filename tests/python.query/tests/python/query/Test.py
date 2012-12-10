import pycom;

class Test:
   def __init__ (self, chandle, ihandle):
      self.m_chandle = chandle;
      self.m_ihandle = ihandle;

   def Run (self, result, error, user_data):
      q = pycom.Query (self.m_chandle, "Tests", 0);
      if q is not None:
         q.Finish();
         if q.matches > 0:
            if result is not None:
               result (self.m_ihandle, 0, user_data);
               return 0;
      if error is not None:
         error (self.m_ihandle, 1, user_data);
         return 0;

