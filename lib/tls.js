// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
'use strict';

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

var internalUtil = require('internal/util');

internalUtil.assertCrypto();

var _require = require('internal/util/types'),
    isUint8Array = _require.isUint8Array;

var net = require('net');

var url = require('url');

var binding = process.binding('crypto');

var Buffer = require('buffer').Buffer;

var canonicalizeIP = process.binding('cares_wrap').canonicalizeIP; // Allow {CLIENT_RENEG_LIMIT} client-initiated session renegotiations
// every {CLIENT_RENEG_WINDOW} seconds. An error event is emitted if more
// renegotiations are seen. The settings are applied to all remote client
// connections.

exports.CLIENT_RENEG_LIMIT = 3;
exports.CLIENT_RENEG_WINDOW = 600;
exports.SLAB_BUFFER_SIZE = 10 * 1024 * 1024;
exports.DEFAULT_CIPHERS = process.binding('constants').crypto.defaultCipherList;
exports.DEFAULT_ECDH_CURVE = 'prime256v1';
exports.getCiphers = internalUtil.cachedResult(function () {
  return internalUtil.filterDuplicateStrings(binding.getSSLCiphers(), true);
}); // Convert protocols array into valid OpenSSL protocols list
// ("\x06spdy/2\x08http/1.1\x08http/1.0")

function convertProtocols(protocols) {
  var lens = new Array(protocols.length);
  var buff = Buffer.allocUnsafe(protocols.reduce(function (p, c, i) {
    var len = Buffer.byteLength(c);
    lens[i] = len;
    return p + 1 + len;
  }, 0));
  var offset = 0;

  for (var i = 0, c = protocols.length; i < c; i++) {
    buff[offset++] = lens[i];
    buff.write(protocols[i], offset);
    offset += lens[i];
  }

  return buff;
}

exports.convertNPNProtocols = function (protocols, out) {
  // If protocols is Array - translate it into buffer
  if (Array.isArray(protocols)) {
    out.NPNProtocols = convertProtocols(protocols);
  } else if (isUint8Array(protocols)) {
    // Copy new buffer not to be modified by user.
    out.NPNProtocols = Buffer.from(protocols);
  }
};

exports.convertALPNProtocols = function (protocols, out) {
  // If protocols is Array - translate it into buffer
  if (Array.isArray(protocols)) {
    out.ALPNProtocols = convertProtocols(protocols);
  } else if (isUint8Array(protocols)) {
    // Copy new buffer not to be modified by user.
    out.ALPNProtocols = Buffer.from(protocols);
  }
};

function unfqdn(host) {
  return host.replace(/[.]$/, '');
}

function splitHost(host) {
  // String#toLowerCase() is locale-sensitive so we use
  // a conservative version that only lowercases A-Z.
  var replacer = function replacer(c) {
    return String.fromCharCode(32 + c.charCodeAt(0));
  };

  return unfqdn(host).replace(/[A-Z]/g, replacer).split('.');
}

function check(hostParts, pattern, wildcards) {
  // Empty strings, null, undefined, etc. never match.
  if (!pattern) return false;
  var patternParts = splitHost(pattern);
  if (hostParts.length !== patternParts.length) return false; // Pattern has empty components, e.g. "bad..example.com".

  if (patternParts.includes('')) return false; // RFC 6125 allows IDNA U-labels (Unicode) in names but we have no
  // good way to detect their encoding or normalize them so we simply
  // reject them.  Control characters and blanks are rejected as well
  // because nothing good can come from accepting them.

  var isBad = function isBad(s) {
    return /(?:[\0- \x80-\uD7FF\uE000-\uFFFF]|[\uD800-\uDBFF][\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])|(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF])/.test(s);
  };

  if (patternParts.some(isBad)) return false; // Check host parts from right to left first.

  for (var i = hostParts.length - 1; i > 0; i -= 1) {
    if (hostParts[i] !== patternParts[i]) return false;
  }

  var hostSubdomain = hostParts[0];
  var patternSubdomain = patternParts[0];
  var patternSubdomainParts = patternSubdomain.split('*'); // Short-circuit when the subdomain does not contain a wildcard.
  // RFC 6125 does not allow wildcard substitution for components
  // containing IDNA A-labels (Punycode) so match those verbatim.

  if (patternSubdomainParts.length === 1 || patternSubdomain.includes('xn--')) return hostSubdomain === patternSubdomain;
  if (!wildcards) return false; // More than one wildcard is always wrong.

  if (patternSubdomainParts.length > 2) return false; // *.tld wildcards are not allowed.

  if (patternParts.length <= 2) return false;

  var _patternSubdomainPart = _slicedToArray(patternSubdomainParts, 2),
      prefix = _patternSubdomainPart[0],
      suffix = _patternSubdomainPart[1];

  if (prefix.length + suffix.length > hostSubdomain.length) return false;
  if (!hostSubdomain.startsWith(prefix)) return false;
  if (!hostSubdomain.endsWith(suffix)) return false;
  return true;
}

