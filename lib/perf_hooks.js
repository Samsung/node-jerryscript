'use strict';

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var _process$binding = process.binding('performance'),
    PerformanceEntry = _process$binding.PerformanceEntry,
    _mark = _process$binding.mark,
    _measure = _process$binding.measure,
    milestones = _process$binding.milestones,
    observerCounts = _process$binding.observerCounts,
    setupObservers = _process$binding.setupObservers,
    timeOrigin = _process$binding.timeOrigin,
    timeOriginTimestamp = _process$binding.timeOriginTimestamp,
    _timerify = _process$binding.timerify,
    constants = _process$binding.constants;

var NODE_PERFORMANCE_ENTRY_TYPE_NODE = constants.NODE_PERFORMANCE_ENTRY_TYPE_NODE,
    NODE_PERFORMANCE_ENTRY_TYPE_MARK = constants.NODE_PERFORMANCE_ENTRY_TYPE_MARK,
    NODE_PERFORMANCE_ENTRY_TYPE_MEASURE = constants.NODE_PERFORMANCE_ENTRY_TYPE_MEASURE,
    NODE_PERFORMANCE_ENTRY_TYPE_GC = constants.NODE_PERFORMANCE_ENTRY_TYPE_GC,
    NODE_PERFORMANCE_ENTRY_TYPE_FUNCTION = constants.NODE_PERFORMANCE_ENTRY_TYPE_FUNCTION,
    NODE_PERFORMANCE_ENTRY_TYPE_HTTP2 = constants.NODE_PERFORMANCE_ENTRY_TYPE_HTTP2,
    NODE_PERFORMANCE_MILESTONE_NODE_START = constants.NODE_PERFORMANCE_MILESTONE_NODE_START,
    NODE_PERFORMANCE_MILESTONE_V8_START = constants.NODE_PERFORMANCE_MILESTONE_V8_START,
    NODE_PERFORMANCE_MILESTONE_LOOP_START = constants.NODE_PERFORMANCE_MILESTONE_LOOP_START,
    NODE_PERFORMANCE_MILESTONE_LOOP_EXIT = constants.NODE_PERFORMANCE_MILESTONE_LOOP_EXIT,
    NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE = constants.NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE,
    NODE_PERFORMANCE_MILESTONE_ENVIRONMENT = constants.NODE_PERFORMANCE_MILESTONE_ENVIRONMENT,
    NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START = constants.NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START,
    NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END = constants.NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END,
    NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START = constants.NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START,
    NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END = constants.NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END,
    NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START = constants.NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START,
    NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END = constants.NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END,
    NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START = constants.NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START,
    NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END = constants.NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END;

var L = require('internal/linkedlist');

var kInspect = require('internal/util').customInspectSymbol;

var _require = require('util'),
    inherits = _require.inherits;

var kCallback = Symbol('callback');
var kTypes = Symbol('types');
var kEntries = Symbol('entries');
var kBuffer = Symbol('buffer');
var kBuffering = Symbol('buffering');
var kQueued = Symbol('queued');
var kTimerified = Symbol('timerified');
var kInsertEntry = Symbol('insert-entry');
var kIndexEntry = Symbol('index-entry');
var kClearEntry = Symbol('clear-entry');
var kGetEntries = Symbol('get-entries');
var kIndex = Symbol('index');
var kMarks = Symbol('marks');
var kCount = Symbol('count');
var kMaxCount = Symbol('max-count');
var kDefaultMaxCount = 150;
observerCounts[NODE_PERFORMANCE_ENTRY_TYPE_MARK] = 1;
observerCounts[NODE_PERFORMANCE_ENTRY_TYPE_MEASURE] = 1;
var observers = {};
var observerableTypes = ['node', 'mark', 'measure', 'gc', 'function', 'http2'];
var IDX_STREAM_STATS_ID = 0;
var IDX_STREAM_STATS_TIMETOFIRSTBYTE = 1;
var IDX_STREAM_STATS_TIMETOFIRSTHEADER = 2;
var IDX_STREAM_STATS_TIMETOFIRSTBYTESENT = 3;
var IDX_STREAM_STATS_SENTBYTES = 4;
var IDX_STREAM_STATS_RECEIVEDBYTES = 5;
var IDX_SESSION_STATS_TYPE = 0;
var IDX_SESSION_STATS_PINGRTT = 1;
var IDX_SESSION_STATS_FRAMESRECEIVED = 2;
var IDX_SESSION_STATS_FRAMESSENT = 3;
var IDX_SESSION_STATS_STREAMCOUNT = 4;
var IDX_SESSION_STATS_STREAMAVERAGEDURATION = 5;
var IDX_SESSION_STATS_DATA_SENT = 6;
var IDX_SESSION_STATS_DATA_RECEIVED = 7;
var IDX_SESSION_STATS_MAX_CONCURRENT_STREAMS = 8;
var sessionStats;
var streamStats;

