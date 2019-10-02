// Originally from narwhal.js (http://narwhaljs.org)
// Copyright (c) 2009 Thomas Robinson <280north.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the 'Software'), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
'use strict';

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _process$binding = process.binding('buffer'),
    compare = _process$binding.compare;

var _process$binding2 = process.binding('util'),
    isSet = _process$binding2.isSet,
    isMap = _process$binding2.isMap,
    isDate = _process$binding2.isDate,
    isRegExp = _process$binding2.isRegExp;

var _require = require('internal/util'),
    objectToString = _require.objectToString;

var _require2 = require('internal/util/types'),
    isArrayBufferView = _require2.isArrayBufferView;

var errors = require('internal/errors');

var _require3 = require('util'),
    inspect = _require3.inspect; // The assert module provides functions that throw
// AssertionError's when particular conditions are not met. The
// assert module must conform to the following interface.


var assert = module.exports = ok; // All of the following functions must throw an AssertionError
// when a corresponding condition is not met, with a message that
// may be undefined if not provided. All assertion methods provide
// both the actual and expected values to the assertion error for
// display purposes.

function innerFail(obj) {
  throw new errors.AssertionError(obj);
}

function fail(actual, expected, message, operator, stackStartFn) {
  var argsLen = arguments.length;

  if (argsLen === 1) {
    message = actual;
  } else if (argsLen === 2) {
    operator = '!=';
  }

  innerFail({
    actual: actual,
    expected: expected,
    message: message,
    operator: operator,
    stackStartFn: stackStartFn || fail
  });
}

assert.fail = fail; // The AssertionError is defined in internal/error.
// new assert.AssertionError({ message: message,
//                             actual: actual,
//                             expected: expected });

assert.AssertionError = errors.AssertionError; // Pure assertion tests whether a value is truthy, as determined
// by !!value.

function ok(value, message) {
  if (!value) {
    innerFail({
      actual: value,
      expected: true,
      message: message,
      operator: '==',
      stackStartFn: ok
    });
  }
}

assert.ok = ok; // The equality assertion tests shallow, coercive equality with ==.

/* eslint-disable no-restricted-properties */

assert.equal = function equal(actual, expected, message) {
  // eslint-disable-next-line eqeqeq
  if (actual != expected) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: '==',
      stackStartFn: equal
    });
  }
}; // The non-equality assertion tests for whether two objects are not
// equal with !=.


assert.notEqual = function notEqual(actual, expected, message) {
  // eslint-disable-next-line eqeqeq
  if (actual == expected) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: '!=',
      stackStartFn: notEqual
    });
  }
}; // The equivalence assertion tests a deep equality relation.


assert.deepEqual = function deepEqual(actual, expected, message) {
  if (!innerDeepEqual(actual, expected, false)) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: 'deepEqual',
      stackStartFn: deepEqual
    });
  }
};
/* eslint-enable */


assert.deepStrictEqual = function deepStrictEqual(actual, expected, message) {
  if (!innerDeepEqual(actual, expected, true)) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: 'deepStrictEqual',
      stackStartFn: deepStrictEqual
    });
  }
}; // Check if they have the same source and flags


function areSimilarRegExps(a, b) {
  return a.source === b.source && a.flags === b.flags;
} // For small buffers it's faster to compare the buffer in a loop. The c++
// barrier including the Uint8Array operation takes the advantage of the faster
// binary compare otherwise. The break even point was at about 300 characters.


function areSimilarTypedArrays(a, b) {
  var len = a.byteLength;

  if (len !== b.byteLength) {
    return false;
  }

  if (len < 300) {
    for (var offset = 0; offset < len; offset++) {
      if (a[offset] !== b[offset]) {
        return false;
      }
    }

    return true;
  }

  return compare(new Uint8Array(a.buffer, a.byteOffset, len), new Uint8Array(b.buffer, b.byteOffset, b.byteLength)) === 0;
}

function isFloatTypedArrayTag(tag) {
  return tag === '[object Float32Array]' || tag === '[object Float64Array]';
}

function isArguments(tag) {
  return tag === '[object Arguments]';
}

