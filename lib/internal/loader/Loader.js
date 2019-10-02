'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var path = require('path');

var _require = require('internal/url'),
    getURLFromFilePath = _require.getURLFromFilePath,
    URL = _require.URL;

var _require2 = require('internal/loader/ModuleWrap'),
    createDynamicModule = _require2.createDynamicModule,
    setImportModuleDynamicallyCallback = _require2.setImportModuleDynamicallyCallback;

var ModuleMap = require('internal/loader/ModuleMap');

var ModuleJob = require('internal/loader/ModuleJob');

var ModuleRequest = require('internal/loader/ModuleRequest');

var errors = require('internal/errors');

var debug = require('util').debuglog('esm'); // Returns a file URL for the current working directory.


function getURLStringForCwd() {
  try {
    return getURLFromFilePath("".concat(process.cwd(), "/")).href;
  } catch (e) {
    e.stack; // If the current working directory no longer exists.

    if (e.code === 'ENOENT') {
      return undefined;
    }

    throw e;
  }
}

function normalizeReferrerURL(referrer) {
  if (typeof referrer === 'string' && path.isAbsolute(referrer)) {
    return getURLFromFilePath(referrer).href;
  }

  return new URL(referrer).href;
}
/* A Loader instance is used as the main entry point for loading ES modules.
 * Currently, this is a singleton -- there is only one used for loading
 * the main module and everything in its dependency graph. */


