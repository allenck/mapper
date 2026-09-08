// import L from "leaflet";
// import { dist, moveAlongBearing, Point } from "./cartesian";
// import { parsePattern, Pattern, PixelOrPercent } from "./pattern";
// import { toString } from "./svg-path";
// import { rotate, translate } from "./path-transform";

// declare module "leaflet" {
//   export interface PathOptions {
//     pattern: string;
//   }
// }

/*export*/ const SvgPatternRenderer = L.SVG.extend({
  _updatePoly(layer/*: L.Path*/, closed/*: boolean*/) {
    this._setPath(
      layer,
      // @ts-expect-error
      pointsToPatternPath(layer._parts, closed, layer.options.pattern),
    );
  },
});

/*export*/ function pointsToPatternPath(
  rings/*: Point[][]*/,
  closed/*: boolean*/,
  patternString/*: string*/,)
  {
  let pattern/*: Pattern*/;
  try {
    pattern = parsePattern(patternString);
  } catch (error) {
    console.error(error);
    pattern = "solid";
  }

  let str = "",
    i/*: number*/,
    j/*: number*/,
    len/*: number*/,
    len2/*: number*/,
    points/*: Point[]*/,
    p/*: Point*/;

  // if a polygon, pattern is "solid" or any pattern part has type "T", draw the whole polyline first
  // Note that this means polygons essentially don't support "F" type patterns
  if (
    closed ||
    pattern === "solid" ||
    pattern.some((part) => part.type === "T")
  ) {
    for (i = 0, len = rings.length; i < len; i++) {
      points = rings[i];
      for (j = 0, len2 = points.length; j < len2; j++) {
        p = points[j];
        // If this is the first point of a ring, move the marker to it and draw nothing
        if (!j) {
          str += `M${p.x} ${p.y}`;
        } else {
          str += `L${p.x} ${p.y}`;
        }
      }
      // closes the ring for polygons
      str += closed ? "z" : "";
    }
  }

  // then draw the patterns
  // Note: intervals given in percentages will render at % intervals of the portion of the geometry that is VISIBLE,
  // not of the whole feature.
  if (pattern !== "solid") {
    for (i = 0, len = rings.length; i < len; i++) {
      points = rings[i];
      if (closed) {
        points.push(points[0]);
      }
      const ringDistance = Math.floor(
        points.reduce(
          (cumulativeDist, point, idx) =>
            idx ? cumulativeDist + dist(points[idx - 1], point) : 0,
          0,
        ),
      );
      let leftoverDistances = pattern.map((p) =>
        pixelsFrom(p.offset, ringDistance),
      );

      for (j = 1, len2 = points.length; j < len2; j++) {
        p = points[j];

        const prevPoint = points[j - 1];
        const segmentBearing = Math.atan2(p.y - prevPoint.y, p.x - prevPoint.x);
        const segmentDist = dist(prevPoint, p);
        for (let patternPart = 0; patternPart < pattern.length; patternPart++) {
          const pixelInterval = pixelsFrom(
            pattern[patternPart].interval,
            ringDistance,
          );
          let k = leftoverDistances[patternPart];
          for (
            ;
            k <= segmentDist;
            k += pixelInterval ? pixelInterval : ringDistance
          ) {
            const pk = moveAlongBearing(prevPoint, k, segmentBearing);
            // move the marker to this point
            str += `M${pk.x} ${pk.y}`;
            // draw the pattern
            // pattern is defined with negative y as the direction of travel,
            // but these bearings assume positive x is direction of travel, so rotate 90 extra degrees
            const patternInstance = toString(
              translate(
                rotate(pattern[patternPart].path, segmentBearing + Math.PI / 2),
                pk.x,
                pk.y,
              ),
            );
            str += patternInstance;
          }
          // set leftover distance
          leftoverDistances[patternPart] = k - segmentDist;
        }
      }
    }
  }

  // SVG complains about empty path strings
  return str || "M0 0";
}

function pixelsFrom(v/*: PixelOrPercent*/, total/*: number*/)/*: number*/ {
  if (v.type === "px") {
    return v.value;
  }

  return Math.floor((v.value / 100) * total);
}

/*
 * Allows compact syntax to be used
 */
L.svgPatternRenderer = function () {
    return new L.SvgPatternRenderer();
};