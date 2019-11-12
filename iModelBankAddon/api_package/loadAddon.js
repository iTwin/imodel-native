/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
const path = require('path');

exports.formatPackageName = function () { return `imodel-bank-${process.platform}-${process.arch}`; }

// Load the right node addon for this platform (OS, cpu)
exports.loadAddon = function () {
    // We make sure we are running in a known platform.
    if (typeof process === "undefined" || process.platform === undefined || process.arch === undefined)
        throw new Error("Error - unknown process");

    const packageName = exports.formatPackageName(); // this includes current platform and CPU
    const addonDir = path.join("@bentley", "imodel-bank", packageName);
    return require(path.join(addonDir, "imodel-bank.node"));
}