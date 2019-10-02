'use strict'; // Regex used for ansi escape code splitting
// Adopted from https://github.com/chalk/ansi-regex/blob/master/index.js
// License: MIT, authors: @sindresorhus, Qix-, and arjunmehta
// Matches all ansi escape code sequences in a string

var _marked =
/*#__PURE__*/
regeneratorRuntime.mark(emitKeys);

function _templateObject4() {
  var data = _taggedTemplateLiteral(["0J"]);

  _templateObject4 = function _templateObject4() {
    return data;
  };

  return data;
}

function _templateObject3() {
  var data = _taggedTemplateLiteral(["2K"]);

  _templateObject3 = function _templateObject3() {
    return data;
  };

  return data;
}

function _templateObject2() {
  var data = _taggedTemplateLiteral(["0K"]);

  _templateObject2 = function _templateObject2() {
    return data;
  };

  return data;
}

function _templateObject() {
  var data = _taggedTemplateLiteral(["1K"]);

  _templateObject = function _templateObject() {
    return data;
  };

  return data;
}

function _taggedTemplateLiteral(strings, raw) { if (!raw) { raw = strings.slice(0); } return Object.freeze(Object.defineProperties(strings, { raw: { value: Object.freeze(raw) } })); }

var ansi = /[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g;
var kEscape = '\x1b';
var getStringWidth;
var isFullWidthCodePoint;

function CSI(strings) {
  var ret = "".concat(kEscape, "[");

  for (var n = 0; n < strings.length; n++) {
    ret += strings[n];
    if (n < (arguments.length <= 1 ? 0 : arguments.length - 1)) ret += n + 1 < 1 || arguments.length <= n + 1 ? undefined : arguments[n + 1];
  }

  return ret;
}

CSI.kEscape = kEscape;
CSI.kClearToBeginning = CSI(_templateObject());
CSI.kClearToEnd = CSI(_templateObject2());
CSI.kClearLine = CSI(_templateObject3());
CSI.kClearScreenDown = CSI(_templateObject4());

if (process.binding('config').hasIntl) {
  var icu = process.binding('icu');

  getStringWidth = function getStringWidth(str, options) {
    options = options || {};
    if (!Number.isInteger(str)) str = stripVTControlCharacters(String(str));
    return icu.getStringWidth(str, Boolean(options.ambiguousAsFullWidth), Boolean(options.expandEmojiSequence));
  };

  isFullWidthCodePoint = function isFullWidthCodePoint(code, options) {
    if (typeof code !== 'number') return false;
    return icu.getStringWidth(code, options) === 2;
  };
} else {
  /**
   * Returns the number of columns required to display the given string.
   */
  getStringWidth = function getStringWidth(str) {
    if (Number.isInteger(str)) return isFullWidthCodePoint(str) ? 2 : 1;
    var width = 0;
    str = stripVTControlCharacters(String(str));

    for (var i = 0; i < str.length; i++) {
      var code = str.codePointAt(i);

      if (code >= 0x10000) {
        // surrogates
        i++;
      }

      if (isFullWidthCodePoint(code)) {
        width += 2;
      } else {
        width++;
      }
    }

    return width;
  };
  /**
   * Returns true if the character represented by a given
   * Unicode code point is full-width. Otherwise returns false.
   */


  isFullWidthCodePoint = function isFullWidthCodePoint(code) {
    if (!Number.isInteger(code)) {
      return false;
    } // Code points are derived from:
    // http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt


    if (code >= 0x1100 && (code <= 0x115f || // Hangul Jamo
    0x2329 === code || // LEFT-POINTING ANGLE BRACKET
    0x232a === code || // RIGHT-POINTING ANGLE BRACKET
    // CJK Radicals Supplement .. Enclosed CJK Letters and Months
    0x2e80 <= code && code <= 0x3247 && code !== 0x303f || // Enclosed CJK Letters and Months .. CJK Unified Ideographs Extension A
    0x3250 <= code && code <= 0x4dbf || // CJK Unified Ideographs .. Yi Radicals
    0x4e00 <= code && code <= 0xa4c6 || // Hangul Jamo Extended-A
    0xa960 <= code && code <= 0xa97c || // Hangul Syllables
    0xac00 <= code && code <= 0xd7a3 || // CJK Compatibility Ideographs
    0xf900 <= code && code <= 0xfaff || // Vertical Forms
    0xfe10 <= code && code <= 0xfe19 || // CJK Compatibility Forms .. Small Form Variants
    0xfe30 <= code && code <= 0xfe6b || // Halfwidth and Fullwidth Forms
    0xff01 <= code && code <= 0xff60 || 0xffe0 <= code && code <= 0xffe6 || // Kana Supplement
    0x1b000 <= code && code <= 0x1b001 || // Enclosed Ideographic Supplement
    0x1f200 <= code && code <= 0x1f251 || // CJK Unified Ideographs Extension B .. Tertiary Ideographic Plane
    0x20000 <= code && code <= 0x3fffd)) {
      return true;
    }

    return false;
  };
}
/**
 * Tries to remove all VT control characters. Use to estimate displayed
 * string width. May be buggy due to not running a real state machine
 */


function stripVTControlCharacters(str) {
  return str.replace(ansi, '');
}
/*
  Some patterns seen in terminal key escape codes, derived from combos seen
  at http://www.midnight-commander.org/browser/lib/tty/key.c

  ESC letter
  ESC [ letter
  ESC [ modifier letter
  ESC [ 1 ; modifier letter
  ESC [ num char
  ESC [ num ; modifier char
  ESC O letter
  ESC O modifier letter
  ESC O 1 ; modifier letter
  ESC N letter
  ESC [ [ num ; modifier char
  ESC [ [ 1 ; modifier letter
  ESC ESC [ num char
  ESC ESC O letter

  - char is usually ~ but $ and ^ also happen with rxvt
  - modifier is 1 +
                (shift     * 1) +
                (left_alt  * 2) +
                (ctrl      * 4) +
                (right_alt * 8)
  - two leading ESCs apparently mean the same as one leading ESC
*/


function emitKeys(stream) {
  var ch, s, escaped, key, code, modifier, cmdStart, cmd, match;
  return regeneratorRuntime.wrap(function emitKeys$(_context) {
    while (1) {
      switch (_context.prev = _context.next) {
        case 0:
          if (!true) {
            _context.next = 245;
            break;
          }

          _context.next = 3;
          return;

        case 3:
          ch = _context.sent;
          s = ch;
          escaped = false;
          key = {
            sequence: null,
            name: undefined,
            ctrl: false,
            meta: false,
            shift: false
          };

          if (!(ch === kEscape)) {
            _context.next = 18;
            break;
          }

          escaped = true;
          _context.t0 = s;
          _context.next = 12;
          return;

        case 12:
          s = _context.t0 += ch = _context.sent;

          if (!(ch === kEscape)) {
            _context.next = 18;
            break;
          }

          _context.t1 = s;
          _context.next = 17;
          return;

        case 17:
          s = _context.t1 += ch = _context.sent;

        case 18:
          if (!(escaped && (ch === 'O' || ch === '['))) {
            _context.next = 240;
            break;
          }

          // ansi escape sequence
          code = ch;
          modifier = 0;

          if (!(ch === 'O')) {
            _context.next = 35;
            break;
          }

          _context.t2 = s;
          _context.next = 25;
          return;

        case 25:
          s = _context.t2 += ch = _context.sent;

          if (!(ch >= '0' && ch <= '9')) {
            _context.next = 32;
            break;
          }

          modifier = (ch >> 0) - 1;
          _context.t3 = s;
          _context.next = 31;
          return;

        case 31:
          s = _context.t3 += ch = _context.sent;

        case 32:
          code += ch;
          _context.next = 70;
          break;

        case 35:
          if (!(ch === '[')) {
            _context.next = 70;
            break;
          }

          _context.t4 = s;
          _context.next = 39;
          return;

        case 39:
          s = _context.t4 += ch = _context.sent;

          if (!(ch === '[')) {
            _context.next = 46;
            break;
          }

          // \x1b[[A
          //      ^--- escape codes might have a second bracket
          code += ch;
          _context.t5 = s;
          _context.next = 45;
          return;

        case 45:
          s = _context.t5 += ch = _context.sent;

        case 46:
          /*
           * Here and later we try to buffer just enough data to get
           * a complete ascii sequence.
           *
           * We have basically two classes of ascii characters to process:
           *
           *
           * 1. `\x1b[24;5~` should be parsed as { code: '[24~', modifier: 5 }
           *
           * This particular example is featuring Ctrl+F12 in xterm.
           *
           *  - `;5` part is optional, e.g. it could be `\x1b[24~`
           *  - first part can contain one or two digits
           *
           * So the generic regexp is like /^\d\d?(;\d)?[~^$]$/
           *
           *
           * 2. `\x1b[1;5H` should be parsed as { code: '[H', modifier: 5 }
           *
           * This particular example is featuring Ctrl+Home in xterm.
           *
           *  - `1;5` part is optional, e.g. it could be `\x1b[H`
           *  - `1;` part is optional, e.g. it could be `\x1b[5H`
           *
           * So the generic regexp is like /^((\d;)?\d)?[A-Za-z]$/
           *
           */
          cmdStart = s.length - 1; // skip one or two leading digits

          if (!(ch >= '0' && ch <= '9')) {
            _context.next = 57;
            break;
          }

          _context.t6 = s;
          _context.next = 51;
          return;

        case 51:
          s = _context.t6 += ch = _context.sent;

          if (!(ch >= '0' && ch <= '9')) {
            _context.next = 57;
            break;
          }

          _context.t7 = s;
          _context.next = 56;
          return;

        case 56:
          s = _context.t7 += ch = _context.sent;

        case 57:
          if (!(ch === ';')) {
            _context.next = 67;
            break;
          }

          _context.t8 = s;
          _context.next = 61;
          return;

        case 61:
          s = _context.t8 += ch = _context.sent;

          if (!(ch >= '0' && ch <= '9')) {
            _context.next = 67;
            break;
          }

          _context.t9 = s;
          _context.next = 66;
          return;

        case 66:
          s = _context.t9 += ch = _context.sent;

        case 67:
          /*
           * We buffered enough data, now trying to extract code
           * and modifier from it
           */
          cmd = s.slice(cmdStart);
          match = void 0;

          if (match = cmd.match(/^(\d\d?)(;(\d))?([~^$])$/)) {
            code += match[1] + match[4];
            modifier = (match[3] || 1) - 1;
          } else if (match = cmd.match(/^((\d;)?(\d))?([A-Za-z])$/)) {
            code += match[4];
            modifier = (match[3] || 1) - 1;
          } else {
            code += cmd;
          }

        case 70:
          // Parse the key modifier
          key.ctrl = !!(modifier & 4);
          key.meta = !!(modifier & 10);
          key.shift = !!(modifier & 1);
          key.code = code; // Parse the key itself

          _context.t10 = code;
          _context.next = _context.t10 === 'OP' ? 77 : _context.t10 === 'OQ' ? 79 : _context.t10 === 'OR' ? 81 : _context.t10 === 'OS' ? 83 : _context.t10 === '[11~' ? 85 : _context.t10 === '[12~' ? 87 : _context.t10 === '[13~' ? 89 : _context.t10 === '[14~' ? 91 : _context.t10 === '[[A' ? 93 : _context.t10 === '[[B' ? 95 : _context.t10 === '[[C' ? 97 : _context.t10 === '[[D' ? 99 : _context.t10 === '[[E' ? 101 : _context.t10 === '[15~' ? 103 : _context.t10 === '[17~' ? 105 : _context.t10 === '[18~' ? 107 : _context.t10 === '[19~' ? 109 : _context.t10 === '[20~' ? 111 : _context.t10 === '[21~' ? 113 : _context.t10 === '[23~' ? 115 : _context.t10 === '[24~' ? 117 : _context.t10 === '[A' ? 119 : _context.t10 === '[B' ? 121 : _context.t10 === '[C' ? 123 : _context.t10 === '[D' ? 125 : _context.t10 === '[E' ? 127 : _context.t10 === '[F' ? 129 : _context.t10 === '[H' ? 131 : _context.t10 === 'OA' ? 133 : _context.t10 === 'OB' ? 135 : _context.t10 === 'OC' ? 137 : _context.t10 === 'OD' ? 139 : _context.t10 === 'OE' ? 141 : _context.t10 === 'OF' ? 143 : _context.t10 === 'OH' ? 145 : _context.t10 === '[1~' ? 147 : _context.t10 === '[2~' ? 149 : _context.t10 === '[3~' ? 151 : _context.t10 === '[4~' ? 153 : _context.t10 === '[5~' ? 155 : _context.t10 === '[6~' ? 157 : _context.t10 === '[[5~' ? 159 : _context.t10 === '[[6~' ? 161 : _context.t10 === '[7~' ? 163 : _context.t10 === '[8~' ? 165 : _context.t10 === '[a' ? 167 : _context.t10 === '[b' ? 170 : _context.t10 === '[c' ? 173 : _context.t10 === '[d' ? 176 : _context.t10 === '[e' ? 179 : _context.t10 === '[2$' ? 182 : _context.t10 === '[3$' ? 185 : _context.t10 === '[5$' ? 188 : _context.t10 === '[6$' ? 191 : _context.t10 === '[7$' ? 194 : _context.t10 === '[8$' ? 197 : _context.t10 === 'Oa' ? 200 : _context.t10 === 'Ob' ? 203 : _context.t10 === 'Oc' ? 206 : _context.t10 === 'Od' ? 209 : _context.t10 === 'Oe' ? 212 : _context.t10 === '[2^' ? 215 : _context.t10 === '[3^' ? 218 : _context.t10 === '[5^' ? 221 : _context.t10 === '[6^' ? 224 : _context.t10 === '[7^' ? 227 : _context.t10 === '[8^' ? 230 : _context.t10 === '[Z' ? 233 : 236;
          break;

        case 77:
          key.name = 'f1';
          return _context.abrupt("break", 238);

        case 79:
          key.name = 'f2';
          return _context.abrupt("break", 238);

        case 81:
          key.name = 'f3';
          return _context.abrupt("break", 238);

        case 83:
          key.name = 'f4';
          return _context.abrupt("break", 238);

        case 85:
          key.name = 'f1';
          return _context.abrupt("break", 238);

        case 87:
          key.name = 'f2';
          return _context.abrupt("break", 238);

        case 89:
          key.name = 'f3';
          return _context.abrupt("break", 238);

        case 91:
          key.name = 'f4';
          return _context.abrupt("break", 238);

        case 93:
          key.name = 'f1';
          return _context.abrupt("break", 238);

        case 95:
          key.name = 'f2';
          return _context.abrupt("break", 238);

        case 97:
          key.name = 'f3';
          return _context.abrupt("break", 238);

        case 99:
          key.name = 'f4';
          return _context.abrupt("break", 238);

        case 101:
          key.name = 'f5';
          return _context.abrupt("break", 238);

        case 103:
          key.name = 'f5';
          return _context.abrupt("break", 238);

        case 105:
          key.name = 'f6';
          return _context.abrupt("break", 238);

        case 107:
          key.name = 'f7';
          return _context.abrupt("break", 238);

        case 109:
          key.name = 'f8';
          return _context.abrupt("break", 238);

        case 111:
          key.name = 'f9';
          return _context.abrupt("break", 238);

        case 113:
          key.name = 'f10';
          return _context.abrupt("break", 238);

        case 115:
          key.name = 'f11';
          return _context.abrupt("break", 238);

        case 117:
          key.name = 'f12';
          return _context.abrupt("break", 238);

        case 119:
          key.name = 'up';
          return _context.abrupt("break", 238);

        case 121:
          key.name = 'down';
          return _context.abrupt("break", 238);

        case 123:
          key.name = 'right';
          return _context.abrupt("break", 238);

        case 125:
          key.name = 'left';
          return _context.abrupt("break", 238);

        case 127:
          key.name = 'clear';
          return _context.abrupt("break", 238);

        case 129:
          key.name = 'end';
          return _context.abrupt("break", 238);

        case 131:
          key.name = 'home';
          return _context.abrupt("break", 238);

        case 133:
          key.name = 'up';
          return _context.abrupt("break", 238);

        case 135:
          key.name = 'down';
          return _context.abrupt("break", 238);

        case 137:
          key.name = 'right';
          return _context.abrupt("break", 238);

        case 139:
          key.name = 'left';
          return _context.abrupt("break", 238);

        case 141:
          key.name = 'clear';
          return _context.abrupt("break", 238);

        case 143:
          key.name = 'end';
          return _context.abrupt("break", 238);

        case 145:
          key.name = 'home';
          return _context.abrupt("break", 238);

        case 147:
          key.name = 'home';
          return _context.abrupt("break", 238);

        case 149:
          key.name = 'insert';
          return _context.abrupt("break", 238);

        case 151:
          key.name = 'delete';
          return _context.abrupt("break", 238);

        case 153:
          key.name = 'end';
          return _context.abrupt("break", 238);

        case 155:
          key.name = 'pageup';
          return _context.abrupt("break", 238);

        case 157:
          key.name = 'pagedown';
          return _context.abrupt("break", 238);

        case 159:
          key.name = 'pageup';
          return _context.abrupt("break", 238);

        case 161:
          key.name = 'pagedown';
          return _context.abrupt("break", 238);

        case 163:
          key.name = 'home';
          return _context.abrupt("break", 238);

        case 165:
          key.name = 'end';
          return _context.abrupt("break", 238);

        case 167:
          key.name = 'up';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 170:
          key.name = 'down';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 173:
          key.name = 'right';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 176:
          key.name = 'left';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 179:
          key.name = 'clear';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 182:
          key.name = 'insert';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 185:
          key.name = 'delete';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 188:
          key.name = 'pageup';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 191:
          key.name = 'pagedown';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 194:
          key.name = 'home';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 197:
          key.name = 'end';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 200:
          key.name = 'up';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 203:
          key.name = 'down';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 206:
          key.name = 'right';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 209:
          key.name = 'left';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 212:
          key.name = 'clear';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 215:
          key.name = 'insert';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 218:
          key.name = 'delete';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 221:
          key.name = 'pageup';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 224:
          key.name = 'pagedown';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 227:
          key.name = 'home';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 230:
          key.name = 'end';
          key.ctrl = true;
          return _context.abrupt("break", 238);

        case 233:
          key.name = 'tab';
          key.shift = true;
          return _context.abrupt("break", 238);

        case 236:
          key.name = 'undefined';
          return _context.abrupt("break", 238);

        case 238:
          _context.next = 241;
          break;

        case 240:
          if (ch === '\r') {
            // carriage return
            key.name = 'return';
          } else if (ch === '\n') {
            // enter, should have been called linefeed
            key.name = 'enter';
          } else if (ch === '\t') {
            // tab
            key.name = 'tab';
          } else if (ch === '\b' || ch === '\x7f') {
            // backspace or ctrl+h
            key.name = 'backspace';
            key.meta = escaped;
          } else if (ch === kEscape) {
            // escape key
            key.name = 'escape';
            key.meta = escaped;
          } else if (ch === ' ') {
            key.name = 'space';
            key.meta = escaped;
          } else if (!escaped && ch <= '\x1a') {
            // ctrl+letter
            key.name = String.fromCharCode(ch.charCodeAt(0) + 'a'.charCodeAt(0) - 1);
            key.ctrl = true;
          } else if (/^[0-9A-Za-z]$/.test(ch)) {
            // letter, number, shift+letter
            key.name = ch.toLowerCase();
            key.shift = /^[A-Z]$/.test(ch);
            key.meta = escaped;
          } else if (escaped) {
            // Escape sequence timeout
            key.name = ch.length ? undefined : 'escape';
            key.meta = true;
          }

        case 241:
          key.sequence = s;

          if (s.length !== 0 && (key.name !== undefined || escaped)) {
            /* Named character or sequence */
            stream.emit('keypress', escaped ? undefined : s, key);
          } else if (s.length === 1) {
            /* Single unnamed character, e.g. "." */
            stream.emit('keypress', s, key);
          }
          /* Unrecognized or broken escape sequence, don't emit anything */


          _context.next = 0;
          break;

        case 245:
        case "end":
          return _context.stop();
      }
    }
  }, _marked);
}

module.exports = {
  emitKeys: emitKeys,
  getStringWidth: getStringWidth,
  isFullWidthCodePoint: isFullWidthCodePoint,
  stripVTControlCharacters: stripVTControlCharacters,
  CSI: CSI
};