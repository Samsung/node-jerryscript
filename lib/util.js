// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
'use strict';

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var errors = require('internal/errors');

var _require = require('internal/encoding'),
    TextDecoder = _require.TextDecoder,
    TextEncoder = _require.TextEncoder;

var isBuffer = require('buffer').Buffer.isBuffer;

var _process$binding = process.binding('util'),
    getPromiseDetails = _process$binding.getPromiseDetails,
    getProxyDetails = _process$binding.getProxyDetails,
    isAnyArrayBuffer = _process$binding.isAnyArrayBuffer,
    isDataView = _process$binding.isDataView,
    isExternal = _process$binding.isExternal,
    isMap = _process$binding.isMap,
    isMapIterator = _process$binding.isMapIterator,
    isPromise = _process$binding.isPromise,
    isSet = _process$binding.isSet,
    isSetIterator = _process$binding.isSetIterator,
    isRegExp = _process$binding.isRegExp,
    isDate = _process$binding.isDate,
    kPending = _process$binding.kPending,
    kRejected = _process$binding.kRejected;

var _require2 = require('internal/util/types'),
    isTypedArray = _require2.isTypedArray;

var _require3 = require('internal/util'),
    customInspectSymbol = _require3.customInspectSymbol,
    deprecate = _require3.deprecate,
    getConstructorOf = _require3.getConstructorOf,
    internalErrorName = _require3.getSystemErrorName,
    isError = _require3.isError,
    promisify = _require3.promisify,
    join = _require3.join;

var inspectDefaultOptions = Object.seal({
  showHidden: false,
  depth: 2,
  colors: false,
  customInspect: true,
  showProxy: false,
  maxArrayLength: 100,
  breakLength: 60
});
var propertyIsEnumerable = Object.prototype.propertyIsEnumerable;
var regExpToString = RegExp.prototype.toString;
var dateToISOString = Date.prototype.toISOString;
var errorToString = Error.prototype.toString;
var CIRCULAR_ERROR_MESSAGE;
var Debug;
/* eslint-disable */

var strEscapeSequencesRegExp = /[\x00-\x1f\x27\x5c]/;
var strEscapeSequencesReplacer = /[\x00-\x1f\x27\x5c]/g;
/* eslint-enable */