function collectHttp2Stats(entry) {
  switch (entry.name) {
    case 'Http2Stream':
      if (streamStats === undefined) streamStats = process.binding('http2').streamStats;
      entry.id = streamStats[IDX_STREAM_STATS_ID] >>> 0;
      entry.timeToFirstByte = streamStats[IDX_STREAM_STATS_TIMETOFIRSTBYTE];
      entry.timeToFirstHeader = streamStats[IDX_STREAM_STATS_TIMETOFIRSTHEADER];
      entry.timeToFirstByteSent = streamStats[IDX_STREAM_STATS_TIMETOFIRSTBYTESENT];
      entry.bytesWritten = streamStats[IDX_STREAM_STATS_SENTBYTES];
      entry.bytesRead = streamStats[IDX_STREAM_STATS_RECEIVEDBYTES];
      break;

    case 'Http2Session':
      if (sessionStats === undefined) sessionStats = process.binding('http2').sessionStats;
      entry.type = sessionStats[IDX_SESSION_STATS_TYPE] >>> 0 === 0 ? 'server' : 'client';
      entry.pingRTT = sessionStats[IDX_SESSION_STATS_PINGRTT];
      entry.framesReceived = sessionStats[IDX_SESSION_STATS_FRAMESRECEIVED];
      entry.framesSent = sessionStats[IDX_SESSION_STATS_FRAMESSENT];
      entry.streamCount = sessionStats[IDX_SESSION_STATS_STREAMCOUNT];
      entry.streamAverageDuration = sessionStats[IDX_SESSION_STATS_STREAMAVERAGEDURATION];
      entry.bytesWritten = sessionStats[IDX_SESSION_STATS_DATA_SENT];
      entry.bytesRead = sessionStats[IDX_SESSION_STATS_DATA_RECEIVED];
      entry.maxConcurrentStreams = sessionStats[IDX_SESSION_STATS_MAX_CONCURRENT_STREAMS];
      break;
  }
}

var errors;

function lazyErrors() {
  if (errors === undefined) errors = require('internal/errors');
  return errors;
}

function _now() {
  var hr = process.hrtime();
  return hr[0] * 1000 + hr[1] / 1e6;
}

function getMilestoneTimestamp(milestoneIdx) {
  var ns = milestones[milestoneIdx];
  if (ns === -1) return ns;
  return ns / 1e6 - timeOrigin;
}

