/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { assert } from "chai";
import { OpenMode, DbResult } from "@bentley/bentleyjs-core";
import { Db } from "../Db";
import * as fs from "fs-extra";
import * as path from "path";
import { KnownTestLocations } from "./KnownTestLocations";
import { Statement } from "../Statement";
import { it } from "./utils";

try { fs.removeSync(KnownTestLocations.outputDir); } catch (_err) { }
fs.mkdirpSync(KnownTestLocations.outputDir);

it("should work with Db", () => {
    const db = new Db();
    const dbpath: string = path.join(KnownTestLocations.outputDir, "newDb.db");
    db.create(dbpath); // (will throw if create fails)
    assert.isTrue(fs.existsSync(dbpath));
    db.close();
    db.open(dbpath, OpenMode.Readonly); // (will throw if open fails)

    const tableName = "Table1";
    const tableDDL = "name TEXT NOT NULL UNIQUE COLLATE NoCase PRIMARY KEY, description TEXT, version INTEGER";

    try {
        db.createTable(tableName, tableDDL); // (will throw if it fails)
        assert.fail("should fail because db is open readOnly");
    } catch (_err) {
        // exception is expected
    }

    db.close();
    db.open(dbpath, OpenMode.ReadWrite); // (will throw if open fails)
    db.createTable(tableName, tableDDL); // (will throw if it fails)

    const names = ["N", "N2"];
    const descriptions = ["D", "D2"];
    const versions = [1, 2];

    db.withPreparedStatement("INSERT into Table1 (Name,Description,Version) VALUES(?,?,?)", (insertStmt: Statement) => {
        let i: number;
        for (i = 0; i < names.length; ++i) {
            insertStmt.reset();
            insertStmt.bindValues([names[i], descriptions[i], versions[i]]);
            assert.equal(insertStmt.step(), DbResult.BE_SQLITE_DONE);
        }
    });

    db.withPreparedStatement("INSERT into Table1 (Name,Description,Version) VALUES(:Name,:Description,:Version)", (insertStmt: Statement) => {
        names.push("N3");
        descriptions.push("D3");
        versions.push(3);
        insertStmt.bindValues({ Name: names[2], Description: descriptions[2], Version: versions[2] });
        assert.equal(insertStmt.step(), DbResult.BE_SQLITE_DONE);
    });

    db.withPreparedStatement("SELECT * from Table1", (stmt: Statement) => {
        let count = 0;
        while (stmt.step() === DbResult.BE_SQLITE_ROW) {
            const row = stmt.getRow();
            assert.equal(row.name, names[count]);
            assert.equal(row.description, descriptions[count]);
            assert.equal(row.version, versions[count]);
            ++count;
        }
        assert.equal(count, names.length);
    });

    db.withPreparedStatement("SELECT description as d from Table1 WHERE name = :nameParam", (stmt: Statement) => {
        stmt.bindValues({ nameParam: "N2" });
        assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
        assert.equal(stmt.getRow().d, "D2");
    });

    db.withPreparedStatement("SELECT description as d from Table1 WHERE name = :nameParam", (stmt: Statement) => {
        stmt.bindValues({ nameParam: "N3" });
        assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
        assert.equal(stmt.getRow().d, "D3");
    });

    db.withPreparedStatement("DELETE from Table1 WHERE name = :nameParam", (stmt: Statement) => {
        stmt.bindValues({ nameParam: "N3" });
        assert.equal(stmt.step(), DbResult.BE_SQLITE_DONE);
    });

    db.withPreparedStatement("SELECT description as d from Table1 WHERE name = :nameParam", (stmt: Statement) => {
        stmt.bindValues({ nameParam: "N3" });
        assert.equal(stmt.step(), DbResult.BE_SQLITE_DONE);
    });

    db.close();
});

export function intialize() {
}