var keyStrRegExp = /^[a-zA-Z_][a-zA-Z_0-9]*$/;
var colorRegExp = /\u001b\[\d\d?m/g;
var numberRegExp = /^(0|[1-9][0-9]*)$/; // Escaped special characters. Use empty strings to fill up unused entries.

var meta = ["\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007", '\\b', '\\t', '\\n', "\\u000b", '\\f', '\\r', "\\u000e", "\\u000f", "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014", "\\u0015", "\\u0016", "\\u0017", "\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f", '', '', '', '', '', '', '', "\\'", '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '\\\\'];

var escapeFn = function escapeFn(str) {
  return meta[str.charCodeAt(0)];
}; // Escape control characters, single quotes and the backslash.
// This is similar to JSON stringify escaping.


function strEscape(str) {
  // Some magic numbers that worked out fine while benchmarking with v8 6.0
  if (str.length < 5000 && !strEscapeSequencesRegExp.test(str)) return "'".concat(str, "'");
  if (str.length > 100) return "'".concat(str.replace(strEscapeSequencesReplacer, escapeFn), "'");
  var result = '';
  var last = 0;

  for (var i = 0; i < str.length; i++) {
    var point = str.charCodeAt(i);

    if (point === 39 || point === 92 || point < 32) {
      if (last === i) {
        result += meta[point];
      } else {
        result += "".concat(str.slice(last, i)).concat(meta[point]);
      }

      last = i + 1;
    }
  }

  if (last === 0) {
    result = str;
  } else if (last !== i) {
    result += str.slice(last);
  }

  return "'".concat(result, "'");
}

function tryStringify(arg) {
  try {
    return JSON.stringify(arg);
  } catch (err) {
    // Populate the circular error message lazily
    if (!CIRCULAR_ERROR_MESSAGE) {
      try {
        var a = {};
        a.a = a;
        JSON.stringify(a);
      } catch (err) {
        CIRCULAR_ERROR_MESSAGE = err.message;
      }
    }

    if (err.name === 'TypeError' && err.message === CIRCULAR_ERROR_MESSAGE) return '[Circular]';
    throw err;
  }
}

function format(f) {
  var i, tempStr;

  if (typeof f !== 'string') {
    if (arguments.length === 0) return '';
    var res = '';

    for (i = 0; i < arguments.length - 1; i++) {
      res += inspect(arguments[i]);
      res += ' ';
    }

    res += inspect(arguments[i]);
    return res;
  }

  if (arguments.length === 1) return f;
  var str = '';
  var a = 1;
  var lastPos = 0;

  for (i = 0; i < f.length - 1; i++) {
    if (f.charCodeAt(i) === 37) {
      // '%'
      var nextChar = f.charCodeAt(++i);

      if (a !== arguments.length) {
        switch (nextChar) {
          case 115:
            // 's'
            tempStr = String(arguments[a++]);
            break;

          case 106:
            // 'j'
            tempStr = tryStringify(arguments[a++]);
            break;

          case 100:
            // 'd'
            tempStr = "".concat(Number(arguments[a++]));
            break;

          case 79:
            // 'O'
            tempStr = inspect(arguments[a++]);
            break;

          case 111:
            // 'o'
            tempStr = inspect(arguments[a++], {
              showHidden: true,
              depth: 4,
              showProxy: true
            });
            break;

          case 105:
            // 'i'
            tempStr = "".concat(parseInt(arguments[a++]));
            break;

          case 102:
            // 'f'
            tempStr = "".concat(parseFloat(arguments[a++]));
            break;

          case 37:
            // '%'
            str += f.slice(lastPos, i);
            lastPos = i + 1;
            continue;

          default:
            // any other character is not a correct placeholder
            continue;
        }

        if (lastPos !== i - 1) str += f.slice(lastPos, i - 1);
        str += tempStr;
        lastPos = i + 1;
      } else if (nextChar === 37) {
        str += f.slice(lastPos, i);
        lastPos = i + 1;
      }
    }
  }

  if (lastPos === 0) str = f;else if (lastPos < f.length) str += f.slice(lastPos);

  while (a < arguments.length) {
    var x = arguments[a++];

    if (_typeof(x) !== 'object' && _typeof(x) !== 'symbol' || x === null) {
      str += " ".concat(x);
    } else {
      str += " ".concat(inspect(x));
    }
  }

  return str;
}

var debugs = {};
var debugEnviron;

function debuglog(set) {
  if (debugEnviron === undefined) {
    debugEnviron = new Set((process.env.NODE_DEBUG || '').split(',').map(function (s) {
      return s.toUpperCase();
    }));
  }

  set = set.toUpperCase();

  if (!debugs[set]) {
    if (debugEnviron.has(set)) {
      var pid = process.pid;

      debugs[set] = function () {
        var msg = exports.format.apply(exports, arguments);
        console.error('%s %d: %s', set, pid, msg);
      };
    } else {
      debugs[set] = function () {};
    }
  }

  return debugs[set];
}
/**
 * Echos the value of a value. Tries to print the value out
 * in the best way possible given the different types.
 *
 * @param {Object} obj The object to print out.
 * @param {Object} opts Optional options object that alters the output.
 */

/* Legacy: obj, showHidden, depth, colors*/


function inspect(obj, opts) {
  // Default options
  var ctx = {
    seen: [],
    stylize: stylizeNoColor,
    showHidden: inspectDefaultOptions.showHidden,
    depth: inspectDefaultOptions.depth,
    colors: inspectDefaultOptions.colors,
    customInspect: inspectDefaultOptions.customInspect,
    showProxy: inspectDefaultOptions.showProxy,
    maxArrayLength: inspectDefaultOptions.maxArrayLength,
    breakLength: inspectDefaultOptions.breakLength,
    indentationLvl: 0
  }; // Legacy...

  if (arguments.length > 2) {
    if (arguments[2] !== undefined) {
      ctx.depth = arguments[2];
    }

    if (arguments.length > 3 && arguments[3] !== undefined) {
      ctx.colors = arguments[3];
    }
  } // Set user-specified options


  if (typeof opts === 'boolean') {
    ctx.showHidden = opts;
  } else if (opts) {
    var optKeys = Object.keys(opts);

    for (var i = 0; i < optKeys.length; i++) {
      ctx[optKeys[i]] = opts[optKeys[i]];
    }
  }

  if (ctx.colors) ctx.stylize = stylizeWithColor;
  if (ctx.maxArrayLength === null) ctx.maxArrayLength = Infinity;
  return formatValue(ctx, obj, ctx.depth);
}

inspect.custom = customInspectSymbol;
Object.defineProperty(inspect, 'defaultOptions', {
  get: function get() {
    return inspectDefaultOptions;
  },
  set: function set(options) {
    if (options === null || _typeof(options) !== 'object') {
      throw new TypeError('"options" must be an object');
    }

    Object.assign(inspectDefaultOptions, options);
    return inspectDefaultOptions;
  }
}); // http://en.wikipedia.org/wiki/ANSI_escape_code#graphics

inspect.colors = Object.assign(Object.create(null), {
  'bold': [1, 22],
  'italic': [3, 23],
  'underline': [4, 24],
  'inverse': [7, 27],
  'white': [37, 39],
  'grey': [90, 39],
  'black': [30, 39],
  'blue': [34, 39],
  'cyan': [36, 39],
  'green': [32, 39],
  'magenta': [35, 39],
  'red': [31, 39],
  'yellow': [33, 39]
}); // Don't use 'blue' not visible on cmd.exe

inspect.styles = Object.assign(Object.create(null), {
  'special': 'cyan',
  'number': 'yellow',
  'boolean': 'yellow',
  'undefined': 'grey',
  'null': 'bold',
  'string': 'green',
  'symbol': 'green',
  'date': 'magenta',
  // "name": intentionally not styling
  'regexp': 'red'
});

function stylizeWithColor(str, styleType) {
  var style = inspect.styles[styleType];

  if (style !== undefined) {
    var color = inspect.colors[style];
    return "\x1B[".concat(color[0], "m").concat(str, "\x1B[").concat(color[1], "m");
  }

  return str;
}

function stylizeNoColor(str, styleType) {
  return str;
}

function ensureDebugIsInitialized() {
  /*if (Debug === undefined) {
    const runInDebugContext = require('vm').runInDebugContext;
    Debug = runInDebugContext('Debug');
  }*/
}

function formatValue(ctx, value, recurseTimes, ln) {
  // Primitive types cannot have properties
  if (_typeof(value) !== 'object' && typeof value !== 'function') {
    return formatPrimitive(ctx.stylize, value);
  }

  if (value === null) {
    return ctx.stylize('null', 'null');
  }

  if (ctx.showProxy) {
    var proxy = getProxyDetails(value);

    if (proxy !== undefined) {
      if (recurseTimes != null) {
        if (recurseTimes < 0) return ctx.stylize('Proxy [Array]', 'special');
        recurseTimes -= 1;
      }

      ctx.indentationLvl += 2;
      var res = [formatValue(ctx, proxy[0], recurseTimes), formatValue(ctx, proxy[1], recurseTimes)];
      ctx.indentationLvl -= 2;
      var str = reduceToSingleString(ctx, res, '', ['[', ']']);
      return "Proxy ".concat(str);
    }
  } // Provide a hook for user-specified inspect functions.
  // Check that value is an object with an inspect function on it


  if (ctx.customInspect) {
    var maybeCustomInspect = value[customInspectSymbol] || value.inspect;

    if (typeof maybeCustomInspect === 'function' && // Filter out the util module, its inspect function is special
    maybeCustomInspect !== exports.inspect && // Also filter out any prototype objects using the circular check.
    !(value.constructor && value.constructor.prototype === value)) {
      var ret = maybeCustomInspect.call(value, recurseTimes, ctx); // If the custom inspection method returned `this`, don't go into
      // infinite recursion.

      if (ret !== value) {
        if (typeof ret !== 'string') {
          return formatValue(ctx, ret, recurseTimes);
        }

        return ret;
      }
    }
  }

  var keys;
  var symbols = Object.getOwnPropertySymbols(value); // Look up the keys of the object.

  if (ctx.showHidden) {
    keys = Object.getOwnPropertyNames(value);
  } else {
    keys = Object.keys(value);
    if (symbols.length !== 0) symbols = symbols.filter(function (key) {
      return propertyIsEnumerable.call(value, key);
    });
  }

  var keyLength = keys.length + symbols.length;
  var constructor = getConstructorOf(value);
  var ctorName = constructor && constructor.name ? "".concat(constructor.name, " ") : '';
  var base = '';
  var formatter = formatObject;
  var braces;
  var noIterator = true;
  var raw; // Iterators and the rest are split to reduce checks

  if (value[Symbol.iterator]) {
    noIterator = false;

    if (Array.isArray(value)) {
      // Only set the constructor for non ordinary ("Array [...]") arrays.
      braces = ["".concat(ctorName === 'Array ' ? '' : ctorName, "["), ']'];
      if (value.length === 0 && keyLength === 0) return "".concat(braces[0], "]");
      formatter = formatArray;
    } else if (isSet(value)) {
      if (value.size === 0 && keyLength === 0) return "".concat(ctorName, "{}");
      braces = ["".concat(ctorName, "{"), '}'];
      formatter = formatSet;
    } else if (isMap(value)) {
      if (value.size === 0 && keyLength === 0) return "".concat(ctorName, "{}");
      braces = ["".concat(ctorName, "{"), '}'];
      formatter = formatMap;
    } else if (isTypedArray(value)) {
      braces = ["".concat(ctorName, "["), ']'];
      formatter = formatTypedArray;
    } else if (isMapIterator(value)) {
      braces = ['MapIterator {', '}'];
      formatter = formatCollectionIterator;
    } else if (isSetIterator(value)) {
      braces = ['SetIterator {', '}'];
      formatter = formatCollectionIterator;
    } else {
      // Check for boxed strings with valueOf()
      // The .valueOf() call can fail for a multitude of reasons
      try {
        raw = value.valueOf();
      } catch (e) {
        /* ignore */
      }

      if (typeof raw === 'string') {
        var formatted = formatPrimitive(stylizeNoColor, raw);
        if (keyLength === raw.length) return ctx.stylize("[String: ".concat(formatted, "]"), 'string');
        base = " [String: ".concat(formatted, "]"); // For boxed Strings, we have to remove the 0-n indexed entries,
        // since they just noisy up the output and are redundant
        // Make boxed primitive Strings look like such

        keys = keys.slice(value.length);
        braces = ['{', '}'];
      } else {
        noIterator = true;
      }
    }
  }

  if (noIterator) {
    braces = ['{', '}'];

    if (ctorName === 'Object ') {
      // Object fast path
      if (keyLength === 0) return '{}';
    } else if (typeof value === 'function') {
      var name = "".concat(constructor.name).concat(value.name ? ": ".concat(value.name) : '');
      if (keyLength === 0) return ctx.stylize("[".concat(name, "]"), 'special');
      base = " [".concat(name, "]");
    } else if (isRegExp(value)) {
      // Make RegExps say that they are RegExps
      if (keyLength === 0 || recurseTimes < 0) return ctx.stylize(regExpToString.call(value), 'regexp');
      base = " ".concat(regExpToString.call(value));
    } else if (isDate(value)) {
      if (keyLength === 0) {
        if (Number.isNaN(value.getTime())) return ctx.stylize(value.toString(), 'date');
        return ctx.stylize(dateToISOString.call(value), 'date');
      } // Make dates with properties first say the date


      base = " ".concat(dateToISOString.call(value));
    } else if (isError(value)) {
      // Make error with message first say the error
      if (keyLength === 0) return formatError(value);
      base = " ".concat(formatError(value));
    } else if (isAnyArrayBuffer(value)) {
      // Fast path for ArrayBuffer and SharedArrayBuffer.
      // Can't do the same for DataView because it has a non-primitive
      // .buffer property that we need to recurse for.
      if (keyLength === 0) return ctorName + "{ byteLength: ".concat(formatNumber(ctx.stylize, value.byteLength), " }");
      braces[0] = "".concat(ctorName, "{");
      keys.unshift('byteLength');
    } else if (isDataView(value)) {
      braces[0] = "".concat(ctorName, "{"); // .buffer goes last, it's not a primitive like the others.

      keys.unshift('byteLength', 'byteOffset', 'buffer');
    } else if (isPromise(value)) {
      braces[0] = "".concat(ctorName, "{");
      formatter = formatPromise;
    } else {
      // Check boxed primitives other than string with valueOf()
      // NOTE: `Date` has to be checked first!
      // The .valueOf() call can fail for a multitude of reasons
      try {
        raw = value.valueOf();
      } catch (e) {
        /* ignore */
      }

      if (typeof raw === 'number') {
        // Make boxed primitive Numbers look like such
        var _formatted = formatPrimitive(stylizeNoColor, raw);

        if (keyLength === 0) return ctx.stylize("[Number: ".concat(_formatted, "]"), 'number');
        base = " [Number: ".concat(_formatted, "]");
      } else if (typeof raw === 'boolean') {
        // Make boxed primitive Booleans look like such
        var _formatted2 = formatPrimitive(stylizeNoColor, raw);

        if (keyLength === 0) return ctx.stylize("[Boolean: ".concat(_formatted2, "]"), 'boolean');
        base = " [Boolean: ".concat(_formatted2, "]");
      } else if (_typeof(raw) === 'symbol') {
        var _formatted3 = formatPrimitive(stylizeNoColor, raw);

        return ctx.stylize("[Symbol: ".concat(_formatted3, "]"), 'symbol');
      } else if (keyLength === 0) {
        if (isExternal(value)) return ctx.stylize('[External]', 'special');
        return "".concat(ctorName, "{}");
      } else {
        braces[0] = "".concat(ctorName, "{");
      }
    }
  } // Using an array here is actually better for the average case than using
  // a Set. `seen` will only check for the depth and will never grow to large.


  if (ctx.seen.indexOf(value) !== -1) return ctx.stylize('[Circular]', 'special');

  if (recurseTimes != null) {
    if (recurseTimes < 0) {
      if (Array.isArray(value)) return ctx.stylize('[Array]', 'special');
      return ctx.stylize('[Object]', 'special');
    }

    recurseTimes -= 1;
  }

  ctx.seen.push(value);
  var output = formatter(ctx, value, recurseTimes, keys);

  for (var i = 0; i < symbols.length; i++) {
    output.push(formatProperty(ctx, value, recurseTimes, symbols[i], 0));
  }

  ctx.seen.pop();
  return reduceToSingleString(ctx, output, base, braces, ln);
}

function formatNumber(fn, value) {
  // Format -0 as '-0'. Checking `value === -0` won't distinguish 0 from -0.
  if (Object.is(value, -0)) return fn('-0', 'number');
  return fn("".concat(value), 'number');
}

function formatPrimitive(fn, value) {
  if (typeof value === 'string') return fn(strEscape(value), 'string');
  if (typeof value === 'number') return formatNumber(fn, value);
  if (typeof value === 'boolean') return fn("".concat(value), 'boolean');
  if (typeof value === 'undefined') return fn('undefined', 'undefined'); // es6 symbol primitive

  return fn(value.toString(), 'symbol');
}

function formatError(value) {
  return value.stack || "[".concat(errorToString.call(value), "]");
}

function formatObject(ctx, value, recurseTimes, keys) {
  var len = keys.length;
  var output = new Array(len);

  for (var i = 0; i < len; i++) {
    output[i] = formatProperty(ctx, value, recurseTimes, keys[i], 0);
  }

  return output;
} // The array is sparse and/or has extra keys


function formatSpecialArray(ctx, value, recurseTimes, keys, maxLength, valLen) {
  var output = [];
  var keyLen = keys.length;
  var visibleLength = 0;
  var i = 0;

  if (keyLen !== 0 && numberRegExp.test(keys[0])) {
    var _iteratorNormalCompletion = true;
    var _didIteratorError = false;
    var _iteratorError = undefined;

    try {
      for (var _iterator = keys[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
        var _key = _step.value;
        if (visibleLength === maxLength) break;
        var index = +_key; // Arrays can only have up to 2^32 - 1 entries

        if (index > Math.pow(2, 32) - 2) break;

        if (i !== index) {
          if (!numberRegExp.test(_key)) break;
          var emptyItems = index - i;
          var ending = emptyItems > 1 ? 's' : '';
          var message = "<".concat(emptyItems, " empty item").concat(ending, ">");
          output.push(ctx.stylize(message, 'undefined'));
          i = index;
          if (++visibleLength === maxLength) break;
        }

        output.push(formatProperty(ctx, value, recurseTimes, _key, 1));
        visibleLength++;
        i++;
      }
    } catch (err) {
      _didIteratorError = true;
      _iteratorError = err;
    } finally {
      try {
        if (!_iteratorNormalCompletion && _iterator["return"] != null) {
          _iterator["return"]();
        }
      } finally {
        if (_didIteratorError) {
          throw _iteratorError;
        }
      }
    }
  }

  if (i < valLen && visibleLength !== maxLength) {
    var len = valLen - i;

    var _ending = len > 1 ? 's' : '';

    var _message = "<".concat(len, " empty item").concat(_ending, ">");

    output.push(ctx.stylize(_message, 'undefined'));
    i = valLen;
    if (keyLen === 0) return output;
  }

  var remaining = valLen - i;

  if (remaining > 0) {
    output.push("... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : ''));
  }

  if (ctx.showHidden && keys[keyLen - 1] === 'length') {
    // No extra keys
    output.push(formatProperty(ctx, value, recurseTimes, 'length', 2));
  } else if (valLen === 0 || keyLen > valLen && keys[valLen - 1] === "".concat(valLen - 1)) {
    // The array is not sparse
    for (i = valLen; i < keyLen; i++) {
      output.push(formatProperty(ctx, value, recurseTimes, keys[i], 2));
    }
  } else if (keys[keyLen - 1] !== "".concat(valLen - 1)) {
    var extra = []; // Only handle special keys

    var key;

    for (i = keys.length - 1; i >= 0; i--) {
      key = keys[i];
      if (numberRegExp.test(key) && +key < Math.pow(2, 32) - 1) break;
      extra.push(formatProperty(ctx, value, recurseTimes, key, 2));
    }

    for (i = extra.length - 1; i >= 0; i--) {
      output.push(extra[i]);
    }
  }

  return output;
}

function formatArray(ctx, value, recurseTimes, keys) {
  var len = Math.min(Math.max(0, ctx.maxArrayLength), value.length);
  var hidden = ctx.showHidden ? 1 : 0;
  var valLen = value.length;
  var keyLen = keys.length - hidden;
  if (keyLen !== valLen || keys[keyLen - 1] !== "".concat(valLen - 1)) return formatSpecialArray(ctx, value, recurseTimes, keys, len, valLen);
  var remaining = valLen - len;
  var output = new Array(len + (remaining > 0 ? 1 : 0) + hidden);

  for (var i = 0; i < len; i++) {
    output[i] = formatProperty(ctx, value, recurseTimes, keys[i], 1);
  }

  if (remaining > 0) output[i++] = "... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : '');
  if (ctx.showHidden === true) output[i] = formatProperty(ctx, value, recurseTimes, 'length', 2);
  return output;
}

function formatTypedArray(ctx, value, recurseTimes, keys) {
  var maxLength = Math.min(Math.max(0, ctx.maxArrayLength), value.length);
  var remaining = value.length - maxLength;
  var output = new Array(maxLength + (remaining > 0 ? 1 : 0));

  for (var i = 0; i < maxLength; ++i) {
    output[i] = formatNumber(ctx.stylize, value[i]);
  }

  if (remaining > 0) output[i] = "... ".concat(remaining, " more item").concat(remaining > 1 ? 's' : '');

  if (ctx.showHidden) {
    // .buffer goes last, it's not a primitive like the others.
    var extraKeys = ['BYTES_PER_ELEMENT', 'length', 'byteLength', 'byteOffset', 'buffer'];

    for (i = 0; i < extraKeys.length; i++) {
      var str = formatValue(ctx, value[extraKeys[i]], recurseTimes);
      output.push("[".concat(extraKeys[i], "]: ").concat(str));
    }
  } // TypedArrays cannot have holes. Therefore it is safe to assume that all
  // extra keys are indexed after value.length.


  for (i = value.length; i < keys.length; i++) {
    output.push(formatProperty(ctx, value, recurseTimes, keys[i], 2));
  }

  return output;
}

function formatSet(ctx, value, recurseTimes, keys) {
  var output = new Array(value.size + keys.length + (ctx.showHidden ? 1 : 0));
  var i = 0;
  var _iteratorNormalCompletion2 = true;
  var _didIteratorError2 = false;
  var _iteratorError2 = undefined;

  try {
    for (var _iterator2 = value[Symbol.iterator](), _step2; !(_iteratorNormalCompletion2 = (_step2 = _iterator2.next()).done); _iteratorNormalCompletion2 = true) {
      var v = _step2.value;
      output[i++] = formatValue(ctx, v, recurseTimes);
    } // With `showHidden`, `length` will display as a hidden property for
    // arrays. For consistency's sake, do the same for `size`, even though this
    // property isn't selected by Object.getOwnPropertyNames().

  } catch (err) {
    _didIteratorError2 = true;
    _iteratorError2 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion2 && _iterator2["return"] != null) {
        _iterator2["return"]();
      }
    } finally {
      if (_didIteratorError2) {
        throw _iteratorError2;
      }
    }
  }

  if (ctx.showHidden) output[i++] = "[size]: ".concat(ctx.stylize("".concat(value.size), 'number'));

  for (var n = 0; n < keys.length; n++) {
    output[i++] = formatProperty(ctx, value, recurseTimes, keys[n], 0);
  }

  return output;
}

function formatMap(ctx, value, recurseTimes, keys) {
  var output = new Array(value.size + keys.length + (ctx.showHidden ? 1 : 0));
  var i = 0;
  var _iteratorNormalCompletion3 = true;
  var _didIteratorError3 = false;
  var _iteratorError3 = undefined;

  try {
    for (var _iterator3 = value[Symbol.iterator](), _step3; !(_iteratorNormalCompletion3 = (_step3 = _iterator3.next()).done); _iteratorNormalCompletion3 = true) {
      var _step3$value = _slicedToArray(_step3.value, 2),
          k = _step3$value[0],
          v = _step3$value[1];

      output[i++] = "".concat(formatValue(ctx, k, recurseTimes), " => ") + formatValue(ctx, v, recurseTimes);
    } // See comment in formatSet

  } catch (err) {
    _didIteratorError3 = true;
    _iteratorError3 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion3 && _iterator3["return"] != null) {
        _iterator3["return"]();
      }
    } finally {
      if (_didIteratorError3) {
        throw _iteratorError3;
      }
    }
  }

  if (ctx.showHidden) output[i++] = "[size]: ".concat(ctx.stylize("".concat(value.size), 'number'));

  for (var n = 0; n < keys.length; n++) {
    output[i++] = formatProperty(ctx, value, recurseTimes, keys[n], 0);
  }

  return output;
}

