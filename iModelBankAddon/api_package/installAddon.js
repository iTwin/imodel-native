/*--------------------------------------------------------------------------------------+
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
const exec = require("child_process").exec;
const path = require("path");
const fs = require("fs");
const os = require("os");
const version = require("./package.json").version;
const formatPackageName = require("./loadAddon.js").formatPackageName;

function copyFolderRecursiveSync(source, target) {
    if (!fs.existsSync(target))
        fs.mkdirSync(target);

    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach(function (file) {
            const curSource = path.join(source, file);
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
const installDir = path.join(os.tmpdir(), "install-imodel-bank");
try { fs.mkdirSync(installDir); } catch (err) { }
fs.copyFileSync(path.join(__dirname, "package.json"), path.join(installDir, "package.json"));

// We will then copy the results of the install from the temp directory into sub-directories below this one.
function installNativePackage(package) {
    const cmdLine = `npm install --no-save @bentley/${package}`;
    console.log(cmdLine);
    exec(cmdLine, { cwd: installDir }, (error, stdout, stderr) => {
        if (error)
            throw error;
        console.log(stdout);
        console.log(stderr);
        copyFolderRecursiveSync(path.join(installDir, "node_modules", "@bentley"), __dirname);
    });
}

installNativePackage(`${formatPackageName()}@${version}`);
