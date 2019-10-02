'use strict';

var _Object$definePropert;

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _typeof(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var util = require('util');

var _require = require('internal/querystring'),
    hexTable = _require.hexTable,
    isHexTable = _require.isHexTable;

var _require2 = require('internal/util'),
    getConstructorOf = _require2.getConstructorOf;

var errors = require('internal/errors');

var querystring = require('querystring');

var _process = process,
    platform = _process.platform;
var isWindows = platform === 'win32';

var _process$binding = process.binding('url'),
    _domainToASCII = _process$binding.domainToASCII,
    _domainToUnicode = _process$binding.domainToUnicode,
    encodeAuth = _process$binding.encodeAuth,
    _toUSVString = _process$binding.toUSVString,
    _parse = _process$binding.parse,
    setURLConstructor = _process$binding.setURLConstructor,
    URL_FLAGS_CANNOT_BE_BASE = _process$binding.URL_FLAGS_CANNOT_BE_BASE,
    URL_FLAGS_HAS_FRAGMENT = _process$binding.URL_FLAGS_HAS_FRAGMENT,
    URL_FLAGS_HAS_HOST = _process$binding.URL_FLAGS_HAS_HOST,
    URL_FLAGS_HAS_PASSWORD = _process$binding.URL_FLAGS_HAS_PASSWORD,
    URL_FLAGS_HAS_PATH = _process$binding.URL_FLAGS_HAS_PATH,
    URL_FLAGS_HAS_QUERY = _process$binding.URL_FLAGS_HAS_QUERY,
    URL_FLAGS_HAS_USERNAME = _process$binding.URL_FLAGS_HAS_USERNAME,
    URL_FLAGS_SPECIAL = _process$binding.URL_FLAGS_SPECIAL,
    kFragment = _process$binding.kFragment,
    kHost = _process$binding.kHost,
    kHostname = _process$binding.kHostname,
    kPathStart = _process$binding.kPathStart,
    kPort = _process$binding.kPort,
    kQuery = _process$binding.kQuery,
    kSchemeStart = _process$binding.kSchemeStart;

var context = Symbol('context');
var cannotBeBase = Symbol('cannot-be-base');
var cannotHaveUsernamePasswordPort = Symbol('cannot-have-username-password-port');
var special = Symbol('special');
var searchParams = Symbol('query');
var kFormat = Symbol('format'); // https://tc39.github.io/ecma262/#sec-%iteratorprototype%-object

var IteratorPrototype = Object.getPrototypeOf(Object.getPrototypeOf([][Symbol.iterator]()));
var unpairedSurrogateRe = /(?:[^\uD800-\uDBFF]|^)[\uDC00-\uDFFF]|[\uD800-\uDBFF](?![\uDC00-\uDFFF])/;

function toUSVString(val) {
  var str = "".concat(val); // As of V8 5.5, `str.search()` (and `unpairedSurrogateRe[@@search]()`) are
  // slower than `unpairedSurrogateRe.exec()`.

  var match = unpairedSurrogateRe.exec(str);
  if (!match) return str;
  return _toUSVString(str, match.index);
} // Refs: https://html.spec.whatwg.org/multipage/browsers.html#concept-origin-opaque


var kOpaqueOrigin = 'null'; // Refs: https://html.spec.whatwg.org/multipage/browsers.html#ascii-serialisation-of-an-origin

function serializeTupleOrigin(scheme, host, port) {
  return "".concat(scheme, "//").concat(host).concat(port === null ? '' : ":".concat(port));
} // This class provides the internal state of a URL object. An instance of this
// class is stored in every URL object and is accessed internally by setters
// and getters. It roughly corresponds to the concept of a URL record in the
// URL Standard, with a few differences. It is also the object transported to
// the C++ binding.
// Refs: https://url.spec.whatwg.org/#concept-url


var URLContext = function URLContext() {
  _classCallCheck(this, URLContext);

  this.flags = 0;
  this.scheme = ':';
  this.username = '';
  this.password = '';
  this.host = null;
  this.port = null;
  this.path = [];
  this.query = null;
  this.fragment = null;
};

var URLSearchParams =
/*#__PURE__*/
function () {
  // URL Standard says the default value is '', but as undefined and '' have
  // the same result, undefined is used to prevent unnecessary parsing.
  // Default parameter is necessary to keep URLSearchParams.length === 0 in
  // accordance with Web IDL spec.
  function URLSearchParams() {
    var init = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : undefined;

    _classCallCheck(this, URLSearchParams);

    if (init === null || init === undefined) {
      this[searchParams] = [];
    } else if (_typeof(init) === 'object' && init !== null || typeof init === 'function') {
      var method = init[Symbol.iterator];

      if (method === this[Symbol.iterator]) {
        // While the spec does not have this branch, we can use it as a
        // shortcut to avoid having to go through the costly generic iterator.
        var childParams = init[searchParams];
        this[searchParams] = childParams.slice();
      } else if (method !== null && method !== undefined) {
        if (typeof method !== 'function') {
          throw new errors.TypeError('ERR_ARG_NOT_ITERABLE', 'Query pairs');
        } // sequence<sequence<USVString>>
        // Note: per spec we have to first exhaust the lists then process them


        var pairs = [];
        var _iteratorNormalCompletion = true;
        var _didIteratorError = false;
        var _iteratorError = undefined;

        try {
          for (var _iterator = init[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
            var pair = _step.value;

            if (_typeof(pair) !== 'object' && typeof pair !== 'function' || pair === null || typeof pair[Symbol.iterator] !== 'function') {
              throw new errors.TypeError('ERR_INVALID_TUPLE', 'Each query pair', '[name, value]');
            }

            var convertedPair = [];
            var _iteratorNormalCompletion2 = true;
            var _didIteratorError2 = false;
            var _iteratorError2 = undefined;

            try {
              for (var _iterator2 = pair[Symbol.iterator](), _step2; !(_iteratorNormalCompletion2 = (_step2 = _iterator2.next()).done); _iteratorNormalCompletion2 = true) {
                var element = _step2.value;
                convertedPair.push(toUSVString(element));
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

            pairs.push(convertedPair);
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

        this[searchParams] = [];

        for (var _i = 0, _pairs = pairs; _i < _pairs.length; _i++) {
          var _pair = _pairs[_i];

          if (_pair.length !== 2) {
            throw new errors.TypeError('ERR_INVALID_TUPLE', 'Each query pair', '[name, value]');
          }

          this[searchParams].push(_pair[0], _pair[1]);
        }
      } else {
        // record<USVString, USVString>
        // Need to use reflection APIs for full spec compliance.
        this[searchParams] = [];
        var keys = Reflect.ownKeys(init);

        for (var i = 0; i < keys.length; i++) {
          var key = keys[i];
          var desc = Reflect.getOwnPropertyDescriptor(init, key);

          if (desc !== undefined && desc.enumerable) {
            var typedKey = toUSVString(key);
            var typedValue = toUSVString(init[key]);
            this[searchParams].push(typedKey, typedValue);
          }
        }
      }
    } else {
      // USVString
      init = toUSVString(init);
      if (init[0] === '?') init = init.slice(1);
      initSearchParams(this, init);
    } // "associated url object"


    this[context] = null;
  }

  _createClass(URLSearchParams, [{
    key: util.inspect.custom,
    value: function value(recurseTimes, ctx) {
      if (!this || !this[searchParams] || this[searchParams][searchParams]) {
        throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
      }

      if (typeof recurseTimes === 'number' && recurseTimes < 0) return ctx.stylize('[Object]', 'special');
      var separator = ', ';

      var innerOpts = util._extend({}, ctx);

      if (recurseTimes !== null) {
        innerOpts.depth = recurseTimes - 1;
      }

      var innerInspect = function innerInspect(v) {
        return util.inspect(v, innerOpts);
      };

      var list = this[searchParams];
      var output = [];

      for (var i = 0; i < list.length; i += 2) {
        output.push("".concat(innerInspect(list[i]), " => ").concat(innerInspect(list[i + 1])));
      }

      var colorRe = /\u001b\[\d\d?m/g;
      var length = output.reduce(function (prev, cur) {
        return prev + cur.replace(colorRe, '').length + separator.length;
      }, -separator.length);

      if (length > ctx.breakLength) {
        return "".concat(this.constructor.name, " {\n  ").concat(output.join(',\n  '), " }");
      } else if (output.length) {
        return "".concat(this.constructor.name, " { ").concat(output.join(separator), " }");
      } else {
        return "".concat(this.constructor.name, " {}");
      }
    }
  }]);

  return URLSearchParams;
}();

function onParseComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];
  ctx.flags = flags;
  ctx.scheme = protocol;
  ctx.username = (flags & URL_FLAGS_HAS_USERNAME) !== 0 ? username : '';
  ctx.password = (flags & URL_FLAGS_HAS_PASSWORD) !== 0 ? password : '';
  ctx.port = port;
  ctx.path = (flags & URL_FLAGS_HAS_PATH) !== 0 ? path : [];
  ctx.query = query;
  ctx.fragment = fragment;
  ctx.host = host;

  if (!this[searchParams]) {
    // invoked from URL constructor
    this[searchParams] = new URLSearchParams();
    this[searchParams][context] = this;
  }

  initSearchParams(this[searchParams], query);
}

function onParseError(flags, input) {
  var error = new errors.TypeError('ERR_INVALID_URL', input);
  error.input = input;
  throw error;
} // Reused by URL constructor and URL#href setter.


function parse(url, input, base) {
  var base_context = base ? base[context] : undefined;
  url[context] = new URLContext();

  _parse(input.trim(), -1, base_context, undefined, onParseComplete.bind(url), onParseError);
}

function onParseProtocolComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_SPECIAL) !== 0) {
    ctx.flags |= URL_FLAGS_SPECIAL;
  } else {
    ctx.flags &= ~URL_FLAGS_SPECIAL;
  }

  ctx.scheme = protocol;
  ctx.port = port;
}

function onParseHostComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_HAS_HOST) !== 0) {
    ctx.host = host;
    ctx.flags |= URL_FLAGS_HAS_HOST;
  } else {
    ctx.host = null;
    ctx.flags &= ~URL_FLAGS_HAS_HOST;
  }

  if (port !== null) ctx.port = port;
}

function onParseHostnameComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_HAS_HOST) !== 0) {
    ctx.host = host;
    ctx.flags |= URL_FLAGS_HAS_HOST;
  } else {
    ctx.host = null;
    ctx.flags &= ~URL_FLAGS_HAS_HOST;
  }
}

function onParsePortComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].port = port;
}

function onParsePathComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = this[context];

  if ((flags & URL_FLAGS_HAS_PATH) !== 0) {
    ctx.path = path;
    ctx.flags |= URL_FLAGS_HAS_PATH;
  } else {
    ctx.path = [];
    ctx.flags &= ~URL_FLAGS_HAS_PATH;
  } // The C++ binding may set host to empty string.


  if ((flags & URL_FLAGS_HAS_HOST) !== 0) {
    ctx.host = host;
    ctx.flags |= URL_FLAGS_HAS_HOST;
  }
}

function onParseSearchComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].query = query;
}

function onParseHashComplete(flags, protocol, username, password, host, port, path, query, fragment) {
  this[context].fragment = fragment;
}

var URL =
/*#__PURE__*/
function () {
  function URL(input, base) {
    _classCallCheck(this, URL);

    // toUSVString is not needed.
    input = "".concat(input);

    if (base !== undefined && (!base[searchParams] || !base[searchParams][searchParams])) {
      base = new URL(base);
    }

    parse(this, input, base);
  }

  _createClass(URL, [{
    key: util.inspect.custom,
    value: function value(depth, opts) {
      if (this == null || Object.getPrototypeOf(this[context]) !== URLContext.prototype) {
        throw new errors.TypeError('ERR_INVALID_THIS', 'URL');
      }

      if (typeof depth === 'number' && depth < 0) return opts.stylize('[Object]', 'special');
      var ctor = getConstructorOf(this);
      var obj = Object.create({
        constructor: ctor === null ? URL : ctor
      });
      obj.href = this.href;
      obj.origin = this.origin;
      obj.protocol = this.protocol;
      obj.username = this.username;
      obj.password = this.password;
      obj.host = this.host;
      obj.hostname = this.hostname;
      obj.port = this.port;
      obj.pathname = this.pathname;
      obj.search = this.search;
      obj.searchParams = this.searchParams;
      obj.hash = this.hash;

      if (opts.showHidden) {
        obj.cannotBeBase = this[cannotBeBase];
        obj.special = this[special];
        obj[context] = this[context];
      }

      return util.inspect(obj, opts);
    }
  }, {
    key: special,
    get: function get() {
      return (this[context].flags & URL_FLAGS_SPECIAL) !== 0;
    }
  }, {
    key: cannotBeBase,
    get: function get() {
      return (this[context].flags & URL_FLAGS_CANNOT_BE_BASE) !== 0;
    } // https://url.spec.whatwg.org/#cannot-have-a-username-password-port

  }, {
    key: cannotHaveUsernamePasswordPort,
    get: function get() {
      var _this$context = this[context],
          host = _this$context.host,
          scheme = _this$context.scheme;
      return host == null || host === '' || this[cannotBeBase] || scheme === 'file:';
    }
  }]);

  return URL;
}();