function formatCollectionIterator(ctx, value, recurseTimes, keys) {
  ensureDebugIsInitialized();
  var mirror = Debug.MakeMirror(value, true);
  var vals = mirror.preview();
  var output = [];
  var _iteratorNormalCompletion4 = true;
  var _didIteratorError4 = false;
  var _iteratorError4 = undefined;

  try {
    for (var _iterator4 = vals[Symbol.iterator](), _step4; !(_iteratorNormalCompletion4 = (_step4 = _iterator4.next()).done); _iteratorNormalCompletion4 = true) {
      var o = _step4.value;
      output.push(formatValue(ctx, o, recurseTimes));
    }
  } catch (err) {
    _didIteratorError4 = true;
    _iteratorError4 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion4 && _iterator4["return"] != null) {
        _iterator4["return"]();
      }
    } finally {
      if (_didIteratorError4) {
        throw _iteratorError4;
      }
    }
  }

  return output;
}

function formatPromise(ctx, value, recurseTimes, keys) {
  var output;

  var _getPromiseDetails = getPromiseDetails(value),
      _getPromiseDetails2 = _slicedToArray(_getPromiseDetails, 2),
      state = _getPromiseDetails2[0],
      result = _getPromiseDetails2[1];

  if (state === kPending) {
    output = ['<pending>'];
  } else {
    var str = formatValue(ctx, result, recurseTimes);
    output = [state === kRejected ? "<rejected> ".concat(str) : str];
  }

  for (var n = 0; n < keys.length; n++) {
    output.push(formatProperty(ctx, value, recurseTimes, keys[n], 0));
  }

  return output;
}

