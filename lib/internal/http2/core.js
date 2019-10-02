'use strict';
/* eslint-disable no-use-before-define */

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance"); }

function _iterableToArray(iter) { if (Symbol.iterator in Object(iter) || Object.prototype.toString.call(iter) === "[object Arguments]") return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = new Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _require = require('internal/util'),
    assertCrypto = _require.assertCrypto,
    kInspect = _require.customInspectSymbol,
    promisify = _require.promisify;

assertCrypto();

var assert = require('assert');

var _require2 = require('buffer'),
    Buffer = _require2.Buffer;

var EventEmitter = require('events');

var fs = require('fs');

var http = require('http');

var net = require('net');

var _require3 = require('stream'),
    Duplex = _require3.Duplex;

var _require4 = require('timers'),
    _unrefActive = _require4._unrefActive,
    enroll = _require4.enroll,
    unenroll = _require4.unenroll;

var tls = require('tls');

var _require5 = require('url'),
    URL = _require5.URL;

var util = require('util');

var _require6 = require('_stream_wrap'),
    StreamWrap = _require6.StreamWrap;

var errors = require('internal/errors');

var _require7 = require('internal/http'),
    utcDate = _require7.utcDate;

var _require8 = require('internal/http2/compat'),
    onServerStream = _require8.onServerStream,
    Http2ServerRequest = _require8.Http2ServerRequest,
    Http2ServerResponse = _require8.Http2ServerResponse;

var _require9 = require('internal/http2/util'),
    assertIsObject = _require9.assertIsObject,
    assertValidPseudoHeaderResponse = _require9.assertValidPseudoHeaderResponse,
    assertValidPseudoHeaderTrailer = _require9.assertValidPseudoHeaderTrailer,
    assertWithinRange = _require9.assertWithinRange,
    getDefaultSettings = _require9.getDefaultSettings,
    getSessionState = _require9.getSessionState,
    getSettings = _require9.getSettings,
    getStreamState = _require9.getStreamState,
    isPayloadMeaningless = _require9.isPayloadMeaningless,
    kSocket = _require9.kSocket,
    mapToHeaders = _require9.mapToHeaders,
    NghttpError = _require9.NghttpError,
    sessionName = _require9.sessionName,
    toHeaderObject = _require9.toHeaderObject,
    updateOptionsBuffer = _require9.updateOptionsBuffer,
    updateSettingsBuffer = _require9.updateSettingsBuffer;

var _require10 = require('internal/util/types'),
    isArrayBufferView = _require10.isArrayBufferView;

var _process$binding = process.binding('async_wrap'),
    async_id_symbol = _process$binding.async_id_symbol;

var binding = process.binding('http2');

var _process$binding2 = process.binding('stream_wrap'),
    ShutdownWrap = _process$binding2.ShutdownWrap,
    WriteWrap = _process$binding2.WriteWrap;

var _process$binding3 = process.binding('util'),
    createPromise = _process$binding3.createPromise,
    promiseResolve = _process$binding3.promiseResolve;

var httpConnectionListener = http._connectionListener;
var debug = util.debuglog('http2');
var kMaxFrameSize = Math.pow(2, 24) - 1;
var kMaxInt = Math.pow(2, 32) - 1;
var kMaxStreams = Math.pow(2, 31) - 1; // eslint-disable-next-line no-control-regex

var kQuotedString = /^[\x09\x20-\x5b\x5d-\x7e\x80-\xff]*$/;
var constants = binding.constants,
    nameForErrorCode = binding.nameForErrorCode;
var NETServer = net.Server;
var TLSServer = tls.Server;

var _require11 = require('_http_common'),
    kIncomingMessage = _require11.kIncomingMessage;

var _require12 = require('_http_server'),
    kServerResponse = _require12.kServerResponse;

var kAlpnProtocol = Symbol('alpnProtocol');
var kAuthority = Symbol('authority');
var kEncrypted = Symbol('encrypted');
var kHandle = Symbol('handle');
var kID = Symbol('id');
var kInit = Symbol('init');
var kInfoHeaders = Symbol('sent-info-headers');
var kMaybeDestroy = Symbol('maybe-destroy');
var kLocalSettings = Symbol('local-settings');
var kOptions = Symbol('options');
var kOwner = Symbol('owner');
var kOrigin = Symbol('origin');
var kProceed = Symbol('proceed');
var kProtocol = Symbol('protocol');
var kProxySocket = Symbol('proxy-socket');
var kRemoteSettings = Symbol('remote-settings');
var kSentHeaders = Symbol('sent-headers');
var kSentTrailers = Symbol('sent-trailers');
var kServer = Symbol('server');
var kSession = Symbol('session');
var kState = Symbol('state');
var kType = Symbol('type');
var kUpdateTimer = Symbol('update-timer');
var kWriteGeneric = Symbol('write-generic');
var kDefaultSocketTimeout = 2 * 60 * 1000;
var paddingBuffer = binding.paddingBuffer,
    PADDING_BUF_FRAME_LENGTH = binding.PADDING_BUF_FRAME_LENGTH,
    PADDING_BUF_MAX_PAYLOAD_LENGTH = binding.PADDING_BUF_MAX_PAYLOAD_LENGTH,
    PADDING_BUF_RETURN_VALUE = binding.PADDING_BUF_RETURN_VALUE;
var NGHTTP2_CANCEL = constants.NGHTTP2_CANCEL,
    NGHTTP2_REFUSED_STREAM = constants.NGHTTP2_REFUSED_STREAM,
    NGHTTP2_DEFAULT_WEIGHT = constants.NGHTTP2_DEFAULT_WEIGHT,
    NGHTTP2_FLAG_END_STREAM = constants.NGHTTP2_FLAG_END_STREAM,
    NGHTTP2_HCAT_PUSH_RESPONSE = constants.NGHTTP2_HCAT_PUSH_RESPONSE,
    NGHTTP2_HCAT_RESPONSE = constants.NGHTTP2_HCAT_RESPONSE,
    NGHTTP2_INTERNAL_ERROR = constants.NGHTTP2_INTERNAL_ERROR,
    NGHTTP2_NO_ERROR = constants.NGHTTP2_NO_ERROR,
    NGHTTP2_SESSION_CLIENT = constants.NGHTTP2_SESSION_CLIENT,
    NGHTTP2_SESSION_SERVER = constants.NGHTTP2_SESSION_SERVER,
    NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE = constants.NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE,
    NGHTTP2_ERR_INVALID_ARGUMENT = constants.NGHTTP2_ERR_INVALID_ARGUMENT,
    NGHTTP2_ERR_STREAM_CLOSED = constants.NGHTTP2_ERR_STREAM_CLOSED,
    HTTP2_HEADER_AUTHORITY = constants.HTTP2_HEADER_AUTHORITY,
    HTTP2_HEADER_DATE = constants.HTTP2_HEADER_DATE,
    HTTP2_HEADER_METHOD = constants.HTTP2_HEADER_METHOD,
    HTTP2_HEADER_PATH = constants.HTTP2_HEADER_PATH,
    HTTP2_HEADER_SCHEME = constants.HTTP2_HEADER_SCHEME,
    HTTP2_HEADER_STATUS = constants.HTTP2_HEADER_STATUS,
    HTTP2_HEADER_CONTENT_LENGTH = constants.HTTP2_HEADER_CONTENT_LENGTH,
    NGHTTP2_SETTINGS_HEADER_TABLE_SIZE = constants.NGHTTP2_SETTINGS_HEADER_TABLE_SIZE,
    NGHTTP2_SETTINGS_ENABLE_PUSH = constants.NGHTTP2_SETTINGS_ENABLE_PUSH,
    NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS = constants.NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS,
    NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE = constants.NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE,
    NGHTTP2_SETTINGS_MAX_FRAME_SIZE = constants.NGHTTP2_SETTINGS_MAX_FRAME_SIZE,
    NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE = constants.NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE,
    HTTP2_METHOD_GET = constants.HTTP2_METHOD_GET,
    HTTP2_METHOD_HEAD = constants.HTTP2_METHOD_HEAD,
    HTTP2_METHOD_CONNECT = constants.HTTP2_METHOD_CONNECT,
    HTTP_STATUS_CONTINUE = constants.HTTP_STATUS_CONTINUE,
    HTTP_STATUS_RESET_CONTENT = constants.HTTP_STATUS_RESET_CONTENT,
    HTTP_STATUS_OK = constants.HTTP_STATUS_OK,
    HTTP_STATUS_NO_CONTENT = constants.HTTP_STATUS_NO_CONTENT,
    HTTP_STATUS_NOT_MODIFIED = constants.HTTP_STATUS_NOT_MODIFIED,
    HTTP_STATUS_SWITCHING_PROTOCOLS = constants.HTTP_STATUS_SWITCHING_PROTOCOLS,
    HTTP_STATUS_MISDIRECTED_REQUEST = constants.HTTP_STATUS_MISDIRECTED_REQUEST,
    STREAM_OPTION_EMPTY_PAYLOAD = constants.STREAM_OPTION_EMPTY_PAYLOAD,
    STREAM_OPTION_GET_TRAILERS = constants.STREAM_OPTION_GET_TRAILERS;
var STREAM_FLAGS_PENDING = 0x0;
var STREAM_FLAGS_READY = 0x1;
var STREAM_FLAGS_CLOSED = 0x2;
var STREAM_FLAGS_HEADERS_SENT = 0x4;
var STREAM_FLAGS_HEAD_REQUEST = 0x8;
var STREAM_FLAGS_ABORTED = 0x10;
var STREAM_FLAGS_HAS_TRAILERS = 0x20;
var SESSION_FLAGS_PENDING = 0x0;
var SESSION_FLAGS_READY = 0x1;
var SESSION_FLAGS_CLOSED = 0x2;
var SESSION_FLAGS_DESTROYED = 0x4; // Top level to avoid creating a closure

function emit(self) {
  for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
    args[_key - 1] = arguments[_key];
  }

  self.emit.apply(self, args);
} // Called when a new block of headers has been received for a given
// stream. The stream may or may not be new. If the stream is new,
// create the associated Http2Stream instance and emit the 'stream'
// event. If the stream is not new, emit the 'headers' event to pass
// the block of headers on.


function onSessionHeaders(handle, id, cat, flags, headers) {
  var session = this[kOwner];
  if (session.destroyed) return;
  var type = session[kType];
  session[kUpdateTimer]();
  debug("Http2Stream ".concat(id, " [Http2Session ") + "".concat(sessionName(type), "]: headers received"));
  var streams = session[kState].streams;
  var endOfStream = !!(flags & NGHTTP2_FLAG_END_STREAM);
  var stream = streams.get(id); // Convert the array of header name value pairs into an object

  var obj = toHeaderObject(headers);

  if (stream === undefined) {
    if (session.closed) {
      // we are not accepting any new streams at this point. This callback
      // should not be invoked at this point in time, but just in case it is,
      // refuse the stream using an RST_STREAM and destroy the handle.
      handle.rstStream(NGHTTP2_REFUSED_STREAM);
      handle.destroy();
      return;
    }

    var opts = {
      readable: !endOfStream
    }; // session[kType] can be only one of two possible values

    if (type === NGHTTP2_SESSION_SERVER) {
      stream = new ServerHttp2Stream(session, handle, id, opts, obj);

      if (obj[HTTP2_HEADER_METHOD] === HTTP2_METHOD_HEAD) {
        // For head requests, there must not be a body...
        // end the writable side immediately.
        stream.end();
        stream[kState].flags |= STREAM_FLAGS_HEAD_REQUEST;
      }
    } else {
      stream = new ClientHttp2Stream(session, handle, id, opts);
      stream.end();
    }

    if (endOfStream) stream[kState].endAfterHeaders = true;
    process.nextTick(emit, session, 'stream', stream, obj, flags, headers);
  } else {
    var event;
    var status = obj[HTTP2_HEADER_STATUS];

    if (cat === NGHTTP2_HCAT_RESPONSE) {
      if (!endOfStream && status !== undefined && status >= 100 && status < 200) {
        event = 'headers';
      } else {
        event = 'response';
      }
    } else if (cat === NGHTTP2_HCAT_PUSH_RESPONSE) {
      event = 'push'; // cat === NGHTTP2_HCAT_HEADERS:
    } else if (!endOfStream && status !== undefined && status >= 200) {
      event = 'response';
    } else {
      event = endOfStream ? 'trailers' : 'headers';
    }

    var _session = stream.session;

    if (status === HTTP_STATUS_MISDIRECTED_REQUEST) {
      var originSet = _session[kState].originSet = initOriginSet(_session);
      originSet["delete"](stream[kOrigin]);
    }

    debug("Http2Stream ".concat(id, " [Http2Session ") + "".concat(sessionName(type), "]: emitting stream '").concat(event, "' event"));
    process.nextTick(emit, stream, event, obj, flags, headers);
  }

  if (endOfStream) {
    stream.push(null);
  }
}