exports.checkServerIdentity = function checkServerIdentity(host, cert) {
  var subject = cert.subject;
  var altNames = cert.subjectaltname;
  var dnsNames = [];
  var uriNames = [];
  var ips = [];
  host = '' + host;

  if (altNames) {
    var _iteratorNormalCompletion = true;
    var _didIteratorError = false;
    var _iteratorError = undefined;

    try {
      for (var _iterator = altNames.split(', ')[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
        var name = _step.value;

        if (name.startsWith('DNS:')) {
          dnsNames.push(name.slice(4));
        } else if (name.startsWith('URI:')) {
          var uri = url.parse(name.slice(4));
          uriNames.push(uri.hostname); // TODO(bnoordhuis) Also use scheme.
        } else if (name.startsWith('IP Address:')) {
          ips.push(canonicalizeIP(name.slice(11)));
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
  }

  var valid = false;
  var reason = 'Unknown reason';

  if (net.isIP(host)) {
    valid = ips.includes(canonicalizeIP(host));
    if (!valid) reason = "IP: ".concat(host, " is not in the cert's list: ").concat(ips.join(', ')); // TODO(bnoordhuis) Also check URI SANs that are IP addresses.
  } else if (subject) {
    host = unfqdn(host); // Remove trailing dot for error messages.

    var hostParts = splitHost(host);

    var wildcard = function wildcard(pattern) {
      return check(hostParts, pattern, true);
    };

    var noWildcard = function noWildcard(pattern) {
      return check(hostParts, pattern, false);
    }; // Match against Common Name only if no supported identifiers are present.


    if (dnsNames.length === 0 && ips.length === 0 && uriNames.length === 0) {
      var cn = subject.CN;
      if (Array.isArray(cn)) valid = cn.some(wildcard);else if (cn) valid = wildcard(cn);
      if (!valid) reason = "Host: ".concat(host, ". is not cert's CN: ").concat(cn);
    } else {
      valid = dnsNames.some(wildcard) || uriNames.some(noWildcard);
      if (!valid) reason = "Host: ".concat(host, ". is not in the cert's altnames: ").concat(altNames);
    }
  } else {
    reason = 'Cert is empty';
  }

  if (!valid) {
    var err = new Error("Hostname/IP doesn't match certificate's altnames: \"".concat(reason, "\""));
    err.reason = reason;
    err.host = host;
    err.cert = cert;
    return err;
  }
}; // Example:
// C=US\nST=CA\nL=SF\nO=Joyent\nOU=Node.js\nCN=ca1\nemailAddress=ry@clouds.org


exports.parseCertString = function parseCertString(s) {
  var out = {};
  var parts = s.split('\n');

  for (var i = 0, len = parts.length; i < len; i++) {
    var sepIndex = parts[i].indexOf('=');

    if (sepIndex > 0) {
      var key = parts[i].slice(0, sepIndex);
      var value = parts[i].slice(sepIndex + 1);

      if (key in out) {
        if (!Array.isArray(out[key])) {
          out[key] = [out[key]];
        }

        out[key].push(value);
      } else {
        out[key] = value;
      }
    }
  }

  return out;
};

exports.createSecureContext = require('_tls_common').createSecureContext;
exports.SecureContext = require('_tls_common').SecureContext;
exports.TLSSocket = require('_tls_wrap').TLSSocket;
exports.Server = require('_tls_wrap').Server;
exports.createServer = require('_tls_wrap').createServer;
exports.connect = require('_tls_wrap').connect; // Deprecated: DEP0064

exports.createSecurePair = require('_tls_legacy').createSecurePair;