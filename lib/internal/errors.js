/* eslint alphabetize-errors: "error" */
'use strict'; // The whole point behind this internal module is to allow Node.js to no
// longer be forced to treat every error message change as a semver-major
// change. The NodeError classes here all expose a `code` property whose
// value statically and permanently identifies the error. While the error
// message may change, the code should not.

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _wrapNativeSuper(Class) { var _cache = typeof Map === "function" ? new Map() : undefined; _wrapNativeSuper = function _wrapNativeSuper(Class) { if (Class === null || !_isNativeFunction(Class)) return Class; if (typeof Class !== "function") { throw new TypeError("Super expression must either be null or a function"); } if (typeof _cache !== "undefined") { if (_cache.has(Class)) return _cache.get(Class); _cache.set(Class, Wrapper); } function Wrapper() { return _construct(Class, arguments, _getPrototypeOf(this).constructor); } Wrapper.prototype = Object.create(Class.prototype, { constructor: { value: Wrapper, enumerable: false, writable: true, configurable: true } }); return _setPrototypeOf(Wrapper, Class); }; return _wrapNativeSuper(Class); }

function isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

function _construct(Parent, args, Class) { if (isNativeReflectConstruct()) { _construct = Reflect.construct; } else { _construct = function _construct(Parent, args, Class) { var a = [null]; a.push.apply(a, args); var Constructor = Function.bind.apply(Parent, a); var instance = new Constructor(); if (Class) _setPrototypeOf(instance, Class.prototype); return instance; }; } return _construct.apply(null, arguments); }

function _isNativeFunction(fn) { return Function.toString.call(fn).indexOf("[native code]") !== -1; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _get(target, property, receiver) { if (typeof Reflect !== "undefined" && Reflect.get) { _get = Reflect.get; } else { _get = function _get(target, property, receiver) { var base = _superPropBase(target, property); if (!base) return; var desc = Object.getOwnPropertyDescriptor(base, property); if (desc.get) { return desc.get.call(receiver); } return desc.value; }; } return _get(target, property, receiver || target); }

function _superPropBase(object, property) { while (!Object.prototype.hasOwnProperty.call(object, property)) { object = _getPrototypeOf(object); if (object === null) break; } return object; }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

var kCode = Symbol('code');
var messages = new Map();

var _process$binding = process.binding('uv'),
    UV_EAI_MEMORY = _process$binding.UV_EAI_MEMORY,
    UV_EAI_NODATA = _process$binding.UV_EAI_NODATA,
    UV_EAI_NONAME = _process$binding.UV_EAI_NONAME;

var defineProperty = Object.defineProperty; // Lazily loaded

var util_ = null;

function lazyUtil() {
  if (!util_) {
    util_ = require('util');
  }

  return util_;
}

var internalUtil = null;

function lazyInternalUtil() {
  if (!internalUtil) {
    internalUtil = require('internal/util');
  }

  return internalUtil;
}

function makeNodeError(Base) {
  return (
    /*#__PURE__*/
    function (_Base) {
      _inherits(NodeError, _Base);

      function NodeError(key) {
        var _this;

        _classCallCheck(this, NodeError);

        for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
          args[_key - 1] = arguments[_key];
        }

        _this = _possibleConstructorReturn(this, _getPrototypeOf(NodeError).call(this, message(key, args)));
        defineProperty(_assertThisInitialized(_this), kCode, {
          configurable: true,
          enumerable: false,
          value: key,
          writable: true
        });
        return _this;
      }

      _createClass(NodeError, [{
        key: "name",
        get: function get() {
          return "".concat(_get(_getPrototypeOf(NodeError.prototype), "name", this), " [").concat(this[kCode], "]");
        },
        set: function set(value) {
          defineProperty(this, 'name', {
            configurable: true,
            enumerable: true,
            value: value,
            writable: true
          });
        }
      }, {
        key: "code",
        get: function get() {
          return this[kCode];
        },
        set: function set(value) {
          defineProperty(this, 'code', {
            configurable: true,
            enumerable: true,
            value: value,
            writable: true
          });
        }
      }]);

      return NodeError;
    }(Base)
  );
}

