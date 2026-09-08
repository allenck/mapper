// import { Point, rotateAroundPoint } from "./cartesian";
// import { CommandOperator, PathCommand, SvgPath, toString } from "./svg-path";

/**
 * Round all the coordinates in a path to the number of places given.
 * @param path The input path
 * @param places The number of decimal places to round to
 * @returns A copy of the path with the coordinates rounded
 */
// TODO is applying the rounding to all parameters a problem for arcs?
/*export*/ function round(path/*: SvgPath*/, places/*: number*/)/*: SvgPath*/ {
  const round = (n/*: number*/) => +n.toFixed(places);
  const result = path.map((c) => {
    return { ...c, parameters: c.parameters.map(round) };
  });
  return result;
}

/**
 * Translate a given path horizontally by `dx` and vertically by `dy`.
 * @param path The input path.
 * @param dx The amount to move the path along the X axis
 * @param dy The amount to move the path along the Y axis
 * @returns A copy of the path translated by (`dx`, `dy`)
 */
/*export*/ function translate(path/*: SvgPath*/, dx/*: number*/, dy/*: number*/)/*: SvgPath*/ {
  if (dx === 0 && dy === 0) {
    return path;
  }
  const result = path.map((c, i) => {
    if (c.isAbsolute) {
      switch (c.operator) {
        // commands with x and y coordinates
        case "M":
        case "L":
        case "C":
        case "S":
        case "Q":
        case "T":
          return {
            ...c,
            parameters: c.parameters.map((v, i) => v + (i % 2 ? dy : dx)),
          };
        // commands with just x coordinates
        case "H":
          return { ...c, parameters: c.parameters.map((x) => x + dx) };
        // commands with just y coordinates
        case "V":
          return { ...c, parameters: c.parameters.map((y) => y + dy) };
        // commands with no coordinates
        case "Z":
          return c;
        // the arc command
        case "A":
          // each arc is defined with 7 parameters where the last two are X and Y
          return {
            ...c,
            parameters: c.parameters.map(
              (v, i) => v + (i % 7 === 5 ? dx : i % 7 === 6 ? dy : 0),
            ),
          };
        default:
          throw new Error("Invalid SVG command: " + c.operator);
      }
    } else {
      // if an "m" command is the first command in a path,
      // its first pair of parameters should be treated as absolute
      if (i === 0 && c.operator === "M") {
        return {
          ...c,
          parameters: [
            c.parameters[0] + dx,
            c.parameters[1] + dy,
            ...c.parameters.slice(2),
          ],
        };
      }
      // most relative commands do not need any transformation
      return c;
    }
  });
  return result;
}

/*export*/ function scale(p/*: SvgPath*/, factor/*: number*/)/*: SvgPath*/ {
  if (factor === 1) {
    return p;
  }
  const result = p.map((c) => {
    switch (c.operator.charAt(0)) {
      case "A":
      case "a":
        return {
          ...c,
          parameters: c.parameters.map((v, i) =>
            i % 7 >= 2 && i % 7 <= 4 ? v : v * factor,
          ),
        };
      default:
        return { ...c, parameters: c.parameters.map((v) => v * factor) };
    }
  });
  return result;
}

/*export*/ function rotate(p/*: SvgPath*/, dtheta/*: number*/)/*: SvgPath */{
  if (dtheta === 0) {
    return p;
  }
  if (
    p.some((c) => !c.isAbsolute || c.operator === "H" || c.operator === "V")
  ) {
    p = toAbsoluteAndRemoveHV(p);
  }
  const result = p.map((c) => {
    if (c.isAbsolute) {
      switch (c.operator) {
        // commands with x and y coordinates
        case "M":
        case "L":
        case "C":
        case "S":
        case "Q":
        case "T":
          const parameters/*: number[]*/ = [];
          for (let i = 1; i < c.parameters.length; i += 2) {
            const x = c.parameters[i - 1];
            const y = c.parameters[i];
            const rotation = rotateAroundPoint({ x, y }, dtheta);
            parameters.push(rotation.x);
            parameters.push(rotation.y);
          }
          return {
            ...c,
            parameters,
          };
        // commands with just x coordinates
        case "H":
          throw new Error("Rotation of H command not supported.");
        // commands with just y coordinates
        case "V":
          throw new Error("Rotation of V command not supported.");
        // commands with no coordinates
        case "Z":
          return c;
        // the arc command
        case "A":
          // each arc is defined with 7 parameters where the last two are X and Y
          const arcCoords/*: number[]*/ = [];
          for (let i = 0; i < c.parameters.length; i++) {
            if (i % 7 < 5) {
              arcCoords.push(c.parameters[i]);
            }
            // intentionally do nothing if i % 7 === 5
            else if (i % 7 === 6) {
              const x = c.parameters[i - 1];
              const y = c.parameters[i];
              const rotation = rotateAroundPoint({ x, y }, dtheta);
              arcCoords.push(rotation.x);
              arcCoords.push(rotation.y);
            }
          }
          return { ...c, parameters: arcCoords };
        default:
          throw new Error("Invalid SVG command: " + c.operator);
      }
    } else {
      throw new Error("Rotation of relative commands not supported.");
    }
  });
  return result;
}

