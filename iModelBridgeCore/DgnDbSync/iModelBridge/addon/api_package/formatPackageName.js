/** Format the name of default platform package that should be used for *this* environment.
 * This method uses the same naming formula that is used by the bb part that generates and
 * publishes the default platform packages (iModelJsNodeAddon:MakePackages).
 */
exports.formatPackageName = function(version_prefix) {
    return `@bentley/bridge-addon-${version_prefix}-${process.platform}-${process.arch}`;
}

