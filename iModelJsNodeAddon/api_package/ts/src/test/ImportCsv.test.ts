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
  `<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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

const subnormalDoubleCases = [
  { name: "1e-310", text: "1e-310", expected: 1e-310 },
  { name: "Number.MIN_VALUE", text: String(Number.MIN_VALUE), expected: Number.MIN_VALUE },
];

const booleanCases = [
  { text: "TRUE", expected: true },
  { text: "True", expected: true },
  { text: "FALSE", expected: false },
  { text: "False", expected: false },
];

const embeddedNulCases = [
  { name: "string", columnIndex: 0, value: "prefix\0suffix" },
  { name: "unicode-string", columnIndex: 0, value: "\u7528\u6237\0suffix" },
  { name: "integer", columnIndex: 1, value: "123\0junk" },
  { name: "double", columnIndex: 2, value: "1.5\0junk" },
  { name: "boolean", columnIndex: 3, value: "true\0junk" },
  { name: "null-marker", columnIndex: 2, value: "NULL\0junk" },
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
        ["Café", "3", "1.5", "true"],
        ["Compass 🧭", "4", "2.5", "false"],
      ];
      const bytes = v8.serialize(rows);

      const rowCount = db.importCSVData("Test.Foo", bytes, mapping);
      expect(rowCount).eq(2);

      const result = readAllFooRows(db);
      expect(result.length).eq(2);
      expect(result[0]).to.deep.include({ name: "Café", quantity: 3, value: 1.5, flag: true });
      expect(result[1]).to.deep.include({ name: "Compass 🧭", quantity: 4, value: 2.5, flag: false });
    });

    it("imports boolean values case-insensitively", () => {
      db = createECDb("importCsvDataBooleanCase.ecdb");
      const rows = booleanCases.map(({ text }, index) => [`Row ${index}`, `${index}`, "1.5", text]);

      expect(db.importCSVData("Test.Foo", v8.serialize(rows), mapping)).eq(booleanCases.length);
      expect(readAllFooRows(db).map(({ flag }) => flag)).to.deep.equal(booleanCases.map(({ expected }) => expected));
    });

    for (const testCase of subnormalDoubleCases) {
      it(`imports a representable subnormal double (${testCase.name})`, () => {
        db = createECDb(`importCsvDataSubnormal-${testCase.name}.ecdb`);
        const bytes = v8.serialize([["Alpha", "3", testCase.text, "true"]]);

        expect(db.importCSVData("Test.Foo", bytes, mapping)).eq(1);
        expect(readAllFooRows(db)[0].value).eq(testCase.expected);
      });
    }

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

    it("imports repeated references to the same row array", () => {
      db = createECDb("importCsvDataRepeatedRow.ecdb");
      const row = ["Alpha", "3", "1.5", "true"];
      const bytes = v8.serialize([row, ["Beta", "4", "2.5", "false"], row]);

      expect(db.importCSVData("Test.Foo", bytes, mapping)).eq(3);
      expect(readAllFooRows(db)).to.deep.equal([
        { name: "Alpha", quantity: 3, value: 1.5, flag: true, isValueNull: false },
        { name: "Alpha", quantity: 3, value: 1.5, flag: true, isValueNull: false },
        { name: "Beta", quantity: 4, value: 2.5, flag: false, isValueNull: false },
      ]);
    });

    it("imports repeated references to a filled sparse row array", () => {
      db = createECDb("importCsvDataRepeatedSparseRow.ecdb");
      const row = new Array<string>(4);
      for (const [index, value] of ["Alpha", "3", "1.5", "true"].entries())
        row[index] = value;

      expect(db.importCSVData("Test.Foo", v8.serialize([row, row]), mapping)).eq(2);
      expect(readAllFooRows(db)).to.deep.equal([
        { name: "Alpha", quantity: 3, value: 1.5, flag: true, isValueNull: false },
        { name: "Alpha", quantity: 3, value: 1.5, flag: true, isValueNull: false },
      ]);
    });

    it("rejects cyclic serialized arrays without inserting rows", () => {
      db = createECDb("importCsvDataCycle.ecdb");
      const row: unknown[] = ["Alpha", "3", "1.5", "true"];
      row[0] = row;

      expect(() => db.importCSVData("Test.Foo", v8.serialize([row]), mapping)).to.throw(/cyclic object references/);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("rejects an invalid serialized object reference without inserting rows", () => {
      db = createECDb("importCsvDataInvalidReference.ecdb");
      const row = ["Alpha", "3", "1.5", "true"];
      const bytes = v8.serialize([row, row]);
      const referenceOffset = bytes.lastIndexOf(Buffer.from([0x5e, 0x01]));
      expect(referenceOffset).to.be.greaterThan(-1);
      bytes[referenceOffset + 1] = 0x7f;

      expect(() => db.importCSVData("Test.Foo", bytes, mapping)).to.throw(/invalid object reference ID/);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("rejects nested shared arrays instead of expanding them into a value tree", () => {
      db = createECDb("importCsvDataNestedReferences.ecdb");
      let rows: unknown[] = ["Alpha"];
      for (let depth = 0; depth < 24; ++depth)
        rows = [rows, rows];

      expect(() => db.importCSVData("Test.Foo", v8.serialize(rows), mapping))
        .to.throw(/only primitive scalar values/);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    for (const testCase of embeddedNulCases) {
      it(`rejects an embedded NUL in a mapped ${testCase.name} and rolls back`, () => {
        db = createECDb(`importCsvDataNul-${testCase.name}.ecdb`);
        const invalidRow = ["Beta", "4", "2.5", "false"];
        invalidRow[testCase.columnIndex] = testCase.value;
        const bytes = v8.serialize([["Alpha", "3", "1.5", "true"], invalidRow]);

        expect(() => db.importCSVData("Test.Foo", bytes, mapping, { nullValue: "NULL" }))
          .to.throw(`CSV data row 2 column ${testCase.columnIndex}`);
        expect(readAllFooRows(db)).to.deep.equal([]);
      });
    }

    it("rejects an embedded NUL in the nullValue option", () => {
      db = createECDb("importCsvDataNulOption.ecdb");
      const bytes = v8.serialize([["Alpha", "3", "", "true"]]);

      expect(() => db.importCSVData("Test.Foo", bytes, mapping, { nullValue: "\0junk" })).to.throw(/nullValue/);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("ignores embedded NUL values in unmapped columns", () => {
      db = createECDb("importCsvDataUnmappedNul.ecdb");
      const bytes = v8.serialize([["Alpha", "3", "1.5", "true", "ignored\0value"]]);

      expect(db.importCSVData("Test.Foo", bytes, mapping)).eq(1);
      expect(readAllFooRows(db)[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
    });

    it("throws and rolls back the whole import when a later row fails to bind", () => {
      db = createECDb("importCsvDataRollback.ecdb");
      db.saveChanges();
      expect(db.importCSVData("Test.Foo", v8.serialize([["Existing", "1", "1.0", "true"]]), mapping)).eq(1);
      const rows = [
        ["Alpha", "3", "1.5", "true"],
        ["Beta", "not-a-number", "2.5", "false"],
      ];
      const bytes = v8.serialize(rows);

      expect(() => db.importCSVData("Test.Foo", bytes, mapping)).to.throw();
      expect(readAllFooRows(db)).to.deep.equal([
        { name: "Existing", quantity: 1, value: 1, flag: true, isValueNull: false },
      ]);
      db.abandonChanges();
      expect(readAllFooRows(db)).to.deep.equal([]);
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

    it("imports boolean values case-insensitively", () => {
      db = createECDb("importCsvFileBooleanCase.ecdb");
      const content = booleanCases.map(({ text }, index) => `Row ${index},${index},1.5,${text}`).join("\n");
      const csvPath = writeCsv("importCsvFileBooleanCase.csv", content);

      expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(booleanCases.length);
      expect(readAllFooRows(db).map(({ flag }) => flag)).to.deep.equal(booleanCases.map(({ expected }) => expected));
    });

    for (const testCase of subnormalDoubleCases) {
      it(`imports a representable subnormal double (${testCase.name})`, () => {
        db = createECDb(`importCsvFileSubnormal-${testCase.name}.ecdb`);
        const csvPath = writeCsv(`importCsvFileSubnormal-${testCase.name}.csv`, `Alpha,3,${testCase.text},true\n`);

        expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(1);
        expect(readAllFooRows(db)[0].value).eq(testCase.expected);
      });
    }

    it("treats every row as data when hasHeader is not set", () => {
      db = createECDb("importCsvFileNoHeader.ecdb");
      const csvPath = writeCsv("importCsvFileNoHeader.csv", "Alpha,3,1.5,true\n");

      const rowCount = db.importCSVFile("Test.Foo", csvPath, mapping);
      expect(rowCount).eq(1);
      expect(readAllFooRows(db)[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
    });

    for (const hasHeader of [false, true]) {
      it(`handles a UTF-8 BOM before a quoted first field with hasHeader=${hasHeader}`, () => {
        db = createECDb(`importCsvFileBom-${hasHeader}.ecdb`);
        const header = hasHeader ? '"Name",Quantity,Amount,Flag\r\n' : "";
        const csvPath = writeCsv(`importCsvFileBom-${hasHeader}.csv`, `\uFEFF${header}"Alpha",3,1.5,true\r\n`);

        expect(db.importCSVFile("Test.Foo", csvPath, mapping, { hasHeader })).eq(1);
        expect(readAllFooRows(db)[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
      });
    }

    it("preserves a BOM character inside a quoted field", () => {
      db = createECDb("importCsvFileBomInField.ecdb");
      const csvPath = writeCsv("importCsvFileBomInField.csv", '"\uFEFFAlpha",3,1.5,true\n');

      expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(1);
      expect(readAllFooRows(db)[0].name).eq("\uFEFFAlpha");
    });

    it("treats a BOM-only file as empty", () => {
      db = createECDb("importCsvFileBomOnly.ecdb");
      const csvPath = writeCsv("importCsvFileBomOnly.csv", "\uFEFF");

      expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(0);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("handles a BOM and quoted fields crossing the file buffer boundary", () => {
      db = createECDb("importCsvFileBomLarge.ecdb");
      const name = `${"x".repeat(64 * 1024 - 5)}"\nend`;
      const csvPath = writeCsv("importCsvFileBomLarge.csv", `\uFEFF"${name.replace(/"/g, '""')}",3,1.5,true\n`);

      expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(1);
      expect(readAllFooRows(db)[0].name).eq(name);
    });

    for (const testCase of embeddedNulCases) {
      it(`rejects an embedded NUL in a mapped ${testCase.name} and rolls back`, () => {
        db = createECDb(`importCsvFileNul-${testCase.name}.ecdb`);
        const invalidRow = ["Beta", "4", "2.5", "false"];
        invalidRow[testCase.columnIndex] = testCase.value;
        const csvPath = writeCsv(`importCsvFileNul-${testCase.name}.csv`, `Alpha,3,1.5,true\n${invalidRow.join(",")}\n`);

        expect(() => db.importCSVFile("Test.Foo", csvPath, mapping, { nullValue: "NULL" }))
          .to.throw(`CSV record 2 column ${testCase.columnIndex}`);
        expect(readAllFooRows(db)).to.deep.equal([]);
      });
    }

    it("rejects an embedded NUL in the nullValue option", () => {
      db = createECDb("importCsvFileNulOption.ecdb");
      const csvPath = writeCsv("importCsvFileNulOption.csv", "Alpha,3,,true\n");

      expect(() => db.importCSVFile("Test.Foo", csvPath, mapping, { nullValue: "\0junk" })).to.throw(/nullValue/);
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("ignores embedded NUL values in unmapped columns", () => {
      db = createECDb("importCsvFileUnmappedNul.ecdb");
      const csvPath = writeCsv("importCsvFileUnmappedNul.csv", "Alpha,3,1.5,true,ignored\0value\n");

      expect(db.importCSVFile("Test.Foo", csvPath, mapping)).eq(1);
      expect(readAllFooRows(db)[0]).to.deep.include({ name: "Alpha", quantity: 3, value: 1.5, flag: true });
    });

    it("throws and rolls back the whole import when a record fails to bind", () => {
      db = createECDb("importCsvFileRollback.ecdb");
      db.saveChanges();
      const existingPath = writeCsv("importCsvFileExisting.csv", "Existing,1,1.0,true\n");
      expect(db.importCSVFile("Test.Foo", existingPath, mapping)).eq(1);
      const csvPath = writeCsv("importCsvFileRollback.csv", "Alpha,3,1.5,true\nBeta,not-a-number,2.5,false\n");

      expect(() => db.importCSVFile("Test.Foo", csvPath, mapping)).to.throw();
      expect(readAllFooRows(db)).to.deep.equal([
        { name: "Existing", quantity: 1, value: 1, flag: true, isValueNull: false },
      ]);
      db.abandonChanges();
      expect(readAllFooRows(db)).to.deep.equal([]);
    });

    it("throws when the CSV file does not exist", () => {
      db = createECDb("importCsvFileMissing.ecdb");
      const missingPath = path.join(outDir, "does-not-exist.csv");

      expect(() => db.importCSVFile("Test.Foo", missingPath, mapping)).to.throw();
    });
  });
});
