'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

var _require$internalBind = require('internal/process').internalBinding('module_wrap'),
    ModuleWrap = _require$internalBind.ModuleWrap,
    setImportModuleDynamicallyCallback = _require$internalBind.setImportModuleDynamicallyCallback;

var debug = require('util').debuglog('esm');

var ArrayJoin = Function.call.bind(Array.prototype.join);
var ArrayMap = Function.call.bind(Array.prototype.map);

var createDynamicModule = function createDynamicModule(exports) {
  var url = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : '';
  var evaluate = arguments.length > 2 ? arguments[2] : undefined;
  debug("creating ESM facade for ".concat(url, " with exports: ").concat(ArrayJoin(exports, ', ')));
  var names = ArrayMap(exports, function (name) {
    return "".concat(name);
  }); // Create two modules: One whose exports are get- and set-able ('reflective'),
  // and one which re-exports all of these but additionally may
  // run an executor function once everything is set up.

  var src = "\n  export let executor;\n  ".concat(ArrayJoin(ArrayMap(names, function (name) {
    return "export let $".concat(name, ";");
  }), '\n'), "\n  /* This function is implicitly returned as the module's completion value */\n  (() => ({\n    setExecutor: fn => executor = fn,\n    reflect: {\n      exports: { ").concat(ArrayJoin(ArrayMap(names, function (name) {
    return "\n        ".concat(name, ": {\n          get: () => $").concat(name, ",\n          set: v => $").concat(name, " = v\n        }");
  }), ', \n'), "\n      }\n    }\n  }));");
  var reflectiveModule = new ModuleWrap(src, "cjs-facade:".concat(url));
  reflectiveModule.instantiate();

  var _reflectiveModule$eva = reflectiveModule.evaluate()(),
      setExecutor = _reflectiveModule$eva.setExecutor,
      reflect = _reflectiveModule$eva.reflect; // public exposed ESM


  var reexports = "\n  import {\n    executor,\n    ".concat(ArrayMap(names, function (name) {
    return "$".concat(name);
  }), "\n  } from \"\";\n  export {\n    ").concat(ArrayJoin(ArrayMap(names, function (name) {
    return "$".concat(name, " as ").concat(name);
  }), ', '), "\n  }\n  if (typeof executor === \"function\") {\n    // add await to this later if top level await comes along\n    executor()\n  }");

  if (typeof evaluate === 'function') {
    setExecutor(function () {
      return evaluate(reflect);
    });
  }

  var module = new ModuleWrap(reexports, "".concat(url));
  module.link(
  /*#__PURE__*/
  _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee() {
    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            return _context.abrupt("return", reflectiveModule);

          case 1:
          case "end":
            return _context.stop();
        }
      }
    }, _callee);
  })));
  module.instantiate();
  return {
    module: module,
    reflect: reflect
  };
};

module.exports = {
  createDynamicModule: createDynamicModule,
  setImportModuleDynamicallyCallback: setImportModuleDynamicallyCallback,
  ModuleWrap: ModuleWrap
};