/** Loads the appropriate version of the default imodeljs-node addon. In order to use this class, an app must have an NPM dependency
 * on this package (imodeljs-nodeaddon). This package, in turn, depends on various versions of the default addon. When the app is NPM-installed,
 * these dependencies tell NPM which addon to download and install. NPM puts it into place in the node_modules/@bentley subdirectory.
 * That is why NodeAddonLoader does not take or compute a filepath to the addon - it just lets 'require' find the addon in node_modules in the usual way.
 */
export declare class NodeAddonLoader {
    /** Loads the appropriate version of the addon */
    static loadAddon(): any | undefined;
}