var AssertionError =
/*#__PURE__*/
function (_Error) {
  _inherits(AssertionError, _Error);

  function AssertionError(options) {
    var _this2;

    _classCallCheck(this, AssertionError);

    if (_typeof(options) !== 'object' || options === null) {
      throw new exports.TypeError('ERR_INVALID_ARG_TYPE', 'options', 'object');
    }

    var actual = options.actual,
        expected = options.expected,
        message = options.message,
        operator = options.operator,
        stackStartFn = options.stackStartFn;

    if (message) {
      _this2 = _possibleConstructorReturn(this, _getPrototypeOf(AssertionError).call(this, message));
    } else {
      var util = lazyUtil();
      if (actual && actual.stack && actual instanceof Error) actual = "".concat(actual.name, ": ").concat(actual.message);
      if (expected && expected.stack && expected instanceof Error) expected = "".concat(expected.name, ": ").concat(expected.message);
      _this2 = _possibleConstructorReturn(this, _getPrototypeOf(AssertionError).call(this, "".concat(util.inspect(actual).slice(0, 128), " ") + "".concat(operator, " ").concat(util.inspect(expected).slice(0, 128))));
    }

    _this2.generatedMessage = !message;
    _this2.name = 'AssertionError [ERR_ASSERTION]';
    _this2.code = 'ERR_ASSERTION';
    _this2.actual = actual;
    _this2.expected = expected;
    _this2.operator = operator;
    Error.captureStackTrace(_assertThisInitialized(_this2), stackStartFn);
    return _possibleConstructorReturn(_this2);
  }

  return AssertionError;
}(_wrapNativeSuper(Error)); // This is defined here instead of using the assert module to avoid a
// circular dependency. The effect is largely the same.


function internalAssert(condition, message) {
  if (!condition) {
    throw new AssertionError({
      message: message,
      actual: false,
      expected: true,
      operator: '=='
    });
  }
}

function message(key, args) {
  var msg = messages.get(key);
  internalAssert(msg, "An invalid error message key was used: ".concat(key, "."));
  var fmt;

  if (typeof msg === 'function') {
    fmt = msg;
  } else {
    var util = lazyUtil();
    fmt = util.format;
    if (args === undefined || args.length === 0) return msg;
    args.unshift(msg);
  }

  return String(fmt.apply(null, args));
} // Utility function for registering the error codes. Only used here. Exported
// *only* to allow for testing.


function E(sym, val) {
  messages.set(sym, typeof val === 'function' ? val : String(val));
}
/**
 * This used to be util._errnoException().
 *
 * @param {number} err - A libuv error number
 * @param {string} syscall
 * @param {string} [original]
 * @returns {Error}
 */


function errnoException(err, syscall, original) {
  // TODO(joyeecheung): We have to use the type-checked
  // getSystemErrorName(err) to guard against invalid arguments from users.
  // This can be replaced with [ code ] = errmap.get(err) when this method
  // is no longer exposed to user land.
  var code = lazyUtil().getSystemErrorName(err);
  var message = original ? "".concat(syscall, " ").concat(code, " ").concat(original) : "".concat(syscall, " ").concat(code);
  var ex = new Error(message); // TODO(joyeecheung): errno is supposed to err, like in uvException

  ex.code = ex.errno = code;
  ex.syscall = syscall;
  Error.captureStackTrace(ex, errnoException);
  return ex;
}
/**
 * This used to be util._exceptionWithHostPort().
 *
 * @param {number} err - A libuv error number
 * @param {string} syscall
 * @param {string} address
 * @param {number} [port]
 * @param {string} [additional]
 * @returns {Error}
 */


function exceptionWithHostPort(err, syscall, address, port, additional) {
  // TODO(joyeecheung): We have to use the type-checked
  // getSystemErrorName(err) to guard against invalid arguments from users.
  // This can be replaced with [ code ] = errmap.get(err) when this method
  // is no longer exposed to user land.
  var code = lazyUtil().getSystemErrorName(err);
  var details = '';

  if (port && port > 0) {
    details = " ".concat(address, ":").concat(port);
  } else if (address) {
    details = " ".concat(address);
  }

  if (additional) {
    details += " - Local (".concat(additional, ")");
  }

  var ex = new Error("".concat(syscall, " ").concat(code).concat(details)); // TODO(joyeecheung): errno is supposed to err, like in uvException

  ex.code = ex.errno = code;
  ex.syscall = syscall;
  ex.address = address;

  if (port) {
    ex.port = port;
  }

  Error.captureStackTrace(ex, exceptionWithHostPort);
  return ex;
}
/**
 * @param {number|string} code - A libuv error number or a c-ares error code
 * @param {string} syscall
 * @param {string} [hostname]
 * @returns {Error}
 */


