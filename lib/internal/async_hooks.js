'use strict';

var errors = require('internal/errors');

var async_wrap = process.binding('async_wrap');
/* async_hook_fields is a Uint32Array wrapping the uint32_t array of
 * Environment::AsyncHooks::fields_[]. Each index tracks the number of active
 * hooks for each type.
 *
 * async_id_fields is a Float64Array wrapping the double array of
 * Environment::AsyncHooks::async_id_fields_[]. Each index contains the ids for
 * the various asynchronous states of the application. These are:
 *  kExecutionAsyncId: The async_id assigned to the resource responsible for the
 *    current execution stack.
 *  kTriggerAsyncId: The async_id of the resource that caused (or 'triggered')
 *    the resource corresponding to the current execution stack.
 *  kAsyncIdCounter: Incremental counter tracking the next assigned async_id.
 *  kDefaultTriggerAsyncId: Written immediately before a resource's constructor
 *    that sets the value of the init()'s triggerAsyncId. The precedence order
 *    of retrieving the triggerAsyncId value is:
 *    1. the value passed directly to the constructor
 *    2. value set in kDefaultTriggerAsyncId
 *    3. executionAsyncId of the current resource.
 *
 * async_ids_stack is a Float64Array that contains part of the async ID
 * stack. Each pushAsyncIds() call adds two doubles to it, and each
 * popAsyncIds() call removes two doubles from it.
 * It has a fixed size, so if that is exceeded, calls to the native
 * side are used instead in pushAsyncIds() and popAsyncIds().
 */

var async_id_symbol = async_wrap.async_id_symbol,
    async_hook_fields = async_wrap.async_hook_fields,
    async_id_fields = async_wrap.async_id_fields; // Store the pair executionAsyncId and triggerAsyncId in a std::stack on
// Environment::AsyncHooks::async_ids_stack_ tracks the resource responsible for
// the current execution stack. This is unwound as each resource exits. In the
// case of a fatal exception this stack is emptied after calling each hook's
// after() callback.

var pushAsyncIds_ = async_wrap.pushAsyncIds,
    popAsyncIds_ = async_wrap.popAsyncIds; // For performance reasons, only track Promises when a hook is enabled.

var enablePromiseHook = async_wrap.enablePromiseHook,
    disablePromiseHook = async_wrap.disablePromiseHook; // Properties in active_hooks are used to keep track of the set of hooks being
// executed in case another hook is enabled/disabled. The new set of hooks is
// then restored once the active set of hooks is finished executing.

var active_hooks = {
  // Array of all AsyncHooks that will be iterated whenever an async event
  // fires. Using var instead of (preferably const) in order to assign
  // active_hooks.tmp_array if a hook is enabled/disabled during hook
  // execution.
  array: [],
  // Use a counter to track nested calls of async hook callbacks and make sure
  // the active_hooks.array isn't altered mid execution.
  call_depth: 0,
  // Use to temporarily store and updated active_hooks.array if the user
  // enables or disables a hook while hooks are being processed. If a hook is
  // enabled() or disabled() during hook execution then the current set of
  // active hooks is duplicated and set equal to active_hooks.tmp_array. Any
  // subsequent changes are on the duplicated array. When all hooks have
  // completed executing active_hooks.tmp_array is assigned to
  // active_hooks.array.
  tmp_array: null,
  // Keep track of the field counts held in active_hooks.tmp_array. Because the
  // async_hook_fields can't be reassigned, store each uint32 in an array that
  // is written back to async_hook_fields when active_hooks.array is restored.
  tmp_fields: null
}; // Each constant tracks how many callbacks there are for any given step of
// async execution. These are tracked so if the user didn't include callbacks
// for a given step, that step can bail out early.

var _async_wrap$constants = async_wrap.constants,
    kInit = _async_wrap$constants.kInit,
    kBefore = _async_wrap$constants.kBefore,
    kAfter = _async_wrap$constants.kAfter,
    kDestroy = _async_wrap$constants.kDestroy,
    kPromiseResolve = _async_wrap$constants.kPromiseResolve,
    kCheck = _async_wrap$constants.kCheck,
    kExecutionAsyncId = _async_wrap$constants.kExecutionAsyncId,
    kAsyncIdCounter = _async_wrap$constants.kAsyncIdCounter,
    kTriggerAsyncId = _async_wrap$constants.kTriggerAsyncId,
    kDefaultTriggerAsyncId = _async_wrap$constants.kDefaultTriggerAsyncId,
    kStackLength = _async_wrap$constants.kStackLength; // Used in AsyncHook and AsyncResource.

