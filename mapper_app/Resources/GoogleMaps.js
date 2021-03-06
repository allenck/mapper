//'use strict';
var googleEarth;
var line;
var map;
var segment;
var siArray;
var Arrow;
var color;
var defaultOptions;
var options;
var txt;
var newSegment, segmentId, arrow,lat,lon;
console.log("Loading GoogleMaps.js");
var image = ["http://maps.google.com/mapfiles/marker.png",
  "http://maps.google.com/mapfiles/dd-start.png",
  "http://maps.google.com/mapfiles/dd-end.png",
  "http://maps.google.com/mapfiles/shadow50.png",
  "http://www.google.com/mapfiles/arrow.png",
  "http://www.google.com/mapfiles/arrowshadow.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/greenblank.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/blueblank.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/redblank.png",
  "http://maps.google.com/mapfiles/shadow50.png",
  "http://acksoft.dyndns.biz/picturegallery/images/17.png",
  "http://acksoft.dyndns.biz/picturegallery/images/129.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/yellowblank.png",
  "http://hpstorage.acksoft.dyndns.biz/picturegallery/images/sbahn_small.png",
  "http://hpstorage.acksoft.dyndns.biz/picturegallery/images/ubahn_small.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/tram.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/tram.shadow.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/white.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/blue-red-blank.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/orange.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/BVGTram.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/subway.png",
  "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/subway.shadow.png",
];
var images = {"default":0, "start":1, "end":2, "shadow":3, "arrow":4, "arrowShadow":5,
  "smallGreen":6, "smallBlue":7,"smallRed":8,"smallShadow":9,"stop":10,"resume":11,"smallYellow":12,
  "sbahn":13, "ubahn":14, "tram":15, "tramshadow":16, "smallWhite":17, "blue-red":18, "orange":19, "bvgtram":20, "subway":21, "subwayshadow":22};
var marker = null;
var selectedLine = null;
var selectedLineClr = "#FF0000";
var selectedPoly = null;
var geocoder;
var currSegment = null;
var bAdding = false;

var rtStartMarker= null;
var rtEndMarker = null;
var infowindow = null;
var osm_MapType;
var stationArray = null;
var overlay=null;
var opacityControl = null;
var bGeocoderRequest = false;
var User_MapType = null;
console.log("GoogleMaps.js line 58!");

var connected = false;
//We use this function because connect statements resolve their target once, immediately
//not at signal emission so they must be connected once the webViewBridge object has been added to the frame
//! <!--  [ connect slots ] -->
function connectSlots()
{
  if ( !connected ) {
      //imageAnalyzer.finishedAnalysis.connect(this, finished);
      //imageAnalyzer.updateProgress.connect(this, updateProg);

  webViewBridge.executeScript.connect(this, processScript);
  webViewBridge.executeScript2.connect(this, processScript2);
  webViewBridge.executeScript3.connect(this, processScript3);
  connected = true;

  //alert("connectSlots " + connected);
      alert(google.maps.version);
  }
}
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
   var myFucn = new Function(call);
   var fRslt = myFucn();
      if(fRslt === null) return;
   if( fRslt instanceof Array)
    webViewBridge.scriptArrayResult( fRslt);
   else
   {
    if(webViewBridge.scriptResult !== null)
        webViewBridge.scriptResult( fRslt);
   }
  }
  catch (err)
  {
   txt=err;
   alert("Error occured calling " + func + " '"+ parms + "'\n" + txt);
  }
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
      var myFucn = new Function(call);
      var fRslt = myFucn();
      if(fRslt === null) return;
      if(webViewBridge.scriptResult !== null)
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
      var myFucn = new Function(call);
      var fRslt = myFucn();
      if(fRslt === null) return;
      if( fRslt instanceof Array)
          webViewBridge.scriptArrayResult( fRslt);
      else
      {
          if(fRslt === 0)
              alert(call);
          if(webViewBridge.scriptResult !== null)
              webViewBridge.scriptResult( fRslt);
      }
  }
  catch (err)
  {
      txt=err;
      alert("Error occured calling " + func + "\n" + call+"\n"+txt);
  }
}
//! <!--  [ connect slots ] -->

function Get_osm_MapType(tile, zoom)
{
 return "http://tile.openstreetmap.org/" +  zoom + "/" + tile.x + "/" + tile.y + ".png";
}
//alert("here1" + google);

osm_MapType = new google.maps.ImageMapType(
{
 getTileUrl: Get_osm_MapType ,
 tileSize: new google.maps.Size(256, 256),
 isPng: true,
 alt: "OpenStreetMap layer",
 name: "OpenStreetMap",
 maxZoom: 18
});
//alert("here2");

var userMap = "Berlin_Bauentwicklung.1940.10000.300.3068";
var userMapTitle = "Berlin 1940";

function Get_User_MapType(tile, zoom)
{
var ymax = 1 << zoom;
var y = ymax - tile.y -1;
return "http://ubuntu-2.acksoft.dyndns.biz:1080/public/map_tiles/" + userMap +"/"  +zoom+"/"+tile.x+"/"+tile.x+"_"+y+"_"+zoom+".png";
}

function setUserMap(map, title)
{
 userMap = map;
 userMapTitle = title;
 User_MapType = new google.maps.ImageMapType(
    {
     getTileUrl: Get_User_MapType ,
     tileSize: new google.maps.Size(256, 256),
     isPng: true,
     alt: userMap,
     name: userMapTitle,
     disableDoubleClickZoom: true,
     maxZoom: 19
    });
 return null;
}

//User_MapType = new google.maps.ImageMapType(
//{
// getTileUrl: Get_User_MapType ,
// tileSize: new google.maps.Size(256, 256),
// isPng: true,
// alt: userMap,
// name: userMapTitle,
// maxZoom: 19
//});

// Class to calculate distance and bearing
function bearing(startLat, startLon, endLat, endLon)
{
    //alert(startLat +" "+ startLon+" "+ endLat+" "+ endLon);
    this.type = "bearing";
    this.getInfo = function () {
        return "bearing " + brng + " " + d + "km";
    }
    var R = 6371; // RADIUS OF THE EARTH IN KM
    var dToRad = 0.0174532925;

    var lat1 = startLat * dToRad;
    var lon1 = startLon * dToRad;
    var lat2 = endLat * dToRad;
    var lon2 = endLon * dToRad;
    var dLat = dToRad * (endLat - startLat);
    var dLon = dToRad * (endLon - startLon);
    var y = Math.sin(dLon) * Math.cos(lat2);
    var x = Math.cos(lat1) * Math.sin(lat2) -
            Math.sin(lat1) * Math.cos(lat2) * Math.cos(dLon);
    var brng = Math.atan2(y, x) / dToRad;
    // save values for writing out later
    if (brng < 0)
        brng = 360.0 + brng;
    //direction = (int)((brng + 22.5) / 45.0) ;
    //if(direction >= 8)
    //    direction = 0;

    // calculate distance
    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2)
            + Math.cos(lat1) * Math.cos(lat2)
            * Math.sin(dLon / 2) * Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c;

    this.getBearing = function()
    {
        return brng;
    }
    this.getDistance = function()
    {
        return d;
    }
}
// class to calculate a point given the distance and bearing
function pointRadialDistance(start, bearing, inDistance)
{
var dToRad = 0.0174532925;
var rEarth = 6371.01;  // Earth's average radius in km
var epsilon = 0.000001;  // threshold for floating-point equality

var degrees = bearing;
while (degrees < -180) degrees += 360;
while (degrees > 180) degrees -= 360;

// convert the angle to radians
var lat1 = start.lat() * dToRad;
var lon1 = start.lng() * dToRad;
var rbrng = degrees * dToRad;
var rd = inDistance / rEarth;    // normalize linear distance to radian angle

var rLat = 0, rLon = 0;
// http://www.movable-type.co.uk/scripts/latlong.html
rLat = Math.asin(Math.sin(lat1) * Math.cos(rd) +
        Math.cos(lat1) * Math.sin(rd) * Math.cos(rbrng));
rLon = lon1 + Math.atan2(Math.sin(rbrng) * Math.sin(rd) * Math.cos(lat1),
                           Math.cos(rd) - Math.sin(lat1) * Math.sin(rLat));
//alert("degrees " + degrees + " rLat " + rLat + " rLon " + rLon);
return new google.maps.LatLng(rLat / dToRad, rLon / dToRad);
}

