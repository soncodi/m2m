var express = require('express');
var app = express();
var server = require('http').createServer(app);
var io = require('socket.io').listen(server);
var port = 8080;

app.use(express.static(__dirname + '/static'));

app.get('/api', function(req, res, next) {
  return res.json({
    up: new Date()
  });
});

app.get('*', function(req, res) {
  return res.sendfile(__dirname + '/static/index.html');
});

io.sockets.on('connection', function(socket) {
  socket.on('ping', function(data) {
    console.log(data);

    socket.emit('pong', {
      pong: new Date()
    });
  });

  setInterval(function push() {
    socket.emit('push', {
      push: new Date()
    });
  }, 1000);
});

server.listen(port, function() {
  console.log('up on %d', port);
});
