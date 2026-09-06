var siArray;
var fRslt;
var mapDiv;
var myRect = null;
var marker = null;
var pinArray = null;
var pins = null;
var pinMarker =null;
var markerPins = null;
var stationArray = [];
//var map = null;
var bAdding = false;
var selectedLine = null;
var hiLitedSegment = null;
var rtStartMarker= null;
var rtEndMarker = null;
var infowindow = null;
var currSegment = null;
var poly2 = null;
let map;
var newSegment, segmentId, arrow,lat=0,lon=0;

var image = ["https://maps.google.com/mapfiles/marker.png",
  "https://maps.google.com/mapfiles/dd-start.png",
  "https://maps.google.com/mapfiles/dd-end.png",
  "redblank.png",
  "https://www.google.com/mapfiles/arrow.png",
  "https://www.google.com/mapfiles/arrowshadow.png",
  "https://maps.google.com/mapfiles/kml/paddle/grn-blank-lv.png",
  "https://maps.google.com/mapfiles/kml/paddle/blu-blank.png",
  "https://maps.google.com/mapfiles/kml/paddle/pink-blank.png",
  "https://maps.google.com/mapfiles/shadow50.png",
  "sl-metro-logo.svg",
  "Strassenbahn-Haltestelle.svg",
  "https://maps.google.com/mapfiles/kml/paddle/ylw-blank.png",
  "S-Bahn-Logo.svg",
  "U-Bahn.svg",
  "tram.png",
  "http:ubuntu-2/public/map_tiles/tram.shadow.png",
  "https://maps.google.com/mapfiles/kml/paddle/wht-blank.png",
  "http:ubuntu-2/public/map_tiles/blue-red-blank.png",
  "https://maps.google.com/mapfiles/kml/paddle/orange-blank.png",
  "http:ubuntu-2/public/map_tiles/BVGTram.png",
  "http:ubuntu-2/public/map_tiles/subway.png",
  "http:ubuntu-2/public/map_tiles/subway.shadow.png",
  "https://maps.google.com/mapfiles/kml/paddle/purple-blank.png",
  "https://maps.google.com/mapfiles/kml/pal5/icon63l.png", // "H"
  "https://maps.google.com/mapfiles/kml/shapes/bus.png",
];
var images = {"default":0, "start":1, "end":2, "shadow":3, "arrow":4, "arrowShadow":5,
  "smallGreen":6, "smallBlue":7,"smallRed":8,"smallShadow":9,"slmetro":10,"haltestelle":11,
  "smallYellow":12,"sbahn":13, "ubahn":14, "tram":15, "tramshadow":16, "smallWhite":17,
  "blue-red":18,"orange":19, "bvgtram":20, "subway":21, "subwayshadow":22, "purple":23,
  "rail":24, "bus":25};

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
   //console.error("Error ocurred calling " + func + " '"+ parms + "'\n" + txt);
    webViewBridge.debug("Error ocurred calling " + func + " '"+ parms + "'\n" + txt);
   //console.trace("trace");
  }
}

window.initialize = function() // called by WebChannel .ie "onLoad()"
{
  initMap();
}

function initMap()
{
    console.log("begin GoogleMaps.js initMap()");
    webViewBridge.debug("initMap started");
    connectSlots();

  L.mapquest.key = MapQuestKey;
  var Lat = webViewBridge.lat;
  var Lon = webViewBridge.lng;
  var zoom = webViewBridge.zoom;
  var mapTypeId = webViewBridge.maptype;
  var mapDiv = document.getElementById("map");
   map = L.mapquest.map('map', {
    center: [Lat, Lon],
    layers: L.mapquest.tileLayer('map'),
    zoom: zoom,
    doubleClickZoom: false
  });

  map.addControl(L.mapquest.control());

   webViewBridge.queryOverlay();

  webViewBridge.displayZoom(map.getZoom());

  map.on("zoomend", function() {
   webViewBridge.displayZoom(map.getZoom());
  });

  map.on( "contextmenu", function(event) {
    // Prevent the browser's default right-click context menu from opening
    L.DomEvent.preventDefault(event);

    webViewBridge.rightClicked(event.latlng.lat, event.latlng.lng);
  });

  map.on( "click", function(event) {
   webViewBridge.clickPoint(event.latlng.lat, event.latlng.lng);
  });

    siArray = [];
  var idleTimeout;
  var idleDelay = 500; // Time in milliseconds
  // Function that runs when the map becomes idle
  // function onMapIdle() {
  //     console.log('Map is idle! Current bounds:', map.getBounds());
  //     // Add your logic here (e.g., fetch markers via AJAX)
  //   webViewBridge.mapInit();
  //   webViewBridge.debug("initMap complete");}

  // // Listen for movement and zoom events
  // map.on('moveend zoomend', function() {
  //     // Clear the previous timeout if the user keeps moving
  //     clearTimeout(idleTimeout);

  //     // Set a new timeout
  //     idleTimeout = setTimeout(onMapIdle, idleDelay);
  // });

    webViewBridge.mapInit();
} // end initMap


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
      //alert("Error occured calling " + func + "\n" + call+"\n"+txt);
    webViewBridge.debug("Error occurred calling " + func + "\n" + call+"\n"+txt);
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
  //map.setCenter(L.latLng(Lat, Lon));
  map.panTo([Lat,Lon]);
 // map.setOptions({disableDoubleClickZoom: true });
 return null;

}

