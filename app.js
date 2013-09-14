var express = require('express');
var app = express();
var port = 8080;

app.get('/', function(req, res, next) {
  return res.json({
    up: new Date()
  });
});

app.listen(port, function() {
  console.log('up on %d', port);
});
