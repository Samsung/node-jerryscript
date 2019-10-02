'use strict';

var assert = require('assert');

var net = require('net');

var _require = require('internal/cluster/utils'),
    sendHelper = _require.sendHelper;

var getOwnPropertyNames = Object.getOwnPropertyNames;
var uv = process.binding('uv');
module.exports = RoundRobinHandle;

function RoundRobinHandle(key, address, port, addressType, fd) {
  var _this = this;

  this.key = key;
  this.all = {};
  this.free = [];
  this.handles = [];
  this.handle = null;
  this.server = net.createServer(assert.fail);
  if (fd >= 0) this.server.listen({
    fd: fd
  });else if (port >= 0) this.server.listen(port, address);else this.server.listen(address); // UNIX socket path.

  this.server.once('listening', function () {
    _this.handle = _this.server._handle;

    _this.handle.onconnection = function (err, handle) {
      return _this.distribute(err, handle);
    };

    _this.server._handle = null;
    _this.server = null;
  });
}

RoundRobinHandle.prototype.add = function (worker, send) {
  var _this2 = this;

  assert(worker.id in this.all === false);
  this.all[worker.id] = worker;

  var done = function done() {
    if (_this2.handle.getsockname) {
      var out = {};

      _this2.handle.getsockname(out); // TODO(bnoordhuis) Check err.


      send(null, {
        sockname: out
      }, null);
    } else {
      send(null, null, null); // UNIX socket.
    }

    _this2.handoff(worker); // In case there are connections pending.

  };

  if (this.server === null) return done(); // Still busy binding.

  this.server.once('listening', done);
  this.server.once('error', function (err) {
    // Hack: translate 'EADDRINUSE' error string back to numeric error code.
    // It works but ideally we'd have some backchannel between the net and
    // cluster modules for stuff like this.
    var errno = uv['UV_' + err.errno];
    send(errno, null);
  });
};

RoundRobinHandle.prototype.remove = function (worker) {
  if (worker.id in this.all === false) return false;
  delete this.all[worker.id];
  var index = this.free.indexOf(worker);
  if (index !== -1) this.free.splice(index, 1);
  if (getOwnPropertyNames(this.all).length !== 0) return false;

  for (var handle; handle = this.handles.shift(); handle.close()) {
    ;
  }

  this.handle.close();
  this.handle = null;
  return true;
};

RoundRobinHandle.prototype.distribute = function (err, handle) {
  this.handles.push(handle);
  var worker = this.free.shift();
  if (worker) this.handoff(worker);
};

RoundRobinHandle.prototype.handoff = function (worker) {
  var _this3 = this;

  if (worker.id in this.all === false) {
    return; // Worker is closing (or has closed) the server.
  }

  var handle = this.handles.shift();

  if (handle === undefined) {
    this.free.push(worker); // Add to ready queue again.

    return;
  }

  var message = {
    act: 'newconn',
    key: this.key
  };
  sendHelper(worker.process, message, handle, function (reply) {
    if (reply.accepted) handle.close();else _this3.distribute(0, handle); // Worker is shutting down. Send to another.

    _this3.handoff(worker);
  });
};