// class to create an arrow
function myArrow(lLat, lLon, mLat, mLon, rLat, rLon, color)
{
    this.type = "myArrow";
    this.getInfo = function () {
        return "Arrow " + mLat + " " + mLon;
    }
    this.mLat = mLat;
    this.mLon = mLon;
    //alert("Arrow" + lLat+" "+ lLon+" "+  mLat+" "+  mLon+" "+  rLat+" "+  rLon+" "+  color);
    var polyPath = new Array();
    polyPath[0] = new google.maps.LatLng(lLat, lLon);
    polyPath[1] = new google.maps.LatLng(mLat, mLon);
    polyPath[2] = new google.maps.LatLng(rLat, rLon);
    var polygon = new google.maps.Polygon({map: map, paths: polyPath, strokeColor: color, fillColor: color, strokeWeight:1, fillOpacity: .75, strokeOpacity: .75});

    this.setMap = function(value){
        polygon.setMap(value);
    }
    this.getMap = function(value){
        return polygon.getMap();
    }
    this.getPoly = function() {
        return polygon;
    }
    this.getPath = function(){
        return polyPath;
    }
}

// Define a symbol using SVG path notation, with an opacity of 1.
var lineSymbol = {
        path: 'M 0,-1 0,1',
        strokeOpacity: 1,
        strokeWeight: 1,
        scale: 3
};

var tickSymbol =
{
    path: 'M -1,0 1,0',
    strokeOpacity: 1,
 strokeWeight:1
};

var singleTick =
{
    path: 'M -1,0 1,0 M 0,-1 0,1',
    strokeOpacity: 1,
    strokeWeight:1
};

var doubleTick =
{
    path: 'M -1.5,0 1.5,0 M 0.5,-1 0.5,1 M -0.5,-1 -0.5,1',
    strokeOpacity: 1,
    strokeWeight:1
};

var doubleLine = {
       path: 'M 0.5,-1 0.5,1 M -0.5,-1 -0.5,1',
       strokeOpacity: 1,
       strokeWeight: 1,
       scale: 3
};

// class to contain segment info
function SegmentInfo(SegmentId, routeName, segmentName, oneWay, Color, tracks, dash )
{
 this.type = "SegmentInfo";

 var icons = [
     [{
          icon: lineSymbol, //0
          offset: '0%',
          repeat: '6px'
      }],
     [{
          icon: lineSymbol, //1
          offset: '50%',
          repeat: '15px'
      }],
     [{
          icon: doubleLine, // 2
          offset: '0%',
          repeat: '6px'
      }],
     [{
       icon: singleTick, // 3
       offset: '0%',
       repeat: '6px'
     }],
     [{
       icon: doubleTick, //4
       offset: '0%',
       repeat: '6px'
     }],
     [{
          icon: doubleLine, //5 Subway
          offset: '70%',
          repeat: '10px'
      }]
 ];
 var j = 0;
 if(tracks === 2) j=2;
 if(tracks===2 && dash ===2 ) j= 4;
 if(tracks ===1 && dash ===2) j= 3;
 if(dash === 1) j = 1;
 if(dash === 3) j = 5;

 //console.error("tracks =" + tracks + " index = " + j);
   this.line = new google.maps.Polyline(
   {
    strokeColor: Color,
    strokeOpacity: 0,
    //strokeWeight: weight,
    icons: icons[j]
   });

// if(dash === 0)
// {
//  this.line = new google.maps.Polyline(
//  {
//   strokeColor: Color,
//   strokeOpacity: 0,
//   //strokeWeight: weight,
//   icons: icons[j]
//  });
// }
// else if(dash ===1) // incline
// {
//  this.line = new google.maps.Polyline(
//  {
//   strokeColor: Color,
//   strokeOpacity: .75,
//   //strokeWeight: weight,
//   icons: [{
//           icon: lineSymbol,
//           offset: '0',
//           repeat: '20px'
//         }]
//  });
// }
// else // dash == 2 Surface PRW
// {
//  this.line = new google.maps.Polyline(
//  {
//   strokeColor: Color,
//   strokeOpacity: 1,
//   //strokeWeight: weight,
//   strokeWeight: 1,
////   icons: {
////           icon: doubleTick,
////           offset: '0',
////           repeat: '10px'
////         }
//               icons: icons[j]
//  });
// }

 this.line.segmentId = SegmentId;
 this.segmentId = SegmentId;
 this.routeName = routeName;
 this.segmentName = segmentName;
 this.oneWay = oneWay;
 this.Color = Color;
 //var Arrow = null;
 var arrow = null;
 this.getLine = function(){
  return newline;
 }
 this.getArrow = function(){
  if(arrow)
  {
  //alert("getArrow " + Arrow.getInfo());
  }
  return arrow;
 }

 this.getColor = function (){
  return Color;
}
var points = 0;
var newline = this.line;

this.getInfo = function () {
  return this.segmentName + " route:" + this.routeName;
}

this.setArrow = function (Arrow){
  arrow = Arrow;
}
var info=this.segmentName + " route" + this.routeName;

// function to determine if the supplied point is on a begining or end linesegement of a segment
this.isPointOnEnd = function(pt)
{
  var line =newline;
  var path = line.getPath();
  var len = path.getLength();
  webViewBridge.setLen(len);
  var i;
  var mIx = 1;
  var b1 = new bearing(pt.lat(), pt.lng(), path.getAt(0).lat(), path.getAt(0).lng());
  if(b1.getDistance() < .020)
      return 0;
  var b2 = new bearing(pt.lat(), pt.lng(), path.getAt(len-1).lat(), path.getAt(len-1).lng());
  if(b2.getDistance() < .020)
      return len-1;
  //alert("segment " + SegmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());
  webViewBridge.setDebug("segment " + SegmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());

  return -1;
}
// events
// Select segment (click)
google.maps.event.addListener(this.line, "click", function(e){

  webViewBridge.setDebug("sId = " + SegmentId + " " + segmentName);
  line = newline;
  Arrow = arrow;
  color = Color;
  //currSegment = this;
  hiLiteSelectedLine();

  var path = line.getPath();
  var len = path.getLength();
  var begin, end,bounds;
  webViewBridge.setLen(len);
  var i;
  var mIx = 1;
  for(i=0; i < path.getLength()-1; i++)
  {
      begin = path.getAt(i);
      end = path.getAt(i+1);
      bounds = SetBounds( begin, end);
      if( bounds.contains(e.latLng)){
          break;
      }
  }
  if(i>0)
      mIx=0;
  //addMarker(i, e.latLng.lat(), e.latLng.lng(), mIx, segmentName + " route:" + routeName);
  addMarker(i, begin.lat(), begin.lng(), mIx, segmentName + " route:" + routeName, SegmentId);
//OK            window.external.selectSegment(i, SegmentId);
  webViewBridge.selectSegment(i, SegmentId);
  addModeOff();
 });


 google.maps.event.addListener(this.line, "rightclick", function(e)
 {
     line = newline;
     Arrow = arrow;
     color = Color;
     //currSegment = this;
     if(selectedLine != null)
         selectedLine.setOptions({strokeColor: selectedLineClr});
     line.setOptions({strokeColor: "#04b4B4"});
     selectedLineClr = Color;
     selectedLine = line;
     var path = line.getPath();
     var len = path.getLength();
     //OK            window.external.setLen(len);

     webViewBridge.setLen(len);
     var i;
     for(i=0; i < path.getLength()-1; i++)
     {
         begin = path.getAt(i);
         end = path.getAt(i+1);
         bounds = SetBounds( begin, end);
         if( bounds.contains(e.latLng)){
             break;
         }
     }
     //OK            window.external.selectSegment(i, SegmentId);
     //webViewBridge.selectSegment(i, SegmentId);

     insertPoint(e, line, SegmentId);
 });
 google.maps.event.addListener(this.line, "dblclick", function(e)
 {
     line = newline;
     Arrow = arrow;
     color = Color;
     var path = line.getPath();
     var i;
     for(i=0; i < path.getLength()-1; i++)
     {
         begin = path.getAt(i);
         end = path.getAt(i+1);
         bounds = SetBounds( begin, end);
         if( bounds.contains(e.latLng)){
             break;
         }
     }
     //            window.external.setStation(e.latLng.lat(), e.latLng.lng(), SegmentId, i);
     webViewBridge.setStation(e.latLng.lat(), e.latLng.lng(), SegmentId, i);
 });
} // end SegmentInfo