var init_symbol = Symbol('init');
var before_symbol = Symbol('before');
var after_symbol = Symbol('after');
var destroy_symbol = Symbol('destroy');
var promise_resolve_symbol = Symbol('promiseResolve');
var emitBeforeNative = emitHookFactory(before_symbol, 'emitBeforeNative');
var emitAfterNative = emitHookFactory(after_symbol, 'emitAfterNative');
var emitDestroyNative = emitHookFactory(destroy_symbol, 'emitDestroyNative');
var emitPromiseResolveNative = emitHookFactory(promise_resolve_symbol, 'emitPromiseResolveNative'); // Setup the callbacks that node::AsyncWrap will call when there are hooks to
// process. They use the same functions as the JS embedder API. These callbacks
// are setup immediately to prevent async_wrap.setupHooks() from being hijacked
// and the cost of doing so is negligible.

async_wrap.setupHooks({
  init: emitInitNative,
  before: emitBeforeNative,
  after: emitAfterNative,
  destroy: emitDestroyNative,
  promise_resolve: emitPromiseResolveNative
}); // Used to fatally abort the process if a callback throws.

function fatalError(e) {
  if (typeof e.stack === 'string') {
    process._rawDebug(e.stack);
  } else {
    var o = {
      message: e
    };
    Error.captureStackTrace(o, fatalError);

    process._rawDebug(o.stack);
  }

  if (process.binding('config').shouldAbortOnUncaughtException) {
    process.abort();
  }

  process.exit(1);
}

function validateAsyncId(asyncId, type) {
  // Skip validation when async_hooks is disabled
  if (async_hook_fields[kCheck] <= 0) return;

  if (!Number.isSafeInteger(asyncId) || asyncId < -1) {
    fatalError(new errors.RangeError('ERR_INVALID_ASYNC_ID', type, asyncId));
  }
} // Emit From Native //
// Used by C++ to call all init() callbacks. Because some state can be setup
// from C++ there's no need to perform all the same operations as in
// emitInitScript.


function emitInitNative(asyncId, type, triggerAsyncId, resource) {
  active_hooks.call_depth += 1; // Use a single try/catch for all hooks to avoid setting up one per iteration.

  try {
    for (var i = 0; i < active_hooks.array.length; i++) {
      if (typeof active_hooks.array[i][init_symbol] === 'function') {
        active_hooks.array[i][init_symbol](asyncId, type, triggerAsyncId, resource);
      }
    }
  } catch (e) {
    fatalError(e);
  } finally {
    active_hooks.call_depth -= 1;
  } // Hooks can only be restored if there have been no recursive hook calls.
  // Also the active hooks do not need to be restored if enable()/disable()
  // weren't called during hook execution, in which case active_hooks.tmp_array
  // will be null.


  if (active_hooks.call_depth === 0 && active_hooks.tmp_array !== null) {
    restoreActiveHooks();
  }
}

function emitHookFactory(symbol, name) {
  // Called from native. The asyncId stack handling is taken care of there
  // before this is called.
  // eslint-disable-next-line func-style
  var fn = function fn(asyncId) {
    active_hooks.call_depth += 1; // Use a single try/catch for all hook to avoid setting up one per
    // iteration.

    try {
      for (var i = 0; i < active_hooks.array.length; i++) {
        if (typeof active_hooks.array[i][symbol] === 'function') {
          active_hooks.array[i][symbol](asyncId);
        }
      }
    } catch (e) {
      fatalError(e);
    } finally {
      active_hooks.call_depth -= 1;
    } // Hooks can only be restored if there have been no recursive hook calls.
    // Also the active hooks do not need to be restored if enable()/disable()
    // weren't called during hook execution, in which case
    // active_hooks.tmp_array will be null.


    if (active_hooks.call_depth === 0 && active_hooks.tmp_array !== null) {
      restoreActiveHooks();
    }
  }; // Set the name property of the anonymous function as it looks good in the
  // stack trace.


  Object.defineProperty(fn, 'name', {
    value: name
  });
  return fn;
} // Manage Active Hooks //


