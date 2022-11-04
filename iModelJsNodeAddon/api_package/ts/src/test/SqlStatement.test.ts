/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import { dbFileName, iModelJsNative } from "./utils";
import { DbResult, Logger, LogLevel } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { openDgnDb } from ".";
import * as sinon from "sinon";

describe("SQLite statements", () => {
  let dgndb: IModelJsNative.DgnDb;
  before((done) => {
    dgndb = openDgnDb(dbFileName);
    done();
  })

  after((done) => {
    dgndb.closeIModel();
    done();
  })

  it("Select using alias without table", () => {
    const stmt = new iModelJsNative.ECSqlStatement();
    const sql = "SELECT 100 as foo";
    stmt.prepare(dgndb, sql, false);
    try {
      expect(stmt.step()).eq(DbResult.BE_SQLITE_ROW);
      const value = stmt.getValue(0);
      const colInfo = value.getColumnInfo();

      expect(colInfo.getPropertyName()).eq("foo");
      expect(colInfo.hasOriginProperty()).to.be.false;
      expect(() => colInfo.getOriginPropertyName()).to.throw("ECSqlColumnInfo does not have an origin property.");
    } finally {
      stmt.dispose();
    }
  });

  it("Select without table", () => {
    const stmt = new iModelJsNative.ECSqlStatement();
    const sql = "SELECT 100";
    stmt.prepare(dgndb, sql, false);
    try {
      expect(stmt.step()).eq(DbResult.BE_SQLITE_ROW);
      const value = stmt.getValue(0);
      const colInfo = value.getColumnInfo();

      expect(colInfo.getPropertyName()).to.exist; //we generate a pseudo name automatically
      expect(colInfo.hasOriginProperty()).to.be.false;
      expect(() => colInfo.getOriginPropertyName()).to.throw("ECSqlColumnInfo does not have an origin property.");
    } finally {
      stmt.dispose();
    }
  });

  it("Select without table using nested recursive cte", () => {
    const stmt = new iModelJsNative.ECSqlStatement();
    const sql = "with recursive cte0 (a,b) as (select 100,200) select * from (select a from cte0 where a=100 and b=200)";
    stmt.prepare(dgndb, sql, false);
    try {
      expect(stmt.step()).eq(DbResult.BE_SQLITE_ROW);
      const value = stmt.getValue(0);
      const colInfo = value.getColumnInfo();

      expect(colInfo.getPropertyName()).to.exist;
      expect(colInfo.hasOriginProperty()).to.be.false;
      expect(() => colInfo.getOriginPropertyName()).to.throw("ECSqlColumnInfo does not have an origin property.");
    } finally {
      stmt.dispose();
    }
  });


  it("Select without table using recursive cte", () => {
    const stmt = new iModelJsNative.ECSqlStatement();
    const sql = "with recursive cte0 (a,b) as (select 100,200) select a from cte0 where a=100 and b=200";
    stmt.prepare(dgndb, sql, false);
    try {
      expect(stmt.step()).eq(DbResult.BE_SQLITE_ROW);
      const value = stmt.getValue(0);
      const colInfo = value.getColumnInfo();

      expect(colInfo.getPropertyName()).to.exist;
      expect(colInfo.hasOriginProperty()).to.be.false;
      expect(() => colInfo.getOriginPropertyName()).to.throw("ECSqlColumnInfo does not have an origin property.");
    } finally {
      stmt.dispose();
    }
  });

  it("Select ECSchemaDef with alias", () => {
    const stmt = new iModelJsNative.ECSqlStatement();
    const sql = "select Name as SchemaName from meta.ECSchemaDef limit 1";
    stmt.prepare(dgndb, sql, false);
    try {
      expect(stmt.step()).eq(DbResult.BE_SQLITE_ROW);
      const value = stmt.getValue(0);
      const colInfo = value.getColumnInfo();

      expect(colInfo.getPropertyName()).eq("SchemaName");
      expect(colInfo.hasOriginProperty()).to.be.true;
      expect(colInfo.getOriginPropertyName()).eq("Name");
    } finally {
      stmt.dispose();
    }
  });

  it("test logging", ()=> {
    Logger.setLevel("test-info", LogLevel.Info);
    Logger.setLevel("test-warn", LogLevel.Warning);
    Logger.setLevel("test-error", LogLevel.Error);
    Logger.setLevel("test-trace", LogLevel.Trace);
    iModelJsNative.clearLogLevelCache();
    const errorLogStub = sinon.stub(Logger, "logError").callsFake(() => { });

    const stmt = new iModelJsNative.SqliteStatement();
    const sql = "SELECT 100 from xxx";
    expect(() => stmt.prepare(dgndb, sql, true)).throws("no such table")
    expect(errorLogStub.callCount).eq(1);

    Logger.setLevel("BeSQLite", LogLevel.None);
    iModelJsNative.clearLogLevelCache();
    expect(() => stmt.prepare(dgndb, sql, true)).throws("no such table")
    expect(errorLogStub.callCount).eq(1);
    stmt.dispose();
  });

});
