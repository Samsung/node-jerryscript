'use strict';

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var EventEmitter = require('events');

var internalUtil = require('internal/util');

var util = require('util');

var defineProperty = Object.defineProperty;
var suicideDeprecationMessage = 'worker.suicide is deprecated. Please use worker.exitedAfterDisconnect.';
module.exports = Worker; // Common Worker implementation shared between the cluster master and workers.

function Worker(options) {
  var _this = this;

  if (!(this instanceof Worker)) return new Worker(options);
  EventEmitter.call(this);
  if (options === null || _typeof(options) !== 'object') options = {};
  this.exitedAfterDisconnect = undefined;
  defineProperty(this, 'suicide', {
    get: internalUtil.deprecate(function () {
      return _this.exitedAfterDisconnect;
    }, suicideDeprecationMessage, 'DEP0007'),
    set: internalUtil.deprecate(function (val) {
      _this.exitedAfterDisconnect = val;
    }, suicideDeprecationMessage, 'DEP0007'),
    enumerable: true
  });
  this.state = options.state || 'none';
  this.id = options.id | 0;

  if (options.process) {
    this.process = options.process;
    this.process.on('error', function (code, signal) {
      return _this.emit('error', code, signal);
    });
    this.process.on('message', function (message, handle) {
      return _this.emit('message', message, handle);
    });
  }
}

util.inherits(Worker, EventEmitter);

Worker.prototype.kill = function () {
  this.destroy.apply(this, arguments);
};

Worker.prototype.send = function () {
  return this.process.send.apply(this.process, arguments);
};

Worker.prototype.isDead = function () {
  return this.process.exitCode != null || this.process.signalCode != null;
};

Worker.prototype.isConnected = function () {
  return this.process.connected;
};