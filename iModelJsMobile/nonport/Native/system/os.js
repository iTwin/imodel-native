'use strict';

function tmpdir() {
    return process.env.TEMP;
}


function hostname() {
    return process.env.HOSTNAME;
}

function userInfo() {
    return { 
	uid: 0, 
	gid: 0,
	username: 'ios',
	homedir: '', 
	shell: 'js' };
}

module.exports = {tmpdir, hostname, userInfo};
