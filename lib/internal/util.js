'use strict';

var errors = require('internal/errors');

var binding = process.binding('util');
var signals = process.binding('constants').os.signals;
var createPromise = binding.createPromise,
    promiseResolve = binding.promiseResolve,
    promiseReject = binding.promiseReject;

var _process$binding = process.binding('uv'),
    errmap = _process$binding.errmap;

var kArrowMessagePrivateSymbolIndex = binding.arrow_message_private_symbol;
var kDecoratedPrivateSymbolIndex = binding.decorated_private_symbol;
var noCrypto = !process.versions.openssl;
var experimentalWarnings = new Set();

function isError(e) {
  return objectToString(e) === '[object Error]' || e instanceof Error;
}

function objectToString(o) {
  return Object.prototype.toString.call(o);
} // Mark that a method should not be used.
// Returns a modified function which warns once by default.
// If --no-deprecation is set, then it is a no-op.


function deprecate(fn, msg, code) {
  if (process.noDeprecation === true) {
    return fn;
  }

  if (code !== undefined && typeof code !== 'string') throw new TypeError('`code` argument must be a string');
  var warned = false;

  function deprecated() {
    if (!warned) {
      warned = true;

      if (code !== undefined) {
        process.emitWarning(msg, 'DeprecationWarning', code, deprecated);
      } else {
        process.emitWarning(msg, 'DeprecationWarning', deprecated);
      }
    }

    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    if (this instanceof deprecated ? this.constructor : void 0) {
      return Reflect.construct(fn, args, this instanceof deprecated ? this.constructor : void 0);
    }

    return fn.apply(this, args);
  } // The wrapper will keep the same prototype as fn to maintain prototype chain


  Object.setPrototypeOf(deprecated, fn);

  if (fn.prototype) {
    // Setting this (rather than using Object.setPrototype, as above) ensures
    // that calling the unwrapped constructor gives an instanceof the wrapped
    // constructor.
    deprecated.prototype = fn.prototype;
  }

  return deprecated;
}

function decorateErrorStack(err) {
  if (!(isError(err) && err.stack) || binding.getHiddenValue(err, kDecoratedPrivateSymbolIndex) === true) return;
  var arrow = binding.getHiddenValue(err, kArrowMessagePrivateSymbolIndex);

  if (arrow) {
    err.stack = arrow + err.stack;
    binding.setHiddenValue(err, kDecoratedPrivateSymbolIndex, true);
  }
}

function assertCrypto() {
  if (noCrypto) throw new Error('Node.js is not compiled with openssl crypto support');
} // The loop should only run at most twice, retrying with lowercased enc
// if there is no match in the first pass.
// We use a loop instead of branching to retry with a helper
// function in order to avoid the performance hit.
// Return undefined if there is no match.


function normalizeEncoding(enc) {
  if (!enc) return 'utf8';
  var retried;

  while (true) {
    switch (enc) {
      case 'utf8':
      case 'utf-8':
        return 'utf8';

      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return 'utf16le';

      case 'latin1':
      case 'binary':
        return 'latin1';

      case 'base64':
      case 'ascii':
      case 'hex':
        return enc;

      default:
        if (retried) return; // undefined

        enc = ('' + enc).toLowerCase();
        retried = true;
    }
  }
}

function emitExperimentalWarning(feature) {
  if (experimentalWarnings.has(feature)) return;
  var msg = "".concat(feature, " is an experimental feature. This feature could ") + 'change at any time';
  experimentalWarnings.add(feature);
  process.emitWarning(msg, 'ExperimentalWarning');
}

function filterDuplicateStrings(items, low) {
  var map = new Map();

  for (var i = 0; i < items.length; i++) {
    var item = items[i];
    var key = item.toLowerCase();

    if (low) {
      map.set(key, key);
    } else {
      map.set(key, item);
    }
  }

  return Array.from(map.values()).sort();
}

function cachedResult(fn) {
  var result;
  return function () {
    if (result === undefined) result = fn();
    return result.slice();
  };
} // Useful for Wrapping an ES6 Class with a constructor Function that
// does not require the new keyword. For instance:
//   class A { constructor(x) {this.x = x;}}
//   const B = createClassWrapper(A);
//   B() instanceof A // true
//   B() instanceof B // true


function createClassWrapper(type) {
  function fn() {
    for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
      args[_key2] = arguments[_key2];
    }

    return Reflect.construct(type, args, (this instanceof fn ? this.constructor : void 0) || type);
  } // Mask the wrapper function name and length values


  Object.defineProperties(fn, {
    name: {
      value: type.name
    },
    length: {
      value: type.length
    }
  });
  Object.setPrototypeOf(fn, type);
  fn.prototype = type.prototype;
  return fn;
}

var signalsToNamesMapping;

