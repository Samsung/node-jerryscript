'use strict';

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var errors = require('internal/errors');

var internalUtil = require('internal/util');

var async_wrap = process.binding('async_wrap');

var internal_async_hooks = require('internal/async_hooks'); // Get functions
// Only used to support a deprecated API. pushAsyncIds, popAsyncIds should
// never be directly in this manner.


var pushAsyncIds = async_wrap.pushAsyncIds,
    popAsyncIds = async_wrap.popAsyncIds; // For userland AsyncResources, make sure to emit a destroy event when the
// resource gets gced.

var registerDestroyHook = async_wrap.registerDestroyHook;
var getHookArrays = internal_async_hooks.getHookArrays,
    enableHooks = internal_async_hooks.enableHooks,
    disableHooks = internal_async_hooks.disableHooks,
    newUid = internal_async_hooks.newUid,
    getDefaultTriggerAsyncId = internal_async_hooks.getDefaultTriggerAsyncId,
    emitInit = internal_async_hooks.emitInit,
    _emitBefore = internal_async_hooks.emitBefore,
    _emitAfter = internal_async_hooks.emitAfter,
    _emitDestroy = internal_async_hooks.emitDestroy; // Get fields

var async_id_fields = async_wrap.async_id_fields; // Get symbols

var _internal_async_hooks = internal_async_hooks.symbols,
    init_symbol = _internal_async_hooks.init_symbol,
    before_symbol = _internal_async_hooks.before_symbol,
    after_symbol = _internal_async_hooks.after_symbol,
    destroy_symbol = _internal_async_hooks.destroy_symbol,
    promise_resolve_symbol = _internal_async_hooks.promise_resolve_symbol;
var async_id_symbol = async_wrap.async_id_symbol,
    trigger_async_id_symbol = async_wrap.trigger_async_id_symbol; // Get constants

var _async_wrap$constants = async_wrap.constants,
    kInit = _async_wrap$constants.kInit,
    kBefore = _async_wrap$constants.kBefore,
    kAfter = _async_wrap$constants.kAfter,
    kDestroy = _async_wrap$constants.kDestroy,
    kTotals = _async_wrap$constants.kTotals,
    kPromiseResolve = _async_wrap$constants.kPromiseResolve,
    kExecutionAsyncId = _async_wrap$constants.kExecutionAsyncId,
    kTriggerAsyncId = _async_wrap$constants.kTriggerAsyncId; // Listener API //