function dnsException(code, syscall, hostname) {
  var message; // FIXME(bnoordhuis) Remove this backwards compatibility nonsense and pass
  // the true error to the user. ENOTFOUND is not even a proper POSIX error!

  if (code === UV_EAI_MEMORY || code === UV_EAI_NODATA || code === UV_EAI_NONAME) {
    code = 'ENOTFOUND'; // Fabricated error name.
  }

  if (typeof code === 'string') {
    // c-ares error code.
    message = "".concat(syscall, " ").concat(code).concat(hostname ? " ".concat(hostname) : '');
  } else {
    // libuv error number
    code = lazyInternalUtil().getSystemErrorName(code);
    message = "".concat(syscall, " ").concat(code);
  } // eslint-disable-next-line no-restricted-syntax


  var ex = new Error(message); // TODO(joyeecheung): errno is supposed to be a number / err, like in
  // uvException.

  ex.errno = code;
  ex.code = code;
  ex.syscall = syscall;

  if (hostname) {
    ex.hostname = hostname;
  }

  Error.captureStackTrace(ex, dnsException);
  return ex;
}

module.exports = exports = {
  dnsException: dnsException,
  errnoException: errnoException,
  exceptionWithHostPort: exceptionWithHostPort,
  message: message,
  Error: makeNodeError(Error),
  TypeError: makeNodeError(TypeError),
  RangeError: makeNodeError(RangeError),
  AssertionError: AssertionError,
  E: E // This is exported only to facilitate testing.

}; // To declare an error message, use the E(sym, val) function above. The sym
// must be an upper case string. The val can be either a function or a string.
// The return value of the function must be a string.
// Examples:
// E('EXAMPLE_KEY1', 'This is the error value');
// E('EXAMPLE_KEY2', (a, b) => return `${a} ${b}`);
//
// Once an error code has been assigned, the code itself MUST NOT change and
// any given error code must never be reused to identify a different error.
//
// Any error code added here should also be added to the documentation
//
// Note: Please try to keep these in alphabetical order

