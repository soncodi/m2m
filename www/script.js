/* globals Highcharts, io */

$(function() {
  Highcharts.setOptions({
    global: {
      useUTC: false
    }
  });

  var dataSeries;

  $('#container').highcharts({
    chart: {
      type: 'spline',
      animation: Highcharts.svg,
      marginRight: 10,
      events: {
        load: function() {
          dataSeries = this.series[0];
        }
      }
    },
    title: {
      text: 'Temperature'
    },
    xAxis: {
      type: 'datetime',
      tickPixelInterval: 150
    },
    yAxis: {
      title: {
        text: 'Value'
      },
      plotLines: [{
        value: 0,
        width: 1,
        color: '#808080'
      }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    legend: {
      enabled: false
    },
    exporting: {
      enabled: false
    },
    series: [{
      name: 'Random data',
      data: (function() {
        var data = [];
        var time = (new Date()).getTime();

        for (var i = -19; i <= 0; i++) {
          data.push({
            x: time + i * 1000,
            y: 0
          });
        }

        return data;
      })()
    }]
  });

  var socket = io.connect();

  socket.emit('ping', {
    ping: new Date()
  });

  socket.on('pong', function(data) {
    console.log(data);
  });

  socket.on('push', function() {
    var x = (new Date()).getTime();
    var y = Math.random();

    dataSeries.addPoint([x, y], true, true);
  });

  // function onDeviceReady() {
  //   console.log('ready');
  // }

  // document.addEventListener('deviceready', onDeviceReady, false);

});
