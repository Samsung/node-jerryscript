'use strict';

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _wrapNativeSuper(Class) { var _cache = typeof Map === "function" ? new Map() : undefined; _wrapNativeSuper = function _wrapNativeSuper(Class) { if (Class === null || !_isNativeFunction(Class)) return Class; if (typeof Class !== "function") { throw new TypeError("Super expression must either be null or a function"); } if (typeof _cache !== "undefined") { if (_cache.has(Class)) return _cache.get(Class); _cache.set(Class, Wrapper); } function Wrapper() { return _construct(Class, arguments, _getPrototypeOf(this).constructor); } Wrapper.prototype = Object.create(Class.prototype, { constructor: { value: Wrapper, enumerable: false, writable: true, configurable: true } }); return _setPrototypeOf(Wrapper, Class); }; return _wrapNativeSuper(Class); }

function isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

function _construct(Parent, args, Class) { if (isNativeReflectConstruct()) { _construct = Reflect.construct; } else { _construct = function _construct(Parent, args, Class) { var a = [null]; a.push.apply(a, args); var Constructor = Function.bind.apply(Parent, a); var instance = new Constructor(); if (Class) _setPrototypeOf(instance, Class.prototype); return instance; }; } return _construct.apply(null, arguments); }

function _isNativeFunction(fn) { return Function.toString.call(fn).indexOf("[native code]") !== -1; }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var binding = process.binding('http2');

var errors = require('internal/errors');

