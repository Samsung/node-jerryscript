'use strict';

var _process$binding = process.binding('util'),
    safeToString = _process$binding.safeToString;

var promiseRejectEvent = process._promiseRejectEvent;
var hasBeenNotifiedProperty = new Map();
var promiseToGuidProperty = new Map();
var pendingUnhandledRejections = [];
var lastPromiseId = 1;
exports.setup = setupPromises;

function getAsynchronousRejectionWarningObject(uid) {
  return new Error('Promise rejection was handled ' + "asynchronously (rejection id: ".concat(uid, ")"));
}

function setupPromises(scheduleMicrotasks, _setupPromises) {
  var deprecationWarned = false;

  _setupPromises(function (event, promise, reason) {
    if (event === promiseRejectEvent.unhandled) unhandledRejection(promise, reason);else if (event === promiseRejectEvent.handled) rejectionHandled(promise);else require('assert').fail(null, null, 'unexpected PromiseRejectEvent');
  });

  function unhandledRejection(promise, reason) {
    hasBeenNotifiedProperty.set(promise, false);
    promiseToGuidProperty.set(promise, lastPromiseId++);
    addPendingUnhandledRejection(promise, reason);
  }

  function rejectionHandled(promise) {
    var hasBeenNotified = hasBeenNotifiedProperty.get(promise);

    if (hasBeenNotified !== undefined) {
      hasBeenNotifiedProperty["delete"](promise);
      var uid = promiseToGuidProperty.get(promise);
      promiseToGuidProperty["delete"](promise);

      if (hasBeenNotified === true) {
        var warning = null;

        if (!process.listenerCount('rejectionHandled')) {
          // Generate the warning object early to get a good stack trace.
          warning = getAsynchronousRejectionWarningObject(uid);
        }

        process.nextTick(function () {
          if (!process.emit('rejectionHandled', promise)) {
            if (warning === null) warning = getAsynchronousRejectionWarningObject(uid);
            warning.name = 'PromiseRejectionHandledWarning';
            warning.id = uid;
            process.emitWarning(warning);
          }
        });
      }
    }
  }

  function emitWarning(uid, reason) {
    try {
      if (reason instanceof Error) {
        process.emitWarning(reason.stack, 'UnhandledPromiseRejectionWarning');
      } else {
        process.emitWarning(safeToString(reason), 'UnhandledPromiseRejectionWarning');
      }
    } catch (e) {// ignored
    }

    var warning = new Error('Unhandled promise rejection. This error originated either by ' + 'throwing inside of an async function without a catch block, ' + 'or by rejecting a promise which was not handled with .catch(). ' + "(rejection id: ".concat(uid, ")"));
    warning.name = 'UnhandledPromiseRejectionWarning';

    try {
      if (reason instanceof Error) {
        warning.stack = reason.stack;
      }
    } catch (err) {// ignored
    }

    process.emitWarning(warning);

    if (!deprecationWarned) {
      deprecationWarned = true;
      process.emitWarning('Unhandled promise rejections are deprecated. In the future, ' + 'promise rejections that are not handled will terminate the ' + 'Node.js process with a non-zero exit code.', 'DeprecationWarning', 'DEP0018');
    }
  }

  function emitPendingUnhandledRejections() {
    var hadListeners = false;

    while (pendingUnhandledRejections.length > 0) {
      var promise = pendingUnhandledRejections.shift();
      var reason = pendingUnhandledRejections.shift();

      if (hasBeenNotifiedProperty.get(promise) === false) {
        hasBeenNotifiedProperty.set(promise, true);
        var uid = promiseToGuidProperty.get(promise);

        if (!process.emit('unhandledRejection', reason, promise)) {
          emitWarning(uid, reason);
        } else {
          hadListeners = true;
        }
      }
    }

    return hadListeners;
  }

  function addPendingUnhandledRejection(promise, reason) {
    pendingUnhandledRejections.push(promise, reason);
    scheduleMicrotasks();
  }

  return emitPendingUnhandledRejections;
}