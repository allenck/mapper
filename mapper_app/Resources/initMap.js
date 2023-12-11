function initMap() {
    //const { Map } = await google.maps.importLibrary("maps");

    console.log("begin GoogleMaps.js initMap()");
    webViewBridge.debug("initMap started");
    connectSlots();


    //var Lat = 52.0;
    var Lat = webViewBridge.lat;
    //var Lon = 13.0;
    var Lon = webViewBridge.lng;
    //var zoom = 13;
    var zoom = webViewBridge.zoom;
    //var mapTypeId = google.maps.MapTypeId.ROADMAP;
    var mapTypeId = webViewBridge.maptype;
    var mapDiv = document.getElementById("map");


    map = new google.maps.Map(mapDiv, {
       center: new google.maps.LatLng(Lat, Lon),
       zoom: zoom,
       scaleControl: true,
       panControl: true,
       draggable: true,
       overviewMapControl: true,
       scrollwheel: true,
       disableDoubleClickZoom: true,
       mapTypeId: mapTypeId
    });
}
