var socket;

$(document).ready(function() {
  setupSocket();

  $('.command-button').click(function() {
    var command = $(this).attr('id').replace(/Button$/, '');
    socket.send(command);
  });
});

function setupSocket(){
  // setup websocket
  // get_appropriate_ws_url is a nifty function by the libwebsockets people
  // it decides what the websocket url is based on the broswer url
  // e.g. https://mygreathost:9099 = wss://mygreathost:9099

  if (BrowserDetect.browser == "Firefox") {
    socket = new MozWebSocket(get_appropriate_ws_url());
  } else {
    socket = new WebSocket(get_appropriate_ws_url());
  }

  try {
    socket.onopen = function() {
      $('#status').css('background-color', "#40ff40")
        .text('websocket connection opened');
    };

    // received message
    socket.onmessage = function got_packet(msg) {
      $('#messages').text(msg.data);
    };

    socket.onclose = function(){
      $('#status').css('background-color', '#ff4040')
        .text('websocket connection CLOSED');
    };
  } catch(exception) {
    alert('Error: ' + exception);
  }
}