function isObjectOrArrayTag(tag) {
  return tag === '[object Array]' || tag === '[object Object]';
} // Notes: Type tags are historical [[Class]] properties that can be set by
// FunctionTemplate::SetClassName() in C++ or Symbol.toStringTag in JS
// and retrieved using Object.prototype.toString.call(obj) in JS
// See https://tc39.github.io/ecma262/#sec-object.prototype.tostring
// for a list of tags pre-defined in the spec.
// There are some unspecified tags in the wild too (e.g. typed array tags).
// Since tags can be altered, they only serve fast failures
//
// Typed arrays and buffers are checked by comparing the content in their
// underlying ArrayBuffer. This optimization requires that it's
// reasonable to interpret their underlying memory in the same way,
// which is checked by comparing their type tags.
// (e.g. a Uint8Array and a Uint16Array with the same memory content
// could still be different because they will be interpreted differently)
// Never perform binary comparisons for Float*Arrays, though,
// since e.g. +0 === -0 is true despite the two values' bit patterns
// not being identical.
//
// For strict comparison, objects should have
// a) The same built-in type tags
// b) The same prototypes.


function strictDeepEqual(actual, expected) {
  if (actual === null || expected === null || _typeof(actual) !== 'object' || _typeof(expected) !== 'object') {
    return false;
  }

  var actualTag = objectToString(actual);
  var expectedTag = objectToString(expected);

  if (actualTag !== expectedTag) {
    return false;
  }

  if (Object.getPrototypeOf(actual) !== Object.getPrototypeOf(expected)) {
    return false;
  }

  if (actualTag === '[object Array]') {
    // Check for sparse arrays and general fast path
    if (actual.length !== expected.length) return false; // Skip testing the part below and continue in the callee function.

    return;
  }

  if (actualTag === '[object Object]') {
    // Skip testing the part below and continue in the callee function.
    return;
  }

  if (isDate(actual)) {
    if (actual.getTime() !== expected.getTime()) {
      return false;
    }
  } else if (isRegExp(actual)) {
    if (!areSimilarRegExps(actual, expected)) {
      return false;
    }
  } else if (actualTag === '[object Error]') {
    // Do not compare the stack as it might differ even though the error itself
    // is otherwise identical. The non-enumerable name should be identical as
    // the prototype is also identical. Otherwise this is caught later on.
    if (actual.message !== expected.message) {
      return false;
    }
  } else if (!isFloatTypedArrayTag(actualTag) && isArrayBufferView(actual)) {
    if (!areSimilarTypedArrays(actual, expected)) {
      return false;
    } // Buffer.compare returns true, so actual.length === expected.length
    // if they both only contain numeric keys, we don't need to exam further


    if (Object.keys(actual).length === actual.length && Object.keys(expected).length === expected.length) {
      return true;
    }
  } else if (typeof actual.valueOf === 'function') {
    var actualValue = actual.valueOf(); // Note: Boxed string keys are going to be compared again by Object.keys

    if (actualValue !== actual) {
      if (!innerDeepEqual(actualValue, expected.valueOf(), true)) return false; // Fast path for boxed primitives

      var lengthActual = 0;
      var lengthExpected = 0;

      if (typeof actualValue === 'string') {
        lengthActual = actual.length;
        lengthExpected = expected.length;
      }

      if (Object.keys(actual).length === lengthActual && Object.keys(expected).length === lengthExpected) {
        return true;
      }
    }
  }
}

function looseDeepEqual(actual, expected) {
  if (actual === null || _typeof(actual) !== 'object') {
    if (expected === null || _typeof(expected) !== 'object') {
      // eslint-disable-next-line eqeqeq
      return actual == expected;
    }

    return false;
  }

  if (expected === null || _typeof(expected) !== 'object') {
    return false;
  }

  if (isDate(actual) && isDate(expected)) {
    return actual.getTime() === expected.getTime();
  }

  if (isRegExp(actual) && isRegExp(expected)) {
    return areSimilarRegExps(actual, expected);
  }

  if (actual instanceof Error && expected instanceof Error) {
    if (actual.message !== expected.message || actual.name !== expected.name) return false;
  }

  var actualTag = objectToString(actual);
  var expectedTag = objectToString(expected);

  if (actualTag === expectedTag) {
    if (!isObjectOrArrayTag(actualTag) && !isFloatTypedArrayTag(actualTag) && isArrayBufferView(actual)) {
      return areSimilarTypedArrays(actual, expected);
    } // Ensure reflexivity of deepEqual with `arguments` objects.
    // See https://github.com/nodejs/node-v0.x-archive/pull/7178

  } else if (isArguments(actualTag) || isArguments(expectedTag)) {
    return false;
  }
}

