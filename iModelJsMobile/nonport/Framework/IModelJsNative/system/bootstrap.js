// A script and not a module. This will be executed in global context
Error["captureStackTrace"] = function (targetObject, constructorOpt) { console.log("captureStackTrace()"); };

// CommonJS loader
var module = require('loader');

// Setup require function
require = function(path) {
    return module._load(path,null,true);
};

//Setup require.resolve function
require.resolve = function(path) { 
    return module._resolveFilename(path);
};

global = this;
/*
const InternalModules = {
    electron : {
        app: undefined 
    },
    https : undefined,
    http : undefined,
    zlib: undefined,
    net: undefined,
    crypto: undefined,
    "form-data": undefined,
    vm: global.vm,
    "IModelJsFs": global.IModelJsFsModule,
    "./IModelJsFs": global.IModelJsFsModule,
    "../IModelJsFs": global.IModelJsFsModule,
    "../../IModelJsFs": global.IModelJsFsModule,
    "util_ex": global.util_ex,
    assert: global.assert,
    fs: global.fs,
    native_module: global.native_module,
    os: global.os
};
*/

// load system module
Buffer = require('buffer').Buffer;
console = require('console');
os = require('os');
IModelJsFsModule = {"IModelJsFs": require('fs')};

// setup temp folder for native addon
imodeljsMobile.knownLocations.tempDir = os.tmpdir();

//load backend
require(process.env.BACKEND_URL);



