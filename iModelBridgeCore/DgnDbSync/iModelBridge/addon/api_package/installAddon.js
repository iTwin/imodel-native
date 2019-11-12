/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
var exec = require('child_process').exec;
var path = require('path');
var fs = require('fs');
var os = require('os');
let version = require("./package.json").version;
var formatPackageName = require('./formatPackageName.js').formatPackageName;

function copyFolderRecursiveSync(source, target) {
    var targetFolder = path.join(target, path.basename(source));
    if (!fs.existsSync(targetFolder))
        fs.mkdirSync(targetFolder);

    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach(function (file) {
            var curSource = path.join(source, file);
            if (fs.lstatSync(curSource).isDirectory()) {
                copyFolderRecursiveSync(curSource, targetFolder);
            } else {
                fs.writeFileSync(path.join(targetFolder, file), fs.readFileSync(curSource));
            }
        });
    }
}

// We have to run npm install in a temp directory. If we try to run it in the current directory,
// it will fight over a (file?) lock that the parent npm install holds.
// Note that we have to copy the current package.json to the temp directory, or else npm install will object.
let installDir = path.join(os.tmpdir(), "install-bridge-addon");
try { fs.mkdirSync(installDir); } catch (err) { }
fs.copyFileSync(path.join(__dirname, "package.json"), path.join(installDir, "package.json"));

// We will then copy the results of the install from teh temp directory into the targetDirectory
// We know that this script is in:               <somewhere>/node_modules/@bentley/imodeljs-bridge
// We want to put the native addon packages in:  <somewhere>/node_modules
let currdir = process.cwd();
let targetDir = path.normalize(path.join(currdir, "..", "..", "..")); // See comment below

// Install in tmp dir and copy into target dir
function installNativePlatformPackage() {
    let cmdLine = `npm install --no-save ${formatPackageName()}@${version}`;
    console.log(cmdLine);
    exec(cmdLine, { cwd: installDir }, (error, stdout, stderr) => {
        if (error)
            throw error;
        console.log(stdout);
        console.log(stderr);
        copyFolderRecursiveSync(path.join(installDir, 'node_modules'), targetDir);
    });
}

installNativePlatformPackage();