var kSocket = Symbol('socket');
var _binding$constants = binding.constants,
    NGHTTP2_SESSION_CLIENT = _binding$constants.NGHTTP2_SESSION_CLIENT,
    NGHTTP2_SESSION_SERVER = _binding$constants.NGHTTP2_SESSION_SERVER,
    HTTP2_HEADER_STATUS = _binding$constants.HTTP2_HEADER_STATUS,
    HTTP2_HEADER_METHOD = _binding$constants.HTTP2_HEADER_METHOD,
    HTTP2_HEADER_AUTHORITY = _binding$constants.HTTP2_HEADER_AUTHORITY,
    HTTP2_HEADER_SCHEME = _binding$constants.HTTP2_HEADER_SCHEME,
    HTTP2_HEADER_PATH = _binding$constants.HTTP2_HEADER_PATH,
    HTTP2_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS = _binding$constants.HTTP2_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS,
    HTTP2_HEADER_ACCESS_CONTROL_MAX_AGE = _binding$constants.HTTP2_HEADER_ACCESS_CONTROL_MAX_AGE,
    HTTP2_HEADER_ACCESS_CONTROL_REQUEST_METHOD = _binding$constants.HTTP2_HEADER_ACCESS_CONTROL_REQUEST_METHOD,
    HTTP2_HEADER_AGE = _binding$constants.HTTP2_HEADER_AGE,
    HTTP2_HEADER_AUTHORIZATION = _binding$constants.HTTP2_HEADER_AUTHORIZATION,
    HTTP2_HEADER_CONTENT_ENCODING = _binding$constants.HTTP2_HEADER_CONTENT_ENCODING,
    HTTP2_HEADER_CONTENT_LANGUAGE = _binding$constants.HTTP2_HEADER_CONTENT_LANGUAGE,
    HTTP2_HEADER_CONTENT_LENGTH = _binding$constants.HTTP2_HEADER_CONTENT_LENGTH,
    HTTP2_HEADER_CONTENT_LOCATION = _binding$constants.HTTP2_HEADER_CONTENT_LOCATION,
    HTTP2_HEADER_CONTENT_MD5 = _binding$constants.HTTP2_HEADER_CONTENT_MD5,
    HTTP2_HEADER_CONTENT_RANGE = _binding$constants.HTTP2_HEADER_CONTENT_RANGE,
    HTTP2_HEADER_CONTENT_TYPE = _binding$constants.HTTP2_HEADER_CONTENT_TYPE,
    HTTP2_HEADER_COOKIE = _binding$constants.HTTP2_HEADER_COOKIE,
    HTTP2_HEADER_DATE = _binding$constants.HTTP2_HEADER_DATE,
    HTTP2_HEADER_DNT = _binding$constants.HTTP2_HEADER_DNT,
    HTTP2_HEADER_ETAG = _binding$constants.HTTP2_HEADER_ETAG,
    HTTP2_HEADER_EXPIRES = _binding$constants.HTTP2_HEADER_EXPIRES,
    HTTP2_HEADER_FROM = _binding$constants.HTTP2_HEADER_FROM,
    HTTP2_HEADER_IF_MATCH = _binding$constants.HTTP2_HEADER_IF_MATCH,
    HTTP2_HEADER_IF_NONE_MATCH = _binding$constants.HTTP2_HEADER_IF_NONE_MATCH,
    HTTP2_HEADER_IF_MODIFIED_SINCE = _binding$constants.HTTP2_HEADER_IF_MODIFIED_SINCE,
    HTTP2_HEADER_IF_RANGE = _binding$constants.HTTP2_HEADER_IF_RANGE,
    HTTP2_HEADER_IF_UNMODIFIED_SINCE = _binding$constants.HTTP2_HEADER_IF_UNMODIFIED_SINCE,
    HTTP2_HEADER_LAST_MODIFIED = _binding$constants.HTTP2_HEADER_LAST_MODIFIED,
    HTTP2_HEADER_LOCATION = _binding$constants.HTTP2_HEADER_LOCATION,
    HTTP2_HEADER_MAX_FORWARDS = _binding$constants.HTTP2_HEADER_MAX_FORWARDS,
    HTTP2_HEADER_PROXY_AUTHORIZATION = _binding$constants.HTTP2_HEADER_PROXY_AUTHORIZATION,
    HTTP2_HEADER_RANGE = _binding$constants.HTTP2_HEADER_RANGE,
    HTTP2_HEADER_REFERER = _binding$constants.HTTP2_HEADER_REFERER,
    HTTP2_HEADER_RETRY_AFTER = _binding$constants.HTTP2_HEADER_RETRY_AFTER,
    HTTP2_HEADER_SET_COOKIE = _binding$constants.HTTP2_HEADER_SET_COOKIE,
    HTTP2_HEADER_TK = _binding$constants.HTTP2_HEADER_TK,
    HTTP2_HEADER_UPGRADE_INSECURE_REQUESTS = _binding$constants.HTTP2_HEADER_UPGRADE_INSECURE_REQUESTS,
    HTTP2_HEADER_USER_AGENT = _binding$constants.HTTP2_HEADER_USER_AGENT,
    HTTP2_HEADER_X_CONTENT_TYPE_OPTIONS = _binding$constants.HTTP2_HEADER_X_CONTENT_TYPE_OPTIONS,
    HTTP2_HEADER_CONNECTION = _binding$constants.HTTP2_HEADER_CONNECTION,
    HTTP2_HEADER_UPGRADE = _binding$constants.HTTP2_HEADER_UPGRADE,
    HTTP2_HEADER_HTTP2_SETTINGS = _binding$constants.HTTP2_HEADER_HTTP2_SETTINGS,
    HTTP2_HEADER_TE = _binding$constants.HTTP2_HEADER_TE,
    HTTP2_HEADER_TRANSFER_ENCODING = _binding$constants.HTTP2_HEADER_TRANSFER_ENCODING,
    HTTP2_HEADER_HOST = _binding$constants.HTTP2_HEADER_HOST,
    HTTP2_HEADER_KEEP_ALIVE = _binding$constants.HTTP2_HEADER_KEEP_ALIVE,
    HTTP2_HEADER_PROXY_CONNECTION = _binding$constants.HTTP2_HEADER_PROXY_CONNECTION,
    HTTP2_METHOD_DELETE = _binding$constants.HTTP2_METHOD_DELETE,
    HTTP2_METHOD_GET = _binding$constants.HTTP2_METHOD_GET,
    HTTP2_METHOD_HEAD = _binding$constants.HTTP2_METHOD_HEAD; // This set is defined strictly by the HTTP/2 specification. Only
// :-prefixed headers defined by that specification may be added to
// this set.

var kValidPseudoHeaders = new Set([HTTP2_HEADER_STATUS, HTTP2_HEADER_METHOD, HTTP2_HEADER_AUTHORITY, HTTP2_HEADER_SCHEME, HTTP2_HEADER_PATH]); // This set contains headers that are permitted to have only a single
// value. Multiple instances must not be specified.