Object.defineProperties(URL.prototype, (_Object$definePropert = {}, _defineProperty(_Object$definePropert, kFormat, {
  enumerable: false,
  configurable: false,
  // eslint-disable-next-line func-name-matching
  value: function format(options) {
    if (options && _typeof(options) !== 'object') throw new errors.TypeError('ERR_INVALID_ARG_TYPE', 'options', 'object');
    options = util._extend({
      fragment: true,
      unicode: false,
      search: true,
      auth: true
    }, options);
    var ctx = this[context];
    var ret = ctx.scheme;

    if (ctx.host !== null) {
      ret += '//';
      var has_username = ctx.username !== '';
      var has_password = ctx.password !== '';

      if (options.auth && (has_username || has_password)) {
        if (has_username) ret += ctx.username;
        if (has_password) ret += ":".concat(ctx.password);
        ret += '@';
      }

      ret += options.unicode ? domainToUnicode(this.host) : this.host;
    } else if (ctx.scheme === 'file:') {
      ret += '//';
    }

    if (this.pathname) ret += this.pathname;
    if (options.search && ctx.query !== null) ret += "?".concat(ctx.query);
    if (options.fragment && ctx.fragment !== null) ret += "#".concat(ctx.fragment);
    return ret;
  }
}), _defineProperty(_Object$definePropert, Symbol.toStringTag, {
  configurable: true,
  value: 'URL'
}), _defineProperty(_Object$definePropert, "toString", {
  // https://heycam.github.io/webidl/#es-stringifier
  writable: true,
  enumerable: true,
  configurable: true,
  // eslint-disable-next-line func-name-matching
  value: function toString() {
    return this[kFormat]({});
  }
}), _defineProperty(_Object$definePropert, "href", {
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[kFormat]({});
  },
  set: function set(input) {
    // toUSVString is not needed.
    input = "".concat(input);
    parse(this, input);
  }
}), _defineProperty(_Object$definePropert, "origin", {
  // readonly
  enumerable: true,
  configurable: true,
  get: function get() {
    // Refs: https://url.spec.whatwg.org/#concept-url-origin
    var ctx = this[context];

    switch (ctx.scheme) {
      case 'blob:':
        if (ctx.path.length > 0) {
          try {
            return new URL(ctx.path[0]).origin;
          } catch (err) {// fall through... do nothing
          }
        }

        return kOpaqueOrigin;

      case 'ftp:':
      case 'gopher:':
      case 'http:':
      case 'https:':
      case 'ws:':
      case 'wss:':
        return serializeTupleOrigin(ctx.scheme, ctx.host, ctx.port);
    }

    return kOpaqueOrigin;
  }
}), _defineProperty(_Object$definePropert, "protocol", {
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[context].scheme;
  },
  set: function set(scheme) {
    // toUSVString is not needed.
    scheme = "".concat(scheme);
    if (scheme.length === 0) return;
    var ctx = this[context];

    if (ctx.scheme === 'file:' && (ctx.host === '' || ctx.host === null)) {
      return;
    }

    _parse(scheme, kSchemeStart, null, ctx, onParseProtocolComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "username", {
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[context].username;
  },
  set: function set(username) {
    // toUSVString is not needed.
    username = "".concat(username);
    if (this[cannotHaveUsernamePasswordPort]) return;
    var ctx = this[context];

    if (username === '') {
      ctx.username = '';
      ctx.flags &= ~URL_FLAGS_HAS_USERNAME;
      return;
    }

    ctx.username = encodeAuth(username);
    ctx.flags |= URL_FLAGS_HAS_USERNAME;
  }
}), _defineProperty(_Object$definePropert, "password", {
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[context].password;
  },
  set: function set(password) {
    // toUSVString is not needed.
    password = "".concat(password);
    if (this[cannotHaveUsernamePasswordPort]) return;
    var ctx = this[context];

    if (password === '') {
      ctx.password = '';
      ctx.flags &= ~URL_FLAGS_HAS_PASSWORD;
      return;
    }

    ctx.password = encodeAuth(password);
    ctx.flags |= URL_FLAGS_HAS_PASSWORD;
  }
}), _defineProperty(_Object$definePropert, "host", {
  enumerable: true,
  configurable: true,
  get: function get() {
    var ctx = this[context];
    var ret = ctx.host || '';
    if (ctx.port !== null) ret += ":".concat(ctx.port);
    return ret;
  },
  set: function set(host) {
    var ctx = this[context]; // toUSVString is not needed.

    host = "".concat(host);

    if (this[cannotBeBase]) {
      // Cannot set the host if cannot-be-base is set
      return;
    }

    _parse(host, kHost, null, ctx, onParseHostComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "hostname", {
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[context].host || '';
  },
  set: function set(host) {
    var ctx = this[context]; // toUSVString is not needed.

    host = "".concat(host);

    if (this[cannotBeBase]) {
      // Cannot set the host if cannot-be-base is set
      return;
    }

    _parse(host, kHostname, null, ctx, onParseHostnameComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "port", {
  enumerable: true,
  configurable: true,
  get: function get() {
    var port = this[context].port;
    return port === null ? '' : String(port);
  },
  set: function set(port) {
    // toUSVString is not needed.
    port = "".concat(port);
    if (this[cannotHaveUsernamePasswordPort]) return;
    var ctx = this[context];

    if (port === '') {
      ctx.port = null;
      return;
    }

    _parse(port, kPort, null, ctx, onParsePortComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "pathname", {
  enumerable: true,
  configurable: true,
  get: function get() {
    var ctx = this[context];
    if (this[cannotBeBase]) return ctx.path[0];
    if (ctx.path.length === 0) return '';
    return "/".concat(ctx.path.join('/'));
  },
  set: function set(path) {
    // toUSVString is not needed.
    path = "".concat(path);
    if (this[cannotBeBase]) return;

    _parse(path, kPathStart, null, this[context], onParsePathComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "search", {
  enumerable: true,
  configurable: true,
  get: function get() {
    var query = this[context].query;
    if (query === null || query === '') return '';
    return "?".concat(query);
  },
  set: function set(search) {
    var ctx = this[context];
    search = toUSVString(search);

    if (search === '') {
      ctx.query = null;
      ctx.flags &= ~URL_FLAGS_HAS_QUERY;
    } else {
      if (search[0] === '?') search = search.slice(1);
      ctx.query = '';
      ctx.flags |= URL_FLAGS_HAS_QUERY;

      if (search) {
        _parse(search, kQuery, null, ctx, onParseSearchComplete.bind(this));
      }
    }

    initSearchParams(this[searchParams], search);
  }
}), _defineProperty(_Object$definePropert, "searchParams", {
  // readonly
  enumerable: true,
  configurable: true,
  get: function get() {
    return this[searchParams];
  }
}), _defineProperty(_Object$definePropert, "hash", {
  enumerable: true,
  configurable: true,
  get: function get() {
    var fragment = this[context].fragment;
    if (fragment === null || fragment === '') return '';
    return "#".concat(fragment);
  },
  set: function set(hash) {
    var ctx = this[context]; // toUSVString is not needed.

    hash = "".concat(hash);

    if (!hash) {
      ctx.fragment = null;
      ctx.flags &= ~URL_FLAGS_HAS_FRAGMENT;
      return;
    }

    if (hash[0] === '#') hash = hash.slice(1);
    ctx.fragment = '';
    ctx.flags |= URL_FLAGS_HAS_FRAGMENT;

    _parse(hash, kFragment, null, ctx, onParseHashComplete.bind(this));
  }
}), _defineProperty(_Object$definePropert, "toJSON", {
  writable: true,
  enumerable: true,
  configurable: true,
  // eslint-disable-next-line func-name-matching
  value: function toJSON() {
    return this[kFormat]({});
  }
}), _Object$definePropert));

function update(url, params) {
  if (!url) return;
  var ctx = url[context];
  var serializedParams = params.toString();

  if (serializedParams) {
    ctx.query = serializedParams;
    ctx.flags |= URL_FLAGS_HAS_QUERY;
  } else {
    ctx.query = null;
    ctx.flags &= ~URL_FLAGS_HAS_QUERY;
  }
}

function initSearchParams(url, init) {
  if (!init) {
    url[searchParams] = [];
    return;
  }

  url[searchParams] = parseParams(init);
} // application/x-www-form-urlencoded parser
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-parser


function parseParams(qs) {
  var out = [];
  var pairStart = 0;
  var lastPos = 0;
  var seenSep = false;
  var buf = '';
  var encoded = false;
  var encodeCheck = 0;
  var i;

  for (i = 0; i < qs.length; ++i) {
    var code = qs.charCodeAt(i); // Try matching key/value pair separator

    if (code === 38
    /*&*/
    ) {
        if (pairStart === i) {
          // We saw an empty substring between pair separators
          lastPos = pairStart = i + 1;
          continue;
        }

        if (lastPos < i) buf += qs.slice(lastPos, i);
        if (encoded) buf = querystring.unescape(buf);
        out.push(buf); // If `buf` is the key, add an empty value.

        if (!seenSep) out.push('');
        seenSep = false;
        buf = '';
        encoded = false;
        encodeCheck = 0;
        lastPos = pairStart = i + 1;
        continue;
      } // Try matching key/value separator (e.g. '=') if we haven't already


    if (!seenSep && code === 61
    /*=*/
    ) {
        // Key/value separator match!
        if (lastPos < i) buf += qs.slice(lastPos, i);
        if (encoded) buf = querystring.unescape(buf);
        out.push(buf);
        seenSep = true;
        buf = '';
        encoded = false;
        encodeCheck = 0;
        lastPos = i + 1;
        continue;
      } // Handle + and percent decoding.


    if (code === 43
    /*+*/
    ) {
        if (lastPos < i) buf += qs.slice(lastPos, i);
        buf += ' ';
        lastPos = i + 1;
      } else if (!encoded) {
      // Try to match an (valid) encoded byte (once) to minimize unnecessary
      // calls to string decoding functions
      if (code === 37
      /*%*/
      ) {
          encodeCheck = 1;
        } else if (encodeCheck > 0) {
        // eslint-disable-next-line no-extra-boolean-cast
        if (!!isHexTable[code]) {
          if (++encodeCheck === 3) encoded = true;
        } else {
          encodeCheck = 0;
        }
      }
    }
  } // Deal with any leftover key or value data
  // There is a trailing &. No more processing is needed.


  if (pairStart === i) return out;
  if (lastPos < i) buf += qs.slice(lastPos, i);
  if (encoded) buf = querystring.unescape(buf);
  out.push(buf); // If `buf` is the key, add an empty value.

  if (!seenSep) out.push('');
  return out;
} // Adapted from querystring's implementation.
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-byte-serializer


var noEscape = [//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, // 0x20 - 0x2F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0x30 - 0x3F
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 0x50 - 0x5F
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 // 0x70 - 0x7F
]; // Special version of hexTable that uses `+` for U+0020 SPACE.

var paramHexTable = hexTable.slice();
paramHexTable[0x20] = '+';

function escapeParam(str) {
  var len = str.length;
  if (len === 0) return '';
  var out = '';
  var lastPos = 0;

  for (var i = 0; i < len; i++) {
    var c = str.charCodeAt(i); // ASCII

    if (c < 0x80) {
      if (noEscape[c] === 1) continue;
      if (lastPos < i) out += str.slice(lastPos, i);
      lastPos = i + 1;
      out += paramHexTable[c];
      continue;
    }

    if (lastPos < i) out += str.slice(lastPos, i); // Multi-byte characters ...

    if (c < 0x800) {
      lastPos = i + 1;
      out += paramHexTable[0xC0 | c >> 6] + paramHexTable[0x80 | c & 0x3F];
      continue;
    }

    if (c < 0xD800 || c >= 0xE000) {
      lastPos = i + 1;
      out += paramHexTable[0xE0 | c >> 12] + paramHexTable[0x80 | c >> 6 & 0x3F] + paramHexTable[0x80 | c & 0x3F];
      continue;
    } // Surrogate pair


    ++i;
    var c2;
    if (i < len) c2 = str.charCodeAt(i) & 0x3FF;else {
      // This branch should never happen because all URLSearchParams entries
      // should already be converted to USVString. But, included for
      // completion's sake anyway.
      c2 = 0;
    }
    lastPos = i + 1;
    c = 0x10000 + ((c & 0x3FF) << 10 | c2);
    out += paramHexTable[0xF0 | c >> 18] + paramHexTable[0x80 | c >> 12 & 0x3F] + paramHexTable[0x80 | c >> 6 & 0x3F] + paramHexTable[0x80 | c & 0x3F];
  }

  if (lastPos === 0) return str;
  if (lastPos < len) return out + str.slice(lastPos);
  return out;
} // application/x-www-form-urlencoded serializer
// Ref: https://url.spec.whatwg.org/#concept-urlencoded-serializer


function serializeParams(array) {
  var len = array.length;
  if (len === 0) return '';
  var output = "".concat(escapeParam(array[0]), "=").concat(escapeParam(array[1]));

  for (var i = 2; i < len; i += 2) {
    output += "&".concat(escapeParam(array[i]), "=").concat(escapeParam(array[i + 1]));
  }

  return output;
} // Mainly to mitigate func-name-matching ESLint rule


function defineIDLClass(proto, classStr, obj) {
  // https://heycam.github.io/webidl/#dfn-class-string
  Object.defineProperty(proto, Symbol.toStringTag, {
    writable: false,
    enumerable: false,
    configurable: true,
    value: classStr
  }); // https://heycam.github.io/webidl/#es-operations

  for (var _i2 = 0, _Object$keys = Object.keys(obj); _i2 < _Object$keys.length; _i2++) {
    var key = _Object$keys[_i2];
    Object.defineProperty(proto, key, {
      writable: true,
      enumerable: true,
      configurable: true,
      value: obj[key]
    });
  }

  var _iteratorNormalCompletion3 = true;
  var _didIteratorError3 = false;
  var _iteratorError3 = undefined;

  try {
    for (var _iterator3 = Object.getOwnPropertySymbols(obj)[Symbol.iterator](), _step3; !(_iteratorNormalCompletion3 = (_step3 = _iterator3.next()).done); _iteratorNormalCompletion3 = true) {
      var _key = _step3.value;
      Object.defineProperty(proto, _key, {
        writable: true,
        enumerable: false,
        configurable: true,
        value: obj[_key]
      });
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
} // for merge sort


function merge(out, start, mid, end, lBuffer, rBuffer) {
  var sizeLeft = mid - start;
  var sizeRight = end - mid;
  var l, r, o;

  for (l = 0; l < sizeLeft; l++) {
    lBuffer[l] = out[start + l];
  }

  for (r = 0; r < sizeRight; r++) {
    rBuffer[r] = out[mid + r];
  }

  l = 0;
  r = 0;
  o = start;

  while (l < sizeLeft && r < sizeRight) {
    if (lBuffer[l] <= rBuffer[r]) {
      out[o++] = lBuffer[l++];
      out[o++] = lBuffer[l++];
    } else {
      out[o++] = rBuffer[r++];
      out[o++] = rBuffer[r++];
    }
  }

  while (l < sizeLeft) {
    out[o++] = lBuffer[l++];
  }

  while (r < sizeRight) {
    out[o++] = rBuffer[r++];
  }
}

defineIDLClass(URLSearchParams.prototype, 'URLSearchParams', {
  append: function append(name, value) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 2) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name', 'value');
    }

    name = toUSVString(name);
    value = toUSVString(value);
    this[searchParams].push(name, value);
    update(this[context], this);
  },
  "delete": function _delete(name) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 1) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name');
    }

    var list = this[searchParams];
    name = toUSVString(name);

    for (var i = 0; i < list.length;) {
      var cur = list[i];

      if (cur === name) {
        list.splice(i, 2);
      } else {
        i += 2;
      }
    }

    update(this[context], this);
  },
  get: function get(name) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 1) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name');
    }

    var list = this[searchParams];
    name = toUSVString(name);

    for (var i = 0; i < list.length; i += 2) {
      if (list[i] === name) {
        return list[i + 1];
      }
    }

    return null;
  },
  getAll: function getAll(name) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 1) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name');
    }

    var list = this[searchParams];
    var values = [];
    name = toUSVString(name);

    for (var i = 0; i < list.length; i += 2) {
      if (list[i] === name) {
        values.push(list[i + 1]);
      }
    }

    return values;
  },
  has: function has(name) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 1) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name');
    }

    var list = this[searchParams];
    name = toUSVString(name);

    for (var i = 0; i < list.length; i += 2) {
      if (list[i] === name) {
        return true;
      }
    }

    return false;
  },
  set: function set(name, value) {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (arguments.length < 2) {
      throw new errors.TypeError('ERR_MISSING_ARGS', 'name', 'value');
    }

    var list = this[searchParams];
    name = toUSVString(name);
    value = toUSVString(value); // If there are any name-value pairs whose name is `name`, in `list`, set
    // the value of the first such name-value pair to `value` and remove the
    // others.

    var found = false;

    for (var i = 0; i < list.length;) {
      var cur = list[i];

      if (cur === name) {
        if (!found) {
          list[i + 1] = value;
          found = true;
          i += 2;
        } else {
          list.splice(i, 2);
        }
      } else {
        i += 2;
      }
    } // Otherwise, append a new name-value pair whose name is `name` and value
    // is `value`, to `list`.


    if (!found) {
      list.push(name, value);
    }

    update(this[context], this);
  },
  sort: function sort() {
    var a = this[searchParams];
    var len = a.length;

    if (len <= 2) {// Nothing needs to be done.
    } else if (len < 100) {
      // 100 is found through testing.
      // Simple stable in-place insertion sort
      // Derived from v8/src/js/array.js
      for (var i = 2; i < len; i += 2) {
        var curKey = a[i];
        var curVal = a[i + 1];
        var j;

        for (j = i - 2; j >= 0; j -= 2) {
          if (a[j] > curKey) {
            a[j + 2] = a[j];
            a[j + 3] = a[j + 1];
          } else {
            break;
          }
        }

        a[j + 2] = curKey;
        a[j + 3] = curVal;
      }
    } else {
      // Bottom-up iterative stable merge sort
      var lBuffer = new Array(len);
      var rBuffer = new Array(len);

      for (var step = 2; step < len; step *= 2) {
        for (var start = 0; start < len - 2; start += 2 * step) {
          var mid = start + step;
          var end = mid + step;
          end = end < len ? end : len;
          if (mid > end) continue;
          merge(a, start, mid, end, lBuffer, rBuffer);
        }
      }
    }

    update(this[context], this);
  },
  // https://heycam.github.io/webidl/#es-iterators
  // Define entries here rather than [Symbol.iterator] as the function name
  // must be set to `entries`.
  entries: function entries() {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    return createSearchParamsIterator(this, 'key+value');
  },
  forEach: function forEach(callback) {
    var thisArg = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : undefined;

    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    if (typeof callback !== 'function') {
      throw new errors.TypeError('ERR_INVALID_CALLBACK');
    }

    var list = this[searchParams];
    var i = 0;

    while (i < list.length) {
      var key = list[i];
      var value = list[i + 1];
      callback.call(thisArg, value, key, this); // in case the URL object's `search` is updated

      list = this[searchParams];
      i += 2;
    }
  },
  // https://heycam.github.io/webidl/#es-iterable
  keys: function keys() {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    return createSearchParamsIterator(this, 'key');
  },
  values: function values() {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    return createSearchParamsIterator(this, 'value');
  },
  // https://heycam.github.io/webidl/#es-stringifier
  // https://url.spec.whatwg.org/#urlsearchparams-stringification-behavior
  toString: function toString() {
    if (!this || !this[searchParams] || this[searchParams][searchParams]) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParams');
    }

    return serializeParams(this[searchParams]);
  }
}); // https://heycam.github.io/webidl/#es-iterable-entries