function formatProperty(ctx, value, recurseTimes, key, array) {
  var name, str;
  var desc = Object.getOwnPropertyDescriptor(value, key) || {
    value: value[key],
    enumerable: true
  };

  if (desc.value !== undefined) {
    var diff = array === 0 ? 3 : 2;
    ctx.indentationLvl += diff;
    str = formatValue(ctx, desc.value, recurseTimes, array === 0);
    ctx.indentationLvl -= diff;
  } else if (desc.get !== undefined) {
    if (desc.set !== undefined) {
      str = ctx.stylize('[Getter/Setter]', 'special');
    } else {
      str = ctx.stylize('[Getter]', 'special');
    }
  } else if (desc.set !== undefined) {
    str = ctx.stylize('[Setter]', 'special');
  } else {
    str = ctx.stylize('undefined', 'undefined');
  }

  if (array === 1) {
    return str;
  }

  if (_typeof(key) === 'symbol') {
    name = "[".concat(ctx.stylize(key.toString(), 'symbol'), "]");
  } else if (desc.enumerable === false) {
    name = "[".concat(key, "]");
  } else if (keyStrRegExp.test(key)) {
    name = ctx.stylize(key, 'name');
  } else {
    name = ctx.stylize(strEscape(key), 'string');
  }

  return "".concat(name, ": ").concat(str);
}

