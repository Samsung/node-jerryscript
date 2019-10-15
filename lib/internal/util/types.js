'use strict';

var ReflectApply = Function.prototype.apply;
//Reflect.apply; // This function is borrowed from the function with the same name on V8 Extras'
// `utils` object. V8 implements Reflect.apply very efficiently in conjunction
// with the spread syntax, such that no additional special case is needed for
// function calls w/o arguments.
// Refs: https://github.com/v8/v8/blob/d6ead37d265d7215cf9c5f768f279e21bd170212/src/js/prologue.js#L152-L156

function uncurryThis(func) {
  return function (thisArg, ...args) { return ReflectApply.call(func, thisArg, args);  };
}

var TypedArrayPrototype = Object.getPrototypeOf(Uint8Array.prototype);
var TypedArrayProto_toStringTag = uncurryThis(Object.getOwnPropertyDescriptor(TypedArrayPrototype, Symbol.toStringTag).get); // Cached to make sure no userland code can tamper with it.

var isArrayBufferView = ArrayBuffer.isView;

function isTypedArray(value) {
  return TypedArrayProto_toStringTag(value) !== undefined;
}

function isUint8Array(value) {
  return TypedArrayProto_toStringTag(value) === 'Uint8Array';
}

module.exports = {
  isArrayBufferView: isArrayBufferView,
  isTypedArray: isTypedArray,
  isUint8Array: isUint8Array
};