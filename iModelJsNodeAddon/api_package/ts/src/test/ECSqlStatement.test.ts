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
});
