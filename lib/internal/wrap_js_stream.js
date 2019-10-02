'use strict';

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

var assert = require('assert');

var util = require('util');

var _require = require('net'),
    Socket = _require.Socket;

var _process$binding = process.binding('js_stream'),
    JSStream = _process$binding.JSStream; // TODO(bmeurer): Change this back to const once hole checks are
// properly optimized away early in Ignition+TurboFan.


var Buffer = require('buffer').Buffer;

var uv = process.binding('uv');
var debug = util.debuglog('stream_wrap');
/* This class serves as a wrapper for when the C++ side of Node wants access
 * to a standard JS stream. For example, TLS or HTTP do not operate on network
 * resources conceptually, although that is the common case and what we are
 * optimizing for; in theory, they are completely composable and can work with
 * any stream resource they see.
 *
 * For the common case, i.e. a TLS socket wrapping around a net.Socket, we
 * can skip going through the JS layer and let TLS access the raw C++ handle
 * of a net.Socket. The flipside of this is that, to maintain composability,
 * we need a way to create "fake" net.Socket instances that call back into a
 * "real" JavaScript stream. JSStreamWrap is exactly this.
 */

var JSStreamWrap =
/*#__PURE__*/
function (_Socket) {
  _inherits(JSStreamWrap, _Socket);

  function JSStreamWrap(stream) {
    var _this;

    _classCallCheck(this, JSStreamWrap);

    var handle = new JSStream();

    handle.close = function (cb) {
      debug('close');

      _this.doClose(cb);
    };

    handle.isAlive = function () {
      return _this.isAlive();
    };

    handle.isClosing = function () {
      return _this.isClosing();
    };

    handle.onreadstart = function () {
      return _this.readStart();
    };

    handle.onreadstop = function () {
      return _this.readStop();
    };

    handle.onshutdown = function (req) {
      return _this.doShutdown(req);
    };

    handle.onwrite = function (req, bufs) {
      return _this.doWrite(req, bufs);
    };

    stream.pause();
    stream.on('error', function (err) {
      return _this.emit('error', err);
    });

    var ondata = function ondata(chunk) {
      if (!(chunk instanceof Buffer)) {
        // Make sure that no further `data` events will happen.
        stream.pause();
        stream.removeListener('data', ondata);

        _this.emit('error', new Error('Stream has StringDecoder'));

        return;
      }

      debug('data', chunk.length);
      if (_this._handle) _this._handle.readBuffer(chunk);
    };

    stream.on('data', ondata);
    stream.once('end', function () {
      debug('end');
      if (_this._handle) _this._handle.emitEOF();
    });
    _this = _possibleConstructorReturn(this, _getPrototypeOf(JSStreamWrap).call(this, {
      handle: handle,
      manualStart: true
    }));
    _this.stream = stream;
    _this._list = null;

    _this.read(0);

    return _this;
  } // Legacy


  _createClass(JSStreamWrap, [{
    key: "isAlive",
    value: function isAlive() {
      return true;
    }
  }, {
    key: "isClosing",
    value: function isClosing() {
      return !this.readable || !this.writable;
    }
  }, {
    key: "readStart",
    value: function readStart() {
      this.stream.resume();
      return 0;
    }
  }, {
    key: "readStop",
    value: function readStop() {
      this.stream.pause();
      return 0;
    }
  }, {
    key: "doShutdown",
    value: function doShutdown(req) {
      var _this2 = this;

      var handle = this._handle;

      var item = this._enqueue('shutdown', req);

      this.stream.end(function () {
        // Ensure that write was dispatched
        setImmediate(function () {
          if (!_this2._dequeue(item)) return;
          handle.finishShutdown(req, 0);
        });
      });
      return 0;
    }
  }, {
    key: "doWrite",
    value: function doWrite(req, bufs) {
      var self = this;
      var handle = this._handle;
      var pending = bufs.length; // Queue the request to be able to cancel it

      var item = this._enqueue('write', req);

      this.stream.cork();

      for (var n = 0; n < bufs.length; n++) {
        this.stream.write(bufs[n], done);
      }

      this.stream.uncork();

      function done(err) {
        if (!err && --pending !== 0) return; // Ensure that this is called once in case of error

        pending = 0; // Ensure that write was dispatched

        setImmediate(function () {
          // Do not invoke callback twice
          if (!self._dequeue(item)) return;
          var errCode = 0;

          if (err) {
            if (err.code && uv['UV_' + err.code]) errCode = uv['UV_' + err.code];else errCode = uv.UV_EPIPE;
          }

          handle.doAfterWrite(req);
          handle.finishWrite(req, errCode);
        });
      }

      return 0;
    }
  }, {
    key: "_enqueue",
    value: function _enqueue(type, req) {
      var item = new QueueItem(type, req);

      if (this._list === null) {
        this._list = item;
        return item;
      }

      item.next = this._list.next;
      item.prev = this._list;
      item.next.prev = item;
      item.prev.next = item;
      return item;
    }
  }, {
    key: "_dequeue",
    value: function _dequeue(item) {
      assert(item instanceof QueueItem);
      var next = item.next;
      var prev = item.prev;
      if (next === null && prev === null) return false;
      item.next = null;
      item.prev = null;

      if (next === item) {
        prev = null;
        next = null;
      } else {
        prev.next = next;
        next.prev = prev;
      }

      if (this._list === item) this._list = next;
      return true;
    }
  }, {
    key: "doClose",
    value: function doClose(cb) {
      var _this3 = this;

      var handle = this._handle;
      setImmediate(function () {
        while (_this3._list !== null) {
          var item = _this3._list;
          var req = item.req;

          _this3._dequeue(item);

          var errCode = uv.UV_ECANCELED;

          if (item.type === 'write') {
            handle.doAfterWrite(req);
            handle.finishWrite(req, errCode);
          } else if (item.type === 'shutdown') {
            handle.finishShutdown(req, errCode);
          }
        } // Should be already set by net.js


        assert.strictEqual(_this3._handle, null);
        cb();
      });
    }
  }], [{
    key: "StreamWrap",
    get: function get() {
      return JSStreamWrap;
    }
  }]);

  return JSStreamWrap;
}(Socket);

function QueueItem(type, req) {
  this.type = type;
  this.req = req;
  this.prev = this;
  this.next = this;
}

module.exports = JSStreamWrap;