var PerformanceNodeTiming =
/*#__PURE__*/
function () {
  function PerformanceNodeTiming() {
    _classCallCheck(this, PerformanceNodeTiming);
  }

  _createClass(PerformanceNodeTiming, [{
    key: kInspect,
    value: function value() {
      return {
        name: 'node',
        entryType: 'node',
        startTime: this.startTime,
        duration: this.duration,
        nodeStart: this.nodeStart,
        v8Start: this.v8Start,
        bootstrapComplete: this.bootstrapComplete,
        environment: this.environment,
        loopStart: this.loopStart,
        loopExit: this.loopExit,
        thirdPartyMainStart: this.thirdPartyMainStart,
        thirdPartyMainEnd: this.thirdPartyMainEnd,
        clusterSetupStart: this.clusterSetupStart,
        clusterSetupEnd: this.clusterSetupEnd,
        moduleLoadStart: this.moduleLoadStart,
        moduleLoadEnd: this.moduleLoadEnd,
        preloadModuleLoadStart: this.preloadModuleLoadStart,
        preloadModuleLoadEnd: this.preloadModuleLoadEnd
      };
    }
  }, {
    key: "name",
    get: function get() {
      return 'node';
    }
  }, {
    key: "entryType",
    get: function get() {
      return 'node';
    }
  }, {
    key: "startTime",
    get: function get() {
      return 0;
    }
  }, {
    key: "duration",
    get: function get() {
      return _now() - timeOrigin;
    }
  }, {
    key: "nodeStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_NODE_START);
    }
  }, {
    key: "v8Start",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_V8_START);
    }
  }, {
    key: "environment",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_ENVIRONMENT);
    }
  }, {
    key: "loopStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_LOOP_START);
    }
  }, {
    key: "loopExit",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_LOOP_EXIT);
    }
  }, {
    key: "bootstrapComplete",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_BOOTSTRAP_COMPLETE);
    }
  }, {
    key: "thirdPartyMainStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_START);
    }
  }, {
    key: "thirdPartyMainEnd",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_THIRD_PARTY_MAIN_END);
    }
  }, {
    key: "clusterSetupStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_START);
    }
  }, {
    key: "clusterSetupEnd",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_CLUSTER_SETUP_END);
    }
  }, {
    key: "moduleLoadStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_START);
    }
  }, {
    key: "moduleLoadEnd",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_MODULE_LOAD_END);
    }
  }, {
    key: "preloadModuleLoadStart",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_START);
    }
  }, {
    key: "preloadModuleLoadEnd",
    get: function get() {
      return getMilestoneTimestamp(NODE_PERFORMANCE_MILESTONE_PRELOAD_MODULE_LOAD_END);
    }
  }]);

  return PerformanceNodeTiming;
}(); // Use this instead of Extends because we want PerformanceEntry in the
// prototype chain but we do not want to use the PerformanceEntry
// constructor for this.


inherits(PerformanceNodeTiming, PerformanceEntry);
var nodeTiming = new PerformanceNodeTiming(); // Maintains a list of entries as a linked list stored in insertion order.

var PerformanceObserverEntryList =
/*#__PURE__*/
function () {
  function PerformanceObserverEntryList() {
    var _Object$definePropert;

    _classCallCheck(this, PerformanceObserverEntryList);

    Object.defineProperties(this, (_Object$definePropert = {}, _defineProperty(_Object$definePropert, kEntries, {
      writable: true,
      enumerable: false,
      value: {}
    }), _defineProperty(_Object$definePropert, kCount, {
      writable: true,
      enumerable: false,
      value: 0
    }), _Object$definePropert));
    L.init(this[kEntries]);
  }

  _createClass(PerformanceObserverEntryList, [{
    key: kInsertEntry,
    value: function value(entry) {
      var item = {
        entry: entry
      };
      L.append(this[kEntries], item);
      this[kCount]++;
      this[kIndexEntry](item);
    }
  }, {
    key: kIndexEntry,
    value: function value(entry) {// Default implementation does nothing
    }
  }, {
    key: kGetEntries,
    value: function value(name, type) {
      var ret = [];
      var list = this[kEntries];

      if (!L.isEmpty(list)) {
        var item = L.peek(list);

        while (item && item !== list) {
          var entry = item.entry;

          if (name && entry.name !== name || type && entry.entryType !== type) {
            item = item._idlePrev;
            continue;
          }

          sortedInsert(ret, entry);
          item = item._idlePrev;
        }
      }

      return ret;
    } // While the items are stored in insertion order, getEntries() is
    // required to return items sorted by startTime.

  }, {
    key: "getEntries",
    value: function getEntries() {
      return this[kGetEntries]();
    }
  }, {
    key: "getEntriesByType",
    value: function getEntriesByType(type) {
      return this[kGetEntries](undefined, "".concat(type));
    }
  }, {
    key: "getEntriesByName",
    value: function getEntriesByName(name, type) {
      return this[kGetEntries]("".concat(name), type !== undefined ? "".concat(type) : type);
    }
  }, {
    key: "length",
    get: function get() {
      return this[kCount];
    }
  }]);

  return PerformanceObserverEntryList;
}();