function getHookArrays() {
  if (active_hooks.call_depth === 0) return [active_hooks.array, async_hook_fields]; // If this hook is being enabled while in the middle of processing the array
  // of currently active hooks then duplicate the current set of active hooks
  // and store this there. This shouldn't fire until the next time hooks are
  // processed.

  if (active_hooks.tmp_array === null) storeActiveHooks();
  return [active_hooks.tmp_array, active_hooks.tmp_fields];
}

function storeActiveHooks() {
  active_hooks.tmp_array = active_hooks.array.slice(); // Don't want to make the assumption that kInit to kDestroy are indexes 0 to
  // 4. So do this the long way.

  active_hooks.tmp_fields = [];
  active_hooks.tmp_fields[kInit] = async_hook_fields[kInit];
  active_hooks.tmp_fields[kBefore] = async_hook_fields[kBefore];
  active_hooks.tmp_fields[kAfter] = async_hook_fields[kAfter];
  active_hooks.tmp_fields[kDestroy] = async_hook_fields[kDestroy];
  active_hooks.tmp_fields[kPromiseResolve] = async_hook_fields[kPromiseResolve];
} // Then restore the correct hooks array in case any hooks were added/removed
// during hook callback execution.


function restoreActiveHooks() {
  active_hooks.array = active_hooks.tmp_array;
  async_hook_fields[kInit] = active_hooks.tmp_fields[kInit];
  async_hook_fields[kBefore] = active_hooks.tmp_fields[kBefore];
  async_hook_fields[kAfter] = active_hooks.tmp_fields[kAfter];
  async_hook_fields[kDestroy] = active_hooks.tmp_fields[kDestroy];
  async_hook_fields[kPromiseResolve] = active_hooks.tmp_fields[kPromiseResolve];
  active_hooks.tmp_array = null;
  active_hooks.tmp_fields = null;
}

function enableHooks() {
  enablePromiseHook();
  async_hook_fields[kCheck] += 1;
}

function disableHooks() {
  disablePromiseHook();
  async_hook_fields[kCheck] -= 1;
} // Sensitive Embedder API //
// Increment the internal id counter and return the value. Important that the
// counter increment first. Since it's done the same way in
// Environment::new_async_uid()


function newUid() {
  return ++async_id_fields[kAsyncIdCounter];
}

function getOrSetAsyncId(object) {
  if (object.hasOwnProperty(async_id_symbol)) {
    return object[async_id_symbol];
  }

  return object[async_id_symbol] = newUid();
} // Return the triggerAsyncId meant for the constructor calling it. It's up to
// the user to safeguard this call and make sure it's zero'd out when the
// constructor is complete.


function getDefaultTriggerAsyncId() {
  var defaultTriggerAsyncId = async_id_fields[kDefaultTriggerAsyncId]; // If defaultTriggerAsyncId isn't set, use the executionAsyncId

  if (defaultTriggerAsyncId < 0) defaultTriggerAsyncId = async_id_fields[kExecutionAsyncId];
  return defaultTriggerAsyncId;
}

function defaultTriggerAsyncIdScope(triggerAsyncId, block) {
  // CHECK(Number.isSafeInteger(triggerAsyncId))
  // CHECK(triggerAsyncId > 0)
  var oldDefaultTriggerAsyncId = async_id_fields[kDefaultTriggerAsyncId];
  async_id_fields[kDefaultTriggerAsyncId] = triggerAsyncId;
  var ret;

  try {
    for (var _len = arguments.length, args = new Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
      args[_key - 2] = arguments[_key];
    }

    ret = Function.prototype.apply.call(block, null, args);
  } finally {
    async_id_fields[kDefaultTriggerAsyncId] = oldDefaultTriggerAsyncId;
  }

  return ret;
}