function reduceToSingleString(ctx, output, base, braces, addLn) {
  var breakLength = ctx.breakLength;

  if (output.length * 2 <= breakLength) {
    var length = 0;

    for (var i = 0; i < output.length && length <= breakLength; i++) {
      if (ctx.colors) {
        length += output[i].replace(colorRegExp, '').length + 1;
      } else {
        length += output[i].length + 1;
      }
    }

    if (length <= breakLength) return "".concat(braces[0]).concat(base, " ").concat(join(output, ', '), " ").concat(braces[1]);
  } // If the opening "brace" is too large, like in the case of "Set {",
  // we need to force the first item to be on the next line or the
  // items will not line up correctly.


  var indentation = ' '.repeat(ctx.indentationLvl);
  var extraLn = addLn === true ? "\n".concat(indentation) : '';
  var ln = base === '' && braces[0].length === 1 ? ' ' : "".concat(base, "\n").concat(indentation, "  ");
  var str = join(output, ",\n".concat(indentation, "  "));
  return "".concat(extraLn).concat(braces[0]).concat(ln).concat(str, " ").concat(braces[1]);
}

function isBoolean(arg) {
  return typeof arg === 'boolean';
}

function isNull(arg) {
  return arg === null;
}

function isNullOrUndefined(arg) {
  return arg === null || arg === undefined;
}

