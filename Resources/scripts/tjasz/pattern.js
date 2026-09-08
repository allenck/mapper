// import Svg, { SvgPath } from "./svg-path";

// export type PixelOrPercent = {
//   value: number;
//   type: "px" | "%";
// };
// type PatternPart = {
//   path: SvgPath;
//   offset: PixelOrPercent;
//   interval: PixelOrPercent;
//   type: "F" | "T";
// };
// export type Pattern = "solid" | PatternPart[];

/*export*/ function parsePattern(s/*: string*/)/*: Pattern*/ {
  if (s === "solid") {
    return "solid";
  }

  const patternParts = s.split(";");
  return patternParts.map((part) => {
    const parameters = part.split(",");
    const path = Svg.parse(parameters[0]);
    const offset = parsePixelOrPercent(parameters[1]);
    const interval = parsePixelOrPercent(parameters[2], {
      value: 100,
      type: "%",
    });
    const type = parameters[3] === "T" ? "T" : "F";
    return {
      path,
      offset,
      interval,
      type,
    };
  });
}

function parsePixelOrPercent(
  s/*: string*/,
  defaultValue/*?: PixelOrPercent*/,
)/*: PixelOrPercent*/ {
  if (!s || !s.length) {
    return defaultValue ?? { value: 0, type: "px" };
  }

  if (s.endsWith("%")) {
    const percent = Number(s.slice(0, s.length - 1));
    if (isNaN(percent)) {
      throw new Error(
        `Invalid pattern. Could not parse ${s} to number or percent.`,
      );
    }
    return { value: percent, type: "%" };
  }

  const value = Number(s);
  if (isNaN(value)) {
    throw new Error(
      `Invalid pattern. Could not parse ${s} to number or percent.`,
    );
  }
  return { value, type: "px" };
}

/*export*/ function patternToString(pattern/*: Pattern*/)/*: string*/ {
  if (pattern === "solid") {
    return "solid";
  }

  return pattern.map(partToString).join(";");
}

function partToString(part/*: PatternPart*/)/*: string*/ {
  return `${Svg.toString(part.path)},${pixelOrPercentToString(part.offset)},${pixelOrPercentToString(part.interval)},${part.type}`;
}

function pixelOrPercentToString(v/*: PixelOrPercent*/)/*: string*/ {
  if (v.type === "%") {
    return `${v.value}%`;
  }
  return v.value.toString();
}