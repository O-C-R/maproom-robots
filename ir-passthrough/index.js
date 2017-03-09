const osc = require('osc');
const express = require('express');
const app = express();

app.use(function(req, res, next) {
  res.header('Access-Control-Allow-Origin', req.headers.origin);
  res.header('Access-Control-Allow-Credentials', 'true');
  res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE,HEAD,DELETE,OPTIONS');
  res.header('Access-Control-Allow-Headers', 'Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With');
  next();
});

app.use(express.static(__dirname + '/public'));

const server = require('http').createServer(app);
const io = require('socket.io')(server);
io.on('connection', function(socket) {
  const clientIP = socket.request.connection.remoteAddress;
  console.log('Got a connection from', clientIP);
});
server.listen(8080);

var udpPort = new osc.UDPPort({
  localAddress: "0.0.0.0",
  localPort: 5201
});

// Listen for incoming OSC bundles.
let nMessages = 0;
udpPort.on("message", function (oscBundle) {
  if (oscBundle.address == '/ir') {
    nMessages++;
    const data = JSON.parse(oscBundle.args[0]);
    io.sockets.emit('ir', data)
  }
});

// Open the socket.
udpPort.open();

// When the port is ready
udpPort.on("ready", function () {
    console.log('udp ready');
});

udpPort.on("error", function (error) {
    console.log("An error occurred: ", error.message);
});

function heartbeat() {
  console.log(new Date(), 'up with messages', nMessages);
  setTimeout(heartbeat, 3000);
}
heartbeat();