function tryClose(fd) {
  // Try to close the file descriptor. If closing fails, assert because
  // an error really should not happen at this point.
  fs.close(fd, function (err) {
    return assert.ifError(err);
  });
} // Called when the Http2Stream has finished sending data and is ready for
// trailers to be sent. This will only be called if the { hasOptions: true }
// option is set.


function onStreamTrailers() {
  var stream = this[kOwner];
  stream[kState].trailersReady = true;
  if (stream.destroyed) return;

  if (!stream.emit('wantTrailers')) {
    // There are no listeners, send empty trailing HEADERS frame and close.
    stream.sendTrailers({});
  }
} // Submit an RST-STREAM frame to be sent to the remote peer.
// This will cause the Http2Stream to be closed.


function submitRstStream(code) {
  if (this[kHandle] !== undefined) {
    this[kHandle].rstStream(code);
  }
}

function onPing(payload) {
  var session = this[kOwner];
  if (session.destroyed) return;
  session[kUpdateTimer]();
  debug("Http2Session ".concat(sessionName(session[kType]), ": new ping received"));
  session.emit('ping', payload);
} // Called when the stream is closed either by sending or receiving an
// RST_STREAM frame, or through a natural end-of-stream.
// If the writable and readable sides of the stream are still open at this
// point, close them. If there is an open fd for file send, close that also.
// At this point the underlying node::http2:Http2Stream handle is no
// longer usable so destroy it also.


function onStreamClose(code) {
  var stream = this[kOwner];
  if (stream.destroyed) return;
  debug("Http2Stream ".concat(stream[kID], " [Http2Session ") + "".concat(sessionName(stream[kSession][kType]), "]: closed with code ").concat(code));
  if (!stream.closed) closeStream(stream, code, kNoRstStream);
  if (stream[kState].fd !== undefined) tryClose(stream[kState].fd); // Defer destroy we actually emit end.

  if (!stream.readable || code !== NGHTTP2_NO_ERROR) {
    // If errored or ended, we can destroy immediately.
    stream[kMaybeDestroy](code);
  } else {
    // Wait for end to destroy.
    stream.on('end', stream[kMaybeDestroy]); // Push a null so the stream can end whenever the client consumes
    // it completely.

    stream.push(null); // If the user hasn't tried to consume the stream (and this is a server
    // session) then just dump the incoming data so that the stream can
    // be destroyed.

    if (stream[kSession][kType] === NGHTTP2_SESSION_SERVER && !stream[kState].didRead && stream._readableState.flowing === null) stream.resume();else stream.read(0);
  }
} // Receives a chunk of data for a given stream and forwards it on
// to the Http2Stream Duplex for processing.


function onStreamRead(nread, buf) {
  var stream = this[kOwner];

  if (nread >= 0 && !stream.destroyed) {
    debug("Http2Stream ".concat(stream[kID], " [Http2Session ") + "".concat(sessionName(stream[kSession][kType]), "]: receiving data chunk ") + "of size ".concat(nread));
    stream[kUpdateTimer]();

    if (!stream.push(buf)) {
      if (!stream.destroyed) // we have to check a second time
        this.readStop();
    }

    return;
  } // Last chunk was received. End the readable side.


  debug("Http2Stream ".concat(stream[kID], " [Http2Session ") + "".concat(sessionName(stream[kSession][kType]), "]: ending readable.")); // defer this until we actually emit end

  if (!stream.readable) {
    stream[kMaybeDestroy]();
  } else {
    stream.on('end', stream[kMaybeDestroy]);
    stream.push(null);
    stream.read(0);
  }
} // Called when the remote peer settings have been updated.
// Resets the cached settings.


function onSettings() {
  var session = this[kOwner];
  if (session.destroyed) return;
  session[kUpdateTimer]();
  debug("Http2Session ".concat(sessionName(session[kType]), ": new settings received"));
  session[kRemoteSettings] = undefined;
  session.emit('remoteSettings', session.remoteSettings);
} // If the stream exists, an attempt will be made to emit an event
// on the stream object itself. Otherwise, forward it on to the
// session (which may, in turn, forward it on to the server)


function onPriority(id, parent, weight, exclusive) {
  var session = this[kOwner];
  if (session.destroyed) return;
  debug("Http2Stream ".concat(id, " [Http2Session ") + "".concat(sessionName(session[kType]), "]: priority [parent: ").concat(parent, ", ") + "weight: ".concat(weight, ", exclusive: ").concat(exclusive, "]"));
  var emitter = session[kState].streams.get(id) || session;

  if (!emitter.destroyed) {
    emitter[kUpdateTimer]();
    emitter.emit('priority', id, parent, weight, exclusive);
  }
} // Called by the native layer when an error has occurred sending a
// frame. This should be exceedingly rare.


function onFrameError(id, type, code) {
  var session = this[kOwner];
  if (session.destroyed) return;
  debug("Http2Session ".concat(sessionName(session[kType]), ": error sending frame ") + "type ".concat(type, " on stream ").concat(id, ", code: ").concat(code));
  var emitter = session[kState].streams.get(id) || session;
  emitter[kUpdateTimer]();
  emitter.emit('frameError', type, code, id);
}

function onAltSvc(stream, origin, alt) {
  var session = this[kOwner];
  if (session.destroyed) return;
  debug("Http2Session ".concat(sessionName(session[kType]), ": altsvc received: ") + "stream: ".concat(stream, ", origin: ").concat(origin, ", alt: ").concat(alt));
  session[kUpdateTimer]();
  session.emit('altsvc', alt, origin, stream);
}

function initOriginSet(session) {
  var originSet = session[kState].originSet;

  if (originSet === undefined) {
    var socket = session[kSocket];
    session[kState].originSet = originSet = new Set();

    if (socket.servername != null) {
      var originString = "https://".concat(socket.servername);
      if (socket.remotePort != null) originString += ":".concat(socket.remotePort); // We have to ensure that it is a properly serialized
      // ASCII origin string. The socket.servername might not
      // be properly ASCII encoded.

      originSet.add(new URL(originString).origin);
    }
  }

  return originSet;
}

function onOrigin(origins) {
  var session = this[kOwner];
  if (session.destroyed) return;
  debug("Http2Session ".concat(sessionName(session[kType]), ": origin received: ") + "".concat(origins.join(', ')));
  session[kUpdateTimer]();
  if (!session.encrypted || session.destroyed) return undefined;
  var originSet = initOriginSet(session);

  for (var n = 0; n < origins.length; n++) {
    originSet.add(origins[n]);
  }

  session.emit('origin', origins);
} // Receiving a GOAWAY frame from the connected peer is a signal that no
// new streams should be created. If the code === NGHTTP2_NO_ERROR, we
// are going to send our close, but allow existing frames to close
// normally. If code !== NGHTTP2_NO_ERROR, we are going to send our own
// close using the same code then destroy the session with an error.
// The goaway event will be emitted on next tick.


function onGoawayData(code, lastStreamID, buf) {
  var session = this[kOwner];
  if (session.destroyed) return;
  debug("Http2Session ".concat(sessionName(session[kType]), ": goaway ").concat(code, " ") + "received [last stream id: ".concat(lastStreamID, "]"));
  var state = session[kState];
  state.goawayCode = code;
  state.goawayLastStreamID = lastStreamID;
  session.emit('goaway', code, lastStreamID, buf);

  if (code === NGHTTP2_NO_ERROR) {
    // If this is a no error goaway, begin shutting down.
    // No new streams permitted, but existing streams may
    // close naturally on their own.
    session.close();
  } else {
    // However, if the code is not NGHTTP_NO_ERROR, destroy the
    // session immediately. We destroy with an error but send a
    // goaway using NGHTTP2_NO_ERROR because there was no error
    // condition on this side of the session that caused the
    // shutdown.
    session.destroy(new errors.Error('ERR_HTTP2_SESSION_ERROR', code), NGHTTP2_NO_ERROR);
  }
} // Returns the padding to use per frame. The selectPadding callback is set
// on the options. It is invoked with two arguments, the frameLen, and the
// maxPayloadLen. The method must return a numeric value within the range
// frameLen <= n <= maxPayloadLen.


function onSelectPadding(fn) {
  return function getPadding() {
    var frameLen = paddingBuffer[PADDING_BUF_FRAME_LENGTH];
    var maxFramePayloadLen = paddingBuffer[PADDING_BUF_MAX_PAYLOAD_LENGTH];
    paddingBuffer[PADDING_BUF_RETURN_VALUE] = fn(frameLen, maxFramePayloadLen);
  };
} // When a ClientHttp2Session is first created, the socket may not yet be
// connected. If request() is called during this time, the actual request
// will be deferred until the socket is ready to go.


function requestOnConnect(headers, options) {
  var session = this[kSession]; // At this point, the stream should have already been destroyed during
  // the session.destroy() method. Do nothing else.

  if (session === undefined || session.destroyed) return; // If the session was closed while waiting for the connect, destroy
  // the stream and do not continue with the request.

  if (session.closed) {
    var err = new errors.Error('ERR_HTTP2_GOAWAY_SESSION');
    this.destroy(err);
    return;
  }

  debug("Http2Session ".concat(sessionName(session[kType]), ": connected, ") + 'initializing request');
  var streamOptions = 0;
  if (options.endStream) streamOptions |= STREAM_OPTION_EMPTY_PAYLOAD;
  if (options.waitForTrailers) streamOptions |= STREAM_OPTION_GET_TRAILERS; // ret will be either the reserved stream ID (if positive)
  // or an error code (if negative)

  var ret = session[kHandle].request(headers, streamOptions, options.parent | 0, options.weight | 0, !!options.exclusive); // In an error condition, one of three possible response codes will be
  // possible:
  // * NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE - Maximum stream ID is reached, this
  //   is fatal for the session
  // * NGHTTP2_ERR_INVALID_ARGUMENT - Stream was made dependent on itself, this
  //   impacts on this stream.
  // For the first two, emit the error on the session,
  // For the third, emit the error on the stream, it will bubble up to the
  // session if not handled.

  if (typeof ret === 'number') {
    var _err;

    switch (ret) {
      case NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE:
        _err = new errors.Error('ERR_HTTP2_OUT_OF_STREAMS');
        this.destroy(_err);
        break;

      case NGHTTP2_ERR_INVALID_ARGUMENT:
        _err = new errors.Error('ERR_HTTP2_STREAM_SELF_DEPENDENCY');
        this.destroy(_err);
        break;

      default:
        session.destroy(new NghttpError(ret));
    }

    return;
  }

  this[kInit](ret.id(), ret);
} // Validates that priority options are correct, specifically:
// 1. options.weight must be a number
// 2. options.parent must be a positive number
// 3. options.exclusive must be a boolean
// 4. if specified, options.silent must be a boolean
//
// Also sets the default priority options if they are not set.


function validatePriorityOptions(options) {
  var err;

  if (options.weight === undefined) {
    options.weight = NGHTTP2_DEFAULT_WEIGHT;
  } else if (typeof options.weight !== 'number') {
    err = new errors.TypeError('ERR_INVALID_OPT_VALUE', 'weight', options.weight);
  }

  if (options.parent === undefined) {
    options.parent = 0;
  } else if (typeof options.parent !== 'number' || options.parent < 0) {
    err = new errors.TypeError('ERR_INVALID_OPT_VALUE', 'parent', options.parent);
  }

  if (options.exclusive === undefined) {
    options.exclusive = false;
  } else if (typeof options.exclusive !== 'boolean') {
    err = new errors.TypeError('ERR_INVALID_OPT_VALUE', 'exclusive', options.exclusive);
  }

  if (options.silent === undefined) {
    options.silent = false;
  } else if (typeof options.silent !== 'boolean') {
    err = new errors.TypeError('ERR_INVALID_OPT_VALUE', 'silent', options.silent);
  }

  if (err) {
    Error.captureStackTrace(err, validatePriorityOptions);
    throw err;
  }
} // When an error occurs internally at the binding level, immediately
// destroy the session.


function onSessionInternalError(code) {
  if (this[kOwner] !== undefined) this[kOwner].destroy(new NghttpError(code));
}