/**
 * Convert's a path's relative commands to absolute commands.
 * Convert's a path's H and V command sto L commands.
 *
 * H, V, and relative commands are difficult to rotate,
 * as the previous point must be known.
 * Applying this before rotation solves that problem.
 *
 * @param path The input path
 * @returns An equivalent path with the H and V commands replaced by L
 * and the relative commands replaced by absolute commands.
 */
function toAbsoluteAndRemoveHV(path/*: SvgPath*/)/*: SvgPath*/ {
  let subpathStart/*: Point = { x: 0, y: 0 }*/;
  let marker/*: Point = { x: 0, y: 0 }*/;
  const commands/*: PathCommand[] */= [];
  for (let i = 0; i < path.length; i++) {
    const c = path[i];
    if (c.isAbsolute) {
      switch (c.operator) {
        case "M":
          marker = {
            x: c.parameters[c.parameters.length - 2],
            y: c.parameters[c.parameters.length - 1],
          };
          subpathStart = { x: c.parameters[0], y: c.parameters[1] };
          commands.push(c);
          break;
        case "L":
        case "C":
        case "S":
        case "Q":
        case "T":
        case "A":
          marker = {
            x: c.parameters[c.parameters.length - 2],
            y: c.parameters[c.parameters.length - 1],
          };
          commands.push(c);
          break;
        case "H":
          marker = { x: c.parameters[c.parameters.length - 1], y: marker.y };
          commands.push({
            isAbsolute: true,
            operator: CommandOperator.Line,
            parameters: c.parameters.map((x) => [x, marker.y]).flat(),
          });
          break;
        case "V":
          marker = { x: marker.x, y: c.parameters[c.parameters.length - 1] };
          commands.push({
            isAbsolute: true,
            operator: CommandOperator.Line,
            parameters: c.parameters.map((y) => [marker.x, y]).flat(),
          });
          break;
        case "Z":
          marker = subpathStart;
          commands.push(c);
          break;
        default:
          throw new Error("Invalid SVG command: " + c.operator);
      }
    } else {
      // update marker and push command(s)
      switch (c.operator) {
        case "M":
          for (let j = 1; j < c.parameters.length; j += 2) {
            marker = {
              x: marker.x + c.parameters[j - 1],
              y: marker.y + c.parameters[j],
            };
            if (j < 2) {
              subpathStart = marker;
            }
            commands.push({
              isAbsolute: true,
              operator: j < 2 ? c.operator : CommandOperator.Line,
              parameters: [marker.x, marker.y],
            });
          }
          break;
        case "L":
        case "T":
          for (let j = 1; j < c.parameters.length; j += 2) {
            marker = {
              x: marker.x + c.parameters[j - 1],
              y: marker.y + c.parameters[j],
            };
            commands.push({
              isAbsolute: true,
              operator: c.operator,
              parameters: [marker.x, marker.y],
            });
          }
          break;
        case "H":
          for (let j = 0; j < c.parameters.length; j++) {
            marker = { x: marker.x + c.parameters[j], y: marker.y };
            commands.push({
              isAbsolute: true,
              operator: CommandOperator.Line,
              parameters: [marker.x, marker.y],
            });
          }
          break;
        case "V":
          for (let j = 0; j < c.parameters.length; j++) {
            marker = { x: marker.x, y: marker.y + c.parameters[j] };
            commands.push({
              isAbsolute: true,
              operator: CommandOperator.Line,
              parameters: [marker.x, marker.y],
            });
          }
          break;
        case "C":
          for (let j = 5; j < c.parameters.length; j += 6) {
            const parameters = [
              marker.x + c.parameters[j - 5],
              marker.y + c.parameters[j - 4],
              marker.x + c.parameters[j - 3],
              marker.y + c.parameters[j - 2],
              marker.x + c.parameters[j - 1],
              marker.y + c.parameters[j],
            ];
            marker = {
              x: marker.x + c.parameters[j - 1],
              y: marker.y + c.parameters[j],
            };
            commands.push({
              isAbsolute: true,
              operator: c.operator,
              parameters,
            });
          }
          break;
        case "S":
        case "Q":
          for (let j = 3; j < c.parameters.length; j += 4) {
            const parameters = [
              marker.x + c.parameters[j - 3],
              marker.y + c.parameters[j - 2],
              marker.x + c.parameters[j - 1],
              marker.y + c.parameters[j],
            ];
            marker = {
              x: marker.x + c.parameters[j - 1],
              y: marker.y + c.parameters[j],
            };
            commands.push({
              isAbsolute: true,
              operator: c.operator,
              parameters,
            });
          }
          break;
        case "A":
          for (let j = 6; j < c.parameters.length; j += 7) {
            marker = {
              x: marker.x + c.parameters[j - 1],
              y: marker.y + c.parameters[j],
            };
            commands.push({
              isAbsolute: true,
              operator: c.operator,
              parameters: [
                c.parameters[j - 6],
                c.parameters[j - 5],
                c.parameters[j - 4],
                c.parameters[j - 3],
                c.parameters[j - 2],
                marker.x,
                marker.y,
              ],
            });
          }
          break;
        case "Z":
          marker = subpathStart;
          commands.push({
            isAbsolute: true,
            operator: c.operator,
            parameters: [],
          });
          break;
        default:
          throw new Error("Invalid SVG command: " + c.operator);
      }
    }
  }

  return commands;
}

const Transform = {
  round,
  translate,
  scale,
  rotate,
};
//export default Transform;