var kSingleValueHeaders = new Set([HTTP2_HEADER_STATUS, HTTP2_HEADER_METHOD, HTTP2_HEADER_AUTHORITY, HTTP2_HEADER_SCHEME, HTTP2_HEADER_PATH, HTTP2_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS, HTTP2_HEADER_ACCESS_CONTROL_MAX_AGE, HTTP2_HEADER_ACCESS_CONTROL_REQUEST_METHOD, HTTP2_HEADER_AGE, HTTP2_HEADER_AUTHORIZATION, HTTP2_HEADER_CONTENT_ENCODING, HTTP2_HEADER_CONTENT_LANGUAGE, HTTP2_HEADER_CONTENT_LENGTH, HTTP2_HEADER_CONTENT_LOCATION, HTTP2_HEADER_CONTENT_MD5, HTTP2_HEADER_CONTENT_RANGE, HTTP2_HEADER_CONTENT_TYPE, HTTP2_HEADER_DATE, HTTP2_HEADER_DNT, HTTP2_HEADER_ETAG, HTTP2_HEADER_EXPIRES, HTTP2_HEADER_FROM, HTTP2_HEADER_IF_MATCH, HTTP2_HEADER_IF_MODIFIED_SINCE, HTTP2_HEADER_IF_NONE_MATCH, HTTP2_HEADER_IF_RANGE, HTTP2_HEADER_IF_UNMODIFIED_SINCE, HTTP2_HEADER_LAST_MODIFIED, HTTP2_HEADER_LOCATION, HTTP2_HEADER_MAX_FORWARDS, HTTP2_HEADER_PROXY_AUTHORIZATION, HTTP2_HEADER_RANGE, HTTP2_HEADER_REFERER, HTTP2_HEADER_RETRY_AFTER, HTTP2_HEADER_TK, HTTP2_HEADER_UPGRADE_INSECURE_REQUESTS, HTTP2_HEADER_USER_AGENT, HTTP2_HEADER_X_CONTENT_TYPE_OPTIONS]); // The HTTP methods in this set are specifically defined as assigning no
// meaning to the request payload. By default, unless the user explicitly
// overrides the endStream option on the request method, the endStream
// option will be defaulted to true when these methods are used.

var kNoPayloadMethods = new Set([HTTP2_METHOD_DELETE, HTTP2_METHOD_GET, HTTP2_METHOD_HEAD]); // The following ArrayBuffer instances are used to share memory more efficiently
// with the native binding side for a number of methods. These are not intended
// to be used directly by users in any way. The ArrayBuffers are created on
// the native side with values that are filled in on demand, the js code then
// reads those values out. The set of IDX constants that follow identify the
// relevant data positions within these buffers.

var settingsBuffer = binding.settingsBuffer,
    optionsBuffer = binding.optionsBuffer; // Note that Float64Array is used here because there is no Int64Array available
// and these deal with numbers that can be beyond the range of Uint32 and Int32.
// The values set on the native side will always be integers. This is not a
// unique example of this, this pattern can be found in use in other parts of
// Node.js core as a performance optimization.

var sessionState = binding.sessionState,
    streamState = binding.streamState;
var IDX_SETTINGS_HEADER_TABLE_SIZE = 0;
var IDX_SETTINGS_ENABLE_PUSH = 1;
var IDX_SETTINGS_INITIAL_WINDOW_SIZE = 2;
var IDX_SETTINGS_MAX_FRAME_SIZE = 3;
var IDX_SETTINGS_MAX_CONCURRENT_STREAMS = 4;
var IDX_SETTINGS_MAX_HEADER_LIST_SIZE = 5;
var IDX_SETTINGS_FLAGS = 6;
var IDX_SESSION_STATE_EFFECTIVE_LOCAL_WINDOW_SIZE = 0;
var IDX_SESSION_STATE_EFFECTIVE_RECV_DATA_LENGTH = 1;
var IDX_SESSION_STATE_NEXT_STREAM_ID = 2;
var IDX_SESSION_STATE_LOCAL_WINDOW_SIZE = 3;
var IDX_SESSION_STATE_LAST_PROC_STREAM_ID = 4;
var IDX_SESSION_STATE_REMOTE_WINDOW_SIZE = 5;
var IDX_SESSION_STATE_OUTBOUND_QUEUE_SIZE = 6;
var IDX_SESSION_STATE_HD_DEFLATE_DYNAMIC_TABLE_SIZE = 7;
var IDX_SESSION_STATE_HD_INFLATE_DYNAMIC_TABLE_SIZE = 8;
var IDX_STREAM_STATE = 0;
var IDX_STREAM_STATE_WEIGHT = 1;
var IDX_STREAM_STATE_SUM_DEPENDENCY_WEIGHT = 2;
var IDX_STREAM_STATE_LOCAL_CLOSE = 3;
var IDX_STREAM_STATE_REMOTE_CLOSE = 4;
var IDX_STREAM_STATE_LOCAL_WINDOW_SIZE = 5;
var IDX_OPTIONS_MAX_DEFLATE_DYNAMIC_TABLE_SIZE = 0;
var IDX_OPTIONS_MAX_RESERVED_REMOTE_STREAMS = 1;
var IDX_OPTIONS_MAX_SEND_HEADER_BLOCK_LENGTH = 2;
var IDX_OPTIONS_PEER_MAX_CONCURRENT_STREAMS = 3;
var IDX_OPTIONS_PADDING_STRATEGY = 4;
var IDX_OPTIONS_MAX_HEADER_LIST_PAIRS = 5;
var IDX_OPTIONS_MAX_OUTSTANDING_PINGS = 6;
var IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS = 7;
var IDX_OPTIONS_MAX_SESSION_MEMORY = 8;
var IDX_OPTIONS_FLAGS = 9;

