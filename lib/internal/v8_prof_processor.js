'use strict';

const vm = require('vm');

const scriptFiles = [
  'internal/v8_prof_polyfill',
  'v8/tools/splaytree',
  'v8/tools/codemap',
  'v8/tools/csvparser',
  'v8/tools/consarray',
  'v8/tools/profile',
  'v8/tools/profile_view',
  'v8/tools/logreader',
  'v8/tools/tickprocessor',
  'v8/tools/SourceMap',
  'v8/tools/tickprocessor-driver'
];
var script = '';

scriptFiles.forEach(function(s) {
  script += process.binding('natives')[s] + '\n';
});

const tickArguments = [];
if (process.platform === 'darwin') {
  tickArguments.push('--mac');
} else if (process.platform === 'win32') {
  tickArguments.push('--windows');
}
tickArguments.push.apply(tickArguments, process.argv.slice(1));
script = `(function(module, require) {
  arguments = ${JSON.stringify(tickArguments)};
  function write (s) { process.stdout.write(s) }
  function printErr(err) { console.error(err); }
  ${script}
})`;
vm.runInThisContext(script)(module, require);
