'use strict';

var _require = require('url'),
    URL = _require.URL;

var CJSmodule = require('module');

var errors = require('internal/errors');

var _require$internalBind = require('internal/process').internalBinding('module_wrap'),
    resolve = _require$internalBind.resolve;

module.exports = function (target, base) {
  if (base === undefined) {
    // We cannot search without a base.
    throw new errors.Error('ERR_MISSING_MODULE', target);
  }

  try {
    return resolve(target, base);
  } catch (e) {
    e.stack; // cause V8 to generate stack before rethrow

    var error = e;

    try {
      var questionedBase = new URL(base);
      var tmpMod = new CJSmodule(questionedBase.pathname, null);
      tmpMod.paths = CJSmodule._nodeModulePaths(new URL('./', questionedBase).pathname);

      var found = CJSmodule._resolveFilename(target, tmpMod);

      error = new errors.Error('ERR_MODULE_RESOLUTION_LEGACY', target, base, found);
    } catch (problemChecking) {// ignore
    }

    throw error;
  }
};