//function initialize()
function initMap()
{
 console.log("begin GoogleMaps.js initialize()");
 connectSlots();
 geocoder  = new google.maps.Geocoder();
 var mapDiv = document.getElementById("map-canvas");

 //var Lat = 52.0;
 var Lat = webViewBridge.lat;
 //var Lon = 13.0;
 var Lon = webViewBridge.lng;
 //var zoom = 13;
 var zoom = webViewBridge.zoom;
 //var mapTypeId = google.maps.MapTypeId.ROADMAP;
 var mapTypeId = webViewBridge.maptype;

 map = new google.maps.Map(mapDiv, {
    center: new google.maps.LatLng(Lat, Lon),
    zoom: zoom,
    scaleControl: true,
    panControl: true,
    draggable: true,
    overviewMapControl: true,
    scrollwheel: true,
    disableDoubleClickZoom: true,
    mapTypeId: google.maps.MapTypeId.ROADMAP
 });

 google.maps.event.addListenerOnce(map, 'idle', function(){
        //this part runs when the mapobject is created and rendered
        google.maps.event.addListenerOnce(map, 'idle', function(){
            //this part runs when the mapobject shown for the first time
            webViewBridge.mapInit();
        });
    });

 google.maps.event.addDomListener(mapDiv, 'resize', function(){
     google.maps.event.trigger(map, 'resize');
    });
 //new google.maps.LatLng(21.291982, -157.821856),
 siArray = new google.maps.MVCArray();
 map.disableDoubleClickZoom = true;
 google.maps.event.addListener(map, "dblclick", addNewPoint);
 map.mapTypes.set('OSM', osm_MapType);
 map.mapTypes.set('UserMap',User_MapType);
 map.setMapTypeId(mapTypeId);

 defaultOptions = /** @type {google.maps.MapTypeControlOptions} */(
 {
  mapTypeControlOptions:
  {
     mapTypeIds: ['OSM',
        google.maps.MapTypeId.ROADMAP,
        google.maps.MapTypeId.SATELLITE,
        google.maps.MapTypeId.HYBRID,
        google.maps.MapTypeId.TERRAIN,
     ],
     scrollwheel: true,
     tilt: 0
  }
 });

 options = /** @type {google.maps.MapTypeControlOptions} */(
 {
  mapTypeControlOptions: {
      mapTypeIds: ['OSM',
         google.maps.MapTypeId.ROADMAP,
         google.maps.MapTypeId.SATELLITE,
         google.maps.MapTypeId.HYBRID,
         google.maps.MapTypeId.TERRAIN,
         'UserMap'
      ],
      scrollwheel: true,
      tilt: 0
  }
 });
 map.setOptions(defaultOptions);
 map.setMapTypeId(mapTypeId);
 stationArray = new google.maps.MVCArray();

 //google.maps.event.trigger(map, 'resize');


 webViewBridge.queryOverlay();

 //OK    window.external.displayZoom(map.getZoom());
 webViewBridge.displayZoom(map.getZoom());

 //alert("initialize end");

} // end initialize()

function initialize()
{
 initMap();
}

function resizeMap()
{
//google.maps.event.trigger(map, 'resize');
var mapDiv = document.getElementById("map-canvas");
}

function createSegment(segmentId, routeName, segmentName, oneWay, color, tracks, dash, points )
{
 newSegment = new SegmentInfo(segmentId, routeName, segmentName, oneWay, color, tracks, dash );

// if(weight !== 0 && weight !==3)
//  alert(newSegment.getInfo() + " args=" + arguments.length);
 line = newSegment.getLine();
 line.setMap(map);
 segment = newSegment;
 var pts = siArray.push(newSegment);

 //window.external.SetDebug(segment.getInfo());
 //OK            window.external.SetDebug("MVCArray points = " + pts);
 webViewBridge.setDebug("MVCArray points = " + pts);
 currSegment = newSegment;
 if(selectedLine != null)
 {
  restoreSelectedLine();
 }
 if(marker)
 {
  marker.setMap();
  marker = null;
 }
 if(circle)
 {
  circle.setMap();
  circle = null;
 }
 if(infowindow !== null)
 {
  infowindow.setMap();
  infowindow = null;
 }
 if(arguments.length > 7)
 {
 //alert(points + " " + arguments.length);
  var path = line.getPath();

  for(var i=0; i < points; i +=2)
  {
   path.push(new google.maps.LatLng(arguments[i+8], arguments[i+9]));
  }
  getPoints();
  placeArrow(path);
 }
 if(typeof points == 'array')
 {
  var path = line.getPath();

  for(var i=0; i < points.length; i +=2)
  {
      path.push(new google.maps.LatLng(points[i], points[i+1]));
  }
  getPoints();
  placeArrow(path);
 }
 return null;
}

function setGeocoderRequest(bRequest)
{
    bGoocoderRequest = bRequest;
    return null;
}

function geocoderRequest(lat, lon)
{
//alert("geocoderRequest: " + lat + " " + lon);
latlng = new google.maps.LatLng(lat, lon);
geocoder.geocode({'latLng': latlng}, function(results, status){
//geocoder.geocode({'bounds': circle.getBounds()}, function(results, status){
  //alert(results);
  if (status == google.maps.GeocoderStatus.OK)
  {
      //alert("count = " + results.length);
      array = new Array();
      for(i=0; i< results.length; i++)
      {
          //alert(results[i].address_components  );
          array.push((i>0?";":"")+results[i].formatted_address);
      }
//                    window.external.getGeocoderResults(array.toString());
      webViewBridge.getGeocoderResults(array.toString());
  }
   else
  {
//TODO                    window.external.getGeocoderResults("Geocode was not successful for the following reason: " + status);
      webViewBridge.getGeocoderResults("Geocode was not successful for the following reason: " + status);
  }

});
    return null;

}
function restoreSelectedLine()
{
if(selectedLine != null)
{
  //selectedLine.breakpt();
  selectedLine.setOptions({strokeColor: selectedLineClr});
  if(selectedPoly)
  {
      selectedPoly.setOptions({strokeColor: selectedLineClr, fillColor:selectedLineClr });
  }
  selectedPoly = null;
  selectedLine = null;
 }
 return null;
}