var PerformanceObserver =
/*#__PURE__*/
function () {
  function PerformanceObserver(callback) {
    var _Object$definePropert2;

    _classCallCheck(this, PerformanceObserver);

    if (typeof callback !== 'function') {
      var _errors = lazyErrors();

      throw new _errors.TypeError('ERR_INVALID_CALLBACK');
    }

    Object.defineProperties(this, (_Object$definePropert2 = {}, _defineProperty(_Object$definePropert2, kTypes, {
      enumerable: false,
      writable: true,
      value: {}
    }), _defineProperty(_Object$definePropert2, kCallback, {
      enumerable: false,
      writable: true,
      value: callback
    }), _defineProperty(_Object$definePropert2, kBuffer, {
      enumerable: false,
      writable: true,
      value: new PerformanceObserverEntryList()
    }), _defineProperty(_Object$definePropert2, kBuffering, {
      enumerable: false,
      writable: true,
      value: false
    }), _defineProperty(_Object$definePropert2, kQueued, {
      enumerable: false,
      writable: true,
      value: false
    }), _Object$definePropert2));
  }

  _createClass(PerformanceObserver, [{
    key: "disconnect",
    value: function disconnect() {
      var types = this[kTypes];
      var keys = Object.keys(types);

      for (var n = 0; n < keys.length; n++) {
        var item = types[keys[n]];

        if (item) {
          L.remove(item);
          observerCounts[keys[n]]--;
        }
      }

      this[kTypes] = {};
    }
  }, {
    key: "observe",
    value: function observe(options) {
      var errors = lazyErrors();

      if (_typeof(options) !== 'object' || options == null) {
        throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'options', 'Object');
      }

      if (!Array.isArray(options.entryTypes)) {
        throw new errors.TypeError('ERR_INVALID_OPT_VALUE', 'entryTypes', options);
      }

      var entryTypes = options.entryTypes.filter(filterTypes).map(mapTypes);

      if (entryTypes.length === 0) {
        throw new errors.Error('ERR_VALID_PERFORMANCE_ENTRY_TYPE');
      }

      this.disconnect();
      this[kBuffer][kEntries] = [];
      L.init(this[kBuffer][kEntries]);
      this[kBuffering] = Boolean(options.buffered);

      for (var n = 0; n < entryTypes.length; n++) {
        var entryType = entryTypes[n];
        var list = getObserversList(entryType);
        var item = {
          obs: this
        };
        this[kTypes][entryType] = item;
        L.append(list, item);
        observerCounts[entryType]++;
      }
    }
  }]);

  return PerformanceObserver;
}();

