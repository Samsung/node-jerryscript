// Copyright (c) 2014, StrongLoop Inc.
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
'use strict';

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _process$binding = process.binding('serdes'),
    _Serializer = _process$binding.Serializer,
    _Deserializer = _process$binding.Deserializer;

var _process$binding2 = process.binding('buffer'),
    copy = _process$binding2.copy;

var _require2 = require('internal/util'),
    objectToString = _require2.objectToString;

var _require3 = require('internal/buffer'),
    FastBuffer = _require3.FastBuffer; // Calling exposed c++ functions directly throws exception as it expected to be
// called with new operator and caused an assert to fire.
// Creating JS wrapper so that it gets caught at JS layer.


var Serializer =
/*#__PURE__*/
function (_Serializer2) {
  _inherits(Serializer, _Serializer2);

  function Serializer() {
    _classCallCheck(this, Serializer);

    return _possibleConstructorReturn(this, _getPrototypeOf(Serializer).apply(this, arguments));
  }

  return Serializer;
}(_Serializer);

var Deserializer =
/*#__PURE__*/
function (_Deserializer2) {
  _inherits(Deserializer, _Deserializer2);

  function Deserializer() {
    _classCallCheck(this, Deserializer);

    return _possibleConstructorReturn(this, _getPrototypeOf(Deserializer).apply(this, arguments));
  }

  return Deserializer;
}(_Deserializer);

var _process$binding3 = process.binding('v8'),
    cachedDataVersionTag = _process$binding3.cachedDataVersionTag,
    setFlagsFromString = _process$binding3.setFlagsFromString,
    heapStatisticsArrayBuffer = _process$binding3.heapStatisticsArrayBuffer,
    heapSpaceStatisticsArrayBuffer = _process$binding3.heapSpaceStatisticsArrayBuffer,
    updateHeapStatisticsArrayBuffer = _process$binding3.updateHeapStatisticsArrayBuffer,
    updateHeapSpaceStatisticsArrayBuffer = _process$binding3.updateHeapSpaceStatisticsArrayBuffer,
    kTotalHeapSizeIndex = _process$binding3.kTotalHeapSizeIndex,
    kTotalHeapSizeExecutableIndex = _process$binding3.kTotalHeapSizeExecutableIndex,
    kTotalPhysicalSizeIndex = _process$binding3.kTotalPhysicalSizeIndex,
    kTotalAvailableSize = _process$binding3.kTotalAvailableSize,
    kUsedHeapSizeIndex = _process$binding3.kUsedHeapSizeIndex,
    kHeapSizeLimitIndex = _process$binding3.kHeapSizeLimitIndex,
    kDoesZapGarbageIndex = _process$binding3.kDoesZapGarbageIndex,
    kMallocedMemoryIndex = _process$binding3.kMallocedMemoryIndex,
    kPeakMallocedMemoryIndex = _process$binding3.kPeakMallocedMemoryIndex,
    kHeapSpaces = _process$binding3.kHeapSpaces,
    kHeapSpaceStatisticsPropertiesCount = _process$binding3.kHeapSpaceStatisticsPropertiesCount,
    kSpaceSizeIndex = _process$binding3.kSpaceSizeIndex,
    kSpaceUsedSizeIndex = _process$binding3.kSpaceUsedSizeIndex,
    kSpaceAvailableSizeIndex = _process$binding3.kSpaceAvailableSizeIndex,
    kPhysicalSpaceSizeIndex = _process$binding3.kPhysicalSpaceSizeIndex;

var kNumberOfHeapSpaces = kHeapSpaces.length;
var heapStatisticsBuffer = new Float64Array(heapStatisticsArrayBuffer);
var heapSpaceStatisticsBuffer = new Float64Array(heapSpaceStatisticsArrayBuffer);

function getHeapStatistics() {
  var buffer = heapStatisticsBuffer;
  updateHeapStatisticsArrayBuffer();
  return {
    'total_heap_size': buffer[kTotalHeapSizeIndex],
    'total_heap_size_executable': buffer[kTotalHeapSizeExecutableIndex],
    'total_physical_size': buffer[kTotalPhysicalSizeIndex],
    'total_available_size': buffer[kTotalAvailableSize],
    'used_heap_size': buffer[kUsedHeapSizeIndex],
    'heap_size_limit': buffer[kHeapSizeLimitIndex],
    'malloced_memory': buffer[kMallocedMemoryIndex],
    'peak_malloced_memory': buffer[kPeakMallocedMemoryIndex],
    'does_zap_garbage': buffer[kDoesZapGarbageIndex]
  };
}

function getHeapSpaceStatistics() {
  var heapSpaceStatistics = new Array(kNumberOfHeapSpaces);
  var buffer = heapSpaceStatisticsBuffer;
  updateHeapSpaceStatisticsArrayBuffer();

  for (var i = 0; i < kNumberOfHeapSpaces; i++) {
    var propertyOffset = i * kHeapSpaceStatisticsPropertiesCount;
    heapSpaceStatistics[i] = {
      space_name: kHeapSpaces[i],
      space_size: buffer[propertyOffset + kSpaceSizeIndex],
      space_used_size: buffer[propertyOffset + kSpaceUsedSizeIndex],
      space_available_size: buffer[propertyOffset + kSpaceAvailableSizeIndex],
      physical_space_size: buffer[propertyOffset + kPhysicalSpaceSizeIndex]
    };
  }

  return heapSpaceStatistics;
}
/* V8 serialization API */