function innerDeepEqual(actual, expected, strict, memos) {
  // All identical values are equivalent, as determined by ===.
  if (actual === expected) {
    return true;
  } // Returns a boolean if (not) equal and undefined in case we have to check
  // further.


  var partialCheck = strict ? strictDeepEqual(actual, expected) : looseDeepEqual(actual, expected);

  if (partialCheck !== undefined) {
    return partialCheck;
  } // For all remaining Object pairs, including Array, objects and Maps,
  // equivalence is determined by having:
  // a) The same number of owned enumerable properties
  // b) The same set of keys/indexes (although not necessarily the same order)
  // c) Equivalent values for every corresponding key/index
  // d) For Sets and Maps, equal contents
  // Note: this accounts for both named and indexed properties on Arrays.
  // Use memos to handle cycles.


  if (memos === undefined) {
    memos = {
      actual: new Map(),
      expected: new Map(),
      position: 0
    };
  } else {
    // We prevent up to two map.has(x) calls by directly retrieving the value
    // and checking for undefined. The map can only contain numbers, so it is
    // safe to check for undefined only.
    var expectedMemoA = memos.actual.get(actual);

    if (expectedMemoA !== undefined) {
      var expectedMemoB = memos.expected.get(expected);

      if (expectedMemoB !== undefined) {
        return expectedMemoA === expectedMemoB;
      }
    }

    memos.position++;
  }

  var aKeys = Object.keys(actual);
  var bKeys = Object.keys(expected);
  var i; // The pair must have the same number of owned properties
  // (keys incorporates hasOwnProperty).

  if (aKeys.length !== bKeys.length) return false; // Cheap key test:

  var keys = {};

  for (i = 0; i < aKeys.length; i++) {
    keys[aKeys[i]] = true;
  }

  for (i = 0; i < aKeys.length; i++) {
    if (keys[bKeys[i]] === undefined) return false;
  }

  memos.actual.set(actual, memos.position);
  memos.expected.set(expected, memos.position);
  var areEq = objEquiv(actual, expected, strict, aKeys, memos);
  memos.actual["delete"](actual);
  memos.expected["delete"](expected);
  return areEq;
}