function isNumber(arg) {
  return typeof arg === 'number';
}

function isString(arg) {
  return typeof arg === 'string';
}

function isSymbol(arg) {
  return _typeof(arg) === 'symbol';
}

function isUndefined(arg) {
  return arg === undefined;
}

function isObject(arg) {
  return arg !== null && _typeof(arg) === 'object';
}

function isFunction(arg) {
  return typeof arg === 'function';
}

function isPrimitive(arg) {
  return arg === null || _typeof(arg) !== 'object' && typeof arg !== 'function';
}

function pad(n) {
  return n < 10 ? "0".concat(n.toString(10)) : n.toString(10);
}

var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']; // 26 Feb 16:19:34

function timestamp() {
  var d = new Date();
  var time = [pad(d.getHours()), pad(d.getMinutes()), pad(d.getSeconds())].join(':');
  return [d.getDate(), months[d.getMonth()], time].join(' ');
} // log is just a thin wrapper to console.log that prepends a timestamp


function log() {
  console.log('%s - %s', timestamp(), exports.format.apply(exports, arguments));
}
/**
 * Inherit the prototype methods from one constructor into another.
 *
 * The Function.prototype.inherits from lang.js rewritten as a standalone
 * function (not on Function.prototype). NOTE: If this file is to be loaded
 * during bootstrapping this function needs to be rewritten using some native
 * functions as prototype setup using normal JavaScript does not work as
 * expected during bootstrapping (see mirror.js in r114903).
 *
 * @param {function} ctor Constructor function which needs to inherit the
 *     prototype.
 * @param {function} superCtor Constructor function to inherit prototype from.
 * @throws {TypeError} Will error if either constructor is null, or if
 *     the super constructor lacks a prototype.
 */