var Performance =
/*#__PURE__*/
function (_PerformanceObserverE) {
  _inherits(Performance, _PerformanceObserverE);

  function Performance() {
    var _this;

    _classCallCheck(this, Performance);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(Performance).call(this));
    _this[kIndex] = _defineProperty({}, kMarks, new Set());
    _this[kMaxCount] = kDefaultMaxCount;

    _this[kInsertEntry](nodeTiming);

    return _this;
  }

  _createClass(Performance, [{
    key: kIndexEntry,
    value: function value(item) {
      var index = this[kIndex];
      var type = item.entry.entryType;
      var items = index[type];

      if (!items) {
        items = index[type] = {};
        L.init(items);
      }

      var entry = item.entry;
      L.append(items, {
        entry: entry,
        item: item
      });
      var count = this[kCount];

      if (count > this[kMaxCount]) {
        var text = count === 1 ? 'is 1 entry' : "are ".concat(count, " entries");
        process.emitWarning('Possible perf_hooks memory leak detected. ' + "There ".concat(text, " in the ") + 'Performance Timeline. Use the clear methods ' + 'to remove entries that are no longer needed or ' + 'set performance.maxEntries equal to a higher ' + 'value (currently the maxEntries is ' + "".concat(this[kMaxCount], ")."));
      }
    }
  }, {
    key: kClearEntry,
    value: function value(type, name) {
      var index = this[kIndex];
      var items = index[type];
      if (!items) return;
      var item = L.peek(items);

      while (item && item !== items) {
        var entry = item.entry;
        var next = item._idlePrev;

        if (name !== undefined) {
          if (entry.name === "".concat(name)) {
            L.remove(item); // remove from the index

            L.remove(item.item); // remove from the master

            this[kCount]--;
          }
        } else {
          L.remove(item); // remove from the index

          L.remove(item.item); // remove from the master

          this[kCount]--;
        }

        item = next;
      }
    }
  }, {
    key: "now",
    value: function now() {
      return _now() - timeOrigin;
    }
  }, {
    key: "mark",
    value: function mark(name) {
      name = "".concat(name);

      _mark(name);

      this[kIndex][kMarks].add(name);
    }
  }, {
    key: "measure",
    value: function measure(name, startMark, endMark) {
      name = "".concat(name);
      endMark = "".concat(endMark);
      startMark = startMark !== undefined ? "".concat(startMark) : '';
      var marks = this[kIndex][kMarks];

      if (!marks.has(endMark) && !(endMark in nodeTiming)) {
        var _errors2 = lazyErrors();

        throw new _errors2.Error('ERR_INVALID_PERFORMANCE_MARK', endMark);
      }

      _measure(name, startMark, endMark);
    }
  }, {
    key: "clearMarks",
    value: function clearMarks(name) {
      name = name !== undefined ? "".concat(name) : name;
      this[kClearEntry]('mark', name);
      if (name !== undefined) this[kIndex][kMarks]["delete"](name);else this[kIndex][kMarks].clear();
    }
  }, {
    key: "clearMeasures",
    value: function clearMeasures(name) {
      this[kClearEntry]('measure', name);
    }
  }, {
    key: "clearGC",
    value: function clearGC() {
      this[kClearEntry]('gc');
    }
  }, {
    key: "clearFunctions",
    value: function clearFunctions(name) {
      this[kClearEntry]('function', name);
    }
  }, {
    key: "clearEntries",
    value: function clearEntries(name) {
      this[kClearEntry](name);
    }
  }, {
    key: "timerify",
    value: function timerify(fn) {
      var _Object$definePropert3;

      if (typeof fn !== 'function') {
        var _errors3 = lazyErrors();

        throw new _errors3.TypeError('ERR_INVALID_ARG_TYPE', 'fn', 'Function');
      }

      if (fn[kTimerified]) return fn[kTimerified];

      var ret = _timerify(fn, fn.length);

      Object.defineProperty(fn, kTimerified, {
        enumerable: false,
        configurable: true,
        writable: false,
        value: ret
      });
      Object.defineProperties(ret, (_Object$definePropert3 = {}, _defineProperty(_Object$definePropert3, kTimerified, {
        enumerable: false,
        configurable: true,
        writable: false,
        value: ret
      }), _defineProperty(_Object$definePropert3, "name", {
        enumerable: false,
        configurable: true,
        writable: false,
        value: "timerified ".concat(fn.name)
      }), _Object$definePropert3));
      return ret;
    }
  }, {
    key: kInspect,
    value: function value() {
      return {
        maxEntries: this.maxEntries,
        nodeTiming: this.nodeTiming,
        timeOrigin: this.timeOrigin
      };
    }
  }, {
    key: "maxEntries",
    set: function set(val) {
      if (typeof val !== 'number' || val >>> 0 !== val) {
        var _errors4 = lazyErrors();

        throw new _errors4.TypeError('ERR_INVALID_ARG_TYPE', 'val', 'number');
      }

      this[kMaxCount] = Math.max(1, val >>> 0);
    },
    get: function get() {
      return this[kMaxCount];
    }
  }, {
    key: "nodeTiming",
    get: function get() {
      return nodeTiming;
    }
  }, {
    key: "timeOrigin",
    get: function get() {
      return timeOriginTimestamp;
    }
  }]);

  return Performance;
}(PerformanceObserverEntryList);