function hiLiteSelectedLine()
{
 if(selectedLine != null)
 {
  restoreSelectedLine();
 }
 line.setOptions({strokeColor: "#04b4B4"});
 if(Arrow)
 {
  var poly;
  poly = Arrow.getPoly();
  poly.setOptions({strokeColor: "#04b4B4", fillColor:"#04b4B4" });
  selectedPoly = Arrow.getPoly();
 }
 selectedLineClr = color;
 selectedLine = line;
 return null;
}

function addModeOn()
{
 bAdding = true;
 map.setOptions({draggableCursor:'Crosshair'});
    return null;
}
function addModeOff()
{
 bAdding = false;
 map.setOptions({draggableCursor:'Hand'});
 return null;
}

function addNewPoint(e)
{
if(bAdding)
{
 map.disableDoubleClickZoom = true;

  if(line === null)
  {
//OK                    window.external.SetDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
   webViewBridge.setDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
   return;
  }
  var path = line.getPath();
  if(path.getLength() == 0)
  {
   addMarker(path.getLength(), e.latLng.lat(), e.latLng.lng(), 1, segment.getInfo(), segment.segmentId);
  }
  path.push(e.latLng);
  getPoints();
  if(path.getLength() > 0)
// window.external.addPoint();
   webViewBridge.addPoint(0, e.latLng.lat(), e.latLng.lng());
  placeArrow(path);
 }
}


  function displayPoint( lat, lon)
  {
      var path = line.getPath();
      path.push(new google.maps.LatLng(lat, lon));
      getPoints();
      placeArrow(path);
  }

  function setCenter(Lat, Lon)
  {
   map.setCenter(new google.maps.LatLng(Lat, Lon));
   map.setOptions({disableDoubleClickZoom: true });
   return null;

  }
  function getCenter()
  {
   var latLng = map.getCenter();
      webViewBridge.setCenter(latLng.lat(), latLng.lng(), map.getZoom(), map.getMapTypeId());
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

function selectSegment(segmentId)
{
    //alert("selectSegment " + segmentId);
    var txt="N";
    //var color ="";
    siArray.forEach(function(si, ix)
    {
        if(si.segmentId !== null && si.segmentId === segmentId)
        {
            //line.breakpt();
            line = si.getLine();
            var path = line.getPath();
            var len = path.getLength();
            webViewBridge.setLen(len);
            color = si.Color;
            Arrow = si.getArrow();
            hiLiteSelectedLine();
            webViewBridge.selectSegment(0, segmentId);
            return true;
        }
    });
    return false;
}

function isSegmentDisplayed(segmentId)
{
 var txt="N";
 var color ="";
 siArray.forEach(function(si, ix)
 {
  try
  {
   if(si.segmentId && si.segmentId === segmentId)
   {
    color=si.getColor();
    var Line = si.getLine();
    if(line.getMap())
     txt = "Y";
    return txt;
   }
  }
  catch (err)
  {
   txt=err;
  }
 });
 webViewBridge.segmentStatus(txt, color);
 return txt;
}

// Erase a line segment and remove it from the list
function clearPolyline(segmentId)
{
//alert("polyline " + segmentId + " called");
siArray.forEach(function(si, ix)
{
  var txt="";
  var path;
  try{
      if(si.segmentId && si.segmentId == segmentId)
      {
          //alert(" ix = " + ix + " " +si.getInfo());
          line = si.getLine();
          line.setMap(null);
          path = line.getPath();
          while(path.getLength() > 0)
          {
              path.pop();
          }
          line.setPath(path);
          //siArray.removeAt(ix);
          line = null;
          Arrow = si.getArrow();
          if(Arrow )
          {
              Arrow.setMap();
              if(Arrow.getMap())
                  alert(Arrow.getInfo + " setmap failed");
              var poly = Arrow.getPoly();
              var path = Arrow.getPath();
              path = null;
              poly = null;
              //Arrow = null;
          }
          //siArray.removeAt(ix);
          si=null;
      }
  }
  catch (err)
  {
      txt=err;
      alert("clearPolyLine: segment: " + segmentId + "\n" + txt);
  }

});
var count = stationArray.getLength();
stationArray.forEach(function(stationMarker, ix)
{
  if(ix >= count)
      return;
  if(stationMarker && stationMarker !== 'undefined' && stationMarker.segmentId  && stationMarker.segmentId === segmentId)
  {
      stationMarker.setMap();
      stationArray.removeAt(ix);
  }
});
//alert("polyline " + segmentId + " cleared");
    return null;
}
function clearMarker()
{
if(marker)
{
  marker.setMap();
  marker = null;
}
if(circle)
{
  circle.setMap();
  circle = null;
}
if(infowindow !== null)
{
  infowindow.setMap();
  infowindow = null;
}
}
  // can be called by the c# program to set an arrow.
  function setArrow(lLat, lLon, mLat, mLon, rLat, rLon, segmentId)
  {
      //alert("setArrow: " + lLat+" "+ lLon+" "+  mLat+" "+  mLon+" "+  rLat+" "+  rLon+" "+  segmentId);
      siArray.forEach(function(si, ix)
      {
          var txt="";
          try{
              if(si.segmentId && si.segmentId == segmentId)
              {
                  si.setArrow ( new myArrow(lLat, lLon, mLat, mLon, rLat, rLon, si.getColor()));
              }
          }
          catch (err)
          {
              txt=err;
          }

      });
  }
  // Clear all the lines from the map
  function clearAll()
  {
   var path;
      while(siArray.getLength() > 0)
      {
          var si = siArray.pop();
          line = si.getLine();
          line.setMap(null);
          path = line.getPath();
          while(path.getLength() > 0)
          {
              path.pop();
          }
          line.setPath(path);
          line = null;
          var oneWay = si.oneWay;
          Arrow = si.getArrow();
         // alert(Arrow.getInfo());

          if(Arrow )
          {
              Arrow.setMap();
              if(Arrow.getMap())
                  alert(Arrow.getInfo + " setmap failed");
              var poly = Arrow.getPoly();
              var path = Arrow.getPath();
              path = null;
              poly = null;
              Arrow = null;
          }
          si=null;
      }
      if(marker)
      {
          marker.setMap();
          marker = null;
      }
      if(circle)
      {
          circle.setMap();
          circle = null;
      }
      if(poly2)
      {
          poly2.setMap();
          poly2 = null;
      }
      if(rtStartMarker !== null)
      {
          rtStartMarker.setMap();
          rtStartMarker = null;
      }
      if(rtEndMarker !== null)
      {
          rtEndMarker.setMap();
          rtEndMarker = null;
      }
      if(infowindow !== null)
      {
          infowindow.setMap();
          infowindow = null;
      }

      selectedLine = null;
      while(stationArray.getLength() > 0)
      {
          var stationMarker = stationArray.pop();
          stationMarker.setMap();
          if(stationMarker.infoWindow)
              stationMarker.infoWindow.setMap();
          stationMarker = null;
      }
      return null;
  }

function insertPoint(e, line, segmentId)
{
 webViewBridge.setDebug("insert point" + e);

 var path = line.getPath();
 // determine which line segment was clicked
 var i;
    var begin, end, bounds;
 for(i=0; i < path.getLength()-1; i++)
 {
  begin = path.getAt(i);
  end = path.getAt(i+1);
  bounds = SetBounds( begin, end);
  if( bounds.contains(e.latLng))
  {
   webViewBridge.setDebug("Insert " + i);
   path.insertAt(i+1, e.latLng);
   line.setPath(path);
//OK                    window.external.insertPoint(segmentId,  i, e.latLng.lat(), e.latLng.lng());
   webViewBridge.insertPoint(segmentId,  i, e.latLng.lat(), e.latLng.lng());
   addMarker(i+1, e.latLng.lat(), e.latLng.lng(), 0, "", segmentId);
   return null;
  }
 }
}
  function fitMapBounds(swLat, swLon, neLat, neLon)
  {
      map.fitBounds(new google.maps.LatLngBounds(new google.maps.LatLng(swLat, swLon), new google.maps.LatLng(neLat, neLon)));
      return null;
  }

  function editSegment(e)
  {
      var path = line.getPath();
      // determine which line segment was clicked
      var i;
      for(i=0; i < path.getLength()-1; i++)
      {
//                window.external.SetDebug("edit " + e.latLng.lat() + ", " + e.latLng.lng() );
          begin = path.getAt(i);
//                window.external.SetDebug(i + " edit  begin " + begin.lat() + ", " + begin.lng() );
          end = path.getAt(i+1);
          //alert("i=" + i + " " +  e.latLng + " " + begin + " " + end);
          bounds = SetBounds( begin, end);
          if( bounds.contains(e.latLng))
          {
//                    window.external.SetDebug(i + " editSegment");
//                    window.external.editSegment(i);
              break;
          }
      }
      return null;
  }


function deletePoint(pt)
{
 var path = line.getPath();
 var len = path.getLength();
 path.removeAt(pt);
 line.setPath(path);
 getPoints();
 // move the arrow as well
 placeArrow(path);
}

function placeArrow(path)
{
 if(path.getLength() > 1)
 {
  arrow =currSegment.getArrow();
  if(arrow)
  {
      arrow.setMap(null);
      var poly = arrow.getPoly();
      poly = null;
      arrow = null;
  }
  //else
  {
  var len = path.getLength();
  var brng = new bearing(path.getAt(len-1).lat(), path.getAt(len-1).lng(),path.getAt(len-2).lat(), path.getAt(len-2).lng() );
  var left = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()-15, .020);
  var right = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()+15, .020);
  currSegment.setArrow(new myArrow(left.lat(), left.lng(), path.getAt(len-1).lat(), path.getAt(len-1).lng(), right.lat(), right.lng(), currSegment.getColor()));
  }
 }
}

