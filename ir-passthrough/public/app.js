var socket = io();

socket.on('ir', function(msg) {
  document.getElementById('contents').innerHTML = JSON.stringify(msg);
})

socket.on('disconnect', function() {
  console.log('you have been disconnected');
});

socket.on('reconnect', function() {
  console.log('you have been reconnected');
});

socket.on('reconnect_error', function() {
  console.log('attempt to reconnect has failed');
});