function settingsCallback(cb, ack, duration) {
  this[kState].pendingAck--;
  this[kLocalSettings] = undefined;

  if (ack) {
    debug("Http2Session ".concat(sessionName(this[kType]), ": settings received"));
    var settings = this.localSettings;
    if (typeof cb === 'function') cb(null, settings, duration);
    this.emit('localSettings', settings);
  } else {
    debug("Http2Session ".concat(sessionName(this[kType]), ": settings canceled"));
    if (typeof cb === 'function') cb(new errors.Error('ERR_HTTP2_SETTINGS_CANCEL'));
  }
} // Submits a SETTINGS frame to be sent to the remote peer.


function submitSettings(settings, callback) {
  if (this.destroyed) return;
  debug("Http2Session ".concat(sessionName(this[kType]), ": submitting settings"));
  this[kUpdateTimer]();
  updateSettingsBuffer(settings);

  if (!this[kHandle].settings(settingsCallback.bind(this, callback))) {
    this.destroy(new errors.Error('ERR_HTTP2_MAX_PENDING_SETTINGS_ACK'));
  }
} // Submits a PRIORITY frame to be sent to the remote peer
// Note: If the silent option is true, the change will be made
// locally with no PRIORITY frame sent.


function submitPriority(options) {
  if (this.destroyed) return;
  this[kUpdateTimer](); // If the parent is the id, do nothing because a
  // stream cannot be made to depend on itself.

  if (options.parent === this[kID]) return;
  this[kHandle].priority(options.parent | 0, options.weight | 0, !!options.exclusive, !!options.silent);
} // Submit a GOAWAY frame to be sent to the remote peer.
// If the lastStreamID is set to <= 0, then the lastProcStreamID will
// be used. The opaqueData must either be a typed array or undefined
// (which will be checked elsewhere).


function submitGoaway(code, lastStreamID, opaqueData) {
  if (this.destroyed) return;
  debug("Http2Session ".concat(sessionName(this[kType]), ": submitting goaway"));
  this[kUpdateTimer]();
  this[kHandle].goaway(code, lastStreamID, opaqueData);
}

var proxySocketHandler = {
  get: function get(session, prop) {
    switch (prop) {
      case 'setTimeout':
      case 'ref':
      case 'unref':
        return session[prop].bind(session);

      case 'destroy':
      case 'emit':
      case 'end':
      case 'pause':
      case 'read':
      case 'resume':
      case 'write':
      case 'setEncoding':
      case 'setKeepAlive':
      case 'setNoDelay':
        throw new errors.Error('ERR_HTTP2_NO_SOCKET_MANIPULATION');

      default:
        var socket = session[kSocket];
        if (socket === undefined) throw new errors.Error('ERR_HTTP2_SOCKET_UNBOUND');
        var value = socket[prop];
        return typeof value === 'function' ? value.bind(socket) : value;
    }
  },
  getPrototypeOf: function getPrototypeOf(session) {
    var socket = session[kSocket];
    if (socket === undefined) throw new errors.Error('ERR_HTTP2_SOCKET_UNBOUND');
    return Reflect.getPrototypeOf(socket);
  },
  set: function set(session, prop, value) {
    switch (prop) {
      case 'setTimeout':
      case 'ref':
      case 'unref':
        session[prop] = value;
        return true;

      case 'destroy':
      case 'emit':
      case 'end':
      case 'pause':
      case 'read':
      case 'resume':
      case 'write':
      case 'setEncoding':
      case 'setKeepAlive':
      case 'setNoDelay':
        throw new errors.Error('ERR_HTTP2_NO_SOCKET_MANIPULATION');

      default:
        var socket = session[kSocket];
        if (socket === undefined) throw new errors.Error('ERR_HTTP2_SOCKET_UNBOUND');
        socket[prop] = value;
        return true;
    }
  }
}; // pingCallback() returns a function that is invoked when an HTTP2 PING
// frame acknowledgement is received. The ack is either true or false to
// indicate if the ping was successful or not. The duration indicates the
// number of milliseconds elapsed since the ping was sent and the ack
// received. The payload is a Buffer containing the 8 bytes of payload
// data received on the PING acknowlegement.

function pingCallback(cb) {
  return function pingCallback(ack, duration, payload) {
    if (ack) {
      cb(null, duration, payload);
    } else {
      cb(new errors.Error('ERR_HTTP2_PING_CANCEL'));
    }
  };
} // Validates the values in a settings object. Specifically:
// 1. headerTableSize must be a number in the range 0 <= n <= kMaxInt
// 2. initialWindowSize must be a number in the range 0 <= n <= kMaxInt
// 3. maxFrameSize must be a number in the range 16384 <= n <= kMaxFrameSize
// 4. maxConcurrentStreams must be a number in the range 0 <= n <= kMaxStreams
// 5. maxHeaderListSize must be a number in the range 0 <= n <= kMaxInt
// 6. enablePush must be a boolean
// All settings are optional and may be left undefined


function validateSettings(settings) {
  settings = Object.assign({}, settings);
  assertWithinRange('headerTableSize', settings.headerTableSize, 0, kMaxInt);
  assertWithinRange('initialWindowSize', settings.initialWindowSize, 0, kMaxInt);
  assertWithinRange('maxFrameSize', settings.maxFrameSize, 16384, kMaxFrameSize);
  assertWithinRange('maxConcurrentStreams', settings.maxConcurrentStreams, 0, kMaxStreams);
  assertWithinRange('maxHeaderListSize', settings.maxHeaderListSize, 0, kMaxInt);

  if (settings.enablePush !== undefined && typeof settings.enablePush !== 'boolean') {
    var err = new errors.TypeError('ERR_HTTP2_INVALID_SETTING_VALUE', 'enablePush', settings.enablePush);
    err.actual = settings.enablePush;
    Error.captureStackTrace(err, 'validateSettings');
    throw err;
  }

  return settings;
} // Creates the internal binding.Http2Session handle for an Http2Session
// instance. This occurs only after the socket connection has been
// established. Note: the binding.Http2Session will take over ownership
// of the socket. No other code should read from or write to the socket.


function setupHandle(socket, type, options) {
  // If the session has been destroyed, go ahead and emit 'connect',
  // but do nothing else. The various on('connect') handlers set by
  // core will check for session.destroyed before progressing, this
  // ensures that those at l`east get cleared out.
  if (this.destroyed) {
    process.nextTick(emit, this, 'connect', this, socket);
    return;
  }

  debug("Http2Session ".concat(sessionName(type), ": setting up session handle"));
  this[kState].flags |= SESSION_FLAGS_READY;
  updateOptionsBuffer(options);
  var handle = new binding.Http2Session(type);
  handle[kOwner] = this;
  handle.error = onSessionInternalError;
  handle.onpriority = onPriority;
  handle.onsettings = onSettings;
  handle.onping = onPing;
  handle.onheaders = onSessionHeaders;
  handle.onframeerror = onFrameError;
  handle.ongoawaydata = onGoawayData;
  handle.onaltsvc = onAltSvc;
  handle.onorigin = onOrigin;
  if (typeof options.selectPadding === 'function') handle.ongetpadding = onSelectPadding(options.selectPadding);
  assert(socket._handle !== undefined, 'Internal HTTP/2 Failure. The socket is not connected. Please ' + 'report this as a bug in Node.js');
  handle.consume(socket._handle._externalStream);
  this[kHandle] = handle;

  if (socket.encrypted) {
    this[kAlpnProtocol] = socket.alpnProtocol;
    this[kEncrypted] = true;
  } else {
    // 'h2c' is the protocol identifier for HTTP/2 over plain-text. We use
    // it here to identify any session that is not explicitly using an
    // encrypted socket.
    this[kAlpnProtocol] = 'h2c';
    this[kEncrypted] = false;
  }

  var settings = _typeof(options.settings) === 'object' ? options.settings : {};
  this.settings(settings);

  if (type === NGHTTP2_SESSION_SERVER && Array.isArray(options.origins)) {
    this.origin.apply(this, _toConsumableArray(options.origins));
  }

  process.nextTick(emit, this, 'connect', this, socket);
} // Emits a close event followed by an error event if err is truthy. Used
// by Http2Session.prototype.destroy()


function emitClose(self, error) {
  if (error) self.emit('error', error);
  self.emit('close');
}

function finishSessionDestroy(session, error) {
  var socket = session[kSocket];
  if (!socket.destroyed) socket.destroy(error);
  session[kProxySocket] = undefined;
  session[kSocket] = undefined;
  session[kHandle] = undefined;
  socket[kSession] = undefined;
  socket[kServer] = undefined; // Finally, emit the close and error events (if necessary) on next tick.

  process.nextTick(emitClose, session, error);
} // Upon creation, the Http2Session takes ownership of the socket. The session
// may not be ready to use immediately if the socket is not yet fully connected.
// In that case, the Http2Session will wait for the socket to connect. Once
// the Http2Session is ready, it will emit its own 'connect' event.
//
// The Http2Session.goaway() method will send a GOAWAY frame, signalling
// to the connected peer that a shutdown is in progress. Sending a goaway
// frame has no other effect, however.
//
// Receiving a GOAWAY frame will cause the Http2Session to first emit a 'goaway'
// event notifying the user that a shutdown is in progress. If the goaway
// error code equals 0 (NGHTTP2_NO_ERROR), session.close() will be called,
// causing the Http2Session to send its own GOAWAY frame and switch itself
// into a graceful closing state. In this state, new inbound or outbound
// Http2Streams will be rejected. Existing *pending* streams (those created
// but without an assigned stream ID or handle) will be destroyed with a
// cancel error. Existing open streams will be permitted to complete on their
// own. Once all existing streams close, session.destroy() will be called
// automatically.
//
// Calling session.destroy() will tear down the Http2Session immediately,
// making it no longer usable. Pending and existing streams will be destroyed.
// The bound socket will be destroyed. Once all resources have been freed up,
// the 'close' event will be emitted. Note that pending streams will be
// destroyed using a specific "ERR_HTTP2_STREAM_CANCEL" error. Existing open
// streams will be destroyed using the same error passed to session.destroy()
//
// If destroy is called with an error, an 'error' event will be emitted
// immediately following the 'close' event.
//
// The socket and Http2Session lifecycles are tightly bound. Once one is
// destroyed, the other should also be destroyed. When the socket is destroyed
// with an error, session.destroy() will be called with that same error.
// Likewise, when session.destroy() is called with an error, the same error
// will be sent to the socket.