function updateOptionsBuffer(options) {
  var flags = 0;

  if (typeof options.maxDeflateDynamicTableSize === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_DEFLATE_DYNAMIC_TABLE_SIZE;
    optionsBuffer[IDX_OPTIONS_MAX_DEFLATE_DYNAMIC_TABLE_SIZE] = options.maxDeflateDynamicTableSize;
  }

  if (typeof options.maxReservedRemoteStreams === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_RESERVED_REMOTE_STREAMS;
    optionsBuffer[IDX_OPTIONS_MAX_RESERVED_REMOTE_STREAMS] = options.maxReservedRemoteStreams;
  }

  if (typeof options.maxSendHeaderBlockLength === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_SEND_HEADER_BLOCK_LENGTH;
    optionsBuffer[IDX_OPTIONS_MAX_SEND_HEADER_BLOCK_LENGTH] = options.maxSendHeaderBlockLength;
  }

  if (typeof options.peerMaxConcurrentStreams === 'number') {
    flags |= 1 << IDX_OPTIONS_PEER_MAX_CONCURRENT_STREAMS;
    optionsBuffer[IDX_OPTIONS_PEER_MAX_CONCURRENT_STREAMS] = options.peerMaxConcurrentStreams;
  }

  if (typeof options.paddingStrategy === 'number') {
    flags |= 1 << IDX_OPTIONS_PADDING_STRATEGY;
    optionsBuffer[IDX_OPTIONS_PADDING_STRATEGY] = options.paddingStrategy;
  }

  if (typeof options.maxHeaderListPairs === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_HEADER_LIST_PAIRS;
    optionsBuffer[IDX_OPTIONS_MAX_HEADER_LIST_PAIRS] = options.maxHeaderListPairs;
  }

  if (typeof options.maxOutstandingPings === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_OUTSTANDING_PINGS;
    optionsBuffer[IDX_OPTIONS_MAX_OUTSTANDING_PINGS] = options.maxOutstandingPings;
  }

  if (typeof options.maxOutstandingSettings === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS;
    optionsBuffer[IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS] = Math.max(1, options.maxOutstandingSettings);
  }

  if (typeof options.maxSessionMemory === 'number') {
    flags |= 1 << IDX_OPTIONS_MAX_SESSION_MEMORY;
    optionsBuffer[IDX_OPTIONS_MAX_SESSION_MEMORY] = Math.max(1, options.maxSessionMemory);
  }

  optionsBuffer[IDX_OPTIONS_FLAGS] = flags;
}