function getPoints()
{
var path = line.getPath();
var len = path.getLength();
path.forEach(setArray);
}

var pointArray = [];
function setArray(element, number)
{
 var pt = [element.lat(), element.lng()];
 pointArray[number]= pt;
}

function getLen()
{
 var len =-1;
 if(line == null)
  len = -1;
 else
 {
  var path = line.getPath();
   len = path.getLength();
 }
 return len;
}

function getPointValues(i)
{
 //alert("getPointValues: " + i);
 var path = line.getPath();
 var myElement = path.getAt(i);
 return i+","+myElement.lat()+","+myElement.lng();
}

// Return the entire array of points for the current line
function getPointArray()
{
 var path = line.getPath();
 var array = new Array(0,0);
 path.forEach(function(pt, ix)
 {
  array[ix*2] = pt.lat();
  array[(ix*2)+1] = pt.lng();
 });
 //alert("points = " +array.length);
 return array;
}


var circle;
function addMarker(i, lat, lon, icon, text, SegmentId)
{
 console.error("addMarker "+  i + " " + lat+ " " + lon+ " " + icon+ " " + text+ " " + SegmentId);
 segmentId = SegmentId;
 siArray.forEach(function(si, ix)
 {
  if(si.segmentId && si.segmentId === segmentId)
  {
      currSegment = si;
  }
 });
 if(marker)
 {
  marker.setMap();
  marker = null;
 }
 //marker.brkpt();
 webViewBridge.setDebug("add marker at lat: " + lat + " lon: " + lon + " point: " + i);
 //window.external.showSegmentsAtPoint(lat,lon);
 webViewBridge.showSegmentsAtPoint(lat,lon, SegmentId);
 marker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),
          draggable: true, icon:image[icon], title:text});
 marker.i = i;
 google.maps.event.addListener(marker, "dragend", function(pt) {
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  var path = line.getPath();
  path.setAt(i,  pt.latLng );
  line.setPath(path);

  // move the arrow as well
  placeArrow(path);
  //line.setMap(map);
//OK                window.external.movePoint( i, pt.latLng.lat(),pt.latLng.lng());

  webViewBridge.movePoint(SegmentId, i, pt.latLng.lat(), pt.latLng.lng());
  //TODO                window.external.showSegmentsAtPoint(lat,lon, SegmentId);
  webViewBridge.showSegmentsAtPoint(lat,lon, SegmentId);
  if(circle)
  {
   circle.setMap();
   circle=null;
  }
  if(bGeocoderRequest)
  {
      circle = new google.maps.Circle({center:pt.latLng, fillOpacity: 0, map: map, strokeColor:"#000000", strokeWeight:1, radius:20, clickable:false});
      geocoderRequest(pt.latLng.lat(), pt.latLng.lng());
  }
 });

 google.maps.event.addListener(marker, "rightclick", function(){
  var pt = marker.position;
//OK                window.external.SetDebug("right click " + pt.lat() + ", " + pt.lng());
  webViewBridge.setDebug("right click " + pt.lat() + ", " + pt.lng());
  var path = line.getPath();
  var len = path.getLength();

  //path.setAt(i,  pt.latLng );
  //line.setPath(path);
  //line.setMap(map);
//OK                window.external.updateIntersection( i, pt.lat(), pt.lng());
   if(len > 1)
    webViewBridge.updateIntersection( marker.i, pt.lat(), pt.lng());
  }
);

google.maps.event.addListener(marker, "dblclick", function()
{
  var path = line.getPath();
  var i;
  for(i=0; i < path.getLength()-1; i++)
  {
      var begin = path.getAt(i);
      var end = path.getAt(i+1);
      var bounds = SetBounds( begin, end);
      if( bounds.contains(marker.getPosition())){
          break;
      }
  }
//                window.external.setStation(marker.getPosition().lat(), marker.getPosition().lng(), SegmentId, i);
  webViewBridge.setStation(marker.getPosition().lat(), marker.getPosition().lng(), SegmentId, i);
 });
 if(circle)
 {
  circle.setMap();
  circle=null;
 }
 circle = new google.maps.Circle({center:new google.maps.LatLng(lat, lon), fillOpacity: 0, map: map, strokeColor:"#000000", strokeWeight:1, radius:20});
 if(bGeocoderRequest)
    geocoderRequest(lat, lon);
}

  function SetBounds( pt1, pt2)
  {
      var swlat, swlng;
      var nelat, nelng;
      if(pt1.lat() < pt2.lat())
      {
          swlat = pt1.lat();
          nelat = pt2.lat();
      }
      else
      {
          swlat = pt2.lat();
          nelat = pt1.lat();
      }
      if(pt1.lng() < pt2.lng())
      {
          swlng = pt1.lng();
          nelng = pt2.lng();
      }
      else
      {
          swlng = pt2.lng();
          nelng = pt1.lng();
      }
      return new google.maps.LatLngBounds(new google.maps.LatLng(swlat, swlng), new google.maps.LatLng(nelat, nelng));
  }

  function displayTerminalMarkers(bDisplay)
  {
      //alert("display terminal markers");
      if(bDisplay)
      {
          if(rtStartMarker !== null)
            rtStartMarker.setMap(map);
          if(rtEndMarker !== null)
            rtEndMarker.setMap(map);
      }
      else
      {
          if(rtStartMarker !== null)
            rtStartMarker.setMap();
          if(rtEndMarker !== null)
            rtEndMarker.setMap();
      }
      return null;
  }
  function addRouteStartMarker( lat, lon, image)
  {
//      this.lat = lat;
//      this.lon = lon;
      if(rtStartMarker !== null)
      {
          rtStartMarker.setMap();
          rtStartMarker = null;
      }
      rtStartMarker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),draggable: true, icon: image});
      google.maps.event.addListener(rtStartMarker, "dragend", function(pt) {
          var found = false;
          siArray.forEach(function(si, ix)
          {
              //try
              //{

                  var i = si.isPointOnEnd(pt.latLng);
                  if(i >=0)
                  {
//OK                            window.external.moveRouteStartMarker(pt.latLng.lat(), pt.latLng.lng(), si.segmentId, i );
                      webViewBridge.moveRouteStartMarker(pt.latLng.lat(), pt.latLng.lng(), si.segmentId, i );
                      found = true;
                  }
              //}
              //catch (e)
              //{
              //}
          });
          if(found == false)
              rtStartMarker.setPosition(new google.maps.LatLng(lat, lon));
      });
      return null;
  }
  function addRouteEndMarker( lat, lon, image)
  {
      if(rtEndMarker !== null)
      {
          rtEndMarker.setMap();
          rtEndMarker = null;
      }
      rtEndMarker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),draggable: true, icon: image});
      google.maps.event.addListener(rtEndMarker, "dragend", function(pt) {
          var found = false;
          siArray.forEach(function(si, ix)
          {
              //try
              //{
                  var i = si.isPointOnEnd(pt.latLng);
                  if(i >=0)
                  {
//                            window.external.moveRouteEndMarker(pt.latLng.lat(), pt.latLng.lng(), si.segmentId, i );
                      webViewBridge.moveRouteEndMarker(pt.latLng.lat(), pt.latLng.lng(), si.segmentId, i );
                     found = true;
                  }
              //}
              //catch (e)
              //{
              //    alert(e);
              //}
          });
          if(found == false)
              rtEndMarker.setPosition(new google.maps.LatLng(lat, lon));
      });
      return null;
  }