var Http2Session =
/*#__PURE__*/
function (_EventEmitter) {
  _inherits(Http2Session, _EventEmitter);

  function Http2Session(type, options, socket) {
    var _this;

    _classCallCheck(this, Http2Session);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(Http2Session).call(this));

    if (!socket._handle || !socket._handle._externalStream) {
      socket = new StreamWrap(socket);
    } // No validation is performed on the input parameters because this
    // constructor is not exported directly for users.
    // If the session property already exists on the socket,
    // then it has already been bound to an Http2Session instance
    // and cannot be attached again.


    if (socket[kSession] !== undefined) throw new errors.Error('ERR_HTTP2_SOCKET_BOUND');
    socket[kSession] = _assertThisInitialized(_this);
    _this[kState] = {
      flags: SESSION_FLAGS_PENDING,
      goawayCode: null,
      goawayLastStreamID: null,
      streams: new Map(),
      pendingStreams: new Set(),
      pendingAck: 0,
      writeQueueSize: 0,
      originSet: undefined
    };
    _this[kEncrypted] = undefined;
    _this[kAlpnProtocol] = undefined;
    _this[kType] = type;
    _this[kProxySocket] = null;
    _this[kSocket] = socket; // Do not use nagle's algorithm

    if (typeof socket.setNoDelay === 'function') socket.setNoDelay(); // Disable TLS renegotiation on the socket

    if (typeof socket.disableRenegotiation === 'function') socket.disableRenegotiation();
    var setupFn = setupHandle.bind(_assertThisInitialized(_this), socket, type, options);

    if (socket.connecting) {
      var connectEvent = socket instanceof tls.TLSSocket ? 'secureConnect' : 'connect';
      socket.once(connectEvent, setupFn);
    } else {
      setupFn();
    }

    debug("Http2Session ".concat(sessionName(type), ": created"));
    return _this;
  } // Returns undefined if the socket is not yet connected, true if the
  // socket is a TLSSocket, and false if it is not.


  _createClass(Http2Session, [{
    key: kUpdateTimer,
    // Resets the timeout counter
    value: function value() {
      if (this.destroyed) return;

      _unrefActive(this);
    } // Sets the id of the next stream to be created by this Http2Session.
    // The value must be a number in the range 0 <= n <= kMaxStreams. The
    // value also needs to be larger than the current next stream ID.

  }, {
    key: "setNextStreamID",
    value: function setNextStreamID(id) {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');
      if (typeof id !== 'number') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'id', 'number');
      if (id <= 0 || id > kMaxStreams) throw new errors.RangeError('ERR_OUT_OF_RANGE');
      this[kHandle].setNextStreamID(id);
    } // If ping is called while we are still connecting, or after close() has
    // been called, the ping callback will be invoked immediately will a ping
    // cancelled error and a duration of 0.0.

  }, {
    key: "ping",
    value: function ping(payload, callback) {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');

      if (typeof payload === 'function') {
        callback = payload;
        payload = undefined;
      }

      if (payload && !isArrayBufferView(payload)) {
        throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'payload', ['Buffer', 'TypedArray', 'DataView']);
      }

      if (payload && payload.length !== 8) {
        throw new errors.RangeError('ERR_HTTP2_PING_LENGTH');
      }

      if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
      var cb = pingCallback(callback);

      if (this.connecting || this.closed) {
        process.nextTick(cb, false, 0.0, payload);
        return;
      }

      return this[kHandle].ping(payload, cb);
    }
  }, {
    key: kInspect,
    value: function value(depth, opts) {
      var obj = {
        type: this[kType],
        closed: this.closed,
        destroyed: this.destroyed,
        state: this.state,
        localSettings: this.localSettings,
        remoteSettings: this.remoteSettings
      };
      return "Http2Session ".concat(util.format(obj));
    } // The socket owned by this session

  }, {
    key: "settings",
    // Submits a SETTINGS frame to be sent to the remote peer.
    value: function settings(_settings, callback) {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');
      assertIsObject(_settings, 'settings');
      _settings = validateSettings(_settings);
      if (callback && typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
      debug("Http2Session ".concat(sessionName(this[kType]), ": sending settings"));
      this[kState].pendingAck++;
      var settingsFn = submitSettings.bind(this, _settings, callback);

      if (this.connecting) {
        this.once('connect', settingsFn);
        return;
      }

      settingsFn();
    } // Sumits a GOAWAY frame to be sent to the remote peer. Note that this
    // is only a notification, and does not affect the usable state of the
    // session with the notable exception that new incoming streams will
    // be rejected automatically.

  }, {
    key: "goaway",
    value: function goaway() {
      var code = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : NGHTTP2_NO_ERROR;
      var lastStreamID = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
      var opaqueData = arguments.length > 2 ? arguments[2] : undefined;
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');

      if (opaqueData !== undefined && !isArrayBufferView(opaqueData)) {
        throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'opaqueData', ['Buffer', 'TypedArray', 'DataView']);
      }

      if (typeof code !== 'number') {
        throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'code', 'number');
      }

      if (typeof lastStreamID !== 'number') {
        throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'lastStreamID', 'number');
      }

      var goawayFn = submitGoaway.bind(this, code, lastStreamID, opaqueData);

      if (this.connecting) {
        this.once('connect', goawayFn);
        return;
      }

      goawayFn();
    } // Destroy the Http2Session, making it no longer usable and cancelling
    // any pending activity.

  }, {
    key: "destroy",
    value: function destroy() {
      var error = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : NGHTTP2_NO_ERROR;
      var code = arguments.length > 1 ? arguments[1] : undefined;
      if (this.destroyed) return;
      debug("Http2Session ".concat(sessionName(this[kType]), ": destroying"));

      if (typeof error === 'number') {
        code = error;
        error = code !== NGHTTP2_NO_ERROR ? new errors.Error('ERR_HTTP2_SESSION_ERROR', code) : undefined;
      }

      if (code === undefined && error != null) code = NGHTTP2_INTERNAL_ERROR;
      var state = this[kState];
      state.flags |= SESSION_FLAGS_DESTROYED; // Clear timeout and remove timeout listeners

      unenroll(this);
      this.removeAllListeners('timeout'); // Destroy any pending and open streams

      var cancel = new errors.Error('ERR_HTTP2_STREAM_CANCEL');

      if (error) {
        cancel.cause = error;
        if (typeof error.message === 'string') cancel.message += " (caused by: ".concat(error.message, ")");
      }

      state.pendingStreams.forEach(function (stream) {
        return stream.destroy(cancel);
      });
      state.streams.forEach(function (stream) {
        return stream.destroy(error);
      }); // Disassociate from the socket and server

      var socket = this[kSocket];
      var handle = this[kHandle]; // Destroy the handle if it exists at this point

      if (handle !== undefined) handle.destroy(code, socket.destroyed); // If the socket is alive, use setImmediate to destroy the session on the
      // next iteration of the event loop in order to give data time to transmit.
      // Otherwise, destroy immediately.

      if (!socket.destroyed) setImmediate(finishSessionDestroy, this, error);else finishSessionDestroy(this, error);
    } // Closing the session will:
    // 1. Send a goaway frame
    // 2. Mark the session as closed
    // 3. Prevent new inbound or outbound streams from being opened
    // 4. Optionally register a 'close' event handler
    // 5. Will cause the session to automatically destroy after the
    //    last currently open Http2Stream closes.
    //
    // Close always assumes a good, non-error shutdown (NGHTTP_NO_ERROR)
    //
    // If the session has not connected yet, the closed flag will still be
    // set but the goaway will not be sent until after the connect event
    // is emitted.

  }, {
    key: "close",
    value: function close(callback) {
      if (this.closed || this.destroyed) return;
      debug("Http2Session ".concat(sessionName(this[kType]), ": marking session closed"));
      this[kState].flags |= SESSION_FLAGS_CLOSED;
      if (typeof callback === 'function') this.once('close', callback);
      this.goaway();
      this[kMaybeDestroy]();
    } // Destroy the session if:
    // * error is not undefined/null
    // * session is closed and there are no more pending or open streams

  }, {
    key: kMaybeDestroy,
    value: function value(error) {
      if (error == null) {
        var state = this[kState]; // Do not destroy if we're not closed and there are pending/open streams

        if (!this.closed || state.streams.size > 0 || state.pendingStreams.size > 0) {
          return;
        }
      }

      this.destroy(error);
    }
  }, {
    key: "_onTimeout",
    value: function _onTimeout() {
      // If the session is destroyed, this should never actually be invoked,
      // but just in case...
      if (this.destroyed) return; // This checks whether a write is currently in progress and also whether
      // that write is actually sending data across the write. The kHandle
      // stored `chunksSentSinceLastWrite` is only updated when a timeout event
      // happens, meaning that if a write is ongoing it should never equal the
      // newly fetched, updated value.

      if (this[kState].writeQueueSize > 0) {
        var handle = this[kHandle];
        var chunksSentSinceLastWrite = handle !== undefined ? handle.chunksSentSinceLastWrite : null;

        if (chunksSentSinceLastWrite !== null && chunksSentSinceLastWrite !== handle.updateChunksSent()) {
          this[kUpdateTimer]();
          return;
        }
      }

      this.emit('timeout');
    }
  }, {
    key: "ref",
    value: function ref() {
      if (this[kSocket]) {
        this[kSocket].ref();
      }
    }
  }, {
    key: "unref",
    value: function unref() {
      if (this[kSocket]) {
        this[kSocket].unref();
      }
    }
  }, {
    key: "encrypted",
    get: function get() {
      return this[kEncrypted];
    } // Returns undefined if the socket is not yet connected, `h2` if the
    // socket is a TLSSocket and the alpnProtocol is `h2`, or `h2c` if the
    // socket is not a TLSSocket.

  }, {
    key: "alpnProtocol",
    get: function get() {
      return this[kAlpnProtocol];
    } // TODO(jasnell): originSet is being added in preparation for ORIGIN frame
    // support. At the current time, the ORIGIN frame specification is awaiting
    // publication as an RFC and is awaiting implementation in nghttp2. Once
    // added, an ORIGIN frame will add to the origins included in the origin
    // set. 421 responses will remove origins from the set.

  }, {
    key: "originSet",
    get: function get() {
      if (!this.encrypted || this.destroyed) return undefined;
      return Array.from(initOriginSet(this));
    } // True if the Http2Session is still waiting for the socket to connect

  }, {
    key: "connecting",
    get: function get() {
      return (this[kState].flags & SESSION_FLAGS_READY) === 0;
    } // True if Http2Session.prototype.close() has been called

  }, {
    key: "closed",
    get: function get() {
      return !!(this[kState].flags & SESSION_FLAGS_CLOSED);
    } // True if Http2Session.prototype.destroy() has been called

  }, {
    key: "destroyed",
    get: function get() {
      return !!(this[kState].flags & SESSION_FLAGS_DESTROYED);
    }
  }, {
    key: "socket",
    get: function get() {
      var proxySocket = this[kProxySocket];
      if (proxySocket === null) return this[kProxySocket] = new Proxy(this, proxySocketHandler);
      return proxySocket;
    } // The session type

  }, {
    key: "type",
    get: function get() {
      return this[kType];
    } // If a GOAWAY frame has been received, gives the error code specified

  }, {
    key: "goawayCode",
    get: function get() {
      return this[kState].goawayCode || NGHTTP2_NO_ERROR;
    } // If a GOAWAY frame has been received, gives the last stream ID reported

  }, {
    key: "goawayLastStreamID",
    get: function get() {
      return this[kState].goawayLastStreamID || 0;
    } // true if the Http2Session is waiting for a settings acknowledgement

  }, {
    key: "pendingSettingsAck",
    get: function get() {
      return this[kState].pendingAck > 0;
    } // Retrieves state information for the Http2Session

  }, {
    key: "state",
    get: function get() {
      return this.connecting || this.destroyed ? {} : getSessionState(this[kHandle]);
    } // The settings currently in effect for the local peer. These will
    // be updated only when a settings acknowledgement has been received.

  }, {
    key: "localSettings",
    get: function get() {
      var settings = this[kLocalSettings];
      if (settings !== undefined) return settings;
      if (this.destroyed || this.connecting) return {};
      return this[kLocalSettings] = getSettings(this[kHandle], false); // Local
    } // The settings currently in effect for the remote peer.

  }, {
    key: "remoteSettings",
    get: function get() {
      var settings = this[kRemoteSettings];
      if (settings !== undefined) return settings;
      if (this.destroyed || this.connecting) return {};
      return this[kRemoteSettings] = getSettings(this[kHandle], true); // Remote
    }
  }]);

  return Http2Session;
}(EventEmitter); // ServerHttp2Session instances should never have to wait for the socket
// to connect as they are always created after the socket has already been
// established.


var ServerHttp2Session =
/*#__PURE__*/
function (_Http2Session) {
  _inherits(ServerHttp2Session, _Http2Session);

  function ServerHttp2Session(options, socket, server) {
    var _this2;

    _classCallCheck(this, ServerHttp2Session);

    _this2 = _possibleConstructorReturn(this, _getPrototypeOf(ServerHttp2Session).call(this, NGHTTP2_SESSION_SERVER, options, socket));
    _this2[kServer] = server;
    return _this2;
  }

  _createClass(ServerHttp2Session, [{
    key: "altsvc",
    // Submits an altsvc frame to be sent to the client. `stream` is a
    // numeric Stream ID. origin is a URL string that will be used to get
    // the origin. alt is a string containing the altsvc details. No fancy
    // API is provided for that.
    value: function altsvc(alt, originOrStream) {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');
      var stream = 0;
      var origin;

      if (typeof originOrStream === 'string') {
        origin = new URL(originOrStream).origin;
        if (origin === 'null') throw new errors.TypeError('ERR_HTTP2_ALTSVC_INVALID_ORIGIN');
      } else if (typeof originOrStream === 'number') {
        if (originOrStream >>> 0 !== originOrStream || originOrStream === 0) throw new errors.RangeError('ERR_OUT_OF_RANGE', 'originOrStream');
        stream = originOrStream;
      } else if (originOrStream !== undefined) {
        // Allow origin to be passed a URL or object with origin property
        if (originOrStream !== null && _typeof(originOrStream) === 'object') origin = originOrStream.origin; // Note: if originOrStream is an object with an origin property other
        // than a URL, then it is possible that origin will be malformed.
        // We do not verify that here. Users who go that route need to
        // ensure they are doing the right thing or the payload data will
        // be invalid.

        if (typeof origin !== 'string') {
          throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'originOrStream', ['string', 'number', 'URL', 'object']);
        } else if (origin === 'null' || origin.length === 0) {
          throw new errors.TypeError('ERR_HTTP2_ALTSVC_INVALID_ORIGIN');
        }
      }

      if (typeof alt !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'alt', 'string');
      if (!kQuotedString.test(alt)) throw new errors.TypeError('ERR_INVALID_CHAR', 'alt'); // Max length permitted for ALTSVC

      if (alt.length + (origin !== undefined ? origin.length : 0) > 16382) throw new errors.TypeError('ERR_HTTP2_ALTSVC_LENGTH');
      this[kHandle].altsvc(stream, origin || '', alt);
    } // Submits an origin frame to be sent.

  }, {
    key: "origin",
    value: function origin() {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');
      if (arguments.length === 0) return;
      var arr = '';
      var len = 0;
      var count = arguments.length;

      for (var i = 0; i < count; i++) {
        var origin = i < 0 || arguments.length <= i ? undefined : arguments[i];

        if (typeof origin === 'string') {
          origin = new URL(origin).origin;
        } else if (origin != null && _typeof(origin) === 'object') {
          origin = origin.origin;
        }

        if (typeof origin !== 'string') throw new errors.Error('ERR_INVALID_ARG_TYPE', 'origin', 'string', origin);
        if (origin === 'null') throw new errors.Error('ERR_HTTP2_INVALID_ORIGIN');
        arr += "".concat(origin, "\0");
        len += origin.length;
      }

      if (len > 16382) throw new errors.Error('ERR_HTTP2_ORIGIN_LENGTH');
      this[kHandle].origin(arr, count);
    }
  }, {
    key: "server",
    get: function get() {
      return this[kServer];
    }
  }]);

  return ServerHttp2Session;
}(Http2Session); // ClientHttp2Session instances have to wait for the socket to connect after
// they have been created. Various operations such as request() may be used,
// but the actual protocol communication will only occur after the socket
// has been connected.


