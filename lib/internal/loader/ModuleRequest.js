'use strict';

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

var fs = require('fs');

var internalCJSModule = require('internal/module');

var CJSModule = require('module');

var internalURLModule = require('internal/url');

var internalFS = require('internal/fs');

var NativeModule = require('native_module');

var _require = require('path'),
    extname = _require.extname,
    _makeLong = _require._makeLong;

var _require2 = require('url'),
    URL = _require2.URL;

var _require3 = require('fs'),
    realpathSync = _require3.realpathSync;

var preserveSymlinks = !!process.binding('config').preserveSymlinks;

var _require4 = require('internal/loader/ModuleWrap'),
    ModuleWrap = _require4.ModuleWrap,
    createDynamicModule = _require4.createDynamicModule;

var errors = require('internal/errors');

var search = require('internal/loader/search');

var asyncReadFile = require('util').promisify(require('fs').readFile);

var debug = require('util').debuglog('esm');

var realpathCache = new Map();
var loaders = new Map();
exports.loaders = loaders; // Strategy for loading a standard JavaScript module

loaders.set('esm',
/*#__PURE__*/
function () {
  var _ref = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee(url) {
    var source;
    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            _context.t0 = "";
            _context.next = 3;
            return asyncReadFile(new URL(url));

          case 3:
            _context.t1 = _context.sent;
            source = _context.t0.concat.call(_context.t0, _context.t1);
            debug("Loading StandardModule ".concat(url));
            return _context.abrupt("return", {
              module: new ModuleWrap(internalCJSModule.stripShebang(source), url),
              reflect: undefined
            });

          case 7:
          case "end":
            return _context.stop();
        }
      }
    }, _callee);
  }));

  return function (_x) {
    return _ref.apply(this, arguments);
  };
}()); // Strategy for loading a node-style CommonJS module

var isWindows = process.platform === 'win32';
var winSepRegEx = /\//g;
loaders.set('cjs',
/*#__PURE__*/
function () {
  var _ref2 = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee2(url) {
    var pathname, module, ctx;
    return regeneratorRuntime.wrap(function _callee2$(_context2) {
      while (1) {
        switch (_context2.prev = _context2.next) {
          case 0:
            pathname = internalURLModule.getPathFromURL(new URL(url));
            module = CJSModule._cache[isWindows ? pathname.replace(winSepRegEx, '\\') : pathname];

            if (!(module && module.loaded)) {
              _context2.next = 6;
              break;
            }

            ctx = createDynamicModule(['default'], url, undefined);
            ctx.reflect.exports["default"].set(module.exports);
            return _context2.abrupt("return", ctx);

          case 6:
            return _context2.abrupt("return", createDynamicModule(['default'], url, function (reflect) {
              debug("Loading CJSModule ".concat(url));

              CJSModule._load(pathname);
            }));

          case 7:
          case "end":
            return _context2.stop();
        }
      }
    }, _callee2);
  }));

  return function (_x2) {
    return _ref2.apply(this, arguments);
  };
}()); // Strategy for loading a node builtin CommonJS module that isn't
// through normal resolution

loaders.set('builtin',
/*#__PURE__*/
function () {
  var _ref3 = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee3(url) {
    return regeneratorRuntime.wrap(function _callee3$(_context3) {
      while (1) {
        switch (_context3.prev = _context3.next) {
          case 0:
            return _context3.abrupt("return", createDynamicModule(['default'], url, function (reflect) {
              debug("Loading BuiltinModule ".concat(url));

              var exports = NativeModule.require(url.substr(5));

              reflect.exports["default"].set(exports);
            }));

          case 1:
          case "end":
            return _context3.stop();
        }
      }
    }, _callee3);
  }));

  return function (_x3) {
    return _ref3.apply(this, arguments);
  };
}());
loaders.set('addon',
/*#__PURE__*/
function () {
  var _ref4 = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee4(url) {
    var ctx;
    return regeneratorRuntime.wrap(function _callee4$(_context4) {
      while (1) {
        switch (_context4.prev = _context4.next) {
          case 0:
            ctx = createDynamicModule(['default'], url, function (reflect) {
              debug("Loading NativeModule ".concat(url));
              var module = {
                exports: {}
              };
              var pathname = internalURLModule.getPathFromURL(new URL(url));
              process.dlopen(module, _makeLong(pathname));
              reflect.exports["default"].set(module.exports);
            });
            return _context4.abrupt("return", ctx);

          case 2:
          case "end":
            return _context4.stop();
        }
      }
    }, _callee4);
  }));

  return function (_x4) {
    return _ref4.apply(this, arguments);
  };
}());
loaders.set('json',
/*#__PURE__*/
function () {
  var _ref5 = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee5(url) {
    return regeneratorRuntime.wrap(function _callee5$(_context5) {
      while (1) {
        switch (_context5.prev = _context5.next) {
          case 0:
            return _context5.abrupt("return", createDynamicModule(['default'], url, function (reflect) {
              debug("Loading JSONModule ".concat(url));
              var pathname = internalURLModule.getPathFromURL(new URL(url));
              var content = fs.readFileSync(pathname, 'utf8');

              try {
                var _exports = JSON.parse(internalCJSModule.stripBOM(content));

                reflect.exports["default"].set(_exports);
              } catch (err) {
                err.message = pathname + ': ' + err.message;
                throw err;
              }
            }));

          case 1:
          case "end":
            return _context5.stop();
        }
      }
    }, _callee5);
  }));

  return function (_x5) {
    return _ref5.apply(this, arguments);
  };
}());

exports.resolve = function (specifier, parentURL) {
  if (NativeModule.nonInternalExists(specifier)) {
    return {
      url: specifier,
      format: 'builtin'
    };
  }

  var url;

  try {
    url = search(specifier, parentURL);
  } catch (e) {
    if (e.message && e.message.startsWith('Cannot find module')) e.code = 'MODULE_NOT_FOUND';
    throw e;
  }

  if (url.protocol !== 'file:') {
    throw new errors.Error('ERR_INVALID_PROTOCOL', url.protocol, 'file:');
  }

  if (!preserveSymlinks) {
    var real = realpathSync(internalURLModule.getPathFromURL(url), _defineProperty({}, internalFS.realpathCacheKey, realpathCache));
    var old = url;
    url = internalURLModule.getURLFromFilePath(real);
    url.search = old.search;
    url.hash = old.hash;
  }

  var ext = extname(url.pathname);

  switch (ext) {
    case '.mjs':
      return {
        url: "".concat(url),
        format: 'esm'
      };

    case '.json':
      return {
        url: "".concat(url),
        format: 'json'
      };

    case '.node':
      return {
        url: "".concat(url),
        format: 'addon'
      };

    case '.js':
      return {
        url: "".concat(url),
        format: 'cjs'
      };

    default:
      throw new errors.Error('ERR_UNKNOWN_FILE_EXTENSION', internalURLModule.getPathFromURL(url));
  }
};