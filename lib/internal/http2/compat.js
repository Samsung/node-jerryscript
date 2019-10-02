'use strict';

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

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

var Stream = require('stream');

var Readable = Stream.Readable;
var binding = process.binding('http2');
var constants = binding.constants;

var errors = require('internal/errors');

var _require = require('internal/http2/util'),
    kSocket = _require.kSocket;

var kBeginSend = Symbol('begin-send');
var kState = Symbol('state');
var kStream = Symbol('stream');
var kRequest = Symbol('request');
var kResponse = Symbol('response');
var kHeaders = Symbol('headers');
var kRawHeaders = Symbol('rawHeaders');
var kTrailers = Symbol('trailers');
var kRawTrailers = Symbol('rawTrailers');
var kProxySocket = Symbol('proxySocket');
var kSetHeader = Symbol('setHeader');
var kAborted = Symbol('aborted');
var HTTP2_HEADER_AUTHORITY = constants.HTTP2_HEADER_AUTHORITY,
    HTTP2_HEADER_METHOD = constants.HTTP2_HEADER_METHOD,
    HTTP2_HEADER_PATH = constants.HTTP2_HEADER_PATH,
    HTTP2_HEADER_SCHEME = constants.HTTP2_HEADER_SCHEME,
    HTTP2_HEADER_STATUS = constants.HTTP2_HEADER_STATUS,
    HTTP_STATUS_CONTINUE = constants.HTTP_STATUS_CONTINUE,
    HTTP_STATUS_EXPECTATION_FAILED = constants.HTTP_STATUS_EXPECTATION_FAILED,
    HTTP_STATUS_METHOD_NOT_ALLOWED = constants.HTTP_STATUS_METHOD_NOT_ALLOWED,
    HTTP_STATUS_OK = constants.HTTP_STATUS_OK;
var statusMessageWarned = false; // Defines and implements an API compatibility layer on top of the core
// HTTP/2 implementation, intended to provide an interface that is as
// close as possible to the current require('http') API

function assertValidHeader(name, value) {
  var err;

  if (name === '' || typeof name !== 'string') {
    err = new errors.TypeError('ERR_INVALID_HTTP_TOKEN', 'Header name', name);
  } else if (isPseudoHeader(name)) {
    err = new errors.Error('ERR_HTTP2_PSEUDOHEADER_NOT_ALLOWED');
  } else if (value === undefined || value === null) {
    err = new errors.TypeError('ERR_HTTP2_INVALID_HEADER_VALUE', value, name);
  }

  if (err !== undefined) {
    Error.captureStackTrace(err, assertValidHeader);
    throw err;
  }
}

function isPseudoHeader(name) {
  switch (name) {
    case HTTP2_HEADER_STATUS: // :status

    case HTTP2_HEADER_METHOD: // :method

    case HTTP2_HEADER_PATH: // :path

    case HTTP2_HEADER_AUTHORITY: // :authority

    case HTTP2_HEADER_SCHEME:
      // :scheme
      return true;

    default:
      return false;
  }
}

function statusMessageWarn() {
  if (statusMessageWarned === false) {
    process.emitWarning('Status message is not supported by HTTP/2 (RFC7540 8.1.2.4)', 'UnsupportedWarning');
    statusMessageWarned = true;
  }
}

function onStreamData(chunk) {
  var request = this[kRequest];
  if (request !== undefined && !request.push(chunk)) this.pause();
}

function onStreamTrailers(trailers, flags, rawTrailers) {
  var request = this[kRequest];

  if (request !== undefined) {
    var _request$kRawTrailers;

    Object.assign(request[kTrailers], trailers);

    (_request$kRawTrailers = request[kRawTrailers]).push.apply(_request$kRawTrailers, _toConsumableArray(rawTrailers));
  }
}

function onStreamEnd() {
  // Cause the request stream to end as well.
  var request = this[kRequest];
  if (request !== undefined) this[kRequest].push(null);
}

function onStreamError(error) {// this is purposefully left blank
  //
  // errors in compatibility mode are
  // not forwarded to the request
  // and response objects.
}

function onRequestPause() {
  this[kStream].pause();
}

function onRequestResume() {
  this[kStream].resume();
}

function onStreamDrain() {
  var response = this[kResponse];
  if (response !== undefined) response.emit('drain');
}

