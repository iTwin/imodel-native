'use strict';

function tmpdir() {
    return process.env.TEMP;
}


function hostname() {
    return process.env.HOSTNAME;
}


module.exports = {tmpdir, hostname};
