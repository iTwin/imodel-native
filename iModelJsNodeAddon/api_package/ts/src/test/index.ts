/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import * as utils from "./utils";
import { assert, DbResult, OpenMode, Id64Array } from "@bentley/bentleyjs-core";
import { IModelJsNative } from "../IModelJsNative";

// tslint:disable:no-console

let imodeljsNative: typeof IModelJsNative;
let dgndb: IModelJsNative.DgnDb;

export function openDgnDb(filename: string): any {
    utils.logTest("Opening " + filename);

    const db: IModelJsNative.DgnDb = new imodeljsNative.DgnDb();
    const res: DbResult = db.openIModel(filename, OpenMode.Readonly);
    assert(res === 0, `${filename} - open failed with status = ${res}`);

    return db;
}

function testSimpleDbQueries() {
    utils.logTest("testSimpleDbQueries");
    assert(dgndb.isOpen());
    assert(dgndb.isReadonly());
    assert(!dgndb.isRedoPossible());
}

function testExportGraphicsBasics() {
    utils.logTest("testExportGraphicsBasics");
    // Find all 3D elements in the test file
    const elementIdArray: Id64Array = [];
    const statement = new imodeljsNative.ECSqlStatement();
    statement.prepare(dgndb, "SELECT ECInstanceId FROM bis.GeometricElement3d");
    while (statement.step() === DbResult.BE_SQLITE_ROW)
        elementIdArray.push(statement.getValue(0).getId());
    assert(elementIdArray.length > 0, "No 3D elements in test file");
    // Expect a mesh to be generated for each element - valid for test.bim, maybe invalid for future test data
    const elementsWithGraphics: any = {};
    const onGraphics = (info: any) => { elementsWithGraphics[info.elementId] = true; };

    const res: DbResult = dgndb.exportGraphics({ elementIdArray, onGraphics });

    assert(res === 0, `IModelDb.exportGraphics returned ${res}`);
    for (const id of elementIdArray)
        assert(elementsWithGraphics[id] !== undefined, `No graphics generated for ${id}`);
}

// Run the tests
imodeljsNative = utils.loadAddon();
dgndb = openDgnDb(utils.dbFileName);

testSimpleDbQueries();
testExportGraphicsBasics();
