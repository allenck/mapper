//'use strict';
var googleEarth;
//var line;
//var grayLine;
let map;
var segment;
var siArray;
//var Arrow;
var color;
var defaultOptions;
var options;
var txt;
var stationArray;// = new google.maps.MVCArray();
var fRslt;
var mapDiv;
var bGoogleInit = false;
var newSegment, segmentId, arrow,lat=0,lon=0;
var marker = null;
var selectedLine = null;
var hiLitedSegment = null;
//var selectedLineClr = "#FF0000";
//var hiLitedLineClr = "#FFFFFF";
//var selectedPoly = null;
var geocoder;
var currSegment = null;
var bAdding = false;
var myRslt = 0;
var rtStartMarker= null;
var rtEndMarker = null;
var infowindow = null;
var osm_MapType;
var stationArray = null;
var overlay=null;
var opacityControl = null;
var bGeocoderRequest = false;
var User_MapType = null;

function echoText(text)
{
    console.log("echoText received: '" +text+"'");
}


console.log("Loading GoogleMaps.js");
var image = ["http://maps.google.com/mapfiles/marker.png",
  "http://maps.google.com/mapfiles/dd-start.png",
  "http://maps.google.com/mapfiles/dd-end.png",
  "http://maps.google.com/mapfiles/shadow50.png",
  "http://www.google.com/mapfiles/arrow.png",
  "http://www.google.com/mapfiles/arrowshadow.png",
  "http://maps.google.com/mapfiles/kml/paddle/grn-blank-lv.png",
  "http://maps.google.com/mapfiles/kml/paddle/blu-blank.png",
  "http://maps.google.com/mapfiles/kml/paddle/pink-blank.png",
  "http://maps.google.com/mapfiles/shadow50.png",
  "http://acksoft.dyndns.biz/picturegallery/images/17.png",
  "http://acksoft.dyndns.biz/picturegallery/images/129.png",
  "http://maps.google.com/mapfiles/kml/paddle/ylw-blank.png",
  "http://hpstorage.acksoft.dyndns.biz/picturegallery/images/sbahn_small.png",
  "http://hpstorage.acksoft.dyndns.biz/picturegallery/images/ubahn_small.png",
  "http:ubuntu-2:1080/public/map_tiles/tram.png",
  "http:ubuntu-2:1080/public/map_tiles/tram.shadow.png",
  "http://maps.google.com/mapfiles/kml/paddle/wht-blank.png",
  "http:ubuntu-2:1080/public/map_tiles/blue-red-blank.png",
  "http://maps.google.com/mapfiles/kml/paddle/orange-blank.png",
  "http:ubuntu-2:1080/public/map_tiles/BVGTram.png",
  "http:ubuntu-2:1080/public/map_tiles/subway.png",
  "http:ubuntu-2:1080/public/map_tiles/subway.shadow.png",
  "http://maps.google.com/mapfiles/kml/paddle/purple-blank.png",
  "http://maps.google.com/mapfiles/kml/pal5/icon63l.png", // "H"
  "http://maps.google.com/mapfiles/kml/shapes/bus.png",
  "https://www.google.com/mapfiles/arrow.png" // big green down arrow
];
var images = {"default":0, "start":1, "end":2, "shadow":3, "arrow":4, "arrowShadow":5,
  "smallGreen":6, "smallBlue":7,"smallRed":8,"smallShadow":9,"stop":10,"resume":11,"smallYellow":12,
  "sbahn":13, "ubahn":14, "tram":15, "tramshadow":16, "smallWhite":17, "blue-red":18,
  "orange":19, "bvgtram":20, "subway":21, "subwayshadow":22, "purple":23, "rail":24, "bus":25, "greenDownArrow":26};

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
      //alert(google.maps.version);
  }
  return;
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
   var myFucn =  Function(call);
   var fRslt = myFucn();
   if(fRslt === null) return;
   if( fRslt instanceof Array)
    webViewBridge.scriptArrayResult( fRslt);
   else
   {
//       console.trace( call + fRslt);
//    if(webViewBridge.scriptResult !== null)
       if("fRslt" in window)
        webViewBridge.scriptResult( fRslt);
       else
        console.trace("bad return: '" + call + fRslt + "'");
   }
  }
  catch (err)
  {
   txt=err;
   alert("Error ocurred calling " + func + " '"+ parms + "'\n" + txt);
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
      var myFucn =  Function(call);
      var fRslt = myFucn();
      if(fRslt === null) return;
//      console.trace( call + fRslt);
//      if(webViewBridge.scriptResult !== null)
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
//          if(fRslt === 0)
//              alert(call);
//          if(webViewBridge.scriptResult !== null)
//          console.trace( call + fRslt);
          if("fRslt" in window)
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

var userMap = "Berlin_Bauentwicklung.1940.10000.300.3068";
var userMapTitle = "Berlin 1940";

function Get_User_MapType(tile, zoom)
{
var ymax = 1 << zoom;
var y = ymax - tile.y -1;
return "http:ubuntu-2:1080/public/map_tiles/" + userMap +"/"  +zoom+"/"+tile.x+"/"+tile.x+"_"+y+"_"+zoom+".png";
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

// User_MapType = new google.maps.ImageMapType(
// {
// getTileUrl: Get_User_MapType ,
// tileSize: new google.maps.Size(256, 256),
// isPng: true,
// alt: userMap,
// name: userMapTitle,
// maxZoom: 19
// });

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
function myArrow(lLat, lLon, mLat, mLon, rLat, rLon, color, segmentId)
{
    this.type = "myArrow";
    this.getInfo = function () {
        return "Arrow " + mLat + " " + mLon;
    }
    this.mLat = mLat;
    this.mLon = mLon;
    var segment = segmentId;
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
    this.getSegment = function()
    {
     return segment;
    }
    this.setOptions = function(options){
     polygon.setOptions(options);
    }
    // Select segment (click)
    google.maps.event.addListener(polygon, "click", function(){
//        alert("arrow clicked on " + segment);
        selectSegment(segment);
    });
} //end myArrow

// new arrow
function nArrow(segmentId, pt, dx, dy)
{
 var arrowSymbol  = {
        path: 'M -3, -4 0, 4 3, 0 0,-4'
    };
}

var zoomIx;
var zoomOffset;
var offsets = [.5, .6, .7, .8, .9, 1.0];

// Define a symbol using SVG path notation, with an opacity of 1.
var lineSymbol = {
    path: 'M 0,-1 0,1',
    strokeOpacity: 1,
    strokeWeight: 2,
    name:  "lineSymbol",
    scale: 3
};

var tickSymbol =
{
    path: 'M -1,0 1,0',
    strokeOpacity: 1,
    strokeWeight:1,
    name:  "tickSymbol"
};

var singleTick =
{
    path: 'M -1,0 1,0 M 0,-1 0,1',
    strokeOpacity: 1,
    strokeWeight:1,
    name:  "singleTick"
};

var doubleTick =
{
    path: 'M -1.5,0 1.5,0 M 1.0,-1 1.0,1 M -1.0,-1 -1.0,1',
    strokeOpacity: 1,
    strokeWeight:1,
    name:  "doubleTick"
};

var pArray = ["M 0.5,-1 0.5,1 M -0.5,-1 -0.5,1", "M 0.6,-1 0.6,1 M -0.6,-1 -0.6,1",
        "M 0.7,-1 0.7,1 M -0.7,-1 -0.7,1","M 0.8,-1 0.8,1 M -0.8,-1 -0.8,1",
        "M 0.9,-1 0.9,1 M -0.9,-1 -0.9,1","M 1.0,-1 1.0,1 M -1.0,-1 -1.0,1"]
var doubleLine = {
    //path: 'M 0.5,-1 0.5,1 M -0.5,-1 -0.5,1',
    path: "M 1.0,-1 1.0,1 M -1.0,-1 -1.0,1",
    strokeOpacity: 1,
    strokeWeight: 2,
    scale: 3,
    name:  "doubleLine"
};

var lineLT = {
    path: 'M -1.5,0 1.5,0  M 1.0,-1 1.0,1',
    strokeOpacity: 1,
    strokeWeight: 2,
    scale: 3,
    name:  "lineLT"
};

var lineRT = {
    path: 'M -1.5,0 1.5,0  M -1.0,-1 -1.0,1',
    strokeOpacity: 1,
    strokeWeight: 2,
    scale: 3,
    name:  "lineRT"
};
var lineL = {
    path: 'M 1.0,-1 1.0,1' ,
    strokeOpacity: 1,
    strokeWeight: 2,
    scale: 3,
    name:  "lineL"
};

var lineR = {
    path: 'M -1.0,-1 -1.0,1',
    strokeOpacity: 1,
    strokeWeight: 2,
    scale: 3,
    name:  "lineR"
};

// class to contain segment info
function SegmentInfo(SegmentId, routeName, segmentName, oneWay, Color, tracks, dash, routeType, trackUsage )
{
    this.type = "SegmentInfo";
    this.line = null;
    this.path = null;
    this.grayLine = null;
    //this.line.segmentId = SegmentId;
    this.segmentId = SegmentId;
    this.routeName = routeName;
    this.segmentName = segmentName;
    this.oneWay = oneWay;
    this.Color = Color;
    this.arrow = null;
    var points = 0;
    //var newline = this.line;
    //var newGrayLine = this.grayLine;

    //webViewBridge.debug("segment " + SegmentId + " usage: " + trackUsage);
    var icons = [
     [{
          icon: lineSymbol, //0 Single track
          offset: '0%',
          repeat: '6px'
      }],
     [{
          icon: lineSymbol, //1 Incline
          offset: '.5',
          repeat: '15px'
      }],
     [{
          icon: doubleLine, // 2 double track street
          offset: '0%',
          repeat: '6px'
      }],
     [{
          icon: singleTick, // 3 Single track PRW
          offset: '0%',
          repeat: '6px'
     }],
     [{
          icon: doubleTick, //4 Double track PRW
          offset: '0%',
          repeat: '6px'
     }],
     [{
          icon: doubleLine, //5 Subway
          offset: '70%',
          repeat: '10px'
     }]
     [{
          icon: lineL, // 6 left line
          offset: '0%',
          repeat: '6px'
      }],
     [{
          icon: lineR, // 7 right line
          offset: '0%',
          repeat: '6px'
      }],
     [{
         icon: lineLT, // 8 left line
         offset: '0%',
         repeat: '6px'
      }],
     [{
         icon: lineRT, // 9 right line
         offset: '0%',
         repeat: '6px'
      }],
    ];

    var j = 0;  // Single track street
    if(tracks === 2) j=2;   // double track street
    if(tracks===2 && dash ===2 ) j= 4; // double track PRW
    if(tracks ===1 && dash ===2) j= 3; // single track PRW
    if(dash === 1) j = 1; // incline
    if(dash === 3) j = 5; // Subway
    if(trackUsage === "L")
    {
        var iconIx = 7;
        var symbol = [{icon: lineR, // 7 right line
                       offset: '0%',
                       repeat: '6px', strokeColor: color
                      }/*,
                      {icon: lineL, // left line
                       offset: '0%',
                       repeat: '6px',
                       strokeOpacity: .5
                      }*/]
        ;
        if(routeType === 1)
        {// PRW
             iconIx = 9;
             symbol = [{icon: lineRT, // 7 right line
                        offset: '0%',
                        repeat: '6px', strokeColor: color
                       }/*,
                       {icon: lineL, // left line
                        offset: '0%',
                        repeat: '6px',
                       strokeOpacity: .5
                       }*/]
             ;

        }

        this.line = new google.maps.Polyline(
        {
            strokeColor: Color,
            strokeOpacity: 0,
            icons: symbol, //icons[iconIx],
            segmentId: SegmentId,
            zIndex: 100
         });

        this.grayLine = new google.maps.Polyline(
        {
            strokeColor: "#A9A9A9",
            strokeOpacity: 0,
            //strokeWeight: weight,
            icons: [{icon: lineL, // 7 right line
                             offset: '0%',
                             repeat: '6px'
                         }],
            segmentId: SegmentId,
            zIndex: 50
        });
        //webViewBridge.debug("segment= " + SegmentId + " trackUsage = '" + trackUsage  + "' routeType="+ routeType + " icon index=" +iconIx );
    }

    if(trackUsage === "R")
    {
        iconIx = 6;
        symbol = [{icon: lineL, // 7 right line
                   offset: '0%',
                   repeat: '6px', strokeColor: color}/*,
                  {
                   icon: lineR, // left line
                   offset: '0%',
                   repeat: '6px',
                   strokeOpacity: .5}*/
                ];
        if(routeType === 1) // PRW
        {
             iconIx = 8;
            symbol = [{icon: lineLT, // 7 right line
                       offset: '0%',
                       repeat: '6px', strokeColor: color}/*,
                      {
                       icon: lineL, // left line
                       offset: '0%',
                       repeat: '6px',
                       strokeOpacity: .5}*/
                    ];
        }
         this.line = new google.maps.Polyline(
         {
             strokeColor: Color,
             strokeOpacity: 0,
             icons: symbol,//icons[iconIx],
             segmentId: SegmentId, zIndex: 100,
             map: map
        });

        this.grayLine = new google.maps.Polyline(
        {
        strokeColor: "#A9A9A9",
        strokeOpacity: 0,
        //strokeWeight: weight,
        icons: [{icon: lineR, // 7 right line
                             offset: '0%',
                             repeat: '6px'
                         }],
        segmentId: SegmentId,
        zIndex: 50
        });
        webViewBridge.debug("segment= " + SegmentId + " trackUsage =" + trackUsage  + " routeType="+ routeType + " icon index=" +iconIx );

    }

    const symbolThree = {
        path: "M -2,-2 2,2 M 2,-2 -2,2",
        strokeColor: "#292",
        strokeWeight: 4,
      };
    if(trackUsage === 'B' || trackUsage === ' ')
    {
        //console.error("tracks =" + tracks + " index = " + j);
        this.line = new google.maps.Polyline(
        {
            strokeColor: Color,
            strokeOpacity: 0,
            //strokeWeight: weight,
            icons: icons[j],
            segmentId: SegmentId
        });
    }

    this.getPath = function()
    {
        if(this.line)
        {
            this.path = this.line.getPath();
            return this.path;
        }
        return null;
    }
    this.setPath = function(p)
    {
        //setMap(null);
        this.line.setPath(p);
        if(this.grayLine)
           this.grayLine.setPath(p);
        this.placeArrow(p);
        //setMap(map);
    }

    this.placeArrow = function (path)
    {
     //var path = this.getPath();
     if(path.getLength() > 1)
     {
      if(this.arrow)
      {
          this.arrow.setMap(null);
          var poly = this.arrow.getPoly();
          poly = null;
          this.arrow = null;
      }
      //else
      {
      var len = path.getLength();
      var brng = new bearing(path.getAt(len-1).lat(), path.getAt(len-1).lng(),path.getAt(len-2).lat(), path.getAt(len-2).lng() );
      var left = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()-15, .020);
      var right = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()+15, .020);
      this.setArrow(new myArrow(left.lat(), left.lng(), path.getAt(len-1).lat(), path.getAt(len-1).lng(), right.lat(), right.lng(), currSegment.getColor(), currSegment.segmentId));
      }
     }
     return;
    }  // end placeArrow()

    this.getPointArray = function ()
    {
     var path = this.getPath();
     var array = new Array(0,0);
     path.forEach(function(pt, ix)
     {
      array[ix*2] = pt.lat();
      array[(ix*2)+1] = pt.lng();
     });
     //alert("points = " +array.length);
     return array;
    }

    this.getLine = function(){
        return this.line;
    }

    this.getGrayLine = function(){
        return this.grayLine;
    }

    this.insertPoint = function(pt, pos)
    {
        var path = this.line.getPath();
        for(i=0; i < path.getLength()-1; i++)
        {
            begin = path.getAt(i);
            end = path.getAt(i+1);
            bounds = setBounds( begin, end);
            if( bounds.contains(pos))
            {
                webViewBridge.setDebug("Insert " + i);
                path.insertAt(i+1, pos);
                this.setPath(path);
                return true;
            }
        }
        return false;
    }

    this.movePoint = function(pt, latLng)
    {
        this.setMap(null);
        var path = this.getPath();
        path[pt] = latLng;
        this.setPath(path);
        this.setMap(map);
        return path;
    }

    this.deletePoint = function(pt, path)
    {
        var path = this.line.getPath();
        path.removeAt(pt);
        this.setPath(path);
        // move the arrow as well
        this.placeArrow(path);
        return path;
    }

    this.setMap = function(map)
    {
        this.line.setMap(map);
        if(this.grayLine)
         this.grayLine.setMap(map);
        if(this.arrow)
         this.arrow.setMap(map);
    }

    this.addNewPoint = function (e)
    {
        if(bAdding)
        {
            map.disableDoubleClickZoom = true;

            if(this.line === null)
            {
            //OK                    window.external.SetDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
                webViewBridge.setDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
                return;
            }
            var path = this.line.getPath();
            if(path.length === 0)
            {
                addMarker(path.getLength(), e.latLng.lat(), e.latLng.lng(), 1, segment.getInfo(), segment.segmentId);
            }
            var pt =path.push(e.latLng);
            this.setPath(path);
            //getPoints();
            if(path.length > 0)
                // window.external.addPoint();
                window.webViewBridge.addPoint(pt, e.latLng.lat(), e.latLng.lng());
            this.placeArrow(path);
        }
        bAdding = false
    }

    this.getArrow = function(){
//        if(this.arrow)
//        {
//        //alert("getArrow " + Arrow.getInfo());
//        }
        return this.arrow;
    }

    this.getColor = function (){
        return Color;
    }

    this.getInfo = function () {
        return this.segmentName + " route:" + this.routeName;
    }

    this.setArrow = function (Arrow){
        this.arrow = Arrow;
    }

    var info=this.segmentName + " route" + this.routeName;

    // function to determine if the supplied point is on a begining or end linesegement of a segment
    this.isPointOnEnd = function(pt)
    {
        var line =newline;
        var path = this.line.getPath();
        var len = path.getLength();
        webViewBridge.setLen(len);
        var i;
        var mIx = 1;
        var b1 = bearing(pt.lat(), pt.lng(), path.getAt(0).lat(), path.getAt(0).lng());
        if(b1.getDistance() < .020)
          return 0;
        var b2 =  bearing(pt.lat(), pt.lng(), path.getAt(len-1).lat(), path.getAt(len-1).lng());
        if(b2.getDistance() < .020)
          return len-1;
        //alert("segment " + SegmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());
        webViewBridge.setDebug("segment " + SegmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());

        return -1;
    }

    // function to see if a point is on the line
    this.isPointOnLine = function(pt)
    {
      //var poly = new google.maps.PolyLine({path: this.getPath});
      rslt = google.maps.geometry.poly.isLocationOnEdge(point, this.line, 10e-1);
      webViewBridge.debug("is point on " + this.segmentId + "result =" + rslt);
    }

    // events
    google.maps.event.addListener(map, "mousemove", function(e){
        if(bAdding)
            map.setOptions({draggableCursor:'Crosshair'});
    });

    // Select segment (click)
    google.maps.event.addListener(this.line, "mouseover", function(e){
        hiLiteLine(this.segmentId);
    });

    google.maps.event.addListener(this.line, "mouseout", function(e){
        restoreLine();
    });

    google.maps.event.addListener(this.line, "click", function(e){
        si = hiLiteSelectedLine(this.segmentId);
        webViewBridge.setDebug("sId = " + si.segmentId + " " + si.segmentName);

        var path = si.line.getPath();
        var len = path.getLength();
        var begin, end,bounds;
        webViewBridge.setLen(len);
        var i;
        var mIx = 1;
        for(i=0; i < path.getLength()-1; i++)
        {
          begin = path.getAt(i);
          end = path.getAt(i+1);
          bounds = setBounds( begin, end);
          if( bounds.contains(e.latLng)){
              break;
          }
        }
        if(i>0)
          mIx=0;
        //addMarker(i, e.latLng.lat(), e.latLng.lng(), mIx, segmentName + " route:" + routeName);
        addMarker(i, begin.lat(), begin.lng(), mIx, si.segmentName + " route:" + si.routeName, si.segmentId);
        //OK            window.external.selectSegment(i, SegmentId);
        webViewBridge.selectSegment(i, si.segmentId);
        addModeOff();
    });

    // right click to add a point
    //google.maps.event.addListener(this.line, "rightclick", function(e)
    this.line.addListener("contextmenu", function(e)
    {
        var si;
        si = hiLiteSelectedLine(this.segmentId);
        var path = si.getPath();
        var len = path.getLength();

        webViewBridge.selectSegment(si.segmentId);
        webViewBridge.setLen(len);
        var i;
        var mIx = 1;
        for(i=0; i < path.getLength()-1; i++)
        {
            begin = path.getAt(i);
            end = path.getAt(i+1);
            bounds = setBounds( begin, end);
            if( bounds.contains(e.latLng))
            {
                break;
            }
        }
        si.insertPoint(i, e.latLng); // insert point after i
        webViewBridge.insertPoint(si.segmentId, i, e.latLng.lat(), e.latLng.lng());
        webViewBridge.selectSegment(i+1, si.segmentId);

        addModeOff();
        if(i>0)
          mIx=0;
        addMarker(i+1, e.latLng.lat(), e.latLng.lng(), mIx, si.segmentName + " route:" + si.routeName, si.segmentId);
    });

//    if(this.grayLine)
//        google.maps.event.addListener(this.line, "click", function(e){
//            webViewBridge.setDebug("sId = " + SegmentId + " " + segmentName);
//            si = hiLiteSelectedLine(segmentId);

////            var path = grayLine.getPath();
//            var len = path.getLength();
//            var begin, end,bounds;
//            webViewBridge.setLen(len);
//            var i;
//            var mIx = 1;
//            for(i=0; i < path.getLength()-1; i++)
//            {
//              begin = path.getAt(i);
//              end = path.getAt(i+1);
//              bounds = setBounds( begin, end);
//              if( bounds.contains(e.latLng)){
//                  break;
//              }
//            }
//            if(i>0)
//              mIx=0;
//            //addMarker(i, e.latLng.lat(), e.latLng.lng(), mIx, segmentName + " route:" + routeName);
////            addMarker(i, begin.lat(), begin.lng(), mIx, segmentName + " route:" + routeName, SegmentId);
//            //OK            window.external.selectSegment(i, SegmentId);
//            webViewBridge.selectSegment(i, SegmentId);
//            addModeOff();
//        });



    // doubleclick to add a station (stop)
//    google.maps.event.addListener(this.line, "dblclick", function(e)
//    {
//     //line = newline;
//     Arrow = arrow;
//     color = Color;
//     var path = line.getPath();
//     var i;
//     for(i=0; i < path.getLength()-1; i++)
//     {
//         begin = path.getAt(i);
//         end = path.getAt(i+1);
//         bounds = setBounds( begin, end);
//         if( bounds.contains(e.latLng)){
//             break;
//         }
//     }
//     //            window.external.setStation(e.latLng.lat(), e.latLng.lng(), SegmentId, i);
//     webViewBridge.setStation(e.latLng.lat(), e.latLng.lng(), SegmentId, i);
//    });
//    google.maps.event.addListener(map, "dblclick", function(e){
//        if(currSegment)
//        {
//            bAdding = true;
//            //currSegment.addNewPoint(e);
//            var line = currSegment.line;
//            if(bAdding)
//            {
//                //map.disableDoubleClickZoom = true;

//                if(line === null)
//                {
//                //OK                    window.external.SetDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
//                    webViewBridge.setDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
//                    return;
//                }
//                var path = line.getPath();
////                if(path.length === 0)
////                {
////                    addMarker(path.getLength(), e.latLng.lat(), e.latLng.lng(), 1, segment.getInfo(), segment.segmentId);
////                }
//                var pt =path.push(e.latLng);
//                currSegment.setPath(path);
//                //getPoints();
//                if(path.length > 0)
//                    // window.external.addPoint();
//                    window.webViewBridge.addPoint(pt, e.latLng.lat(), e.latLng.lng());
//                currSegment.placeArrow(path);
//            }}
//     });

} // end SegmentInfo



