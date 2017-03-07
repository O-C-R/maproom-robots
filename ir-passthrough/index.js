const osc = require('osc');
const express = require('express');
const app = express();

app.use(express.static(__dirname + '/public'));

const server = require('http').createServer(app);
const io = require('socket.io')(server);
io.on('connection', function(conn) {
  console.log('Got a connection.');
});
server.listen(8080);

var udpPort = new osc.UDPPort({
  localAddress: "0.0.0.0",
  localPort: 5201
});

// Listen for incoming OSC bundles.
udpPort.on("message", function (oscBundle) {
  if (oscBundle.address == '/ir') {
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
