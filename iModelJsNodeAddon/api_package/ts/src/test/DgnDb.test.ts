/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64Array, Id64String, IModelStatus, OpenMode } from "@itwin/core-bentley";
import { BlobRange, DbBlobRequest, DbBlobResponse, DbQueryRequest, DbQueryResponse, DbRequestKind, DbResponseStatus, ProfileOptions } from "@itwin/core-common";
import { DomainOptions } from "@itwin/core-common/lib/cjs/BriefcaseTypes";
import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";

// Crash reporting on linux is gated by the presence of this env variable.
if (os.platform() === "linux")
  process.env.LINUX_MINIDUMP_ENABLED = "yes";

describe("basic tests", () => {

  let dgndb: IModelJsNative.DgnDb;

  before((done) => {
    dgndb = openDgnDb(dbFileName, { schemaLockHeld: true });
    done();
  });

  after((done) => {
    dgndb.closeIModel();
    done();
  });

  // verify that throwing javascript exceptions from C++ works
  it("testExceptions", () => {
    // first try a function
    expect(() => (iModelJsNative as any).addFontWorkspace()).to.throw("Argument 0");

    // now try methods
    const db = new iModelJsNative.DgnDb() as any;
    expect(() => db.openIModel()).to.throw("Argument 0");
    expect(() => db.saveFileProperty()).to.throw("requires 2");

    // from Node
    expect(() => db.nonsense()).to.throw("not a function");
  });

  it("testTileVersionInfo", () => {
    const ver = iModelJsNative.getTileVersionInfo();
    assert.isTrue(typeof ver !== "undefined");
  });

  it("testSimpleDbQueries", () => {
    assert.isTrue(dgndb.isOpen());
    assert.isFalse(dgndb.isReadonly());
    assert.isFalse(dgndb.isRedoPossible());
    expect(() => dgndb.deleteElement("0x33333")).to.throw("missing id");
  });

  it("testExportGraphicsBasics", () => {
    // Find all 3D elements in the test file
    const elementIdArray: Id64Array = [];
    const statement = new iModelJsNative.ECSqlStatement();
    statement.prepare(dgndb, "SELECT ECInstanceId FROM bis.GeometricElement3d");
    while (DbResult.BE_SQLITE_ROW === statement.step())
      elementIdArray.push(statement.getValue(0).getId());
    statement.dispose();

    assert(elementIdArray.length > 0, "No 3D elements in test file");
    // Expect a mesh to be generated for each element - valid for test.bim, maybe invalid for future test data
    const elementsWithGraphics: any = {};
    const onGraphics = (info: any) => {
      elementsWithGraphics[info.elementId] = true;
    };
    const res = dgndb.exportGraphics({ elementIdArray, onGraphics });

    assert.equal(res, 0, `IModelDb.exportGraphics returned ${res}`);
    for (const id of elementIdArray)
      assert.isDefined(elementsWithGraphics[id], `No graphics generated for ${id}`);
  });

  it("testSchemaImport", () => {
    const writeDbFileName = copyFile("testSchemaImport.bim", dbFileName);
    // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCore.01.00.15'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
    const db = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    assert.isTrue(db !== undefined);
    expect(() => db.getSchemaProps("PresentationRules")).to.throw("schema not found"); // presentationrules alias is 'pr'.
    let bisProps = db.getSchemaProps("BisCore");
    assert.isTrue(bisProps.version === "01.00.00");
    const schemaPath = path.join(iModelJsNative.DgnDb.getAssetsDir(), "ECSchemas/Domain/PresentationRules.ecschema.xml");
    const result = db.importSchemas([schemaPath], { schemaLockHeld: false });
    assert.isTrue(result === DbResult.BE_SQLITE_OK);

    db.getSchemaProps("PresentationRules");
    bisProps = db.getSchemaProps("BisCore");
    assert.isTrue(bisProps.version >= "01.00.15"); // PR references 01.00.15+, so importing PR will cause it to upgrade.
  });

  it("testSchemaImportPrefersExistingAndLocalOverStandard", () => {
    const testFileName = copyFile("testSchemaImportPrefersExistingOverStandard.bim", dbFileName);
    const db = openDgnDb(testFileName);
    const assetsDir = path.join(getAssetsDir(), "ImportSchemaTests");
    const test100Path = path.join(assetsDir, "Test.01.00.00.ecschema.xml");

    // BisCore will not be updated because Test only requests BisCore.01.00.00 which is already in the db
    // db has higher precedence than standard schema paths so BisCore from the db is used as the schema ref
    const bisProps = db.getSchemaProps("BisCore");
    let result = db.importSchemas([test100Path], { schemaLockHeld: false });
    assert.equal(result, DbResult.BE_SQLITE_OK);
    assert.equal(db.getSchemaProps("BisCore").version, bisProps.version, "BisCore after Test 1.0.0 import");

    const testRefProps = db.getSchemaProps("TestRef");
    assert.equal(testRefProps.version, "01.00.00", "TestRef after Test 1.0.0 import");

    // TestRef is updated to version 1.0.1 even though Test only references 1.0.0
    // local directory has higher precedence than the db
    const subAssetsDir = path.join(assetsDir, "LocalReferences");
    const test101Path = path.join(subAssetsDir, "Test.01.00.01.ecschema.xml");
    result = db.importSchemas([test101Path], { schemaLockHeld: false });
    assert.equal(result, DbResult.BE_SQLITE_OK);
    assert.equal(db.getSchemaProps("TestRef").version, "01.00.01", "TestRef after Test 1.0.1 import");
  });

  it("testSchemaExport", () => {
    const xml = dgndb.schemaToXmlString("BisCore", IModelJsNative.ECVersion.V2_0);
    assert.isString(xml);
    if (xml === undefined)
      return;
    assert.isTrue(xml.includes("http://www.bentley.com/schemas/Bentley.ECXML.2.0"));

    const xml32 = dgndb.schemaToXmlString("BisCore", IModelJsNative.ECVersion.V3_2);
    assert.isString(xml32);
    if (xml32 === undefined)
      return;
    assert.isTrue(xml32.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));

    const notThere = dgndb.schemaToXmlString("NotThere.NotThere", IModelJsNative.ECVersion.V2_0);
    assert.isUndefined(notThere);
  });

  it("testCrashReportingConfig", () => {
    if (os.platform() === "darwin" || process.env.AddressSanitizer === "yes") {
      // Currently unsupported on the Mac.
      // With Address Sanitizer it fails with 'JsInterop::InitializeCrashReporting: Failed to start the crashpad handler'
      return;
    }
    iModelJsNative.setCrashReporting({
      enableCrashDumps: true,
      crashDir: __dirname,
      params: [
        { name: "foo", value: "bar" },
        { name: "foo2", value: "baz" },
      ],
    });

    iModelJsNative.setCrashReportProperty("dynamic1", "1");
    iModelJsNative.setCrashReportProperty("dynamic2", "2");
    iModelJsNative.setCrashReportProperty("dynamic2", "22");
    iModelJsNative.setCrashReportProperty("dynamic3", "3");
    iModelJsNative.setCrashReportProperty("dynamic3", undefined);

    const props = iModelJsNative.getCrashReportProperties();
    assert.isTrue(undefined !== props.find((nvpair) => nvpair.name === "foo" && nvpair.value === "bar"));
    assert.isTrue(undefined !== props.find((nvpair) => nvpair.name === "foo2" && nvpair.value === "baz"));
    assert.isTrue(undefined !== props.find((nvpair) => nvpair.name === "dynamic1" && nvpair.value === "1"));
    assert.isTrue(undefined !== props.find((nvpair) => nvpair.name === "dynamic2" && nvpair.value === "22"));
    assert.isTrue(undefined === props.find((nvpair) => nvpair.name === "dynamic3"));

    // iModelJsNative.NativeDevTools.signal(3);
  });

  it("should be able to open partially valid db", async () => {
    // InvalidFile.bim has a valid sqlite db header, so it passes the isValidDbFile check, but has an overall invalid sqlite structure.
    // So, It then fails with SQLITE_NOTADB when statements are stepped and prepared on it.
    const pathToDb = path.join(getAssetsDir(), "InvalidFile.bim");

    const db = new iModelJsNative.SQLiteDb();
    // rawSQLite being false causes us to look for the presence of be_prop table, which gives us SQLITE_NOTADB
    expect(() => db.openDb(pathToDb, {openMode: OpenMode.ReadWrite})).to.throw("file is not a database");
    // rawSQLite being true skips be_prop check so we can open the database, but will fail later on if we step and prepare on the db.
    expect(() => db.openDb(pathToDb, {openMode: OpenMode.ReadWrite, rawSQLite: true})).to.not.throw();
    db.closeDb();
  });

  it("WAL mode", () => {
    const withWal = new iModelJsNative.DgnDb();
    const tempDbName = path.join(getOutputDir(), "testWal.bim");
    if (fs.existsSync(tempDbName))
      fs.removeSync(tempDbName);

    withWal.createIModel(tempDbName, { rootSubject: { name: "wal" } });
    withWal.enableWalMode();
    withWal.performCheckpoint();
    withWal.setAutoCheckpointThreshold(2000);
    withWal.closeIModel();
  });

  it("testGetSchemaProps", async () => {
    assert.isTrue(dgndb.isOpen());
    expect(() => dgndb.getSchemaProps("DoesNotExist")).to.throw("schema not found");
    const props = dgndb.getSchemaProps("BisCore");
    expect(props.name).equal("BisCore");
  });

  it("testGetSchemaPropsAsync", async () => {
    assert.isTrue(dgndb.isOpen());
    await expect(dgndb.getSchemaPropsAsync("DoesNotExist")).rejectedWith("schema not found");
    const props = await dgndb.getSchemaPropsAsync("BisCore");
    expect(props.name).equal("BisCore");
  });

  function roundCoords(xyz: number[]): number[] {
    return xyz.map((w) => Math.round(w));
  }

  interface Extents {
    low: number[];
    high: number[];
  }

  function expectExtents(actual: any, expected: Extents) {
    expect(actual.low.length).to.equal(3);
    expect(actual.high.length).to.equal(3);
    expect(roundCoords(actual.low)).to.deep.equal(expected.low);
    expect(roundCoords(actual.high)).to.deep.equal(expected.high);
  }

  function expectNullExtents(extents: any): void {
    expect(extents.low.length).to.equal(3);
    expect(extents.high.length).to.equal(3);
    for (let i = 0; i < 3; i++)
      expect(extents.low[i]).least(extents.high[i]);
  }

  it("queryModelExtents", () => {
    expect(() => dgndb.queryModelExtents({ id: "NotAnId" })).to.throw("Invalid id").property("errorNumber").equal(IModelStatus.InvalidId);
    expect(() => dgndb.queryModelExtents({ id: "0xabcdef" })).to.throw("not found").property("errorNumber").equal(IModelStatus.NotFound);
    expect(() => dgndb.queryModelExtents({ id: "0x1" })).to.throw("error=10040").property("errorNumber").equal(IModelStatus.WrongModel);
    expect(() => dgndb.queryModelExtents({ id: "0x1c" })).to.throw("error=10022").property("errorNumber").equal(IModelStatus.NoGeometry);

    expectExtents(dgndb.queryModelExtents({ id: "0x23" }).modelExtents, { low: [-10, -16, -10], high: [14, 6, 10] });
  });

  it("queryModelExtentsAsync", async () => {
    async function expectResult(modelId: string, expected: IModelStatus | Extents | "null"): Promise<void> {
      const results = await dgndb.queryModelExtentsAsync([modelId]);
      expect(results.length).to.equal(1);
      const result = results[0];
      expect(result.id).to.equal(modelId);
      if (typeof expected === "number") {
        expect(result.status).to.equal(expected);
        expectNullExtents(result.extents);
      } else {
        expect(result.status).to.equal(IModelStatus.Success);
        if ("null" === expected)
          expectNullExtents(result.extents);
        else
          expectExtents(result.extents, expected);
      }
    }

    await expectResult("0", IModelStatus.InvalidId);
    await expectResult("0xabcdef", IModelStatus.NotFound);
    await expectResult("0x1", IModelStatus.WrongModel);
    await expectResult("0x1c", "null");
    await expectResult("0x23", { low: [-10, -16, -10], high: [14, 6, 10] });
  });

  it("queryModelExtentsAsync supports multiple models", async () => {
    const results = await dgndb.queryModelExtentsAsync(["0", "0xabcdef", "0x1", "0x1c", "0x23", "0x24"]);
    expect(results.length).to.equal(6);

    function expectResult(index: number, expected: IModelStatus | Extents | "null"): void {
      const result = results[index];
      if (typeof expected === "number") {
        expect(result.status).to.equal(expected);
        expectNullExtents(result.extents);
      } else {
        expect(result.status).to.equal(IModelStatus.Success);
        if ("null" === expected)
          expectNullExtents(result.extents);
        else
          expectExtents(result.extents, expected);
      }
    }

    expectResult(0, IModelStatus.InvalidId);
    expectResult(1, IModelStatus.NotFound);
    expectResult(2, IModelStatus.WrongModel);
    expectResult(3, "null");
    expectResult(4, { low: [-10, -16, -10], high: [14, 6, 10] });
    expectResult(5, { low: [5, 3, -10], high: [30, 25, 10] });
  });

  // NB: The test iModel contains 4 spheres and no other geometry.
  describe("generateElementMeshes", () => {
    it("throws if source is not a geometric element", async () => {
      const msg = "Geometric element required";
      await expect(dgndb.generateElementMeshes({ source: "NotAnId" })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({})).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0" })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0x1" })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0x123456789" })).rejectedWith(msg);
    });

    it("produces meshes", async () => {
      const elemIds = ["0x38", "0x3a", "0x3b", "0x39"];
      for (const source of elemIds) {
        const bytes = await dgndb.generateElementMeshes({
          source,
          chordTolerance: 0.001,
        });

        const numBytes = bytes.length;
        expect(numBytes).least(32);

        for (let i = 0; i < 4; i++)
          expect(bytes[i]).to.equal("LMSH".charCodeAt(i));

        let u32 = new Uint32Array(bytes.buffer);
        expect(u32[1]).to.equal(0);

        for (let i = 0; i < 4; i++)
          expect(bytes[8 + i]).to.equal("PLFC".charCodeAt(i));

        const numPolyfaceBytes = u32[3];
        expect(numPolyfaceBytes).least(8);

        const fewerBytes = await dgndb.generateElementMeshes({
          source,
          chordTolerance: 0.1,
        });

        expect(fewerBytes.length < bytes.length).to.be.true;
        expect(fewerBytes.length).least(32);

        u32 = new Uint32Array(fewerBytes.buffer);
        expect(u32[3] < numPolyfaceBytes).to.be.true;
      }
    });
  });

  it("profile/schema upgrade with schemaLockHeld flag", async () => {
    /**
     * This test verify if schemaLockHeld is true profile upgrade should not fail when imodel have overflow tables.
     */
    const originalFile = path.join(getAssetsDir(), "test-with-overflow.bim");
    const testFile = path.join(getAssetsDir(), "test-with-overflow-upgrade.bim");
    if (fs.existsSync(testFile)) {
      fs.unlinkSync(testFile);
    }

    fs.copyFileSync(originalFile, testFile);

    const dbForProfileUpgrade = openDgnDb(testFile, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    dbForProfileUpgrade.saveChanges();
    dbForProfileUpgrade.closeIModel();

    const dbForSchemaUpgrade = openDgnDb(testFile, { domain: DomainOptions.Upgrade, schemaLockHeld: true });
    dbForSchemaUpgrade.saveChanges();
    dbForSchemaUpgrade.closeIModel();
  });

  it("import schema with schemaLockHeld flag", async () => {
    let t = 0;
    const generateIntProp = (propCount: number, prefix: string = "P") => {
      let xml = "";
      for (let i = 0; i < Math.max(propCount, 1); ++i) {
        xml += `<ECProperty propertyName="${prefix}${i}" typeName="int"/>\n`;
      }
      return xml;
    };
    const generateSchema = (classPropCount: number, structPropCount: number) => {
      const schemaXml = `<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="SchemaVersionTest" alias="vt" version="01.00.0${t++}" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
          <ECStructClass typeName="ChangeInfoStruct" modifier="None">
            ${generateIntProp(structPropCount)}
          </ECStructClass>
          <ECEntityClass typeName="TestElement">
            <BaseClass>bis:GeometricElement2d</BaseClass>
            <ECStructProperty propertyName="structProp" typeName="ChangeInfoStruct" />
            ${generateIntProp(classPropCount)}
          </ECEntityClass>
      </ECSchema>`;
      return schemaXml;
    };
    const testFile = dbFileName.replace("test.bim", "test-schema-import.bim");
    if (fs.existsSync(testFile)) {
      fs.unlinkSync(testFile);
    }
    fs.copyFileSync(dbFileName, testFile);
    const db = openDgnDb(testFile);
    db.saveChanges();

    // import a schema with changes that only required SchemaLock
    let rc = db.importXmlSchemas([generateSchema(20, 1)], { schemaLockHeld: false });
    assert.equal(rc, DbResult.BE_SQLITE_OK);
    db.saveChanges();

    // eslint-disable-next-line @typescript-eslint/naming-convention
    const BE_SQLITE_ERROR_DataTransformRequired = (DbResult.BE_SQLITE_IOERR | 23 << 24);

    // import should fail when schemaLockHeld flag is set to false which will fail the operation if data transform is required.
    rc = db.importXmlSchemas([generateSchema(20, 20)], { schemaLockHeld: false });
    assert.equal(rc, BE_SQLITE_ERROR_DataTransformRequired);

    // import should be successful when schemaLockHeld flag is set to true so it can transform data if required.
    rc = db.importXmlSchemas([generateSchema(20, 20)], { schemaLockHeld: true });
    assert.equal(rc, DbResult.BE_SQLITE_OK);
    db.saveChanges();
    db.closeIModel();
  });

  it("ecsql query", async () => {
    const query = async (ecsql: string) => {
      const request: DbQueryRequest = {
        kind: DbRequestKind.ECSql,
        query: ecsql,
      };
      return new Promise<DbQueryResponse>((resolve) => {
        dgndb.concurrentQueryExecute(request, (response) => {
          resolve(response as DbQueryResponse);
        });
      });
    };
    const resp = await query("SELECT ECInstanceId FROM Bis.Element LIMIT 5");
    assert(resp.status === DbResponseStatus.Done);
    assert(resp.rowCount === 5);
    assert(resp.error === "");
    assert(resp.data[0], "0x1");
    assert(resp.data[1], "0xe");
    assert(resp.data[2], "0x10");
    assert(resp.data[3], "0x11");
    assert(resp.data[4], "0x13");
  });

  it("blob query", async () => {
    const query = async (className: string, accessString: string, instanceId: Id64String, range?: BlobRange) => {
      const request: DbBlobRequest = {
        kind: DbRequestKind.BlobIO,
        className,
        accessString,
        instanceId,
        range,
      };
      return new Promise<DbBlobResponse>((resolve) => {
        dgndb.concurrentQueryExecute(request, (response) => {
          resolve(response as DbBlobResponse);
        });
      });
    };
    const resp = await query("Bis.GeometricElement3d", "GeometryStream", "0x39");
    assert(resp.status === DbResponseStatus.Done);
    assert(resp.rawBlobSize === 201);
    assert(resp.data instanceof Uint8Array);
    assert(resp.data?.length === 201);
    assert(Buffer.from(resp.data).toString("base64") === "yQCAAjAABgAA+AAAAAEAAAAIDQgBAUAEAAAAMAAAABwAAAAYABQADAUeEQEIBgAHBRgBAQgBAf8VDEgAAAsAAACoAAAAYmcwMDAxZmIQASMUAAoADgAHBUIACgUQCAAHDAUIiAYAfAAEAAYAAAC2dX71ziceQHxSG5uae5m8PEMbZw/wtDwABSgoAAC8DXhGH+CTPLgNKAkXBNC8CQggAAC2vz2w0buqDRAIwDy3MigAJAAAGC1EVPsh+b8JCCQJQAEAAAAAAAAA");
  });

  it("blob query with range", async () => {
    const query = async (className: string, accessString: string, instanceId: Id64String, range?: BlobRange) => {
      const request: DbBlobRequest = {
        kind: DbRequestKind.BlobIO,
        className,
        accessString,
        instanceId,
        range,
      };
      return new Promise<DbBlobResponse>((resolve) => {
        dgndb.concurrentQueryExecute(request, (response) => {
          resolve(response as DbBlobResponse);
        });
      });
    };
    const resp = await query("Bis.GeometricElement3d", "GeometryStream", "0x39", { offset: 5, count: 10 });
    assert(resp.status === DbResponseStatus.Done);
    assert(resp.rawBlobSize === 201);
    assert(resp.data instanceof Uint8Array);
    assert(resp.data?.length === 10);
    assert(Buffer.from(resp.data).toString("base64") === "AAYAAPgAAAABAA==");
  });

  it("testSimpleDbQueries", () => {
    assert.isTrue(dgndb.isOpen());
    assert.isFalse(dgndb.isReadonly());
    assert.isFalse(dgndb.isRedoPossible());
    expect(() => dgndb.deleteElement("0x33333")).to.throw("missing id");
  });

  it("testGetECClassMetaData custom attributes", async () => {
    assert.isTrue(dgndb.isOpen());
    const result = dgndb.getECClassMetaData("BisCore", "ISubModeledElement");
    assert(result.result);
    const classMetaData = JSON.parse(result.result);
    expect(classMetaData).to.deep.equal({
      ecclass: "BisCore:ISubModeledElement",
      description:
        "An interface which indicates that an Element can be broken down or described by a (sub) Model.  " +
        "This interface is mutually exclusive with IParentElement.",
      modifier: "Abstract",
      displayLabel: "Modellable Element",
      baseClasses: [],
      customAttributes: [{
        ecclass: "CoreCustomAttributes:IsMixin",
        properties: {
          // eslint-disable-next-line @typescript-eslint/naming-convention
          AppliesToEntityClass: "Element",
        },
      }],
    });
  });

  describe("testConvertEC2XmlSchemas", () => {
    const query = async (db: IModelJsNative.DgnDb, ecsql: string) => {
      const request: DbQueryRequest = {
        kind: DbRequestKind.ECSql,
        query: ecsql,
      };
      return new Promise<DbQueryResponse>((resolve) => {
        db.concurrentQueryExecute(request, (response) => {
          resolve(response as DbQueryResponse);
        });
      });
    };

    it("verify namespace", () => {
      const ec2SchemaXml = `<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECSchemaReference name="RefSchema" version="01.00" prefix="rs" />
          <ECClass typeName="TestEntityClass" isDomainClass="true" />
        </ECSchema>`;

      const ec2RefSchema = `<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" nameSpacePrefix="rs" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECClass typeName="TestStructClass" isStruct="true" />
        </ECSchema>`;

      const ec3Schemas: string[] = iModelJsNative.SchemaUtility.convertEC2XmlSchemas([ec2SchemaXml, ec2RefSchema]);
      assert.equal(ec3Schemas.length, 2);
      // converted EC3 schemas are always sorted in dependancy order: RefSchema-> TestSchema
      const ec3RefSchema = ec3Schemas[0];
      const ec3SchemaXml = ec3Schemas[1];

      assert.isTrue(ec3RefSchema.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
    });

    it("rename reserved words", () => {
      const ec2SchemaXml = `<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECSchemaReference name='BisCore' version='01.00' prefix='bis' />
          <ECClass typeName="TestEntityClass" isDomainClass="true">
            <BaseClass>bis:GraphicalElement3d</BaseClass>
            <ECProperty propertyName="Id" typeName="string" />
            <ECProperty propertyName="ECInstanceId" typeName="string" />
            <ECProperty propertyName="ECClassId" typeName="string" />
            <ECProperty propertyName="SourceECInstanceId" typeName="string" />
            <ECProperty propertyName="SourceId" typeName="string" />
            <ECProperty propertyName="SourceECClassId" typeName="string" />
            <ECProperty propertyName="TargetECInstanceId" typeName="string" />
            <ECProperty propertyName="TargetId" typeName="string" />
            <ECProperty propertyName="TargetECClassId" typeName="string" />
          </ECClass>
          <ECClass typeName="TestStructClass" isStruct="true">
            <BaseClass>bis:GraphicalElement3d</BaseClass>
            <ECProperty propertyName="Id" typeName="string" />
            <ECProperty propertyName="ECInstanceId" typeName="string" />
            <ECProperty propertyName="ECClassId" typeName="string" />
          </ECClass>
        </ECSchema>`;

      const ec3Schemas: string[] = iModelJsNative.SchemaUtility.convertEC2XmlSchemas([ec2SchemaXml]);

      const writeDbFileName = copyFile("RenameReservedWords.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      const rc = db.importXmlSchemas(ec3Schemas, { schemaLockHeld: true });
      assert.equal(rc, DbResult.BE_SQLITE_OK);
      db.saveChanges();

      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("TestSchema");
      expect(schema.name).equal("TestSchema");
      assert.isTrue(schema.version === "01.00.00");

      // Function to check if a property exists in the given class/struct metadata
      const doesPropertyExist = (metadata: any, propertyName: string) => {
        const properties = metadata.properties;
        return Object.keys(properties).some((property: string) => property === propertyName);
      };

      const classMeta = db.getECClassMetaData("TestSchema", "TestEntityClass");
      assert(classMeta.result);
      const classMetaData = JSON.parse(classMeta.result);

      assert.isFalse(doesPropertyExist(classMetaData, "id")); // The id property is a reserved keyword and should have been renamed
      assert.isFalse(doesPropertyExist(classMetaData, "eCClassId"));  // The eCClassId property is a reserved keyword and should have been renamed
      assert.isFalse(doesPropertyExist(classMetaData, "eCInstanceId")); // The eCInstanceId property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(classMetaData, "testSchema_Id_"));  // The id property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(classMetaData, "testSchema_ECClassId_")); // The eCClassId property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(classMetaData, "testSchema_ECInstanceId_"));  // The eCInstanceId property is a reserved keyword and should have been renamed

      assert.isTrue(doesPropertyExist(classMetaData, "sourceECInstanceId"));  // The sourceECInstanceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(classMetaData, "sourceId"));  // The sourceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(classMetaData, "sourceECClassId")); // The sourceECClassId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(classMetaData, "targetECInstanceId"));  // The targetECInstanceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(classMetaData, "targetId"));  // The targetId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(classMetaData, "targetECClassId")); // The targetECClassId property is allowed on Entity classes and should not be renamed

      const structMeta = db.getECClassMetaData("TestSchema", "TestStructClass");
      assert(structMeta.result);
      const structMetaData = JSON.parse(structMeta.result);

      assert.isTrue(doesPropertyExist(structMetaData, "id")); // The id property is not a reserved keyword for Struct classes and should not be renamed
      assert.isTrue(doesPropertyExist(structMetaData, "eCClassId"));  // The eCClassId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isTrue(doesPropertyExist(structMetaData, "eCInstanceId")); // The eCInstanceId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(structMetaData, "testSchema_Id_"));  // The id property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(structMetaData, "testSchema_ECClassId_")); // The eCClassId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(structMetaData, "testSchema_ECInstanceId_"));  // The eCInstanceId property is not a reserved keyword for Struct classes and should not be renamed
    });

    it("enumeration in ref schema", async () => {
      const schemaXML = `<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
          <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />
          <ECSchemaReference name='TrapRef' version='78.00' prefix='trRef' />
          <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
            <BaseClass>trRef:D</BaseClass>
            <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>
              <ECCustomAttributes>
                <StandardValues xmlns='EditorCustomAttributes.01.00'>
                  <ValueMap>
                    <ValueMap>
                      <DisplayString>Sensei</DisplayString>
                      <Value>0</Value>
                    </ValueMap>
                  </ValueMap>
                </StandardValues>
              </ECCustomAttributes>
            </ECProperty>
          </ECClass>
        </ECSchema>`;

      const schemaXMLRef = `<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
          <ECSchemaReference name='BisCore' version='01.00' prefix='bis' />
          <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />
          <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
            <BaseClass>bis:GraphicalElement3d</BaseClass>
            <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>
              <ECCustomAttributes>
                <StandardValues xmlns='EditorCustomAttributes.01.00'>
                  <ValueMap>
                    <ValueMap>
                      <DisplayString>Sensei</DisplayString>
                      <Value>0</Value>
                    </ValueMap>
                  </ValueMap>
                </StandardValues>
              </ECCustomAttributes>
            </ECProperty>
         </ECClass>
        </ECSchema>`;

      const ec3Schemas: string[] = iModelJsNative.SchemaUtility.convertEC2XmlSchemas([schemaXML, schemaXMLRef]);
      // converted EC3 schemas are always sorted in dependancy order. Last schemas would be TrapRef -> Trap
      const ec3RefSchema = ec3Schemas[ec3Schemas.length - 2];
      const ec3SchemaXml = ec3Schemas[ec3Schemas.length - 1];

      assert.isTrue(ec3RefSchema.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));

      const writeDbFileName = copyFile("SchemaConvertEnum.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      const rc = db.importXmlSchemas(ec3Schemas, { schemaLockHeld: true });
      assert.equal(rc, DbResult.BE_SQLITE_OK);
      db.saveChanges();

      const refSchema: IModelJsNative.SchemaProps = db.getSchemaProps("TrapRef");
      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("Trap");

      // Enumeration should have been created in refschema
      let resp = await query(db, `SELECT e.Name FROM meta.ECSchemaDef s JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations WHERE s.Name='${refSchema.name}'`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.rowCount === 1);
      assert(resp.error === "");
      assert(resp.data[0], "D_TitleA");

      // Enumeration should not have been created in schema
      resp = await query(db, `SELECT e.Name, e.* FROM meta.ECSchemaDef s JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations WHERE s.Name='${schema.name}'`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.rowCount === 0);
      assert(resp.error === "");
    });

    it("reference schema is in schemaContext", async () => {
      const schemaXMLRef = `<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='TrapRef' version='78.00' nameSpacePrefix='trRef' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
          <ECSchemaReference name='BisCore' version='01.00' prefix='bis' />
          <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />
          <ECClass typeName='D' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
            <BaseClass>bis:GraphicalElement3d</BaseClass>
            <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>
              <ECCustomAttributes>
                <StandardValues xmlns='EditorCustomAttributes.01.00'>
                  <ValueMap>
                    <ValueMap>
                      <DisplayString>Sensei</DisplayString>
                      <Value>0</Value>
                    </ValueMap>
                  </ValueMap>
                </StandardValues>
              </ECCustomAttributes>
            </ECProperty>
         </ECClass>
        </ECSchema>`;

      const refSchemaPath = path.join(getOutputDir(), "TrapRef.78.00.00.ecschema.xml");
      fs.writeFileSync(refSchemaPath, schemaXMLRef);

      const schemaContext = new iModelJsNative.ECSchemaXmlContext();
      schemaContext.addSchemaPath(getOutputDir());

      const schemaXML = `<?xml version='1.0' encoding='UTF-8'?>
        <ECSchema schemaName='Trap' version='78.00' nameSpacePrefix='tr' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
          <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />
          <ECSchemaReference name='TrapRef' version='78.00' prefix='trRef' />
          <ECClass typeName='A' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>
            <BaseClass>trRef:D</BaseClass>
            <ECProperty propertyName='TitleA' typeName='int' displayLabel='Title'>
              <ECCustomAttributes>
                <StandardValues xmlns='EditorCustomAttributes.01.00'>
                  <ValueMap>
                    <ValueMap>
                      <DisplayString>Sensei</DisplayString>
                      <Value>0</Value>
                    </ValueMap>
                  </ValueMap>
                </StandardValues>
              </ECCustomAttributes>
            </ECProperty>
          </ECClass>
        </ECSchema>`;

      const ec3Schemas: string[] = iModelJsNative.SchemaUtility.convertEC2XmlSchemas([schemaXML], schemaContext);
      // converted EC3 schemas are always sorted in dependancy order. Last schemas would be Trap
      const ec3SchemaXml = ec3Schemas[ec3Schemas.length - 1];

      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));

      const writeDbFileName = copyFile("SchemaConvertContext.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      const rc = db.importXmlSchemas(ec3Schemas, { schemaLockHeld: true });
      assert.equal(rc, DbResult.BE_SQLITE_OK);
      db.saveChanges();

      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("Trap");

      // Enumeration should not have been created in schema
      const resp = await query(db, `SELECT e.Name, e.* FROM meta.ECSchemaDef s JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations WHERE s.Name='${schema.name}'`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.rowCount === 0);
      assert(resp.error === "");
    });
  });
});
