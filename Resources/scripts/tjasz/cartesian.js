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