var performance = new Performance();

function getObserversList(type) {
  var list = observers[type];

  if (list === undefined) {
    list = observers[type] = {};
    L.init(list);
  }

  return list;
}

function doNotify() {
  this[kQueued] = false;
  this[kCallback](this[kBuffer], this);
  this[kBuffer][kEntries] = [];
  L.init(this[kBuffer][kEntries]);
} // Set up the callback used to receive PerformanceObserver notifications


function observersCallback(entry) {
  var type = mapTypes(entry.entryType);
  if (type === NODE_PERFORMANCE_ENTRY_TYPE_HTTP2) collectHttp2Stats(entry);
  performance[kInsertEntry](entry);
  var list = getObserversList(type);
  var current = L.peek(list);

  while (current && current.obs) {
    var observer = current.obs; // First, add the item to the observers buffer

    var buffer = observer[kBuffer];
    buffer[kInsertEntry](entry); // Second, check to see if we're buffering

    if (observer[kBuffering]) {
      // If we are, schedule a setImmediate call if one hasn't already
      if (!observer[kQueued]) {
        observer[kQueued] = true; // Use setImmediate instead of nextTick to give more time
        // for multiple entries to collect.

        setImmediate(doNotify.bind(observer));
      }
    } else {
      // If not buffering, notify immediately
      doNotify.call(observer);
    }

    current = current._idlePrev;
  }
}

setupObservers(observersCallback);

function filterTypes(i) {
  return observerableTypes.indexOf("".concat(i)) >= 0;
}

function mapTypes(i) {
  switch (i) {
    case 'node':
      return NODE_PERFORMANCE_ENTRY_TYPE_NODE;

    case 'mark':
      return NODE_PERFORMANCE_ENTRY_TYPE_MARK;

    case 'measure':
      return NODE_PERFORMANCE_ENTRY_TYPE_MEASURE;

    case 'gc':
      return NODE_PERFORMANCE_ENTRY_TYPE_GC;

    case 'function':
      return NODE_PERFORMANCE_ENTRY_TYPE_FUNCTION;

    case 'http2':
      return NODE_PERFORMANCE_ENTRY_TYPE_HTTP2;
  }
} // The specification requires that PerformanceEntry instances are sorted
// according to startTime. Unfortunately, they are not necessarily created
// in that same order, and can be reported to the JS layer in any order,
// which means we need to keep the list sorted as we insert.


function getInsertLocation(list, entryStartTime) {
  var start = 0;
  var end = list.length;

  while (start < end) {
    var pivot = end + start >>> 1;
    if (list[pivot].startTime === entryStartTime) return pivot;
    if (list[pivot].startTime < entryStartTime) start = pivot + 1;else end = pivot;
  }

  return start;
}

function sortedInsert(list, entry) {
  var entryStartTime = entry.startTime;

  if (list.length === 0 || list[list.length - 1].startTime < entryStartTime) {
    list.push(entry);
    return;
  }

  if (list[0] && list[0].startTime > entryStartTime) {
    list.unshift(entry);
    return;
  }

  var location = getInsertLocation(list, entryStartTime);
  list.splice(location, 0, entry);
}

module.exports = {
  performance: performance,
  PerformanceObserver: PerformanceObserver
};
Object.defineProperty(module.exports, 'constants', {
  configurable: false,
  enumerable: true,
  value: constants
});