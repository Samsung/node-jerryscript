'use strict';

var errors = require('internal/errors');

var util = require('util');

var constants = process.binding('constants').os.signals;
var internalBinding = process._internalBinding;
delete process._internalBinding;

var assert = process.assert = function (x, msg) {
  if (!x) throw new Error(msg || 'assertion error');
};

function setup_performance() {
  require('perf_hooks');
} // Set up the process.cpuUsage() function.


function setup_cpuUsage(_cpuUsage) {
  // Create the argument array that will be passed to the native function.
  var cpuValues = new Float64Array(2); // Replace the native function with the JS version that calls the native
  // function.

  process.cpuUsage = function cpuUsage(prevValue) {
    // If a previous value was passed in, ensure it has the correct shape.
    if (prevValue) {
      if (!previousValueIsValid(prevValue.user)) {
        throw new TypeError('value of user property of argument is invalid');
      }

      if (!previousValueIsValid(prevValue.system)) {
        throw new TypeError('value of system property of argument is invalid');
      }
    } // Call the native function to get the current values.


    var errmsg = _cpuUsage(cpuValues);

    if (errmsg) {
      throw new Error('unable to obtain CPU usage: ' + errmsg);
    } // If a previous value was passed in, return diff of current from previous.


    if (prevValue) {
      return {
        user: cpuValues[0] - prevValue.user,
        system: cpuValues[1] - prevValue.system
      };
    } // If no previous value passed in, return current value.


    return {
      user: cpuValues[0],
      system: cpuValues[1]
    }; // Ensure that a previously passed in value is valid. Currently, the native
    // implementation always returns numbers <= Number.MAX_SAFE_INTEGER.

    function previousValueIsValid(num) {
      return Number.isFinite(num) && num <= Number.MAX_SAFE_INTEGER && num >= 0;
    }
  };
} // The 3 entries filled in by the original process.hrtime contains
// the upper/lower 32 bits of the second part of the value,
// and the remaining nanoseconds of the value.


function setup_hrtime(_hrtime) {
  var hrValues = new Uint32Array(3);

  process.hrtime = function hrtime(time) {
    _hrtime(hrValues);

    if (time !== undefined) {
      if (Array.isArray(time) && time.length === 2) {
        var sec = hrValues[0] * 0x100000000 + hrValues[1] - time[0];
        var nsec = hrValues[2] - time[1];
        var needsBorrow = nsec < 0;
        return [needsBorrow ? sec - 1 : sec, needsBorrow ? nsec + 1e9 : nsec];
      }

      throw new TypeError('process.hrtime() only accepts an Array tuple');
    }

    return [hrValues[0] * 0x100000000 + hrValues[1], hrValues[2]];
  };
}

function setupMemoryUsage(_memoryUsage) {
  var memValues = new Float64Array(4);

  process.memoryUsage = function memoryUsage() {
    _memoryUsage(memValues);

    return {
      rss: memValues[0],
      heapTotal: memValues[1],
      heapUsed: memValues[2],
      external: memValues[3]
    };
  };
}

function setupConfig(_source) {
  // NativeModule._source
  // used for `process.config`, but not a real module
  var config = _source.config;
  delete _source.config;
  process.config = JSON.parse(config, function (key, value) {
    if (value === 'true') return true;
    if (value === 'false') return false;
    return value;
  });
  var processConfig = process.binding('config');

  if (typeof Intl !== 'undefined' && Intl.hasOwnProperty('v8BreakIterator')) {
    var oldV8BreakIterator = Intl.v8BreakIterator;
    var des = Object.getOwnPropertyDescriptor(Intl, 'v8BreakIterator');
    des.value = require('internal/util').deprecate(function v8BreakIterator() {
      if (processConfig.hasSmallICU && !processConfig.icuDataDir) {
        // Intl.v8BreakIterator() would crash w/ fatal error, so throw instead.
        throw new Error('v8BreakIterator: full ICU data not installed. ' + 'See https://github.com/nodejs/node/wiki/Intl');
      }

      return Reflect.construct(oldV8BreakIterator, arguments);
    }, 'Intl.v8BreakIterator is deprecated and will be removed soon.', 'DEP0017');
    Object.defineProperty(Intl, 'v8BreakIterator', des);
  }
}

function setupKillAndExit() {
  process.exit = function (code) {
    if (code || code === 0) process.exitCode = code;

    if (!process._exiting) {
      process._exiting = true;
      process.emit('exit', process.exitCode || 0);
    }

    process.reallyExit(process.exitCode || 0);
  };

  process.kill = function (pid, sig) {
    var err; // eslint-disable-next-line eqeqeq

    if (pid != (pid | 0)) {
      throw new TypeError('invalid pid');
    } // preserve null signal


    if (0 === sig) {
      err = process._kill(pid, 0);
    } else {
      sig = sig || 'SIGTERM';

      if (constants[sig]) {
        err = process._kill(pid, constants[sig]);
      } else {
        throw new Error("Unknown signal: ".concat(sig));
      }
    }

    if (err) throw errors.errnoException(err, 'kill');
    return true;
  };
}

function setupSignalHandlers() {
  // Load events module in order to access prototype elements on process like
  // process.addListener.
  var signalWraps = {};

  function isSignal(event) {
    return typeof event === 'string' && constants[event] !== undefined;
  } // Detect presence of a listener for the special signal types


  process.on('newListener', function (type, listener) {
    if (isSignal(type) && !signalWraps.hasOwnProperty(type)) {
      var Signal = process.binding('signal_wrap').Signal;
      var wrap = new Signal();
      wrap.unref();

      wrap.onsignal = function () {
        process.emit(type, type);
      };

      var signum = constants[type];
      var err = wrap.start(signum);

      if (err) {
        wrap.close();
        throw errors.errnoException(err, 'uv_signal_start');
      }

      signalWraps[type] = wrap;
    }
  });
  process.on('removeListener', function (type, listener) {
    if (signalWraps.hasOwnProperty(type) && this.listenerCount(type) === 0) {
      signalWraps[type].close();
      delete signalWraps[type];
    }
  });
}

function setupChannel() {
  // If we were spawned with env NODE_CHANNEL_FD then load that up and
  // start parsing data from that stream.
  if (process.env.NODE_CHANNEL_FD) {
    var fd = parseInt(process.env.NODE_CHANNEL_FD, 10);
    assert(fd >= 0); // Make sure it's not accidentally inherited by child processes.

    delete process.env.NODE_CHANNEL_FD;

    var cp = require('child_process'); // Load tcp_wrap to avoid situation where we might immediately receive
    // a message.
    // FIXME is this really necessary?


    process.binding('tcp_wrap');

    cp._forkChild(fd);

    assert(process.send);
  }
}

function setupRawDebug(_rawDebug) {
  process._rawDebug = function () {
    _rawDebug(util.format.apply(null, arguments));
  };
}

module.exports = {
  setup_performance: setup_performance,
  setup_cpuUsage: setup_cpuUsage,
  setup_hrtime: setup_hrtime,
  setupMemoryUsage: setupMemoryUsage,
  setupConfig: setupConfig,
  setupKillAndExit: setupKillAndExit,
  setupSignalHandlers: setupSignalHandlers,
  setupChannel: setupChannel,
  setupRawDebug: setupRawDebug,
  internalBinding: internalBinding
};