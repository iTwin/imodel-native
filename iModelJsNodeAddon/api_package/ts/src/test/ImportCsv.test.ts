/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import * as v8 from "v8";
import { DbResult, Guid } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

const testSchemaXml =
  `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
  <ECEntityClass typeName="Foo" modifier="Sealed">
    <ECProperty propertyName="Name" typeName="string"/>
    <ECProperty propertyName="Quantity" typeName="int"/>
    <ECProperty propertyName="Amount" typeName="double"/>
    <ECProperty propertyName="Flag" typeName="boolean"/>
  </ECEntityClass>
  </ECSchema>`;

const mapping = [
  { columnIndex: 0, propertyName: "Name" },
  { columnIndex: 1, propertyName: "Quantity" },
  { columnIndex: 2, propertyName: "Amount" },
  { columnIndex: 3, propertyName: "Flag" },
];

describe("ImportCsv", () => {
  const outDir = getOutputDir();

  function createECDb(fileName: string): IModelJsNative.ECDb {
    const outPath = path.join(outDir, fileName);
    if (fs.existsSync(outPath))
      fs.unlinkSync(outPath);

    const ecdb = new iModelJsNative.ECDb();
    ecdb.createDb(outPath);

    const schemaPath = path.join(outDir, `${Guid.createValue()}.ecschema.xml`);
    fs.writeFileSync(schemaPath, testSchemaXml);
    ecdb.importSchema(schemaPath);
    return ecdb;
  }

  function readAllFooRows(db: IModelJsNative.ECDb): Array<{ name: string, quantity: number, value: number, flag: boolean, isValueNull: boolean }> {
    const stmt = new iModelJsNative.ECSqlStatement();
    try {
      stmt.prepare(db, "SELECT Name, Quantity, Amount, Flag FROM Test.Foo ORDER BY Quantity");
      const rows: Array<{ name: string, quantity: number, value: number, flag: boolean, isValueNull: boolean }> = [];
      while (stmt.step() === DbResult.BE_SQLITE_ROW) {
        rows.push({
          name: stmt.getValue(0).getString(),
          quantity: stmt.getValue(1).getInt(),
          value: stmt.getValue(2).getDouble(),
          flag: stmt.getValue(3).getBoolean(),
          isValueNull: stmt.getValue(2).isNull(),
        });
      }
      return rows;
    } finally {
      stmt.dispose();
    }
  }

  describe("importCSVData", () => {
    let db: IModelJsNative.ECDb;

    afterEach(() => {
      db.dispose();
      db.closeDb();
    });

    it("imports V8-serialized string rows and returns the row count", () => {
      db = createECDb("importCsvData.ecdb");
      const rows = [
        ["Alpha", "3", "1.5", "true"],
        ["Beta", "4", "2.5", "false"],
      ];
      const bytes = v8.serialize(rows);

      const rowCount = db.importCSVData("Test.Foo", bytes, mapping);
      expect(rowCount).eq(2);

      const result = readAllFooRows(db);
      expect(result.length).eq(2);
      expect(result[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
      expect(result[1]).to.deep.include({ name: "Beta", quantity: 4, value: 2.5, flag: false });
    });

    it("binds the configured nullValue option as NULL", () => {
      db = createECDb("importCsvDataNull.ecdb");
      const rows = [["Gamma", "5", "NULL_MARKER", "true"]];
      const bytes = v8.serialize(rows);

      const rowCount = db.importCSVData("Test.Foo", bytes, mapping, { nullValue: "NULL_MARKER" });
      expect(rowCount).eq(1);

      const result = readAllFooRows(db);
      expect(result.length).eq(1);
      expect(result[0].isValueNull).eq(true);
    });

    it("throws and rolls back the whole import when a later row fails to bind", () => {
      db = createECDb("importCsvDataRollback.ecdb");
      const rows = [
        ["Alpha", "3", "1.5", "true"],
        ["Beta", "not-a-number", "2.5", "false"],
      ];
      const bytes = v8.serialize(rows);

      expect(() => db.importCSVData("Test.Foo", bytes, mapping)).to.throw();
      expect(readAllFooRows(db).length).eq(0);
    });

    it("throws for a mapping with duplicate columnIndex values", () => {
      db = createECDb("importCsvDataBadMapping.ecdb");
      const bytes = v8.serialize([["Alpha", "3", "1.5", "true"]]);
      const badMapping = [
        { columnIndex: 0, propertyName: "Name" },
        { columnIndex: 0, propertyName: "Quantity" },
      ];
      expect(() => db.importCSVData("Test.Foo", bytes, badMapping)).to.throw();
    });
  });

  describe("importCSVFile", () => {
    let db: IModelJsNative.ECDb;

    afterEach(() => {
      db.dispose();
      db.closeDb();
    });

    function writeCsv(fileName: string, content: string): string {
      const csvPath = path.join(outDir, fileName);
      fs.writeFileSync(csvPath, content);
      return csvPath;
    }

    it("streams a CSV file with a header row and returns the record count", () => {
      db = createECDb("importCsvFile.ecdb");
      const csvPath = writeCsv("importCsvFile.csv", "Name,Quantity,Amount,Flag\nAlpha,3,1.5,true\nBeta,4,2.5,false\n");

      const rowCount = db.importCSVFile("Test.Foo", csvPath, mapping, { hasHeader: true });
      expect(rowCount).eq(2);

      const result = readAllFooRows(db);
      expect(result.length).eq(2);
      expect(result[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
      expect(result[1]).to.deep.include({ name: "Beta", quantity: 4, value: 2.5, flag: false });
    });

    it("treats every row as data when hasHeader is not set", () => {
      db = createECDb("importCsvFileNoHeader.ecdb");
      const csvPath = writeCsv("importCsvFileNoHeader.csv", "Alpha,3,1.5,true\n");

      const rowCount = db.importCSVFile("Test.Foo", csvPath, mapping);
      expect(rowCount).eq(1);
      expect(readAllFooRows(db)[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
    });

    it("throws and rolls back the whole import when a record fails to bind", () => {
      db = createECDb("importCsvFileRollback.ecdb");
      const csvPath = writeCsv("importCsvFileRollback.csv", "Alpha,3,1.5,true\nBeta,not-a-number,2.5,false\n");

      expect(() => db.importCSVFile("Test.Foo", csvPath, mapping)).to.throw();
      expect(readAllFooRows(db).length).eq(0);
    });

    it("throws when the CSV file does not exist", () => {
      db = createECDb("importCsvFileMissing.ecdb");
      const missingPath = path.join(outDir, "does-not-exist.csv");

      expect(() => db.importCSVFile("Test.Foo", missingPath, mapping)).to.throw();
    });
  });
});
