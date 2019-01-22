/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
const path = require('path');

exports.formatPackageName = function () { return `imodeljs-${process.platform}-${process.arch}`; }

// Load the right node addon for this platform (OS, cpu)
exports.loadNativePlatform = function () {
  // We make sure we are running in a known platform.
  if (typeof process === "undefined" || process.platform === undefined || process.arch === undefined)
    throw new Error("Error - unknown process");

  const packageName = exports.formatPackageName(); // this includes current platform and CPU
  const addonDir = path.join("@bentley", "imodeljs-native", packageName);
  return require(path.join(addonDir, "imodeljs.node"));
}