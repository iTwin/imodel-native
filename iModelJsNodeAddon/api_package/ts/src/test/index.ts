/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import { logTest, iModelJsNative, dbFileName } from "./utils";
import { OpenMode, DbResult, Id64Array } from "@bentley/bentleyjs-core";
import { IModelJsNative } from "../NativeLibrary";
import { assert } from "chai";
import * as Mocha from "mocha";
import * as fs from "fs";
import * as path from "path";

let dgndb: IModelJsNative.DgnDb;

export function openDgnDb(filename: string): any {
    logTest("Opening " + filename);

    const db = new iModelJsNative.DgnDb();
    const res = db.openIModel(filename, OpenMode.ReadWrite);
    assert.equal(res, 0, `${filename} - open failed with status = ${res}`);
    return db;
}

function testSimpleDbQueries() {
    logTest("testSimpleDbQueries");
    assert.isTrue(dgndb.isOpen());
    assert.isFalse(dgndb.isReadonly());
    assert.isFalse(dgndb.isRedoPossible());
}

function testExportGraphicsBasics() {
    logTest("testExportGraphicsBasics");
    // Find all 3D elements in the test file
    const elementIdArray: Id64Array = [];
    const statement = new iModelJsNative.ECSqlStatement();
    statement.prepare(dgndb, "SELECT ECInstanceId FROM bis.GeometricElement3d");
    while (DbResult.BE_SQLITE_ROW === statement.step())
        elementIdArray.push(statement.getValue(0).getId());
    statement.dispose();

    assert(elementIdArray.length > 0, "No 3D elements in test file");
    // Expect a mesh to be generated for each element - valid for test.bim, maybe invalid for future test data
    const elementsWithGraphics: any = {};
    const onGraphics = (info: any) => { elementsWithGraphics[info.elementId] = true; };
    const res = dgndb.exportGraphics({ elementIdArray, onGraphics });

    assert.equal(res, 0, `IModelDb.exportGraphics returned ${res}`);
    for (const id of elementIdArray)
        assert.isDefined(elementsWithGraphics[id], `No graphics generated for ${id}`);
}

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

// Run the tests
dgndb = openDgnDb(dbFileName);

testSimpleDbQueries();
testExportGraphicsBasics();
runMochaTests();
