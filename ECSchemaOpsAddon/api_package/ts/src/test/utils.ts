/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import * as path from "path";
import * as fs from "fs";
import { Logger, LogLevel } from "@bentley/bentleyjs-core";
import { assert } from "chai";
import { ECSchemaOpsNative } from "../ECSchemaOpsNative";

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

export function loadInstalledAddon(dir?: string): typeof ECSchemaOpsNative {
  return require("@bentley/ecschema-ops/loadNativePlatform.js").loadAddon(dir);
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

  return path.join(process.env.OutRoot, platformSubDir, "ecschemaopsaddon_pkgs");
}

export function loadLocalBuildOfAddon(): typeof ECSchemaOpsNative {
  const generatedPkgsDir = computeGeneratedPkgsDir();

  const apiPkgDir = path.join(generatedPkgsDir, "ecschema-ops");

  assert(fs.existsSync(generatedPkgsDir), `${apiPkgDir} - local build of ecschema-ops not found`);

  const formatPackageName = require(path.join(apiPkgDir, "loadNativePlatform.js")).formatPackageName;

  const nativePackageName = formatPackageName();

  const addonFile = path.join(generatedPkgsDir, nativePackageName, "ecschema-ops.node");

  assert(fs.existsSync(addonFile), `${addonFile} - local build of ecschema-ops.node not found`);

  return require(addonFile);
}

export function logTest(msg: string) {
  // tslint:disable-next-line:no-console
  console.log(msg);
}

export function getTestAssetsDir(): string {
  return path.join(__dirname, "assets");
}

export function loadAddon(): typeof ECSchemaOpsNative {
  return useLocalBuild ? loadLocalBuildOfAddon() : loadInstalledAddon();
}

export const ecSchemaOpsNative: typeof ECSchemaOpsNative = loadAddon();
