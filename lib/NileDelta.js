var ModuleName, isNode, moduleName;

isNode = typeof module !== 'undefined' && module.exports;

if (isNode) {
  moduleName = require('../build/Release/NileDelta.node');
}

ModuleName = function() {};

if (isNode) {
  module.exports = ModuleName;
}

ModuleName.hello = function(world) {
  return moduleName.hello(world);
};