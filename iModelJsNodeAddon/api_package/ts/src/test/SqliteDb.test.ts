/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, OpenMode } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("SQLiteDb", () => {
  const outDir = getOutputDir();

  function createSqliteDb(fileName: string): IModelJsNative.SQLiteDb {
    const outPath = path.join(outDir, fileName);
    if (fs.existsSync(outPath))
      fs.unlinkSync(outPath);

    const db = new iModelJsNative.SQLiteDb();
    db.createDb(outPath);
    return db;
  }

  describe("lifecycle", () => {
    it("create, open, close, dispose", () => {
      const dbPath = path.join(outDir, "sqlite-lifecycle.db");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      const db = new iModelJsNative.SQLiteDb();
      assert.isFalse(db.isOpen());

      db.createDb(dbPath);
      assert.isTrue(db.isOpen());
      assert.equal(db.getFilePath(), dbPath);

      db.saveChanges();
      db.closeDb();
      assert.isFalse(db.isOpen());

      db.openDb(dbPath, { openMode: OpenMode.ReadWrite });
      assert.isTrue(db.isOpen());
      assert.isFalse(db.isReadonly());

      db.closeDb();

      db.openDb(dbPath, { openMode: OpenMode.Readonly });
      assert.isTrue(db.isOpen());
      assert.isTrue(db.isReadonly());

      db.dispose();
      assert.isFalse(db.isOpen());
    });

    it("dispose is idempotent", () => {
      const db = createSqliteDb("sqlite-dispose.db");
      db.dispose();
      db.dispose(); // should not throw
    });
  });

  describe("raw SQL operations", () => {
    let db: IModelJsNative.SQLiteDb;
    const dbFile = "sqlite-raw-sql.db";

    beforeEach(() => {
      db = createSqliteDb(dbFile);
    });

    afterEach(() => {
      db.dispose();
    });

    it("CREATE TABLE and INSERT", () => {
      const stmt = new iModelJsNative.SqliteStatement();
      stmt.prepare(db, "CREATE TABLE Test(Id INTEGER PRIMARY KEY, Name TEXT, Value REAL)");
      assert.equal(stmt.step(), DbResult.BE_SQLITE_DONE);
      stmt.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO Test(Name, Value) VALUES(?, ?)");
      ins.bindString(1, "foo");
      ins.bindDouble(2, 3.14);
      assert.equal(ins.step(), DbResult.BE_SQLITE_DONE);
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT Name, Value FROM Test");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValueString(0), "foo");
      assert.approximately(sel.getValueDouble(1), 3.14, 1e-10);
      sel.dispose();
    });

    it("UPDATE and DELETE", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE Test(Id INTEGER PRIMARY KEY, Name TEXT)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO Test(Name) VALUES(?)");
      ins.bindString(1, "original");
      ins.step();
      ins.dispose();

      const lastId = db.getLastInsertRowId();

      const upd = new iModelJsNative.SqliteStatement();
      upd.prepare(db, "UPDATE Test SET Name=? WHERE Id=?");
      upd.bindString(1, "updated");
      upd.bindInteger(2, lastId);
      assert.equal(upd.step(), DbResult.BE_SQLITE_DONE);
      upd.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT Name FROM Test WHERE Id=?");
      sel.bindInteger(1, lastId);
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValueString(0), "updated");
      sel.dispose();

      const del = new iModelJsNative.SqliteStatement();
      del.prepare(db, "DELETE FROM Test WHERE Id=?");
      del.bindInteger(1, lastId);
      assert.equal(del.step(), DbResult.BE_SQLITE_DONE);
      del.dispose();

      const count = new iModelJsNative.SqliteStatement();
      count.prepare(db, "SELECT COUNT(*) FROM Test");
      assert.equal(count.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(count.getValueInteger(0), 0);
      count.dispose();
    });

    it("column metadata", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE Meta(A INTEGER, B TEXT, C REAL, D BLOB)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO Meta VALUES(1, 'hello', 2.5, X'DEADBEEF')");
      ins.step();
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT A, B, C, D FROM Meta");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getColumnCount(), 4);
      assert.equal(sel.getColumnName(0), "A");
      assert.equal(sel.getColumnName(1), "B");
      assert.equal(sel.getColumnName(2), "C");
      assert.equal(sel.getColumnName(3), "D");
      sel.dispose();
    });

    it("null value handling", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE NullTest(A TEXT, B INTEGER)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO NullTest(A, B) VALUES(NULL, NULL)");
      ins.step();
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT A, B FROM NullTest");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.isValueNull(0));
      assert.isTrue(sel.isValueNull(1));
      sel.dispose();
    });

    it("bindNull", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE NullBind(A TEXT, B INTEGER)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO NullBind VALUES(?, ?)");
      ins.bindNull(1);
      ins.bindInteger(2, 99);
      ins.step();
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT A, B FROM NullBind");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.isTrue(sel.isValueNull(0));
      assert.equal(sel.getValueInteger(1), 99);
      sel.dispose();
    });

    it("blob binding and retrieval", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE BlobTest(Data BLOB)");
      setup.step();
      setup.dispose();

      const blob = new Uint8Array([0, 1, 127, 128, 255]);
      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO BlobTest VALUES(?)");
      ins.bindBlob(1, blob);
      ins.step();
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT Data FROM BlobTest");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      const result = sel.getValueBlob(0);
      assert.deepEqual(new Uint8Array(result), blob);
      assert.equal(sel.getColumnBytes(0), blob.length);
      sel.dispose();
    });

    it("multiple rows iteration", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE Multi(Val INTEGER)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO Multi VALUES(?)");
      for (let i = 0; i < 20; i++) {
        ins.bindInteger(1, i);
        ins.step();
        ins.reset();
        ins.clearBindings();
      }
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT Val FROM Multi ORDER BY Val");
      let count = 0;
      while (sel.step() === DbResult.BE_SQLITE_ROW) {
        assert.equal(sel.getValueInteger(0), count);
        count++;
      }
      assert.equal(count, 20);
      sel.dispose();
    });

    it("reset and clearBindings allow re-execution", () => {
      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE ResetTest(Val TEXT)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO ResetTest VALUES(?)");
      ins.bindString(1, "first");
      ins.step();
      ins.reset();
      ins.clearBindings();
      ins.bindString(1, "second");
      ins.step();
      ins.dispose();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT COUNT(*) FROM ResetTest");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValueInteger(0), 2);
      sel.dispose();
    });
  });

  describe("WAL mode", () => {
    it("enableWalMode and checkpoint", () => {
      const db = createSqliteDb("sqlite-wal.db");
      db.enableWalMode();
      db.performCheckpoint();
      db.setAutoCheckpointThreshold(1000);
      db.closeDb();
    });
  });

  describe("saveChanges and abandonChanges", () => {
    it("saveChanges persists data", () => {
      const dbPath = path.join(outDir, "sqlite-save.db");
      const db = createSqliteDb("sqlite-save.db");

      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE SaveTest(Name TEXT)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO SaveTest VALUES('persisted')");
      ins.step();
      ins.dispose();
      db.saveChanges();
      db.closeDb();

      db.openDb(dbPath, { openMode: OpenMode.Readonly });
      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT Name FROM SaveTest");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValueString(0), "persisted");
      sel.dispose();
      db.dispose();
    });

    it("abandonChanges discards data", () => {
      const db = createSqliteDb("sqlite-abandon.db");

      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE AbandonTest(Name TEXT)");
      setup.step();
      setup.dispose();
      db.saveChanges(); // save the table creation

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO AbandonTest VALUES('discarded')");
      ins.step();
      ins.dispose();
      db.abandonChanges();

      const sel = new iModelJsNative.SqliteStatement();
      sel.prepare(db, "SELECT COUNT(*) FROM AbandonTest");
      assert.equal(sel.step(), DbResult.BE_SQLITE_ROW);
      assert.equal(sel.getValueInteger(0), 0);
      sel.dispose();
      db.dispose();
    });
  });

  describe("file properties", () => {
    it("save and query file property with string", () => {
      const db = createSqliteDb("sqlite-fileprop.db");

      db.saveFileProperty({ namespace: "test", name: "myProp" }, "hello world", undefined);
      db.saveChanges();

      const val = db.queryFileProperty({ namespace: "test", name: "myProp" }, true);
      assert.equal(val, "hello world");

      db.dispose();
    });

    it("save and query file property with blob", () => {
      const db = createSqliteDb("sqlite-fileprop-blob.db");
      const blobData = new Uint8Array([10, 20, 30, 40, 50]);

      db.saveFileProperty({ namespace: "blobns", name: "blobProp" }, undefined, blobData);
      db.saveChanges();

      const val = db.queryFileProperty({ namespace: "blobns", name: "blobProp" }, false);
      assert.isDefined(val);

      db.dispose();
    });

    it("queryNextAvailableFileProperty returns next index", () => {
      const db = createSqliteDb("sqlite-fileprop-next.db");

      const next1 = db.queryNextAvailableFileProperty({ namespace: "seq", name: "item" });
      assert.isNumber(next1);

      db.saveFileProperty({ namespace: "seq", name: "item", id: next1 }, "first", undefined);
      db.saveChanges();

      const next2 = db.queryNextAvailableFileProperty({ namespace: "seq", name: "item" });
      assert.isAbove(next2, next1);

      db.dispose();
    });
  });

  describe("vacuum and analyze", () => {
    it("vacuum succeeds on db with data", () => {
      const db = createSqliteDb("sqlite-vacuum.db");

      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE VacTest(Data TEXT)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO VacTest VALUES(?)");
      for (let i = 0; i < 100; i++) {
        ins.bindString(1, `row-${i}-${"x".repeat(100)}`);
        ins.step();
        ins.reset();
        ins.clearBindings();
      }
      ins.dispose();
      db.saveChanges();

      // Delete all rows then vacuum to reclaim space
      const del = new iModelJsNative.SqliteStatement();
      del.prepare(db, "DELETE FROM VacTest");
      del.step();
      del.dispose();
      db.saveChanges();

      db.vacuum();
      db.dispose();
    });

    it("analyze succeeds", () => {
      const db = createSqliteDb("sqlite-analyze.db");

      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE AnalyzeTest(Val INTEGER)");
      setup.step();
      setup.dispose();
      db.saveChanges();

      db.analyze();
      db.dispose();
    });
  });

  describe("getLastInsertRowId and getLastError", () => {
    it("getLastInsertRowId returns valid id after insert", () => {
      const db = createSqliteDb("sqlite-lastid.db");

      const setup = new iModelJsNative.SqliteStatement();
      setup.prepare(db, "CREATE TABLE IdTest(Name TEXT)");
      setup.step();
      setup.dispose();

      const ins = new iModelJsNative.SqliteStatement();
      ins.prepare(db, "INSERT INTO IdTest VALUES('test')");
      ins.step();
      ins.dispose();

      const lastId = db.getLastInsertRowId();
      assert.isNumber(lastId);
      assert.isAbove(lastId, 0);

      db.dispose();
    });

    it("getLastError returns error info", () => {
      const db = createSqliteDb("sqlite-lasterr.db");
      const err = db.getLastError();
      assert.isDefined(err);
      db.dispose();
    });
  });

  describe("embedded files", () => {
    it("embed, query, extract, remove in SQLiteDb", () => {
      const db = createSqliteDb("sqlite-embed.db");
      const testData = Buffer.from("embedded content for SQLiteDb test");

      // Write test data to a temp file for embedFile
      const tempFilePath = path.join(outDir, "sqlite-embed-input.txt");
      fs.writeFileSync(tempFilePath, testData);

      db.embedFile({ name: "test-file.txt", localFileName: tempFilePath, date: Date.now() });
      db.saveChanges();

      const queryResult = db.queryEmbeddedFile("test-file.txt");
      assert.isDefined(queryResult);

      const extractPath = path.join(outDir, "sqlite-extracted.txt");
      if (fs.existsSync(extractPath))
        fs.unlinkSync(extractPath);
      db.extractEmbeddedFile({ name: "test-file.txt", localFileName: extractPath });
      assert.isTrue(fs.existsSync(extractPath));

      db.removeEmbeddedFile("test-file.txt");
      db.saveChanges();

      const afterRemove = db.queryEmbeddedFile("test-file.txt");
      assert.isUndefined(afterRemove);
      db.dispose();

      if (fs.existsSync(extractPath))
        fs.unlinkSync(extractPath);
      if (fs.existsSync(tempFilePath))
        fs.unlinkSync(tempFilePath);
    });
  });

  describe("error handling", () => {
    it("open non-existent file throws", () => {
      const db = new iModelJsNative.SQLiteDb();
      expect(() => db.openDb("/nonexistent/path/db.sqlite", { openMode: OpenMode.Readonly })).to.throw();
      db.dispose();
    });

    it("invalid SQL throws on step", () => {
      const db = createSqliteDb("sqlite-invalid-sql.db");
      const stmt = new iModelJsNative.SqliteStatement();
      expect(() => stmt.prepare(db, "NOT VALID SQL")).to.throw();
      stmt.dispose();
      db.dispose();
    });

    it("rawSQLite flag allows opening non-standard db files", () => {
      const dbPath = path.join(outDir, "sqlite-raw.db");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      // Create a standard sqlite db without be_Prop table
      const db = new iModelJsNative.SQLiteDb();
      db.createDb(dbPath);
      db.saveChanges();
      db.closeDb();

      // Should be able to open with rawSQLite
      db.openDb(dbPath, { openMode: OpenMode.ReadWrite, rawSQLite: true });
      assert.isTrue(db.isOpen());
      db.dispose();
    });
  });
});