var AsyncHook =
/*#__PURE__*/
function () {
  function AsyncHook(_ref) {
    var init = _ref.init,
        before = _ref.before,
        after = _ref.after,
        destroy = _ref.destroy,
        promiseResolve = _ref.promiseResolve;

    _classCallCheck(this, AsyncHook);

    if (init !== undefined && typeof init !== 'function') throw new errors.TypeError('ERR_ASYNC_CALLBACK', 'hook.init');
    if (before !== undefined && typeof before !== 'function') throw new errors.TypeError('ERR_ASYNC_CALLBACK', 'hook.before');
    if (after !== undefined && typeof after !== 'function') throw new errors.TypeError('ERR_ASYNC_CALLBACK', 'hook.after');
    if (destroy !== undefined && typeof destroy !== 'function') throw new errors.TypeError('ERR_ASYNC_CALLBACK', 'hook.destroy');
    if (promiseResolve !== undefined && typeof promiseResolve !== 'function') throw new errors.TypeError('ERR_ASYNC_CALLBACK', 'hook.promiseResolve');
    this[init_symbol] = init;
    this[before_symbol] = before;
    this[after_symbol] = after;
    this[destroy_symbol] = destroy;
    this[promise_resolve_symbol] = promiseResolve;
  }

  _createClass(AsyncHook, [{
    key: "enable",
    value: function enable() {
      // The set of callbacks for a hook should be the same regardless of whether
      // enable()/disable() are run during their execution. The following
      // references are reassigned to the tmp arrays if a hook is currently being
      // processed.
      var _getHookArrays = getHookArrays(),
          _getHookArrays2 = _slicedToArray(_getHookArrays, 2),
          hooks_array = _getHookArrays2[0],
          hook_fields = _getHookArrays2[1]; // Each hook is only allowed to be added once.


      if (hooks_array.includes(this)) return this;
      var prev_kTotals = hook_fields[kTotals];
      hook_fields[kTotals] = 0; // createHook() has already enforced that the callbacks are all functions,
      // so here simply increment the count of whether each callbacks exists or
      // not.

      hook_fields[kTotals] += hook_fields[kInit] += +!!this[init_symbol];
      hook_fields[kTotals] += hook_fields[kBefore] += +!!this[before_symbol];
      hook_fields[kTotals] += hook_fields[kAfter] += +!!this[after_symbol];
      hook_fields[kTotals] += hook_fields[kDestroy] += +!!this[destroy_symbol];
      hook_fields[kTotals] += hook_fields[kPromiseResolve] += +!!this[promise_resolve_symbol];
      hooks_array.push(this);

      if (prev_kTotals === 0 && hook_fields[kTotals] > 0) {
        enableHooks();
      }

      return this;
    }
  }, {
    key: "disable",
    value: function disable() {
      var _getHookArrays3 = getHookArrays(),
          _getHookArrays4 = _slicedToArray(_getHookArrays3, 2),
          hooks_array = _getHookArrays4[0],
          hook_fields = _getHookArrays4[1];

      var index = hooks_array.indexOf(this);
      if (index === -1) return this;
      var prev_kTotals = hook_fields[kTotals];
      hook_fields[kTotals] = 0;
      hook_fields[kTotals] += hook_fields[kInit] -= +!!this[init_symbol];
      hook_fields[kTotals] += hook_fields[kBefore] -= +!!this[before_symbol];
      hook_fields[kTotals] += hook_fields[kAfter] -= +!!this[after_symbol];
      hook_fields[kTotals] += hook_fields[kDestroy] -= +!!this[destroy_symbol];
      hook_fields[kTotals] += hook_fields[kPromiseResolve] -= +!!this[promise_resolve_symbol];
      hooks_array.splice(index, 1);

      if (prev_kTotals > 0 && hook_fields[kTotals] === 0) {
        disableHooks();
      }

      return this;
    }
  }]);

  return AsyncHook;
}();

function createHook(fns) {
  return new AsyncHook(fns);
}

function executionAsyncId() {
  return async_id_fields[kExecutionAsyncId];
}

function triggerAsyncId() {
  return async_id_fields[kTriggerAsyncId];
} // Embedder API //


var destroyedSymbol = Symbol('destroyed');

var AsyncResource =
/*#__PURE__*/
function () {
  function AsyncResource(type) {
    var opts = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};

    _classCallCheck(this, AsyncResource);

    if (typeof type !== 'string') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'type', 'string');

    if (typeof opts === 'number') {
      opts = {
        triggerAsyncId: opts,
        requireManualDestroy: false
      };
    } else if (opts.triggerAsyncId === undefined) {
      opts.triggerAsyncId = getDefaultTriggerAsyncId();
    } // Unlike emitInitScript, AsyncResource doesn't supports null as the
    // triggerAsyncId.


    var triggerAsyncId = opts.triggerAsyncId;

    if (!Number.isSafeInteger(triggerAsyncId) || triggerAsyncId < -1) {
      throw new errors.RangeError('ERR_INVALID_ASYNC_ID', 'triggerAsyncId', triggerAsyncId);
    }

    this[async_id_symbol] = newUid();
    this[trigger_async_id_symbol] = triggerAsyncId; // this prop name (destroyed) has to be synchronized with C++

    this[destroyedSymbol] = {
      destroyed: false
    };
    emitInit(this[async_id_symbol], type, this[trigger_async_id_symbol], this);

    if (!opts.requireManualDestroy) {
      registerDestroyHook(this, this[async_id_symbol], this[destroyedSymbol]);
    }
  }

  _createClass(AsyncResource, [{
    key: "emitBefore",
    value: function emitBefore() {
      _emitBefore(this[async_id_symbol], this[trigger_async_id_symbol]);

      return this;
    }
  }, {
    key: "emitAfter",
    value: function emitAfter() {
      _emitAfter(this[async_id_symbol]);

      return this;
    }
  }, {
    key: "runInAsyncScope",
    value: function runInAsyncScope(fn, thisArg) {
      _emitBefore(this[async_id_symbol], this[trigger_async_id_symbol]);

      var ret;

      try {
        for (var _len = arguments.length, args = new Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
          args[_key - 2] = arguments[_key];
        }

        ret = Function.prototype.apply.call(fn, thisArg, args);
      } finally {
        _emitAfter(this[async_id_symbol]);
      }

      return ret;
    }
  }, {
    key: "emitDestroy",
    value: function emitDestroy() {
      this[destroyedSymbol].destroyed = true;

      _emitDestroy(this[async_id_symbol]);

      return this;
    }
  }, {
    key: "asyncId",
    value: function asyncId() {
      return this[async_id_symbol];
    }
  }, {
    key: "triggerAsyncId",
    value: function triggerAsyncId() {
      return this[trigger_async_id_symbol];
    }
  }]);

  return AsyncResource;
}(); // triggerId was renamed to triggerAsyncId. This was in 8.2.0 during the
// experimental stage so the alias can be removed at any time, we are just
// being nice :)


