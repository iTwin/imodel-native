/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
/** Loads the appropriate version of the default imodeljs-node addon. In order to use this class, an app must have an NPM dependency
 * on this package (imodeljs-nodeaddon). This package, in turn, depends on various versions of the default addon. When the app is NPM-installed,
 * these dependencies tell NPM which addon to download and install. NPM puts it into place in the node_modules/@bentley subdirectory of the parent.
 * That is why NodeAddonLoader does not take or compute a filepath to the addon - it just lets 'require' find the addon in node_modules in the usual way.
 */
class NodeAddonLoader {

    /** Loads the appropriate version of the addon
     * KEEP THIS CONSISTENT WITH ../MakePackages.py
     * AND WITH imodeljs-core/source/backend/AddonRegistry.ts.
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