async function initMap() {
    const { Map } = await google.maps.importLibrary("maps");
    if(bGoogleInit)
        return;

     console.log("begin GoogleMaps.js initMap()");
     webViewBridge.debug("initMap started");
     connectSlots();
     geocoder  = new google.maps.Geocoder();

     //var Lat = 52.0;
     var Lat = webViewBridge.lat;
     //var Lon = 13.0;
     var Lon = webViewBridge.lng;
     //var zoom = 13;
     var zoom = webViewBridge.zoom;
     //var mapTypeId = google.maps.MapTypeId.ROADMAP;
     var mapTypeId = webViewBridge.maptype;
     var mapDiv = document.getElementById("map");


     map = new Map(mapDiv, {
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

//    initMap2();
//}

//function initMap2()
//{
//    webViewBridge.debug("initMap2 called");
    //geocoder  = new google.maps.Geocoder();
    //stationArray = new google.maps.MVCArray();
    //siArray = new google.maps.MVCArray();

    osm_MapType = new google.maps.ImageMapType(
    {
     getTileUrl: Get_osm_MapType ,
     tileSize: new google.maps.Size(256, 256),
     isPng: true,
     alt: "OpenStreetMap layer",
     name: "OpenStreetMap",
     maxZoom: 18
    });

    google.maps.event.addListenerOnce(map, 'idle', function(){
        //this part runs when the mapobject is created and rendered
        google.maps.event.addListenerOnce(map, 'idle', function(){
            //this part runs when the mapobject shown for the first time
            webViewBridge.mapInit();
        });
    });

    google.maps.event.addListener(mapDiv, 'resize', function(){
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
      map.setOptions( { styles: styles["hide"] })
     map.setMapTypeId(mapTypeId);
     stationArray = new google.maps.MVCArray();

     //google.maps.event.trigger(map, 'resize');


     webViewBridge.queryOverlay();

     //OK    window.external.displayZoom(map.getZoom());
     webViewBridge.displayZoom(map.getZoom());


    function latLng2Point(latLng, map) {
      var topRight = map.getProjection().fromLatLngToPoint(map.getBounds().getNorthEast());
      var bottomLeft = map.getProjection().fromLatLngToPoint(map.getBounds().getSouthWest());
      var scale = Math.pow(2, map.getZoom());
      var worldPoint = map.getProjection().fromLatLngToPoint(latLng);
      return new google.maps.Point((worldPoint.x - bottomLeft.x) * scale, (worldPoint.y - topRight.y) * scale);
    }
    function point2LatLng(point, map) {
      var topRight = map.getProjection().fromLatLngToPoint(map.getBounds().getNorthEast());
      var bottomLeft = map.getProjection().fromLatLngToPoint(map.getBounds().getSouthWest());
      var scale = Math.pow(2, map.getZoom());
      var worldPoint = new google.maps.Point(point.x / scale + bottomLeft.x, point.y / scale + topRight.y);
      return map.getProjection().fromPointToLatLng(worldPoint);
    }

//    var menuDisplayed = false;
//    var menuBox = null;
//    var contextMenuLatLng;
//    window.addEventListener("contextmenu", function() {
//            var left = arguments[0].clientX;
//            var top = arguments[0].clientY;

//            menuBox = document.getElementById("menu");
//            menuBox.style.left = left + "px";
//            menuBox.style.top = top + "px";
//            menuBox.style.display = "block";

//        latLng = point2LatLng(arguments[0], map);
//        if(findLine(latLng)!== null)
//        {
//            arguments[0].preventDefault();
//            return;
//        }
//        contextMenuLatLng= latLng.lat().toFixed(6) + "," + latLng.lng().toFixed(6);
//        menuBox.textContent =contextMenuLatLng
//            arguments[0].preventDefault();

//            menuDisplayed = true;
//        }, false);
//        window.addEventListener("click", function() {
//            if(menuDisplayed == true){
//                menuBox.style.display = "none";
//                webViewBridge.rightClicked(contextMenuLatLng);
//            }
//        }, true);

    google.maps.event.addListener(map, "zoom_changed", function() {
     webViewBridge.displayZoom(map.getZoom());
    });
    zoomIx = map.getZoom()-17;
    zoomOffset = offsets[zoomIx];

    webViewBridge.initialized();
    bGoogleInit = true;
    webViewBridge.debug("initMap complete");
} // end initMap2()


const styles = {
  default: [],
  hide: [
    {
      featureType: "poi",
      stylers: [{ visibility: "off" }],
    },
    {
      featureType: "transit",
      elementType: "labels.icon",
      stylers: [{ visibility: "off" }],
    },
  ],
};

window.initialize = function() // called by WebChannel .ie "onLoad()"
{
    //try {

    initMap();
    // }
    // catch(err)
    // {
    //     //console.log(err);
    // }

//    google.maps.event.addListener(map, "zoom_changed", function() {
//     webViewBridge.displayZoom(map.getZoom());
//    });
//    zoomIx = map.getZoom()-17;
//    zoomOffset = offsets[zoomIx];
//    //alert("zoomOffset = " + zoomOffset);

}

function resizeMap()
{
//google.maps.event.trigger(map, 'resize');
var mapDiv = document.getElementById("map");
}

var arrowSymbol  = {
       path: 'M 0,-4 -3,-4 0,0 3,-4'
   };

function createSegment(segmentId, routeName, segmentName, oneWay, color, tracks, dash, routeType, trackUsage, points )
{
    webViewBridge.setDebug("SegmentId "+ segmentId + "usage: "+ trackUsage);
    var linePath = new google.maps.MVCArray();
    for(var i=0; i < points; i +=2)
    {
        linePath.push(new google.maps.LatLng(arguments[i+10], arguments[i+11]));
    }

    newSegment = new SegmentInfo(segmentId, routeName, segmentName, oneWay, color, tracks, dash, routeType, trackUsage );
    // if(weight !== 0 && weight !==3)
    //  alert(newSegment.getInfo() + " args=" + arguments.length);
    line = newSegment.getLine();
    line.setMap(map);
    grayLine = newSegment.getGrayLine();
    if(grayLine)
        grayLine.setMap(map);
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
    if(arguments.length > 8)
    {
        //alert(points + " " + arguments.length);
        var path = line.getPath();

        for(var i=0; i < points; i +=2)
        {
            path.push(new google.maps.LatLng(arguments[i+10], arguments[i+11]));
        }
        if(grayLine)
        {
            var grayPath = grayLine.getPath();
            webViewBridge.debug("grayLine: segmentid="+grayLine.segmentId+ " path = "+ grayPath);
            for(var i=0; i < points; i +=2)
            {
                grayPath.push(new google.maps.LatLng(arguments[i+10], arguments[i+11]));
            }
        }

        //getPoints();
        newSegment.placeArrow(path);
    }
    if(typeof points == 'array')
    {
        path = line.getPath();

        for( i=0; i < points.length; i +=2)
        {
          path.push(new google.maps.LatLng(points[i], points[i+1]));
        }
        if(grayLine)
        {
            grayPath = grayLine.getPath();
            webViewBridge.debug("grayLine: segmentid="+grayLine.segmentId+ " path = "+ grayPath);
            for(i=0; i < points; i +=2)
            {
                grayPath.push(new google.maps.LatLng(arguments[i+10], arguments[i+11]));
            }
        }
        //getPoints();
        newSegment.placeArrow(path);
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
  selectedLine.line.setOptions({strokeColor: selectedLine.color});
  if(selectedLine.grayLine)
      selectedLine.grayLine.setOptions({strokeColor: selectedLine.grayColor});
  if(selectedLine.arrow)
  {
      selectedLine.arrow.setOptions({strokeColor: selectedLine.color, fillColor:selectedLine.color });
  }
  selectedLine = null;
 }
 return null;
}

function hiLiteSelectedLine(segmentId)
{
    var line;
    var grayLine;
    var arrow;
    var selectedSi;

    siArray.forEach(function(si, ix)
    {
        if(si.segmentId !== null && si.segmentId === segmentId)
        {
            line = si.line;
            grayline = si.grayLine;
            arrow = si.arrow;
            selectedSi = si;
        }
    });
    if(selectedLine != null)
    {
        restoreSelectedLine();
    }
    var color = line.strokeColor;
    line.setOptions({strokeColor: "#04b4B4"});
    var grayLineClr;
    if(grayLine)
    {
        grayLineClr = grayLine.strokeColor;
        grayLine.setOptions({strokeColor: "#dedede"});
    }

    if(arrow)
    {
        arrow.setOptions({strokeColor: "#04b4B4", fillColor:"#04b4B4" });
    }
    selectedLine = {si: selectedSi, segmentId: segmentId, line: line, arrow: arrow, grayLine: grayLine,
     color: color, grayColor: grayLineClr};
    return selectedSi;
}

function restoreLine()
{
    if(hiLitedSegment != null)
    {
      //selectedLine.breakpt();
        hiLitedSegment.line.setOptions({strokeColor: hiLitedSegment.color});
        if(hiLitedSegment.grayLine)
            hiLitedSegment.grayLine.setOptions({strokeColor: hiLitedSegment.grayColor});
        if(hiLitedSegment.arrow)
        {
          hiLitedSegment.arrow.setOptions({strokeColor: hiLitedSegment.color, fillColor:hiLitedSegment.color });
        }
        map.setOptions({draggableCursor:''});
        hiLitedSegment = null;
     }
     return null;
}

function hiLiteLine(segmentId)
{
    var line;
    var grayLine;
    var arrow;

    siArray.forEach(function(si, ix)
    {
        if(si.segmentId !== null && si.segmentId === segmentId)
        {
            line = si.line;
            grayline = si.grayLine;
            arrow = si.arrow;
        }
    });
    var color = line.strokeColor;
    if(hiLitedSegment != null)
    {
        restoreLine();
    }
    line.setOptions({strokeColor: "#04b4B4", cursor:'Crosshair'}) ;
    var grayLineClr;
    if(grayLine)
    {
        grayLineClr = grayLine.strokeColor;
        grayLine.setOptions({strokeColor: "#dedede"});
    }

    if(arrow)
    {
        var poly;
        poly = arrow.getPoly();
        poly.setOptions({strokeColor: "#04b4B4", fillColor:"#04b4B4" });
        selectedPoly = arrow.getPoly();
    }

    hiLitedSegment = {segmentId: segmentId, line: line, arrow: arrow, grayLine: grayLine,
                      color: color, grayColor: grayLineClr};
    return null;
}

function addModeOn(segmentId)
{
    currentSegment = getSegmentInfo(segmentId);
    bAdding = true;
    map.setOptions({draggableCursor:'Crosshair'});
    webViewBridge.addPointMode(bAdding);
    return null;
}

function addModeOff()
{
    bAdding = false;
    map.setOptions({draggableCursor:'default'});
    webViewBridge.addPointMode(bAdding);
    currentSegment = null;
    return null;
}

function isAddModeOn()
{
 if(bAdding)
  return "true"
 else
  return "false";
}

function addNewPoint(e)
{
    if(bAdding)
    {
        if(!currSegment)
            return;
        var line = currSegment.line;

        if(line === null)
        {
            //OK                    window.external.SetDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
            webViewBridge.setDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
            return;
        }
        var path = line.getPath();
        if(path.getLength() === 0)
        {
            addMarker(path.getLength(), e.latLng.lat(), e.latLng.lng(), 1, segment.getInfo(), segment.segmentId);
        }
        path.push(e.latLng);
        //getPoints();
        if(path.getLength() > 0)
            // window.external.addPoint();
            webViewBridge.addPoint(0, e.latLng.lat(), e.latLng.lng());
        currSegment.placeArrow(path);
    }
}


//  function displayPoint( lat, lon)
//  {
//      var path = line.getPath();
//      path.push(new google.maps.LatLng(lat, lon));
//      //getPoints();
//      placeArrow(path);
//      return;
//  }

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
            hiLiteSelectedLine(si.segmentId);
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
          line = null;

          grayLine = si.getGrayLine();
          if(grayLine)
          {

              grayLine.setMap(null);
              path = grayLine.getPath();
              while(path.getLength() > 0)
              {
                  path.pop();
              }
              grayLine.setPath(path);
              grayLine = null;
          }

          //siArray.removeAt(ix);
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

    if(stationArray)
    {
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
    }
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
    return;
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
              si.setArrow (  myArrow(lLat, lLon, mLat, mLon, rLat, rLon, si.getColor(), segmentId));
          }
      }
      catch (err)
      {
          txt=err;
      }

  });
 return;
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

      grayLine = si.getGrayLine();
      if(grayLine)
      {
          grayLine.setMap(null);
          path = grayLine.getPath();
          while(path.getLength() > 0)
          {
              path.pop();
          }
          grayLine.setPath(path);
          grayLine = null;
      }
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
      if(infowindow.marker !== null)
          infowindow.marker.setMap();
      infowindow = null;
  }

  selectedLine = null;

  if(stationArray)
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
  bounds = setBounds( begin, end);
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
 return;
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
          bounds = setBounds( begin, end);
          if( bounds.contains(e.latLng))
          {
//                    window.external.SetDebug(i + " editSegment");
//                    window.external.editSegment(i);
              break;
          }
      }
      return null;
  }