function getCenter()
{
 var latLng = map.getCenter();
 webViewBridge.setCenter(latLng.lat, latLng.lng, map.getZoom(), /*map.getMapTypeId()*/"");
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

var circle;
function addMarker(i, lat, lon, icon, text, SegmentId)
{
 console.log("addMarker "+  i + " " + lat+ " " + lon+ " " + icon+ " " + text+ " " + SegmentId);
 segmentId = SegmentId;
  var position = L.latLng(lat,lon);

 //siArray.forEach(function(si, ix)
 for(const [ix,si] of siArray.entries())
 {
  if(si.segmentId === segmentId)
  {
      currSegment = si;
      break;
  }
 };
 // var line = currSegment.line;
 // var grayLine = currSegment.grayLine;
 // var arrow = currSegment.arrow;
 if(marker)
 {
  marker.remove();
  marker = null;
 }
 //marker.brkpt();
 webViewBridge.setDebug("add marker at lat: " + lat + " lon: " + lon + " point: " + i);
 //window.external.showSegmentsAtPoint(lat,lon);
 webViewBridge.showSegmentsAtPoint(lat,lon, SegmentId);
 if(typeof icon == "number")
 {
     if(icon === -1) // use default icon
     {
         var myCustomIcon = L.icon({
            iconUrl: 'https://assets.mapquestapi.com/icon/v2/marker-red.png', // Your URL here
            shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
            iconSize: [25, 41],
            iconAnchor: [12, 41],
            popupAnchor: [1, -34],
            shadowSize: [41, 41]
         });

         // marker = new google.maps.marker.AdvancedMarkerElement({map: map, position: new google.maps.LatLng(lat, lon),
         //         gmpDraggable: true,  content: pin.element});
       marker = L.marker([lat,lon],{
                           draggable: true,
                           icon: myCustomIcon}).addTo(map);
     }
     else
     {
         // var pin = null;
         // switch(icon)
         // {
         // case 0:
         //     pin = new google.maps.marker.PinElement();
         //     break;
         // case 1:
         //     pin = new google.maps.marker.PinElement({background: "#00FF00", glyph:"0"});
         //     break;
         // case 2:
         //     pin = new google.maps.marker.PinElement({background: "#FF0000" });
         // }
         var iconUrl = 'https://assets.mapquestapi.com/icon/v2/marker-start.png';
         if(text.startsWith("point"))
         {
            let pt = text.replace("point", "");
             if(pt === "0")
                 iconUrl = 'https://assets.mapquestapi.com/icon/v2/marker-start.png';
             else
                 iconUrl = 'https://assets.mapquestapi.com/icon/v2/marker-' + pt + '.png';
         }
         else
             iconUrl = 'https://assets.mapquestapi.com/icon/v2/marker-blue.png'
         var myCustomIcon2 = L.icon({
             iconUrl: iconUrl, // Your URL here
             shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
             iconSize: [25, 41],
             iconAnchor: [12, 41],
             popupAnchor: [1, -34],
             shadowSize: [41, 41]
         });

         // marker = new google.maps.marker.AdvancedMarkerElement({map: map, position: new google.maps.LatLng(lat, lon),
         //   gmpDraggable: true, content: pin.element});
         marker = L.marker([lat,lon],{
                             draggable: true,
                             icon: myCustomIcon2}).addTo(map);
     }
 }
 else
  // marker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),
  //            draggable: true, icon:icon, title:text});
     marker = L.marker([lat, lon],{
                         draggable: true,
                         icon: icon}).addTo(map);

 marker.i = i;

 //google.maps.event.addListener(marker, "drag", function(pt) {
  marker.on('drag', function(pt){
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  var path = currSegment.path;
  //path.setAt(i,  pt.latLng );
  path[i] = pt;
  // if(grayLine)
  //   grayLine.setPath(path);
  // if(arrow)
  //     arrow.setMap(null);
 });

 //google.maps.event.addListener(marker, "dragend", function(pt) {
 marker.on('dragend', function(event) {
     var changedMarker = event.target;
     var position = changedMarker.getLatLng();

     console.log("New position: " + position.lat + ", " + position.lng);
     //webViewBridge.movePoint(SegmentId, i, pt.latLng.lat(), pt.latLng.lng());
     // path[mIx]= position;
     if(currSegment.leftLine)
     {
         currSegment.leftLine.remove();
         currSegment.leftLine.setLatLngs(path);
         currSegment.leftLine.addTo(map);
     }
     if(currSegment.rightLine)
     {
         currSegment.rightLine.remove();
         currSegment.rightLine.setLatLngs(path);
         currSegment.rightLine.addTo(map);
     }
     if(currSegment.singleLine)
     {
         currSegment.singleLine.remove();
         currSegment.singleLine.setLatLngs(path);
         currSegment.singleLine.addTo(map);
     }

     webViewBridge.movePoint(segmentId, mIx, position.lat, position.lng);

     var array = [];
     //new Array(0,0);
     //path.forEach(function(pt, ix)
     for ([pt,ix] of currSegment.path.entries())
     {
      array.push(position.lat);
      array.push(position.lng);
     };
     webViewBridge.movePointX(segmentId, mIx, position.lat, position.lng, array);
 });
 //google.maps.event.addListener(marker, "rightclick", function(){
  marker.on('contextmenu', function(){

  var pt = marker.position;
//OK                window.external.SetDebug("right click " + pt.lat() + ", " + pt.lng());
  webViewBridge.setDebug("right click " + pt.lat() + ", " + pt.lng());
  var path = line.getPath();
  var len = path.getLength();

//OK                window.external.updateIntersection( i, pt.lat(), pt.lng());
   if(len > 1)
    webViewBridge.updateIntersection( marker.i, pt.lat(), pt.lng());
  }
);

    //google.maps.event.addListener(marker, "dblclick", function()
    marker.on('dblclick', function()
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
  circle.remove();
  circle=null;
 }
 //circle = new google.maps.Circle({center:new google.maps.LatLng(lat, lon), fillOpacity: 0, map: map, strokeColor:"#000000", strokeWeight:1, radius:20});
 circle = L.circle([lat,lon], {
     color: 'red',
     fillColor: '#f03',
     fillOpacity: 0.5,
     radius: 20
 }).addTo(map);
 // if(bGeocoderRequest)
 //    geocoderRequest(lat, lon);
 return;
}

function createSegment(segmentId, routeName, segmentName, oneWay, showArrow, color, tracks, dash, routeType, trackUsage, points )
{
    var linePath = [];
    //var newSegment = null;
     webViewBridge.setDebug("segmentId: "+ segmentId + "usage: "+ trackUsage);
    for(var i=0; i < points; i +=2)
    {
        linePath.push( L.latLng(arguments[i+11], arguments[i+12]));
    }
    const newSegment = new SegmentInfo(segmentId, routeName, segmentName, oneWay, showArrow, color, tracks, dash, routeType, trackUsage, linePath );
    var pts = siArray.push(newSegment);
    console.log(pts + " segmentId: "+ newSegment.segmentId + " name: " + newSegment.segmentName);

}

function SegmentInfo(segmentId, routeName, segmentName, oneWay, showArrow, color, tracks, dash, routeType, trackUsage, path )
{
  this.type = "SegmentInfo";
  this.line = null;
  this.path = path;
  this.grayLine = null;
  this.segmentId = segmentId;
  this.routeName = routeName;
  this.segmentName = segmentName;
  this.oneWay = oneWay;
  this.showArrow = showArrow;
  this.color = color;
  this.tracks = tracks;
  this.dash = dash;
  this.trackUsage = trackUsage;
  this.arrow = null;
  this.routeType = routeType;
  this.leftLineColor = null;
  this.rightLineColor = null;
  this.singleLineColor = null;
  this.marker = null;

    // methods
    // return the path as an array of lat,lng, lat,lng, ...
    this.getPointArray = function ()
    {
     var array = [];
     //path.forEach(function(pt, ix)
     for ([pt,ix] of path.entries())
     {
      array.push(pt.lat);
      array.push(pt.lng);
     };
     return array;
    }

    function setEvents(line)
    {
        // Select segment (click)
        line.on( "mouseover", function(e){
            hiLiteLine(this.segmentId);
        });

        line.on("mouseout", function(e){
            restoreLine();
        });

        function getDistance(x1, y1, x2, y2) {
            return Math.hypot(x2 - x1, y2 - y1);
        }
        var greenIcon = new L.Icon({
          iconUrl: 'https://raw.githubusercontent.com/pointhi/leaflet-color-markers/master/img/marker-icon-2x-green.png',
          shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
          iconSize: [25, 41],
          iconAnchor: [12, 41],
          popupAnchor: [1, -34],
          shadowSize: [41, 41]
        });

        line.on("click", function(e){
            if(this.marker)
            {
                this.marker.remove();
                this.marker = null;
            }
            // hiLiteSelectedLine();
            webViewBridge.setDebug("sId = " + segmentId + " " + segmentName);

            //var path = line.path;
            var len = path.length;
            //var begin, end,bounds;
            webViewBridge.setLen(len);
            var clickLocation = e.latlng;
            var i;
            var mIx = -1;
            var dist = Infinity;
            var d = Infinity;
            var latLng, closestPoint =null;
            for(i=0; i < path.length-1; i++)
            {

              latLng = path[i];
              d = getDistance(latLng.lat, latLng.lng, clickLocation.lat, clickLocation.lng);
              if(d < dist)
              {
                mIx=i;
                dist = d;
                closestPoint = latLng;
              }
            }

            addMarker(i, closestPoint.lat, closestPoint.lng, mIx, segmentName + " route:" + routeName, segmentId);
            // //OK            window.external.selectSegment(i, SegmentId);
            var array = [];
            //path.forEach(function(pt, ix)
            for ([pt,ix] of path.entries())
            {
             array.push(pt.lat);
             array.push(pt.lng);
            };            webViewBridge.selectSegmentX(i, segmentId, array);
            webViewBridge.selectSegment(i, segmentId);
            addModeOff();
        });

        // right click to add a point
        //google.maps.event.addListener(this.line, "rightclick", function(e)
        line.on("contextmenu", function(e)
        {
            var si;
            si = hiLiteSelectedLine();
            // var path = si.getPath();
            // var len = path.getLength();

            // webViewBridge.selectSegment(0,si.segmentId);
            // webViewBridge.setLen(len);
            // var i;
            // var mIx = 1;
            // for(i=0; i < path.getLength()-1; i++)
            // {
            //     begin = path.getAt(i);
            //     end = path.getAt(i+1);
            //     bounds = setBounds( begin, end);
            //     if( bounds.contains(e.latLng))
            //     {
            //         break;
            //     }
            // }
            // si.insertPoint(i, e.latLng); // insert point after i
            // webViewBridge.insertPoint(si.segmentId, i, e.latLng.lat(), e.latLng.lng());
            // webViewBridge.insertPointX(si.segmentId, i, si.getPointArray());

            // webViewBridge.selectSegment(i+1, si.segmentId);
            // webViewBridge.selectSegmentX(i+1, si.segmentId, si.getPointArray());

            // addModeOff();
            // if(i>0)
            //   mIx=0;
            // addMarker(i+1, e.latLng.lat(), e.latLng.lng(), mIx, si.segmentName + " route:" + si.routeName, si.segmentId);
        });
    }  // end SegmentInfo.setEvents()

    function hiLiteLine()
    {

        if(leftLine)
            leftLine.setStyle({color: "#04b4B4"});
        if(rightLine)
            rightLine.setStyle({color: "#04b4B4"});
        if(singleLine)
            singleLine.setStyle({color: "#04b4B4"});
        if(arrowDecorator)
            arrowDecorator.setStyle({color: "#04b4B4"});
    }   // end SegmentInfo.hiLiteLine()

    function restoreLine()
    {

        if(leftLine)
            leftLine.setStyle({color: leftLineColor});
        if(rightLine)
            rightLine.setStyle({color: rightLineColor});
        if(singleLine)
            singleLine.setStyle({color: singleLineColor});
        if(arrowDecorator)
            arrowDecorator.setStyle({color: arrowDecoratorColor});
    }   // end SegmentInfo.restoreLine()




    // insert new point after pt
    this.insertPoint = function(pt, pos)
    {
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

    // move polyline's point at pt
    this.movePoint = function(pt, latLng)
    {
        path[pt] = latLng;
        if(leftLine)
            leftLine.setLatLngs(path);
        if(rightLine)
            rightLine.setLatLngs(path);
        if(singleLine)
            singleLine.setLatLngs(path);

        return path;
    }

    // update the paths
    this.updatePath = function()
    {
        if(leftLine)
            leftLine.setLatLngs(path);
        if(rightLine)
            rightLine.setLatLngs(path);
        if(singleLine)
            singleLine.setLatLngs(path);
    }

    // remove a point from a polyline
    this.deletePoint = function(pt, path)
    {
        path.removeAt(pt);
        if(leftLine)
            leftLine.setLatLngs(path);
        if(rightLine)
            rightLine.setLatLngs(path);
        if(singleLine)
            singleLine.setLatLngs(path);
        return path;
    }

    this.addNewPoint = function (e)
    {
        if(bAdding)
        {
            //map.disableDoubleClickZoom = true;

            if(path.length === 0)
            {
                addMarker(path.getLength(), e.latLng.lat(), e.latLng.lng(), 1, segment.getInfo(), segment.segmentId);
            }
            var pt =path.push(e.latLng);
            this.setPath(path);
            //getPoints();
            if(path.length > 0)
            {
                // window.external.addPoint();
                window.webViewBridge.addPoint(pt, e.latLng.lat(), e.latLng.lng());
                //window.webViewBridge.addPointX(pt, getPointArray());
            }
            this.placeArrow(path);
        }
        bAdding = false
    }

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



    // create the polylines, arrows. etc
    var singleLine;
    var leftLine = null;
    var rightLine = null;
    var decorator = null;
    var arrowDecorator = null;
    this.remove = function()
    {
        if(leftLine)
          leftLine.remove();
        if(rightLine)
          rightLine.remove();
        if(singleLine)
          singleLine.remove();

        if(decorator)
            decorator.remove();
        if(arrowDecorator)
            arrowDecorator.remove();

    }

    this.createLines = function()
    {

        webViewBridge.setDebug("SegmentId "+ segmentId + "usage: "+ trackUsage);

        if(tracks === 2)
        {
          leftLine = L.polyline(path, {color: color, weight: 2, offset: -4}).addTo(map);
            leftLineColor = color;
            if(trackUsage !== "L" && trackUsage !== " ")
                leftLineColor = "#A9A9A9";
            setEvents(leftLine);
          rightLine = L.polyline(path, {color: color, weight: 2, offset: +4}).addTo(map);
            rightLineColor = color;
            if(trackUsage !== "R" && trackUsage !== " ")
                rightLineColor = "#A9A9A9";
            setEvents(rightLine);
            if(showArrow)
                arrowDecorator = L.polylineDecorator(rightLine,{
                    patterns: [
                      {
                          offset: '100%',
                          repeat: 0,
                          symbol: L.Symbol.arrowHead({pixelSize: 15, pathOptions:{color: color} })
                      }
                        ]
              }).addTo(map);
        }
        else
        {
            singleLine = L.polyline(path,{color: color, weight: 2}).addTo(map);
            singleLineColor = color;
            setEvents(singleLine);
            if(showArrow)
                arrowDecorator = L.polylineDecorator(singleLine,{
                    patterns: [
                        {
                            offset: '100%',
                            repeat: 0,
                            symbol: L.Symbol.arrowHead({pixelSize: 15, pathOptions:{color: color} })
                        }
                    ]
            }).addTo(map);
        }

        // events
        map.on( "mousemove", function(e){
            if(bAdding)
            {
                //map.setOptions({draggableCursor:'Crosshair'});
                map.getContainer().style.cursor = 'crosshair';
            }
        });
    } // end createLines
    this.createLines();
}   // end SegmentInfo()

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

var elevated =
{
    path: 'M -2.5,0 2.5,0 M 1.0,-2 1.0,2 M -1.0,-2 -1.0,2',
    strokeOpacity: 1,
    strokeWeight:1,
    name:  "elevated"
};


//var pArray = ["M 0.5,-1 0.5,1 M -0.5,-1 -0.5,1", "M 0.6,-1 0.6,1 M -0.6,-1 -0.6,1",
//        "M 0.7,-1 0.7,1 M -0.7,-1 -0.7,1","M 0.8,-1 0.8,1 M -0.8,-1 -0.8,1",
//        "M 0.9,-1 0.9,1 M -0.9,-1 -0.9,1","M 1.0,-1 1.0,1 M -1.0,-1 -1.0,1"]
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

 // Erase a line segment
 function clearPolyline(si)
 {
     if(si)
        si.remove();
    return null;
 }


function clearMarker()
{
    if(marker)
    {
      marker.remove();
      marker = null;
    }
    if(circle)
    {
      circle.remove();
      circle = null;
    }
    if(infowindow !== null)
    {
      infowindow.setMap();
      infowindow = null;
    }
    return;
}

function clearAll()
{
    //clear all segments
    while(this.siArray.length > 0)
    {
        var si = this.siArray.pop();
        clearPolyline(si);
    }

    // clear other things

    return null;
}

function clearAll_x()
{

    var path;
    while(siArray.length > 0)
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
      var showArrow = si.showArrow;
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
      marker.remove();
      marker = null;
  }
  if(circle)
  {
      circle.remove();
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
  clearRectangle();
  selectedLine = null;
  clearPins();

  clearPinMarker();

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

function getSegmentInfo(segmentId)
{
    if(siArray.length ===0 )
    {
        console.log("siArray is empty");
        return null;
    }
    for (const [si, index] of siArray.entries())
    {
        if(si.segmentId === segmentId)
            return si;
    }
    console.log("segment " + segmentId + " not found");

    return null;
}

function addModeOn(segmentId)
{
    currentSegment = getSegmentInfo(segmentId);
    bAdding = true;
    // map.setOptions({draggableCursor:'Crosshair'});
    map.getContainer().style.cursor = 'crosshair';
    webViewBridge.addPointMode(bAdding);
    return null;
}

function addModeOff()
{
    bAdding = false;
//    map.setOptions({draggableCursor:'default'});
    map.getContainer().style.cursor = '';
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

function fitMapBounds(swLat, swLon, neLat, neLon)
{
  const bounds = [L.latLng(swLat, swLon), L.latLng(neLat, neLon)];
    map.fitBounds(bounds);
    return null;
}

function removeStationMarker(stationKey)
{
 var count = stationArray.getLength();
 //stationArray.forEach(function(element, index)
  for (const [element, index] of stationArray.entries())
 {
  if(index >= count)
      return;
  if(element && element !== 'undefined' && element.stationKey === stationKey)
  {
      element.setMap();
      stationArray.removeAt(index);
      return;
  }
 };
    return null;
}

function removeStationMarkers()
{
    if(!stationArray)
        return;
 var count = stationArray.length;
 //stationArray.forEach(function(element, index)
 for (const [element, index] of stationArray.entries())
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
 };
    return null;
}

function getStationMarkerIconType(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "???";
 //stationArray.forEach(function(element, index)
 for (const [element, index] of stationArray.entries())
 {
  if(index >= count)
   return rVal;
  if(element && element != 'undefined' && element.stationKey === stationKey)
  {
   //alert("icontype = " + element.typeIcon);
   rVal =  element.typeIcon;
   return rVal;
  }
 };
 return rVal;
}

function displayStationMarker(stationKey, bDisplay)
{
 if(!stationArray)
    return ;
 var count = stationArray.getLength();
 console.error("displayStationMarker " + stationKey + " count = " + count);
 //stationArray.forEach(function(element, index)
 for (const [element, index] of stationArray.entries())
 {
  if(index >= count)
    return;
  if(element && element.stationKey === stationKey)
  {
   element.setVisible(bDisplay)
   webViewBridge.setDebug("stationMarker " + stationKey + " is now visible " + element.getVisible());
  }
 };
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
 //stationArray.forEach(function(element, index)
 for (const [element, index] of stationArray.entries())
 {
  if(index >= count)
    return;
  if(element && element.stationKey === stationKey)
  {
   element.setIcon(icon);
   element.typeIcon = typeIcon;
  }
 };
 return;
}

function isStationMarkerDisplayed(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "false";
 //stationArray.forEach(function(element, index)
 for (const [element, index] of stationArray.entries())
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
  return rVal;
 };
 console.error("stationKey " + stationKey + " not found 2");
 return rVal;
}

function setDefaultOptions()
{

}

function clearRectangle()
{
    if(myRect)
    {
        myRect.setMap(null);
        myRect = null;
    }
}

function addPinMarker(latLng, title)
{
    var pinId = markerPins.length;
    var pin = new google.maps.marker.PinElement({background: "#FFFF00", glyph:pinId.toString()});
    var pinMarker = new google.maps.marker.AdvancedMarkerElement({map: map, position: latLng,
            gmpDraggable: true,  content: pin.element, title:title});

    markerPins.push(pinMarker);

    google.maps.event.addListener(marker, "dragend", function(latLng, pinId) {

        webViewBridge.pinClicked(pinId, latLng.lat(), event.latLng.lng(), title,-1,0,-1,'');
    });

}

function getPinLocations()
{
    const pointsArray = [];
    //markerPins.forEach(function(pinMarker, index)
    for (const [pinMarker, index] of markerPins.entries())
    {
       pointsArray.push(pinMarker.position);
       console.log(pinMarker.position);
    };

    var array = [];//new Array(0,0);
    //pointsArray.forEach(function(latLng, ix)
    for ([latLng, ix] of pointsArray.entries())
    {
     array[ix*2] = latLng.lat;
     array[(ix*2)+1] = latLng.lng;
    };
    return array;
}

function clearPinMarker()
{
    if(pinMarker)
    {
        pinMarker.setMap(null);
        pinMarker = null;
    }
}

function showStreetPins(firstlat, firstlon, secondlat, secondlon, title, id, location, draggable, seq)
{
    //clearPins();
    latLng1 = new google.maps.LatLng(firstlat, firstlon);
    latLng2 = new google.maps.LatLng(secondlat, secondlon);
    var pin1 = new google.maps.marker.PinElement({background: "#FFFF00", glyph:"1"});
    var pinMarker1 = new google.maps.marker.AdvancedMarkerElement({map: map, position: latLng1,
            gmpDraggable: draggable,  content: pin1.element, title: title});
    google.maps.event.addListener(pinMarker1, "dragend", function(pt) {
        webViewBridge.pinClicked(0, pt.latLng.lat(), pt.latLng.lng(), title, id, location, seq);
    });

    markerPins.push(pinMarker1);
    var pin2 = new google.maps.marker.PinElement({background: "#FFFF00", glyph:"2"});
    var pinMarker2 = new google.maps.marker.AdvancedMarkerElement({map: map, position: latLng2,
            gmpDraggable: draggable,  content: pin2.element, title: title});
    google.maps.event.addListener(pinMarker2, "dragend", function(pt) {
        webViewBridge.pinClicked(1, pt.latLng.lat(), pt.latLng.lng(), title, id, location, seq);
    });
    markerPins.push(pinMarker2);
}

function clearPins()
{
    //markerPins.forEach(function(pinMarker, index)
    for ([pinMarker, index] of pinMarker.entries())
    {
     pinMarker.setMap(null);
    };
    markerPins.clear();
}

function alertClose()
{
    alert("you may now close this window");
}

function loadOverlay(name, opacity, minZoom, maxZoom, source, bounds, urls)
{
 console.log("load overlay: " + name + " opacity =" + opacity + " minZoom =" + minZoom + " maxZoom = " + maxZoom + " source = " + source + " bounds = " + bounds + " urls = " + urls);
 console.log("urls type = " + typeof(urls));
//  if( Object.prototype.toString.call( urls ) === '[object Array]' )
//  {
//   console.log("size = " + urls.length +" url = " + urls[0] );
//  }

//  if(minZoom < 0 || maxZoom > 20)
//      console.warn("invalid min/max zoom for overlay: " + name + " opacity =" + opacity + " minZoom =" + minZoom + " maxZoom = " + maxZoom);
//  if ( overlay !== null)
//  {
//   map.overlayMapTypes.clear();
//   overlay = null;
//   if(opacityControl !== null)
//   {
//    opacityControl.remove();
//    opacityControl = null;
//   }
//  }
//  if(name === null || name === "")
//  {
//   return;
//  }
//  var vals = bounds.split(",");

// //     var mapBounds = new google.maps.LatLngBounds(
// //                 new google.maps.LatLng(38.623972, -90.330807),
// //                 new google.maps.LatLng(38.658606, -90.273631));
// this.overlayBounds = new google.maps.LatLngBounds(new google.maps.LatLng(vals[1], vals[0]),  new google.maps.LatLng(vals[3], vals[2]));
//    var mapMinZoom = minZoom;
//    var mapMaxZoom = maxZoom;
//    var opts = {
//    streetViewControl: false,
//    tilt: 0,
//    mapTypeId: google.maps.MapTypeId.HYBRID,
//    center: new google.maps.LatLng(0,0),
//    zoom: mapMinZoom
//    }

//  overlay = new Overlay(name, opacity, minZoom, maxZoom, source, overlayBounds, urls);
//  if(opacityControl === null)
//  {
//   opacityControl = new OpacityControl('opacityControl', map, google.maps.ControlPosition.RIGHT_TOP, overlay);
//   opacityControl.initialize(map);
//   google.maps.event.addListener(opacityControl, "opacitychanged", function()
//   {
//    webViewBridge.opacityChanged( overlay.name, overlay.getOpacity() );
//   });
//  }
//  google.maps.event.addListener(map, "zoom_changed", function() {
//   webViewBridge.displayZoom(map.getZoom());
//   var newZoom = map.getZoom();

//   if(overlay != null)
//   {
//    //console.error("zoom changed: zoom = "+ newZoom + " minZoom =" + overlay.minZoom + " maxZoom = " + overlay.maxZoom);
//    if(newZoom < overlay.minZoom || newZoom > overlay.maxZoom)
//    {
//     if(opacityControl !== null) {
//      opacityControl.remove();
//      opacityControl = null;
//     }
//    }
//    else
//    {
//     if(opacityControl === null)
//     {
//      opacityControl = new OpacityControl('opacityControl', map, google.maps.ControlPosition.RIGHT_TOP, overlay);
//      opacityControl.initialize(map);
//      google.maps.event.addListener(opacityControl, "opacitychanged", function()
//      {
//       webViewBridge.opacityChanged( overlay.name, overlay.getOpacity() );
//      });
//      webViewBridge.setDebug("opacity control added");
//     }
//    }
//   }
//  });
}

function setOverlayOpacity(Opacity) {
 if(overlay)
 {
  overlay.setOpacity(Opacity);
 }
 return;
}

function setBounds( pt1, pt2)
{
    var swlat, swlng;
    var nelat, nelng;
    if(pt1.lat < pt2.lat)
    {
        swlat = pt1.lat;
        nelat = pt2.lat;
    }
    else
    {
        swlat = pt2.lat;
        nelat = pt1.lat;
    }
    if(pt1.lng < pt2.lng)
    {
        swlng = pt1.lng;
        nelng = pt2.lng;
    }
    else
    {
        swlng = pt2.lng;
        nelng = pt1.lng;
    }
    //return new google.maps.LatLngBounds(new google.maps.LatLng(swlat, swlng), new google.maps.LatLng(nelat, nelng));
    //map.setBounds(swlng,nelng);
    // Create a bounding box
    var bounds = L.latLngBounds(swlng, nelng);

    // Make the map zoom and pan to fit this box
    map.fitBounds(bounds);
}

function hiLiteLine(segmentId)
{

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

// Find the closest point on the line
function findClosestPointOnLineToPolygon(polyGeo, lineGeo) {
    // Treat the polygon's outer boundary as a line
    const polygonTarget = turf.polygonToLine(polyGeo);

    let minDistance = Infinity;
    let closestPointOnLine = null;

    // Explode the polygon boundary into individual vertices (points)
    const vertices = turf.explode(polygonTarget);

    // Loop through each vertex of the polygon to find the closest spot on our polyline
    turf.featureEach(vertices, function (currentVertex) {
        // Find the math-checked closest point on the line from this specific vertex
        const snapped = turf.nearestPointOnLine(lineGeo, currentVertex);

        // Measure the distance between the vertex and that snapped point
        const distance = turf.distance(currentVertex, snapped, { units: 'kilometers' });

        // If it's shorter than what we found before, save it!
        if (distance < minDistance) {
            minDistance = distance;
            closestPointOnLine = snapped;
        }
    });

    return closestPointOnLine; // Returns a GeoJSON Point feature
}

function getCurrBounds()
{
    var bounds =  map.getBounds();
    var ne = bounds.getNorthEast();
    var sw = bounds.getSouthWest();
    webViewBridge.cityBounds( ne.lat, ne.lng, sw.lat, sw.lng);

}
