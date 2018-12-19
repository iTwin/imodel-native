/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
const exec = require('child_process').exec;
const path = require('path');
const fs = require('fs');
const os = require('os');
const version = require("./package.json").version;
const formatPackageName = require('./formatPackageName.js').formatPackageName;

function copyFolderRecursiveSync(source, target) {
    if (!fs.existsSync(target))
        fs.mkdirSync(target);

    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach(function (file) {
            var curSource = path.join(source, file);
            if (fs.lstatSync(curSource).isDirectory()) {
                copyFolderRecursiveSync(curSource, path.join(target, path.basename(curSource)));
            } else {
                fs.writeFileSync(path.join(target, file), fs.readFileSync(curSource));
            }
        });
    }
}

// We have to run npm install in a temp directory. If we try to run it in the current directory,
// it will fight over a (file?) lock that the parent npm install holds.
// Note that we have to copy the current package.json to the temp directory, or else npm install will object.
const installDir = path.join(os.tmpdir(), "install-imodeljs-native");
try { fs.mkdirSync(installDir); } catch (err) { }
fs.copyFileSync(path.join(__dirname, "package.json"),  path.join(installDir, "package.json"));

// We will then copy the results of the install from the temp directory into the targetDirectory
// We know that this script is in: <somewhere>/node_modules/@bentley/imodeljs-native-platform-api
// We want to put the native addon packages in:  <somewhere>/node_modules/@bentley/imodeljs-native-platform-api/lib
const currdir = process.cwd();
const targetDir = path.normalize(path.join(currdir, "lib")); // See comment below

// Install in tmp dir and copy into target dir
function installNativePlatformPackage(packages) {
    const cmdLine = `npm install --no-save ${packages}`;
    console.log(cmdLine);
    exec(cmdLine, { cwd: installDir }, (error, stdout, stderr) => {
        if (error)
            throw error;
        console.log(stdout);
        console.log(stderr);
        copyFolderRecursiveSync(path.join(installDir, 'node_modules'), targetDir);
    });
}

installNativePlatformPackage(`${formatPackageName()}@${version}`);
