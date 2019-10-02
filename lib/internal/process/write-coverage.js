'use strict';

var process = require('process');

var path = require('path');

var _require = require('fs'),
    mkdirSync = _require.mkdirSync,
    writeFileSync = _require.writeFileSync;

function writeCoverage() {
  if (!global.__coverage__) {
    return;
  }

  var dirname = path.join(path.dirname(process.execPath), '.coverage');
  var filename = "coverage-".concat(process.pid, "-").concat(Date.now(), ".json");

  try {
    mkdirSync(dirname);
  } catch (err) {
    if (err.code !== 'EEXIST') {
      console.error(err);
      return;
    }
  }

  var target = path.join(dirname, filename);
  var coverageInfo = JSON.stringify(global.__coverage__);

  try {
    writeFileSync(target, coverageInfo);
  } catch (err) {
    console.error(err);
  }
}

function setup() {
  var reallyReallyExit = process.reallyExit;

  process.reallyExit = function (code) {
    writeCoverage();
    reallyReallyExit(code);
  };

  process.on('exit', writeCoverage);
}

exports.setup = setup;