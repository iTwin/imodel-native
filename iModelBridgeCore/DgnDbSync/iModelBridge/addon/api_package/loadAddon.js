/*--------------------------------------------------------------------------------------+
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
var path = require('path');
var formatPackageName = require('./formatPackageName.js').formatPackageName;

// Load the right node addon for this platform (node|electron, OS, cpu)
// @param dir - optional directory from which addon should be loaded
exports.loadAddon = function (dir) {
    return require(path.join(dir || "", formatPackageName(), "addon", "bridge-addon.node"));
}