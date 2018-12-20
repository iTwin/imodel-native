/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
var path = require('path');
var formatPackageName = require('./formatPackageName.js').formatPackageName;

// Compute the part of the addon name that corresponds to the platform that we are running on right now
function getCurrentPlatformPrefix() {
    const electronVersion = process.versions.electron;
    if (typeof electronVersion !== "undefined") {
        const electronVersionParts = electronVersion.split(".");
        return "e_" + electronVersionParts[0]; // use only major version number
    } 
    
    const nodeVersion = process.version.substring(1).split("."); // strip off the character 'v' from the start of the string
    return "n_" + nodeVersion[0]; // use only major version number
}

// Load the right node addon for this platform (node|electron, OS, cpu)
// @param dir - optional directory from which addon should be loaded
exports.loadNativePlatform = function(dir) {
    const addonDir = dir || path.join("@bentley", "imodeljs-native-platform-api", "lib");
    return require(path.join(addonDir, formatPackageName(getCurrentPlatformPrefix()), "addon", "imodeljs.node"));
}