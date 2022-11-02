/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

// console.log('Platform: ', process.platform);
// console.log('Node version: ', process.version);
// console.log('Node arch: ', process.arch);
// console.log('Node dependencies: ', process.versions);

import * as fs from "fs";
import * as Mocha from "mocha";
import * as path from "path";
import { Logger, LogLevel, OpenMode } from "@itwin/core-bentley";
import { iModelJsNative } from "./utils";

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

Logger.initializeToConsole();
Logger.setLevelDefault(LogLevel.Warning);
iModelJsNative.logger = Logger;

export function openDgnDb(filename: string) {
  const db = new iModelJsNative.DgnDb();
  db.openIModel(filename, OpenMode.ReadWrite);
  return db;
}

// Run the tests
runMochaTests();