function setHasEqualElement(set, val1, strict, memo) {
  // Go looking.
  var _iteratorNormalCompletion = true;
  var _didIteratorError = false;
  var _iteratorError = undefined;

  try {
    for (var _iterator = set[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
      var val2 = _step.value;

      if (innerDeepEqual(val1, val2, strict, memo)) {
        // Remove the matching element to make sure we do not check that again.
        set["delete"](val2);
        return true;
      }
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

  return false;
} // Note: we actually run this multiple times for each loose key!
// This is done to prevent slowing down the average case.


function setHasLoosePrim(a, b, val) {
  var altValues = findLooseMatchingPrimitives(val);
  if (altValues === undefined) return false;
  var matches = 1;

  for (var i = 0; i < altValues.length; i++) {
    if (b.has(altValues[i])) {
      matches--;
    }

    if (a.has(altValues[i])) {
      matches++;
    }
  }

  return matches === 0;
}

function setEquiv(a, b, strict, memo) {
  // This code currently returns false for this pair of sets:
  //   assert.deepEqual(new Set(['1', 1]), new Set([1]))
  //
  // In theory, all the items in the first set have a corresponding == value in
  // the second set, but the sets have different sizes. Its a silly case,
  // and more evidence that deepStrictEqual should always be preferred over
  // deepEqual.
  if (a.size !== b.size) return false; // This is a lazily initiated Set of entries which have to be compared
  // pairwise.

  var set = null;
  var _iteratorNormalCompletion2 = true;
  var _didIteratorError2 = false;
  var _iteratorError2 = undefined;

  try {
    for (var _iterator2 = a[Symbol.iterator](), _step2; !(_iteratorNormalCompletion2 = (_step2 = _iterator2.next()).done); _iteratorNormalCompletion2 = true) {
      var _val = _step2.value;

      // Note: Checking for the objects first improves the performance for object
      // heavy sets but it is a minor slow down for primitives. As they are fast
      // to check this improves the worst case scenario instead.
      if (_typeof(_val) === 'object' && _val !== null) {
        if (set === null) {
          set = new Set();
        } // If the specified value doesn't exist in the second set its an not null
        // object (or non strict only: a not matching primitive) we'll need to go
        // hunting for something thats deep-(strict-)equal to it. To make this
        // O(n log n) complexity we have to copy these values in a new set first.


        set.add(_val);
      } else if (!b.has(_val) && (strict || !setHasLoosePrim(a, b, _val))) {
        return false;
      }
    }
  } catch (err) {
    _didIteratorError2 = true;
    _iteratorError2 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion2 && _iterator2["return"] != null) {
        _iterator2["return"]();
      }
    } finally {
      if (_didIteratorError2) {
        throw _iteratorError2;
      }
    }
  }

  if (set !== null) {
    var _iteratorNormalCompletion3 = true;
    var _didIteratorError3 = false;
    var _iteratorError3 = undefined;

    try {
      for (var _iterator3 = b[Symbol.iterator](), _step3; !(_iteratorNormalCompletion3 = (_step3 = _iterator3.next()).done); _iteratorNormalCompletion3 = true) {
        var val = _step3.value;

        // We have to check if a primitive value is already
        // matching and only if it's not, go hunting for it.
        if (_typeof(val) === 'object' && val !== null) {
          if (!setHasEqualElement(set, val, strict, memo)) return false;
        } else if (!a.has(val) && (strict || !setHasLoosePrim(b, a, val))) {
          return false;
        }
      }
    } catch (err) {
      _didIteratorError3 = true;
      _iteratorError3 = err;
    } finally {
      try {
        if (!_iteratorNormalCompletion3 && _iterator3["return"] != null) {
          _iterator3["return"]();
        }
      } finally {
        if (_didIteratorError3) {
          throw _iteratorError3;
        }
      }
    }
  }

  return true;
}

function findLooseMatchingPrimitives(prim) {
  var values, number;

  switch (_typeof(prim)) {
    case 'number':
      values = ['' + prim];
      if (prim === 1 || prim === 0) values.push(Boolean(prim));
      return values;

    case 'string':
      number = +prim;

      if ('' + number === prim) {
        values = [number];
        if (number === 1 || number === 0) values.push(Boolean(number));
      }

      return values;

    case 'undefined':
      return [null];

    case 'object':
      // Only pass in null as object!
      return [undefined];

    case 'boolean':
      number = +prim;
      return [number, '' + number];
  }
} // This is a ugly but relatively fast way to determine if a loose equal entry
// actually has a correspondent matching entry. Otherwise checking for such
// values would be way more expensive (O(n^2)).
// Note: we actually run this multiple times for each loose key!
// This is done to prevent slowing down the average case.


function mapHasLoosePrim(a, b, key1, memo, item1, item2) {
  var altKeys = findLooseMatchingPrimitives(key1);
  if (altKeys === undefined) return false;
  var setA = new Set();
  var setB = new Set();
  var keyCount = 1;
  setA.add(item1);

  if (b.has(key1)) {
    keyCount--;
    setB.add(item2);
  }

  for (var i = 0; i < altKeys.length; i++) {
    var key2 = altKeys[i];

    if (a.has(key2)) {
      keyCount++;
      setA.add(a.get(key2));
    }

    if (b.has(key2)) {
      keyCount--;
      setB.add(b.get(key2));
    }
  }

  if (keyCount !== 0 || setA.size !== setB.size) return false;
  var _iteratorNormalCompletion4 = true;
  var _didIteratorError4 = false;
  var _iteratorError4 = undefined;

  try {
    for (var _iterator4 = setA[Symbol.iterator](), _step4; !(_iteratorNormalCompletion4 = (_step4 = _iterator4.next()).done); _iteratorNormalCompletion4 = true) {
      var val = _step4.value;

      if (_typeof(val) === 'object' && val !== null) {
        if (!setHasEqualElement(setB, val, false, memo)) return false;
      } else if (!setB.has(val) && !setHasLoosePrim(setA, setB, val)) {
        return false;
      }
    }
  } catch (err) {
    _didIteratorError4 = true;
    _iteratorError4 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion4 && _iterator4["return"] != null) {
        _iterator4["return"]();
      }
    } finally {
      if (_didIteratorError4) {
        throw _iteratorError4;
      }
    }
  }

  return true;
}

