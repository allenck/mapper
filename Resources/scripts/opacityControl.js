/*
 * Opacity GControl by Klokan Petr Pridal (based on XSlider of Mike Williams)
 */

function OpacityControl( id, map, position, overlay ) {
  this.id = id;
  this.map = map;
  this.position = position;
  this.overlay = overlay;
}
//OpacityControl.prototype = new GControl();
//OpacityControl.prototype = new google.maps.Object();


// This function positions the slider to match the specified opacity
OpacityControl.prototype.setSlider = function(pos) {
  var left = Math.round((58*pos));
  this.slide.left = left;
  this.knob.style.left = left+"px";
  this.knob.style.top = "0px"; // correction001
}

// This function reads the slider and sets the overlay opacity level
OpacityControl.prototype.setOpacity = function() {
  //this.overlay.getTileLayer().opacity = this.slide.left/58;
  overlay.setOpacity(this.slide.left/58);
  //this.map.removeOverlay(this.overlay);
  //this.map.addOverlay(this.overlay);
}

// This gets called by the API when addControl(new OpacityControl())
OpacityControl.prototype.initialize = function(map) {
  var that=this;
  this.map = map;

  // Is this MSIE, if so we need to use AlphaImageLoader
  var agent = navigator.userAgent.toLowerCase();
  if ((agent.indexOf("msie") > -1) && (agent.indexOf("opera") < 1)){this.ie = true} else {this.ie = false}

  // create the background graphic as a <div> containing an image
  var container = document.createElement("div");
  container.style.width="70px";
  container.style.height="21px";
  container.id = this.id;
  container.title = 'move slider to change overlay opacity';

  // Handle transparent PNG files in MSIE
  if (this.ie) {
    var loader = "filter:progid:DXImageTransform.Microsoft.AlphaImageLoader("+
      "src='opacity-slider2.png', sizingMethod='crop');";
    container.innerHTML = '<div style="height:21px; width:70px; ' +loader+ '" ></div>';
  } else {
    container.innerHTML = '<div style="height:21px; width:70px; background-image:url(opacity-slider2.png)" ></div>';
  }

  // create the knob as a GDraggableObject
  // Handle transparent PNG files in MSIE
  if (this.ie) {
    var loader = "progid:DXImageTransform.Microsoft.AlphaImageLoader("+
      "src='opacity-slider2.png', sizingMethod='crop');";
    this.knob = document.createElement("div");
    this.knob.style.height="21px";
    this.knob.style.width="13px";
    this.knob.style.overflow="hidden";
    this.knob_img = document.createElement("div");
    this.knob_img.style.height="21px";
    this.knob_img.style.width="83px";
    this.knob_img.style.filter=loader;
    this.knob_img.style.position="relative";
    this.knob_img.style.left="-70px";
    this.knob.appendChild(this.knob_img);
  } else {
    this.knob = document.createElement("div");
    this.knob.style.height="21px";
    this.knob.style.width="13px";
    this.knob.style.backgroundImage="url(opacity-slider2.png)";
    this.knob.style.backgroundPosition="-70px 0px";
  }
  this.knob.style.visible=true;
  container.appendChild(this.knob);
  //this.slide=new GDraggableObject(this.knob, {container:container});
  this.slide= new ExtDraggableObject(this.knob, {restrictY:true, toleranceX:25, container:container});
  this.slide.setDraggableCursor('pointer');
  this.slide.setDraggingCursor('pointer');
  this.container = container;

  // attach the control to the map
  //map.getContainer().appendChild(container);
  this.map.controls[this.position].push(container);

  // init slider
  this.setSlider( this.overlay.getOpacity() );

  // Listen for the slider being moved and set the opacity
  //GEvent.addListener(this.slide, "dragend", function() {});
  opacity = this.overlay.getOpacity();
  //alert("opacity " + opacity);
  this.slide.setValueX(opacity*58/100);
  google.maps.event.addListener(this.slide, "drag", function() {
    var val = that.slide.valueX();
    val = val *100/58;
    //alert("opacity " + val);
    that.overlay.setOpacity(val);
    });
  google.maps.event.addListener(this.slide, "dragend", function(){
        google.maps.event.trigger(opacityControl, 'opacitychanged');
    });
  //GEvent.addListener(this.container, "click", function( x, y ) { alert(x, y) });

  return container;
}

// Set the default position for the control
OpacityControl.prototype.getDefaultPosition = function() {
  return new GControlPosition(G_ANCHOR_TOP_RIGHT, new GSize(7, 47));
}

// Remove the control from the map.
OpacityControl.prototype.remove = function() {
   this.map.controls[this.position].pop(this.container);
   google.maps.event.clearInstanceListeners(this);
}
