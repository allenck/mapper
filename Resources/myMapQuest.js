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

var circle;
function addMarker(index, lat, lon, icon, text, segmentId)
{
 this.index = index;
 this.lat = lat;
 this.lon = lon;
 this.icon = icon;
 this.text = text;
 this.segmentId = segmentId;

 console.log("addMarker "+  index + " " + lat+ " " + lon+ " " + icon+ " " + text+ " " + segmentId);
  var position = L.latLng(lat,lon);

 //siArray.forEach(function(si, ix)
 for(const si of siArray)
 {
  if(si.segmentId === segmentId)
  {
      currSegment = si;
  }
 };
 if(marker)
 {
  marker.remove();
  marker = null;
 }
 //marker.brkpt();
 webViewBridge.setDebug("add marker at lat: " + lat + " lon: " + lon + " point: " + index);
 //window.external.showSegmentsAtPoint(lat,lon);
 webViewBridge.showSegmentsAtPoint(lat,lon, segmentId);
 if(typeof icon == "number")
 {
     var iconUrl;
     var options;
     if(icon === -1) // use default icon
     {
        //  const pin = new google.maps.marker.PinElement({
        //      scale: 1.0,
        //     glyph:text,
        //     background: "#FBBC04",
        // });

        // marker = new google.maps.marker.AdvancedMarkerElement({map: map, position: new google.maps.LatLng(lat, lon),
        //         gmpDraggable: true,  content: pin.element});
        options = {
                     isAlphaNumericIcon: true
                         , text: text
                         , iconShape: 'marker'
                         , borderColor: '#FBBC04'
                         , textColor: '#00ABDC'
                 };
        var myCustomIcon = L.BeautifyIcon.icon(options);

        marker = L.marker([lat,lon],{
                           draggable: true,
                           icon: myCustomIcon}).addTo(map);
     }
     else
     {
         switch(icon)
         {
         case 0:
             //pin = new google.maps.marker.PinElement();
             options = {
                         isAlphaNumericIcon: true
                             , text: text
                             , iconShape: 'marker'
                             , borderColor: '#FF0000'
                             , textColor: '#00ABDC'
                     };
             break;
         case 1:
             //pin = new google.maps.marker.PinElement({background: "#00FF00", glyph:"0"});
             options = {
                         isAlphaNumericIcon: true
                             , text: "0"
                             , iconShape: 'marker'
                             , borderColor: '#00FF00'
                             , textColor: '#00ABDC'
                     };
             break;
         case 2:
             //pin = new google.maps.marker.PinElement({background: "#FF0000" });
             options = {
                         isAlphaNumericIcon: true
                             , text: text
                             , iconShape: 'marker'
                             , borderColor: '#FF0000'
                             , textColor: '#FFFFFF'
                             , backgroundColor: '#FF0000'
                     };
             break;
         }
         var myCustomIcon2 = L.BeautifyIcon.icon(options);

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

 marker.icon  = icon;

 //google.maps.event.addListener(marker, "drag", function(pt) {
  marker.on('drag', function(pt){
  //window.external.SetDebug("drag end " + pt.latLng.lat() + ", " + pt.latLng.lng());
  //var path = currSegment.path;
  //path.setAt(i,  pt.latLng );
  currSegment.path[index] = pt.latlng;
  currSegment.setPath(currSegment.path);
 });

 //google.maps.event.addListener(marker, "dragend", function(pt) {
 marker.on('dragend', function(event) {
     var si = getSegmentInfo(segmentId);
     var position0 = si.path[index];
     var position = event.target.getLatLng();

     console.log("New position: " + position.lat + ", " + position.lng);
     webViewBridge.movePoint(segmentId, index, position.lat, position.lng);
     currSegment.movePoint(index, position);


     webViewBridge.movePoint(segmentId, index, position.lat, position.lng);

     webViewBridge.movePointX(segmentId, index, position.lat, position.lng, currSegment.getPointArray());
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
     color: 'black',
     fillColor: 'white',
     fillOpacity: 0,
     radius: 20
 }).addTo(map);
 // if(bGeocoderRequest)
 //    geocoderRequest(lat, lon);
 return;
}

function addModeOn(segmentId)
{
    //currentSegment = getSegmentInfo(segmentId);
    for (const [ix, si] of siArray.entries())
    {
       if(si.segmentId !== null && si.segmentId === segmentId)
       {
            currSegment = si;
            bAdding = true;
            map.getContainer().style.cursor = 'crosshair';
            webViewBridge.addPointMode(bAdding);
       }
    }
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


function addNewPoint(e)
{
    if(bAdding)
    {
        if(!currSegment)
            return;
        // var line = currSegment.line;

        // if(line === null)
        // {
        //     //OK                    window.external.SetDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
        //     webViewBridge.setDebug("No line defined " + e.latLng.lat() + " " + e.latLng.lng());
        //     return;
        // }
        var path = currSegment.path;
        if(path.length === 0)
        {
            addMarker(path.length, e.latlng.lat, e.latlng.lng, 1, currSegment.segmentName, currSegment.segmentId);
        }
        path.push(e.latlng);
        //getPoints();
        if(path.length > 0)
            // window.external.addPoint();
            webViewBridge.addPoint(0, e.latlng.lat, e.latlng.lng);
        if(path.length > 1)
        {

        }

        //currSegment.placeArrow(path);
    }
} // end addNewPoint()

function addPinMarker(latLng, title)
{
    var pinId = markerPins.length;
    // var pin = new google.maps.marker.PinElement({background: "#FFFF00", glyph:pinId.toString()});
    // var pinMarker = new google.maps.marker.AdvancedMarkerElement({map: map, position: latLng,
    //         gmpDraggable: true,  content: pin.element, title:title});
    options = {
                isAlphaNumericIcon: true
                    , text: title
                    , iconShape: 'marker'
                    , borderColor: '#FFFF00'
                    , textColor: '#00ABDC'
            };
            L.marker([33.73, 73.487548828125], { icon: L.BeautifyIcon.icon(options), draggable: true }).addTo(map).bindPopup("I'm Beautify");
    markerPins.push(pinMarker);

    pinMarker.on(marker, "dragend", function(latLng, pinId) {

        webViewBridge.pinClicked(pinId, latLng.lat, event.latLng.lng, title,-1,0,-1,'');
    });

} //end addPinMarker()

function addRouteEndMarker( lat, lon, image, label)
{
    if(rtEndMarker !== null)
    {
        rtEndMarker.remove();
        rtEndMarker = null;
    }
    // rtEndMarker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),
    //                                          draggable: true, icon: image, label: label});
    // var options =         options = {
    //     isAlphaNumericIcon: true
    //         , text: text
    //         , iconShape: 'marker'
    //         , borderColor: red
    //         , textColor: '#00ABDC'
    // };
    // var myCustomIcon = L.BeautifyIcon.icon(options);
    var myCustomIcon = L.icon({
                                  iconUrl: image, // The path to your file
                                  iconSize: [25,41],              // Size of the icon [width, height]
                                  iconAnchor: [12, 41],            // Point of the icon which will correspond to marker's location [x, y]
                                  popupAnchor: [-3, -76]           // Point from which the popup should open relative to the iconAnchor [x, y]
                                });

    rtEndMarker = L.marker([lat,lon],{
              draggable: true,
              icon: myCustomIcon}).addTo(map);
    rtEndMarker.bindToolTip(label,{
                                permanent: true,   // Keeps the label open constantly
                                direction: "top",  // Positions the label above the marker ('top', 'bottom', 'left', 'right', 'center')
                                offset: [0, -10]   // Tweaks the exact position [x, y] pixel offsets)
                                });
    rtEndMarker.on( "dragend", function(pt) {
        var found = false;
        siArray.forEach(function(si, ix)
        {
                var i = si.isPointOnEnd(pt.latLng);
                if(i >=0)
                {
                    webViewBridge.moveRouteEndMarker(pt.latlng.lat, pt.latlng.lng, si.segmentId, i );
                   found = true;
                }
        });
        if(found == false)
            rtEndMarker.setLatLng([lat, lon]);
    });
    return null;
}

function addRouteStartMarker( lat, lon, image, label)
{
    // image ignored

    if(rtStartMarker !== null)
    {
        rtStartMarker.remove();
        rtStartMarker = null;
    }

    // rtStartMarker = new google.maps.Marker({map: map, position: new google.maps.LatLng(lat, lon),
    //                                            draggable: true, icon: image, label: label});
    // var options =         options = {
    //     isAlphaNumericIcon: true
    //         , text: text
    //         , iconShape: 'marker'
    //         , borderColor: green
    //         , textColor: '#00ABDC'
    // };
    // var myCustomIcon = L.BeautifyIcon.icon(options);
    var myCustomIcon = L.icon({
                                  iconUrl: image, // The path to your file
                                  iconSize: [25, 41],              // Size of the icon [width, height]
                                  iconAnchor: [12, 41],            // Point of the icon which will correspond to marker's location [x, y]
                                  popupAnchor: [-3, -76]           // Point from which the popup should open relative to the iconAnchor [x, y]
                                });
    rtStartMarker = L.marker([lat,lon],{
              draggable: true,
              icon: myCustomIcon}).addTo(map);
    rtStartMarker.bindToolTip(label,{
                                permanent: true,   // Keeps the label open constantly
                                direction: "top",  // Positions the label above the marker ('top', 'bottom', 'left', 'right', 'center')
                                offset: [0, -10]   // Tweaks the exact position [x, y] pixel offsets)
                                });
    rtStartMarker.on( "dragend", function(pt) {
        var found = false;
        for ( const [ix, si] of siArray.entries())
        {
                var i = si.isPointOnEnd(pt.latLng);
                if(i >=0)
                {
                    webViewBridge.moveRouteStartMarker(pt.latlng.lat, pt.latlng.lng, si.segmentId, i );
                    found = true;
                }
        };
        if(found == false)
            rtStartMarker.setLatLng([lat, lon]);
    });
    return null;
}

function addStationMarker(lat, lon, visible, segmentId, stationName, stationKey, infoKey, HTMLText, typeIcon)
{
 var stationMarker = null;
 console.log("addStationMarker lat=" + lat +  " lon=" + lon + " visible=" +  visible  + " segmentid =" + segmentId + " name= "  + stationName + " stationKey=" +  stationKey + " infoKey="+ infoKey + " text =" + HTMLText +  " icon=" + typeIcon);

 var bPresent = false;
 if(stationArray)
 {
  // check to see if already present
  var count = stationArray.length;
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
 //var shadow = getShadow(typeIcon);

 console.log("icon is typeof " + typeof(icon) + " " + icon)
 // var stationMarker = new google.maps.Marker({
 //                                              position: new google.maps.LatLng(lat, lon),
 //                                              //icon:icon,
 //                                                icon: new google.maps.MarkerImage(icon,
 //                                                    null, null, null, new google.maps.Size(16,16)),shadow:shadow,
 //                                              draggable:true,
 //                                              title:stationName
 //                                             });
 // options = {
 //              isAlphaNumericIcon: true
 //                  , text: text
 //                  , iconShape: 'marker'
 //                  , borderColor: yellow
 //                  , textColor: '#00ABDC'
 //          };
 // var myCustomIcon = L.BeautifyIcon.icon(options);

 stationMarker = L.marker([lat,lon],{
                    draggable: true,
                    icon: icon}).addTo(map);
 // stationMarker.setMap(map);
 // stationMarker.setVisible(visible)
 stationMarker.segmentId = segmentId;
 stationMarker.stationKey = stationKey;
 stationMarker.HTMLText = HTMLText;
 stationMarker.typeIcon = typeIcon;
 stationMarker.infoWindow = null;
  //stationArray.push(stationMarker);

 stationMarker.on("rightclick", function(){
//                window.external.updateStation(stationMarker.stationKey, segmentId);
      webViewBridge.updateStation(stationMarker.stationKey, segmentId);
 });
 if(HTMLText)
 {
     // var ifwo = {iconShape: 'rectangle', iconSize: [11,11], iconAnchor: [11,10],borderColor: 'yellow', draggable: true};
     //  stationMarker.infoKey = infoKey;
     // var ifwomicon = L.BeautifyIcon.icon(ifwo);
     //  //stationMarker.infoWindow = new google.maps.InfoWindow({content:HTMLText, position:new google.maps.LatLng(lat, lon)});
     //  stationMarker.infoWindow = L.marker([lat, lon], {icon: ifwomicon})
     //  .addTo(map)
     //  .bindPopup( HTMLText)
 }
 stationMarker.on('click', function(pt)
 {
     //stationMarker.infoWindow.openPopup();
     stationMarker.infowindow = L.popup({offset: L.point(0, -22)})
        .setLatLng([lat,lon])
        .setContent(HTMLText)
        .openOn(map); // Using openOn automatically closes other open popups
 });
 // stationMarker.infoWindow.on('click', function(pt)
 // {
 //     stationMarker.infoWindow.openPopup();
 // });

 stationMarker.on("dragend", function(pt)
 {
  //var found = false;
  //var closestPoint=new google.maps.LatLng(0,0);
  var closestPoint=pt.latLng;
  var distance = 9999999.0;
  var segmentId = stationMarker.segmentId;
  siArray.forEach(function(si, ix)
  {
   //var line = si.getLine();
   //var path = si.path;
   path.forEach(function(pt2, ix2)
   {
    var newBearing =   bearing(pt.latlng.lat, pt.latlng.lng, pt2.lat, pt2.lng);
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
  webViewBridge.moveStationMarker(stationMarker.stationKey, segmentId, closestPoint.lat, closestPoint.lng);
 //if(found == false)
 //    rtStartMarker.setPosition(new google.maps.LatLng(lat, lon));
 });
   stationArray.push(stationMarker);
 //return null;
 return;
}


function alertClose()
{
    alert("you may now close this window");
}

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
}   // end bearing




function clearAll()
{
   //clear all segments
   while(this.siArray.length)
   {
       si = siArray.pop();
       si.remove();
   }

   // clear other things
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
       poly2.remove();
       poly2 = null;
   }
   if(rtStartMarker !== null)
   {
       rtStartMarker.remove();
       rtStartMarker = null;
   }
   if(rtEndMarker !== null)
   {
       rtEndMarker.remove();
       rtEndMarker = null;
   }
   if(infowindow !== null)
   {
       infowindow.remove();
       if(infowindow.marker !== null && infowindow.marker !== undefined)
           infowindow.marker.remove();
       infowindow = null;
   }
   clearRectangle();
   selectedLine = null;
 //   clearPins();

 //   clearPinMarker();

   // if(stationArray)
       while(stationArray.length > 0)
       {
           var stationMarker = stationArray.pop();
           stationMarker.remove();
           if(stationMarker.infoWindow)
               stationMarker.infoWindow.remove();
           stationMarker = null;
       }
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

function clearPinMarker()
{
    if(pinMarker)
    {
        pinMarker.setMap(null);
        pinMarker = null;
    }
}

// Erase a line segment and remove from the map
function clearPolyline(segmentId)
{
    for (const [index, si] of this.siArray.entries())
    {
        if(si.segmentId === segmentId)
        {
            si.remove();
            this.siArray.splice(index,1);
            return;
        }
    }
    //console.error("clearPolyline: unable to remove segmentId " + segmentId );

   return null;
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

function clearRectangle()
{
    if(myRect)
    {
        myRect.remove();
        myRect = null;
    }
}

function closeCityBoundsButton()
{
    // TODO
    // if(map.controls[google.maps.ControlPosition.BOTTOM_CENTER].length>0)
    //     map.controls[google.maps.ControlPosition.BOTTOM_CENTER].removeAt(0);
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

} // end create segment

function displayRouteComment(latitude, longitude, HTMLText, commentKey, route, date, companyKey)
{
  marker = null;
 if(infowindow !== null)
 {
     //infowindow.marker.remove();
  infowindow.remove();
  infowindow = null;
 }
 if(latitude === 0 && longitude === 0)
 {
  latitude = map.getCenter().lat;
  longitude = map.getCenter().lon;
 }
 // var rectIcon = L.BeautifyIcon.icon({iconShape: 'rectangle', iconSize: [22,22]
 //                                        , iconAnchor: [11,10],borderColor: 'yellow'});
 var arrowIcon =  L.icon({iconUrl: 'https://www.google.com/mapfiles/arrow.png'});
 //infowindow = new google.maps.InfoWindow({content:HTMLText, maxWidth: 300, ariaLabel: "Comment"});

 infowindow = L.popup({offset: L.point(11, 6),
                          autoClose: false,
                          closeOnClick: false})
 .setLatLng([latitude,longitude])
 .setContent(HTMLText)
 .openOn(map); // Using openOn automatically closes other open popups

 // this.marker = new google.maps.Marker({
 //       position: new google.maps.LatLng(latitude, longitude),
 //       map: map,
 //       icon: image[images.arrow],
 //       zIndex: 10,
 //       draggable: true,
 //       visible: true,
 //       title: "comment"
 //     });
 //    var customIcon = L.icon({iconUrl: icon});
    infowindow.marker = L.marker([latitude, longitude], {
                               icon: arrowIcon,
                               draggable: true
                           }).addTo(map).bindTooltip('drag to move');
infowindow.on('remove', function()
{
    infowindow.marker.remove();
});
 //  if(this.marker === null)
 //      alert("marker is null");
 // //infowindow.setMap(map);
 // infowindow.marker = this.marker;
 infowindow.commentKey = commentKey;
 infowindow.route = route;
 infowindow.date = date;
 infowindow.lat = latitude;
 infowindow.lon = longitude;
 infowindow.companyKey = companyKey
 infowindow.marker.on("drag", function(pt)
 {
     infowindow.setLatLng(pt.latlng);
 });

 //google.maps.event.addListener(this.marker, "dragend", function(pt) {
  infowindow.marker.on('dragend', function(pt) {
  webViewBridge.moveRouteComment(infowindow.route, infowindow.date, pt.latlng.lat, pt.latlng.lng, infowindow.companyKey);
  });
 //google.maps.event.addListener(infowindow, "closeclick", function(){
  infowindow.on( "closeclick", function(){
 });
 return 0;
}// end displayRouteComment()

function displayStationMarker(stationKey, bDisplay)
{
 if(!stationArray)
    return ;
 var count = stationArray.length;
 console.error("displayStationMarker " + stationKey + " count = " + count);
 //stationArray.forEach(function(element, index)
 for (const [index, element] of stationArray.entries())
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
} // end displayStationMarker

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
} // end displayStationMarkers()

function displayTerminalMarkers(bDisplay)
{
    //alert("display terminal markers");
    if(bDisplay)
    {
        if(rtStartMarker !== null)
          rtStartMarker.remove();
        if(rtEndMarker !== null)
          rtEndMarker.remove();
    }
    else
    {
        if(rtStartMarker !== null)
          rtStartMarker.addTo(map);
        if(rtEndMarker !== null)
          rtEndMarker.addTo(map);
    }
    return null;
}
function fitMapBounds(swLat, swLon, neLat, neLon)
{
  const bounds = [L.latLng(swLat, swLon), L.latLng(neLat, neLon)];
    map.fitBounds(bounds);
    return null;
}// end fitMapBounds()

function geocoderRequest(lat, lon)
{

}

function getAdjacentPoints(latlngs, targetLatLng) {
    //const latlngs = polyline.getLatLngs(); // Array of L.LatLng points

    // Find the index of the target point (using a basic coordinate match)
    const index = latlngs.findIndex(pt =>
        pt.lat === targetLatLng.lat && pt.lng === targetLatLng.lng
    );

    if (index === -1) return { prev: null, next: null }; // Point not found

    return {
        prev: index > 0 ? latlngs[index - 1] : null,
        next: index < latlngs.length - 1 ? latlngs[index + 1] : null
    };
} // end getAdjacentPoints()

// returns map center LatLng
function getCenter()
{
 var latLng = map.getCenter();
 webViewBridge.setCenter(latLng.lat, latLng.lng, map.getZoom(), /*map.getMapTypeId()*/"");
 return latLng;
} // end get Center

function getCurrBounds()
{
    var bounds =  map.getBounds();
    var ne = bounds.getNorthEast();
    var sw = bounds.getSouthWest();
    webViewBridge.cityBounds( ne.lat, ne.lng, sw.lat, sw.lng);

} // end getCurrBounds()

function getIcon(typeIcon)
{
    var icon;
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
      case "slmetro":
          icon = image[images.slmetro];
          break;
      case "haltstelle":
          icon = image[images.haltstelle];
          break;
      default:
         //alert("icon type = " + typeIcon);
          console.error("invalid typeIcon: " + typeIcon);
         icon = image[images.default];;
         break;
     }
    }
    var markerIcon = new L.Icon({
                                   iconUrl: icon,
                                   shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
                                   iconSize: [25, 41],
                                   iconAnchor: [12, 41],
                                   popupAnchor: [1, -34],
                                   shadowSize: [41, 41]
                                 });

 return markerIcon;
}


function getMapType()
{
    return map.getMapTypeId();
} // end getMapType()

function getPinLocations()
{
    const pointsArray = [];
    //markerPins.forEach(function(pinMarker, index)
    for (const [index, pinMarker] of markerPins.entries())
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
} // getPinLocations

function getSegmentInfo(segmentId)
{
    if(siArray.length === 0 )
    {
        console.log("siArray is empty");
        return null;
    }
    for (const [index, si] of siArray.entries())
    {
        if(si.segmentId === segmentId)
            return si;
    }
    console.log("segment " + segmentId + " not found");

    return null;
} // getSegmentInfo

function getStationMarkerIconType(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "???";
 //stationArray.forEach(function(element, index)
 for (const [index, element] of stationArray.entries())
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
} // end getStationMarkerIconType()

function isStationMarkerDisplayed(stationKey)
{
 var count = stationArray.getLength();
 var rVal = "false";
 //stationArray.forEach(function(element, index)
 for (const [index,element] of stationArray.entries())
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
} // end isStationMarkerDisplayed()

// called onLoad by webChannel to initialize map
function initMap()
{
    console.log("begin GoogleMaps.js initMap()");
    webViewBridge.debug("initMap started");
    connectSlots();

    const osmTiles = new L.TileLayer(
      "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
      {
        attribution:
          '&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors',
      },
    );

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
    doubleClickZoom: false,
    // renderer: new SvgPatternRenderer(),
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

    map.on('dblclick', function(event)
    {
        addNewPoint(event);
    });

    siArray = [];
    markerPins = [];

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

function isAddModeOn()
{
 if(bAdding)
  return "true"
 else
  return "false";
}// end isAddModeOn()

function nextRouteComment()
{
 infowindow.marker.remove();
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, infowindow.commentKey,
                                     infowindow.companyKey, 1);
 return 0;
} // nextRouteComment()

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
    return L.latLng(rLat / dToRad, rLon / dToRad);
} // end pointRadialDistance()

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

function prevRouteComment()
{
 infowindow.marker.remove();
 webViewBridge.getInfoWindowComments(infowindow.lat, infowindow.lon, infowindow.route, infowindow.date, infowindow.commentKey,
                                     infowindow.companyKey, -1);
} // end prevRouteComment()


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
   if(myFucn == null)
   {
       console.error("function not found: " + call);
       return;
   }
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
} // end processScript()

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
      if(myFucn == null)
      {
          console.error("function not found: " + call);
          return;
      }
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
} // end processScript2()

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
      if(myFucn == null)
      {
          console.error("function not found: " + call);
          return;
      }
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
} // end processScript3()


function removeStationMarker(stationKey)
{
 var count = stationArray.length;
 //stationArray.forEach(function(element, index)
  for (const [index, element] of stationArray.entries())
 {
  if(index >= count)
      return;
  if(element && element !== 'undefined' && element.stationKey === stationKey)
  {
      element.remove();
      stationArray.removeAt(index);
      return;
  }
 };
    return null;
}// end removeStationMarker()

function removeStationMarkers()
{
    if(!stationArray)
        return;
 var count = stationArray.length;
 //stationArray.forEach(function(element, index)
 for (const [index, element] of stationArray.entries())
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
} // end removeStationMarkers()

function SegmentInfo(segmentId, routeName, segmentName, oneWay, showArrow, color, tracks, dash, routeType, trackUsage, path )
{
  this.type = "SegmentInfo";
  this.line = null;
  this.path = path;
  //this.grayLine = null;
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
  this.marker = null;

    var singleLine = null;
    var leftLine = null;
    var rightLine = null;
    var leftLineColor = null;
    var rightLineColor = null;
    var singleLineColor = null;
    var decorator = null;
    var arrowDecorator = null;


    // methods
    // return the path as an array of lat,lng, lat,lng, ...
    this.getPointArray = function ()
    {
     var array = [];
     //path.forEach(function(pt, ix)
     for ([ix,pt] of path.entries())
     {
      array.push(pt.lat);
      array.push(pt.lng);
     };
     return array;
    }

    function setEvents(line, target)
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

            addMarker(mIx, closestPoint.lat, closestPoint.lng, 1, segmentName + " route:" + routeName, segmentId);
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
            // var si;
            // si = hiLiteLine();
            // var path = si.getPath();
            // var len = path.getLength();

            webViewBridge.selectSegment(0,segmentId);
            webViewBridge.setLen(path.length);
            var i;
            // var mIx = 1;
            for(i=0; i < path.length-1; i++)
            {
                begin = path[i];
                end = path[i+1];
                if(begin.lat === end.lat &&  begin.lng === end.lng)
                    continue;
                bounds = L.latLngBounds( begin, end);
                if( bounds.contains(e.latlng))
                {
                    break;
                }
            }

            //insertPoint(i, e.latlng); // insert point after i
            path.splice(i,0,e.latlng);

            webViewBridge.insertPoint(segmentId, i, e.latlng.lat, e.latlng.lng);
            // webViewBridge.insertPointX(segmentId, i, this.getPointArray());

            webViewBridge.selectSegment(i+1, segmentId);
            // webViewBridge.selectSegmentX(i+1, segmentId, this.getPointArray());

            addModeOff();
            if(i>0)
              mIx=0;
            addMarker(i+1, e.latlng.lat, e.latlng.lng, mIx, segmentName + " route:" + routeName, segmentId);
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
        if(decorator)
           decorator.setStyle({color: "#04b4B4"});
    }
// end SegmentInfo.hiLiteLine()

    function restoreLine()
    {
        if(leftLine)
            leftLine.setStyle({color: leftLineColor});
        if(rightLine)
            rightLine.setStyle({color: rightLineColor});
        if(singleLine)
            singleLine.setStyle({color: singleLineColor});
        if(arrowDecorator)
            arrowDecorator.setStyle({color: color});
        if(decorator)
           decorator.setStyle({color: color});

    }   // end SegmentInfo.restoreLine()




    // insert new point after pt
    insertPoint = function(pt, pos)
    {
        path.splice(pt, 0, pos);
        remove();
        createLines();

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
        if(decorator)
            decorator.setLatLngs(path);
        if(arrowDecorator)
            arrowDecorator.setLatLngs(path);
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
            if(leftLine)
                leftLine.setLatLngs(path);
            if(rightLine)
                rightLine.setLatLngs(path);
            if(singleLine)
                singleLine.setLatLngs(path);
            if(decorator)
                decorator.setLatLngs(path);
            if(arrowDecorator)
                arrowDecorator.setLatLngs(path);
            //getPoints();
            if(path.length > 0)
            {
                // window.external.addPoint();
                window.webViewBridge.addPoint(pt, e.latLng.lat(), e.latLng.lng());
                //window.webViewBridge.addPointX(pt, getPointArray());
            }
            var line;
            if(leftLine)
                line = leftLine;
            if(singleLine)
                line = singleLine;
            arrowDecorator = L.polylineDecorator(line,{
                patterns: [
                    {
                        offset: '100%',
                        repeat: 0,
                        symbol: L.Symbol.arrowHead({pixelSize: 15, pathOptions:{color: color} })
                    }
                ]
            }).addTo(map);
        }
        bAdding = false
    }

    this.setPath = function(path)
    {
        if(leftLine)
            leftLine.setLatLngs(path);
        if(rightLine)
            rightLine.setLatLngs(path);
        if(singleLine)
            singleLine.setLatLngs(path);
        if(decorator)
            decorator.setLatLngs(path);
        if(arrowDecorator)
            arrowDecorator.setLatLngs(path);
    }

    // function to determine if the supplied point is on a begining or end linesegement of a segment
    this.isPointOnEnd = function(pt)
    {
        // var line =newline;
        // var path = this.line.getPath();
        var len = path.length;
        webViewBridge.setLen(len);
        var i;
        var mIx = 1;
        var b1 = bearing(pt.lat, pt.lng, path[0].lat, path[0].lng);
        if(b1.getDistance() < .020)
          return 0;
        var b2 =  bearing(pt.lat, pt.lng, path[len-1].lat, path[len-1].lng);
        if(b2.getDistance() < .020)
          return len-1;
        //alert("segment " + SegmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());
        webViewBridge.setDebug("segment " + segmentId + " distance = " + b1.getDistance() + " " + b2.getDistance());

        return -1;
    }



    // create the polylines, arrows. etc
    this.remove = function()
    {
        if(decorator)
            decorator.remove();
        if(arrowDecorator)
            arrowDecorator.remove();


        if(leftLine)
          leftLine.remove();
        if(rightLine)
          rightLine.remove();
        if(singleLine)
          singleLine.remove();

        if(circle)
        {
            circle.remove();
        }
    }

    this.createLines = function()
    {
        var newIcon = webViewBridge.createIcon(color);
        var iconName = color + '-rect.png';
        iconName = iconName.replace('#','');
        var IconPath = 'images/' + iconName;


        webViewBridge.setDebug("SegmentId "+ segmentId + "usage: "+ trackUsage);
        var curLine;
        if(tracks === 2)
        {
          leftLine = L.polyline(path, {color: color, weight: 2, offset: -2}).addTo(map);
          curLine = leftLine;
          leftLineColor = color;
          if(trackUsage !== "L" && trackUsage !== " ")
            leftLineColor = "#A9A9A9";
          setEvents(leftLine, this);
          rightLine = L.polyline(path, {color: color, weight: 2, offset: +2}).addTo(map);
          rightLineColor = color;
          if(trackUsage !== "R" && trackUsage !== " ")
          rightLineColor = "#A9A9A9";
          setEvents(rightLine, this);
        }
        else
        {
            singleLine = L.polyline(path,{color: color, weight: 2}).addTo(map);
            curLine = singleLine;
            singleLineColor = color;
            setEvents(singleLine, this);
        }
        // now add decorator and arrow
        if(showArrow)
              arrowDecorator = L.polylineDecorator(curLine,{
                  patterns: [
                    {
                        offset: '100%',
                        repeat: 0,
                        symbol: L.Symbol.arrowHead({pixelSize: 5, pathOptions:{color: color} })
                    }
                      ]
            }).addTo(map);
        if(dash ===2)
        {
            //  Add the Tick Marks layer over the polyline
            // decorator = L.polylineDecorator(curLine, {
            //     patterns: [
            //         {
            //             offset: 0,          // Where to start the ticks (0 means the beginning)
            //             repeat: '12px',     // Put a tick mark every 50 pixels along the line
            //             symbol: L.Symbol.dash({
            //                 pixelSize: 10,  // The length of the tick mark
            //                 pathOptions: {
            //                     color: color, // Color of the tick marks
            //                     weight: 1,        // Thickness of the tick marks
            //                     angle: 90         // Rotates the dash 90 degrees to make it a tick
            //                 }
            //            })

            //         }
            //     ]
            // }).addTo(map);

            // decorator = L.polylineDecorator(curLine, {
            //     patterns: [
            //         {
            //             offset: '5%',
            //             repeat: '50px',
            //             symbol: L.Symbol.marker()({
            //                 rotate: true, // Tells the decorator to rotate the marker along the line path
            //                 markerOptions: {
            //                     icon: L.divIcon({
            //                         className: 'my-custom-dash',
            //                         html: '<div style="width:10px; height:2px; background:black;"></div>',
            //                         iconSize: [10, 2]
            //                     })
            //                 }
            //             })
            //         }
            //     ]
            // }).addTo(map);

            // var feature = {
            //     type: "Feature",
            //     properties: {
            //       label: "tick marks every 20% of clipped view",
            //       pattern: "M-4 0 4 0,,20%,T",
            //     },
            //     geometry: {
            //       type: "LineString",
            //       coordinates: [
            //         path
            //       ],
            //     },
            //   };
            // const decorator = new L.GeoJSON(feature).addTo(map);


            // decorator = L.polylineDecorator(curLine, {
            //                                     patterns: [
            //                                         {offset: 5, repeat: 25, symbol: L.Symbol.tickMark({pixelSize: 15, pathOptions: {fillOpacity: 1, weight: 0}})}
            //                                     ]
            //                                 }).addTo(map);

        // --- Example with a rotated marker ---
            // options = {
            //             iconShape: 'rectangle'
            //                , borderWidth: 0
            //                , backgroundColor: color
            //                , iconSize: [12,2]
            //                , iconAnchor: [6, 2]
            // };

            // decorator = L.polylineDecorator(
            //     curLine,
            //     {
            //         patterns: [
            //             { offset: 0, repeat: 10, symbol: L.Symbol.dash({pixelSize: 5, pathOptions: {color: '#000000', weight: 1, opacity: 0.2}}) },
            //             { offset: 0, repeat: 5, symbol: L.Symbol.marker({rotate: true, markerOptions: {
            //                 icon: L.BeautifyIcon.icon(options)
            //             }})}
            //         ]
            //     }
            // ).addTo(map);

            // using icon png file
            var iconPath = 'images/FF0000_rect.png';
            decorator = L.polylineDecorator(
                curLine,
                {
                    patterns: [
                        { offset: 0, repeat: 10, symbol: L.Symbol.dash({pixelSize: 5, pathOptions: {color: '#000000', weight: 1, opacity: 0.2}}) },
                        { offset: 0, repeat: 5, symbol: L.Symbol.marker({rotate: true, markerOptions: {
                            icon: L.icon({
                                iconUrl: iconPath,
                                iconSize: [12,1],
                                iconAnchor: [6, 1]
                            })
                        }})}
                    ]
                }
            ).addTo(map);
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

function setCenter(Lat, Lon)
{
  //map.setCenter(L.latLng(Lat, Lon));
  map.panTo([Lat,Lon]);
 // map.setOptions({disableDoubleClickZoom: true });
 return null;

} // end set getCenter()

function setMapType(mapTypeId)
{
    if(mapTypeId === "")
        mapTypeId = "ROADMAP";
    map.setMapTypeId(mapTypeId);
    return null;
} // end setMapType()

function setZoom(zoom)
{
    map.setZoom(zoom);
    return null;
} // end setZoom()












function setDefaultOptions()
{

} // end setDefaultOptions()

function setOverlayOpacity(Opacity) {
 if(overlay)
 {
  overlay.setOpacity(Opacity);
 }
 return;
}// end setOverlayOpacity()

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
}// end setBounds()

function showRouteComment(bDisplay)
{
 if(infowindow !== null)
 {
  if(bDisplay)
  {
   infowindow.addTo(map);
  }
  else
  {
   infowindow.remove();
   infowindow.marker.remove();
  }
 }
 return;
} // end showRouteComment()

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
}// end showStreetPins()

function updateStationMarker(stationKey, typeIcon)
{
 if(!stationArray)
    return ;
 var icon = getIcon(typeIcon)
 var shadow = getShadow(typeIcon);


 var count = stationArray.getLength();
 console.error("displayStationMarker " + stationKey + " count = " + count);
 //stationArray.forEach(function(element, index)
 for (const [index,element] of stationArray.entries())
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
}// end updateStationMarker()

window.initialize = function() // called by WebChannel .ie "onLoad()"
{
  initMap();
} // end window.initialize


