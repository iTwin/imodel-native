/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, Guid } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("ECSqlStatement", () => {
  let db: IModelJsNative.ECDb;
  let stmt: IModelJsNative.ECSqlStatement;
  const outDir = getOutputDir();

  beforeEach(() => {
    db = new iModelJsNative.ECDb();
    stmt = new iModelJsNative.ECSqlStatement();
  });

  afterEach(() => {
    stmt.dispose();
    db.dispose();
    db.closeDb();
  });

  function createECDb(directory: string, fileName: string, schemaXml?: string) {
    if (!fs.existsSync(directory))
      fs.mkdirSync(directory);

    const outPath = path.join(directory, fileName);
    if (fs.existsSync(outPath))
      fs.unlinkSync(outPath);

    const ecdb = new iModelJsNative.ECDb();
    ecdb.createDb(outPath);

    if (!schemaXml)
      return ecdb;

    const schemaPath = path.join(directory, `${Guid.createValue()}.ecschema.xml`);
    if (fs.existsSync(schemaPath))
      fs.unlinkSync(schemaPath);

    fs.writeFileSync(schemaPath, schemaXml);

    ecdb.importSchema(schemaPath);
    return ecdb;
  }

  function formatCurrentRow(resp: any, meta: any, useJsName: boolean): any {
    const formattedRow = {};
    for (const prop of meta) {
      const propName = useJsName ? prop.jsonName : prop.name;
      const val = resp[prop.index];
      if (typeof val !== "undefined" && val !== null) {
        Object.defineProperty(formattedRow, propName, {
          value: val,
          enumerable: true,
          writable: true,
        });
      }
    }
    return formattedRow;
  }

  it("should throw exception if toRow is called when statement is not prepared", () => {
    expect(() => stmt.toRow({})).to.throw("ECSqlStatement is not prepared.");
  });

  it("should return correct row when toRow is called after statement is prepared and executed", async () => {
    db = createECDb(outDir, "bindnumbers.ecdb",
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="D" typeName="double"/>
        <ECProperty propertyName="I" typeName="int"/>
        <ECProperty propertyName="L" typeName="long"/>
        <ECProperty propertyName="S" typeName="string"/>
      </ECEntityClass>
      </ECSchema>`);
    assert.isTrue(db.isOpen());

    const doubleVal: number = 3.5;
    stmt.prepare(db, "INSERT INTO Test.Foo(D, I, L, S) VALUES(?, ?, ?, ?)");
    stmt.getBinder(1).bindDouble(doubleVal);
    stmt.getBinder(2).bindDouble(doubleVal);
    stmt.getBinder(3).bindDouble(doubleVal);
    stmt.getBinder(4).bindDouble(doubleVal);
    const r = stmt.stepForInsert();
    assert.equal(r.status, DbResult.BE_SQLITE_DONE);
    const id = r.id;
    stmt.clearBindings();
    stmt.dispose();

    stmt.prepare(db, "SELECT D,I,L,S FROM Test.Foo WHERE ECInstanceId=?");
    stmt.getBinder(1).bindId(id);
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    const args = { classIdsToClassNames: true, useJsName: true };
    const resp = stmt.toRow(args);
    const meta = stmt.getMetadata();
    const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
    assert.equal(row.d, doubleVal);
    assert.equal(row.i, 3);
    assert.equal(row.l, 3);
    assert.equal(row.s, "3.5");
  });

  it("should handle classIdsToClassNames option correctly when toRow is called", async () => {
    const boolVal: boolean = true;
    db = createECDb(outDir, "classIdsToClassNames.ecdb",
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="B0" typeName="boolean"/>
      </ECEntityClass>
      </ECSchema>`);
    assert.isTrue(db.isOpen());

    stmt.prepare(db, "INSERT INTO test.Foo(B0) VALUES(?)");
    stmt.getBinder(1).bindBoolean(boolVal);
    const res = stmt.stepForInsert();
    assert.equal(res.status, DbResult.BE_SQLITE_DONE);
    const id = res.id;
    stmt.clearBindings();
    stmt.dispose();

    stmt.prepare(db, "SELECT ECInstanceId, ECClassId, B0 FROM test.Foo WHERE ECInstanceId=?");
    stmt.getBinder(1).bindId(id);
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);

    {
      const args = { classIdsToClassNames: false, useJsName: false };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.ECInstanceId, id);
      assert.equal(row.ECClassId, "0x58");
      assert.equal(row.B0, boolVal);
    }

    {
      const args = { classIdsToClassNames: false, useJsName: true };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.equal(row.b0, boolVal);
    }

    {
      const args = { classIdsToClassNames: true, useJsName: false };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.ECInstanceId, id);
      assert.equal(row.ECClassId, "Test.Foo");
      assert.equal(row.B0, boolVal);
    }

    {
      const args = { classIdsToClassNames: true, useJsName: true };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.equal(row.b0, boolVal);
    }
  });

  it("should handle rowFormat option correctly when toRow is called", () => {
    const boolVal: boolean = true;
    db = createECDb(outDir, "rowFormat.ecdb",
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="B0" typeName="boolean"/>
      </ECEntityClass>
      </ECSchema>`);
    assert.isTrue(db.isOpen());

    stmt.prepare(db, "INSERT INTO test.Foo(B0) VALUES(?)");
    stmt.getBinder(1).bindBoolean(boolVal);
    const res = stmt.stepForInsert();
    assert.equal(res.status, DbResult.BE_SQLITE_DONE);
    const id = res.id;
    stmt.clearBindings();
    stmt.dispose();

    stmt.prepare(db, "SELECT ECInstanceId, ECClassId, B0 FROM test.Foo WHERE ECInstanceId=?");
    stmt.getBinder(1).bindId(id);
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    {
      const args = { classIdsToClassNames: true, useJsName: true };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.equal(row.b0, boolVal);
    }

    {
      const args = { classIdsToClassNames: true, useJsName: false };
      const resp = stmt.toRow(args);
      const meta = stmt.getMetadata();
      const row = formatCurrentRow(resp.data, meta.meta, args.useJsName);
      assert.equal(row.ECInstanceId, id);
      assert.equal(row.ECClassId, "Test.Foo");
      assert.equal(row.B0, boolVal);
    }
  });

  it("should return correct row metadata when getMetadata is called after statement is prepared and executed", () => {
    const boolVal: boolean = true;
    db = createECDb(outDir, "metaData.ecdb",
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="B0" typeName="boolean"/>
      </ECEntityClass>
      </ECSchema>`);
    assert.isTrue(db.isOpen());

    stmt.prepare(db, "INSERT INTO test.Foo(B0) VALUES(?)");
    stmt.getBinder(1).bindBoolean(boolVal);
    const res = stmt.stepForInsert();
    assert.equal(res.status, DbResult.BE_SQLITE_DONE);
    const id = res.id;
    stmt.clearBindings();
    stmt.dispose();

    stmt.prepare(db, "SELECT ECInstanceId, ECClassId, B0 FROM test.Foo WHERE ECInstanceId=?");
    stmt.getBinder(1).bindId(id);
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);

    const expectedMeta = {
      meta: [
        { className: "", accessString: "ECInstanceId", generated: false, index: 0, jsonName: "id", name: "ECInstanceId", extendedType: "Id", typeName: "long" },
        { className: "", accessString: "ECClassId", generated: false, index: 1, jsonName: "className", name: "ECClassId", extendedType: "ClassId", typeName: "long" },
        { className: "Test:Foo", accessString: "B0", generated: false, index: 2, jsonName: "b0", name: "B0", extendedType: "", typeName: "boolean" },
      ],
    };
    const meta = stmt.getMetadata();
    assert.deepEqual(meta, expectedMeta);
  });

  describe("crash regressions", () => {
    const testSchema =
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="I" typeName="int"/>
        <ECProperty propertyName="S" typeName="string"/>
      </ECEntityClass>
      </ECSchema>`;

    // bindParams used to cast its argument to an object without checking that it is one,
    // so a primitive was reinterpreted as a pointer and dereferenced (e.g. the bytes of a
    // JS string became the address), crashing the process instead of throwing.
    it("should throw instead of crashing when bindParams is given a non-object", () => {
      db = createECDb(outDir, "bindparams_nonobject.ecdb", testSchema);
      stmt.prepare(db, "SELECT I FROM test.Foo WHERE I=?");

      for (const notAnObject of ["AAAAAAAABBBBBBBB", 42, true, 0, "", null, undefined]) {
        expect(() => stmt.bindParams(notAnObject as any), `bindParams(${String(notAnObject)})`).to.throw();
      }

      // a real object in the shape bindParams expects ({ name: { type, value } }) still works.
      // type 4 is ECSqlParam::Type::Integer.
      expect(() => stmt.bindParams({ 1: { type: 4, value: 123 } })).to.not.throw();
    });

    // A binder points at memory owned by the statement. Disposing the statement freed it,
    // and the retained binder then made a virtual call on freed memory.
    it("should throw instead of crashing when a binder is used after the statement is disposed", () => {
      db = createECDb(outDir, "binder_after_dispose.ecdb", testSchema);
      stmt.prepare(db, "SELECT I FROM test.Foo WHERE I=?");
      const binder = stmt.getBinder(1);

      binder.bindInteger(1); // valid while the statement is prepared
      stmt.dispose();

      expect(() => binder.bindInteger(1)).to.throw();
      expect(() => binder.bindString("x")).to.throw();
      expect(() => binder.bindNull()).to.throw();

      stmt.prepare(db, "SELECT S FROM test.Foo WHERE S=?");
      expect(() => binder.bindInteger(1)).to.throw();
      expect(() => stmt.getBinder(1).bindString("x")).to.not.throw();
    });

    // Binders stay usable across reset/clearBindings - only finalizing invalidates them.
    it("should keep binders usable after reset and clearBindings", () => {
      db = createECDb(outDir, "binder_after_reset.ecdb", testSchema);
      stmt.prepare(db, "SELECT I FROM test.Foo WHERE I=?");
      const binder = stmt.getBinder(1);

      binder.bindInteger(1);
      stmt.reset();
      expect(() => binder.bindInteger(2)).to.not.throw();
      stmt.clearBindings();
      expect(() => binder.bindInteger(3)).to.not.throw();
    });

    // Closing the db force-finalized the underlying sqlite statement, leaving this
    // statement dangling; using it afterwards was a use after free.
    it("should throw instead of crashing when a statement outlives its db", () => {
      const ownDb = createECDb(outDir, "stmt_after_dbclose.ecdb", testSchema);
      const ownStmt = new iModelJsNative.ECSqlStatement();
      ownStmt.prepare(ownDb, "SELECT I FROM test.Foo");

      ownDb.closeDb();

      expect(() => ownStmt.step()).to.throw();
      expect(() => ownStmt.getColumnCount()).to.throw();
      expect(() => ownStmt.dispose()).to.not.throw();
    });

    // A compound select whose branches have different column counts is only rejected
    // during preparation, but type resolution indexed across branches while parsing.
    it("should report an error instead of crashing on a compound select with mismatched column counts", () => {
      db = createECDb(outDir, "compound_select_mismatch.ecdb", testSchema);

      const invalid = [
        "SELECT b FROM (SELECT NULL a, NULL b UNION ALL SELECT 1)",
        "SELECT b FROM (SELECT ? a, ? b UNION ALL SELECT 1)",
        "SELECT * FROM (SELECT NULL a, NULL b UNION ALL SELECT 1)",
        "SELECT b FROM (SELECT NULL a, NULL b EXCEPT SELECT 1)",
        "SELECT b FROM (SELECT NULL a, NULL b INTERSECT SELECT 1)",
        "WITH cte AS (SELECT NULL a, NULL b UNION ALL SELECT 1) SELECT * FROM cte",
        "WITH cte(x,y) AS (SELECT NULL, NULL UNION ALL SELECT 1) SELECT * FROM cte",
      ];

      for (const ecsql of invalid) {
        const s = new iModelJsNative.ECSqlStatement();
        const status = (() => {
          try {
            return s.prepare(db, ecsql, false).status;
          } finally {
            s.dispose();
          }
        })();
        expect(status, ecsql).to.not.equal(DbResult.BE_SQLITE_OK);
      }
    });

    // The lexer was handed one character per read, which made it re-shift the pending
    // token for every character - quadratic in the length of a single token. A megabyte
    // sized identifier or string literal effectively hung the parser.
    it("should parse very long tokens in linear time", () => {
      db = createECDb(outDir, "long_tokens.ecdb", testSchema);

      const prepare = (ecsql: string) => {
        const s = new iModelJsNative.ECSqlStatement();
        try {
          s.prepare(db, ecsql, false);
        } finally {
          s.dispose();
        }
      };

      const start = Date.now();
      prepare(`SELECT [${"A".repeat(1000 * 1000)}] FROM test.Foo`);
      prepare(`SELECT '${"x".repeat(1000 * 1000)}'`);
      const elapsed = Date.now() - start;

      // quadratic behaviour takes many minutes at this size; linear is well under a second
      expect(elapsed, `parsing 1MB tokens took ${elapsed}ms`).to.be.lessThan(30 * 1000);
    });

    // Long tokens must still parse correctly, not just quickly.
    it("should parse long string literals and identifiers correctly", () => {
      db = createECDb(outDir, "long_tokens_values.ecdb", testSchema);

      const value = "y".repeat(100 * 1000);
      stmt.prepare(db, "INSERT INTO test.Foo(I,S) VALUES(1,?)");
      stmt.getBinder(1).bindString(value);
      assert.equal(stmt.stepForInsert().status, DbResult.BE_SQLITE_DONE);
      stmt.dispose();

      // a long literal in the ECSql text itself
      stmt.prepare(db, `SELECT I FROM test.Foo WHERE S='${value}'`);
      assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
      stmt.dispose();

      // named parameters are located by scanning position, which the block reader changed
      stmt.prepare(db, "SELECT I FROM test.Foo WHERE S=:p AND I=:i");
      stmt.getBinder("p").bindString(value);
      stmt.getBinder("i").bindInteger(1);
      assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    });
  });
});