Object.defineProperty(URLSearchParams.prototype, Symbol.iterator, {
  writable: true,
  configurable: true,
  value: URLSearchParams.prototype.entries
}); // https://heycam.github.io/webidl/#dfn-default-iterator-object

function createSearchParamsIterator(target, kind) {
  var iterator = Object.create(URLSearchParamsIteratorPrototype);
  iterator[context] = {
    target: target,
    kind: kind,
    index: 0
  };
  return iterator;
} // https://heycam.github.io/webidl/#dfn-iterator-prototype-object


var URLSearchParamsIteratorPrototype = Object.create(IteratorPrototype);
defineIDLClass(URLSearchParamsIteratorPrototype, 'URLSearchParams Iterator', _defineProperty({
  next: function next() {
    if (!this || Object.getPrototypeOf(this) !== URLSearchParamsIteratorPrototype) {
      throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParamsIterator');
    }

    var _this$context2 = this[context],
        target = _this$context2.target,
        kind = _this$context2.kind,
        index = _this$context2.index;
    var values = target[searchParams];
    var len = values.length;

    if (index >= len) {
      return {
        value: undefined,
        done: true
      };
    }

    var name = values[index];
    var value = values[index + 1];
    this[context].index = index + 2;
    var result;

    if (kind === 'key') {
      result = name;
    } else if (kind === 'value') {
      result = value;
    } else {
      result = [name, value];
    }

    return {
      value: result,
      done: false
    };
  }
}, util.inspect.custom, function (recurseTimes, ctx) {
  if (this == null || this[context] == null || this[context].target == null) throw new errors.TypeError('ERR_INVALID_THIS', 'URLSearchParamsIterator');
  if (typeof recurseTimes === 'number' && recurseTimes < 0) return ctx.stylize('[Object]', 'special');

  var innerOpts = util._extend({}, ctx);

  if (recurseTimes !== null) {
    innerOpts.depth = recurseTimes - 1;
  }

  var _this$context3 = this[context],
      target = _this$context3.target,
      kind = _this$context3.kind,
      index = _this$context3.index;
  var output = target[searchParams].slice(index).reduce(function (prev, cur, i) {
    var key = i % 2 === 0;

    if (kind === 'key' && key) {
      prev.push(cur);
    } else if (kind === 'value' && !key) {
      prev.push(cur);
    } else if (kind === 'key+value' && !key) {
      prev.push([target[searchParams][index + i - 1], cur]);
    }

    return prev;
  }, []);
  var breakLn = util.inspect(output, innerOpts).includes('\n');
  var outputStrs = output.map(function (p) {
    return util.inspect(p, innerOpts);
  });
  var outputStr;

  if (breakLn) {
    outputStr = "\n  ".concat(outputStrs.join(',\n  '));
  } else {
    outputStr = " ".concat(outputStrs.join(', '));
  }

  return "".concat(this[Symbol.toStringTag], " {").concat(outputStr, " }");
}));

