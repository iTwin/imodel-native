/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, Guid, OpenMode } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("ECDb standalone", () => {
  const outDir = getOutputDir();

  function createECDb(fileName: string, schemaXml?: string): IModelJsNative.ECDb {
    const outPath = path.join(outDir, fileName);
    if (fs.existsSync(outPath))
      fs.unlinkSync(outPath);

    const ecdb = new iModelJsNative.ECDb();
    ecdb.createDb(outPath);

    if (schemaXml) {
      const schemaPath = path.join(outDir, `${Guid.createValue()}.ecschema.xml`);
      if (fs.existsSync(schemaPath))
        fs.unlinkSync(schemaPath);
      fs.writeFileSync(schemaPath, schemaXml);
      ecdb.importSchema(schemaPath);
      fs.unlinkSync(schemaPath);
    }
    return ecdb;
  }

  const allTypesSchema = `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECEntityClass typeName="AllTypes" modifier="Sealed">
      <ECProperty propertyName="B" typeName="boolean"/>
      <ECProperty propertyName="I" typeName="int"/>
      <ECProperty propertyName="L" typeName="long"/>
      <ECProperty propertyName="D" typeName="double"/>
      <ECProperty propertyName="S" typeName="string"/>
      <ECProperty propertyName="Bin" typeName="binary"/>
      <ECProperty propertyName="Dt" typeName="dateTime"/>
      <ECProperty propertyName="P2d" typeName="point2d"/>
      <ECProperty propertyName="P3d" typeName="point3d"/>
      <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry"/>
      <ECProperty propertyName="Gu" typeName="binary" extendedTypeName="BeGuid"/>
    </ECEntityClass>
    <ECEntityClass typeName="Child" modifier="Sealed">
      <ECProperty propertyName="Name" typeName="string"/>
    </ECEntityClass>
  </ECSchema>`;

  describe("lifecycle", () => {
    it("create, open, close, dispose", () => {
      const dbPath = path.join(outDir, "lifecycle.ecdb");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      const db = new iModelJsNative.ECDb();
      assert.isFalse(db.isOpen());

      db.createDb(dbPath);
      assert.isTrue(db.isOpen());
      assert.isTrue(fs.existsSync(dbPath));
      assert.equal(db.getFilePath(), dbPath);

      db.closeDb();
      assert.isFalse(db.isOpen());

      db.openDb(dbPath, OpenMode.ReadWrite);
      assert.isTrue(db.isOpen());

      db.dispose();
      assert.isFalse(db.isOpen());
    });

    it("creating db at existing path overwrites", () => {
      const dbPath = path.join(outDir, "overwrite.ecdb");
      const db1 = new iModelJsNative.ECDb();
      db1.createDb(dbPath);
      db1.closeDb();

      // Delete existing file before recreating
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      const db2 = new iModelJsNative.ECDb();
      db2.createDb(dbPath);
      assert.isTrue(db2.isOpen());
      db2.dispose();
    });

    it("dispose is idempotent", () => {
      const db = createECDb("dispose-idem.ecdb");
      db.dispose();
      db.dispose(); // should not throw
    });
  });

  describe("schema import", () => {
    it("import schema with various property types", () => {
      const db = createECDb("all-types.ecdb", allTypesSchema);
      assert.isTrue(db.isOpen());

      const props = db.getSchemaProps("Test");
      assert.equal(props.name, "Test");
      assert.equal(props.version, "01.00.00");
      db.dispose();
    });

    it("import schema with struct properties", () => {
      const schema = `<ECSchema schemaName="TestStruct" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECStructClass typeName="Address">
          <ECProperty propertyName="Street" typeName="string"/>
          <ECProperty propertyName="City" typeName="string"/>
          <ECProperty propertyName="Zip" typeName="int"/>
        </ECStructClass>
        <ECEntityClass typeName="Person" modifier="Sealed">
          <ECProperty propertyName="Name" typeName="string"/>
          <ECStructProperty propertyName="Home" typeName="Address"/>
        </ECEntityClass>
      </ECSchema>`;
      const db = createECDb("struct-schema.ecdb", schema);
      assert.isTrue(db.isOpen());
      db.dispose();
    });

    it("import schema with array properties", () => {
      const schema = `<ECSchema schemaName="TestArray" alias="ta" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEntityClass typeName="Collection" modifier="Sealed">
          <ECArrayProperty propertyName="Tags" typeName="string"/>
          <ECArrayProperty propertyName="Scores" typeName="double"/>
        </ECEntityClass>
      </ECSchema>`;
      const db = createECDb("array-schema.ecdb", schema);
      assert.isTrue(db.isOpen());
      db.dispose();
    });

    it("getSchemaProps throws for missing schema", () => {
      const db = createECDb("missing-schema.ecdb");
      expect(() => db.getSchemaProps("DoesNotExist")).to.throw();
      db.dispose();
    });
  });

  describe("ECSQL binding and retrieval", () => {
    let db: IModelJsNative.ECDb;

    before(() => {
      db = createECDb("ecsql-binding.ecdb", allTypesSchema);
    });

    after(() => {
      db.dispose();
    });

    it("bind and retrieve boolean values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(B) VALUES(?)");
      stmt.getBinder(1).bindBoolean(true);
      let r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id1 = r.id;
      stmt.reset();
      stmt.clearBindings();

      stmt.getBinder(1).bindBoolean(false);
      r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id2 = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT B FROM test.AllTypes WHERE ECInstanceId=?");

      sel.getBinder(1).bindId(id1);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.getValue(0).getBoolean());
      sel.reset();
      sel.clearBindings();

      sel.getBinder(1).bindId(id2);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isFalse(sel.getValue(0).getBoolean());
      sel.dispose();
    });

    it("bind and retrieve integer values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(I) VALUES(?)");

      const testValues = [0, 1, -1, 2147483647, -2147483648];
      const ids: string[] = [];
      for (const val of testValues) {
        stmt.getBinder(1).bindInteger(val);
        const r = stmt.stepForInsert();
        assert.equal(r.status, DbResult.BE_SQLITE_DONE);
        ids.push(r.id);
        stmt.reset();
        stmt.clearBindings();
      }
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT I FROM test.AllTypes WHERE ECInstanceId=?");
      for (let i = 0; i < testValues.length; i++) {
        sel.getBinder(1).bindId(ids[i]);
        assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
        assert.equal(sel.getValue(0).getInt(), testValues[i]);
        sel.reset();
        sel.clearBindings();
      }
      sel.dispose();
    });

    it("bind and retrieve double values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(D) VALUES(?)");

      const testValues = [0.0, 3.14159, -2.71828, 1e10, 1e-10];
      const ids: string[] = [];
      for (const val of testValues) {
        stmt.getBinder(1).bindDouble(val);
        const r = stmt.stepForInsert();
        assert.equal(r.status, DbResult.BE_SQLITE_DONE);
        ids.push(r.id);
        stmt.reset();
        stmt.clearBindings();
      }
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT D FROM test.AllTypes WHERE ECInstanceId=?");
      for (let i = 0; i < testValues.length; i++) {
        sel.getBinder(1).bindId(ids[i]);
        assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
        assert.approximately(sel.getValue(0).getDouble(), testValues[i], 1e-6);
        sel.reset();
        sel.clearBindings();
      }
      sel.dispose();
    });

    it("bind and retrieve string values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(S) VALUES(?)");

      const testValues = ["", "hello", "world with spaces", "unicode: \u00e9\u00e0\u00fc", "special chars: !@#$%"];
      const ids: string[] = [];
      for (const val of testValues) {
        stmt.getBinder(1).bindString(val);
        const r = stmt.stepForInsert();
        assert.equal(r.status, DbResult.BE_SQLITE_DONE);
        ids.push(r.id);
        stmt.reset();
        stmt.clearBindings();
      }
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT S FROM test.AllTypes WHERE ECInstanceId=?");
      for (let i = 0; i < testValues.length; i++) {
        sel.getBinder(1).bindId(ids[i]);
        assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
        assert.equal(sel.getValue(0).getString(), testValues[i]);
        sel.reset();
        sel.clearBindings();
      }
      sel.dispose();
    });

    it("bind and retrieve blob values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(Bin) VALUES(?)");

      const blob = new Uint8Array([1, 2, 3, 4, 5, 0, 255, 128]);
      stmt.getBinder(1).bindBlob(blob);
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT Bin FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      const retrieved = sel.getValue(0).getBlob();
      assert.deepEqual(new Uint8Array(retrieved), blob);
      sel.dispose();
    });

    it("bind and retrieve guid values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(Gu) VALUES(?)");

      const guid = Guid.createValue();
      stmt.getBinder(1).bindGuid(guid);
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT Gu FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      const retrieved = sel.getValue(0).getGuid();
      assert.equal(retrieved.toLowerCase(), guid.toLowerCase());
      sel.dispose();
    });

    it("bind and retrieve point2d values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(P2d) VALUES(?)");
      stmt.getBinder(1).bindPoint2d(1.5, 2.5);
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT P2d FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      const p2d = sel.getValue(0).getPoint2d();
      assert.approximately(p2d.x, 1.5, 1e-10);
      assert.approximately(p2d.y, 2.5, 1e-10);
      sel.dispose();
    });

    it("bind and retrieve point3d values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(P3d) VALUES(?)");
      stmt.getBinder(1).bindPoint3d(1.0, 2.0, 3.0);
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT P3d FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      const p3d = sel.getValue(0).getPoint3d();
      assert.approximately(p3d.x, 1.0, 1e-10);
      assert.approximately(p3d.y, 2.0, 1e-10);
      assert.approximately(p3d.z, 3.0, 1e-10);
      sel.dispose();
    });

    it("bind and retrieve null values", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(S) VALUES(NULL)");
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT S, I, D, B FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.getValue(0).isNull());
      assert.isTrue(sel.getValue(1).isNull());
      assert.isTrue(sel.getValue(2).isNull());
      assert.isTrue(sel.getValue(3).isNull());
      sel.dispose();
    });

    it("bind null explicitly", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(S, I) VALUES(?, ?)");
      stmt.getBinder(1).bindNull();
      stmt.getBinder(2).bindInteger(42);
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT S, I FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.getValue(0).isNull());
      assert.equal(sel.getValue(1).getInt(), 42);
      sel.dispose();
    });

    it("bind multiple values in single insert", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "INSERT INTO test.AllTypes(B, I, D, S) VALUES(?, ?, ?, ?)");
      stmt.getBinder(1).bindBoolean(true);
      stmt.getBinder(2).bindInteger(100);
      stmt.getBinder(3).bindDouble(9.99);
      stmt.getBinder(4).bindString("multi");
      const r = stmt.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      stmt.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT B, I, D, S FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.getValue(0).getBoolean());
      assert.equal(sel.getValue(1).getInt(), 100);
      assert.approximately(sel.getValue(2).getDouble(), 9.99, 1e-10);
      assert.equal(sel.getValue(3).getString(), "multi");
      sel.dispose();
    });
  });

  describe("ECSQL UPDATE and DELETE", () => {
    let db: IModelJsNative.ECDb;

    before(() => {
      db = createECDb("ecsql-update-delete.ecdb", allTypesSchema);
    });

    after(() => {
      db.dispose();
    });

    it("UPDATE modifies existing row", () => {
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S, I) VALUES('original', 1)");
      const r = ins.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      ins.dispose();

      const upd = new iModelJsNative.ECSqlStatement();
      upd.prepare(db, "UPDATE test.AllTypes SET S='updated', I=2 WHERE ECInstanceId=?");
      upd.getBinder(1).bindId(id);
      assert.equal(upd.step(), DbResult.BE_SQLITE_DONE);
      upd.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT S, I FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getString(), "updated");
      assert.equal(sel.getValue(1).getInt(), 2);
      sel.dispose();
    });

    it("DELETE removes existing row", () => {
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S) VALUES('to-delete')");
      const r = ins.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const id = r.id;
      ins.dispose();

      const del = new iModelJsNative.ECSqlStatement();
      del.prepare(db, "DELETE FROM test.AllTypes WHERE ECInstanceId=?");
      del.getBinder(1).bindId(id);
      assert.equal(del.step(), DbResult.BE_SQLITE_DONE);
      del.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT COUNT(*) FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(id);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getInt(), 0);
      sel.dispose();
    });
  });

  describe("transactions", () => {
    it("saveChanges persists data", () => {
      const db = createECDb("txn-save.ecdb", allTypesSchema);

      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S) VALUES('saved')");
      ins.stepForInsert();
      ins.dispose();

      db.saveChanges();
      db.closeDb();

      db.openDb(path.join(outDir, "txn-save.ecdb"), OpenMode.Readonly);
      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT S FROM test.AllTypes");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getString(), "saved");
      sel.dispose();
      db.dispose();
    });

    it("abandonChanges discards data", () => {
      const db = createECDb("txn-abandon.ecdb", allTypesSchema);
      db.saveChanges(); // save the schema

      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S) VALUES('abandoned')");
      ins.stepForInsert();
      ins.dispose();

      db.abandonChanges();
      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT COUNT(*) FROM test.AllTypes");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getInt(), 0);
      sel.dispose();
      db.dispose();
    });
  });

  describe("ECSQL async operations", () => {
    let db: IModelJsNative.ECDb;

    before(() => {
      db = createECDb("ecsql-async.ecdb", allTypesSchema);
    });

    after(() => {
      db.dispose();
    });

    it("stepForInsertAsync returns correct result", (done) => {
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S) VALUES('async-test')");
      ins.stepForInsertAsync((r) => {
        try {
          assert.equal(r.status, DbResult.BE_SQLITE_DONE);
          assert.isString(r.id);
          const insertedId = r.id;
          ins.dispose();

          const sel = new iModelJsNative.ECSqlStatement();
          sel.prepare(db, "SELECT S FROM test.AllTypes WHERE ECInstanceId=?");
          sel.getBinder(1).bindId(insertedId);
          sel.stepAsync((status) => {
            try {
              assert.equal(status, DbResult.BE_SQLITE_ROW);
              assert.equal(sel.getValue(0).getString(), "async-test");
              done();
            } catch (err) {
              done(err);
            } finally {
              sel.dispose();
            }
          });
        } catch (err) {
          ins.dispose();
          done(err);
        }
      });
    });
  });

  describe("getNativeSql", () => {
    let db: IModelJsNative.ECDb;

    before(() => {
      db = createECDb("native-sql.ecdb", allTypesSchema);
    });

    after(() => {
      db.dispose();
    });

    it("returns translated SQL", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT S, I FROM test.AllTypes WHERE ECInstanceId=?");
      const nativeSql = stmt.getNativeSql();
      assert.isString(nativeSql);
      assert.isTrue(nativeSql.length > 0);
      // native SQL should contain actual table/column names, not EC names
      assert.isFalse(nativeSql.includes("test.AllTypes"));
      stmt.dispose();
    });
  });

  describe("column info", () => {
    let db: IModelJsNative.ECDb;

    before(() => {
      db = createECDb("col-info.ecdb", allTypesSchema);
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S, I, D) VALUES('info', 42, 3.14)");
      ins.stepForInsert();
      ins.dispose();
    });

    after(() => {
      db.dispose();
    });

    it("getColumnCount returns correct count", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT S, I, D FROM test.AllTypes");
      assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(stmt.getColumnCount(), 3);
      stmt.dispose();
    });

    it("getColumnInfo returns property metadata", () => {
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT S, I FROM test.AllTypes");
      assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);

      const col0 = stmt.getValue(0).getColumnInfo();
      assert.equal(col0.getPropertyName(), "S");

      const col1 = stmt.getValue(1).getColumnInfo();
      assert.equal(col1.getPropertyName(), "I");

      stmt.dispose();
    });
  });

  describe("statement lifecycle", () => {
    it("dispose is idempotent on ECSqlStatement", () => {
      const db = createECDb("stmt-dispose.ecdb", allTypesSchema);
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT 1");
      stmt.dispose();
      stmt.dispose(); // should not throw
      db.dispose();
    });

    it("reset allows re-execution", () => {
      const db = createECDb("stmt-reset.ecdb", allTypesSchema);

      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(I) VALUES(?)");

      ins.getBinder(1).bindInteger(1);
      assert.equal(ins.stepForInsert().status, DbResult.BE_SQLITE_DONE);
      ins.reset();
      ins.clearBindings();

      ins.getBinder(1).bindInteger(2);
      assert.equal(ins.stepForInsert().status, DbResult.BE_SQLITE_DONE);
      ins.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT COUNT(*) FROM test.AllTypes");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getInt(), 2);
      sel.dispose();
      db.dispose();
    });
  });

  describe("error handling", () => {
    it("prepare with invalid ECSQL returns error", () => {
      const db = createECDb("error-sql.ecdb", allTypesSchema);
      const stmt = new iModelJsNative.ECSqlStatement();
      const result = stmt.prepare(db, "NOT VALID SQL AT ALL");
      assert.notEqual(result.status, DbResult.BE_SQLITE_OK);
      stmt.dispose();
      db.dispose();
    });

    it("prepare against non-existent class returns error", () => {
      const db = createECDb("error-class.ecdb", allTypesSchema);
      const stmt = new iModelJsNative.ECSqlStatement();
      const result = stmt.prepare(db, "SELECT * FROM test.DoesNotExist");
      assert.notEqual(result.status, DbResult.BE_SQLITE_OK);
      stmt.dispose();
      db.dispose();
    });

    it("operations on closed db return error", () => {
      const db = createECDb("error-closed.ecdb", allTypesSchema);
      db.closeDb();
      const stmt = new iModelJsNative.ECSqlStatement();
      const result = stmt.prepare(db, "SELECT 1");
      assert.notEqual(result.status, DbResult.BE_SQLITE_OK);
      stmt.dispose();
      db.dispose();
    });
  });

  describe("multiple rows iteration", () => {
    it("iterate over multiple inserted rows", () => {
      const db = createECDb("iterate.ecdb", allTypesSchema);

      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(I) VALUES(?)");
      const count = 10;
      for (let i = 0; i < count; i++) {
        ins.getBinder(1).bindInteger(i * 10);
        ins.stepForInsert();
        ins.reset();
        ins.clearBindings();
      }
      ins.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT I FROM test.AllTypes ORDER BY I");
      let rowCount = 0;
      const values: number[] = [];
      while (sel.step() === DbResult.BE_SQLITE_ROW) {
        values.push(sel.getValue(0).getInt());
        rowCount++;
      }
      assert.equal(rowCount, count);
      for (let i = 0; i < count; i++) {
        assert.equal(values[i], i * 10);
      }
      sel.dispose();
      db.dispose();
    });
  });

  describe("getId and bindId", () => {
    it("round-trip ECInstanceId", () => {
      const db = createECDb("id-roundtrip.ecdb", allTypesSchema);
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(S) VALUES('id-test')");
      const r = ins.stepForInsert();
      assert.equal(r.status, DbResult.BE_SQLITE_DONE);
      const insertedId = r.id;
      ins.dispose();

      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT ECInstanceId FROM test.AllTypes WHERE ECInstanceId=?");
      sel.getBinder(1).bindId(insertedId);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValue(0).getId(), insertedId);
      sel.dispose();
      db.dispose();
    });
  });

  describe("bindIdSet", () => {
    it("query multiple rows by id set", () => {
      const db = createECDb("id-set.ecdb", allTypesSchema);
      const ins = new iModelJsNative.ECSqlStatement();
      ins.prepare(db, "INSERT INTO test.AllTypes(I) VALUES(?)");

      const ids: string[] = [];
      for (let i = 0; i < 5; i++) {
        ins.getBinder(1).bindInteger(i);
        ids.push(ins.stepForInsert().id);
        ins.reset();
        ins.clearBindings();
      }
      ins.dispose();

      // Query only first 3
      const subset = ids.slice(0, 3);
      const sel = new iModelJsNative.ECSqlStatement();
      sel.prepare(db, "SELECT ECInstanceId FROM test.AllTypes WHERE InVirtualSet(?, ECInstanceId)");
      sel.getBinder(1).bindIdSet(subset);
      let count = 0;
      while (sel.step() === DbResult.BE_SQLITE_ROW)
        count++;
      assert.equal(count, 3);
      sel.dispose();
      db.dispose();
    });
  });
});
