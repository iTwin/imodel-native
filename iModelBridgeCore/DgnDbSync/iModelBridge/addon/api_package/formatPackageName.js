/** Format the name of default platform package that should be used for *this* environment. */
exports.formatPackageName = function() {return `@bentley/bridge-addon-${process.platform}-${process.arch}`;}