var ClientHttp2Session =
/*#__PURE__*/
function (_Http2Session2) {
  _inherits(ClientHttp2Session, _Http2Session2);

  function ClientHttp2Session(options, socket) {
    _classCallCheck(this, ClientHttp2Session);

    return _possibleConstructorReturn(this, _getPrototypeOf(ClientHttp2Session).call(this, NGHTTP2_SESSION_CLIENT, options, socket));
  } // Submits a new HTTP2 request to the connected peer. Returns the
  // associated Http2Stream instance.


  _createClass(ClientHttp2Session, [{
    key: "request",
    value: function request(headers, options) {
      debug("Http2Session ".concat(sessionName(this[kType]), ": initiating request"));
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_SESSION');
      if (this.closed) throw new errors.Error('ERR_HTTP2_GOAWAY_SESSION');
      this[kUpdateTimer]();
      assertIsObject(headers, 'headers');
      assertIsObject(options, 'options');
      headers = Object.assign(Object.create(null), headers);
      options = Object.assign({}, options);
      if (headers[HTTP2_HEADER_METHOD] === undefined) headers[HTTP2_HEADER_METHOD] = HTTP2_METHOD_GET;
      var connect = headers[HTTP2_HEADER_METHOD] === HTTP2_METHOD_CONNECT;

      if (!connect) {
        if (headers[HTTP2_HEADER_AUTHORITY] === undefined) headers[HTTP2_HEADER_AUTHORITY] = this[kAuthority];
        if (headers[HTTP2_HEADER_SCHEME] === undefined) headers[HTTP2_HEADER_SCHEME] = this[kProtocol].slice(0, -1);
        if (headers[HTTP2_HEADER_PATH] === undefined) headers[HTTP2_HEADER_PATH] = '/';
      } else {
        if (headers[HTTP2_HEADER_AUTHORITY] === undefined) throw new errors.Error('ERR_HTTP2_CONNECT_AUTHORITY');
        if (headers[HTTP2_HEADER_SCHEME] !== undefined) throw new errors.Error('ERR_HTTP2_CONNECT_SCHEME');
        if (headers[HTTP2_HEADER_PATH] !== undefined) throw new errors.Error('ERR_HTTP2_CONNECT_PATH');
      }

      validatePriorityOptions(options);

      if (options.endStream === undefined) {
        // For some methods, we know that a payload is meaningless, so end the
        // stream by default if the user has not specifically indicated a
        // preference.
        options.endStream = isPayloadMeaningless(headers[HTTP2_HEADER_METHOD]);
      } else if (typeof options.endStream !== 'boolean') {
        throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'endStream', options.endStream);
      }

      var headersList = mapToHeaders(headers);
      if (!Array.isArray(headersList)) throw headersList;
      var stream = new ClientHttp2Stream(this, undefined, undefined, {});
      stream[kSentHeaders] = headers;
      stream[kOrigin] = "".concat(headers[HTTP2_HEADER_SCHEME], "://") + "".concat(headers[HTTP2_HEADER_AUTHORITY]); // Close the writable side of the stream if options.endStream is set.

      if (options.endStream) stream.end();
      if (options.waitForTrailers) stream[kState].flags |= STREAM_FLAGS_HAS_TRAILERS;
      var onConnect = requestOnConnect.bind(stream, headersList, options);

      if (this.connecting) {
        this.once('connect', onConnect);
      } else {
        onConnect();
      }

      return stream;
    }
  }]);

  return ClientHttp2Session;
}(Http2Session);

function createWriteReq(req, handle, data, encoding) {
  switch (encoding) {
    case 'utf8':
    case 'utf-8':
      return handle.writeUtf8String(req, data);

    case 'ascii':
      return handle.writeAsciiString(req, data);

    case 'ucs2':
    case 'ucs-2':
    case 'utf16le':
    case 'utf-16le':
      return handle.writeUcs2String(req, data);

    case 'latin1':
    case 'binary':
      return handle.writeLatin1String(req, data);

    case 'buffer':
      return handle.writeBuffer(req, data);

    default:
      return handle.writeBuffer(req, Buffer.from(data, encoding));
  }
}

function trackWriteState(stream, bytes) {
  var session = stream[kSession];
  stream[kState].writeQueueSize += bytes;
  session[kState].writeQueueSize += bytes;
  session[kHandle].chunksSentSinceLastWrite = 0;
}

function afterDoStreamWrite(status, handle, req) {
  var stream = handle[kOwner];
  var session = stream[kSession];
  stream[kUpdateTimer]();
  var bytes = req.bytes;
  stream[kState].writeQueueSize -= bytes;
  if (session !== undefined) session[kState].writeQueueSize -= bytes;
  if (typeof req.callback === 'function') req.callback(null);
  req.handle = undefined;
}

function streamOnResume() {
  if (!this.destroyed) this[kHandle].readStart();
}

function streamOnPause() {
  if (!this.destroyed && !this.pending) this[kHandle].readStop();
}

function afterShutdown() {
  this.callback();
  var stream = this.handle[kOwner];
  if (stream) stream[kMaybeDestroy]();
}

function finishSendTrailers(stream, headersList) {
  // The stream might be destroyed and in that case
  // there is nothing to do.
  // This can happen because finishSendTrailers is
  // scheduled via setImmediate.
  if (stream.destroyed) {
    return;
  }

  stream[kState].flags &= ~STREAM_FLAGS_HAS_TRAILERS;
  var ret = stream[kHandle].trailers(headersList);
  if (ret < 0) stream.destroy(new NghttpError(ret));else stream[kMaybeDestroy]();
}

var kNoRstStream = 0;
var kSubmitRstStream = 1;
var kForceRstStream = 2;

function closeStream(stream, code) {
  var rstStreamStatus = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : kSubmitRstStream;
  var state = stream[kState];
  state.flags |= STREAM_FLAGS_CLOSED;
  state.rstCode = code; // Clear timeout and remove timeout listeners

  stream.setTimeout(0);
  stream.removeAllListeners('timeout');
  var _stream$_writableStat = stream._writableState,
      ending = _stream$_writableStat.ending,
      finished = _stream$_writableStat.finished;

  if (!ending) {
    // If the writable side of the Http2Stream is still open, emit the
    // 'aborted' event and set the aborted flag.
    if (!stream.aborted) {
      state.flags |= STREAM_FLAGS_ABORTED;
      stream.emit('aborted');
    } // Close the writable side.


    stream.end();
  }

  if (rstStreamStatus !== kNoRstStream) {
    var finishFn = finishCloseStream.bind(stream, code);
    if (!ending || finished || code !== NGHTTP2_NO_ERROR || rstStreamStatus === kForceRstStream) finishFn();else stream.once('finish', finishFn);
  }
}

function finishCloseStream(code) {
  var rstStreamFn = submitRstStream.bind(this, code); // If the handle has not yet been assigned, queue up the request to
  // ensure that the RST_STREAM frame is sent after the stream ID has
  // been determined.

  if (this.pending) {
    this.push(null);
    this.once('ready', rstStreamFn);
    return;
  }

  rstStreamFn();
} // An Http2Stream is a Duplex stream that is backed by a
// node::http2::Http2Stream handle implementing StreamBase.