/* JS methods for the base objects */


Serializer.prototype._getDataCloneError = Error;

Deserializer.prototype.readRawBytes = function readRawBytes(length) {
  var offset = this._readRawBytes(length); // `this.buffer` can be a Buffer or a plain Uint8Array, so just calling
  // `.slice()` doesn't work.


  return new FastBuffer(this.buffer.buffer, this.buffer.byteOffset + offset, length);
};
/* Keep track of how to handle different ArrayBufferViews.
 * The default Serializer for Node does not use the V8 methods for serializing
 * those objects because Node's `Buffer` objects use pooled allocation in many
 * cases, and their underlying `ArrayBuffer`s would show up in the
 * serialization. Because a) those may contain sensitive data and the user
 * may not be aware of that and b) they are often much larger than the `Buffer`
 * itself, custom serialization is applied. */


var arrayBufferViewTypes = [Int8Array, Uint8Array, Uint8ClampedArray, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array, DataView];
var arrayBufferViewTypeToIndex = new Map();
{
  var dummy = new ArrayBuffer();
  var _iteratorNormalCompletion = true;
  var _didIteratorError = false;
  var _iteratorError = undefined;

  try {
    for (var _iterator = arrayBufferViewTypes.entries()[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
      var _step$value = _slicedToArray(_step.value, 2),
          i = _step$value[0],
          ctor = _step$value[1];

      var tag = objectToString(new ctor(dummy));
      arrayBufferViewTypeToIndex.set(tag, i);
    }
  } catch (err) {
    _didIteratorError = true;
    _iteratorError = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion && _iterator["return"] != null) {
        _iterator["return"]();
      }
    } finally {
      if (_didIteratorError) {
        throw _iteratorError;
      }
    }
  }
}
var bufferConstructorIndex = arrayBufferViewTypes.push(Buffer) - 1;

var DefaultSerializer =
/*#__PURE__*/
function (_Serializer3) {
  _inherits(DefaultSerializer, _Serializer3);

  function DefaultSerializer() {
    var _this;

    _classCallCheck(this, DefaultSerializer);

    _this = _possibleConstructorReturn(this, _getPrototypeOf(DefaultSerializer).call(this));

    _this._setTreatArrayBufferViewsAsHostObjects(true);

    return _this;
  }

  _createClass(DefaultSerializer, [{
    key: "_writeHostObject",
    value: function _writeHostObject(abView) {
      var i = 0;

      if (abView.constructor === Buffer) {
        i = bufferConstructorIndex;
      } else {
        var _tag = objectToString(abView);

        i = arrayBufferViewTypeToIndex.get(_tag);

        if (i === undefined) {
          throw new this._getDataCloneError("Unknown host object type: ".concat(_tag));
        }
      }

      this.writeUint32(i);
      this.writeUint32(abView.byteLength);
      this.writeRawBytes(new Uint8Array(abView.buffer, abView.byteOffset, abView.byteLength));
    }
  }]);

  return DefaultSerializer;
}(Serializer);

var DefaultDeserializer =
/*#__PURE__*/
function (_Deserializer3) {
  _inherits(DefaultDeserializer, _Deserializer3);

  function DefaultDeserializer(buffer) {
    _classCallCheck(this, DefaultDeserializer);

    return _possibleConstructorReturn(this, _getPrototypeOf(DefaultDeserializer).call(this, buffer));
  }

  _createClass(DefaultDeserializer, [{
    key: "_readHostObject",
    value: function _readHostObject() {
      var typeIndex = this.readUint32();
      var ctor = arrayBufferViewTypes[typeIndex];
      var byteLength = this.readUint32();

      var byteOffset = this._readRawBytes(byteLength);

      var BYTES_PER_ELEMENT = ctor.BYTES_PER_ELEMENT || 1;
      var offset = this.buffer.byteOffset + byteOffset;

      if (offset % BYTES_PER_ELEMENT === 0) {
        return new ctor(this.buffer.buffer, offset, byteLength / BYTES_PER_ELEMENT);
      } else {
        // Copy to an aligned buffer first.
        var buffer_copy = Buffer.allocUnsafe(byteLength);
        copy(this.buffer, buffer_copy, 0, byteOffset, byteOffset + byteLength);
        return new ctor(buffer_copy.buffer, buffer_copy.byteOffset, byteLength / BYTES_PER_ELEMENT);
      }
    }
  }]);

  return DefaultDeserializer;
}(Deserializer);

function serialize(value) {
  var ser = new DefaultSerializer();
  ser.writeHeader();
  ser.writeValue(value);
  return ser.releaseBuffer();
}

function deserialize(buffer) {
  var der = new DefaultDeserializer(buffer);
  der.readHeader();
  return der.readValue();
}

module.exports = exports = {
  cachedDataVersionTag: cachedDataVersionTag,
  getHeapStatistics: getHeapStatistics,
  getHeapSpaceStatistics: getHeapSpaceStatistics,
  setFlagsFromString: setFlagsFromString,
  Serializer: Serializer,
  Deserializer: Deserializer,
  DefaultSerializer: DefaultSerializer,
  DefaultDeserializer: DefaultDeserializer,
  deserialize: deserialize,
  serialize: serialize
};