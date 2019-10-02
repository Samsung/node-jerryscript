'use strict';

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

var EventEmitter = require('events'); // This object keeps track of the sockets that are sent


var SocketListSend =
/*#__PURE__*/
function (_EventEmitter) {
  _inherits(SocketListSend, _EventEmitter);

  function SocketListSend(child, key) {
    var _this;

    _classCallCheck(this, SocketListSend);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(SocketListSend).call(this));
    _this.key = key;
    _this.child = child;
    child.once('exit', function () {
      return _this.emit('exit', _assertThisInitialized(_this));
    });
    return _this;
  }

  _createClass(SocketListSend, [{
    key: "_request",
    value: function _request(msg, cmd, callback) {
      var self = this;
      if (!this.child.connected) return onclose();
      this.child.send(msg);

      function onclose() {
        self.child.removeListener('internalMessage', onreply);
        callback(new Error('child closed before reply'));
      }

      function onreply(msg) {
        if (!(msg.cmd === cmd && msg.key === self.key)) return;
        self.child.removeListener('disconnect', onclose);
        self.child.removeListener('internalMessage', onreply);
        callback(null, msg);
      }

      this.child.once('disconnect', onclose);
      this.child.on('internalMessage', onreply);
    }
  }, {
    key: "close",
    value: function close(callback) {
      this._request({
        cmd: 'NODE_SOCKET_NOTIFY_CLOSE',
        key: this.key
      }, 'NODE_SOCKET_ALL_CLOSED', callback);
    }
  }, {
    key: "getConnections",
    value: function getConnections(callback) {
      this._request({
        cmd: 'NODE_SOCKET_GET_COUNT',
        key: this.key
      }, 'NODE_SOCKET_COUNT', function (err, msg) {
        if (err) return callback(err);
        callback(null, msg.count);
      });
    }
  }]);

  return SocketListSend;
}(EventEmitter); // This object keeps track of the sockets that are received


var SocketListReceive =
/*#__PURE__*/
function (_EventEmitter2) {
  _inherits(SocketListReceive, _EventEmitter2);

  function SocketListReceive(child, key) {
    var _this2;

    _classCallCheck(this, SocketListReceive);

    _this2 = _possibleConstructorReturn(this, _getPrototypeOf(SocketListReceive).call(this));
    _this2.connections = 0;
    _this2.key = key;
    _this2.child = child;

    function onempty(self) {
      if (!self.child.connected) return;
      self.child.send({
        cmd: 'NODE_SOCKET_ALL_CLOSED',
        key: self.key
      });
    }

    _this2.child.on('internalMessage', function (msg) {
      if (msg.key !== _this2.key) return;

      if (msg.cmd === 'NODE_SOCKET_NOTIFY_CLOSE') {
        // Already empty
        if (_this2.connections === 0) return onempty(_assertThisInitialized(_this2)); // Wait for sockets to get closed

        _this2.once('empty', onempty);
      } else if (msg.cmd === 'NODE_SOCKET_GET_COUNT') {
        if (!_this2.child.connected) return;

        _this2.child.send({
          cmd: 'NODE_SOCKET_COUNT',
          key: _this2.key,
          count: _this2.connections
        });
      }
    });

    return _this2;
  }

  _createClass(SocketListReceive, [{
    key: "add",
    value: function add(obj) {
      var _this3 = this;

      this.connections++; // Notify the previous owner of the socket about its state change

      obj.socket.once('close', function () {
        _this3.connections--;
        if (_this3.connections === 0) _this3.emit('empty', _this3);
      });
    }
  }]);

  return SocketListReceive;
}(EventEmitter);

module.exports = {
  SocketListSend: SocketListSend,
  SocketListReceive: SocketListReceive
};