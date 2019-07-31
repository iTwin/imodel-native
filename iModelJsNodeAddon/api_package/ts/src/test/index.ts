/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import * as utils from "./utils";
import { assert, DbResult, OpenMode } from "@bentley/bentleyjs-core";
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

// Run the tests
imodeljsNative = utils.loadAddon();
dgndb = openDgnDb(utils.dbFileName);

testSimpleDbQueries();