function inherits(ctor, superCtor) {
  if (ctor === undefined || ctor === null) throw new TypeError('The constructor to "inherits" must not be ' + 'null or undefined');
  if (superCtor === undefined || superCtor === null) throw new TypeError('The super constructor to "inherits" must not ' + 'be null or undefined');

  if (superCtor.prototype === undefined) {
    throw new TypeError('The super constructor to "inherits" must ' + 'have a prototype');
  }

  ctor.super_ = superCtor;
  Object.setPrototypeOf(ctor.prototype, superCtor.prototype);
}

function _extend(target, source) {
  // Don't do anything if source isn't an object
  if (source === null || _typeof(source) !== 'object') return target;
  var keys = Object.keys(source);
  var i = keys.length;

  while (i--) {
    target[keys[i]] = source[keys[i]];
  }

  return target;
} // Deprecated old stuff.


function print() {
  for (var i = 0, len = arguments.length; i < len; ++i) {
    process.stdout.write(String(i < 0 || arguments.length <= i ? undefined : arguments[i]));
  }
}

function puts() {
  for (var i = 0, len = arguments.length; i < len; ++i) {
    process.stdout.write("".concat(i < 0 || arguments.length <= i ? undefined : arguments[i], "\n"));
  }
}

function debug(x) {
  process.stderr.write("DEBUG: ".concat(x, "\n"));
}