function onStreamAbortedRequest() {
  var request = this[kRequest];

  if (request !== undefined && request[kState].closed === false) {
    request[kAborted] = true;
    request.emit('aborted');
  }
}

function onStreamAbortedResponse() {// non-op for now
}

function resumeStream(stream) {
  stream.resume();
}

var proxySocketHandler = {
  get: function get(stream, prop) {
    switch (prop) {
      case 'on':
      case 'once':
      case 'end':
      case 'emit':
      case 'destroy':
        return stream[prop].bind(stream);

      case 'writable':
      case 'destroyed':
        return stream[prop];

      case 'readable':
        if (stream.destroyed) return false;
        var request = stream[kRequest];
        return request ? request.readable : stream.readable;

      case 'setTimeout':
        var session = stream.session;
        if (session !== undefined) return session.setTimeout.bind(session);
        return stream.setTimeout.bind(stream);

      case 'write':
      case 'read':
      case 'pause':
      case 'resume':
        throw new errors.Error('ERR_HTTP2_NO_SOCKET_MANIPULATION');

      default:
        var ref = stream.session !== undefined ? stream.session[kSocket] : stream;
        var value = ref[prop];
        return typeof value === 'function' ? value.bind(ref) : value;
    }
  },
  getPrototypeOf: function getPrototypeOf(stream) {
    if (stream.session !== undefined) return Reflect.getPrototypeOf(stream.session[kSocket]);
    return Reflect.getPrototypeOf(stream);
  },
  set: function set(stream, prop, value) {
    switch (prop) {
      case 'writable':
      case 'readable':
      case 'destroyed':
      case 'on':
      case 'once':
      case 'end':
      case 'emit':
      case 'destroy':
        stream[prop] = value;
        return true;

      case 'setTimeout':
        var session = stream.session;
        if (session !== undefined) session.setTimeout = value;else stream.setTimeout = value;
        return true;

      case 'write':
      case 'read':
      case 'pause':
      case 'resume':
        throw new errors.Error('ERR_HTTP2_NO_SOCKET_MANIPULATION');

      default:
        var ref = stream.session !== undefined ? stream.session[kSocket] : stream;
        ref[prop] = value;
        return true;
    }
  }
};

function onStreamCloseRequest() {
  var req = this[kRequest];
  if (req === undefined) return;
  var state = req[kState];
  state.closed = true;
  req.push(null); // if the user didn't interact with incoming data and didn't pipe it,
  // dump it for compatibility with http1

  if (!state.didRead && !req._readableState.resumeScheduled) req.resume();
  this[kProxySocket] = null;
  this[kRequest] = undefined;
  req.emit('close');
}

function onStreamTimeout(kind) {
  return function onStreamTimeout() {
    var obj = this[kind];
    obj.emit('timeout');
  };
}

