'use strict';

if (!process.binding('config').hasIntl) {
  return;
}

var normalizeEncoding = require('internal/util').normalizeEncoding;
var Buffer = require('buffer').Buffer;

var icu = process.binding('icu');
var isUint8Array = require('internal/util/types').isUint8Array;

// Transcodes the Buffer from one encoding to another, returning a new
// Buffer instance.
function transcode(source, fromEncoding, toEncoding) {
  if (!isUint8Array(source))
    throw new TypeError('"source" argument must be a Buffer or Uint8Array');
  if (source.length === 0) return Buffer.alloc(0);

  fromEncoding = normalizeEncoding(fromEncoding) || fromEncoding;
  toEncoding = normalizeEncoding(toEncoding) || toEncoding;
  var result = icu.transcode(source, fromEncoding, toEncoding);
  if (typeof result !== 'number')
    return result;

  var code = icu.icuErrName(result);
  var err = new Error(`Unable to transcode Buffer [${code}]`);
  err.code = code;
  err.errno = result;
  throw err;
}

module.exports = {
  transcode
};