function getDefaultSettings() {
  settingsBuffer[IDX_SETTINGS_FLAGS] = 0;
  binding.refreshDefaultSettings();
  var holder = Object.create(null);
  var flags = settingsBuffer[IDX_SETTINGS_FLAGS];

  if ((flags & 1 << IDX_SETTINGS_HEADER_TABLE_SIZE) === 1 << IDX_SETTINGS_HEADER_TABLE_SIZE) {
    holder.headerTableSize = settingsBuffer[IDX_SETTINGS_HEADER_TABLE_SIZE];
  }

  if ((flags & 1 << IDX_SETTINGS_ENABLE_PUSH) === 1 << IDX_SETTINGS_ENABLE_PUSH) {
    holder.enablePush = settingsBuffer[IDX_SETTINGS_ENABLE_PUSH] === 1;
  }

  if ((flags & 1 << IDX_SETTINGS_INITIAL_WINDOW_SIZE) === 1 << IDX_SETTINGS_INITIAL_WINDOW_SIZE) {
    holder.initialWindowSize = settingsBuffer[IDX_SETTINGS_INITIAL_WINDOW_SIZE];
  }

  if ((flags & 1 << IDX_SETTINGS_MAX_FRAME_SIZE) === 1 << IDX_SETTINGS_MAX_FRAME_SIZE) {
    holder.maxFrameSize = settingsBuffer[IDX_SETTINGS_MAX_FRAME_SIZE];
  }

  if ((flags & 1 << IDX_SETTINGS_MAX_CONCURRENT_STREAMS) === 1 << IDX_SETTINGS_MAX_CONCURRENT_STREAMS) {
    holder.maxConcurrentStreams = settingsBuffer[IDX_SETTINGS_MAX_CONCURRENT_STREAMS];
  }

  if ((flags & 1 << IDX_SETTINGS_MAX_HEADER_LIST_SIZE) === 1 << IDX_SETTINGS_MAX_HEADER_LIST_SIZE) {
    holder.maxHeaderListSize = settingsBuffer[IDX_SETTINGS_MAX_HEADER_LIST_SIZE];
  }

  return holder;
} // remote is a boolean. true to fetch remote settings, false to fetch local.
// this is only called internally


function getSettings(session, remote) {
  if (remote) session.remoteSettings();else session.localSettings();
  return {
    headerTableSize: settingsBuffer[IDX_SETTINGS_HEADER_TABLE_SIZE],
    enablePush: !!settingsBuffer[IDX_SETTINGS_ENABLE_PUSH],
    initialWindowSize: settingsBuffer[IDX_SETTINGS_INITIAL_WINDOW_SIZE],
    maxFrameSize: settingsBuffer[IDX_SETTINGS_MAX_FRAME_SIZE],
    maxConcurrentStreams: settingsBuffer[IDX_SETTINGS_MAX_CONCURRENT_STREAMS],
    maxHeaderListSize: settingsBuffer[IDX_SETTINGS_MAX_HEADER_LIST_SIZE]
  };
}

function updateSettingsBuffer(settings) {
  var flags = 0;

  if (typeof settings.headerTableSize === 'number') {
    flags |= 1 << IDX_SETTINGS_HEADER_TABLE_SIZE;
    settingsBuffer[IDX_SETTINGS_HEADER_TABLE_SIZE] = settings.headerTableSize;
  }

  if (typeof settings.maxConcurrentStreams === 'number') {
    flags |= 1 << IDX_SETTINGS_MAX_CONCURRENT_STREAMS;
    settingsBuffer[IDX_SETTINGS_MAX_CONCURRENT_STREAMS] = settings.maxConcurrentStreams;
  }

  if (typeof settings.initialWindowSize === 'number') {
    flags |= 1 << IDX_SETTINGS_INITIAL_WINDOW_SIZE;
    settingsBuffer[IDX_SETTINGS_INITIAL_WINDOW_SIZE] = settings.initialWindowSize;
  }

  if (typeof settings.maxFrameSize === 'number') {
    flags |= 1 << IDX_SETTINGS_MAX_FRAME_SIZE;
    settingsBuffer[IDX_SETTINGS_MAX_FRAME_SIZE] = settings.maxFrameSize;
  }

  if (typeof settings.maxHeaderListSize === 'number') {
    flags |= 1 << IDX_SETTINGS_MAX_HEADER_LIST_SIZE;
    settingsBuffer[IDX_SETTINGS_MAX_HEADER_LIST_SIZE] = settings.maxHeaderListSize;
  }

  if (typeof settings.enablePush === 'boolean') {
    flags |= 1 << IDX_SETTINGS_ENABLE_PUSH;
    settingsBuffer[IDX_SETTINGS_ENABLE_PUSH] = Number(settings.enablePush);
  }

  settingsBuffer[IDX_SETTINGS_FLAGS] = flags;
}

