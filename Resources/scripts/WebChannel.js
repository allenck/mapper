//alert("Loading WebChannel.js");

var webViewBridge = null;
var channel = null;
var wsUri = "ws://localhost:12345";
console.log("Loading WebChannel.js");

function onLoad() {
var baseUrl
if (location.search !== "")
  baseUrl = (/[?&]webChannelBaseUrl=([A-Za-z0-9\-:/\.]+)/.exec(location.search)[1]);
else
  baseUrl = "ws://localhost:12345";

console.log("Connecting to WebSocket server at " + baseUrl + ".");
//alert("Connecting to WebSocket server at " + baseUrl + ".");
var socket = new WebSocket(baseUrl);

socket.onclose = function()
{
 console.error("web channel closed");
    alert("connection to application has been closed. ");
    closeWindow();
};
socket.onerror = function(error)
{
 console.error("web channel error: " + error);
};
socket.onopen = function()
{
 //output("WebSocket connected, setting up QWebChannel.");
 console.log("WebSocket connected, setting up QWebChannel.");
 channel = new QWebChannel(socket, function(channel)  {
  console.log("enter QWebChannel" + channel.objects);
  webViewBridge = channel.objects.webViewBridge;
     if(webViewBridge == null)
     {
      console.error("webViewBridge is NULL!")
     }

  console.log("connect to signals", webViewBridge);
  //connect to a signal
  webViewBridge.executeScript.connect(function(func, parms) {
   processScript(func, parms);
  });

  //connect to a signal
   webViewBridge.executeScript2.connect(function(func, parms, name, value) {
    processScript2(func, parms, name, value);
   } );

   //connect to a signal
   webViewBridge.executeScript3.connect(function(func, objArray, count) {
    processScript3(func, objArray, count);
   });
   window.connected = true;
   window.initialize();

  });
 }
}