var Loader =
/*#__PURE__*/
function () {
  function Loader() {
    var base = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : getURLStringForCwd();

    _classCallCheck(this, Loader);

    if (typeof base !== 'string') {
      throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'base', 'string');
    }

    this.moduleMap = new ModuleMap();
    this.base = base; // The resolver has the signature
    //   (specifier : string, parentURL : string, defaultResolve)
    //       -> Promise<{ url : string,
    //                    format: anything in Loader.validFormats }>
    // where defaultResolve is ModuleRequest.resolve (having the same
    // signature itself).
    // If `.format` on the returned value is 'dynamic', .dynamicInstantiate
    // will be used as described below.

    this.resolver = ModuleRequest.resolve; // This hook is only called when resolve(...).format is 'dynamic' and has
    // the signature
    //   (url : string) -> Promise<{ exports: { ... }, execute: function }>
    // Where `exports` is an object whose property names define the exported
    // names of the generated module. `execute` is a function that receives
    // an object with the same keys as `exports`, whose values are get/set
    // functions for the actual exported values.

    this.dynamicInstantiate = undefined;
  }

  _createClass(Loader, [{
    key: "hook",
    value: function hook(_ref) {
      var _ref$resolve = _ref.resolve,
          resolve = _ref$resolve === void 0 ? ModuleRequest.resolve : _ref$resolve,
          dynamicInstantiate = _ref.dynamicInstantiate;
      // Use .bind() to avoid giving access to the Loader instance when it is
      // called as this.resolver(...);
      this.resolver = resolve.bind(null);
      this.dynamicInstantiate = dynamicInstantiate;
    } // Typechecking wrapper around .resolver().

  }, {
    key: "resolve",
    value: function () {
      var _resolve = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee(specifier) {
        var parentURL,
            _ref2,
            url,
            format,
            _args = arguments;

        return regeneratorRuntime.wrap(function _callee$(_context) {
          while (1) {
            switch (_context.prev = _context.next) {
              case 0:
                parentURL = _args.length > 1 && _args[1] !== undefined ? _args[1] : this.base;

                if (!(typeof parentURL !== 'string')) {
                  _context.next = 3;
                  break;
                }

                throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'parentURL', 'string');

              case 3:
                _context.next = 5;
                return this.resolver(specifier, parentURL, ModuleRequest.resolve);

              case 5:
                _ref2 = _context.sent;
                url = _ref2.url;
                format = _ref2.format;

                if (Loader.validFormats.includes(format)) {
                  _context.next = 10;
                  break;
                }

                throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'format', Loader.validFormats);

              case 10:
                if (!(typeof url !== 'string')) {
                  _context.next = 12;
                  break;
                }

                throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'url', 'string');

              case 12:
                if (!(format === 'builtin')) {
                  _context.next = 14;
                  break;
                }

                return _context.abrupt("return", {
                  url: "node:".concat(url),
                  format: format
                });

              case 14:
                if (!(format !== 'dynamic')) {
                  _context.next = 19;
                  break;
                }

                if (ModuleRequest.loaders.has(format)) {
                  _context.next = 17;
                  break;
                }

                throw new errors.Error('ERR_UNKNOWN_MODULE_FORMAT', format);

              case 17:
                if (url.startsWith('file:')) {
                  _context.next = 19;
                  break;
                }

                throw new errors.Error('ERR_INVALID_PROTOCOL', url, 'file:');

              case 19:
                return _context.abrupt("return", {
                  url: url,
                  format: format
                });

              case 20:
              case "end":
                return _context.stop();
            }
          }
        }, _callee, this);
      }));

      function resolve(_x) {
        return _resolve.apply(this, arguments);
      }

      return resolve;
    }() // May create a new ModuleJob instance if one did not already exist.

  }, {
    key: "getModuleJob",
    value: function () {
      var _getModuleJob = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee3(specifier) {
        var parentURL,
            _ref3,
            url,
            format,
            job,
            loaderInstance,
            dynamicInstantiate,
            _args3 = arguments;

        return regeneratorRuntime.wrap(function _callee3$(_context3) {
          while (1) {
            switch (_context3.prev = _context3.next) {
              case 0:
                parentURL = _args3.length > 1 && _args3[1] !== undefined ? _args3[1] : this.base;
                _context3.next = 3;
                return this.resolve(specifier, parentURL);

              case 3:
                _ref3 = _context3.sent;
                url = _ref3.url;
                format = _ref3.format;
                job = this.moduleMap.get(url);

                if (!(job === undefined)) {
                  _context3.next = 18;
                  break;
                }

                if (!(format === 'dynamic')) {
                  _context3.next = 15;
                  break;
                }

                dynamicInstantiate = this.dynamicInstantiate;

                if (!(typeof dynamicInstantiate !== 'function')) {
                  _context3.next = 12;
                  break;
                }

                throw new errors.Error('ERR_MISSING_DYNAMIC_INSTANTIATE_HOOK');

              case 12:
                loaderInstance =
                /*#__PURE__*/
                function () {
                  var _ref4 = _asyncToGenerator(
                  /*#__PURE__*/
                  regeneratorRuntime.mark(function _callee2(url) {
                    var _ref5, exports, execute;

                    return regeneratorRuntime.wrap(function _callee2$(_context2) {
                      while (1) {
                        switch (_context2.prev = _context2.next) {
                          case 0:
                            _context2.next = 2;
                            return dynamicInstantiate(url);

                          case 2:
                            _ref5 = _context2.sent;
                            exports = _ref5.exports;
                            execute = _ref5.execute;
                            return _context2.abrupt("return", createDynamicModule(exports, url, function (reflect) {
                              debug("Loading custom loader ".concat(url));
                              execute(reflect.exports);
                            }));

                          case 6:
                          case "end":
                            return _context2.stop();
                        }
                      }
                    }, _callee2);
                  }));

                  return function loaderInstance(_x3) {
                    return _ref4.apply(this, arguments);
                  };
                }();

                _context3.next = 16;
                break;

              case 15:
                loaderInstance = ModuleRequest.loaders.get(format);

              case 16:
                job = new ModuleJob(this, url, loaderInstance);
                this.moduleMap.set(url, job);

              case 18:
                return _context3.abrupt("return", job);

              case 19:
              case "end":
                return _context3.stop();
            }
          }
        }, _callee3, this);
      }));

      function getModuleJob(_x2) {
        return _getModuleJob.apply(this, arguments);
      }

      return getModuleJob;
    }()
  }, {
    key: "import",
    value: function () {
      var _import2 = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee4(specifier) {
        var parentURL,
            job,
            module,
            _args4 = arguments;
        return regeneratorRuntime.wrap(function _callee4$(_context4) {
          while (1) {
            switch (_context4.prev = _context4.next) {
              case 0:
                parentURL = _args4.length > 1 && _args4[1] !== undefined ? _args4[1] : this.base;
                _context4.next = 3;
                return this.getModuleJob(specifier, parentURL);

              case 3:
                job = _context4.sent;
                _context4.next = 6;
                return job.run();

              case 6:
                module = _context4.sent;
                return _context4.abrupt("return", module.namespace());

              case 8:
              case "end":
                return _context4.stop();
            }
          }
        }, _callee4, this);
      }));

      function _import(_x4) {
        return _import2.apply(this, arguments);
      }

      return _import;
    }()
  }], [{
    key: "registerImportDynamicallyCallback",
    value: function registerImportDynamicallyCallback(loader) {
      setImportModuleDynamicallyCallback(
      /*#__PURE__*/
      function () {
        var _ref6 = _asyncToGenerator(
        /*#__PURE__*/
        regeneratorRuntime.mark(function _callee5(referrer, specifier) {
          return regeneratorRuntime.wrap(function _callee5$(_context5) {
            while (1) {
              switch (_context5.prev = _context5.next) {
                case 0:
                  return _context5.abrupt("return", loader["import"](specifier, normalizeReferrerURL(referrer)));

                case 1:
                case "end":
                  return _context5.stop();
              }
            }
          }, _callee5);
        }));

        return function (_x5, _x6) {
          return _ref6.apply(this, arguments);
        };
      }());
    }
  }]);

  return Loader;
}();

Loader.validFormats = ['esm', 'cjs', 'builtin', 'addon', 'json', 'dynamic'];
Object.setPrototypeOf(Loader.prototype, null);
module.exports = Loader;