function emitInitScript(asyncId, type, triggerAsyncId, resource) {
  validateAsyncId(asyncId, 'asyncId');
  if (triggerAsyncId !== null) validateAsyncId(triggerAsyncId, 'triggerAsyncId');

  if (async_hook_fields[kCheck] > 0 && (typeof type !== 'string' || type.length <= 0)) {
    throw new errors.TypeError('ERR_ASYNC_TYPE', type);
  } // Short circuit all checks for the common case. Which is that no hooks have
  // been set. Do this to remove performance impact for embedders (and core).


  if (async_hook_fields[kInit] === 0) return; // This can run after the early return check b/c running this function
  // manually means that the embedder must have used getDefaultTriggerAsyncId().

  if (triggerAsyncId === null) {
    triggerAsyncId = getDefaultTriggerAsyncId();
  }

  emitInitNative(asyncId, type, triggerAsyncId, resource);
}

function emitBeforeScript(asyncId, triggerAsyncId) {
  // Validate the ids. An id of -1 means it was never set and is visible on the
  // call graph. An id < -1 should never happen in any circumstance. Throw
  // on user calls because async state should still be recoverable.
  validateAsyncId(asyncId, 'asyncId');
  validateAsyncId(triggerAsyncId, 'triggerAsyncId');
  pushAsyncIds(asyncId, triggerAsyncId);
  if (async_hook_fields[kBefore] > 0) emitBeforeNative(asyncId);
}

function emitAfterScript(asyncId) {
  validateAsyncId(asyncId, 'asyncId');
  if (async_hook_fields[kAfter] > 0) emitAfterNative(asyncId);
  popAsyncIds(asyncId);
}

function emitDestroyScript(asyncId) {
  validateAsyncId(asyncId, 'asyncId'); // Return early if there are no destroy callbacks, or invalid asyncId.

  if (async_hook_fields[kDestroy] === 0 || asyncId <= 0) return;
  async_wrap.queueDestroyAsyncId(asyncId);
} // This is the equivalent of the native push_async_ids() call.


function pushAsyncIds(asyncId, triggerAsyncId) {
  var offset = async_hook_fields[kStackLength];
  if (offset * 2 >= async_wrap.async_ids_stack.length) return pushAsyncIds_(asyncId, triggerAsyncId);
  async_wrap.async_ids_stack[offset * 2] = async_id_fields[kExecutionAsyncId];
  async_wrap.async_ids_stack[offset * 2 + 1] = async_id_fields[kTriggerAsyncId];
  async_hook_fields[kStackLength]++;
  async_id_fields[kExecutionAsyncId] = asyncId;
  async_id_fields[kTriggerAsyncId] = triggerAsyncId;
} // This is the equivalent of the native pop_async_ids() call.


function popAsyncIds(asyncId) {
  if (async_hook_fields[kStackLength] === 0) return false;
  var stackLength = async_hook_fields[kStackLength];

  if (async_hook_fields[kCheck] > 0 && async_id_fields[kExecutionAsyncId] !== asyncId) {
    // Do the same thing as the native code (i.e. crash hard).
    return popAsyncIds_(asyncId);
  }

  var offset = stackLength - 1;
  async_id_fields[kExecutionAsyncId] = async_wrap.async_ids_stack[2 * offset];
  async_id_fields[kTriggerAsyncId] = async_wrap.async_ids_stack[2 * offset + 1];
  async_hook_fields[kStackLength] = offset;
  return offset > 0;
}

module.exports = {
  // Private API
  getHookArrays: getHookArrays,
  symbols: {
    init_symbol: init_symbol,
    before_symbol: before_symbol,
    after_symbol: after_symbol,
    destroy_symbol: destroy_symbol,
    promise_resolve_symbol: promise_resolve_symbol
  },
  enableHooks: enableHooks,
  disableHooks: disableHooks,
  // Sensitive Embedder API
  newUid: newUid,
  getOrSetAsyncId: getOrSetAsyncId,
  getDefaultTriggerAsyncId: getDefaultTriggerAsyncId,
  defaultTriggerAsyncIdScope: defaultTriggerAsyncIdScope,
  emitInit: emitInitScript,
  emitBefore: emitBeforeScript,
  emitAfter: emitAfterScript,
  emitDestroy: emitDestroyScript
};