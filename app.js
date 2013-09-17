var express = require('express');
var http = require('http');
var querystring = require('querystring');
var app = express();
var server = require('http').createServer(app);
var io = require('socket.io').listen(server);
var port = 8080;

var root = 'http://api.m2m.io/2';
var user = 'asoncodi@gmail.com';
var pass = '';

var Client = require('node-rest-client').Client;

var client = new Client({
  user: user,
  password: pass
});

function doPost(content) {
  var options = {
    host: 'att-api.m2m.io',
    port: 80,
    path: '/2/account/domain/1ce215c3f9414890642cbc67595780a7/stuff/arduino/thing/device02/publish' +
      '?topic=1ce215c3f9414890642cbc67595780a7%2Farduino%2Fdevice02%2Fcmd',
    method: 'POST',
    headers: {
      'Authorization': 'Basic YXNvbmNvZGlAZ21haWwuY29tOmVjb3NlbnNl',
      'Content-Type': 'application/x-www-form-urlencoded',
      'Content-Length': Buffer.byteLength(content)
    }
  };

  var req = http.request(options, function(res) {
    res.setEncoding('utf8');
    res.on('data', function (chunk) {
      console.log("body: " + chunk);
    });
  });

  req.write(content);
  req.end();
}

var temp = [];
var moisture = [];
var light = [];

var fanStatus = 0;
var lightStatus = 0;

setInterval(function() {
  client.get(root + '/account/domain/1ce215c3f9414890642cbc67595780a7' +
      '/stuff/arduino/thing/device02/present', function(data, response) {
    var attrs = data.attributes;

    if (!attrs) {
      return;
    }

    fanStatus = (attrs.fs === '1');
    lightStatus = (attrs.ls === '1');

    var t = parseFloat(attrs.t) || 0;
    var m = parseFloat(attrs.m) || 0;
    var l = parseFloat(attrs.l) || 0;

    if (temp.length > 9) {
      temp.shift();
      moisture.shift();
      light.shift();
    }

    temp.push(t);
    moisture.push(m);
    light.push(l);
  });
}, 1000);

app.use(express.static(__dirname + '/www'));

app.use(function(req, res, next) {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE');
  res.header('Access-Control-Allow-Headers', 'Content-Type, Authorization, X-Requested-With');
  res.header('Access-Control-Allow-Credentials', 'true');

  // for CORS preflight
  if (req.method === 'OPTIONS') {
    res.send(200);
  }
  else {
    next();
  }
});

app.use(express.bodyParser());

app.post('/api/fan', function (req, res, next) {
  console.log('posting fan', req.body.fanon);

  doPost(querystring.stringify({
    f: req.body.fanon
  }));

  return res.json({
    ok: 'ok'
  });
});

app.post('/api/light', function (req, res, next) {
  console.log('posting light', req.body.lighton);

  doPost(querystring.stringify({
    l: req.body.lighton
  }));

  return res.json({
    ok: 'ok'
  });
});

app.get('*', function(req, res) {
  return res.sendfile(__dirname + '/www/index.html');
});

io.sockets.on('connection', function(socket) {
  socket.on('ping', function(data) {
    socket.emit('pong', {
      pong: new Date()
    });
  });

  var pushInterval = setInterval(function() {
    socket.emit('push', {
      temp: temp,
      moisture: moisture,
      light: light,
      fanStatus: fanStatus,
      lightStatus: lightStatus
    });
  }, 2000);

  socket.on('disconnect', function() {
    console.log('client disconnected');

    if (pushInterval) {
      clearInterval(pushInterval);
    }
  });
});

server.listen(port, function() {
  console.log('up on %d', port);
});