function getSignalsToNamesMapping() {
  if (signalsToNamesMapping !== undefined) return signalsToNamesMapping;
  signalsToNamesMapping = Object.create(null);

  for (var key in signals) {
    signalsToNamesMapping[signals[key]] = key;
  }

  return signalsToNamesMapping;
}

function convertToValidSignal(signal) {
  if (typeof signal === 'number' && getSignalsToNamesMapping()[signal]) return signal;

  if (typeof signal === 'string') {
    var signalName = signals[signal.toUpperCase()];
    if (signalName) return signalName;
  }

  throw new errors.Error('ERR_UNKNOWN_SIGNAL', signal);
}

function getConstructorOf(obj) {
  while (obj) {
    var descriptor = Object.getOwnPropertyDescriptor(obj, 'constructor');

    if (descriptor !== undefined && typeof descriptor.value === 'function' && descriptor.value.name !== '') {
      return descriptor.value;
    }

    obj = Object.getPrototypeOf(obj);
  }

  return null;
}

function getSystemErrorName(err) {
  var entry = errmap.get(err);
  return entry ? entry[0] : "Unknown system error ".concat(err);
}

var kCustomPromisifiedSymbol = Symbol('util.promisify.custom');
var kCustomPromisifyArgsSymbol = Symbol('customPromisifyArgs');

function promisify(orig) {
  if (typeof orig !== 'function') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'original', 'function');

  if (orig[kCustomPromisifiedSymbol]) {
    var _fn = orig[kCustomPromisifiedSymbol];

    if (typeof _fn !== 'function') {
      throw new TypeError('The [util.promisify.custom] property must be ' + 'a function');
    }

    Object.defineProperty(_fn, kCustomPromisifiedSymbol, {
      value: _fn,
      enumerable: false,
      writable: false,
      configurable: true
    });
    return _fn;
  } // Names to create an object from in case the callback receives multiple
  // arguments, e.g. ['stdout', 'stderr'] for child_process.exec.


  var argumentNames = orig[kCustomPromisifyArgsSymbol];

  function fn() {
    var promise = createPromise();

    try {
      for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
        args[_key3] = arguments[_key3];
      }

      orig.call.apply(orig, [this].concat(args, [function (err) {
        if (err) {
          promiseReject(promise, err);
        } else if (argumentNames !== undefined && (arguments.length <= 1 ? 0 : arguments.length - 1) > 1) {
          var obj = {};

          for (var i = 0; i < argumentNames.length; i++) {
            obj[argumentNames[i]] = i + 1 < 1 || arguments.length <= i + 1 ? undefined : arguments[i + 1];
          }

          promiseResolve(promise, obj);
        } else {
          promiseResolve(promise, arguments.length <= 1 ? undefined : arguments[1]);
        }
      }]));
    } catch (err) {
      promiseReject(promise, err);
    }

    return promise;
  }

  Object.setPrototypeOf(fn, Object.getPrototypeOf(orig));
  Object.defineProperty(fn, kCustomPromisifiedSymbol, {
    value: fn,
    enumerable: false,
    writable: false,
    configurable: true
  });
  return Object.defineProperties(fn, Object.getOwnPropertyDescriptors(orig));
}

promisify.custom = kCustomPromisifiedSymbol; // The build-in Array#join is slower in v8 6.0

function join(output, separator) {
  var str = '';

  if (output.length !== 0) {
    for (var i = 0; i < output.length - 1; i++) {
      // It is faster not to use a template string here
      str += output[i];
      str += separator;
    }

    str += output[i];
  }

  return str;
} // About 1.5x faster than the two-arg version of Array#splice().


function spliceOne(list, index) {
  for (var i = index, k = i + 1, n = list.length; k < n; i += 1, k += 1) {
    list[i] = list[k];
  }

  list.pop();
}

module.exports = {
  assertCrypto: assertCrypto,
  cachedResult: cachedResult,
  convertToValidSignal: convertToValidSignal,
  createClassWrapper: createClassWrapper,
  decorateErrorStack: decorateErrorStack,
  deprecate: deprecate,
  emitExperimentalWarning: emitExperimentalWarning,
  filterDuplicateStrings: filterDuplicateStrings,
  getConstructorOf: getConstructorOf,
  getSystemErrorName: getSystemErrorName,
  isError: isError,
  join: join,
  normalizeEncoding: normalizeEncoding,
  objectToString: objectToString,
  promisify: promisify,
  spliceOne: spliceOne,
  // Symbol used to customize promisify conversion
  customPromisifyArgs: kCustomPromisifyArgsSymbol,
  // Symbol used to provide a custom inspect function for an object as an
  // alternative to using 'inspect'
  customInspectSymbol: Symbol('util.inspect.custom'),
  // Used by the buffer module to capture an internal reference to the
  // default isEncoding implementation, just in case userland overrides it.
  kIsEncodingSymbol: Symbol('node.isEncoding')
};