function getSessionState(session) {
  session.refreshState();
  return {
    effectiveLocalWindowSize: sessionState[IDX_SESSION_STATE_EFFECTIVE_LOCAL_WINDOW_SIZE],
    effectiveRecvDataLength: sessionState[IDX_SESSION_STATE_EFFECTIVE_RECV_DATA_LENGTH],
    nextStreamID: sessionState[IDX_SESSION_STATE_NEXT_STREAM_ID],
    localWindowSize: sessionState[IDX_SESSION_STATE_LOCAL_WINDOW_SIZE],
    lastProcStreamID: sessionState[IDX_SESSION_STATE_LAST_PROC_STREAM_ID],
    remoteWindowSize: sessionState[IDX_SESSION_STATE_REMOTE_WINDOW_SIZE],
    outboundQueueSize: sessionState[IDX_SESSION_STATE_OUTBOUND_QUEUE_SIZE],
    deflateDynamicTableSize: sessionState[IDX_SESSION_STATE_HD_DEFLATE_DYNAMIC_TABLE_SIZE],
    inflateDynamicTableSize: sessionState[IDX_SESSION_STATE_HD_INFLATE_DYNAMIC_TABLE_SIZE]
  };
}

function getStreamState(stream) {
  stream.refreshState();
  return {
    state: streamState[IDX_STREAM_STATE],
    weight: streamState[IDX_STREAM_STATE_WEIGHT],
    sumDependencyWeight: streamState[IDX_STREAM_STATE_SUM_DEPENDENCY_WEIGHT],
    localClose: streamState[IDX_STREAM_STATE_LOCAL_CLOSE],
    remoteClose: streamState[IDX_STREAM_STATE_REMOTE_CLOSE],
    localWindowSize: streamState[IDX_STREAM_STATE_LOCAL_WINDOW_SIZE]
  };
}

function isIllegalConnectionSpecificHeader(name, value) {
  switch (name) {
    case HTTP2_HEADER_CONNECTION:
    case HTTP2_HEADER_UPGRADE:
    case HTTP2_HEADER_HOST:
    case HTTP2_HEADER_HTTP2_SETTINGS:
    case HTTP2_HEADER_KEEP_ALIVE:
    case HTTP2_HEADER_PROXY_CONNECTION:
    case HTTP2_HEADER_TRANSFER_ENCODING:
      return true;

    case HTTP2_HEADER_TE:
      return value !== 'trailers';

    default:
      return false;
  }
}

function assertValidPseudoHeader(key) {
  if (!kValidPseudoHeaders.has(key)) {
    var err = new errors.Error('ERR_HTTP2_INVALID_PSEUDOHEADER', key);
    Error.captureStackTrace(err, assertValidPseudoHeader);
    return err;
  }
}

function assertValidPseudoHeaderResponse(key) {
  if (key !== ':status') {
    var err = new errors.Error('ERR_HTTP2_INVALID_PSEUDOHEADER', key);
    Error.captureStackTrace(err, assertValidPseudoHeaderResponse);
    return err;
  }
}

function assertValidPseudoHeaderTrailer(key) {
  var err = new errors.Error('ERR_HTTP2_INVALID_PSEUDOHEADER', key);
  Error.captureStackTrace(err, assertValidPseudoHeaderTrailer);
  return err;
}

function mapToHeaders(map) {
  var assertValuePseudoHeader = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : assertValidPseudoHeader;
  var ret = '';
  var count = 0;
  var keys = Object.keys(map);
  var singles = new Set();

  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    var value = map[key];
    if (value === undefined || key === '') continue;
    key = key.toLowerCase();
    var isSingleValueHeader = kSingleValueHeaders.has(key);
    var isArray = Array.isArray(value);

    if (isArray) {
      switch (value.length) {
        case 0:
          continue;

        case 1:
          value = String(value[0]);
          isArray = false;
          break;

        default:
          if (isSingleValueHeader) return new errors.Error('ERR_HTTP2_HEADER_SINGLE_VALUE', key);
      }
    } else {
      value = String(value);
    }

    if (isSingleValueHeader) {
      if (singles.has(key)) return new errors.Error('ERR_HTTP2_HEADER_SINGLE_VALUE', key);
      singles.add(key);
    }

    if (key[0] === ':') {
      var err = assertValuePseudoHeader(key);
      if (err !== undefined) return err;
      ret = "".concat(key, "\0").concat(value, "\0").concat(ret);
      count++;
    } else {
      if (isIllegalConnectionSpecificHeader(key, value)) {
        return new errors.Error('ERR_HTTP2_INVALID_CONNECTION_HEADERS', key);
      }

      if (isArray) {
        for (var k = 0; k < value.length; k++) {
          var val = String(value[k]);
          ret += "".concat(key, "\0").concat(val, "\0");
        }

        count += value.length;
      } else {
        ret += "".concat(key, "\0").concat(value, "\0");
        count++;
      }
    }
  }

  return [ret, count];
}