function domainToASCII(domain) {
  if (arguments.length < 1) throw new errors.TypeError('ERR_MISSING_ARGS', 'domain'); // toUSVString is not needed.

  return _domainToASCII("".concat(domain));
}

function domainToUnicode(domain) {
  if (arguments.length < 1) throw new errors.TypeError('ERR_MISSING_ARGS', 'domain'); // toUSVString is not needed.

  return _domainToUnicode("".concat(domain));
} // Utility function that converts a URL object into an ordinary
// options object as expected by the http.request and https.request
// APIs.


function urlToOptions(url) {
  var options = {
    protocol: url.protocol,
    hostname: url.hostname,
    hash: url.hash,
    search: url.search,
    pathname: url.pathname,
    path: "".concat(url.pathname).concat(url.search),
    href: url.href
  };

  if (url.port !== '') {
    options.port = Number(url.port);
  }

  if (url.username || url.password) {
    options.auth = "".concat(url.username, ":").concat(url.password);
  }

  return options;
}

function getPathFromURLWin32(url) {
  var hostname = url.hostname;
  var pathname = url.pathname;

  for (var n = 0; n < pathname.length; n++) {
    if (pathname[n] === '%') {
      var third = pathname.codePointAt(n + 2) | 0x20;

      if (pathname[n + 1] === '2' && third === 102 || // 2f 2F /
      pathname[n + 1] === '5' && third === 99) {
        // 5c 5C \
        return new errors.TypeError('ERR_INVALID_FILE_URL_PATH', 'must not include encoded \\ or / characters');
      }
    }
  }

  pathname = decodeURIComponent(pathname);

  if (hostname !== '') {
    // If hostname is set, then we have a UNC path
    // Pass the hostname through domainToUnicode just in case
    // it is an IDN using punycode encoding. We do not need to worry
    // about percent encoding because the URL parser will have
    // already taken care of that for us. Note that this only
    // causes IDNs with an appropriate `xn--` prefix to be decoded.
    return "//".concat(domainToUnicode(hostname)).concat(pathname);
  } else {
    // Otherwise, it's a local path that requires a drive letter
    var letter = pathname.codePointAt(1) | 0x20;
    var sep = pathname[2];

    if (letter < 97 || letter > 122 || // a..z A..Z
    sep !== ':') {
      return new errors.TypeError('ERR_INVALID_FILE_URL_PATH', 'must be absolute');
    }

    return pathname.slice(1);
  }
}

