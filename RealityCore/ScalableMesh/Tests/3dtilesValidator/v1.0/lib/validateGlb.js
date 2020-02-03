'use strict';
var fs = require('fs');
var os = require('os');
var path = require('path');
var uuid = require('uuid');
var childProcess = require('child_process');
var Cesium = require('cesium');
var fileExist = require('file-exists');
var Promise = require('bluebird');
var queue = require('childprocess-queue');

var defined = Cesium.defined;

module.exports = validateGlb;

//var gltfValidatorPath = path.join (__dirname, '../../gltf_validator/bin/gltf_validator.dart');
var gltfValidatorPath = 'gltf_validator';
var glbfilepath = path.join(os.tmpdir(), 'temp_glb_file_');
var id = 0;

/**
 * Check if the glb is valid binary glTF.
 *
 * @param {Buffer} glb The glb buffer.
 * @returns {String} An error message if validation fails, otherwise undefined.
 */
function validateGlb(glb) {
    var version = glb.readUInt32LE(4);
    var message = '';

    if (version !== 1) {
        message = 'Invalid Glb version: ' + version + '. Version must be 1.';
        return Promise.resolve(message);
    }

    //if (fileExist.sync(gltfValidatorPath)) {
		id += 1;
		var filename = glbfilepath + id.toString() + '.glb';
        var filehandle = fs.openSync(filename, 'w+');
        fs.writeSync(filehandle, glb, 0, glb.length, 0);
        fs.closeSync(filehandle);
        //var child = childProcess.spawn('cmd', ['/s', '/c', gltfValidatorPath, glbfilepath]);
        return new Promise(function (resolve, reject) {
			queue.spawn('cmd', ['/s', '/c', gltfValidatorPath, filename], { onCreate: function (child){
			//queue.spawn('cmd', ['/s', '/c', 'dart.exe', gltfValidatorPath, filename], { onCreate: function (child){
				child.stdout.on('data', function(data) {
					message += data.toString();
				});
				child.on('exit', function (code) {
					process.stdout.write('\r                    \rRemaining: ' + queue.getCurrentQueueSize().toString());
			        fs.unlink(filename, (err) => {
						if (err) {
							message = 'Unable to delete file: ' + filename;
							resolve(message);
						}
						var obj = JSON.parse(message);
						if (obj.result === "ERROR") {
							message = JSON.stringify(obj.errors);
							resolve(message);
						}
						if (code == 0) {
							message = undefined;
							resolve(message);
						}
						else {
							if (message == undefined) {message = 'Input GLTF is invalid';}
							resolve(message);
						}						
					});
				});			
			}});
        });
    //}
}