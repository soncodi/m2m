var express = require('express');
var app = express();
var port = 8080;

app.use(express.static(__dirname + '/static'));

app.get('/api', function(req, res, next) {
  return res.json({
    up: new Date()
  });
});

app.listen(port, function() {
  console.log('up on %d', port);
});
