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

var EventEmitter = require('events');

var util = require('util');

var _process$binding = process.binding('inspector'),
    Connection = _process$binding.Connection,
    _open = _process$binding.open,
    url = _process$binding.url;

if (!Connection) throw new Error('Inspector is not available');
var connectionSymbol = Symbol('connectionProperty');
var messageCallbacksSymbol = Symbol('messageCallbacks');
var nextIdSymbol = Symbol('nextId');
var onMessageSymbol = Symbol('onMessage');

var Session =
/*#__PURE__*/
function (_EventEmitter) {
  _inherits(Session, _EventEmitter);

  function Session() {
    var _this;

    _classCallCheck(this, Session);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(Session).call(this));
    _this[connectionSymbol] = null;
    _this[nextIdSymbol] = 1;
    _this[messageCallbacksSymbol] = new Map();
    return _this;
  }

  _createClass(Session, [{
    key: "connect",
    value: function connect() {
      var _this2 = this;

      if (this[connectionSymbol]) throw new Error('Already connected');
      this[connectionSymbol] = new Connection(function (message) {
        return _this2[onMessageSymbol](message);
      });
    }
  }, {
    key: onMessageSymbol,
    value: function value(message) {
      var parsed = JSON.parse(message);

      try {
        if (parsed.id) {
          var callback = this[messageCallbacksSymbol].get(parsed.id);
          this[messageCallbacksSymbol]["delete"](parsed.id);
          if (callback) callback(parsed.error || null, parsed.result || null);
        } else {
          this.emit(parsed.method, parsed);
          this.emit('inspectorNotification', parsed);
        }
      } catch (error) {
        process.emitWarning(error);
      }
    }
  }, {
    key: "post",
    value: function post(method, params, callback) {
      if (typeof method !== 'string') {
        throw new TypeError("\"method\" must be a string, got ".concat(_typeof(method), " instead"));
      }

      if (!callback && util.isFunction(params)) {
        callback = params;
        params = null;
      }

      if (params && _typeof(params) !== 'object') {
        throw new TypeError("\"params\" must be an object, got ".concat(_typeof(params), " instead"));
      }

      if (callback && typeof callback !== 'function') {
        throw new TypeError("\"callback\" must be a function, got ".concat(_typeof(callback), " instead"));
      }

      if (!this[connectionSymbol]) {
        throw new Error('Session is not connected');
      }

      var id = this[nextIdSymbol]++;
      var message = {
        id: id,
        method: method
      };

      if (params) {
        message.params = params;
      }

      if (callback) {
        this[messageCallbacksSymbol].set(id, callback);
      }

      this[connectionSymbol].dispatch(JSON.stringify(message));
    }
  }, {
    key: "disconnect",
    value: function disconnect() {
      if (!this[connectionSymbol]) return;
      this[connectionSymbol].disconnect();
      this[connectionSymbol] = null;
      var remainingCallbacks = this[messageCallbacksSymbol].values();
      var _iteratorNormalCompletion = true;
      var _didIteratorError = false;
      var _iteratorError = undefined;

      try {
        for (var _iterator = remainingCallbacks[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
          var callback = _step.value;
          process.nextTick(callback, new Error('Session was closed'));
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

      this[messageCallbacksSymbol].clear();
      this[nextIdSymbol] = 1;
    }
  }]);

  return Session;
}(EventEmitter);

module.exports = {
  open: function open(port, host, wait) {
    return _open(port, host, !!wait);
  },
  close: process._debugEnd,
  url: url,
  Session: Session
};