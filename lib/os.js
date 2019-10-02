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

var _process$binding = process.binding('util'),
    pushValToArrayMax = _process$binding.pushValToArrayMax;

var constants = process.binding('constants').os;

var _require = require('internal/util'),
    deprecate = _require.deprecate;

var _require2 = require('internal/os'),
    getCIDRSuffix = _require2.getCIDRSuffix;

var isWindows = process.platform === 'win32';

var _process$binding2 = process.binding('os'),
    getCPUs = _process$binding2.getCPUs,
    getFreeMem = _process$binding2.getFreeMem,
    getHomeDirectory = _process$binding2.getHomeDirectory,
    getHostname = _process$binding2.getHostname,
    getInterfaceAddresses = _process$binding2.getInterfaceAddresses,
    getLoadAvg = _process$binding2.getLoadAvg,
    getOSRelease = _process$binding2.getOSRelease,
    getOSType = _process$binding2.getOSType,
    getTotalMem = _process$binding2.getTotalMem,
    getUserInfo = _process$binding2.getUserInfo,
    getUptime = _process$binding2.getUptime,
    isBigEndian = _process$binding2.isBigEndian;

getFreeMem[Symbol.toPrimitive] = function () {
  return getFreeMem();
};

getHostname[Symbol.toPrimitive] = function () {
  return getHostname();
};

getHomeDirectory[Symbol.toPrimitive] = function () {
  return getHomeDirectory();
};

getOSRelease[Symbol.toPrimitive] = function () {
  return getOSRelease();
};

getOSType[Symbol.toPrimitive] = function () {
  return getOSType();
};

getTotalMem[Symbol.toPrimitive] = function () {
  return getTotalMem();
};

getUptime[Symbol.toPrimitive] = function () {
  return getUptime();
};

var kEndianness = isBigEndian ? 'BE' : 'LE';
var tmpDirDeprecationMsg = 'os.tmpDir() is deprecated. Use os.tmpdir() instead.';
var getNetworkInterfacesDepMsg = 'os.getNetworkInterfaces is deprecated. Use os.networkInterfaces instead.';
var avgValues = new Float64Array(3);
var cpuValues = new Float64Array(6 * pushValToArrayMax);

function loadavg() {
  getLoadAvg(avgValues);
  return [avgValues[0], avgValues[1], avgValues[2]];
}

function addCPUInfo() {
  for (var i = 0, c = 0; i < arguments.length; ++i, c += 6) {
    this[this.length] = {
      model: arguments[i],
      speed: cpuValues[c],
      times: {
        user: cpuValues[c + 1],
        nice: cpuValues[c + 2],
        sys: cpuValues[c + 3],
        idle: cpuValues[c + 4],
        irq: cpuValues[c + 5]
      }
    };
  }
}

function cpus() {
  return getCPUs(addCPUInfo, cpuValues, []);
}

function arch() {
  return process.arch;
}

arch[Symbol.toPrimitive] = function () {
  return process.arch;
};

function platform() {
  return process.platform;
}

platform[Symbol.toPrimitive] = function () {
  return process.platform;
};

function tmpdir() {
  var path;

  if (isWindows) {
    path = process.env.TEMP || process.env.TMP || (process.env.SystemRoot || process.env.windir) + '\\temp';
    if (path.length > 1 && path.endsWith('\\') && !path.endsWith(':\\')) path = path.slice(0, -1);
  } else {
    path = process.env.TMPDIR || process.env.TMP || process.env.TEMP || '/tmp';
    if (path.length > 1 && path.endsWith('/')) path = path.slice(0, -1);
  }

  return path;
}

tmpdir[Symbol.toPrimitive] = function () {
  return tmpdir();
};

function endianness() {
  return kEndianness;
}

endianness[Symbol.toPrimitive] = function () {
  return kEndianness;
};

function networkInterfaces() {
  var interfaceAddresses = getInterfaceAddresses();
  return Object.entries(interfaceAddresses).reduce(function (acc, _ref) {
    var _ref2 = _slicedToArray(_ref, 2),
        key = _ref2[0],
        val = _ref2[1];

    acc[key] = val.map(function (v) {
      var protocol = v.family.toLowerCase();
      var suffix = getCIDRSuffix(v.netmask, protocol);
      var cidr = suffix ? "".concat(v.address, "/").concat(suffix) : null;
      return Object.assign({}, v, {
        cidr: cidr
      });
    });
    return acc;
  }, {});
}

module.exports = exports = {
  arch: arch,
  cpus: cpus,
  EOL: isWindows ? '\r\n' : '\n',
  endianness: endianness,
  freemem: getFreeMem,
  homedir: getHomeDirectory,
  hostname: getHostname,
  loadavg: loadavg,
  networkInterfaces: networkInterfaces,
  platform: platform,
  release: getOSRelease,
  tmpdir: tmpdir,
  totalmem: getTotalMem,
  type: getOSType,
  userInfo: getUserInfo,
  uptime: getUptime,
  // Deprecated APIs
  getNetworkInterfaces: deprecate(getInterfaceAddresses, getNetworkInterfacesDepMsg, 'DEP0023'),
  tmpDir: deprecate(tmpdir, tmpDirDeprecationMsg, 'DEP0022')
};
Object.defineProperty(module.exports, 'constants', {
  configurable: false,
  enumerable: true,
  value: constants
});