function deletePoint(pt, segmentId)
{
    var si = getSegmentInfo(segmentId);
    si.deletePoint(pt);
}

//function placeArrow(path, segment)
//{
// if(path.getLength() > 1)
// {
//  arrow =currSegment.getArrow();
//  if(arrow)
//  {
//      arrow.setMap(null);
//      var poly = arrow.getPoly();
//      poly = null;
//      arrow = null;
//  }
//  //else
//  {
//  var len = path.getLength();
//  var brng = new bearing(path.getAt(len-1).lat(), path.getAt(len-1).lng(),path.getAt(len-2).lat(), path.getAt(len-2).lng() );
//  var left = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()-15, .020);
//  var right = pointRadialDistance( new google.maps.LatLng(path.getAt(len-1).lat(), path.getAt(len-1).lng()), brng.getBearing()+15, .020);
//  currSegment.setArrow(new myArrow(left.lat(), left.lng(), path.getAt(len-1).lat(), path.getAt(len-1).lng(), right.lat(), right.lng(), currSegment.getColor(), currSegment.segmentId));
//  }
// }
// return;
//}  // end placeArrow()

//function getPoints()
//{
//    var path = line.getPath();
//    var len = path.getLength();
//    path.forEach(setArray);
//    return;
//}

//var pointArray = [];
//function setArray(element, number)
//{
// var pt = [element.lat(), element.lng()];
// pointArray[number]= pt;
//    return;
//}