E('ERR_ARG_NOT_ITERABLE', '%s must be iterable');
E('ERR_ASSERTION', function (msg) {
  return msg;
});
E('ERR_ASYNC_CALLBACK', function (name) {
  return "".concat(name, " must be a function");
});
E('ERR_ASYNC_TYPE', function (s) {
  return "Invalid name for async \"type\": ".concat(s);
});
E('ERR_ENCODING_INVALID_ENCODED_DATA', function (enc) {
  return "The encoded data was not valid for encoding ".concat(enc);
});
E('ERR_ENCODING_NOT_SUPPORTED', function (enc) {
  return "The \"".concat(enc, "\" encoding is not supported");
});
E('ERR_FALSY_VALUE_REJECTION', 'Promise was rejected with falsy value');
E('ERR_HTTP2_ALREADY_SHUTDOWN', 'Http2Session is already shutdown or destroyed');
E('ERR_HTTP2_ALTSVC_INVALID_ORIGIN', 'HTTP/2 ALTSVC frames require a valid origin');
E('ERR_HTTP2_ALTSVC_LENGTH', 'HTTP/2 ALTSVC frames are limited to 16382 bytes');
E('ERR_HTTP2_CONNECT_AUTHORITY', ':authority header is required for CONNECT requests');
E('ERR_HTTP2_CONNECT_PATH', 'The :path header is forbidden for CONNECT requests');
E('ERR_HTTP2_CONNECT_SCHEME', 'The :scheme header is forbidden for CONNECT requests');
E('ERR_HTTP2_FRAME_ERROR', function (type, code, id) {
  var msg = "Error sending frame type ".concat(type);
  if (id !== undefined) msg += " for stream ".concat(id);
  msg += " with code ".concat(code);
  return msg;
});
E('ERR_HTTP2_GOAWAY_SESSION', 'New streams cannot be created after receiving a GOAWAY');
E('ERR_HTTP2_HEADERS_AFTER_RESPOND', 'Cannot specify additional headers after response initiated');
E('ERR_HTTP2_HEADERS_OBJECT', 'Headers must be an object');
E('ERR_HTTP2_HEADERS_SENT', 'Response has already been initiated.');
E('ERR_HTTP2_HEADER_REQUIRED', function (name) {
  return "The ".concat(name, " header is required");
});
E('ERR_HTTP2_HEADER_SINGLE_VALUE', function (name) {
  return "Header field \"".concat(name, "\" must have only a single value");
});
E('ERR_HTTP2_INFO_HEADERS_AFTER_RESPOND', 'Cannot send informational headers after the HTTP message has been sent');
E('ERR_HTTP2_INFO_STATUS_NOT_ALLOWED', 'Informational status codes cannot be used');
E('ERR_HTTP2_INVALID_CONNECTION_HEADERS', 'HTTP/1 Connection specific headers are forbidden: "%s"');
E('ERR_HTTP2_INVALID_HEADER_VALUE', 'Invalid value "%s" for header "%s"');
E('ERR_HTTP2_INVALID_INFO_STATUS', function (code) {
  return "Invalid informational status code: ".concat(code);
});
E('ERR_HTTP2_INVALID_ORIGIN', 'HTTP/2 ORIGIN frames require a valid origin');
E('ERR_HTTP2_INVALID_PACKED_SETTINGS_LENGTH', 'Packed settings length must be a multiple of six');
E('ERR_HTTP2_INVALID_PSEUDOHEADER', function (name) {
  return "\"".concat(name, "\" is an invalid pseudoheader or is used incorrectly");
});
E('ERR_HTTP2_INVALID_SESSION', 'The session has been destroyed');
E('ERR_HTTP2_INVALID_SETTING_VALUE', function (name, value) {
  return "Invalid value for setting \"".concat(name, "\": ").concat(value);
});
E('ERR_HTTP2_INVALID_STREAM', 'The stream has been destroyed');
E('ERR_HTTP2_MAX_PENDING_SETTINGS_ACK', function (max) {
  return "Maximum number of pending settings acknowledgements (".concat(max, ")");
});
E('ERR_HTTP2_NESTED_PUSH', 'A push stream cannot initiate another push stream.', Error);
E('ERR_HTTP2_NO_SOCKET_MANIPULATION', 'HTTP/2 sockets should not be directly manipulated (e.g. read and written)');
E('ERR_HTTP2_ORIGIN_LENGTH', 'HTTP/2 ORIGIN frames are limited to 16382 bytes');
E('ERR_HTTP2_OUT_OF_STREAMS', 'No stream ID is available because maximum stream ID has been reached');
E('ERR_HTTP2_PAYLOAD_FORBIDDEN', function (code) {
  return "Responses with ".concat(code, " status must not have a payload");
});
E('ERR_HTTP2_PING_CANCEL', 'HTTP2 ping cancelled');
E('ERR_HTTP2_PING_LENGTH', 'HTTP2 ping payload must be 8 bytes');
E('ERR_HTTP2_PSEUDOHEADER_NOT_ALLOWED', 'Cannot set HTTP/2 pseudo-headers');
E('ERR_HTTP2_PUSH_DISABLED', 'HTTP/2 client has disabled push streams');
E('ERR_HTTP2_SEND_FILE', 'Only regular files can be sent');
E('ERR_HTTP2_SESSION_ERROR', 'Session closed with error code %s');
E('ERR_HTTP2_SETTINGS_CANCEL', 'HTTP2 session settings canceled');
E('ERR_HTTP2_SOCKET_BOUND', 'The socket is already bound to an Http2Session');
E('ERR_HTTP2_SOCKET_UNBOUND', 'The socket has been disconnected from the Http2Session');
E('ERR_HTTP2_STATUS_101', 'HTTP status code 101 (Switching Protocols) is forbidden in HTTP/2');
E('ERR_HTTP2_STATUS_INVALID', 'Invalid status code: %s');
E('ERR_HTTP2_STREAM_CANCEL', 'The pending stream has been canceled');
E('ERR_HTTP2_STREAM_ERROR', 'Stream closed with error code %s');
E('ERR_HTTP2_STREAM_SELF_DEPENDENCY', 'A stream cannot depend on itself');
E('ERR_HTTP2_TRAILERS_ALREADY_SENT', 'Trailing headers have already been sent');
E('ERR_HTTP2_TRAILERS_NOT_READY', 'Trailing headers cannot be sent until after the wantTrailers event is ' + 'emitted');
E('ERR_HTTP2_UNSUPPORTED_PROTOCOL', 'protocol "%s" is unsupported.');
E('ERR_HTTP_HEADERS_SENT', 'Cannot render headers after they are sent to the client');
E('ERR_HTTP_INVALID_CHAR', 'Invalid character in statusMessage.');
E('ERR_HTTP_INVALID_STATUS_CODE', function (originalStatusCode) {
  return "Invalid status code: ".concat(originalStatusCode);
});
E('ERR_HTTP_TRAILER_INVALID', 'Trailers are invalid with this transfer encoding');
E('ERR_INDEX_OUT_OF_RANGE', 'Index out of range');
E('ERR_INVALID_ARG_TYPE', invalidArgType);
E('ERR_INVALID_ARRAY_LENGTH', function (name, len, actual) {
  internalAssert(typeof actual === 'number', 'actual must be a number');
  return "The array \"".concat(name, "\" (length ").concat(actual, ") must be of length ").concat(len, ".");
});
E('ERR_INVALID_ASYNC_ID', function (type, id) {
  return "Invalid ".concat(type, " value: ").concat(id);
});
E('ERR_INVALID_CALLBACK', 'callback must be a function');
E('ERR_INVALID_CHAR', 'Invalid character in %s');
E('ERR_INVALID_FD', function (fd) {
  return "\"fd\" must be a positive integer: ".concat(fd);
});
E('ERR_INVALID_FILE_URL_HOST', 'File URL host %s');
E('ERR_INVALID_FILE_URL_PATH', 'File URL path %s');
E('ERR_INVALID_HANDLE_TYPE', 'This handle type cannot be sent');
E('ERR_INVALID_HTTP_TOKEN', '%s must be a valid HTTP token ["%s"]');
E('ERR_INVALID_OPT_VALUE', function (name, value) {
  return "The value \"".concat(String(value), "\" is invalid for option \"").concat(name, "\"");
});
E('ERR_INVALID_PERFORMANCE_MARK', 'The "%s" performance mark has not been set');
E('ERR_INVALID_PROTOCOL', function (protocol, expectedProtocol) {
  return "Protocol \"".concat(protocol, "\" not supported. Expected \"").concat(expectedProtocol, "\"");
});
E('ERR_INVALID_SYNC_FORK_INPUT', function (value) {
  return 'Asynchronous forks do not support Buffer, Uint8Array or string' + "input: ".concat(value);
});
E('ERR_INVALID_THIS', 'Value of "this" must be of type %s');
E('ERR_INVALID_TUPLE', '%s must be an iterable %s tuple');
E('ERR_INVALID_URL', 'Invalid URL: %s');
E('ERR_INVALID_URL_SCHEME', function (expected) {
  return "The URL must be ".concat(oneOf(expected, 'scheme'));
});
E('ERR_IPC_CHANNEL_CLOSED', 'channel closed');
E('ERR_IPC_DISCONNECTED', 'IPC channel is already disconnected');
E('ERR_IPC_ONE_PIPE', 'Child process can have only one IPC pipe');
E('ERR_IPC_SYNC_FORK', 'IPC cannot be used with synchronous forks');
E('ERR_MISSING_ARGS', missingArgs);
E('ERR_MISSING_DYNAMIC_INSTANTIATE_HOOK', 'The ES Module loader may not return a format of \'dynamic\' when no ' + 'dynamicInstantiate function was provided');
E('ERR_MISSING_MODULE', 'Cannot find module %s');
E('ERR_MODULE_RESOLUTION_LEGACY', '%s not found by import in %s.' + ' Legacy behavior in require() would have found it at %s');
E('ERR_NAPI_CONS_FUNCTION', 'Constructor must be a function');
E('ERR_NAPI_CONS_PROTOTYPE_OBJECT', 'Constructor.prototype must be an object');
E('ERR_NAPI_INVALID_DATAVIEW_ARGS', 'byte_offset + byte_length should be less than or eqaul to the size in ' + 'bytes of the array passed in');
E('ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT', 'start offset of %s should be a ' + 'multiple of %s');
E('ERR_NAPI_INVALID_TYPEDARRAY_LENGTH', 'Invalid typed array length');
E('ERR_NO_CRYPTO', 'Node.js is not compiled with OpenSSL crypto support');
E('ERR_NO_ICU', '%s is not supported on Node.js compiled without ICU');
E('ERR_OUT_OF_RANGE', 'The "%s" argument is out of range');
E('ERR_PARSE_HISTORY_DATA', 'Could not parse history data in %s');
E('ERR_REQUIRE_ESM', 'Must use import to load ES Module: %s');
E('ERR_SOCKET_ALREADY_BOUND', 'Socket is already bound');
E('ERR_SOCKET_BAD_BUFFER_SIZE', 'Buffer size must be a positive integer');
E('ERR_SOCKET_BAD_PORT', 'Port should be > 0 and < 65536');
E('ERR_SOCKET_BAD_TYPE', 'Bad socket type specified. Valid types are: udp4, udp6');
E('ERR_SOCKET_BUFFER_SIZE', function (reason) {
  return "Could not get or set buffer size: ".concat(reason);
});
E('ERR_SOCKET_CANNOT_SEND', 'Unable to send data');
E('ERR_SOCKET_DGRAM_NOT_RUNNING', 'Not running');
E('ERR_UNKNOWN_BUILTIN_MODULE', function (id) {
  return "No such built-in module: ".concat(id);
});
E('ERR_UNKNOWN_FILE_EXTENSION', 'Unknown file extension: %s');
E('ERR_UNKNOWN_MODULE_FORMAT', 'Unknown module format: %s');
E('ERR_UNKNOWN_SIGNAL', function (signal) {
  return "Unknown signal: ".concat(signal);
});
E('ERR_UNKNOWN_STDIN_TYPE', 'Unknown stdin file type');
E('ERR_UNKNOWN_STREAM_TYPE', 'Unknown stream file type');
E('ERR_V8BREAKITERATOR', 'Full ICU data not installed. ' + 'See https://github.com/nodejs/node/wiki/Intl');
E('ERR_VALID_PERFORMANCE_ENTRY_TYPE', 'At least one valid performance entry type is required');
E('ERR_VALUE_OUT_OF_RANGE', function (start, end, value) {
  return "The value of \"".concat(start, "\" must be ").concat(end, ". Received \"").concat(value, "\"");
});

