function NetinfoCallback(o)
{
  $('txtmac').value = o.mac;
  $('txtip').value = o.ip;
  $('txtgw').value = o.gw;
  $('txtsn').value = o.sn;
  $('txtdns').value = o.dns;
  if(typeof(window.external)!='undefined')
  {
    obj = $$_ie('input', 'dhcp');
  }
  else
  {
    obj = $$('dhcp');
  }
}
function getNetinfo()
{
  var oUpdate;
  setTimeout(function()
    {
      oUpdate = new AJAX('get_netinfo.cgi', function(t)
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
    , 1500);
}
