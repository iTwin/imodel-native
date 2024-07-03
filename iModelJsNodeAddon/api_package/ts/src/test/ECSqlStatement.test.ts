/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, Guid } from "@itwin/core-bentley";
import { QueryRowFormat } from "@itwin/core-common";
import { Range3d } from "@itwin/core-geometry";
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

  function formatCurrentRow(currentResp: any, rowFormat: QueryRowFormat = QueryRowFormat.UseECSqlPropertyNames): any {
    const formattedRow = {};
    const uniqueNames = new Map<string, number>();
    for (const prop of currentResp.meta) {
      const propName = rowFormat === QueryRowFormat.UseJsPropertyNames ? prop.jsonName : prop.name;
      const val = currentResp.data[prop.index];
      if (typeof val !== "undefined" && val !== null) {
        let uniquePropName = propName;
        if (uniqueNames.has(propName)) {
          uniqueNames.set(propName, uniqueNames.get(propName)! + 1);
          uniquePropName = `${propName}_${uniqueNames.get(propName)!}`;
        } else {
          uniqueNames.set(propName,0);
        }

        Object.defineProperty(formattedRow, uniquePropName, {
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
    const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
    const resp = stmt.toRow(args);
    const row = formatCurrentRow(resp, args.rowFormat);
    assert.equal(row.d, doubleVal);
    assert.equal(row.i, 3);
    assert.equal(row.l, 3);
    assert.equal(row.s, "3.5");
  });

  it("should handle abbreviateBlobs option correctly when toRow is called", async () => {
    const testRange = new Range3d(1.2, 2.3, 3.4, 4.5, 5.6, 6.7);
    const blobVal = new Uint8Array(testRange.toFloat64Array().buffer);
    const abbreviatedBlobVal = `{"bytes":${ blobVal.byteLength }}`;
    const boolVal: boolean = true;

    db = createECDb(outDir, "abbreviateBlobs.ecdb",
      `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECEntityClass typeName="Foo" modifier="Sealed">
        <ECProperty propertyName="Bl" typeName="binary"/>
        <ECProperty propertyName="Bo" typeName="boolean"/>
      </ECEntityClass>
      </ECSchema>`);
    assert.isTrue(db.isOpen());

    stmt.prepare(db, "INSERT INTO test.Foo(Bl,Bo) VALUES(?,?)");
    stmt.getBinder(1).bindBlob(blobVal);
    stmt.getBinder(2).bindBoolean(boolVal);
    const res = stmt.stepForInsert();
    assert.equal(res.status, DbResult.BE_SQLITE_DONE);
    const id = res.id;
    stmt.clearBindings();
    stmt.dispose();

    stmt.prepare(db, "SELECT ECInstanceId, ECClassId, Bl, Bo FROM test.Foo WHERE ECInstanceId=?");
    stmt.getBinder(1).bindId(id);
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    {
      const args = { convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      expect(() => stmt.toRow(args)).to.throw("abbreviateBlobs argument missing");
    }

    {
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.deepEqual(row.bl, blobVal);
      assert.equal(row.bo, boolVal);
    }

    {
      const args = { abbreviateBlobs: true, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.deepEqual(row.bl, abbreviatedBlobVal);
      assert.equal(row.bo, boolVal);
    }
  });

  it("should handle convertClassIdsToClassNames option correctly when toRow is called", async () => {
    const boolVal: boolean = true;
    db = createECDb(outDir, "convertClassIdsToClassNames.ecdb",
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
      const args = { abbreviateBlobs: false, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      expect(() => stmt.toRow(args)).to.throw("convertClassIdsToClassNames argument missing");
    }

    {
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: false, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
      assert.equal(row.id, id);
      assert.equal(row.className, "0x58");
      assert.equal(row.b0, boolVal);
    }

    {
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
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
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: false, includeMetaData: true };
      expect(() => stmt.toRow(args)).to.throw("rowFormat argument missing");
    }

    {
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
      assert.equal(row.id, id);
      assert.equal(row.className, "Test.Foo");
      assert.equal(row.b0, boolVal);
    }

    {
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseECSqlPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      const row = formatCurrentRow(resp, args.rowFormat);
      assert.equal(row.ECInstanceId, id);
      assert.equal(row.ECClassId, "Test.Foo");
      assert.equal(row.B0, boolVal);
    }
  });

  it("should handle includeMetaData option correctly when toRow is called", () => {
    const boolVal: boolean = true;
    db = createECDb(outDir, "includeMetaData.ecdb",
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
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: false, rowFormat: QueryRowFormat.UseJsPropertyNames };
      expect(() => stmt.toRow(args)).to.throw("includeMetaData argument missing");
    }

    {
      const expectedResp = {
        data: ["0x1", "Test.Foo", true],
      };
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseJsPropertyNames, includeMetaData: false };
      const resp = stmt.toRow(args);
      assert.deepEqual(resp, expectedResp);
    }

    {
      const expectedResp = {
        data: ["0x1", "Test.Foo", true],
        meta: [
          { className: "", accessString: "ECInstanceId", generated: false, index: 0, jsonName: "id", name: "ECInstanceId", extendedType: "Id", typeName: "long" },
          { className: "", accessString: "ECClassId", generated: false, index: 1, jsonName: "className", name: "ECClassId", extendedType: "ClassId", typeName: "long" },
          { className: "Test:Foo", accessString: "B0", generated: false, index: 2, jsonName: "b0", name: "B0", extendedType: "", typeName: "boolean" },
        ],
      };
      const args = { abbreviateBlobs: false, convertClassIdsToClassNames: true, rowFormat: QueryRowFormat.UseECSqlPropertyNames, includeMetaData: true };
      const resp = stmt.toRow(args);
      assert.deepEqual(resp, expectedResp);
    }
  });
});
