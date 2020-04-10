
'use strict';
const pretty = require('js_pretty_print').pretty;

function log(str) {
    process.log("LOG", pretty(str));
}

function warn(str) {
    return process.log("WARN", pretty(str));
}

function error(str) {
    return process.log("ERROR", pretty(str));
}
function print(obj) {
    return log(pretty(obj));
}
module.exports = {log, warn, error, print};
