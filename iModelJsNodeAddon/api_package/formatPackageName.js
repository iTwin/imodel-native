/** Format the name of default platform package that should be used for this environment. */
exports.formatPackageName = function() {return `@bentley/imodeljs-${process.platform}-${process.arch}`;}
