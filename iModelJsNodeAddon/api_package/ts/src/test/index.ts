/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import { logTest, loadAddon, dbFileName } from "./utils";
import { OpenMode, DbResult, Id64Array } from "@bentley/bentleyjs-core";
import { IModelJsNative } from "../IModelJsNative";
import { assert } from "chai";

let imodeljsNative: typeof IModelJsNative;
let dgndb: IModelJsNative.DgnDb;

export function openDgnDb(filename: string): any {
    logTest("Opening " + filename);

    const db = new imodeljsNative.DgnDb();
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
    const statement = new imodeljsNative.ECSqlStatement();
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

// Run the tests
imodeljsNative = loadAddon();
dgndb = openDgnDb(dbFileName);

testSimpleDbQueries();
testExportGraphicsBasics();