Object.defineProperty(AsyncResource.prototype, 'triggerId', {
  get: internalUtil.deprecate(function () {
    return AsyncResource.prototype.triggerAsyncId;
  }, 'AsyncResource.triggerId is deprecated. ' + 'Use AsyncResource.triggerAsyncId instead.', 'DEP0072')
});

function runInAsyncIdScope(asyncId, cb) {
  // Store the async id now to make sure the stack is still good when the ids
  // are popped off the stack.
  var prevId = executionAsyncId();
  pushAsyncIds(asyncId, prevId);

  try {
    cb();
  } finally {
    popAsyncIds(asyncId);
  }
} // Placing all exports down here because the exported classes won't export
// otherwise.


module.exports = {
  // Public API
  createHook: createHook,
  executionAsyncId: executionAsyncId,
  triggerAsyncId: triggerAsyncId,
  // Embedder API
  AsyncResource: AsyncResource
}; // Deprecated API //
// currentId was renamed to executionAsyncId. This was in 8.2.0 during the
// experimental stage so the alias can be removed at any time, we are just
// being nice :)

Object.defineProperty(module.exports, 'currentId', {
  get: internalUtil.deprecate(function () {
    return executionAsyncId;
  }, 'async_hooks.currentId is deprecated. ' + 'Use async_hooks.executionAsyncId instead.', 'DEP0070')
}); // triggerId was renamed to triggerAsyncId. This was in 8.2.0 during the
// experimental stage so the alias can be removed at any time, we are just
// being nice :)

Object.defineProperty(module.exports, 'triggerId', {
  get: internalUtil.deprecate(function () {
    return triggerAsyncId;
  }, 'async_hooks.triggerId is deprecated. ' + 'Use async_hooks.triggerAsyncId instead.', 'DEP0071')
});
Object.defineProperty(module.exports, 'runInAsyncIdScope', {
  get: internalUtil.deprecate(function () {
    return runInAsyncIdScope;
  }, 'async_hooks.runInAsyncIdScope is deprecated. ' + 'Create an AsyncResource instead.', 'DEP0086')
});
Object.defineProperty(module.exports, 'newUid', {
  get: internalUtil.deprecate(function () {
    return newUid;
  }, 'async_hooks.newUid is deprecated. ' + 'Use AsyncResource instead.', 'DEP0085')
});
Object.defineProperty(module.exports, 'initTriggerId', {
  get: internalUtil.deprecate(function () {
    return getDefaultTriggerAsyncId;
  }, 'async_hooks.initTriggerId is deprecated. ' + 'Use the AsyncResource default instead.', 'DEP0085')
});
Object.defineProperty(module.exports, 'emitInit', {
  get: internalUtil.deprecate(function () {
    return emitInit;
  }, 'async_hooks.emitInit is deprecated. ' + 'Use AsyncResource constructor instead.', 'DEP0085')
});
Object.defineProperty(module.exports, 'emitBefore', {
  get: internalUtil.deprecate(function () {
    return _emitBefore;
  }, 'async_hooks.emitBefore is deprecated. ' + 'Use AsyncResource.emitBefore instead.', 'DEP0085')
});
Object.defineProperty(module.exports, 'emitAfter', {
  get: internalUtil.deprecate(function () {
    return _emitAfter;
  }, 'async_hooks.emitAfter is deprecated. ' + 'Use AsyncResource.emitAfter instead.', 'DEP0085')
});
Object.defineProperty(module.exports, 'emitDestroy', {
  get: internalUtil.deprecate(function () {
    return _emitDestroy;
  }, 'async_hooks.emitDestroy is deprecated. ' + 'Use AsyncResource.emitDestroy instead.', 'DEP0085')
});