function mapHasEqualEntry(set, map, key1, item1, strict, memo) {
  // To be able to handle cases like:
  //   Map([[{}, 'a'], [{}, 'b']]) vs Map([[{}, 'b'], [{}, 'a']])
  // ... we need to consider *all* matching keys, not just the first we find.
  var _iteratorNormalCompletion5 = true;
  var _didIteratorError5 = false;
  var _iteratorError5 = undefined;

  try {
    for (var _iterator5 = set[Symbol.iterator](), _step5; !(_iteratorNormalCompletion5 = (_step5 = _iterator5.next()).done); _iteratorNormalCompletion5 = true) {
      var key2 = _step5.value;

      if (innerDeepEqual(key1, key2, strict, memo) && innerDeepEqual(item1, map.get(key2), strict, memo)) {
        set["delete"](key2);
        return true;
      }
    }
  } catch (err) {
    _didIteratorError5 = true;
    _iteratorError5 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion5 && _iterator5["return"] != null) {
        _iterator5["return"]();
      }
    } finally {
      if (_didIteratorError5) {
        throw _iteratorError5;
      }
    }
  }

  return false;
}

function mapEquiv(a, b, strict, memo) {
  if (a.size !== b.size) return false;
  var set = null;
  var _iteratorNormalCompletion6 = true;
  var _didIteratorError6 = false;
  var _iteratorError6 = undefined;

  try {
    for (var _iterator6 = a[Symbol.iterator](), _step6; !(_iteratorNormalCompletion6 = (_step6 = _iterator6.next()).done); _iteratorNormalCompletion6 = true) {
      var _step6$value = _slicedToArray(_step6.value, 2),
          key = _step6$value[0],
          item1 = _step6$value[1];

      if (_typeof(key) === 'object' && key !== null) {
        if (set === null) {
          set = new Set();
        }

        set.add(key);
      } else {
        // By directly retrieving the value we prevent another b.has(key) check in
        // almost all possible cases.
        var item2 = b.get(key);

        if ((item2 === undefined && !b.has(key) || !innerDeepEqual(item1, item2, strict, memo)) && (strict || !mapHasLoosePrim(a, b, key, memo, item1, item2))) {
          return false;
        }
      }
    }
  } catch (err) {
    _didIteratorError6 = true;
    _iteratorError6 = err;
  } finally {
    try {
      if (!_iteratorNormalCompletion6 && _iterator6["return"] != null) {
        _iterator6["return"]();
      }
    } finally {
      if (_didIteratorError6) {
        throw _iteratorError6;
      }
    }
  }

  if (set !== null) {
    var _iteratorNormalCompletion7 = true;
    var _didIteratorError7 = false;
    var _iteratorError7 = undefined;

    try {
      for (var _iterator7 = b[Symbol.iterator](), _step7; !(_iteratorNormalCompletion7 = (_step7 = _iterator7.next()).done); _iteratorNormalCompletion7 = true) {
        var _step7$value = _slicedToArray(_step7.value, 2),
            key = _step7$value[0],
            item = _step7$value[1];

        if (_typeof(key) === 'object' && key !== null) {
          if (!mapHasEqualEntry(set, a, key, item, strict, memo)) return false;
        } else if (!a.has(key) && (strict || !mapHasLoosePrim(b, a, key, memo, item))) {
          return false;
        }
      }
    } catch (err) {
      _didIteratorError7 = true;
      _iteratorError7 = err;
    } finally {
      try {
        if (!_iteratorNormalCompletion7 && _iterator7["return"] != null) {
          _iterator7["return"]();
        }
      } finally {
        if (_didIteratorError7) {
          throw _iteratorError7;
        }
      }
    }
  }

  return true;
}

function objEquiv(a, b, strict, keys, memos) {
  // Sets and maps don't have their entries accessible via normal object
  // properties.
  if (isSet(a)) {
    if (!isSet(b) || !setEquiv(a, b, strict, memos)) return false;
  } else if (isMap(a)) {
    if (!isMap(b) || !mapEquiv(a, b, strict, memos)) return false;
  } else if (isSet(b) || isMap(b)) {
    return false;
  } // The pair must have equivalent values for every corresponding key.
  // Possibly expensive deep test:


  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if (!innerDeepEqual(a[key], b[key], strict, memos)) return false;
  }

  return true;
} // The non-equivalence assertion tests for any deep inequality.