function getPathFromURLPosix(url) {
  if (url.hostname !== '') {
    return new errors.TypeError('ERR_INVALID_FILE_URL_HOST', "must be \"localhost\" or empty on ".concat(platform));
  }

  var pathname = url.pathname;

  for (var n = 0; n < pathname.length; n++) {
    if (pathname[n] === '%') {
      var third = pathname.codePointAt(n + 2) | 0x20;

      if (pathname[n + 1] === '2' && third === 102) {
        return new errors.TypeError('ERR_INVALID_FILE_URL_PATH', 'must not include encoded / characters');
      }
    }
  }

  return decodeURIComponent(pathname);
}

function getPathFromURL(path) {
  if (path == null || !path[searchParams] || !path[searchParams][searchParams]) {
    return path;
  }

  if (path.protocol !== 'file:') return new errors.TypeError('ERR_INVALID_URL_SCHEME', 'file');
  return isWindows ? getPathFromURLWin32(path) : getPathFromURLPosix(path);
} // We percent-encode % character when converting from file path to URL,
// as this is the only character that won't be percent encoded by
// default URL percent encoding when pathname is set.


var percentRegEx = /%/g;

function getURLFromFilePath(filepath) {
  var tmp = new URL('file://');
  if (filepath.includes('%')) filepath = filepath.replace(percentRegEx, '%25');
  tmp.pathname = filepath;
  return tmp;
}

