/* globals Highcharts, io */

$(function() {
  Highcharts.setOptions({
    global: {
      useUTC: false
    }
  });

  var temp;
  var moisture;
  var light;

  $('#temp').highcharts({
    chart: {
      type: 'spline',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { temp = this.series[0]; } }
    },
    title: { text: 'Temperature'},
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Temperature',
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

  $('#moisture').highcharts({
    chart: {
      type: 'spline',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { moisture = this.series[0]; } }
    },
    title: { text: 'Moisture'},
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Moisture',
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

  $('#light').highcharts({
    chart: {
      type: 'spline',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { light = this.series[0]; } }
    },
    title: { text: 'Light'},
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Light',
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

  // var socket = io.connect();
  var socket = io.connect('http://eco-sense.us:8080');

  socket.emit('ping', {
    ping: new Date()
  });

  socket.on('pong', function(data) {
    console.log(data);
  });

  socket.on('push', function(data) {
    var x = (new Date()).getTime();

    temp.addPoint([x, parseFloat(data.temp)], true, true);
    moisture.addPoint([x, parseFloat(data.moisture)], true, true);
    light.addPoint([x, parseFloat(data.light)], true, true);
  });
});