//function getLen()
//{
// var len =-1;
// if(line == null)
//  len = -1;
// else
// {
//  var path = line.getPath();
//   len = path.getLength();
// }
// return len;
//}

//function getPointValues(i)
//{
// //alert("getPointValues: " + i);
// var path = line.getPath();
// var myElement = path.getAt(i);
// return i+","+myElement.lat()+","+myElement.lng();
//}

function getSegmentInfo(segmentId)
{
    if(selectedLine && selectedLine.segmentId == segmentId)
        return selectedLine.si;
    var si;
    for(let i =0; i < siArray.length; i++)
    {
        si = siArray.getAt(i);
        if(si.segmentId == segmentId)
        {
            break;
        }
    }
    if(!si)
        console.error("segmentId " + segmentId + " is invalid");
    return si;
}

// Return the entire array of points for the current line
function getPointArray(segmentId)
{
    var si = getSegmentInfo(segmentId);
    //var line = si.line;
    //var path = si.line.getPath();
    var siArray = si.getPointArray();
//    var array = new Array(0,0);
//    path.forEach(function(pt, ix)
//    {
//        array[ix*2] = pt.lat();
//        array[(ix*2)+1] = pt.lng();
//    });
//    //alert("points = " +array.length);
    return siArray;
}


var circle;
function addMarker(i, lat, lon, icon, text, SegmentId)
{
 console.log("addMarker "+  i + " " + lat+ " " + lon+ " " + icon+ " " + text+ " " + SegmentId);
 segmentId = SegmentId;
 siArray.forEach(function(si, ix)
 {
  if(si.segmentId && si.segmentId === segmentId)
  {
      currSegment = si;
  }
 });
 var line = currSegment.line;
 var grayLine = currSegment.grayLine;
 var arrow = currSegment.arrow;
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

 google.maps.event.addListener(marker, "drag", function(pt) {
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  var path = line.getPath();
  path.setAt(i,  pt.latLng );
  if(grayLine)
    grayLine.setPath(path);
  if(arrow)
      arrow.setMap(null);
 });

 google.maps.event.addListener(marker, "dragend", function(pt) {
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  var path = line.getPath();
  path.setAt(i,  pt.latLng );
  line.setPath(path);

  // move the arrow as well
  //placeArrow(path);
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
          var bounds = setBounds( begin, end);
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
 return;
}

  function setBounds( pt1, pt2)
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

  function findLine(point)
  {
      siArray.forEach(function(si, ix)
      {
          if(si.isPointOnLine(point)) {
              webViewBridge.debug("findLine is " + si.getSegment());
              return si;
          }
      });
      webViewBridge.debug("findLine is null");
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
 return;
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
      case "rail":
          icon = image[images.rail];
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
    if(!stationArray)
        return;
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
  if(element && element != 'undefined' && element.stationKey === stationKey)
  {
   //alert("icontype = " + element.typeIcon);
   rVal =  element.typeIcon;
   return rVal;
  }
 });
 return rVal;
}

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
 return;
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
 return;
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
      return;
  }

  // showLocation() is called when you click on the Search button
  // in the form.  It geocodes the address entered into the form
  // and adds a marker to the map at that location.
  function showLocation(address) {
    //var address = document.geocoderForm.address.value;
    geocoder.geocode({'address':address}, addAddressToMap);
      return;
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
 return;

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
 return;
}