function invalidArgType(name, expected, actual) {
  internalAssert(name, 'name is required');
  var msg = "The \"".concat(name, "\" argument must be ").concat(oneOf(expected, 'type'));

  if (arguments.length >= 3) {
    msg += ". Received type ".concat(actual !== null ? _typeof(actual) : 'null');
  }

  return msg;
}

function missingArgs() {
  for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
    args[_key2] = arguments[_key2];
  }

  internalAssert(args.length > 0, 'At least one arg needs to be specified');
  var msg = 'The ';
  var len = args.length;
  args = args.map(function (a) {
    return "\"".concat(a, "\"");
  });

  switch (len) {
    case 1:
      msg += "".concat(args[0], " argument");
      break;

    case 2:
      msg += "".concat(args[0], " and ").concat(args[1], " arguments");
      break;

    default:
      msg += args.slice(0, len - 1).join(', ');
      msg += ", and ".concat(args[len - 1], " arguments");
      break;
  }

  return "".concat(msg, " must be specified");
}

function oneOf(expected, thing) {
  internalAssert(expected, 'expected is required');
  internalAssert(typeof thing === 'string', 'thing is required');

  if (Array.isArray(expected)) {
    var len = expected.length;
    internalAssert(len > 0, 'At least one expected value needs to be specified');
    expected = expected.map(function (i) {
      return String(i);
    });

    if (len > 2) {
      return "one of ".concat(thing, " ").concat(expected.slice(0, len - 1).join(', '), ", or ") + expected[len - 1];
    } else if (len === 2) {
      return "one of ".concat(thing, " ").concat(expected[0], " or ").concat(expected[1]);
    } else {
      return "of ".concat(thing, " ").concat(expected[0]);
    }
  } else {
    return "of ".concat(thing, " ").concat(String(expected));
  }
}