assert.notDeepEqual = function notDeepEqual(actual, expected, message) {
  if (innerDeepEqual(actual, expected, false)) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: 'notDeepEqual',
      stackStartFn: notDeepEqual
    });
  }
};

assert.notDeepStrictEqual = notDeepStrictEqual;

function notDeepStrictEqual(actual, expected, message) {
  if (innerDeepEqual(actual, expected, true)) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: 'notDeepStrictEqual',
      stackStartFn: notDeepStrictEqual
    });
  }
} // The strict equality assertion tests strict equality, as determined by ===.


assert.strictEqual = function strictEqual(actual, expected, message) {
  if (actual !== expected) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: '===',
      stackStartFn: strictEqual
    });
  }
}; // The strict non-equality assertion tests for strict inequality, as
// determined by !==.


assert.notStrictEqual = function notStrictEqual(actual, expected, message) {
  if (actual === expected) {
    innerFail({
      actual: actual,
      expected: expected,
      message: message,
      operator: '!==',
      stackStartFn: notStrictEqual
    });
  }
};

function compareExceptionKey(actual, expected, key, msg) {
  if (!innerDeepEqual(actual[key], expected[key], true)) {
    innerFail({
      actual: actual[key],
      expected: expected[key],
      message: msg || "".concat(key, ": expected ").concat(inspect(expected[key]), ", ") + "not ".concat(inspect(actual[key])),
      operator: 'throws',
      stackStartFn: assert["throws"]
    });
  }
}

function expectedException(actual, expected, msg) {
  if (typeof expected !== 'function') {
    if (expected instanceof RegExp) return expected.test(actual); // assert.doesNotThrow does not accept objects.

    if (arguments.length === 2) {
      throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'expected', ['Function', 'RegExp'], expected);
    } // The name and message could be non enumerable. Therefore test them
    // explicitly.


    if ('name' in expected) {
      compareExceptionKey(actual, expected, 'name', msg);
    }

    if ('message' in expected) {
      compareExceptionKey(actual, expected, 'message', msg);
    }

    for (var _i2 = 0, _Object$keys = Object.keys(expected); _i2 < _Object$keys.length; _i2++) {
      var key = _Object$keys[_i2];
      compareExceptionKey(actual, expected, key, msg);
    }

    return true;
  } // Guard instanceof against arrow functions as they don't have a prototype.


  if (expected.prototype !== undefined && actual instanceof expected) {
    return true;
  }

  if (Error.isPrototypeOf(expected)) {
    return false;
  }

  return expected.call({}, actual) === true;
}

function getActual(block) {
  if (typeof block !== 'function') {
    throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'block', 'Function', block);
  }

  try {
    block();
  } catch (e) {
    return e;
  }
}

function waitForActual(_x) {
  return _waitForActual.apply(this, arguments);
} // Expected to throw an error.


function _waitForActual() {
  _waitForActual = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee(block) {
    var resultPromise;
    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            if (!(typeof block !== 'function')) {
              _context.next = 2;
              break;
            }

            throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'block', 'Function', block);

          case 2:
            // Return a rejected promise if `block` throws synchronously.
            resultPromise = block();
            _context.prev = 3;
            _context.next = 6;
            return resultPromise;

          case 6:
            _context.next = 11;
            break;

          case 8:
            _context.prev = 8;
            _context.t0 = _context["catch"](3);
            return _context.abrupt("return", _context.t0);

          case 11:
            return _context.abrupt("return", errors.NO_EXCEPTION_SENTINEL);

          case 12:
          case "end":
            return _context.stop();
        }
      }
    }, _callee, null, [[3, 8]]);
  }));
  return _waitForActual.apply(this, arguments);
}

function expectsError(stackStartFn, actual, error, message) {
  if (typeof error === 'string') {
    if (arguments.length === 4) {
      throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'error', ['Function', 'RegExp'], error);
    }

    message = error;
    error = null;
  }

  if (actual === undefined) {
    var details = '';

    if (error && error.name) {
      details += " (".concat(error.name, ")");
    }

    details += message ? ": ".concat(message) : '.';
    var fnType = stackStartFn === rejects ? 'rejection' : 'exception';
    innerFail({
      actual: actual,
      expected: error,
      operator: stackStartFn.name,
      message: "Missing expected ".concat(fnType).concat(details),
      stackStartFn: stackStartFn
    });
  }

  if (error && expectedException(actual, error, message) === false) {
    throw actual;
  }
}