function error() {
  for (var i = 0, len = arguments.length; i < len; ++i) {
    process.stderr.write("".concat(i < 0 || arguments.length <= i ? undefined : arguments[i], "\n"));
  }
}

function callbackifyOnRejected(reason, cb) {
  // `!reason` guard inspired by bluebird (Ref: https://goo.gl/t5IS6M).
  // Because `null` is a special error value in callbacks which means "no error
  // occurred", we error-wrap so the callback consumer can distinguish between
  // "the promise rejected with null" or "the promise fulfilled with undefined".
  if (!reason) {
    var newReason = new errors.Error('ERR_FALSY_VALUE_REJECTION');
    newReason.reason = reason;
    reason = newReason;
    Error.captureStackTrace(reason, callbackifyOnRejected);
  }

  return cb(reason);
}

function callbackify(original) {
  if (typeof original !== 'function') {
    throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'original', 'function');
  } // We DO NOT return the promise as it gives the user a false sense that
  // the promise is actually somehow related to the callback's execution
  // and that the callback throwing will reject the promise.


  function callbackified() {
    var _this = this;

    for (var _len = arguments.length, args = new Array(_len), _key2 = 0; _key2 < _len; _key2++) {
      args[_key2] = arguments[_key2];
    }

    var maybeCb = args.pop();

    if (typeof maybeCb !== 'function') {
      throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'last argument', 'function');
    }

    var cb = function cb() {
      for (var _len2 = arguments.length, args = new Array(_len2), _key3 = 0; _key3 < _len2; _key3++) {
        args[_key3] = arguments[_key3];
      }

      Reflect.apply(maybeCb, _this, args);
    }; // In true node style we process the callback on `nextTick` with all the
    // implications (stack, `uncaughtException`, `async_hooks`)


    Reflect.apply(original, this, args).then(function (ret) {
      return process.nextTick(cb, null, ret);
    }, function (rej) {
      return process.nextTick(callbackifyOnRejected, rej, cb);
    });
  }

  Object.setPrototypeOf(callbackified, Object.getPrototypeOf(original));
  Object.defineProperties(callbackified, Object.getOwnPropertyDescriptors(original));
  return callbackified;
}

function getSystemErrorName(err) {
  if (typeof err !== 'number' || err >= 0 || !Number.isSafeInteger(err)) {
    throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'err', 'negative number');
  }

  return internalErrorName(err);
} // Keep the `exports =` so that various functions can still be monkeypatched


module.exports = exports = {
  _errnoException: errors.errnoException,
  _exceptionWithHostPort: errors.exceptionWithHostPort,
  _extend: _extend,
  callbackify: callbackify,
  debuglog: debuglog,
  deprecate: deprecate,
  format: format,
  getSystemErrorName: getSystemErrorName,
  inherits: inherits,
  inspect: inspect,
  isArray: Array.isArray,
  isBoolean: isBoolean,
  isBuffer: isBuffer,
  isNull: isNull,
  isNullOrUndefined: isNullOrUndefined,
  isNumber: isNumber,
  isString: isString,
  isSymbol: isSymbol,
  isUndefined: isUndefined,
  isRegExp: isRegExp,
  isObject: isObject,
  isDate: isDate,
  isError: isError,
  isFunction: isFunction,
  isPrimitive: isPrimitive,
  log: log,
  promisify: promisify,
  TextDecoder: TextDecoder,
  TextEncoder: TextEncoder,
  // Deprecated Old Stuff
  debug: deprecate(debug, 'util.debug is deprecated. Use console.error instead.', 'DEP0028'),
  error: deprecate(error, 'util.error is deprecated. Use console.error instead.', 'DEP0029'),
  print: deprecate(print, 'util.print is deprecated. Use console.log instead.', 'DEP0026'),
  puts: deprecate(puts, 'util.puts is deprecated. Use console.log instead.', 'DEP0027')
};