import * as path from "path";
import * as fs from "fs";
import { IModelJsNative, NativeLibrary } from "../NativeLibrary";
import { assert } from "@bentley/bentleyjs-core";

// Parse command-line arguments
let useLocalBuild = true;
export let dbFileName: string;
if (process.argv.length <= 2) {
  dbFileName = path.resolve(__dirname, "assets", "test.bim");
} else {

  dbFileName = process.argv[2];

  for (let i = 3; i < process.argv.length; ++i) {
    if ((process.argv[i] === "-i") || (process.argv[i] === "--installed")) {
      useLocalBuild = false;
    }
  }
}

export function loadInstalledAddon(): typeof IModelJsNative {
  return require(NativeLibrary.libraryName);
}

export function loadLocalBuildOfAddon(): any {
  if (process.env.OutRoot === undefined) {
    throw new Error("You must define 'OutRoot' in your environment");
  }

  const platformSubDirs = {
    win32: "Winx64",
    linux: "LinuxX64",
    darwin: "MacOSX64",
  };

  if (!platformSubDirs.hasOwnProperty(process.platform)) {
    throw new Error(process.platform + " - Unsupported platform");
  }

  const platformSubDir: NodeJS.Platform = (platformSubDirs as any)[process.platform];

  const generatedPkgsDir = path.join(process.env.OutRoot, platformSubDir, "imodeljsnodeaddon_pkgs");

  const apiPkgDir = path.join(generatedPkgsDir, "imodeljs-native");

  assert(fs.existsSync(generatedPkgsDir), `${apiPkgDir} - local build of imodeljs-native not found`);

  const addonFile = path.join(generatedPkgsDir, NativeLibrary.archName, NativeLibrary.nodeAddonName);
  assert(fs.existsSync(addonFile), `${addonFile} - local build of imodeljs.node not found`);

  return require(addonFile);
}

export function logTest(msg: string) {
  // tslint:disable-next-line:no-console
  console.log("Test: " + msg);
}

function loadAddon() {
  return useLocalBuild ? loadLocalBuildOfAddon() : loadInstalledAddon();
}

export const iModelJsNative: typeof IModelJsNative = loadAddon();