// Option to display RouteComments
function showRouteComment(bDisplay)
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
   infowindow.marker.setMap();
  }
 }
 return;
}

function displayRouteComment(latitude, longitude, HTMLText, route, date, companyKey)
{
 this.marker = null;
 if(infowindow !== null)
 {
  infowindow.setMap();
  infowindow = null;
 }
 if(latitude === 0 && longitude === 0)
 {
  latitude = map.getCenter().lat;
  longitude = map.getCenter().lon;
 }

 // infowindow = new google.maps.InfoWindow({content:date+HTMLText, position:new google.maps.LatLng(lat, lon)}, 'return 0');
 infowindow = new google.maps.InfoWindow({content:HTMLText, maxWidth: 300});
 //var icon = image[images.greenDownArrow];
 this.marker = new google.maps.Marker({
       position: new google.maps.LatLng(latitude, longitude),
       map: map,
       icon: image[images.greenDownArrow],
       zIndex: 10,
       draggable: true,
       visible: true,
       title: "comment"
     });
  if(this.marker === null)
      alert("marker is null");
 //infowindow.setMap(map);
 infowindow.marker = this.marker;
 infowindow.route = route;
 infowindow.date = date;
 infowindow.lat = latitude;
 infowindow.lon = longitude;
 infowindow.companyKey = companyKey

 google.maps.event.addListener(this.marker, "dragend", function(pt) {
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  webViewBridge.moveRouteComment(infowindow.route, infowindow.date, pt.latLng.lat(), pt.latLng.lng(), infowindow.companyKey);
 })
 google.maps.event.addListener(infowindow, "closeclick", function(){
     this.marker.setVisible(false);
     this.marker.setMap();
     this.marker = null;
 })
 infowindow.open(map, this.marker);
 // })
 return 0;
}

