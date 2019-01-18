function DioCallback(o)
{
  var pin = o.dio_p;
  $('txtdio_s'+pin).value = o.dio_s;
  $('txtdio_d'+pin).value = o.dio_d;
}
function led1Callback(o)
{
  $('led1_txt').innerHTML = o.led1_txt;
}
function getDio(o)
{
  var p = o.attributes['pin'].value;
  var oUpdate;
  oUpdate = new AJAX('get_dio'+p+'.cgi',
  function(t)
    {
      try
      {
        eval(t);
      }
      catch(e)
      {
        alert(e);
      }
    }
  );
  oUpdate.doGet();
}
function setDiostate(o)
{
  var p = o.attributes['pin'].value;
  /*var v=$('txtdio_s'+p).value;*/
  var v = o.attributes['s'].value;
  dout = new AJAX('set_diostate.cgi',
  function(t)
    {
      try
      {
        /*eval(t);*/
        document.getElementById('led1_txt').innerHTML = t;
      }
      catch(e)
      {
        alert(e);
      }
    }
  );
  dout.doPost('pin='+p+'&val='+v);
}
function setDiodir(o)
{
  var p = o.attributes['pin'].value;
  /*var v=$('txtdio_d'+p).value;*/
  var v = o.attributes['d'].value;
  dout = new AJAX('set_diodir.cgi',
  function(t)
    {
      try
      {
        eval(t);
      }
      catch(e)
      {
        alert(e);
      }
    }
  );
  dout.doPost('pin='+p+'&val='+v);
}
function getled1()
{
  var oUpdate;
  setTimeout(function()
    {
      oUpdate = new AJAX('get_led1.cgi',
      function(t)
        {
          try
          {
            eval(t);
          }
          catch(e)
          {
            alert(e);
          }
        }
      );
      oUpdate.doGet();
    }
    , 300);
    setTimeout('getled1()', 3000);
}
