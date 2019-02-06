function AJAX(a, e)
{
  var c = d();
  c.onreadystatechange = b;
  function d()
  {
    if(window.XMLHttpRequest)
    {
      return new XMLHttpRequest()
    }
    else
    {
      if(window.ActiveXObject)
      {
        return new ActiveXObject("Microsoft.XMLHTTP")
      }
    }
  }
  function b()
  {
    if(c.readyState==4)
    {
      if(c.status==200)
      {
        if(e)
        {
          e(c.responseText)
        }
      }
    }
  }
  this.doGet = function()
  {
    c.open("GET", a, true); c.send(null)
  };
  this.doPost = function(f)
  {
    c.open("POST", a, true);
    c.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    c.setRequestHeader("ISAJAX", "yes");
    c.send(f)
  }
}
function $(a)
{
  return document.getElementById(a)
}
function $$(a)
{
  return document.getElementsByName(a)
}
function $$_ie(a, c)
{
  if(!a)
  {
    a = "*"
  }
  var b = document.getElementsByTagName(a);
  var e = []; for(var d = 0; d<b.length; d++)
  {
    att = b[d].getAttribute("name");
    if(att==c)
    {
      e.push(b[d])
    }
  }return e
}

