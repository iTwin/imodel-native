/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
var path = require('path');

/** Compute the name of default platform package that should be used for this environment.
 * This method uses the same naming formula that is used by the bb part that generates and
 * publishes the default platform packages (iModelJsNodeAddon:MakePackages).
 */
function computeDefaultImodelNodeAddonPackageName() {

    // *** KEEP THIS CONSISTENT WITH iModelJsNodeAddon/MakePackages.py IN MERCURIAL ***

    let version_prefix;
    const electronVersion = process.versions.electron;
    if (typeof electronVersion !== "undefined") {
        const electronVersionParts = electronVersion.split(".");
        version_prefix = "e_" + electronVersionParts[0]; // use only major version number
    } else {
        const nodeVersion = process.version.substring(1).split("."); // strip off the character 'v' from the start of the string
        version_prefix = "n_" + nodeVersion[0]; // use only major version number
    }
    return `"@bentley/imodeljs-${version_prefix}-${process.platform}-${process.arch}`;
}

/** Compute the name of default addon that should be used for this environment.
 * This method uses the same naming formula that is used by the bb part that generates and
 * publishes the default platform packages (iModelJsNodeAddon:MakePackages).
 */
function computeDefaultImodelNodeAddonName() {
    return path.join(NodeAddonPackageName.computeDefaultImodelNodeAddonPackageName(), "addon", "imodeljs.node");
}
