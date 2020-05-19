/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import * as Mocha from "mocha";
import * as fs from "fs";
import * as path from "path";

// Run mocha tests on all *.test.ts files
function runMochaTests() {
    const mocha = new Mocha();
    mocha.options = {
        useColors: true,
    };

    // Gather up all *.test.js files
    fs.readdirSync(__dirname).filter((file) => file.toLowerCase().endsWith("test.js")).forEach((file) => {
        mocha.addFile(path.join(__dirname, file));
    });

    mocha.run((failures) => {
        process.exitCode = failures ? 1 : 0;  // exit with non-zero status if there were failures
    });
}

runMochaTests();
