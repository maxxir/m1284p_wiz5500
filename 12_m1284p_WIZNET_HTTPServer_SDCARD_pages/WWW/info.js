function getInfo()
{
 var oUpdate;
 setTimeout(function()
  {
   oUpdate = new AJAX('get_info.cgi', function(t)
    {
     try
     {
      //*eval(t);
      document.getElementById('info_txt').innerHTML = t;
     }
     catch(e)
     {
      alert(e);
     }
    }
   ); oUpdate.doGet();
  }
  , 300); setTimeout('getInfo()', 3000);
}