function nextRouteComment()
{
 infowindow.marker.setMap();
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, 1);
 return 0;
}

function prevRouteComment()
{
 infowindow.marker.setMap();
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, -1);
}

function setDefaultOptions()
{
 //alert("setDefaultOptions");
 map.setOptions(defaultOptions);
 //map.setOptions({ styles: styles["default"] });
 myRslt = 0;
}

function setOptions()
{
 //alert("setDefaultOptions");
 map.setOptions(options);
 //map.setOptions({ styles: styles["hide"] });
}

function setOption(option)
{
 map.setOptions(option);
}

function isOverlayLoaded()
{
 if(overlay == null)
    return "false";
 return "true";
}

/**
 * Creates a control that recenters the map on Chicago.
 */
function createCenterControl(map) {
  const controlButton = document.createElement("button");

  // Set CSS for the control.
  controlButton.style.backgroundColor = "#fff";
  controlButton.style.border = "2px solid #fff";
  controlButton.style.borderRadius = "3px";
  controlButton.style.boxShadow = "0 2px 6px rgba(0,0,0,.3)";
  controlButton.style.color = "rgb(25,25,25)";
  controlButton.style.cursor = "pointer";
  controlButton.style.fontFamily = "Roboto,Arial,sans-serif";
  controlButton.style.fontSize = "16px";
  controlButton.style.lineHeight = "38px";
  controlButton.style.margin = "8px 0 22px";
  controlButton.style.padding = "0 5px";
  controlButton.style.textAlign = "center";

  controlButton.textContent = "Set City Bounds";
  controlButton.title = "Click to save the bounds";
  controlButton.type = "button";

  // Setup the click event listeners: simply set the map to Chicago.
  controlButton.addEventListener("click", () => {
    //map.setCenter(chicago);
    var bounds =  map.getBounds();
    var ne = bounds.getNorthEast();
    var sw = bounds.getSouthWest();
    webViewBridge.cityBounds( ne.lat(), ne.lng(), sw.lat(), sw.lng());
  });

  return controlButton;
}
function addCityBoundsButton()
{
    // Create the DIV to hold the control.
      const centerControlDiv = document.createElement("div");
      // Create the control.
      const centerControl = createCenterControl(map);
      // Append the control to the DIV.
      centerControlDiv.appendChild(centerControl);
      if(map.controls[google.maps.ControlPosition.BOTTOM_CENTER].length ===0)
          map.controls[google.maps.ControlPosition.BOTTOM_CENTER].push(centerControlDiv);
}

