'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var is_reused_symbol = Symbol('isReused');

var FreeList =
/*#__PURE__*/
function () {
  function FreeList(name, max, ctor) {
    _classCallCheck(this, FreeList);

    this.name = name;
    this.ctor = ctor;
    this.max = max;
    this.list = [];
  }

  _createClass(FreeList, [{
    key: "alloc",
    value: function alloc() {
      var item;

      if (this.list.length > 0) {
        item = this.list.pop();
        item[is_reused_symbol] = true;
      } else {
        item = this.ctor.apply(this, arguments);
        item[is_reused_symbol] = false;
      }

      return item;
    }
  }, {
    key: "free",
    value: function free(obj) {
      if (this.list.length < this.max) {
        this.list.push(obj);
        return true;
      }

      return false;
    }
  }]);

  return FreeList;
}();

module.exports = {
  FreeList: FreeList,
  symbols: {
    is_reused_symbol: is_reused_symbol
  }
};