function addStationMarker(lat, lon, visible, segmentId, stationName, stationKey, infoKey, HTMLText, typeIcon)
{
 console.error("addStationMarker lat=" + lat +  " lon=" + lon + " visible=" +  visible  + " segmentid =" + segmentId + " name= "  + stationName + " stationKey=" +  stationKey + " infoKey="+ infoKey + " text =" + HTMLText +  " icon=" + typeIcon);

 var bPresent = false;
 if(stationArray)
 {
  // check to see if already present
  var count = stationArray.getLength();
  stationArray.forEach(function(element, index)
  {
   if(index >= count)
   {
    return;
   }
   if(element  && element.stationKey === stationKey)
   {
    console.error("stationKey " + stationKey + " is already present");
    bPresent = true;
    return;
   }
  });
 }
 if(bPresent)
     return;
 var icon = getIcon(typeIcon);
 var shadow = getShadow(typeIcon);

 console.error("icon is typeof " + typeof(icon) + " " + icon)
 var stationMarker = new google.maps.Marker({
                                              position: new google.maps.LatLng(lat, lon),
                                              icon:icon,
                                              shadow:shadow,
                                              draggable:true,
                                              title:stationName
                                             });
 stationMarker.setMap(map);
 stationMarker.setVisible(visible)
 stationMarker.segmentId = segmentId;
 stationMarker.stationKey = stationKey;
 stationMarker.HTMLText = HTMLText;
 stationMarker.typeIcon = typeIcon;
 stationMarker.infoWindow = null;
  //stationArray.push(stationMarker);

 google.maps.event.addListener(stationMarker, "rightclick", function(){
//                window.external.updateStation(stationMarker.stationKey, segmentId);
      webViewBridge.updateStation(stationMarker.stationKey, segmentId);
 });
 if(HTMLText)
 {
      stationMarker.infoKey = infoKey;
      stationMarker.infoWindow = new google.maps.InfoWindow({content:HTMLText, position:new google.maps.LatLng(lat, lon)});
      google.maps.event.addListener(stationMarker, "click", function() {
          stationMarker.infoWindow.open(map);
      });
 }
 google.maps.event.addListener(stationMarker, "dragend", function(pt)
 {
  //var found = false;
  //var closestPoint=new google.maps.LatLng(0,0);
  var closestPoint=pt.latLng;
  var distance = 9999999.0;
  var segmentId = stationMarker.segmentId;
  siArray.forEach(function(si, ix)
  {
   var line = si.getLine();
   var path = line.getPath();
   path.forEach(function(pt2, ix2)
   {
    var newBearing =  new bearing(pt.latLng.lat(), pt.latLng.lng(), pt2.lat(), pt2.lng());
    var newDistance = newBearing.getDistance();
    if(newDistance < distance)
    {
     distance = newDistance;
     //var x = distance.getx();
     closestPoint = pt2;
     segmentId = si.segmentId;
    }
   });
  });
  webViewBridge.moveStationMarker(stationMarker.stationKey, segmentId, closestPoint.lat(), closestPoint.lng());
 //if(found == false)
 //    rtStartMarker.setPosition(new google.maps.LatLng(lat, lon));
 });
   stationArray.push(stationMarker);
 //return null;
}

function getIcon(typeIcon)
{
    var icon = image[images.default];
    if(typeIcon)
    {
     switch (typeIcon)
     {
      case "arrow":
         icon = image[images.arrow];
         break;
      case "red":
         icon = image[images.smallred];
         break;
      case "blue":
         icon = image[images.smallBlue];
         break;
      case "green":
         icon = image[images.smallGreen];
         break;
      case "sbahn":
         icon = image[images.sbahn];
         break;
      case "ubahn":
         icon = image[images.ubahn];
         break;
      case "tram":
         icon = image[images.tram];
         break;
      case "yellow":
         icon = image[images.smallYellow];
         break;
      case "orange":
         icon = image[images.orange];
         break;
      case "bvgtram":
          icon = image[images.bvgtram];
          break;
      case "tram":
          icon = image[images.tram];
          break;
      case "subway":
          icon = image[images.subway];
          break;
      default:
         //alert("icon type = " + typeIcon);
         icon = image[images.smallWhite];
         break;
     }
    }
 return icon;
}

function getShadow(typeIcon)
{
    var shadow = image[images.smallShadow];
    if(typeIcon)
    {
     switch (typeIcon)
     {
      case "tram":
          shadow = image[images.tramshadow];
          break;
      case "subway":
          shadow = image[images.subwayshadow];
          break;
      default:
         shadow = image[images.smallShadow];
         break;
     }
 }
 return shadow;
}

function displayStationMarkers(bDisplay)
{
 //alert(stationArray);
 if(!stationArray)
  return null;
 stationArray.forEach(function(element, index)
 {
  if(element)
  {
   element.setVisible(bDisplay);
  }
 });
 return null;
}

function removeStationMarker(stationKey)
{
 var count = stationArray.getLength();
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
      return;
  if(element && element !== 'undefined' && element.stationKey === stationKey)
  {
      element.setMap();
      stationArray.removeAt(index);
      return;
  }
 });
    return null;
}

function removeStationMarkers()
{
 var count = stationArray.getLength();
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
      return;
  while(stationArray.getLength() > 0)
  {
   var stationMarker = stationArray.pop();
   stationMarker.setMap();
   if(stationMarker.infoWindow)
    stationMarker.infoWindow.setMap();
   stationMarker = null;
  }
 });
    return null;
}

