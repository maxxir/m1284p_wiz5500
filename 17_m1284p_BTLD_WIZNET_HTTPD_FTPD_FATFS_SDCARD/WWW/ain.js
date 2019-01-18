function AinCallback(o)
{
  var pin = o.ain_p;
  $('txtain_v'+pin).value = o.ain_v;
  AinDrawgraph(o);
}
function getAin(o)
{
  var p = o.attributes['pin'].value;
  var oUpdate;
  setTimeout(function()
    {
      oUpdate = new AJAX('get_ain'+p+'.cgi',
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
}
function AinDrawgraph(o)
{
  var pin = o.ain_p;
  var val = o.ain_v;
  $('ain_v'+pin).style.width = val*500/1023+'px';
}
function getAin6_update()
{
  var oUpdate;
  setTimeout(function()
    {
      oUpdate = new AJAX('get_ain6.cgi',
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
    , 300); setTimeout('getAin6_update()', 500);
}