var Http2Stream =
/*#__PURE__*/
function (_Duplex) {
  _inherits(Http2Stream, _Duplex);

  function Http2Stream(session, options) {
    var _this3;

    _classCallCheck(this, Http2Stream);

    options.allowHalfOpen = true;
    options.decodeStrings = false;
    _this3 = _possibleConstructorReturn(this, _getPrototypeOf(Http2Stream).call(this, options));
    _this3[async_id_symbol] = -1; // Corking the stream automatically allows writes to happen
    // but ensures that those are buffered until the handle has
    // been assigned.

    _this3.cork();

    _this3[kSession] = session;
    session[kState].pendingStreams.add(_assertThisInitialized(_this3)); // Allow our logic for determining whether any reads have happened to
    // work in all situations. This is similar to what we do in _http_incoming.

    _this3._readableState.readingMore = true;
    _this3[kState] = {
      didRead: false,
      flags: STREAM_FLAGS_PENDING,
      rstCode: NGHTTP2_NO_ERROR,
      writeQueueSize: 0,
      trailersReady: false,
      endAfterHeaders: false
    };

    _this3.on('pause', streamOnPause);

    return _this3;
  }

  _createClass(Http2Stream, [{
    key: kUpdateTimer,
    value: function value() {
      if (this.destroyed) return;

      _unrefActive(this);

      if (this[kSession]) _unrefActive(this[kSession]);
    }
  }, {
    key: kInit,
    value: function value(id, handle) {
      var state = this[kState];
      state.flags |= STREAM_FLAGS_READY;
      var session = this[kSession];
      session[kState].pendingStreams["delete"](this);
      session[kState].streams.set(id, this);
      this[kID] = id;
      this[async_id_symbol] = handle.getAsyncId();
      handle[kOwner] = this;
      this[kHandle] = handle;
      handle.ontrailers = onStreamTrailers;
      handle.onstreamclose = onStreamClose;
      handle.onread = onStreamRead;
      this.uncork();
      this.emit('ready');
    }
  }, {
    key: kInspect,
    value: function value(depth, opts) {
      var obj = {
        id: this[kID] || '<pending>',
        closed: this.closed,
        destroyed: this.destroyed,
        state: this.state,
        readableState: this._readableState,
        writableState: this._writableState
      };
      return "Http2Stream ".concat(util.format(obj));
    }
  }, {
    key: "_onTimeout",
    value: function _onTimeout() {
      if (this.destroyed) return; // This checks whether a write is currently in progress and also whether
      // that write is actually sending data across the write. The kHandle
      // stored `chunksSentSinceLastWrite` is only updated when a timeout event
      // happens, meaning that if a write is ongoing it should never equal the
      // newly fetched, updated value.

      if (this[kState].writeQueueSize > 0) {
        var handle = this[kSession][kHandle];
        var chunksSentSinceLastWrite = handle !== undefined ? handle.chunksSentSinceLastWrite : null;

        if (chunksSentSinceLastWrite !== null && chunksSentSinceLastWrite !== handle.updateChunksSent()) {
          this[kUpdateTimer]();
          return;
        }
      }

      this.emit('timeout');
    } // true if the HEADERS frame has been sent

  }, {
    key: kProceed,
    value: function value() {
      assert.fail(null, null, 'Implementors MUST implement this. Please report this as a ' + 'bug in Node.js');
    }
  }, {
    key: kWriteGeneric,
    value: function value(writev, data, encoding, cb) {
      // When the Http2Stream is first created, it is corked until the
      // handle and the stream ID is assigned. However, if the user calls
      // uncork() before that happens, the Duplex will attempt to pass
      // writes through. Those need to be queued up here.
      if (this.pending) {
        this.once('ready', this[kWriteGeneric].bind(this, writev, data, encoding, cb));
        return;
      } // If the stream has been destroyed, there's nothing else we can do
      // because the handle has been destroyed. This should only be an
      // issue if a write occurs before the 'ready' event in the case where
      // the duplex is uncorked before the stream is ready to go. In that
      // case, drop the data on the floor. An error should have already been
      // emitted.


      if (this.destroyed) return;
      this[kUpdateTimer]();
      if (!this.headersSent) this[kProceed]();
      var handle = this[kHandle];
      var req = new WriteWrap();
      req.stream = this[kID];
      req.handle = handle;
      req.callback = cb;
      req.oncomplete = afterDoStreamWrite;
      req.async = false;
      var err;

      if (writev) {
        var chunks = new Array(data.length << 1);

        for (var i = 0; i < data.length; i++) {
          var entry = data[i];
          chunks[i * 2] = entry.chunk;
          chunks[i * 2 + 1] = entry.encoding;
        }

        err = handle.writev(req, chunks);
      } else {
        err = createWriteReq(req, handle, data, encoding);
      }

      if (err) return this.destroy(errors.errnoException(err, 'write', req.error), cb);
      trackWriteState(this, req.bytes);
    }
  }, {
    key: "_write",
    value: function _write(data, encoding, cb) {
      this[kWriteGeneric](false, data, encoding, cb);
    }
  }, {
    key: "_writev",
    value: function _writev(data, cb) {
      this[kWriteGeneric](true, data, '', cb);
    }
  }, {
    key: "_final",
    value: function _final(cb) {
      var _this4 = this;

      var handle = this[kHandle];

      if (this[kID] === undefined) {
        this.once('ready', function () {
          return _this4._final(cb);
        });
      } else if (handle !== undefined) {
        debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(this[kSession][kType]), "]: _final shutting down"));
        var req = new ShutdownWrap();
        req.oncomplete = afterShutdown;
        req.callback = cb;
        req.handle = handle;
        handle.shutdown(req);
      } else {
        cb();
      }
    }
  }, {
    key: "_read",
    value: function _read(nread) {
      if (this.destroyed) {
        this.push(null);
        return;
      }

      if (!this[kState].didRead) {
        this._readableState.readingMore = false;
        this[kState].didRead = true;
      }

      if (!this.pending) {
        streamOnResume.call(this);
      } else {
        this.once('ready', streamOnResume);
      }
    }
  }, {
    key: "priority",
    value: function priority(options) {
      if (this.destroyed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      assertIsObject(options, 'options');
      options = Object.assign({}, options);
      validatePriorityOptions(options);
      var priorityFn = submitPriority.bind(this, options); // If the handle has not yet been assigned, queue up the priority
      // frame to be sent as soon as the ready event is emitted.

      if (this.pending) {
        this.once('ready', priorityFn);
        return;
      }

      priorityFn();
    }
  }, {
    key: "sendTrailers",
    value: function sendTrailers(headers) {
      if (this.destroyed || this.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this[kSentTrailers]) throw new errors.Error('ERR_HTTP2_TRAILERS_ALREADY_SENT');
      if (!this[kState].trailersReady) throw new errors.Error('ERR_HTTP2_TRAILERS_NOT_READY');
      assertIsObject(headers, 'headers');
      headers = Object.assign(Object.create(null), headers);
      var session = this[kSession];
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: sending trailers"));
      this[kUpdateTimer]();
      var headersList = mapToHeaders(headers, assertValidPseudoHeaderTrailer);
      if (!Array.isArray(headersList)) throw headersList;
      this[kSentTrailers] = headers; // Send the trailers in setImmediate so we don't do it on nghttp2 stack.

      setImmediate(finishSendTrailers, this, headersList);
    }
  }, {
    key: "close",
    // Close initiates closing the Http2Stream instance by sending an RST_STREAM
    // frame to the connected peer. The readable and writable sides of the
    // Http2Stream duplex are closed and the timeout timer is unenrolled. If
    // a callback is passed, it is registered to listen for the 'close' event.
    //
    // If the handle and stream ID have not been assigned yet, the close
    // will be queued up to wait for the ready event. As soon as the stream ID
    // is determined, the close will proceed.
    //
    // Submitting the RST_STREAM frame to the underlying handle will cause
    // the Http2Stream to be closed and ultimately destroyed. After calling
    // close, it is still possible to queue up PRIORITY and RST_STREAM frames,
    // but no DATA and HEADERS frames may be sent.
    value: function close() {
      var code = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : NGHTTP2_NO_ERROR;
      var callback = arguments.length > 1 ? arguments[1] : undefined;
      if (typeof code !== 'number') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'code', 'number');
      if (code < 0 || code > kMaxInt) throw new errors.RangeError('ERR_OUT_OF_RANGE', 'code');
      if (callback !== undefined && typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
      if (this.closed) return;
      if (callback !== undefined) this.once('close', callback);
      closeStream(this, code);
    } // Called by this.destroy().
    // * Will submit an RST stream to shutdown the stream if necessary.
    //   This will cause the internal resources to be released.
    // * Then cleans up the resources on the js side

  }, {
    key: "_destroy",
    value: function _destroy(err, callback) {
      var session = this[kSession];
      var handle = this[kHandle];
      var id = this[kID];
      debug("Http2Stream ".concat(this[kID] || '<pending>', " [Http2Session ") + "".concat(sessionName(session[kType]), "]: destroying stream"));
      var state = this[kState];
      var code = err != null ? NGHTTP2_INTERNAL_ERROR : state.rstCode || NGHTTP2_NO_ERROR;
      var hasHandle = handle !== undefined;
      if (!this.closed) closeStream(this, code, hasHandle ? kForceRstStream : kNoRstStream);
      this.push(null);

      if (hasHandle) {
        handle.destroy();
        session[kState].streams["delete"](id);
      } else {
        session[kState].pendingStreams["delete"](this);
      } // Adjust the write queue size for accounting


      session[kState].writeQueueSize -= state.writeQueueSize;
      state.writeQueueSize = 0; // RST code 8 not emitted as an error as its used by clients to signify
      // abort and is already covered by aborted event, also allows more
      // seamless compatibility with http1

      if (err == null && code !== NGHTTP2_NO_ERROR && code !== NGHTTP2_CANCEL) err = new errors.Error('ERR_HTTP2_STREAM_ERROR', nameForErrorCode[code] || code);
      this[kSession] = undefined;
      this[kHandle] = undefined; // This notifies the session that this stream has been destroyed and
      // gives the session the opportunity to clean itself up. The session
      // will destroy if it has been closed and there are no other open or
      // pending streams.

      session[kMaybeDestroy]();
      callback(err);
      process.nextTick(emit, this, 'close', code);
    } // The Http2Stream can be destroyed if it has closed and if the readable
    // side has received the final chunk.

  }, {
    key: kMaybeDestroy,
    value: function value() {
      var code = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : NGHTTP2_NO_ERROR;

      if (code !== NGHTTP2_NO_ERROR) {
        this.destroy();
        return;
      } // TODO(mcollina): remove usage of _*State properties


      if (this._writableState.finished) {
        if (!this.readable && this.closed) {
          this.destroy();
          return;
        } // We've submitted a response from our server session, have not attempted
        // to process any incoming data, and have no trailers. This means we can
        // attempt to gracefully close the session.


        var state = this[kState];

        if (this.headersSent && this[kSession][kType] === NGHTTP2_SESSION_SERVER && !(state.flags & STREAM_FLAGS_HAS_TRAILERS) && !state.didRead && this._readableState.flowing === null) {
          // By using setImmediate we allow pushStreams to make it through
          // before the stream is officially closed. This prevents a bug
          // in most browsers where those pushStreams would be rejected.
          setImmediate(this.close.bind(this));
        }
      }
    }
  }, {
    key: "endAfterHeaders",
    get: function get() {
      return this[kState].endAfterHeaders;
    }
  }, {
    key: "sentHeaders",
    get: function get() {
      return this[kSentHeaders];
    }
  }, {
    key: "sentTrailers",
    get: function get() {
      return this[kSentTrailers];
    }
  }, {
    key: "sentInfoHeaders",
    get: function get() {
      return this[kInfoHeaders];
    }
  }, {
    key: "pending",
    get: function get() {
      return this[kID] === undefined;
    } // The id of the Http2Stream, will be undefined if the socket is not
    // yet connected.

  }, {
    key: "id",
    get: function get() {
      return this[kID];
    } // The Http2Session that owns this Http2Stream.

  }, {
    key: "session",
    get: function get() {
      return this[kSession];
    }
  }, {
    key: "headersSent",
    get: function get() {
      return !!(this[kState].flags & STREAM_FLAGS_HEADERS_SENT);
    } // true if the Http2Stream was aborted abnormally.

  }, {
    key: "aborted",
    get: function get() {
      return !!(this[kState].flags & STREAM_FLAGS_ABORTED);
    } // true if dealing with a HEAD request

  }, {
    key: "headRequest",
    get: function get() {
      return !!(this[kState].flags & STREAM_FLAGS_HEAD_REQUEST);
    } // The error code reported when this Http2Stream was closed.

  }, {
    key: "rstCode",
    get: function get() {
      return this[kState].rstCode;
    } // State information for the Http2Stream

  }, {
    key: "state",
    get: function get() {
      var id = this[kID];
      if (this.destroyed || id === undefined) return {};
      return getStreamState(this[kHandle], id);
    }
  }, {
    key: "closed",
    get: function get() {
      return !!(this[kState].flags & STREAM_FLAGS_CLOSED);
    }
  }]);

  return Http2Stream;
}(Duplex);

function processHeaders(headers) {
  assertIsObject(headers, 'headers');
  headers = Object.assign(Object.create(null), headers);
  if (headers[HTTP2_HEADER_STATUS] == null) headers[HTTP2_HEADER_STATUS] = HTTP_STATUS_OK;
  headers[HTTP2_HEADER_DATE] = utcDate();
  var statusCode = headers[HTTP2_HEADER_STATUS] |= 0; // This is intentionally stricter than the HTTP/1 implementation, which
  // allows values between 100 and 999 (inclusive) in order to allow for
  // backwards compatibility with non-spec compliant code. With HTTP/2,
  // we have the opportunity to start fresh with stricter spec compliance.
  // This will have an impact on the compatibility layer for anyone using
  // non-standard, non-compliant status codes.

  if (statusCode < 200 || statusCode > 599) throw new errors.RangeError('ERR_HTTP2_STATUS_INVALID', headers[HTTP2_HEADER_STATUS]);
  return headers;
}

function processRespondWithFD(self, fd, headers) {
  var offset = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : 0;
  var length = arguments.length > 4 && arguments[4] !== undefined ? arguments[4] : -1;
  var streamOptions = arguments.length > 5 && arguments[5] !== undefined ? arguments[5] : 0;
  var state = self[kState];
  state.flags |= STREAM_FLAGS_HEADERS_SENT;
  var headersList = mapToHeaders(headers, assertValidPseudoHeaderResponse);
  self[kSentHeaders] = headers;

  if (!Array.isArray(headersList)) {
    self.destroy(headersList);
    return;
  } // Close the writable side of the stream


  self.end();
  var ret = self[kHandle].respondFD(fd, headersList, offset, length, streamOptions);

  if (ret < 0) {
    self.destroy(new NghttpError(ret));
    return;
  } // exact length of the file doesn't matter here, since the
  // stream is closing anyway - just use 1 to signify that
  // a write does exist


  trackWriteState(self, 1);
}

function doSendFD(session, options, fd, headers, streamOptions, err, stat) {
  if (err) {
    this.destroy(err);
    return;
  } // This can happen if the stream is destroyed or closed while we are waiting
  // for the file descriptor to be opened or the stat call to be completed.
  // In either case, we do not want to continue because the we are shutting
  // down and should not attempt to send any data.


  if (this.destroyed || this.closed) {
    this.destroy(new errors.Error('ERR_HTTP2_INVALID_STREAM'));
    return;
  }

  var statOptions = {
    offset: options.offset !== undefined ? options.offset : 0,
    length: options.length !== undefined ? options.length : -1
  }; // options.statCheck is a user-provided function that can be used to
  // verify stat values, override or set headers, or even cancel the
  // response operation. If statCheck explicitly returns false, the
  // response is canceled. The user code may also send a separate type
  // of response so check again for the HEADERS_SENT flag

  if (typeof options.statCheck === 'function' && options.statCheck.call(this, stat, headers, statOptions) === false || this[kState].flags & STREAM_FLAGS_HEADERS_SENT) {
    return;
  }

  processRespondWithFD(this, fd, headers, statOptions.offset | 0, statOptions.length | 0, streamOptions);
}

