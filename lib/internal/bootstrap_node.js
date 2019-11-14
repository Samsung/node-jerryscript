// Hello, and welcome to hacking node.js!
//
// This file is invoked by node::LoadEnvironment in src/node.cc, and is
// responsible for bootstrapping the node.js core. As special caution is given
// to the performance of the startup process, many dependencies are invoked
// lazily.
'use strict';

(function (process, // bootstrapper properties... destructured to
// avoid retaining a reference to the bootstrap
// object.
_ref) {

  // Polyfill methods.
  // TODO: implement them in JerryScript.
  if (!Object.getOwnPropertyDescriptors) {
    Object.defineProperty(Object, 'getOwnPropertyDescriptors', {
      value: function(O) {
        // 2. Let ownKeys be ? obj.[[OwnPropertyKeys]]().
        var ownKeys = Object.getOwnPropertyNames(O).concat(Object.getOwnPropertySymbols(O));
        // 3. Let descriptors be ! ObjectCreate(%ObjectPrototype%).
        var descriptors = {};
        // 4. For each element key of ownKeys in List order, do
        var length = ownKeys.length;
        for (var i = 0; i < length; i++) {
          var key = ownKeys[i];
          // a. Let desc be ? obj.[[GetOwnProperty]](key).
          // b. Let descriptor be ! FromPropertyDescriptor(desc).
          var descriptor = Object.getOwnPropertyDescriptor(O, key);
          // c. If descriptor is not undefined, perform ! CreateDataProperty(descriptors, key, descriptor).
          if (descriptor !== undefined) {
            descriptors[key] = descriptor;
          }
        }
        // 5. Return descriptors.
        return descriptors;
      }
    });
  }

  // https://tc39.github.io/ecma262/#sec-array.prototype.includes
  if (!Array.prototype.includes) {
    Object.defineProperty(Array.prototype, 'includes', {
      value: function(searchElement, fromIndex) {

        if (this == null) {
          throw new TypeError('"this" is null or not defined');
        }

        // 1. Let O be ? ToObject(this value).
        var o = Object(this);
        // 2. Let len be ? ToLength(? Get(O, "length")).
        var len = o.length >>> 0;
        // 3. If len is 0, return false.
        if (len === 0) {
          return false;
        }

        // 4. Let n be ? ToInteger(fromIndex).
        //    (If fromIndex is undefined, this step produces the value 0.)
        var n = fromIndex | 0;

        // 5. If n ≥ 0, then
        //  a. Let k be n.
        // 6. Else n < 0,
        //  a. Let k be len + n.
        //  b. If k < 0, let k be 0.
        var k = Math.max(n >= 0 ? n : len - Math.abs(n), 0);

        function sameValueZero(x, y) {
          return x === y || (typeof x === 'number' && typeof y === 'number' && isNaN(x) && isNaN(y));
        }

        // 7. Repeat, while k < len
        while (k < len) {
          // a. Let elementK be the result of ? Get(O, ! ToString(k)).
          // b. If SameValueZero(searchElement, elementK) is true, return true.
          if (sameValueZero(o[k], searchElement)) {
            return true;
          }
          // c. Increase k by 1.
          k++;
        }
        // 8. Return false
        return false;
      }
    });
  }

  if (!Error.captureStackTrace) {
    Object.defineProperty(Error, 'captureStackTrace', {
      value: function(targetObject, constructorOpt) {
        // TODO: implement this.
        targetObject.stack = "Not implemented yet."
      }
    });
  }

  var _setupProcessObject = _ref._setupProcessObject,
      _setupNextTick = _ref._setupNextTick,
      _setupPromises = _ref._setupPromises,
      _cpuUsage = _ref._cpuUsage,
      _hrtime = _ref._hrtime,
      _memoryUsage = _ref._memoryUsage,
      _rawDebug = _ref._rawDebug;

  function startup() {
    var EventEmitter = NativeModule.require('events');

    process._eventsCount = 0;
    var origProcProto = Object.getPrototypeOf(process);
    Object.setPrototypeOf(origProcProto, EventEmitter.prototype);
    EventEmitter.call(process);
    setupProcessObject(); // do this good and early, since it handles errors.

    setupProcessFatal();
    setupProcessICUVersions();
    setupGlobalVariables();

    var _process = NativeModule.require('internal/process');

    _process.setupConfig(NativeModule._source);

    _process.setupSignalHandlers();

    NativeModule.require('internal/process/warning').setup();

    NativeModule.require('internal/process/next_tick').setup(_setupNextTick, _setupPromises);

    NativeModule.require('internal/process/stdio').setup();

    var perf = process.binding('performance');
    var _perf$constants = perf.constants,
        NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE = _perf$constants.NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE,
        NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START = _perf$constants.NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START,
        NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END = _perf$constants.NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END,
        NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START = _perf$constants.NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START,
        NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END = _perf$constants.NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END,
        NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START = _perf$constants.NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START,
        NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END = _perf$constants.NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END,
        NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START = _perf$constants.NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START,
        NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END = _perf$constants.NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END;

    _process.setup_hrtime(_hrtime); //_process.setup_performance();


    _process.setup_cpuUsage(_cpuUsage);

    _process.setupMemoryUsage(_memoryUsage);

    _process.setupKillAndExit(); //if (global.__coverage__)
    //  NativeModule.require('internal/process/write-coverage').setup();
    //NativeModule.require('internal/trace_events_async_hooks').setup();
    //NativeModule.require('internal/inspector_async_hook').setup();


    _process.setupChannel();

    _process.setupRawDebug(_rawDebug);

    var browserGlobals = !process._noBrowserGlobals;

    if (browserGlobals) {
      setupGlobalTimeouts();
      setupGlobalConsole();
    } // Ensure setURLConstructor() is called before the native
    // URL::ToObject() method is used.


    NativeModule.require('internal/url'); // On OpenBSD process.execPath will be relative unless we
    // get the full path before process.execPath is used.


    if (process.platform === 'openbsd') {
      var _NativeModule$require = NativeModule.require('fs'),
          realpathSync = _NativeModule$require.realpathSync;

      process.execPath = realpathSync(process.execPath);
    }

    Object.defineProperty(process, 'argv0', {
      enumerable: true,
      configurable: false,
      value: process.argv[0]
    });
    process.argv[0] = process.execPath; // Handle `--debug*` deprecation and invalidation

    if (process._invalidDebug) {
      process.emitWarning('`node --debug` and `node --debug-brk` are invalid. ' + 'Please use `node --inspect` or `node --inspect-brk` instead.', 'DeprecationWarning', 'DEP0062', startup, true);
      process.exit(9);
    } else if (process._deprecatedDebugBrk) {
      process.emitWarning('`node --inspect --debug-brk` is deprecated. ' + 'Please use `node --inspect-brk` instead.', 'DeprecationWarning', 'DEP0062', startup, true);
    }

    if (process.binding('config').experimentalModules) {
      process.emitWarning('The ESM module loader is experimental.', 'ExperimentalWarning', undefined);
    } // There are various modes that Node can run in. The most common two
    // are running from a script and running the REPL - but there are a few
    // others like the debugger or running --eval arguments. Here we decide
    // which mode we run in.


    if (NativeModule.exists('_third_party_main')) {
      // To allow people to extend Node in different ways, this hook allows
      // one to drop a file lib/_third_party_main.js into the build
      // directory which will be executed instead of Node's normal loading.
      process.nextTick(function () {
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START);

        NativeModule.require('_third_party_main');

        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END);
      });
    } else if (process.argv[1] === 'inspect' || process.argv[1] === 'debug') {
      if (process.argv[1] === 'debug') {
        process.emitWarning('`node debug` is deprecated. Please use `node inspect` instead.', 'DeprecationWarning', 'DEP0068');
      } // Start the debugger agent


      process.nextTick(function () {
        NativeModule.require('node-inspect/lib/_inspect').start();
      });
    } else if (process.profProcess) {
      NativeModule.require('internal/v8_prof_processor');
    } else {
      // There is user code to be run
      // If this is a worker in cluster mode, start up the communication
      // channel. This needs to be done before any user code gets executed
      // (including preload modules).
      if (process.argv[1] && process.env.NODE_UNIQUE_ID) {
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START);

        var cluster = NativeModule.require('cluster');

        cluster._setupWorker();

        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END); // Make sure it's not accidentally inherited by child processes.

        delete process.env.NODE_UNIQUE_ID;
      }

      if (process._eval != null && !process._forceRepl) {
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START);
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END); // User passed '-e' or '--eval' arguments to Node without '-i' or
        // '--interactive'

        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START);
        preloadModules();
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END);

        var internalModule = NativeModule.require('internal/module');

        internalModule.addBuiltinLibsToObject(global);
        evalScript('[eval]');
      } else if (process.argv[1] && process.argv[1] !== '-') {
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START); // make process.argv[1] into a full path

        var path = NativeModule.require('path');

        process.argv[1] = path.resolve(process.argv[1]);

        var Module = NativeModule.require('module'); // check if user passed `-c` or `--check` arguments to Node.


        if (process._syntax_check_only != null) {
          var fs = NativeModule.require('fs'); // read the source


          var filename = Module._resolveFilename(process.argv[1]);

          var source = fs.readFileSync(filename, 'utf-8');
          checkScriptSyntax(source, filename);
          process.exit(0);
        }

        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END);
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START);
        preloadModules();
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END);
        Module.runMain();
      } else {
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START);
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END);
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START);
        preloadModules();
        perf.markMilestone(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END); // If -i or --interactive were passed, or stdin is a TTY.

        if (process._forceRepl || NativeModule.require('tty').isatty(0)) {
          // REPL
          var cliRepl = NativeModule.require('internal/repl');

          cliRepl.createInternalRepl(process.env, function (err, repl) {
            if (err) {
              throw err;
            }

            repl.on('exit', function () {
              if (repl._flushing) {
                repl.pause();
                return repl.once('flushHistory', function () {
                  process.exit();
                });
              }

              process.exit();
            });
          });

          if (process._eval != null) {
            // User passed '-e' or '--eval'
            evalScript('[eval]');
          }
        } else {
          // Read all of stdin - execute it.
          process.stdin.setEncoding('utf8');
          var code = '';
          process.stdin.on('data', function (d) {
            code += d;
          });
          process.stdin.on('end', function () {
            if (process._syntax_check_only != null) {
              checkScriptSyntax(code, '[stdin]');
            } else {
              process._eval = code;
              evalScript('[stdin]');
            }
          });
        }
      }
    }

    perf.markMilestone(NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE);
  }

  function setupProcessObject() {
    _setupProcessObject(pushValueToArray);

    function pushValueToArray() {
      for (var i = 0; i < arguments.length; i++) {
        this.push(arguments[i]);
      }
    }
  }

  function setupGlobalVariables() {
    Object.defineProperty(global, Symbol.toStringTag, {
      value: 'global',
      writable: false,
      enumerable: false,
      configurable: true
    });
    global.process = process;

    var util = NativeModule.require('util');

    function makeGetter(name) {
      return util.deprecate(function () {
        return this;
      }, "'".concat(name, "' is deprecated, use 'global'"), 'DEP0016');
    }

    function makeSetter(name) {
      return util.deprecate(function (value) {
        Object.defineProperty(this, name, {
          configurable: true,
          writable: true,
          enumerable: true,
          value: value
        });
      }, "'".concat(name, "' is deprecated, use 'global'"), 'DEP0016');
    }

    Object.defineProperties(global, {
      GLOBAL: {
        configurable: true,
        get: makeGetter('GLOBAL'),
        set: makeSetter('GLOBAL')
      },
      root: {
        configurable: true,
        get: makeGetter('root'),
        set: makeSetter('root')
      }
    });
    global.Buffer = NativeModule.require('buffer').Buffer;
    process.domain = null;
    process._exiting = false;
  }

  function setupGlobalTimeouts() {
    var timers = NativeModule.require('timers');

    global.clearImmediate = timers.clearImmediate;
    global.clearInterval = timers.clearInterval;
    global.clearTimeout = timers.clearTimeout;
    global.setImmediate = timers.setImmediate;
    global.setInterval = timers.setInterval;
    global.setTimeout = timers.setTimeout;
  }

  function setupGlobalConsole() {
    var originalConsole = global.console;

    var Module = NativeModule.require('module'); // Setup Node.js global.console


    var wrappedConsole = NativeModule.require('console');

    Object.defineProperty(global, 'console', {
      configurable: true,
      enumerable: true,
      get: function get() {
        return wrappedConsole;
      }
    });
    setupInspector(originalConsole, wrappedConsole, Module);
  }

  function setupInspector(originalConsole, wrappedConsole, Module) {
    if (!process.config.variables.v8_enable_inspector) {
      return;
    }

    var _process$binding = process.binding('inspector'),
        addCommandLineAPI = _process$binding.addCommandLineAPI,
        consoleCall = _process$binding.consoleCall; // Setup inspector command line API


    var _NativeModule$require2 = NativeModule.require('internal/module'),
        makeRequireFunction = _NativeModule$require2.makeRequireFunction;

    var path = NativeModule.require('path');

    var cwd = tryGetCwd(path);
    var consoleAPIModule = new Module('<inspector console>');
    consoleAPIModule.paths = Module._nodeModulePaths(cwd).concat(Module.globalPaths);
    addCommandLineAPI('require', makeRequireFunction(consoleAPIModule));
    var config = {};

    for (var _i = 0, _Object$keys = Object.keys(wrappedConsole); _i < _Object$keys.length; _i++) {
      var key = _Object$keys[_i];
      if (!originalConsole.hasOwnProperty(key)) continue; // If global console has the same method as inspector console,
      // then wrap these two methods into one. Native wrapper will preserve
      // the original stack.

      wrappedConsole[key] = consoleCall.bind(wrappedConsole, originalConsole[key], wrappedConsole[key], config);
    }

    for (var _i2 = 0, _Object$keys2 = Object.keys(originalConsole); _i2 < _Object$keys2.length; _i2++) {
      var _key = _Object$keys2[_i2];
      if (wrappedConsole.hasOwnProperty(_key)) continue;
      wrappedConsole[_key] = originalConsole[_key];
    }
  }

  function setupProcessFatal() {
    var async_wrap = process.binding('async_wrap'); // Arrays containing hook flags and ids for async_hook calls.

    var async_hook_fields = async_wrap.async_hook_fields,
        async_id_fields = async_wrap.async_id_fields; // Internal functions needed to manipulate the stack.

    var clearAsyncIdStack = async_wrap.clearAsyncIdStack;
    var _async_wrap$constants = async_wrap.constants,
        kAfter = _async_wrap$constants.kAfter,
        kExecutionAsyncId = _async_wrap$constants.kExecutionAsyncId,
        kDefaultTriggerAsyncId = _async_wrap$constants.kDefaultTriggerAsyncId,
        kStackLength = _async_wrap$constants.kStackLength;

    process._fatalException = function (er) {
      var caught; // It's possible that kDefaultTriggerAsyncId was set for a constructor
      // call that threw and was never cleared. So clear it now.

      async_id_fields[kDefaultTriggerAsyncId] = -1;
      if (process.domain && process.domain._errorHandler) caught = process.domain._errorHandler(er);
      if (!caught) caught = process.emit('uncaughtException', er); // If someone handled it, then great.  otherwise, die in C++ land
      // since that means that we'll exit the process, emit the 'exit' event

      if (!caught) {
        try {
          if (!process._exiting) {
            process._exiting = true;
            process.emit('exit', 1);
          }
        } catch (er) {// nothing to be done about it at this point.
        }
      } else {
        // If we handled an error, then make sure any ticks get processed
        NativeModule.require('timers').setImmediate(process._tickCallback); // Emit the after() hooks now that the exception has been handled.


        if (async_hook_fields[kAfter] > 0) {
          do {
            NativeModule.require('internal/async_hooks').emitAfter(async_id_fields[kExecutionAsyncId]);
          } while (async_hook_fields[kStackLength] > 0); // Or completely empty the id stack.

        } else {
          clearAsyncIdStack();
        }
      }

      return caught;
    };
  }

  function setupProcessICUVersions() {
    var icu = process.binding('config').hasIntl ? process.binding('icu') : undefined;
    if (!icu) return; // no Intl/ICU: nothing to add here.
    // With no argument, getVersion() returns a comma separated list
    // of possible types.

    var versionTypes = icu.getVersion().split(',');

    for (var n = 0; n < versionTypes.length; n++) {
      var name = versionTypes[n];
      var version = icu.getVersion(name);
      Object.defineProperty(process.versions, name, {
        writable: false,
        enumerable: true,
        value: version
      });
    }
  }

  function tryGetCwd(path) {
    try {
      return process.cwd();
    } catch (ex) {
      // getcwd(3) can fail if the current working directory has been deleted.
      // Fall back to the directory name of the (absolute) executable path.
      // It's not really correct but what are the alternatives?
      return path.dirname(process.execPath);
    }
  }

  function wrapForBreakOnFirstLine(source) {
    if (!process._breakFirstLine) return source;
    var fn = "function() {\n\n".concat(source, ";\n\n}");
    return "process.binding('inspector').callAndPauseOnStart(".concat(fn, ", {})");
  }

  function evalScript(name) {
    var Module = NativeModule.require('module');

    var path = NativeModule.require('path');

    var cwd = tryGetCwd(path);
    var module = new Module(name);
    module.filename = path.join(cwd, name);
    module.paths = Module._nodeModulePaths(cwd);
    var body = wrapForBreakOnFirstLine(process._eval);
    var script = "global.__filename = ".concat(JSON.stringify(name), ";\n") + 'global.exports = exports;\n' + 'global.module = module;\n' + 'global.__dirname = __dirname;\n' + 'global.require = require;\n' + 'return require("vm").runInThisContext(' + "".concat(JSON.stringify(body), ", { filename: ") + "".concat(JSON.stringify(name), ", displayErrors: true });\n");

    var result = module._compile(script, "".concat(name, "-wrapper"));

    if (process._print_eval) console.log(result); // Handle any nextTicks added in the first tick of the program.

    process._tickCallback();
  } // Load preload modules


  function preloadModules() {
    if (process._preload_modules) {
      NativeModule.require('module')._preloadModules(process._preload_modules);
    }
  }

  function checkScriptSyntax(source, filename) {
    var Module = NativeModule.require('module');

    var vm = NativeModule.require('vm');

    var internalModule = NativeModule.require('internal/module'); // remove Shebang


    source = internalModule.stripShebang(source); // remove BOM

    source = internalModule.stripBOM(source); // wrap it

    source = Module.wrap(source); // compile the script, this will throw if it fails

    new vm.Script(source, {
      displayErrors: true,
      filename: filename
    });
  } // Below you find a minimal module system, which is used to load the node
  // core modules found in lib/*.js. All core modules are compiled into the
  // node binary, so they can be loaded faster.
  var ContextifyScript = process.binding('contextify').ContextifyScript;


  function runInThisContext(code, options) {
    NativeModule.require("internal/regenerator");
    var script = new ContextifyScript(code, options);
    return script.runInThisContext();
    //return eval(code);
  }

  function NativeModule(id) {
    this.filename = "".concat(id, ".js");
    this.id = id;
    this.exports = {};
    this.loaded = false;
    this.loading = false;
  }

  NativeModule._source = process.binding('natives');
  NativeModule._cache = {};
  var config = process.binding('config');
  if (!config.exposeHTTP2) delete NativeModule._source.http2;

  NativeModule.require = function (id) {
    if (id === 'native_module') {
      return NativeModule;
    }

    var cached = NativeModule.getCached(id);

    if (cached && (cached.loaded || cached.loading)) {
      return cached.exports;
    }

    if (!NativeModule.exists(id)) {
      // Model the error off the internal/errors.js model, but
      // do not use that module given that it could actually be
      // the one causing the error if there's a bug in Node.js
      var err = new Error("No such built-in module: ".concat(id));
      err.code = 'ERR_UNKNOWN_BUILTIN_MODULE';
      err.name = 'Error [ERR_UNKNOWN_BUILTIN_MODULE]';
      throw err;
    }

    process.moduleLoadList.push("NativeModule ".concat(id));
    var nativeModule = new NativeModule(id);
    nativeModule.cache();
    nativeModule.compile();
    return nativeModule.exports;
  };

  NativeModule.getCached = function (id) {
    return NativeModule._cache[id];
  };

  NativeModule.exists = function (id) {
    return NativeModule._source.hasOwnProperty(id);
  };

  if (config.exposeInternals) {
    NativeModule.nonInternalExists = NativeModule.exists;

    NativeModule.isInternal = function (id) {
      return false;
    };
  } else {
    NativeModule.nonInternalExists = function (id) {
      return NativeModule.exists(id) && !NativeModule.isInternal(id);
    };

    NativeModule.isInternal = function (id) {
      return id.startsWith('internal/');
    };
  }

  NativeModule.getSource = function (id) {
    return NativeModule._source[id];
  };

  NativeModule.wrap = function (script) {
    return NativeModule.wrapper[0] + script + NativeModule.wrapper[1];
  };

  NativeModule.wrapper = ['(function (exports, require, module, __filename, __dirname) { ', '\n});'];

  NativeModule.prototype.compile = function () {
    var source = NativeModule.getSource(this.id);
    source = NativeModule.wrap(source);
    this.loading = true;

    try {
      var fn = runInThisContext(source, {
        filename: this.filename,
        lineOffset: 0,
        displayErrors: true
      });
      fn(this.exports, NativeModule.require, this, this.filename);
      this.loaded = true;
    } finally {
      this.loading = false;
    }
  };

  NativeModule.prototype.cache = function () {
    NativeModule._cache[this.id] = this;
  };

  startup();
});