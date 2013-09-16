/* globals Highcharts, io */

$(function() {
  function arrsum(a, b) {
    return a + b;
  }

  Highcharts.setOptions({
    global: {
      useUTC: false
    }
  });

  var temp;
  var moisture;
  var light;

  $('#temp').highcharts({
    credits: {
      enabled: false
    },
    chart: {
      type: 'area',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { temp = this.series[0]; } }
    },
    title: { text: null },
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2) + '°';
      }
    },
    plotOptions: {
      area: {
        fillColor: {
          linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1},
          stops: [
            [0, Highcharts.Color(Highcharts.getOptions().colors[3]).setOpacity(0.6).get('rgba')],
            [1, Highcharts.Color(Highcharts.getOptions().colors[0]).setOpacity(0.2).get('rgba')]
          ]
        },
        lineWidth: 1,
        marker: { enabled: false },
        shadow: false,
        states: { hover: { lineWidth: 1 } },
        threshold: null
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Temperature',
      data: (function() {
        var data = [];
        var time = (new Date()).getTime();

        for (var i = -9; i <= 0; i++) {
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
    credits: {
      enabled: false
    },
    chart: {
      type: 'area',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { moisture = this.series[0]; } }
    },
    title: { text: null },
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    plotOptions: {
      area: {
        fillColor: {
          linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1},
          stops: [
            [0, Highcharts.Color(Highcharts.getOptions().colors[0]).setOpacity(0).get('rgba')],
            [1, Highcharts.getOptions().colors[0]]
          ]
        },
        lineWidth: 1,
        marker: { enabled: false },
        shadow: false,
        states: { hover: { lineWidth: 1 } },
        threshold: null
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Moisture',
      data: (function() {
        var data = [];
        var time = (new Date()).getTime();

        for (var i = -9; i <= 0; i++) {
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
    credits: {
      enabled: false
    },
    chart: {
      type: 'spline',
      animation: Highcharts.svg,
      marginRight: 10,
      events: { load: function() { light = this.series[0]; } }
    },
    title: { text: null },
    xAxis: { type: 'datetime', tickPixelInterval: 150 },
    yAxis: {
      title: { text: 'Value' },
      plotLines: [{ value: 0, width: 1, color: '#808080' }]
    },
    tooltip: {
      formatter: function() {
        return '<b>'+ this.series.name +'</b><br/>'+
          Highcharts.dateFormat('%H:%M:%S', this.x) +'<br/>'+
          Highcharts.numberFormat(this.y, 2);
      }
    },
    plotOptions: {
      spline: {
        lineWidth: 4,
        states: { hover: { lineWidth: 5 } },
        marker: { enabled: false }
      }
    },
    legend: { enabled: false },
    exporting: { enabled: false },
    series: [{
      name: 'Light',
      data: (function() {
        var data = [];
        var time = (new Date()).getTime();

        for (var i = -9; i <= 0; i++) {
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

    var t = data.temp;
    var m = data.moisture;
    var l = data.light;

    temp.addPoint([x, t[t.length - 1]], true, true);
    moisture.addPoint([x, m[m.length - 1]], true, true);
    light.addPoint([x, l[l.length - 1]], true, true);

    var tsum = t.reduce(arrsum);
    var tavg = tsum / t.length;
    var tmax = Math.max.apply(null, t);
    var tmin = Math.min.apply(null, t);

    var msum = m.reduce(arrsum);
    var mavg = msum / m.length;
    var mmax = Math.max.apply(null, m);
    var mmin = Math.min.apply(null, m);

    var lsum = l.reduce(arrsum);
    var lavg = lsum / l.length;
    var lmax = Math.max.apply(null, l);
    var lmin = Math.min.apply(null, l);

    $('.temp .avg .val').text(tavg.toFixed(0));
    $('.temp .max .val').text(tmax.toFixed(0));
    $('.temp .min .val').text(tmin.toFixed(0));

    $('.moisture .avg .val').text(mavg.toFixed(0));
    $('.moisture .max .val').text(mmax.toFixed(0));
    $('.moisture .min .val').text(mmin.toFixed(0));

    $('.light .avg .val').text(lavg.toFixed(0));
    $('.light .max .val').text(lmax.toFixed(0));
    $('.light .min .val').text(lmin.toFixed(0));
  });
});
