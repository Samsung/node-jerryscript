'use strict';

function getCIDRSuffix(mask) {
  var protocol = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'ipv4';
  var isV6 = protocol === 'ipv6';
  var bitsString = mask.split(isV6 ? ':' : '.').filter(function (v) {
    return !!v;
  }).map(function (v) {
    return pad(parseInt(v, isV6 ? 16 : 10).toString(2), isV6);
  }).join('');

  if (isValidMask(bitsString)) {
    return countOnes(bitsString);
  } else {
    return null;
  }
}

function pad(binaryString, isV6) {
  var groupLength = isV6 ? 16 : 8;
  var binLen = binaryString.length;
  return binLen < groupLength ? "".concat('0'.repeat(groupLength - binLen)).concat(binaryString) : binaryString;
}

function isValidMask(bitsString) {
  var firstIndexOfZero = bitsString.indexOf(0);
  var lastIndexOfOne = bitsString.lastIndexOf(1);
  return firstIndexOfZero < 0 || firstIndexOfZero > lastIndexOfOne;
}

function countOnes(bitsString) {
  return bitsString.split('').reduce(function (acc, bit) {
    return acc += parseInt(bit, 10);
  }, 0);
}

module.exports = {
  getCIDRSuffix: getCIDRSuffix
};