var Http2ServerRequest =
/*#__PURE__*/
function (_Readable) {
  _inherits(Http2ServerRequest, _Readable);

  function Http2ServerRequest(stream, headers, options, rawHeaders) {
    var _this;

    _classCallCheck(this, Http2ServerRequest);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(Http2ServerRequest).call(this, options));
    _this[kState] = {
      closed: false,
      didRead: false
    };
    _this[kHeaders] = headers;
    _this[kRawHeaders] = rawHeaders;
    _this[kTrailers] = {};
    _this[kRawTrailers] = [];
    _this[kStream] = stream;
    _this[kAborted] = false;
    stream[kProxySocket] = null;
    stream[kRequest] = _assertThisInitialized(_this); // Pause the stream..

    stream.on('trailers', onStreamTrailers);
    stream.on('end', onStreamEnd);
    stream.on('error', onStreamError);
    stream.on('aborted', onStreamAbortedRequest);
    stream.on('close', onStreamCloseRequest);
    stream.on('timeout', onStreamTimeout(kRequest));

    _this.on('pause', onRequestPause);

    _this.on('resume', onRequestResume);

    return _this;
  }

  _createClass(Http2ServerRequest, [{
    key: "_read",
    value: function _read(nread) {
      var state = this[kState];

      if (!state.closed) {
        if (!state.didRead) {
          state.didRead = true;
          this[kStream].on('data', onStreamData);
        } else {
          process.nextTick(resumeStream, this[kStream]);
        }
      } else {
        this.emit('error', new errors.Error('ERR_HTTP2_INVALID_STREAM'));
      }
    }
  }, {
    key: "setTimeout",
    value: function setTimeout(msecs, callback) {
      if (this[kState].closed) return;
      this[kStream].setTimeout(msecs, callback);
    }
  }, {
    key: "aborted",
    get: function get() {
      return this[kAborted];
    }
  }, {
    key: "complete",
    get: function get() {
      return this._readableState.ended || this[kState].closed || this[kStream].destroyed;
    }
  }, {
    key: "stream",
    get: function get() {
      return this[kStream];
    }
  }, {
    key: "headers",
    get: function get() {
      return this[kHeaders];
    }
  }, {
    key: "rawHeaders",
    get: function get() {
      return this[kRawHeaders];
    }
  }, {
    key: "trailers",
    get: function get() {
      return this[kTrailers];
    }
  }, {
    key: "rawTrailers",
    get: function get() {
      return this[kRawTrailers];
    }
  }, {
    key: "httpVersionMajor",
    get: function get() {
      return 2;
    }
  }, {
    key: "httpVersionMinor",
    get: function get() {
      return 0;
    }
  }, {
    key: "httpVersion",
    get: function get() {
      return '2.0';
    }
  }, {
    key: "socket",
    get: function get() {
      var stream = this[kStream];
      var proxySocket = stream[kProxySocket];
      if (proxySocket === null) return stream[kProxySocket] = new Proxy(stream, proxySocketHandler);
      return proxySocket;
    }
  }, {
    key: "connection",
    get: function get() {
      return this.socket;
    }
  }, {
    key: "method",
    get: function get() {
      return this[kHeaders][HTTP2_HEADER_METHOD];
    },
    set: function set(method) {
      if (typeof method !== 'string' || method.trim() === '') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'method', 'string');
      this[kHeaders][HTTP2_HEADER_METHOD] = method;
    }
  }, {
    key: "authority",
    get: function get() {
      return this[kHeaders][HTTP2_HEADER_AUTHORITY];
    }
  }, {
    key: "scheme",
    get: function get() {
      return this[kHeaders][HTTP2_HEADER_SCHEME];
    }
  }, {
    key: "url",
    get: function get() {
      return this[kHeaders][HTTP2_HEADER_PATH];
    },
    set: function set(url) {
      this[kHeaders][HTTP2_HEADER_PATH] = url;
    }
  }]);

  return Http2ServerRequest;
}(Readable);

function onStreamTrailersReady() {
  this.sendTrailers(this[kResponse][kTrailers]);
}

function onStreamCloseResponse() {
  var res = this[kResponse];
  if (res === undefined) return;
  var state = res[kState];
  if (this.headRequest !== state.headRequest) return;
  state.closed = true;
  this[kProxySocket] = null;
  this.removeListener('wantTrailers', onStreamTrailersReady);
  this[kResponse] = undefined;
  res.emit('finish');
  res.emit('close');
}

