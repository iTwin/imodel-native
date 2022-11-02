/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
const exec = require("child_process").exec;
const path = require("path");
const fs = require("fs");
const os = require("os");
const version = require("./package.json").version;

// This is the list of all supported platforms. Keep it up to date with the SUPPORTED_PLATFORMs in 'iModelJsNodeAddon/makePackages.py'.
const supportedPlatforms = [
  {
    name: "win32",
    description: "Microsoft Windows",
    architectures: ["x64"]
  },
  {
    name: "linux",
    description: "Linux",
    architectures: ["x64"]
  },
  {
    name: "darwin",
    description: "macOS",
    architectures: ["x64", "arm64"]
  }
];

// Before attempting to install the platform-specific addon, check that this
// platform is supported. If not, exit with a friendly error message.

function showErrorAndExit(msg, alsoShowSupported) {
  console.log("ERROR - iTwin.js cannot be installed.");
  console.log(msg);
  if (alsoShowSupported) {
    console.log("iTwin.js runs on:");
    for (const supportedPlatform of supportedPlatforms) {
      console.log(`\t${supportedPlatform.description} (${supportedPlatform.architectures.join()})`);
    }
  }
  process.exit(1);
}

function checkSupportedPlatform() {
  const knownPlatform = supportedPlatforms.find(p => p.name === process.platform);
  if (!knownPlatform)
    showErrorAndExit(`iTwin.js does not run on the ${process.platform} platform.`, true);

  if (!knownPlatform.architectures.find(a => a === process.arch))
    showErrorAndExit(`iTwin.js does not run on the ${process.arch} version of ${knownPlatform.description}. iTwin.js runs on ${knownPlatform.architectures.join()} only.`);
}

// Utility function to copy a directory and all subdirectories
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

if (process.env.IMODELJS_NATIVE_OVERRIDE !== undefined) {
  // expect IMODELJS_NATIVE_OVERRIDE to have the necessary platform in it already. 
  console.log(`OVERRIDE DETECTED`);
  const overrideFolder = path.resolve(process.env.IMODELJS_NATIVE_OVERRIDE);
  console.log(`Copying contents of folder ${overrideFolder} to ${__dirname}`)
  copyFolderRecursiveSync(overrideFolder, __dirname);
} else {
  // We have to run npm install in a temp directory. If we try to run it in the current directory,
  // it will fight over a (file?) lock that the parent npm install holds.
  // Note that we have to copy the current package.json to the temp directory, or else npm install will object.
  const installDir = path.join(os.tmpdir(), "install-imodeljs-native");
  try { fs.mkdirSync(installDir); } catch (err) { }
  fs.copyFileSync(path.join(__dirname, "package.json"), path.join(installDir, "package.json"));

  // We will then copy the results of the install from the temp directory into sub-directories below this one.
  function installNativePackage() {
    checkSupportedPlatform();

  // This platform is supported. Try to install the appropriate addon package.
  const cmdLine = `npm install --no-save @bentley/imodeljs-${process.platform}-${process.arch}@${version}`;
    console.log(cmdLine);
    exec(cmdLine, { cwd: installDir }, (error, stdout, stderr) => {
      if (error)
        throw error;
      console.log(stdout);
      console.log(stderr);
      copyFolderRecursiveSync(path.join(installDir, "node_modules", "@bentley"), __dirname);
    });
  }

  installNativePackage();
}
