// Copyright JS Foundation and other contributors, http://js.foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var obj = {};

// Checking behavior with normal inputs
var array = ["foo", 1, "bar", obj, 2, "baz"];
assert(array.copyWithin(2,0,6).toString() === "foo,1,foo,1,bar,[object Object]");
assert(array.copyWithin(0,1,3).toString() === "1,foo,foo,1,bar,[object Object]");
assert(array.copyWithin(3,0,4).toString() === "1,foo,foo,1,foo,foo");

// Checking behavior with default inputs
var array = ["foo", 1, "bar", obj, 2, "baz"];
assert(array.copyWithin().toString() === "foo,1,bar,[object Object],2,baz");
assert(array.copyWithin(2).toString() === "foo,1,foo,1,bar,[object Object]");
assert(array.copyWithin(1,4).toString() === "foo,bar,[object Object],1,bar,[object Object]");

// Checking behavior when argument is negative or bigger then length
var array = ["foo", 1, "bar", obj, 2, "baz"];
assert(array.copyWithin(12,3,-3).toString() === "foo,1,bar,[object Object],2,baz");
assert(array.copyWithin(-2,-4,3).toString() === "foo,1,bar,[object Object],bar,baz");
assert(array.copyWithin(1,-5,30).toString() === "foo,1,bar,[object Object],bar,baz");

// Checking behavior with undefined, NaN, +/- Infinity
var array = ["foo", 1, "bar", obj, 2, "baz"];
assert(array.copyWithin(undefined).toString() === "foo,1,bar,[object Object],2,baz");
assert(array.copyWithin(2, NaN).toString()=== "foo,1,foo,1,bar,[object Object]");
assert(array.copyWithin(2,undefined,5).toString() === "foo,1,foo,1,foo,1");

var array = ["foo", 1, "bar", obj, 2, "baz"];
assert(array.copyWithin(Infinity,2,NaN).toString() === "foo,1,bar,[object Object],2,baz");
assert(array.copyWithin(Infinity,-Infinity,4).toString()=== "foo,1,bar,[object Object],2,baz");
assert(array.copyWithin(NaN,0,3).toString() === "foo,1,bar,[object Object],2,baz");

// Checking behavior when no length property defined
var obj = { copyWithin : Array.prototype.copyWithin };

obj.copyWithin();
assert(obj.length === undefined);

// Checking behavior when unable to get length
var obj = { copyWithin : Array.prototype.copyWithin };
Object.defineProperty(obj, 'length', { 'get' : function () {throw new ReferenceError ("foo"); } });

try {
  obj.copyWithin(1);
  assert(false)
} catch (e) {
  assert(e.message === "foo");
  assert(e instanceof ReferenceError);
}

// Checking behavior when unable to get element
var obj = { copyWithin : Array.prototype.copyWithin, length : 5 };
Object.defineProperty(obj, '2', { 'get' : function () {throw new ReferenceError ("foo"); } });

try {
  obj.copyWithin(2);
  assert(false);
} catch (e) {
  assert(e.message === "foo");
  assert(e instanceof ReferenceError);
}

// Checking behavior when a property is not defined
var obj = { '0' : 2, '2' : "foo", length : 3, copyWithin : Array.prototype.copyWithin };

obj.copyWithin(1);
assert(obj[0] === 2);
assert(obj[1] === 2);
