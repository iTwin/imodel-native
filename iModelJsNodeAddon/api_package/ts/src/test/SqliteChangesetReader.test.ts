/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { DbOpcode, DbResult, OpenMode } from "@itwin/core-bentley";
import * as fs from "fs";
import * as path from "path";
import { IModelJsNative } from "../NativeLibrary";
import { getAssetsDir, getOutputDir, iModelJsNative } from "./utils";
import { expect } from "chai";

enum DbChangeStage {
  Old = 0,
  New = 1,
}

enum DbValueType {
  IntegerVal = 1,
  FloatVal = 2,
  TextVal = 3,
  BlobVal = 4,
  NullVal = 5,
}

type ChangeValueType = Uint8Array | number | string | null | undefined;
interface IChange {
  tableName: string;
  primaryKeys: ChangeValueType[];
  op: "updated" | "inserted" | "deleted";
  before: ChangeValueType[];
  after: ChangeValueType[] ;
  isIndirect: boolean;
}

interface TableSchema {
  columnCount: number;
  primaryKeyColumns: number[];
}

function collectTableSchemas(changesetFiles: string[]): { tables: Map<string, TableSchema>, changeCount: number } {
  const tables = new Map<string, TableSchema>();
  let changeCount = 0;
  for (const changesetFile of changesetFiles) {
    const reader = new iModelJsNative.SqliteChangesetReader();
    try {
      reader.openFile(changesetFile, false);
      while (reader.step()) {
        ++changeCount;
        const tableName = reader.getTableName();
        const columnCount = reader.getColumnCount();
        const current = tables.get(tableName);
        if (!current || current.columnCount < columnCount)
          tables.set(tableName, { columnCount, primaryKeyColumns: reader.getPrimaryKeyColumnIndexes() });
      }
    } finally {
      reader.close();
    }
  }
  return { tables, changeCount };
}

function createSchema(db: IModelJsNative.SQLiteDb, tables: Map<string, TableSchema>): void {
  for (const [tableName, schema] of tables) {
    const columns = Array.from({ length: schema.columnCount }, (_, index) => `"c${index}"`);
    const primaryKey = schema.primaryKeyColumns.map((index) => `"c${index}"`).join(",");
    expect(primaryKey, `${tableName} must declare a primary key`).not.empty;
    const stmt = new iModelJsNative.SqliteStatement();
    try {
      stmt.prepare(db, `CREATE TABLE "${tableName}" (${columns.join(",")}, PRIMARY KEY (${primaryKey}))`);
      expect(stmt.step()).equals(DbResult.BE_SQLITE_DONE);
    } finally {
      stmt.dispose();
    }
  }
}