function closeCityBoundsButton()
{
    if(map.controls[google.maps.ControlPosition.BOTTOM_CENTER].length>0)
        map.controls[google.maps.ControlPosition.BOTTOM_CENTER].removeAt(0);
}

function getCurrBounds()
{
    var bounds =  map.getBounds();
    var ne = bounds.getNorthEast();
    var sw = bounds.getSouthWest();
    webViewBridge.cityBounds( ne.lat(), ne.lng(), sw.lat(), sw.lng());

}

function downloadFile(url, fileName) {
  fetch(url, { method: 'get', mode: 'no-cors', referrerPolicy: 'no-referrer' })
    .then(res => res.blob())
    .then(res => {
      const aElement = document.createElement('a');
      aElement.setAttribute('download', fileName);
      const href = URL.createObjectURL(res);
      aElement.href = href;
      aElement.setAttribute('target', '_blank');
      aElement.click();
      URL.revokeObjectURL(href);
    });
};

function screenshot()
{
      html2canvas(document.body, {
      allowTait: true,
      useCORS: true
      }).then(function(canvas) {

            const base64image = canvas.toDataURL("image/png");
            webViewBridge.screenshot(base64image);
    });
}

function alertClose()
{
    alert("you may now close this window");
}

console.log("GoogleMaps.js loaded!");

onLoad();
