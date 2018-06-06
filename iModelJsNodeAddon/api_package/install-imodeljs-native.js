/*--------------------------------------------------------------------------------------+
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
var exec = require('child_process').exec;
var path = require('path');
let version = require("./package.json").version;

// *** KEEP THIS CONSISTENT WITH iModelJsNodeAddon/MakePackages.py IN MERCURIAL ***

let currdir = path.join(process.cwd(), "..", "..", ".."); // See comment below
console.log("In " + currdir);

function installNativePlatformPackage(version_prefix) {
    let cmdLine = 'npm install @bentley/bentleyjs-core'; // install --no-save ' + `@bentley/imodeljs-${version_prefix}-${process.platform}-${process.arch}@${version}`;
    console.log(cmdLine);
    exec(cmdLine, { cwd: currdir }, (error, stdout, stderr) => {
        if (error)
            throw error;
        console.log(stdout);
        console.log(stderr);
    });
}

installNativePlatformPackage('n_8');
if (process.platform.toLowerCase() != 'linux')
    installNativePlatformPackage('e_2');

// Why set cwd to {_dirname}../../.. ?
// We know that this script is in <somewhere>/node_modules/@bentley/imodeljs-native-platform-api
// We want to install the native addon packages in <somewhere>