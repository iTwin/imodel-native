/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import { dbFileName, iModelJsNative } from "./utils";
import { DbResult } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { openDgnDb } from ".";

describe("origin property on ColumnInfo", () => {
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

});
