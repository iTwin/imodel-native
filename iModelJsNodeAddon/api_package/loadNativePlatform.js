/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
const path = require('path');
const formatPackageName = require('./formatPackageName.js').formatPackageName;

// Load the right node addon for this platform (node|electron, OS, cpu)
// @param dir - optional directory from which addon should be loaded
exports.loadNativePlatform = function(dir) {
    const packageName = formatPackageName(); // this includes current platform and CPU

    // Under windows, we need to find the correct version of Napi.dll, depending on whether we're running under
    // Node or Electron. [N.B. The only difference is that Napi.dll forwards its exports to "Node.exe" if we're
    // under Node, and "Node.dll" under Electron.] To do that we prepend the appropriate subdirectory to the PATH
    // so the loader will find the correct one.
    // Note: this is only necessary under Windows.
    if (process.platform == "win32") {
        const engine = (typeof process.versions.electron !== "undefined") ? "Electron" : "Node";
        process.env.path = path.join(__dirname, "lib", packageName, engine) + ";" + process.env.path;
    }

    const addonDir = dir || path.join("@bentley", "imodeljs-native-platform-api", "lib", packageName);
    return require(path.join(addonDir, "imodeljs.node"));
}