var Http2ServerResponse =
/*#__PURE__*/
function (_Stream) {
  _inherits(Http2ServerResponse, _Stream);

  function Http2ServerResponse(stream, options) {
    var _this2;

    _classCallCheck(this, Http2ServerResponse);

    _this2 = _possibleConstructorReturn(this, _getPrototypeOf(Http2ServerResponse).call(this, options));
    _this2[kState] = {
      closed: false,
      ending: false,
      headRequest: false,
      sendDate: true,
      statusCode: HTTP_STATUS_OK
    };
    _this2[kHeaders] = Object.create(null);
    _this2[kTrailers] = Object.create(null);
    _this2[kStream] = stream;
    stream[kProxySocket] = null;
    stream[kResponse] = _assertThisInitialized(_this2);
    _this2.writable = true;
    stream.on('drain', onStreamDrain);
    stream.on('aborted', onStreamAbortedResponse);
    stream.on('close', onStreamCloseResponse);
    stream.on('wantTrailers', onStreamTrailersReady);
    stream.on('timeout', onStreamTimeout(kResponse));
    return _this2;
  } // User land modules such as finalhandler just check truthiness of this
  // but if someone is actually trying to use this for more than that
  // then we simply can't support such use cases


  _createClass(Http2ServerResponse, [{
    key: "setTrailer",
    value: function setTrailer(name, value) {
      if (typeof name !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'name', 'string');
      name = name.trim().toLowerCase();
      assertValidHeader(name, value);
      this[kTrailers][name] = value;
    }
  }, {
    key: "addTrailers",
    value: function addTrailers(headers) {
      var keys = Object.keys(headers);
      var key = '';

      for (var i = 0; i < keys.length; i++) {
        key = keys[i];
        this.setTrailer(key, headers[key]);
      }
    }
  }, {
    key: "getHeader",
    value: function getHeader(name) {
      if (typeof name !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'name', 'string');
      name = name.trim().toLowerCase();
      return this[kHeaders][name];
    }
  }, {
    key: "getHeaderNames",
    value: function getHeaderNames() {
      return Object.keys(this[kHeaders]);
    }
  }, {
    key: "getHeaders",
    value: function getHeaders() {
      return Object.assign({}, this[kHeaders]);
    }
  }, {
    key: "hasHeader",
    value: function hasHeader(name) {
      if (typeof name !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'name', 'string');
      name = name.trim().toLowerCase();
      return Object.prototype.hasOwnProperty.call(this[kHeaders], name);
    }
  }, {
    key: "removeHeader",
    value: function removeHeader(name) {
      if (typeof name !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'name', 'string');
      if (this[kStream].headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      name = name.trim().toLowerCase();
      delete this[kHeaders][name];
    }
  }, {
    key: "setHeader",
    value: function setHeader(name, value) {
      if (typeof name !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'name', 'string');
      if (this[kStream].headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      this[kSetHeader](name, value);
    }
  }, {
    key: kSetHeader,
    value: function value(name, _value) {
      name = name.trim().toLowerCase();
      assertValidHeader(name, _value);
      this[kHeaders][name] = _value;
    }
  }, {
    key: "flushHeaders",
    value: function flushHeaders() {
      var state = this[kState];
      if (!state.closed && !this[kStream].headersSent) this.writeHead(state.statusCode);
    }
  }, {
    key: "writeHead",
    value: function writeHead(statusCode, statusMessage, headers) {
      var state = this[kState];
      if (state.closed) throw new errors.Error('ERR_HTTP2_INVALID_STREAM');
      if (this[kStream].headersSent) throw new errors.Error('ERR_HTTP2_HEADERS_SENT');
      if (typeof statusMessage === 'string') statusMessageWarn();
      if (headers === undefined && _typeof(statusMessage) === 'object') headers = statusMessage;

      if (_typeof(headers) === 'object') {
        var keys = Object.keys(headers);
        var key = '';

        for (var i = 0; i < keys.length; i++) {
          key = keys[i];
          this[kSetHeader](key, headers[key]);
        }
      }

      state.statusCode = statusCode;
      this[kBeginSend]();
    }
  }, {
    key: "write",
    value: function write(chunk, encoding, cb) {
      if (typeof encoding === 'function') {
        cb = encoding;
        encoding = 'utf8';
      }

      if (this[kState].closed) {
        var err = new errors.Error('ERR_HTTP2_INVALID_STREAM');
        if (typeof cb === 'function') process.nextTick(cb, err);else throw err;
        return;
      }

      var stream = this[kStream];
      if (!stream.headersSent) this.writeHead(this[kState].statusCode);
      return stream.write(chunk, encoding, cb);
    }
  }, {
    key: "end",
    value: function end(chunk, encoding, cb) {
      var stream = this[kStream];
      var state = this[kState];

      if ((state.closed || state.ending) && state.headRequest === stream.headRequest) {
        return false;
      }

      if (typeof chunk === 'function') {
        cb = chunk;
        chunk = null;
      } else if (typeof encoding === 'function') {
        cb = encoding;
        encoding = 'utf8';
      }

      if (chunk !== null && chunk !== undefined) this.write(chunk, encoding);
      var isFinished = this.finished;
      state.headRequest = stream.headRequest;
      state.ending = true;

      if (typeof cb === 'function') {
        if (isFinished) this.once('finish', cb);else stream.once('finish', cb);
      }

      if (!stream.headersSent) this.writeHead(this[kState].statusCode);
      if (isFinished) onStreamCloseResponse.call(stream);else stream.end();
    }
  }, {
    key: "destroy",
    value: function destroy(err) {
      if (this[kState].closed) return;
      this[kStream].destroy(err);
    }
  }, {
    key: "setTimeout",
    value: function setTimeout(msecs, callback) {
      if (this[kState].closed) return;
      this[kStream].setTimeout(msecs, callback);
    }
  }, {
    key: "createPushResponse",
    value: function createPushResponse(headers, callback) {
      if (typeof callback !== 'function') throw new errors.TypeError('ERR_INVALID_CALLBACK');

      if (this[kState].closed) {
        process.nextTick(callback, new errors.Error('ERR_HTTP2_INVALID_STREAM'));
        return;
      }

      this[kStream].pushStream(headers, {}, function (err, stream, headers, options) {
        if (err) {
          callback(err);
          return;
        }

        callback(null, new Http2ServerResponse(stream));
      });
    }
  }, {
    key: kBeginSend,
    value: function value() {
      var state = this[kState];
      var headers = this[kHeaders];
      headers[HTTP2_HEADER_STATUS] = state.statusCode;
      var options = {
        endStream: state.ending,
        waitForTrailers: true
      };
      this[kStream].respond(headers, options);
    } // TODO doesn't support callbacks

  }, {
    key: "writeContinue",
    value: function writeContinue() {
      var stream = this[kStream];
      if (stream.headersSent || this[kState].closed) return false;
      stream.additionalHeaders(_defineProperty({}, HTTP2_HEADER_STATUS, HTTP_STATUS_CONTINUE));
      return true;
    }
  }, {
    key: "_header",
    get: function get() {
      return this.headersSent;
    }
  }, {
    key: "finished",
    get: function get() {
      var stream = this[kStream];
      return stream.destroyed || stream._writableState.ended || this[kState].closed;
    }
  }, {
    key: "socket",
    get: function get() {
      // this is compatible with http1 which removes socket reference
      // only from ServerResponse but not IncomingMessage
      if (this[kState].closed) return;
      var stream = this[kStream];
      var proxySocket = stream[kProxySocket];
      if (proxySocket === null) return stream[kProxySocket] = new Proxy(stream, proxySocketHandler);
      return proxySocket;
    }
  }, {
    key: "connection",
    get: function get() {
      return this.socket;
    }
  }, {
    key: "stream",
    get: function get() {
      return this[kStream];
    }
  }, {
    key: "headersSent",
    get: function get() {
      return this[kStream].headersSent;
    }
  }, {
    key: "sendDate",
    get: function get() {
      return this[kState].sendDate;
    },
    set: function set(bool) {
      this[kState].sendDate = Boolean(bool);
    }
  }, {
    key: "statusCode",
    get: function get() {
      return this[kState].statusCode;
    },
    set: function set(code) {
      code |= 0;
      if (code >= 100 && code < 200) throw new errors.RangeError('ERR_HTTP2_INFO_STATUS_NOT_ALLOWED');
      if (code < 100 || code > 599) throw new errors.RangeError('ERR_HTTP2_STATUS_INVALID', code);
      this[kState].statusCode = code;
    }
  }, {
    key: "statusMessage",
    get: function get() {
      statusMessageWarn();
      return '';
    },
    set: function set(msg) {
      statusMessageWarn();
    }
  }]);

  return Http2ServerResponse;
}(Stream);

function onServerStream(ServerRequest, ServerResponse, stream, headers, flags, rawHeaders) {
  var server = this;
  var request = new ServerRequest(stream, headers, undefined, rawHeaders);
  var response = new ServerResponse(stream); // Check for the CONNECT method

  var method = headers[HTTP2_HEADER_METHOD];

  if (method === 'CONNECT') {
    if (!server.emit('connect', request, response)) {
      response.statusCode = HTTP_STATUS_METHOD_NOT_ALLOWED;
      response.end();
    }

    return;
  } // Check for Expectations


  if (headers.expect !== undefined) {
    if (headers.expect === '100-continue') {
      if (server.listenerCount('checkContinue')) {
        server.emit('checkContinue', request, response);
      } else {
        response.writeContinue();
        server.emit('request', request, response);
      }
    } else if (server.listenerCount('checkExpectation')) {
      server.emit('checkExpectation', request, response);
    } else {
      response.statusCode = HTTP_STATUS_EXPECTATION_FAILED;
      response.end();
    }

    return;
  }

  server.emit('request', request, response);
}

module.exports = {
  onServerStream: onServerStream,
  Http2ServerRequest: Http2ServerRequest,
  Http2ServerResponse: Http2ServerResponse
};