var NghttpError =
/*#__PURE__*/
function (_Error) {
  _inherits(NghttpError, _Error);

  function NghttpError(ret) {
    var _this;

    _classCallCheck(this, NghttpError);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(NghttpError).call(this, binding.nghttp2ErrorString(ret)));
    _this.code = 'ERR_HTTP2_ERROR';
    _this.name = 'Error [ERR_HTTP2_ERROR]';
    _this.errno = ret;
    return _this;
  }

  return NghttpError;
}(_wrapNativeSuper(Error));

function assertIsObject(value, name) {
  var types = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 'object';

  if (value !== undefined && (value === null || _typeof(value) !== 'object' || Array.isArray(value))) {
    var err = new errors.TypeError('ERR_INVALID_ARG_TYPE', name, types);
    Error.captureStackTrace(err, assertIsObject);
    throw err;
  }
}

function assertWithinRange(name, value) {
  var min = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 0;
  var max = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : Infinity;

  if (value !== undefined && (typeof value !== 'number' || value < min || value > max)) {
    var err = new errors.RangeError('ERR_HTTP2_INVALID_SETTING_VALUE', name, value);
    err.min = min;
    err.max = max;
    err.actual = value;
    Error.captureStackTrace(err, assertWithinRange);
    throw err;
  }
}

function toHeaderObject(headers) {
  var obj = Object.create(null);

  for (var n = 0; n < headers.length; n = n + 2) {
    var name = headers[n];
    var value = headers[n + 1];
    if (name === HTTP2_HEADER_STATUS) value |= 0;
    var existing = obj[name];

    if (existing === undefined) {
      obj[name] = name === HTTP2_HEADER_SET_COOKIE ? [value] : value;
    } else if (!kSingleValueHeaders.has(name)) {
      switch (name) {
        case HTTP2_HEADER_COOKIE:
          // https://tools.ietf.org/html/rfc7540#section-8.1.2.5
          // "...If there are multiple Cookie header fields after decompression,
          //  these MUST be concatenated into a single octet string using the
          //  two-octet delimiter of 0x3B, 0x20 (the ASCII string "; ") before
          //  being passed into a non-HTTP/2 context."
          obj[name] = "".concat(existing, "; ").concat(value);
          break;

        case HTTP2_HEADER_SET_COOKIE:
          // https://tools.ietf.org/html/rfc7230#section-3.2.2
          // "Note: In practice, the "Set-Cookie" header field ([RFC6265]) often
          // appears multiple times in a response message and does not use the
          // list syntax, violating the above requirements on multiple header
          // fields with the same name.  Since it cannot be combined into a
          // single field-value, recipients ought to handle "Set-Cookie" as a
          // special case while processing header fields."
          existing.push(value);
          break;

        default:
          // https://tools.ietf.org/html/rfc7230#section-3.2.2
          // "A recipient MAY combine multiple header fields with the same field
          // name into one "field-name: field-value" pair, without changing the
          // semantics of the message, by appending each subsequent field value
          // to the combined field value in order, separated by a comma."
          obj[name] = "".concat(existing, ", ").concat(value);
          break;
      }
    }
  }

  return obj;
}

function isPayloadMeaningless(method) {
  return kNoPayloadMethods.has(method);
}

function sessionName(type) {
  switch (type) {
    case NGHTTP2_SESSION_CLIENT:
      return 'client';

    case NGHTTP2_SESSION_SERVER:
      return 'server';

    default:
      return '<invalid>';
  }
}

module.exports = {
  assertIsObject: assertIsObject,
  assertValidPseudoHeaderResponse: assertValidPseudoHeaderResponse,
  assertValidPseudoHeaderTrailer: assertValidPseudoHeaderTrailer,
  assertWithinRange: assertWithinRange,
  getDefaultSettings: getDefaultSettings,
  getSessionState: getSessionState,
  getSettings: getSettings,
  getStreamState: getStreamState,
  isPayloadMeaningless: isPayloadMeaningless,
  kSocket: kSocket,
  mapToHeaders: mapToHeaders,
  NghttpError: NghttpError,
  sessionName: sessionName,
  toHeaderObject: toHeaderObject,
  updateOptionsBuffer: updateOptionsBuffer,
  updateSettingsBuffer: updateSettingsBuffer
};