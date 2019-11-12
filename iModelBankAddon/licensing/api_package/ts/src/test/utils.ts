import * as path from "path";
import * as fs from "fs";
import { Logger, LogLevel } from "@bentley/bentleyjs-core";
import { assert } from "chai";
import { IModelBankLicensingNative } from "../IModelBankLicensingNative";

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

export function loadInstalledAddon(dir?: string): typeof IModelBankLicensingNative {
  return require("@bentley/imodel-bank-licensing/loadAddon.js").loadAddon(dir);
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

  return path.join(process.env.OutRoot, platformSubDir, "imodelbanklicensingaddon_pkgs");
}

export function loadLocalBuildOfAddon(): typeof IModelBankLicensingNative {
  const generatedPkgsDir = computeGeneratedPkgsDir();

  const apiPkgDir = path.join(generatedPkgsDir, "imodel-bank-licensing");

  assert(fs.existsSync(generatedPkgsDir), `${apiPkgDir} - local build of imodel-bank-licensing not found`);

  const formatPackageName = require(path.join(apiPkgDir, "loadAddon.js")).formatPackageName;

  const nativePackageName = formatPackageName();

  const addonFile = path.join(generatedPkgsDir, nativePackageName, "imodel-bank-licensing.node");

  assert(fs.existsSync(addonFile), `${addonFile} - local build of imodel-bank-licensing.node not found`);

  return require(addonFile);
}

export function logTest(msg: string) {
  // tslint:disable-next-line:no-console
  console.log(msg);
}

export function loadAddon(): typeof IModelBankLicensingNative {
  return useLocalBuild ? loadLocalBuildOfAddon() : loadInstalledAddon();
}

export const tests: any[] = [];

export function it(nm: string, func: any) {
    tests.push(() => {
        logTest(nm + "...");
        try {
            func();
            logTest(" OK");
        } catch (err) {
            logTest(" ERROR: " + err.message);
        }
    });
}