function doSendFileFD(session, options, fd, headers, streamOptions, err, stat) {
  var onError = options.onError;

  if (err) {
    tryClose(fd);
    if (onError) onError(err);else this.destroy(err);
    return;
  }

  if (!stat.isFile()) {
    var _err2 = new errors.Error('ERR_HTTP2_SEND_FILE');

    if (onError) onError(_err2);else this.destroy(_err2);
    return;
  }

  if (this.destroyed || this.closed) {
    tryClose(fd);
    this.destroy(new errors.Error('ERR_HTTP2_INVALID_STREAM'));
    return;
  }

  var statOptions = {
    offset: options.offset !== undefined ? options.offset : 0,
    length: options.length !== undefined ? options.length : -1
  }; // options.statCheck is a user-provided function that can be used to
  // verify stat values, override or set headers, or even cancel the
  // response operation. If statCheck explicitly returns false, the
  // response is canceled. The user code may also send a separate type
  // of response so check again for the HEADERS_SENT flag

  if (typeof options.statCheck === 'function' && options.statCheck.call(this, stat, headers) === false || this[kState].flags & STREAM_FLAGS_HEADERS_SENT) {
    return;
  }

  statOptions.length = statOptions.length < 0 ? stat.size - +statOptions.offset : Math.min(stat.size - +statOptions.offset, statOptions.length);
  headers[HTTP2_HEADER_CONTENT_LENGTH] = statOptions.length;
  processRespondWithFD(this, fd, headers, options.offset | 0, statOptions.length | 0, streamOptions);
}

function afterOpen(session, options, headers, streamOptions, err, fd) {
  var state = this[kState];
  var onError = options.onError;

  if (err) {
    if (onError) onError(err);else this.destroy(err);
    return;
  }

  if (this.destroyed || this.closed) {
    tryClose(fd);
    return;
  }

  state.fd = fd;
  fs.fstat(fd, doSendFileFD.bind(this, session, options, fd, headers, streamOptions));
}

function streamOnError(err) {// we swallow the error for parity with HTTP1
  // all the errors that ends here are not critical for the project
}

var ServerHttp2Stream =
/*#__PURE__*/
function (_Http2Stream) {
  _inherits(ServerHttp2Stream, _Http2Stream);

  function ServerHttp2Stream(session, handle, id, options, headers) {
    var _this5;

    _classCallCheck(this, ServerHttp2Stream);

    _this5 = _possibleConstructorReturn(this, _getPrototypeOf(ServerHttp2Stream).call(this, session, options));

    _this5[kInit](id, handle);

    _this5[kProtocol] = headers[HTTP2_HEADER_SCHEME];
    _this5[kAuthority] = headers[HTTP2_HEADER_AUTHORITY];

    _this5.on('error', streamOnError);

    return _this5;
  } // true if the remote peer accepts push streams


  _createClass(ServerHttp2Stream, [{
    key: "pushStream",
    // create a push stream, call the given callback with the created
    // Http2Stream for the push stream.
    value: function pushStream(headers, options, callback) {
      if (!this.pushAllowed) throw new errors.Error('ERR_HTTP2_PUSH_DISABLED');
      if (this[kID] % 2 === 0) throw new errors.Error('ERR_HTTP2_NESTED_PUSH');
      var session = this[kSession];
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: initiating push stream"));
      this[kUpdateTimer]();

      if (typeof options === 'function') {
        callback = options;
        options = undefined;
      }

      if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
      assertIsObject(options, 'options');
      options = Object.assign({}, options);
      options.endStream = !!options.endStream;
      assertIsObject(headers, 'headers');
      headers = Object.assign(Object.create(null), headers);
      if (headers[HTTP2_HEADER_METHOD] === undefined) headers[HTTP2_HEADER_METHOD] = HTTP2_METHOD_GET;
      if (headers[HTTP2_HEADER_AUTHORITY] === undefined) headers[HTTP2_HEADER_AUTHORITY] = this[kAuthority];
      if (headers[HTTP2_HEADER_SCHEME] === undefined) headers[HTTP2_HEADER_SCHEME] = this[kProtocol];
      if (headers[HTTP2_HEADER_PATH] === undefined) headers[HTTP2_HEADER_PATH] = '/';
      var headRequest = false;
      if (headers[HTTP2_HEADER_METHOD] === HTTP2_METHOD_HEAD) headRequest = options.endStream = true;
      options.readable = false;
      var headersList = mapToHeaders(headers);
      if (!Array.isArray(headersList)) throw headersList;
      var streamOptions = options.endStream ? STREAM_OPTION_EMPTY_PAYLOAD : 0;
      var ret = this[kHandle].pushPromise(headersList, streamOptions);
      var err;

      if (typeof ret === 'number') {
        switch (ret) {
          case NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE:
            err = new errors.Error('ERR_HTTP2_OUT_OF_STREAMS');
            break;

          case NGHTTP2_ERR_STREAM_CLOSED:
            err = new errors.Error('ERR_HTTP2_INVALID_STREAM');
            break;

          default:
            err = new NghttpError(ret);
            break;
        }

        process.nextTick(callback, err);
        return;
      }

      var id = ret.id();
      var stream = new ServerHttp2Stream(session, ret, id, options, headers);
      stream[kSentHeaders] = headers;
      if (options.endStream) stream.end();
      if (headRequest) stream[kState].flags |= STREAM_FLAGS_HEAD_REQUEST;
      process.nextTick(callback, null, stream, headers, 0);
    } // Initiate a response on this Http2Stream

  }, {
    key: "respond",
    value: function respond(headers, options) {
      if (this.destroyed || this.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this.headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      var state = this[kState];
      assertIsObject(options, 'options');
      options = Object.assign({}, options);
      var session = this[kSession];
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: initiating response"));
      this[kUpdateTimer]();
      options.endStream = !!options.endStream;
      var streamOptions = 0;
      if (options.endStream) streamOptions |= STREAM_OPTION_EMPTY_PAYLOAD;

      if (options.waitForTrailers) {
        streamOptions |= STREAM_OPTION_GET_TRAILERS;
        state.flags |= STREAM_FLAGS_HAS_TRAILERS;
      }

      headers = processHeaders(headers);
      var statusCode = headers[HTTP2_HEADER_STATUS] |= 0; // Payload/DATA frames are not permitted in these cases so set
      // the options.endStream option to true so that the underlying
      // bits do not attempt to send any.

      if (statusCode === HTTP_STATUS_NO_CONTENT || statusCode === HTTP_STATUS_RESET_CONTENT || statusCode === HTTP_STATUS_NOT_MODIFIED || this.headRequest === true) {
        options.endStream = true;
      }

      var headersList = mapToHeaders(headers, assertValidPseudoHeaderResponse);
      if (!Array.isArray(headersList)) throw headersList;
      this[kSentHeaders] = headers;
      state.flags |= STREAM_FLAGS_HEADERS_SENT; // Close the writable side if the endStream option is set

      if (options.endStream) this.end();
      var ret = this[kHandle].respond(headersList, streamOptions);
      if (ret < 0) this.destroy(new NghttpError(ret));
    } // Initiate a response using an open FD. Note that there are fewer
    // protections with this approach. For one, the fd is not validated by
    // default. In respondWithFile, the file is checked to make sure it is a
    // regular file, here the fd is passed directly. If the underlying
    // mechanism is not able to read from the fd, then the stream will be
    // reset with an error code.

  }, {
    key: "respondWithFD",
    value: function respondWithFD(fd, headers, options) {
      if (this.destroyed || this.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this.headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      var session = this[kSession];
      assertIsObject(options, 'options');
      options = Object.assign({}, options);
      if (options.offset !== undefined && typeof options.offset !== 'number') throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'offset', options.offset);
      if (options.length !== undefined && typeof options.length !== 'number') throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'length', options.length);

      if (options.statCheck !== undefined && typeof options.statCheck !== 'function') {
        throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'statCheck', options.statCheck);
      }

      var streamOptions = 0;

      if (options.waitForTrailers) {
        streamOptions |= STREAM_OPTION_GET_TRAILERS;
        this[kState].flags |= STREAM_FLAGS_HAS_TRAILERS;
      }

      if (typeof fd !== 'number') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'fd', 'number');
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: initiating response"));
      this[kUpdateTimer]();
      headers = processHeaders(headers);
      var statusCode = headers[HTTP2_HEADER_STATUS] |= 0; // Payload/DATA frames are not permitted in these cases

      if (statusCode === HTTP_STATUS_NO_CONTENT || statusCode === HTTP_STATUS_RESET_CONTENT || statusCode === HTTP_STATUS_NOT_MODIFIED) {
        throw new errors.Error('ERR_HTTP2_PAYLOAD_FORBIDDEN', statusCode);
      }

      if (options.statCheck !== undefined) {
        fs.fstat(fd, doSendFD.bind(this, session, options, fd, headers, streamOptions));
        return;
      }

      processRespondWithFD(this, fd, headers, options.offset, options.length, streamOptions);
    } // Initiate a file response on this Http2Stream. The path is passed to
    // fs.open() to acquire the fd with mode 'r', then the fd is passed to
    // fs.fstat(). Assuming fstat is successful, a check is made to ensure
    // that the file is a regular file, then options.statCheck is called,
    // giving the user an opportunity to verify the details and set additional
    // headers. If statCheck returns false, the operation is aborted and no
    // file details are sent.

  }, {
    key: "respondWithFile",
    value: function respondWithFile(path, headers, options) {
      if (this.destroyed || this.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this.headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      assertIsObject(options, 'options');
      options = Object.assign({}, options);
      if (options.offset !== undefined && typeof options.offset !== 'number') throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'offset', options.offset);
      if (options.length !== undefined && typeof options.length !== 'number') throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'length', options.length);

      if (options.statCheck !== undefined && typeof options.statCheck !== 'function') {
        throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'statCheck', options.statCheck);
      }

      var streamOptions = 0;

      if (options.waitForTrailers) {
        streamOptions |= STREAM_OPTION_GET_TRAILERS;
        this[kState].flags |= STREAM_FLAGS_HAS_TRAILERS;
      }

      var session = this[kSession];
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: initiating response"));
      this[kUpdateTimer]();
      headers = processHeaders(headers);
      var statusCode = headers[HTTP2_HEADER_STATUS] |= 0; // Payload/DATA frames are not permitted in these cases

      if (statusCode === HTTP_STATUS_NO_CONTENT || statusCode === HTTP_STATUS_RESET_CONTENT || statusCode === HTTP_STATUS_NOT_MODIFIED) {
        throw new errors.Error('ERR_HTTP2_PAYLOAD_FORBIDDEN', statusCode);
      }

      fs.open(path, 'r', afterOpen.bind(this, session, options, headers, streamOptions));
    } // Sends a block of informational headers. In theory, the HTTP/2 spec
    // allows sending a HEADER block at any time during a streams lifecycle,
    // but the HTTP request/response semantics defined in HTTP/2 places limits
    // such that HEADERS may only be sent *before* or *after* DATA frames.
    // If the block of headers being sent includes a status code, it MUST be
    // a 1xx informational code and it MUST be sent before the request/response
    // headers are sent, or an error will be thrown.

  }, {
    key: "additionalHeaders",
    value: function additionalHeaders(headers) {
      if (this.destroyed || this.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this.headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_AFTER_RESPOND');
      assertIsObject(headers, 'headers');
      headers = Object.assign(Object.create(null), headers);
      var session = this[kSession];
      debug("Http2Stream ".concat(this[kID], " [Http2Session ") + "".concat(sessionName(session[kType]), "]: sending additional headers"));

      if (headers[HTTP2_HEADER_STATUS] != null) {
        var statusCode = headers[HTTP2_HEADER_STATUS] |= 0;
        if (statusCode === HTTP_STATUS_SWITCHING_PROTOCOLS) throw new errors.Error('ERR_HTTP2_STATUS_101');

        if (statusCode < 100 || statusCode >= 200) {
          throw new errors.RangeError('ERR_HTTP2_INVALID_INFO_STATUS', headers[HTTP2_HEADER_STATUS]);
        }
      }

      this[kUpdateTimer]();
      var headersList = mapToHeaders(headers, assertValidPseudoHeaderResponse);
      if (!Array.isArray(headersList)) throw headersList;
      if (!this[kInfoHeaders]) this[kInfoHeaders] = [headers];else this[kInfoHeaders].push(headers);
      var ret = this[kHandle].info(headersList);
      if (ret < 0) this.destroy(new NghttpError(ret));
    }
  }, {
    key: "pushAllowed",
    get: function get() {
      return !this.destroyed && !this.closed && !this.session.closed && !this.session.destroyed && this[kSession].remoteSettings.enablePush;
    }
  }]);

  return ServerHttp2Stream;
}(Http2Stream);