function getStationMarkerIconType(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "???";
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
   return rVal;
  if(element && element != 'undefined' && element.stationKey == stationKey)
  {
   //alert("icontype = " + element.typeIcon);
   rVal =  element.typeIcon;
   return rVal;
  }
 });
 return rVal;
}

  var animationPath =null;
  var animationPoly = null;
  //var markerArray = null;
  var markerImageArray = null;
  function createAnimation()
  {
      animationPath = new google.maps.MVCArray();
      //markerArray = new google.maps.MVCArray();
      markerImageArray = new google.maps.MVCArray();
  }
  function addAnimationPoint(lat, lon, bStop, content)
  {
      var point = new google.maps.LatLng(lat, lon);
      point.bStop = bStop;
      point.content = content;
      animationPath.push(point);
      if(animationPath.getLength() > 1)
      {
          var first = animationPath.getLength()-2;
          var next = animationPath.getLength()-1;
          var imagePath = webViewBridge.getImagePath(first);
          var markerImage = new google.maps.MarkerImage(imagePath, new google.maps.Size(120,120), new google.maps.Point(0,0),new google.maps.Point(60,60));
          markerImageArray.push( markerImage);
      }
  }
  //var imagePath;
  function setImagePath(path)
  {
      imagePath = path;
  }
  function startAnimation()
  {
      animationPoly = new google.maps.Polyline({
        strokeColor: "#0000FF",
        strokeOpacity: .75,
        strokeWeight: 2
      });
      if(marker)
      {
          marker.setMap();
          marker=null;
      }
      marker = new google.maps.Marker({position: animationPath.getAt(0), icon:markerImageArray.getAt(0)});
      marker.setMap(map);
      map.setCenter(animationPath.getAt(0));
      if(poly2)
      {
          poly2.setMap();
          poly2 = null;
      }
      poly2 = new google.maps.Polyline({
        strokeColor: "#0000FF",
        strokeOpacity: .75,
        strokeWeight: 2
      });
      poly2.setMap(map);
      map.setZoom(16);
      setTimeout("animate(0)",2000);  // Allow time for the initial map display

  }
  var k =0;
  var step = 5; // metres
  var tick = 150; // milliseconds
  var stepnum=0;
  var speed = "";
  var poly2 = null;
  var currPt = 0;

  function updatePoly(d)
  {
      poly2.getPath().push(animationPath.getAt(d));
  }

  function animate(d)
  {
      if(animationPath == null || d >= animationPath.getLength())
          return;
      var p = animationPath.getAt(d);   // get the point
      if(k++>180/step)
      {
          map.panTo(p);
          k=0;
      }
      var bounds = map.getBounds();
      if(!bounds.contains(p))
          map.panTo(p);
      if(markerImageArray.getAt(d))
      {
          marker.setIcon(markerImageArray.getAt(d));
          marker.setPosition(p);
      }
      updatePoly(d);
      if(p.bStop === true)
      {
          var content = p.content +
              "<p><img src='" + image[images.stop] + "' onclick='stopAnimation();' title='click to stop'/>  " +
              "<img src='" + image[images.resume] + "' onclick='animationContinue();' title='click to continue'/></p>";
          var info = new google.maps.InfoWindow({content:content, position:p});
          infoWindow = info;
          google.maps.event.addListener(info, "closeclick", function(){
              setTimeout("animate("+(d+1)+")", tick);
          });
          info.open(map);
      }
      else
      {
          setTimeout("animate("+(d+1)+")", tick);
          currPt = d+1;
      }
  }
  function stopAnimation()
  {
      animationPath = null;
      marker.setMap();
      marker = null;
      infoWindow.close();
  }
  function animationContinue()
  {
      setTimeout("animate("+(currPt+1)+")", tick);
      infoWindow.close();
  }

  // addAddressToMap() is called when the geocoder returns an
  // answer.  It adds a marker to the map with an open info window
  // showing the nicely formatted version of the address and the country code.
  function addAddressToMap(results, status) {
    //map.clearOverlays();
      if (status == google.maps.GeocoderStatus.OK) {
          map.setCenter(results[0].geometry.location);
          marker = new google.maps.Marker({
              map: map,
              position: results[0].geometry.location
          });
          //reverseGeocode(point.y,point.x);
          var CountryNameCode = "";
          var i = 0;
          while(results[0].address_components[i])
          {
              if(results[0].address_components[i].types[0] == "country")
              {
                  CountryNameCode = results[0].address_components[i].short_name;
                  break;
              }
              i++;
          }
          var contentString = results[0].formatted_address + '<br/>' +
              '<b>Country code:</b> ' + CountryNameCode;
          var infowindow = new google.maps.InfoWindow({content: contentString});
          infowindow.open(map,marker);

      } else {
          alert("Sorry, we were unable to geocode that address");
    }
  }

  // showLocation() is called when you click on the Search button
  // in the form.  It geocodes the address entered into the form
  // and adds a marker to the map at that location.
  function showLocation(address) {
    //var address = document.geocoderForm.address.value;
    geocoder.geocode({'address':address}, addAddressToMap);
  }

// class to create an overlay
function Overlay(name, opacity, minZoom, maxZoom, source, bounds, urls)
{
 var imageMapType = null;
 this.name = name;
 this.opacity = opacity;
 this.minZoom = minZoom;
 this.maxZoom = maxZoom;
 this.source = source;
 this.setOpacity= function(o) {
  opacity = o;
  imageMapType.setOpacity(opacity/100);
 };
 this.getOpacity = function()
 {
  return opacity;
 }
 // nodejs:
 //var str = "http://localhost:3000"+"/"+zoom+ "/" +x+"/"+coord.y+".png";
       //console.error(str + " x=" + coord.x+ " coord.y=" + coord.y+ " zoom=" + zoom + " y=" + y);
 if(source === "mbtiles")
 {
  imageMapType = new google.maps.ImageMapType( {
   getTileUrl: function(coord, zoom) {
      var str = urls + "?db="+name+ ".mbtiles&z="+zoom+"&x="+coord.x+"&y="+((1 << zoom) - coord.y - 1);
      console.log(str + " x=" + coord.x+ " coord.y=" + coord.y+ " zoom=" + zoom + " y=" + coord.y);
      return str;
     },
     tileSize: new google.maps.Size(256, 256)
    });
 }
 else
 if(source === "acksoft")
 {
  imageMapType = new google.maps.ImageMapType( {
   getTileUrl: function(coord, zoom)
   {
    var ymax = 1 << zoom;
    var y = ymax - coord.y -1;
    var x = coord.x;
    var str = urls + name + "/" +zoom+"/"+x+"/"+x+"_"+y+"_"+zoom+".png";
    return str;
   },
   tileSize: new google.maps.Size(256, 256)
  });
 }
 else if(source === "tileserver")
 {
  // localhost tileserver

  imageMapType = new google.maps.ImageMapType({
                getTileUrl: function(coord, zoom) {
                  var proj = map.getProjection();
                  var z2 = Math.pow(2, zoom);
                  var tileXSize = 256 / z2;
                  var tileYSize = 256 / z2;
//                  var tileBounds = new google.maps.LatLngBounds(
//                    proj.fromPointToLatLng(new google.maps.Point(coord.x * tileXSize, (coord.y + 1) * tileYSize)),
//                    proj.fromPointToLatLng(new google.maps.Point((coord.x + 1) * tileXSize, coord.y * tileYSize))
//                  );
//                  if (!mapBounds.intersects(tileBounds) || zoom < minZoom || zoom > maxZoom) return null;
                  return "http://localhost/tileserver.php?/index.json?/" + name+"/{z}/{x}/{y}.png".replace('{z}',zoom).replace('{x}',coord.x).replace('{y}',coord.y);
                },
                tileSize: new google.maps.Size(256, 256),
                minZoom: minZoom,
                maxZoom: maxZoom,
                name: 'Tiles'
            });
 }
 else if(source === "georeferencer")
 {
  var vals = bounds.split(",");

//     var mapBounds = new google.maps.LatLngBounds(
//                 new google.maps.LatLng(38.623972, -90.330807),
//                 new google.maps.LatLng(38.658606, -90.273631));
  var mapBounds = new google.maps.LatLngBounds(new google.maps.LatLng(vals[1], vals[0]),  new google.maps.LatLng(vals[3], vals[2]));
             var mapMinZoom = minZoom;
             var mapMaxZoom = maxZoom;
             var opts = {
               streetViewControl: false,
               tilt: 0,
               mapTypeId: google.maps.MapTypeId.HYBRID,
               center: new google.maps.LatLng(0,0),
               zoom: mapMinZoom
             }
     imageMapType = new google.maps.ImageMapType({
                 getTileUrl: function(coord, zoom) {
                   var proj = map.getProjection();
                   var z2 = Math.pow(2, zoom);
                   var tileXSize = 256 / z2;
                   var tileYSize = 256 / z2;
                   var tileBounds = new google.maps.LatLngBounds(
                     proj.fromPointToLatLng(new google.maps.Point(coord.x * tileXSize, (coord.y + 1) * tileYSize)),
                     proj.fromPointToLatLng(new google.maps.Point((coord.x + 1) * tileXSize, coord.y * tileYSize))
                   );
//                   if (!mapBounds.intersects(tileBounds) || zoom < mapMinZoom || zoom > mapMaxZoom) return null;
//                   return ["http://georeferencer-0.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png","http://georeferencer-1.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png","http://georeferencer-2.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png","http://georeferencer-3.tileserver.com//7600abd7e81c8d7fbc5043849452e2770741fd01/map/ztaRqNjoqdA7eUNIHwtt6W/201509152031-GrcyZ5/polynomial/{z}/{x}/{y}.png"][(coord.x+coord.y)%4].replace('{z}',zoom).replace('{x}',coord.x).replace('{y}',coord.y);
//                     return urls[(coord.x+coord.y)%4].replace('{z}',zoom).replace('{x}',coord.x).replace('{y}',coord.y);
                     return urls[0].replace('{z}',zoom).replace('{x}',coord.x).replace('{y}',coord.y);
                 },
                 tileSize: new google.maps.Size(256, 256),
                 minZoom: mapMinZoom,
                 maxZoom: mapMaxZoom,
                 name: 'Tiles'
             });

             map.overlayMapTypes.push(imageMapType);
             map.fitBounds(mapBounds);
 }
 else
  return "";
 imageMapType.setOpacity(opacity/100);
 map.overlayMapTypes.push(imageMapType);

}

