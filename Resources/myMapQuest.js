

// deprecated
function processScript(func, parms)
{
  //alert("func: " + func + " parms: " +parms);
  var call = "var myRslt = " +func;
  call += "(";
  call += parms;
  call += ");";
  call += "return myRslt;";
  //alert(call);
  try
  {
   //eval(call);
   var myFucn =  Function(call);
   var fRslt = myFucn();
   if(fRslt === null) return;
   if( fRslt instanceof Array)
    webViewBridge.scriptArrayResult( fRslt);
   else
   {
    if("fRslt" in window)
     webViewBridge.scriptResult( fRslt);
    else
     console.trace("bad return: '" + call + fRslt + "'");
   }
  }
  catch (err)
  {
   txt=err;
   //alert("Error ocurred calling " + func + " '"+ parms + "'\n" + txt);
   console.error("Error ocurred calling " + func + " '"+ parms + "'\n" + txt);
   console.trace("trace");
  }
}

window.initialize = function() // called by WebChannel .ie "onLoad()"
{
    //initMap();
  L.mapquest.key = MapQuestKey;

  var Lat = webViewBridge.lat;
  var Lon = webViewBridge.lng;
  var zoom = webViewBridge.zoom;
  var mapTypeId = webViewBridge.maptype;
  var mapDiv = document.getElementById("map");
  var map = L.mapquest.map('map', {
    center: [Lat, Lon],
    layers: L.mapquest.tileLayer('map'),
    zoom: zoom
  });

  map.addControl(L.mapquest.control());
  onLoad();
  connectSlots();



  webViewBridge.debug("initMap started");
  connectSlots();
}

// deprecated
function processScript2(func, parms, name, value)
{
  alert("func: " + func + " parms: " +parms);
  //eval("comments = value;");
  window["comments"] = value;
  var call = "var myRslt = " +func;
  call += "(";
  call += parms;
  call += ");";
  call += "return myRslt;";
  //alert(call);
  try
  {
      //eval(call);
      var myFucn =  Function(call);
      var fRslt = myFucn();
      if(fRslt === null) return;
      if("fRslt" in window)
          webViewBridge.scriptResult( fRslt);
  }
  catch (err)
  {
      txt=err;
      alert("Error occured calling " + func + "\n" + txt);
  }
}

function processScript3(func, objArray, count)
{
  var i = 0;
  if(count <=0)
  {
      alert("invalid call to processScript3");
      return;
  }
  var parms = "var0";
  //eval("var0 = objArray[0];");
  window['var0'] = objArray[0];
  var call = "var myRslt = " +func;
  call += "(";
  for(i=1; i < count; i++)
  {
      //eval("var"+i+"=objArray[i];");
      window['var'+i] = objArray[i];
      parms += ",var"+i;
  }
  call += parms;
  call += ");";
  call += "return myRslt;";
  //alert(call);
  try
  {
      //eval(call);
      var myFucn =  Function(call);
      var fRslt = myFucn();
      if(fRslt === null) return;
      if( fRslt instanceof Array)
          webViewBridge.scriptArrayResult( fRslt);
      else
      {
          if("fRslt" in window)
              webViewBridge.scriptFunctionResult(func, fRslt);
      }
  }
  catch (err)
  {
      txt=err;
      alert("Error occured calling " + func + "\n" + call+"\n"+txt);
  }
}
var connected = false;
//We use this function because connect statements resolve their target once, immediately
//not at signal emission so they must be connected once the webViewBridge object has been added to the frame
//! <!--  [ connect slots ] -->
function connectSlots()
{
  if ( !connected ) {
  webViewBridge.executeScript.connect(this, processScript);
  webViewBridge.executeScript2.connect(this, processScript2);
  webViewBridge.executeScript3.connect(this, processScript3);
  connected = true;
  }
  return;
}

function setCenter(Lat, Lon)
{
 // map.setCenter(new google.maps.LatLng(Lat, Lon));
 // map.setOptions({disableDoubleClickZoom: true });
 return null;

}

function getCenter()
{
 var latLng = map.getCenter();
 webViewBridge.setCenter(latLng.lat(), latLng.lng(), map.getZoom(), map.getMapTypeId());
 return latLng;
}

function setZoom(zoom)
{
    map.setZoom(zoom);
    return null;
}
function setMapType(mapTypeId)
{
    if(mapTypeId === "")
        mapTypeId = "ROADMAP";
    map.setMapTypeId(mapTypeId);
    return null;
}
function getMapType()
{
    return map.getMapTypeId();
}
