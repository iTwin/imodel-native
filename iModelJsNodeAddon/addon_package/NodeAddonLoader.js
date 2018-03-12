/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
/** Loads the appropriate version of the standard imodeljs-native-platform addon. */
class NodeAddonLoader {

    /** Loads the appropriate version of the addon
     * KEEP THIS CONSISTENT WITH ../MakePackages.py
     * AND WITH imodeljs-core/source/backend/NativePlatformRegistry.ts.
     */
    static loadAddon() {

        if (typeof (process) === "undefined" || process.version === "")
            throw new Error("NodeAddonLoader could not determine process type");
        let versionCode;
        const electronVersion = process.versions.electron;
        if (typeof electronVersion !== "undefined") {
            versionCode = "e_" + electronVersion.replace(/\./g, "_");
        }
        else {
            const nodeVersion = process.version.substring(1).split("."); // strip off the character 'v' from the start of the string
            versionCode = "n_" + nodeVersion[0] + "_" + nodeVersion[1]; // use only major and minor version numbers
        }
        let addonPackage = "@bentley/imodeljs-" + versionCode + "-" + process.platform + "-" + process.arch;
    
        let addonName = addonPackage + "/addon/imodeljs.node";

        return require(addonName);
    }
}

exports.NodeAddonLoader = NodeAddonLoader;