function expectsNoError(stackStartFn, actual, error, message) {
  if (actual === undefined) return;

  if (typeof error === 'string') {
    message = error;
    error = null;
  }

  if (!error || expectedException(actual, error)) {
    var details = message ? ": ".concat(message) : '.';
    var fnType = stackStartFn === doesNotReject ? 'rejection' : 'exception';
    innerFail({
      actual: actual,
      expected: error,
      operator: stackStartFn.name,
      message: "Got unwanted ".concat(fnType).concat(details),
      stackStartFn: stackStartFn
    });
  }

  throw actual;
}

function _throws(block) {
  for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
    args[_key - 1] = arguments[_key];
  }

  expectsError.apply(void 0, [_throws, getActual(block)].concat(args));
}

assert["throws"] = _throws;

function rejects(_x2) {
  return _rejects.apply(this, arguments);
}

function _rejects() {
  _rejects = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee2(block) {
    var _len2,
        args,
        _key2,
        _args2 = arguments;

    return regeneratorRuntime.wrap(function _callee2$(_context2) {
      while (1) {
        switch (_context2.prev = _context2.next) {
          case 0:
            for (_len2 = _args2.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
              args[_key2 - 1] = _args2[_key2];
            }

            _context2.t0 = expectsError;
            _context2.t1 = void 0;
            _context2.t2 = rejects;
            _context2.next = 6;
            return waitForActual(block);

          case 6:
            _context2.t3 = _context2.sent;
            _context2.t4 = args;
            _context2.t5 = [_context2.t2, _context2.t3].concat(_context2.t4);

            _context2.t0.apply.call(_context2.t0, _context2.t1, _context2.t5);

          case 10:
          case "end":
            return _context2.stop();
        }
      }
    }, _callee2);
  }));
  return _rejects.apply(this, arguments);
}

assert.rejects = rejects;

function doesNotThrow(block) {
  for (var _len3 = arguments.length, args = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
    args[_key3 - 1] = arguments[_key3];
  }

  expectsNoError.apply(void 0, [doesNotThrow, getActual(block)].concat(args));
}

assert.doesNotThrow = doesNotThrow;

function doesNotReject(_x3) {
  return _doesNotReject.apply(this, arguments);
}

function _doesNotReject() {
  _doesNotReject = _asyncToGenerator(
  /*#__PURE__*/
  regeneratorRuntime.mark(function _callee3(block) {
    var _len4,
        args,
        _key4,
        _args3 = arguments;

    return regeneratorRuntime.wrap(function _callee3$(_context3) {
      while (1) {
        switch (_context3.prev = _context3.next) {
          case 0:
            for (_len4 = _args3.length, args = new Array(_len4 > 1 ? _len4 - 1 : 0), _key4 = 1; _key4 < _len4; _key4++) {
              args[_key4 - 1] = _args3[_key4];
            }

            _context3.t0 = expectsNoError;
            _context3.t1 = void 0;
            _context3.t2 = doesNotReject;
            _context3.next = 6;
            return waitForActual(block);

          case 6:
            _context3.t3 = _context3.sent;
            _context3.t4 = args;
            _context3.t5 = [_context3.t2, _context3.t3].concat(_context3.t4);

            _context3.t0.apply.call(_context3.t0, _context3.t1, _context3.t5);

          case 10:
          case "end":
            return _context3.stop();
        }
      }
    }, _callee3);
  }));
  return _doesNotReject.apply(this, arguments);
}

assert.doesNotReject = doesNotReject;

assert.ifError = function ifError(err) {
  if (err) throw err;
}; // Expose a strict only variant of assert


function strict(value, message) {
  if (!value) {
    innerFail({
      actual: value,
      expected: true,
      message: message,
      operator: '==',
      stackStartFn: strict
    });
  }
}

assert.strict = Object.assign(strict, assert, {
  equal: assert.strictEqual,
  deepEqual: assert.deepStrictEqual,
  notEqual: assert.notStrictEqual,
  notDeepEqual: assert.notDeepStrictEqual
});
assert.strict.strict = assert.strict;