describe("Native sqlite changeset reader", () => {
  it("computeChangesetId", () => {
    const props = {
      id: "22e5e7652fa738cd79a77c539b6a72736f5f7de3",
      parentId: "115f80f24edf5aa7f24b51d9dface3223c42cff3",
      pathname:path.join(getAssetsDir(), "f5f7de3.cs")
    }
    const id = iModelJsNative.DgnDb.computeChangesetId(props);
    expect(id).equals(props.id)
  });
  it("changeset reader", () => {
    const reader = new iModelJsNative.SqliteChangesetReader();
    const testCsFile = path.join(getAssetsDir(), "test.cs");
    reader.openFile(testCsFile, false);

    const changes: IChange[] = [];
    while (reader.step()) {
      changes.push({
        tableName: reader.getTableName(),
        primaryKeys : reader.getPrimaryKeys(),
        op: reader.getOpCode() === DbOpcode.Delete ? "deleted" : (reader.getOpCode() === DbOpcode.Update ? "updated" : "inserted"),
        before: reader.getRow(DbChangeStage.Old),
        after: reader.getRow(DbChangeStage.New),
        isIndirect: reader.isIndirectChange(),
      });
    }
    reader.close();

    const elementChanges = Array.from(changes.filter((x) => x.tableName === "bis_Element"));
    const modelChanges = Array.from(changes.filter((x) => x.tableName === "bis_Model"));

    expect(elementChanges.length).eq(155);
    expect(modelChanges.length).eq(3);
    expect(elementChanges.filter((x) => x.op === "inserted").length).eq(69);
    expect(elementChanges.filter((x) => x.op === "updated").length).eq(86);
    expect(elementChanges.filter((x) => x.op === "deleted").length).eq(0);
    expect(modelChanges.filter((x) => x.op === "inserted").length).eq(0);
    expect(modelChanges.filter((x) => x.op === "updated").length).eq(3);
    expect(modelChanges.filter((x) => x.op === "deleted").length).eq(0);
    expect(changes.filter((x) => x.isIndirect).length).eq(3);
    expect(changes.filter((x) => x.primaryKeys[0] === "0xd4000000052f").map((x) => x.tableName)[0]).eq("bis_Element");
    expect(changes.filter((x) => x.primaryKeys[0] === "0xd4000000052f").map((x) => x.op)[0]).eq("inserted");
    expect(changes.filter((x) => x.primaryKeys[0] === "0xcd00000002f5").map((x) => x.op)[0]).eq("updated");
  });
  it("opens a changeset group with a SQLiteDb", () => {
    const changesetFiles = ["f5f7de3.cs", "test.cs"].map((fileName) => path.join(getAssetsDir(), fileName));
    const { tables, changeCount: ungroupedChangeCount } = collectTableSchemas(changesetFiles);
    const dbFileName = path.join(getOutputDir(), "changeset-reader.db");
    fs.rmSync(dbFileName, { force: true });
    const db = new iModelJsNative.SQLiteDb();
    try {
      db.createDb(dbFileName, undefined, { rawSQLite: true });
      createSchema(db, tables);

      const reader = new iModelJsNative.SqliteChangesetReader();
      try {
        reader.openGroup(changesetFiles, db, false);
        let changeCount = 0;
        while (reader.step()) {
          expect(tables.has(reader.getTableName())).is.true;
          ++changeCount;
        }
        expect(changeCount).equals(342);
        expect(changeCount).equals(ungroupedChangeCount);
      } finally {
        reader.close();
      }
    } finally {
      db.closeDb();
      fs.rmSync(dbFileName, { force: true });
    }
  });

  it("rejects a changeset group when a SQLiteDb lacks its tables", () => {
    const dbFileName = path.join(getOutputDir(), "changeset-reader-missing-schema.db");
    fs.rmSync(dbFileName, { force: true });
    const db = new iModelJsNative.SQLiteDb();
    try {
      db.createDb(dbFileName, undefined, { rawSQLite: true });
      const reader = new iModelJsNative.SqliteChangesetReader();
      const testCsFile = path.join(getAssetsDir(), "test.cs");
      expect(() => reader.openGroup([testCsFile], db, false)).throws("changeset table bis_Element does not exist");
      reader.close();
    } finally {
      db.closeDb();
      fs.rmSync(dbFileName, { force: true });
    }
  });

  it("routes DgnDb and ECDb through changeset group schema validation", () => {
    const changesetFile = path.join(getAssetsDir(), "test.cs");
    const dgnDb = new iModelJsNative.DgnDb();
    try {
      dgnDb.openIModel(path.join(getAssetsDir(), "test.bim"), OpenMode.Readonly);
      const reader = new iModelJsNative.SqliteChangesetReader();
      try {
        expect(() => reader.openGroup([changesetFile], dgnDb, false)).throws(/has fewer columns|does not exist/);
      } finally {
        reader.close();
      }
    } finally {
      dgnDb.closeFile();
    }

    const ecDb = new iModelJsNative.ECDb();
    try {
      expect(ecDb.openDb(path.join(getAssetsDir(), "test.bim"), OpenMode.Readonly)).equals(DbResult.BE_SQLITE_OK);
      const reader = new iModelJsNative.SqliteChangesetReader();
      try {
        expect(() => reader.openGroup([changesetFile], ecDb, false)).throws(/has fewer columns|does not exist/);
      } finally {
        reader.close();
      }
    } finally {
      ecDb.closeDb();
    }
  });
  it("getColumnValueXXXX() methods", () => {
    const reader = new iModelJsNative.SqliteChangesetReader();
    const testCsFile = path.join(getAssetsDir(), "test.cs");
    reader.openFile(testCsFile, false);

    expect(reader.step()).is.true;
    expect(reader.getTableName()).equals("bis_Element");
    expect(reader.getOpCode()).equals(DbOpcode.Insert);
    expect(reader.getColumnCount()).equals(12);
    expect(reader.getColumnValueType(0, DbChangeStage.Old)).is.undefined;
    expect(reader.getColumnValueType(100000, DbChangeStage.New)).is.undefined;

    expect(reader.getColumnValueType(0, DbChangeStage.New)).equals(DbValueType.IntegerVal);
    expect(reader.getColumnValueType(1, DbChangeStage.New)).equals(DbValueType.IntegerVal);
    expect(reader.getColumnValueType(2, DbChangeStage.New)).equals(DbValueType.IntegerVal);
    expect(reader.getColumnValueType(3, DbChangeStage.New)).equals(DbValueType.FloatVal);
    expect(reader.getColumnValueType(4, DbChangeStage.New)).equals(DbValueType.IntegerVal);
    expect(reader.getColumnValueType(5, DbChangeStage.New)).equals(DbValueType.IntegerVal);
    expect(reader.getColumnValueType(6, DbChangeStage.New)).equals(DbValueType.NullVal);
    expect(reader.getColumnValueType(7, DbChangeStage.New)).equals(DbValueType.NullVal);
    expect(reader.getColumnValueType(8, DbChangeStage.New)).equals(DbValueType.NullVal);
    expect(reader.getColumnValueType(9, DbChangeStage.New)).equals(DbValueType.NullVal);
    expect(reader.getColumnValueType(10, DbChangeStage.New)).equals(DbValueType.BlobVal);
    expect(reader.getColumnValueType(11, DbChangeStage.New)).equals(DbValueType.NullVal);

    expect(reader.getColumnValueInteger(0, DbChangeStage.New)).equals(233096465089837);
    expect(reader.getColumnValueId(0, DbChangeStage.New)).equals("0xd4000000052d");
    expect(reader.getColumnValueText(0, DbChangeStage.New)).equals("233096465089837");
    expect(reader.getColumnValueDouble(0, DbChangeStage.New)).equals(233096465089837);
    expect(reader.getColumnValueBinary(0, DbChangeStage.New)).deep.equals(new Uint8Array([50, 51, 51, 48, 57, 54, 52, 54, 53, 48, 56, 57, 56, 51, 55]));
    expect(reader.isColumnValueNull(0, DbChangeStage.New)).is.false;
  });

  it("check exceptions", () => {
    const reader = new iModelJsNative.SqliteChangesetReader();
    expect(() => reader.step()).throw("step(): no changeset opened.");
    expect(() => reader.getColumnCount()).throw("getColumnCount(): there is no current row.");
    expect(() => reader.getDdlChanges()).throw("getDdlChanges(): no changeset opened.");
    expect(() => reader.getOpCode()).throw("getOpCode(): there is no current row.");
    expect(() => reader.getPrimaryKeys()).throw("getPrimaryKeys(): there is no current row.");
    expect(() => reader.getRow(DbChangeStage.Old)).throw("getRow(): there is no current row.");
    expect(() => reader.getRow(DbChangeStage.New)).throw("getRow(): there is no current row.");
    expect(() => reader.getTableName()).throw("getTableName(): there is no current row.");
    expect(() => reader.getColumnValue(0, DbChangeStage.Old)).not.throw;
    expect(() => reader.getColumnValue(0, DbChangeStage.New)).not.throw;
    expect(() => reader.getColumnValueType(0, DbChangeStage.Old)).not.throw;
    expect(() => reader.getColumnValueType(0, DbChangeStage.New)).not.throw;
  });
});
