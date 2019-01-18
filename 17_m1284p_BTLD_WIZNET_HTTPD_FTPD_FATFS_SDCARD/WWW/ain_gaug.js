google.load('visualization', '1',
  {
  packages:['gauge']
  }
); google.setOnLoadCallback(AinDrawGoogleGauge);

function AinCallback(o)
{
  var pin = o.ain_p;
  $('txtain_v'+pin).value = o.ain_v;
  AinDrawGoogleGauge(o);
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

function AinDrawGoogleGauge(o)
{
  var val = o.ain_v;
  //var temp_val = Number(((((val*3300)/1023)-500)/10).toFixed(2));
  var temp_val = Number(val); //here in range 0..1023
  if(isNaN(temp_val)) temp_val = 0;
  var data = google.visualization.arrayToDataTable([['Label', 'Value'],['ADC6', 80]]);
  var options =
  {
    width: 400,
    height: 120,
    max: 1023,
    min: 0,
    greenFrom: 0,
    greenTo: 512,
    redFrom: 918,
    redTo: 1023,
    yellowFrom: 714,
    yellowTo: 918,
    majorTicks: ['0', '512', '1023'],
    minorTicks: 5
  };
  var chart = new google.visualization.Gauge(document.getElementById('chart_div'));
  data.setValue(0, 1, temp_val);
  chart.draw(data, options);
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
    , 300);
    setTimeout('getAin6_update()', 500);
}
