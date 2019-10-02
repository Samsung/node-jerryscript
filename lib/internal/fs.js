'use strict';

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require2 = require('stream'),
    Writable = _require2.Writable;

var fs = require('fs');

var util = require('util');

var _process$binding$fs = process.binding('constants').fs,
    O_APPEND = _process$binding$fs.O_APPEND,
    O_CREAT = _process$binding$fs.O_CREAT,
    O_EXCL = _process$binding$fs.O_EXCL,
    O_RDONLY = _process$binding$fs.O_RDONLY,
    O_RDWR = _process$binding$fs.O_RDWR,
    O_SYNC = _process$binding$fs.O_SYNC,
    O_TRUNC = _process$binding$fs.O_TRUNC,
    O_WRONLY = _process$binding$fs.O_WRONLY;

function assertEncoding(encoding) {
  if (encoding && !Buffer.isEncoding(encoding)) {
    throw new Error("Unknown encoding: ".concat(encoding));
  }
}

function stringToFlags(flag) {
  if (typeof flag === 'number') {
    return flag;
  }

  switch (flag) {
    case 'r':
      return O_RDONLY;

    case 'rs': // Fall through.

    case 'sr':
      return O_RDONLY | O_SYNC;

    case 'r+':
      return O_RDWR;

    case 'rs+': // Fall through.

    case 'sr+':
      return O_RDWR | O_SYNC;

    case 'w':
      return O_TRUNC | O_CREAT | O_WRONLY;

    case 'wx': // Fall through.

    case 'xw':
      return O_TRUNC | O_CREAT | O_WRONLY | O_EXCL;

    case 'w+':
      return O_TRUNC | O_CREAT | O_RDWR;

    case 'wx+': // Fall through.

    case 'xw+':
      return O_TRUNC | O_CREAT | O_RDWR | O_EXCL;

    case 'a':
      return O_APPEND | O_CREAT | O_WRONLY;

    case 'ax': // Fall through.

    case 'xa':
      return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;

    case 'as': // Fall through.

    case 'sa':
      return O_APPEND | O_CREAT | O_WRONLY | O_SYNC;

    case 'a+':
      return O_APPEND | O_CREAT | O_RDWR;

    case 'ax+': // Fall through.

    case 'xa+':
      return O_APPEND | O_CREAT | O_RDWR | O_EXCL;

    case 'as+': // Fall through.

    case 'sa+':
      return O_APPEND | O_CREAT | O_RDWR | O_SYNC;
  }

  throw new Error('Unknown file open flag: ' + flag);
} // Temporary hack for process.stdout and process.stderr when piped to files.


function SyncWriteStream(fd, options) {
  var _this = this;

  Writable.call(this);
  options = options || {};
  this.fd = fd;
  this.readable = false;
  this.autoClose = options.autoClose === undefined ? true : options.autoClose;
  this.on('end', function () {
    return _this._destroy();
  });
}

util.inherits(SyncWriteStream, Writable);

SyncWriteStream.prototype._write = function (chunk, encoding, cb) {
  fs.writeSync(this.fd, chunk, 0, chunk.length);
  cb();
  return true;
};

SyncWriteStream.prototype._destroy = function () {
  if (this.fd === null) // already destroy()ed
    return;
  if (this.autoClose) fs.closeSync(this.fd);
  this.fd = null;
  return true;
};

SyncWriteStream.prototype.destroySoon = SyncWriteStream.prototype.destroy = function () {
  this._destroy();

  this.emit('close');
  return true;
};

module.exports = {
  assertEncoding: assertEncoding,
  stringToFlags: stringToFlags,
  SyncWriteStream: SyncWriteStream,
  realpathCacheKey: Symbol('realpathCacheKey')
};