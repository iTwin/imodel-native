/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { use as chaiuse } from "chai";
import * as chaiAsPromised from "chai-as-promised";
import * as fs from "fs";
import * as Mocha from "mocha";
import * as path from "path";
import { Logger, LogLevel, OpenMode } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { iModelJsNative } from "./utils";

import type { UpgradeOptions } from "@itwin/core-common";
// Run mocha tests on all *.test.ts files
function runMochaTests() {
  const mocha = new Mocha();
  mocha.options = {
    useColors: true,
    // timeout: 0, For some reason mocha does not care what timeout you pass here.
  };
  mocha.suite.timeout(0);

  // Gather up all *.test.js files
  fs.readdirSync(__dirname).filter((file) => file.toLowerCase().endsWith("test.js")).forEach((file) => {
    mocha.addFile(path.join(__dirname, file));
  });

  mocha.run((failures) => {
    process.exitCode = failures ? 1 : 0;  // exit with non-zero status if there were failures
  });
}

chaiuse(chaiAsPromised);

Logger.initializeToConsole();
Logger.setLevelDefault(LogLevel.Error);
iModelJsNative.logger = {
  // note: using private Logger fields is temporary until the version of `@itwin/core-bentley` is updated
  // to a version where Logger has them public
  get minLevel() {
    return (Logger as any)._minLevel;
  },
  get categoryFilter() {
    return [...((Logger as any)._categoryFilter as Map<string, LogLevel>).entries()].reduce(
      (categoryFilter, [categoryName, logLevel]) => ({ ...categoryFilter, [categoryName]: logLevel }),
      {},
    );
  },
  logTrace: (c, m) => Logger.logTrace(c, m),
  logInfo: (c, m) => Logger.logInfo(c, m),
  logWarning: (c, m) => Logger.logWarning(c, m),
  logError: (c, m) => Logger.logError(c, m),
};

export function openDgnDb(filename: string, upgradeOptions?: UpgradeOptions & IModelJsNative.SchemaImportOptions) {
  const db = new iModelJsNative.DgnDb();
  db.openIModel(filename, OpenMode.ReadWrite, upgradeOptions);
  return db;
}

// Run the tests
runMochaTests();
