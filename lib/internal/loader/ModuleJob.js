'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var _require$internalBind = require('internal/process').internalBinding('module_wrap'),
    ModuleWrap = _require$internalBind.ModuleWrap;

var _require = require('internal/safe_globals'),
    SafeSet = _require.SafeSet,
    SafePromise = _require.SafePromise;

var assert = require('assert');

var resolvedPromise = SafePromise.resolve();
var enableDebug = (process.env.NODE_DEBUG || '').match(/\besm\b/) || process.features.debug;
/* A ModuleJob tracks the loading of a single Module, and the ModuleJobs of
 * its dependencies, over time. */

var ModuleJob =
/*#__PURE__*/
function () {
  // `loader` is the Loader instance used for loading dependencies.
  // `moduleProvider` is a function
  function ModuleJob(loader, url, moduleProvider) {
    var _this = this;

    _classCallCheck(this, ModuleJob);

    this.loader = loader;
    this.error = null;
    this.hadError = false; // This is a Promise<{ module, reflect }>, whose fields will be copied
    // onto `this` by `link()` below once it has been resolved.

    this.modulePromise = moduleProvider(url);
    this.module = undefined;
    this.reflect = undefined; // Wait for the ModuleWrap instance being linked with all dependencies.

    var link =
    /*#__PURE__*/
    function () {
      var _ref = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee2() {
        var dependencyJobs, _ref2;

        return regeneratorRuntime.wrap(function _callee2$(_context2) {
          while (1) {
            switch (_context2.prev = _context2.next) {
              case 0:
                dependencyJobs = [];
                _context2.next = 3;
                return _this.modulePromise;

              case 3:
                _ref2 = _context2.sent;
                _this.module = _ref2.module;
                _this.reflect = _ref2.reflect;
                assert(_this.module instanceof ModuleWrap);

                _this.module.link(
                /*#__PURE__*/
                function () {
                  var _ref3 = _asyncToGenerator(
                  /*#__PURE__*/
                  regeneratorRuntime.mark(function _callee(dependencySpecifier) {
                    var dependencyJobPromise, dependencyJob;
                    return regeneratorRuntime.wrap(function _callee$(_context) {
                      while (1) {
                        switch (_context.prev = _context.next) {
                          case 0:
                            dependencyJobPromise = _this.loader.getModuleJob(dependencySpecifier, url);
                            dependencyJobs.push(dependencyJobPromise);
                            _context.next = 4;
                            return dependencyJobPromise;

                          case 4:
                            dependencyJob = _context.sent;
                            _context.next = 7;
                            return dependencyJob.modulePromise;

                          case 7:
                            return _context.abrupt("return", _context.sent.module);

                          case 8:
                          case "end":
                            return _context.stop();
                        }
                      }
                    }, _callee);
                  }));

                  return function (_x) {
                    return _ref3.apply(this, arguments);
                  };
                }());

                if (enableDebug) {
                  // Make sure all dependencies are entered into the list synchronously.
                  Object.freeze(dependencyJobs);
                }

                return _context2.abrupt("return", SafePromise.all(dependencyJobs));

              case 10:
              case "end":
                return _context2.stop();
            }
          }
        }, _callee2);
      }));

      return function link() {
        return _ref.apply(this, arguments);
      };
    }(); // Promise for the list of all dependencyJobs.


    this.linked = link(); // instantiated == deep dependency jobs wrappers instantiated,
    // module wrapper instantiated

    this.instantiated = undefined;
  }

  _createClass(ModuleJob, [{
    key: "instantiate",
    value: function () {
      var _instantiate2 = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee3() {
        return regeneratorRuntime.wrap(function _callee3$(_context3) {
          while (1) {
            switch (_context3.prev = _context3.next) {
              case 0:
                if (!this.instantiated) {
                  _context3.next = 2;
                  break;
                }

                return _context3.abrupt("return", this.instantiated);

              case 2:
                return _context3.abrupt("return", this.instantiated = this._instantiate());

              case 3:
              case "end":
                return _context3.stop();
            }
          }
        }, _callee3, this);
      }));

      function instantiate() {
        return _instantiate2.apply(this, arguments);
      }

      return instantiate;
    }() // This method instantiates the module associated with this job and its
    // entire dependency graph, i.e. creates all the module namespaces and the
    // exported/imported variables.

  }, {
    key: "_instantiate",
    value: function () {
      var _instantiate3 = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee5() {
        var jobsInGraph, addJobsToDependencyGraph, _iteratorNormalCompletion, _didIteratorError, _iteratorError, _iterator, _step, dependencyJob;

        return regeneratorRuntime.wrap(function _callee5$(_context5) {
          while (1) {
            switch (_context5.prev = _context5.next) {
              case 0:
                jobsInGraph = new SafeSet();

                addJobsToDependencyGraph =
                /*#__PURE__*/
                function () {
                  var _ref4 = _asyncToGenerator(
                  /*#__PURE__*/
                  regeneratorRuntime.mark(function _callee4(moduleJob) {
                    var dependencyJobs;
                    return regeneratorRuntime.wrap(function _callee4$(_context4) {
                      while (1) {
                        switch (_context4.prev = _context4.next) {
                          case 0:
                            if (!jobsInGraph.has(moduleJob)) {
                              _context4.next = 2;
                              break;
                            }

                            return _context4.abrupt("return");

                          case 2:
                            jobsInGraph.add(moduleJob);
                            _context4.next = 5;
                            return moduleJob.linked;

                          case 5:
                            dependencyJobs = _context4.sent;
                            return _context4.abrupt("return", Promise.all(dependencyJobs.map(addJobsToDependencyGraph)));

                          case 7:
                          case "end":
                            return _context4.stop();
                        }
                      }
                    }, _callee4);
                  }));

                  return function addJobsToDependencyGraph(_x2) {
                    return _ref4.apply(this, arguments);
                  };
                }();

                _context5.prev = 2;
                _context5.next = 5;
                return addJobsToDependencyGraph(this);

              case 5:
                _context5.next = 11;
                break;

              case 7:
                _context5.prev = 7;
                _context5.t0 = _context5["catch"](2);

                if (!this.hadError) {
                  this.error = _context5.t0;
                  this.hadError = true;
                }

                throw _context5.t0;

              case 11:
                this.module.instantiate();
                _iteratorNormalCompletion = true;
                _didIteratorError = false;
                _iteratorError = undefined;
                _context5.prev = 15;

                for (_iterator = jobsInGraph[Symbol.iterator](); !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
                  dependencyJob = _step.value;
                  // Calling `this.module.instantiate()` instantiates not only the
                  // ModuleWrap in this module, but all modules in the graph.
                  dependencyJob.instantiated = resolvedPromise;
                }

                _context5.next = 23;
                break;

              case 19:
                _context5.prev = 19;
                _context5.t1 = _context5["catch"](15);
                _didIteratorError = true;
                _iteratorError = _context5.t1;

              case 23:
                _context5.prev = 23;
                _context5.prev = 24;

                if (!_iteratorNormalCompletion && _iterator["return"] != null) {
                  _iterator["return"]();
                }

              case 26:
                _context5.prev = 26;

                if (!_didIteratorError) {
                  _context5.next = 29;
                  break;
                }

                throw _iteratorError;

              case 29:
                return _context5.finish(26);

              case 30:
                return _context5.finish(23);

              case 31:
                return _context5.abrupt("return", this.module);

              case 32:
              case "end":
                return _context5.stop();
            }
          }
        }, _callee5, this, [[2, 7], [15, 19, 23, 31], [24,, 26, 30]]);
      }));

      function _instantiate() {
        return _instantiate3.apply(this, arguments);
      }

      return _instantiate;
    }()
  }, {
    key: "run",
    value: function () {
      var _run = _asyncToGenerator(
      /*#__PURE__*/
      regeneratorRuntime.mark(function _callee6() {
        var module;
        return regeneratorRuntime.wrap(function _callee6$(_context6) {
          while (1) {
            switch (_context6.prev = _context6.next) {
              case 0:
                _context6.next = 2;
                return this.instantiate();

              case 2:
                module = _context6.sent;
                _context6.prev = 3;
                module.evaluate();
                _context6.next = 13;
                break;

              case 7:
                _context6.prev = 7;
                _context6.t0 = _context6["catch"](3);
                _context6.t0.stack;
                this.hadError = true;
                this.error = _context6.t0;
                throw _context6.t0;

              case 13:
                return _context6.abrupt("return", module);

              case 14:
              case "end":
                return _context6.stop();
            }
          }
        }, _callee6, this, [[3, 7]]);
      }));

      function run() {
        return _run.apply(this, arguments);
      }

      return run;
    }()
  }]);

  return ModuleJob;
}();

Object.setPrototypeOf(ModuleJob.prototype, null);
module.exports = ModuleJob;