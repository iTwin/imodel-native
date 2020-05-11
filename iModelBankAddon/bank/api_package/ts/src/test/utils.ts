import * as path from "path";
import * as fs from "fs";
import { Logger, LogLevel } from "@bentley/bentleyjs-core";
import { assert } from "chai";
import { IModelBankNative } from "../IModelBankNative";

Logger.initializeToConsole();
Logger.setLevelDefault(LogLevel.Info);

// Parse command-line arguments
let useLocalBuild = true;
if (process.argv.length > 2) {
  for (let i = 3; i < process.argv.length; ++i) {
    if ((process.argv[i] === "-i") || (process.argv[i] === "--installed")) {
      useLocalBuild = false;
    }
  }
}

export function loadInstalledAddon(dir?: string): typeof IModelBankNative {
  return require("@bentley/imodel-bank/loadAddon.js").loadAddon(dir);
}

export function computeGeneratedPkgsDir(): string {
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

  return path.join(process.env.OutRoot, platformSubDir, "imodelbankaddon_pkgs");
}

export function loadLocalBuildOfAddon(): typeof IModelBankNative {
  const generatedPkgsDir = computeGeneratedPkgsDir();

  const apiPkgDir = path.join(generatedPkgsDir, "imodel-bank");

  assert(fs.existsSync(generatedPkgsDir), `${apiPkgDir} - local build of imodel-bank not found`);

  const formatPackageName = require(path.join(apiPkgDir, "loadAddon.js")).formatPackageName;

  const nativePackageName = formatPackageName();

  const addonFile = path.join(generatedPkgsDir, nativePackageName, "imodel-bank.node");

  assert(fs.existsSync(addonFile), `${addonFile} - local build of imodel-bank.node not found`);

  return require(addonFile);
}

export function logTest(msg: string) {
  // tslint:disable-next-line:no-console
  console.log(msg);
}

export function loadAddon(): typeof IModelBankNative {
  return useLocalBuild ? loadLocalBuildOfAddon() : loadInstalledAddon();
}