function loadOverlay(name, opacity, minZoom, maxZoom, source, bounds, urls)
{
 console.log("load overlay: " + name + " opacity =" + opacity + " minZoom =" + minZoom + " maxZoom = " + maxZoom + " source = " + source + " bounds = " + bounds + " urls = " + urls);
    console.log("urls type = " + typeof(urls));
    if( Object.prototype.toString.call( urls ) === '[object Array]' )
    {
     console.log("size = " + urls.length +" url = " + urls[0] );
    }

 if(minZoom < 0 || maxZoom > 20)
     console.warn("invalid min/max zoom for overlay: " + name + " opacity =" + opacity + " minZoom =" + minZoom + " maxZoom = " + maxZoom);
 if ( overlay !== null)
 {
  map.overlayMapTypes.clear();
  overlay = null;
  if(opacityControl !== null)
  {
   opacityControl.remove();
   opacityControl = null;
  }
 }
 if(name === null || name === "")
 {
  return;
 }
 overlay = new Overlay(name, opacity, minZoom, maxZoom, source, bounds, urls);
 if(opacityControl === null)
 {
  opacityControl = new OpacityControl('opacityControl', map, google.maps.ControlPosition.RIGHT_TOP, overlay);
  opacityControl.initialize(map);
  google.maps.event.addListener(opacityControl, "opacitychanged", function()
  {
   webViewBridge.opacityChanged( overlay.name, overlay.getOpacity() );
  });
 }
 google.maps.event.addListener(map, "zoom_changed", function() {
  webViewBridge.displayZoom(map.getZoom());
  var newZoom = map.getZoom();

  if(overlay != null)
  {
   //console.error("zoom changed: zoom = "+ newZoom + " minZoom =" + overlay.minZoom + " maxZoom = " + overlay.maxZoom);
   if(newZoom < overlay.minZoom || newZoom > overlay.maxZoom)
   {
    if(opacityControl !== null) {
     opacityControl.remove();
     opacityControl = null;
    }
   }
   else
   {
    if(opacityControl === null)
    {
     opacityControl = new OpacityControl('opacityControl', map, google.maps.ControlPosition.RIGHT_TOP, overlay);
     opacityControl.initialize(map);
     google.maps.event.addListener(opacityControl, "opacitychanged", function()
     {
      webViewBridge.opacityChanged( overlay.name, overlay.getOpacity() );
     });
     webViewBridge.setDebug("opacity control added");
    }
   }
  }
 });
}

function setOverlayOpacity(Opacity) {
 if(overlay)
 {
  overlay.setOpacity(Opacity);
 }
}

function showRouteInfo(bDisplay)
{
 if(infowindow !== null)
 {
  if(bDisplay)
  {
      infowindow.setMap(map);
  }
  else
  {
     infowindow.setMap();
  }
 }
}

function displayRouteInfo(lat, lon, HTMLText, route, date)
{
 if(infowindow !== null)
 {
  infowindow.setMap();
  infowindow = null;
 }

 infowindow = new google.maps.InfoWindow({content:date+HTMLText, position:new google.maps.LatLng(lat, lon)}, 'return 0');
 //infowindow.setMap(map);
 infowindow.route = route;
 infowindow.date = date;
 infowindow.lat = lat;
 infowindow.lon = lon;
}

function nextRouteComment()
{
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, 1);
 return 0;
}

function prevRouteComment()
{
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, -1);
 return 0;
}

function setDefaultOptions()
{
 //alert("setDefaultOptions");
 map.setOptions(defaultOptions);
}

function setOptions()
{
 //alert("setDefaultOptions");
 map.setOptions(options);
}

function isOverlayLoaded()
{
 if(overlay == null)
    return "false";
 return "true";
}


//alert("begin Loading GoogleMaps.js 2042");

//google.maps.event.addDomListener(window, "load", initialize);

function displayStationMarker(stationKey, bDisplay)
{
 if(!stationArray)
    return ;
 var count = stationArray.getLength();
 console.error("displayStationMarker " + stationKey + " count = " + count);
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
    return;
  if(element && element.stationKey === stationKey)
  {
   element.setVisible(bDisplay)
   webViewBridge.setDebug("stationMarker " + stationKey + " is now visible " + element.getVisible());
  }
 });
}

function updateStationMarker(stationKey, typeIcon)
{
 if(!stationArray)
    return ;
 var icon = getIcon(typeIcon)
 var shadow = getShadow(typeIcon);


 var count = stationArray.getLength();
 console.error("displayStationMarker " + stationKey + " count = " + count);
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
    return;
  if(element && element.stationKey === stationKey)
  {
   element.setIcon(icon);
   element.typeIcon = typeIcon;
  }
 });
}

function isStationMarkerDisplayed(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "false";
 stationArray.forEach(function(element, index)
 {
  if(index >= count)
  {
   console.error("stationKey " + stationKey + " not found");
   return "false";
  }
  if(element  && element.stationKey === stationKey)
  {
    console.log("stationKey " + stationKey + " is visible " + element.getVisible());
    if(element.getVisible())
        return "visible";
    else
        return "hidden";
  }
 });
 console.error("stationKey " + stationKey + " not found 2");
 return rVal;
}
window.addEventListener("beforeunload", function (e) {
  var confirmationMessage = "\o/";
  /* Do you small action code here */
    alert("closing");
  (e || window.event).returnValue = confirmationMessage; //Gecko + IE
  return confirmationMessage;                            //Webkit, Safari, Chrome
});
console.log("GoogleMaps.js loaded!");
