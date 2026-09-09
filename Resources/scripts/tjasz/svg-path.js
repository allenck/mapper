const CommandOperator = {
  Move: "M",
  Line: "L",
  Horizontal: "H",
  Vertical: "V",
  Cubic: "C",
  SmoothCubic: "S",
  Quadratic: "Q",
  SmoothQuadratic: "T",
  Arc: "A",
  Close: "Z",
}

// export type PathCommand = {
//   isAbsolute: boolean;
//   operator: CommandOperator;
//   parameters: number[];
// };

// export type SvgPath = PathCommand[];

const defaultCommand/*: PathCommand*/ = {
  isAbsolute: true,
  operator: CommandOperator.Move,
  parameters: [0, 0],
};

/*export*/ function toString(path/*: SvgPath*/)/*: string*/ {
  let str = "";

  for (const c of path) {
    str += (
      c.isAbsolute ? c.operator.toString() : c.operator.toString().toLowerCase()
    ).charAt(0);
    str += c.parameters.join(" ");
  }

  return str.trim();
}

/*export*/ function parse(path/*: string*/)/*: SvgPath*/ {
  if (!path || !path.length) {
    return [defaultCommand];
  }

  // Only numeric characters (0-9, period, and the negative operator),
  // command names (MLHVCSQTAZ and lowercase equivalents)
  // whitespace, and commas are allowed.
  const invalidCharacters = path.match(/[^\sMmLlHhVvCcSsQqTtAaZz0-9,.-]/);
  if (invalidCharacters?.length) {
    throw new Error(
      `Invalid SVG contains non-allowed characters '${invalidCharacters.join("")}': ${path}`,
    );
  }

  const commandStrings = path.trim().split(/(?=[MmLlHhVvCcSsQqTtAaZz])/);
  const commands = commandStrings.map(parseCommand);
  return commands;
}

function parseCommand(command/*: string*/)/*: PathCommand*/ {
  const commandName = command.charAt(0);
  const isAbsolute = "MLHVCSQTAZ".includes(commandName);
  const operator = parseCommandOperator(command);

  const parametersString = command.slice(1).trim();
  const parameters = parametersString.length
    ? parametersString.match(/[+-]?([0-9]*[.])?[0-9]+/g)?.map((v) => {
        const n = Number(v);
        if (isNaN(n)) {
          throw new Error("SVG path command parameter was not a number: " + v);
        }
        return n;
      }) ?? []
    : [];

  const expectedParamCount = getParamCountForOperator(operator);
  if (!checkRequiredParamCount(parameters, expectedParamCount)) {
    throw new Error(
      `Invalid parameter count for ${operator}. Expected multiple of ${expectedParamCount}. Got ${parameters.length}: ${command}`,
    );
  }

  return {
    isAbsolute,
    operator,
    parameters,
  };
}

function parseCommandOperator(operator/*: string*/) {
  operator = operator.toUpperCase().charAt(0);
  switch (operator) {
    case "M":
      return CommandOperator.Move;
    case "L":
      return CommandOperator.Line;
    case "H":
      return CommandOperator.Horizontal;
    case "V":
      return CommandOperator.Vertical;
    case "C":
      return CommandOperator.Cubic;
    case "S":
      return CommandOperator.SmoothCubic;
    case "Q":
      return CommandOperator.Quadratic;
    case "T":
      return CommandOperator.SmoothQuadratic;
    case "A":
      return CommandOperator.Arc;
    case "Z":
      return CommandOperator.Close;
    default:
      throw new Error(`Invalid SVG path command: ${operator}`);
  }
}

function checkRequiredParamCount(parameters/*: number[]*/, count/*: number*/)/*: boolean*/ {
  if (count === 0) {
    return parameters.length === 0;
  }
  if (count === 1) {
    return true;
  }
  return parameters.length % count === 0;
}

function getParamCountForOperator(operator/*: CommandOperator*/)/*: number*/ {
  switch (operator) {
    case CommandOperator.Move:
    case CommandOperator.Line:
    case CommandOperator.SmoothQuadratic:
      return 2;
    case CommandOperator.Horizontal:
    case CommandOperator.Vertical:
      return 1;
    case CommandOperator.Cubic:
      return 6;
    case CommandOperator.SmoothCubic:
    case CommandOperator.Quadratic:
      return 4;
    case CommandOperator.Arc:
      return 7;
    case CommandOperator.Close:
      return 0;
    default:
      throw new Error(`Invalid SVG path command: ${operator}`);
  }
}

const Path = {
  toString,
  parse,
};
//export default Path;