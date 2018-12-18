/** Format the name of default platform package that should be used for *this* environment. */
exports.formatPackageName = function(version_prefix) {
    return `@bentley/bridge-addon-${version_prefix}-${process.platform}-${process.arch}`;
}