function NativeURL(ctx) {
  this[context] = ctx;
}

NativeURL.prototype = URL.prototype;

function constructUrl(flags, protocol, username, password, host, port, path, query, fragment) {
  var ctx = new URLContext();
  ctx.flags = flags;
  ctx.scheme = protocol;
  ctx.username = (flags & URL_FLAGS_HAS_USERNAME) !== 0 ? username : '';
  ctx.password = (flags & URL_FLAGS_HAS_PASSWORD) !== 0 ? password : '';
  ctx.port = port;
  ctx.path = (flags & URL_FLAGS_HAS_PATH) !== 0 ? path : [];
  ctx.query = query;
  ctx.fragment = fragment;
  ctx.host = host;
  var url = new NativeURL(ctx);
  url[searchParams] = new URLSearchParams();
  url[searchParams][context] = url;
  initSearchParams(url[searchParams], query);
  return url;
}

setURLConstructor(constructUrl);
module.exports = {
  toUSVString: toUSVString,
  getPathFromURL: getPathFromURL,
  getURLFromFilePath: getURLFromFilePath,
  URL: URL,
  URLSearchParams: URLSearchParams,
  domainToASCII: domainToASCII,
  domainToUnicode: domainToUnicode,
  urlToOptions: urlToOptions,
  formatSymbol: kFormat,
  searchParamsSymbol: searchParams
};