ServerHttp2Stream.prototype[kProceed] = ServerHttp2Stream.prototype.respond;

var ClientHttp2Stream =
/*#__PURE__*/
function (_Http2Stream2) {
  _inherits(ClientHttp2Stream, _Http2Stream2);

  function ClientHttp2Stream(session, handle, id, options) {
    var _this6;

    _classCallCheck(this, ClientHttp2Stream);

    _this6 = _possibleConstructorReturn(this, _getPrototypeOf(ClientHttp2Stream).call(this, session, options));
    _this6[kState].flags |= STREAM_FLAGS_HEADERS_SENT;
    if (id !== undefined) _this6[kInit](id, handle);

    _this6.on('headers', handleHeaderContinue);

    return _this6;
  }

  return ClientHttp2Stream;
}(Http2Stream);

function handleHeaderContinue(headers) {
  if (headers[HTTP2_HEADER_STATUS] === HTTP_STATUS_CONTINUE) this.emit('continue');
}

var setTimeout = {
  configurable: true,
  enumerable: true,
  writable: true,
  value: function value(msecs, callback) {
    if (this.destroyed) return;

    if (typeof msecs !== 'number') {
      throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'msecs', 'number');
    }

    if (msecs === 0) {
      unenroll(this);

      if (callback !== undefined) {
        if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
        this.removeListener('timeout', callback);
      }
    } else {
      enroll(this, msecs);
      this[kUpdateTimer]();

      if (callback !== undefined) {
        if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
        this.once('timeout', callback);
      }
    }

    return this;
  }
};
Object.defineProperty(Http2Stream.prototype, 'setTimeout', setTimeout);
Object.defineProperty(Http2Session.prototype, 'setTimeout', setTimeout); // When the socket emits an error, destroy the associated Http2Session and
// foward it the same error.

function socketOnError(error) {
  var session = this[kSession];

  if (session !== undefined) {
    // We can ignore ECONNRESET after GOAWAY was received as there's nothing
    // we can do and the other side is fully within its rights to do so.
    if (error.code === 'ECONNRESET' && session[kState].goawayCode !== null) return session.destroy();
    debug("Http2Session ".concat(sessionName(session[kType]), ": socket error [") + "".concat(error.message, "]"));
    session.destroy(error);
  }
} // Handles the on('stream') event for a session and forwards
// it on to the server object.


function sessionOnStream(stream, headers, flags, rawHeaders) {
  if (this[kServer] !== undefined) this[kServer].emit('stream', stream, headers, flags, rawHeaders);
}

function sessionOnPriority(stream, parent, weight, exclusive) {
  if (this[kServer] !== undefined) this[kServer].emit('priority', stream, parent, weight, exclusive);
}

function sessionOnError(error) {
  if (this[kServer]) this[kServer].emit('sessionError', error, this);
} // When the session times out on the server, try emitting a timeout event.
// If no handler is registered, destroy the session.


function sessionOnTimeout() {
  // if destroyed or closed already, do nothing
  if (this.destroyed || this.closed) return;
  var server = this[kServer];
  if (!server.emit('timeout', this)) this.destroy(); // No error code, just things down.
}

function connectionListener(socket) {
  debug('Http2Session server: received a connection');
  var options = this[kOptions] || {};

  if (socket.alpnProtocol === false || socket.alpnProtocol === 'http/1.1') {
    // Fallback to HTTP/1.1
    if (options.allowHTTP1 === true) {
      socket.server[kIncomingMessage] = options.Http1IncomingMessage;
      socket.server[kServerResponse] = options.Http1ServerResponse;
      return httpConnectionListener.call(this, socket);
    } // Let event handler deal with the socket


    debug("Unknown protocol from ".concat(socket.remoteAddress, ":").concat(socket.remotePort));

    if (!this.emit('unknownProtocol', socket)) {
      // We don't know what to do, so let's just tell the other side what's
      // going on in a format that they *might* understand.
      socket.end('HTTP/1.0 403 Forbidden\r\n' + 'Content-Type: text/plain\r\n\r\n' + 'Unknown ALPN Protocol, expected `h2` to be available.\n' + 'If this is a HTTP request: The server was not ' + 'configured with the `allowHTTP1` option or a ' + 'listener for the `unknownProtocol` event.\n');
    }

    return;
  }

  socket.on('error', socketOnError);
  socket.on('close', socketOnClose); // Set up the Session

  var session = new ServerHttp2Session(options, socket, this);
  session.on('stream', sessionOnStream);
  session.on('priority', sessionOnPriority);
  session.on('error', sessionOnError);
  if (this.timeout) session.setTimeout(this.timeout, sessionOnTimeout);
  socket[kServer] = this;
  this.emit('session', session);
}

function initializeOptions(options) {
  assertIsObject(options, 'options');
  options = Object.assign({}, options);
  options.allowHalfOpen = true;
  assertIsObject(options.settings, 'options.settings');
  options.settings = Object.assign({}, options.settings); // Used only with allowHTTP1

  options.Http1IncomingMessage = options.Http1IncomingMessage || http.IncomingMessage;
  options.Http1ServerResponse = options.Http1ServerResponse || http.ServerResponse;
  options.Http2ServerRequest = options.Http2ServerRequest || Http2ServerRequest;
  options.Http2ServerResponse = options.Http2ServerResponse || Http2ServerResponse;
  return options;
}

function initializeTLSOptions(options, servername) {
  options = initializeOptions(options);
  options.ALPNProtocols = ['h2'];
  if (options.allowHTTP1 === true) options.ALPNProtocols.push('http/1.1');
  if (servername !== undefined && options.servername === undefined) options.servername = servername;
  return options;
}

function onErrorSecureServerSession(err, socket) {
  if (!this.emit('clientError', err, socket)) socket.destroy(err);
}

var Http2SecureServer =
/*#__PURE__*/
function (_TLSServer) {
  _inherits(Http2SecureServer, _TLSServer);

  function Http2SecureServer(options, requestListener) {
    var _this7;

    _classCallCheck(this, Http2SecureServer);

    options = initializeTLSOptions(options);
    _this7 = _possibleConstructorReturn(this, _getPrototypeOf(Http2SecureServer).call(this, options, connectionListener));
    _this7[kOptions] = options;
    _this7.timeout = kDefaultSocketTimeout;

    _this7.on('newListener', setupCompat);

    if (typeof requestListener === 'function') _this7.on('request', requestListener);

    _this7.on('tlsClientError', onErrorSecureServerSession);

    return _this7;
  }

  _createClass(Http2SecureServer, [{
    key: "setTimeout",
    value: function setTimeout(msecs, callback) {
      this.timeout = msecs;

      if (callback !== undefined) {
        if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
        this.on('timeout', callback);
      }

      return this;
    }
  }]);

  return Http2SecureServer;
}(TLSServer);

var Http2Server =
/*#__PURE__*/
function (_NETServer) {
  _inherits(Http2Server, _NETServer);

  function Http2Server(options, requestListener) {
    var _this8;

    _classCallCheck(this, Http2Server);

    _this8 = _possibleConstructorReturn(this, _getPrototypeOf(Http2Server).call(this, connectionListener));
    _this8[kOptions] = initializeOptions(options);
    _this8.timeout = kDefaultSocketTimeout;

    _this8.on('newListener', setupCompat);

    if (typeof requestListener === 'function') _this8.on('request', requestListener);
    return _this8;
  }

  _createClass(Http2Server, [{
    key: "setTimeout",
    value: function setTimeout(msecs, callback) {
      this.timeout = msecs;

      if (callback !== undefined) {
        if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');
        this.on('timeout', callback);
      }

      return this;
    }
  }]);

  return Http2Server;
}(NETServer);

function setupCompat(ev) {
  if (ev === 'request') {
    this.removeListener('newListener', setupCompat);
    this.on('stream', onServerStream.bind(this, this[kOptions].Http2ServerRequest, this[kOptions].Http2ServerResponse));
  }
}

function socketOnClose() {
  var session = this[kSession];

  if (session !== undefined) {
    debug("Http2Session ".concat(sessionName(session[kType]), ": socket closed"));
    var err = session.connecting ? new errors.Error('ERR_SOCKET_CLOSED') : null;
    var state = session[kState];
    state.streams.forEach(function (stream) {
      return stream.close(NGHTTP2_CANCEL);
    });
    state.pendingStreams.forEach(function (stream) {
      return stream.close(NGHTTP2_CANCEL);
    });
    session.close();
    session[kMaybeDestroy](err);
  }
}

function connect(authority, options, listener) {
  if (typeof options === 'function') {
    listener = options;
    options = undefined;
  }

  assertIsObject(options, 'options');
  options = Object.assign({}, options);
  if (typeof authority === 'string') authority = new URL(authority);
  assertIsObject(authority, 'authority', ['string', 'object', 'URL']);
  var protocol = authority.protocol || options.protocol || 'https:';
  var port = '' + (authority.port !== '' ? authority.port : authority.protocol === 'http:' ? 80 : 443);
  var host = authority.hostname || authority.host || 'localhost';
  var socket;

  if (typeof options.createConnection === 'function') {
    socket = options.createConnection(authority, options);
  } else {
    switch (protocol) {
      case 'http:':
        socket = net.connect(port, host);
        break;

      case 'https:':
        socket = tls.connect(port, host, initializeTLSOptions(options, host));
        break;

      default:
        throw new errors.Error('ERR_HTTP2_UNSUPPORTED_PROTOCOL', protocol);
    }
  }

  socket.on('error', socketOnError);
  socket.on('close', socketOnClose);
  var session = new ClientHttp2Session(options, socket);
  session[kAuthority] = "".concat(options.servername || host, ":").concat(port);
  session[kProtocol] = protocol;
  if (typeof listener === 'function') session.once('connect', listener);
  return session;
} // Support util.promisify


Object.defineProperty(connect, promisify.custom, {
  value: function value(authority, options) {
    var promise = createPromise();
    var server = connect(authority, options, function () {
      return promiseResolve(promise, server);
    });
    return promise;
  }
});

function createSecureServer(options, handler) {
  assertIsObject(options, 'options');
  return new Http2SecureServer(options, handler);
}

function createServer(options, handler) {
  if (typeof options === 'function') {
    handler = options;
    options = {};
  }

  assertIsObject(options, 'options');
  return new Http2Server(options, handler);
} // Returns a Base64 encoded settings frame payload from the given
// object. The value is suitable for passing as the value of the
// HTTP2-Settings header frame.


function getPackedSettings(settings) {
  assertIsObject(settings, 'settings');
  updateSettingsBuffer(validateSettings(settings));
  return binding.packSettings();
}

function getUnpackedSettings(buf) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

  if (!isArrayBufferView(buf)) {
    throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'buf', ['Buffer', 'TypedArray', 'DataView']);
  }

  if (buf.length % 6 !== 0) throw new errors.RangeError('ERR_HTTP2_INVALID_PACKED_SETTINGS_LENGTH');
  var settings = {};
  var offset = 0;

  while (offset < buf.length) {
    var id = buf.readUInt16BE(offset);
    offset += 2;
    var value = buf.readUInt32BE(offset);

    switch (id) {
      case NGHTTP2_SETTINGS_HEADER_TABLE_SIZE:
        settings.headerTableSize = value;
        break;

      case NGHTTP2_SETTINGS_ENABLE_PUSH:
        settings.enablePush = value !== 0;
        break;

      case NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS:
        settings.maxConcurrentStreams = value;
        break;

      case NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE:
        settings.initialWindowSize = value;
        break;

      case NGHTTP2_SETTINGS_MAX_FRAME_SIZE:
        settings.maxFrameSize = value;
        break;

      case NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
        settings.maxHeaderListSize = value;
        break;
    }

    offset += 4;
  }

  if (options != null && options.validate) validateSettings(settings);
  return settings;
} // Exports


module.exports = {
  connect: connect,
  constants: constants,
  createServer: createServer,
  createSecureServer: createSecureServer,
  getDefaultSettings: getDefaultSettings,
  getPackedSettings: getPackedSettings,
  getUnpackedSettings: getUnpackedSettings,
  Http2Session: Http2Session,
  Http2Stream: Http2Stream,
  Http2ServerRequest: Http2ServerRequest,
  Http2ServerResponse: Http2ServerResponse
};
/* eslint-enable no-use-before-define */