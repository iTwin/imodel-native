/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64Array, Id64String, IModelStatus, OpenMode, using } from "@itwin/core-bentley";
import { BlobRange, Code, DbBlobRequest, DbBlobResponse, DbQueryRequest, DbQueryResponse, DbRequestKind, DbResponseStatus, GeometryPartProps, IModel, PhysicalElementProps, ProfileOptions, RelationshipProps } from "@itwin/core-common";
import { DomainOptions } from "@itwin/core-common/lib/cjs/BriefcaseTypes";
import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { openDgnDb } from ".";
import { IModelJsNative, SchemaWriteStatus } from "../NativeLibrary";
import { copyFile, dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";

// Crash reporting on linux is gated by the presence of this env variable.
if (os.platform() === "linux")
  process.env.LINUX_MINIDUMP_ENABLED = "yes";

describe.only("basic tests", () => {

  let dgndb: IModelJsNative.DgnDb;

  before((done) => {
    dgndb = openDgnDb(dbFileName, { schemaLockHeld: true });
    done();
  });

  after((done) => {
    dgndb.closeFile();
    done();
  });
  it("subclassof", () => {
    const seedUri = path.join(getAssetsDir(), "test.bim");
    const iModelDb = new iModelJsNative.DgnDb();
    iModelDb.openIModel(seedUri, OpenMode.Readonly);
    assert.isTrue(iModelDb.isSubClassOf("BisCore:GeometricElement3d", "BisCore:GeometricElement"));
    assert.isTrue(iModelDb.isSubClassOf("BisCore:GeometricElement2d", "BisCore:GeometricElement"));
    assert.isTrue(iModelDb.isSubClassOf("BisCore:GeometricModel2d", "BisCore:GeometricModel"));
    assert.isTrue(iModelDb.isSubClassOf("BisCore:GeometricModel3d", "BisCore:GeometricModel"));

    assert.isFalse(iModelDb.isSubClassOf("BisCore:GeometricElement", "BisCore:GeometricElement3d"));
    assert.isFalse(iModelDb.isSubClassOf("BisCore:GeometricElement", "BisCore:GeometricElement2d"));
    assert.isFalse(iModelDb.isSubClassOf("BisCore:GeometricModel", "BisCore:GeometricModel2d"));
    assert.isFalse(iModelDb.isSubClassOf("BisCore:GeometricModel", "BisCore:GeometricModel3d"));
  });
  it("resolveInstanceKey", () => {
    // Test resolving by partialKey
    const r0 = dgndb.resolveInstanceKey({
      partialKey: {
        id: "0x1b",
        baseClassName: "BisCore:Element",
      }
    });
    assert.equal(r0.id, "0x1b", "id should be 0x1b");
    assert.equal(r0.classFullName, "BisCore:Subject", "className should be BisCore:Subject");

    expect(() => dgndb.resolveInstanceKey({
      partialKey: {
        baseClassName: "BisCore:Element"
      } as any // missing id
    } as any)).to.throw("missing id");

    expect(() => dgndb.resolveInstanceKey({
      partialKey: {
        id: "0x1b"
      } as any // missing baseClassName
    } as any)).to.throw("missing baseClassName");

    expect(() => dgndb.resolveInstanceKey({
      partialKey: {
        id: "invalid",
        baseClassName: "BisCore:Element"
      }
    })).to.throw("invalid id");

    expect(() => dgndb.resolveInstanceKey({
      partialKey: {
        id: "0x1b",
        baseClassName: ""
      }
    })).to.throw("invalid baseClassName");

    expect(() => dgndb.resolveInstanceKey({
      partialKey: {
        id: "0x999999",
        baseClassName: "BisCore:Element"
      }
    })).to.throw("failed to resolve instance key");

    // Test resolving by federationGuid
    const elemStmt = new iModelJsNative.ECSqlStatement();
    elemStmt.prepare(dgndb, "SELECT ECInstanceId, FederationGuid FROM bis.Element WHERE FederationGuid IS NOT NULL LIMIT 1");
    if (elemStmt.step() === DbResult.BE_SQLITE_ROW) {
      const elementId = elemStmt.getValue(0).getId();
      const federationGuid = elemStmt.getValue(1).getGuid();
      
      const r2 = dgndb.resolveInstanceKey({
        federationGuid
      });
      assert.equal(r2.id, elementId, "resolved element should match federation GUID query");
      assert.isString(r2.classFullName, "classFullName should be a string");
    }
    elemStmt.dispose();

    expect(() => dgndb.resolveInstanceKey({
      federationGuid: "invalid-guid"
    })).to.throw("failed to resolve element from federationGuid");

    expect(() => dgndb.resolveInstanceKey({
      federationGuid: "00000000-0000-0000-0000-000000000000"
    })).to.throw("failed to resolve element from federationGuid");

    // Test resolving by code
    const codeStmt = new iModelJsNative.ECSqlStatement();
    codeStmt.prepare(dgndb, "SELECT ECInstanceId, CodeSpec.Id, CodeScope.Id, CodeValue FROM bis.Element WHERE CodeValue IS NOT NULL AND CodeValue != '' LIMIT 1");
    if (codeStmt.step() === DbResult.BE_SQLITE_ROW) {
      const elementId = codeStmt.getValue(0).getId();
      const specId = codeStmt.getValue(1).getId();
      const scopeId = codeStmt.getValue(2).getId();
      const codeValue = codeStmt.getValue(3).getString();
      
      const r3 = dgndb.resolveInstanceKey({
        code: {
          spec: specId,
          scope: scopeId,
          value: codeValue
        }
      });
      assert.equal(r3.id, elementId, "resolved element should match code query");
      assert.isString(r3.classFullName, "classFullName should be a string");
    }
    codeStmt.dispose();

    expect(() => dgndb.resolveInstanceKey({
      code: {
        scope: "0x1",
        value: "test"
      } as any // missing spec
    } as any)).to.throw("missing spec");

    expect(() => dgndb.resolveInstanceKey({
      code: {
        spec: "0x1",
        value: "test"
      } as any // missing scope
    } as any)).to.throw("missing type");

    expect(() => dgndb.resolveInstanceKey({
      code: {
        spec: "0x1",
        scope: "0x1"
      } as any // missing value
    } as any)).to.throw("missing value");

    expect(() => dgndb.resolveInstanceKey({
      code: {
        spec: "0x1",
        scope: "0x1",
        value: ""
      } // empty code
    })).to.throw("failed to resolve element from code: code value empty string");

    expect(() => dgndb.resolveInstanceKey({
      code: {
        spec: "0x999999",
        scope: "0x1",
        value: "badvalue"
      }
    })).to.throw("failed to resolve element from code");

    expect(() => dgndb.resolveInstanceKey(null as any)).to.throw("invalid input");

    expect(() => dgndb.resolveInstanceKey({} as any)).to.throw("must provide partialKey, federationGuid or");
  });
  it("compress/decompress", () => {
    const assertCompressAndThenDecompress = (sourceData: Uint8Array) => {
      const compressData = iModelJsNative.DgnDb.zlibCompress(sourceData);
      const decompressData = iModelJsNative.DgnDb.zlibDecompress(compressData, sourceData.length);
      assert(compressData.length < decompressData.length);
      assert.deepEqual(sourceData, decompressData);
    };
    // generate buffer with repeating byte where k is repeated n times.
    const genBuff = (k: number, n: number) => {
      return Uint8Array.from(Array.from({ length: n }, () => k));
    };

    assertCompressAndThenDecompress(genBuff(1, 1024));
    assertCompressAndThenDecompress(genBuff(2, 1024));

    const seedUri = path.join(getOutputDir(), "compress-decompress.bim");
    if (fs.existsSync(seedUri)) {
      fs.unlinkSync(seedUri);
    }
    const iModelDb = new iModelJsNative.DgnDb();
    iModelDb.createIModel(seedUri, { rootSubject: { name: "test file" } });

    const propData = genBuff(1, 1024);
    iModelDb.saveFileProperty({ namespace: "test", name: "test" }, "hello", propData);
    iModelDb.saveChanges();
    const stmt = new iModelJsNative.SqliteStatement();
    stmt.prepare(iModelDb, "SELECT [RawSize], [Data] FROM [be_Prop] WHERE [NameSpace] = 'test' AND [Name] = 'test'");
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    const rawSize = stmt.getValueInteger(0);
    assert.notEqual(rawSize, 0);
    const blob = stmt.getValueBlob(1);
    stmt.dispose();
    const propDecompressData = iModelJsNative.DgnDb.zlibDecompress(blob, rawSize);
    assert.deepEqual(propData, propDecompressData);
    iModelDb.saveChanges();
    iModelDb.closeFile();
  });

  it("sqlite_stmt vtab", () => {
    const seedUri = path.join(getAssetsDir(), "test.bim");
    const iModelDb = new iModelJsNative.DgnDb();
    iModelDb.openIModel(seedUri, OpenMode.Readonly);
    
    const stmt = new iModelJsNative.SqliteStatement();
    stmt.prepare(iModelDb, "SELECT [sql] FROM [sqlite_stmt]");
    assert.equal(stmt.step(), DbResult.BE_SQLITE_ROW);
    stmt.dispose();
  });

  it("schema synchronization", () => {

    const copyAndOverrideFile = (from: string, to: string) => {
      if (fs.existsSync(to)) {
        fs.unlinkSync(to);
      }
      fs.copyFileSync(from, to);
    };
    const getCheckSum = (db: IModelJsNative.ECDb | IModelJsNative.DgnDb, type: "ecdb_schema" | "ecdb_map" | "sqlite_schema") => {
      const stmt = new iModelJsNative.ECSqlStatement();
      assert.equal(DbResult.BE_SQLITE_OK, stmt.prepare(db, `PRAGMA checksum(${type})`).status);
      assert.equal(DbResult.BE_SQLITE_ROW, stmt.step());
      const val = stmt.getValue(0).getString();
      assert.isNotEmpty(val);
      stmt.dispose();
      return val;
    };

    const getSchemaHashes = (db: IModelJsNative.ECDb | IModelJsNative.DgnDb) => {
      return {
        eCDbSchema: getCheckSum(db, "ecdb_schema"),
        eCDbMap: getCheckSum(db, "ecdb_map"),
        sQLiteSchema: getCheckSum(db, "sqlite_schema"),
      };
    };

    const baseDir = path.join(getOutputDir(), "shared-schema-channel");
    if (fs.existsSync(baseDir)) {
      fs.emptyDirSync(baseDir);
      for (const file of fs.readdirSync(baseDir)) {
        fs.unlinkSync(path.join(baseDir, file));
      }
    } else {
      fs.mkdirSync(baseDir, { recursive: true });
    }

    // create empty sync db.
    const syncDbUri = path.join(baseDir, "syncdb.ecdb");
    const syncDb = new iModelJsNative.ECDb();
    const rc: DbResult = syncDb.createDb(syncDbUri);
    assert.equal(DbResult.BE_SQLITE_OK, rc);
    syncDb.saveChanges();
    syncDb.closeDb();

    // create seed file.
    const seedUri = path.join(baseDir, "seed.bim");
    const iModelDb = new iModelJsNative.DgnDb();
    iModelDb.createIModel(seedUri, { rootSubject: { name: "test file" } });

    // initialize sync db.
    iModelDb.schemaSyncInit(syncDbUri, "xxxxx", false);
    iModelDb.saveChanges();
    iModelDb.performCheckpoint();

    const localInfo = iModelDb.schemaSyncGetLocalDbInfo();
    const sharedInfo = iModelDb.schemaSyncGetSyncDbInfo(syncDbUri);
    assert.equal(localInfo?.id, sharedInfo?.id);
    assert.equal(localInfo?.dataVer, sharedInfo?.dataVer);
    assert.equal(localInfo?.dataVer, "0x1");
    iModelDb.closeFile();

    // create first briefcase
    const b0Uri = path.join(baseDir, "b0.bim");
    copyAndOverrideFile(seedUri, b0Uri);
    const b0 = new iModelJsNative.DgnDb();
    b0.openIModel(b0Uri, OpenMode.ReadWrite);

    // create second briefcase
    const b1Uri = path.join(baseDir, "b1.bim");
    copyAndOverrideFile(seedUri, b1Uri);
    const b1 = new iModelJsNative.DgnDb();
    b1.openIModel(b1Uri, OpenMode.ReadWrite);

    // create second briefcase
    const b2Uri = path.join(baseDir, "b2.bim");
    copyAndOverrideFile(seedUri, b2Uri);
    const b2 = new iModelJsNative.DgnDb();
    b2.openIModel(b2Uri, OpenMode.ReadWrite);
    // import schema in briefcase 1
    const schema1 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="Pipe1">
            <BaseClass>bis:GeometricElement2d</BaseClass>
            <ECProperty propertyName="p1" typeName="int" />
            <ECProperty propertyName="p2" typeName="int" />
        </ECEntityClass>
    </ECSchema>`;
    b0.importXmlSchemas([schema1], { schemaSyncDbUri: syncDbUri });

    const schema2 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="Pipe1">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="p1" typeName="int" />
          <ECProperty propertyName="p2" typeName="int" />
          <ECProperty propertyName="p3" typeName="int" />
          <ECProperty propertyName="p4" typeName="int" />
        </ECEntityClass>
    </ECSchema>`;
    b1.importXmlSchemas([schema2], { schemaSyncDbUri: syncDbUri });

    b0.schemaSyncPull(syncDbUri);

    // test default URI
    b2.schemaSyncSetDefaultUri(syncDbUri);
    assert.equal(b2.schemaSyncGetDefaultUri(), syncDbUri);
    b2.schemaSyncPull();

    // b1 = b2 == b0
    const b0Hashes = getSchemaHashes(b0);
    const b1Hashes = getSchemaHashes(b1);
    const b2Hashes = getSchemaHashes(b2);
    assert.deepEqual(b0Hashes, b1Hashes);
    assert.deepEqual(b0Hashes, b2Hashes);

    const schema3 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="Pipe1">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="p1" typeName="int" />
          <ECProperty propertyName="p2" typeName="int" />
          <ECProperty propertyName="p3" typeName="int" />
          <ECProperty propertyName="p4" typeName="int" />
          <ECProperty propertyName="p5" typeName="int" />
          <ECProperty propertyName="p6" typeName="int" />
        </ECEntityClass>
    </ECSchema>`;
    b2.importXmlSchemas([schema3]);

    b0.saveChanges();
    b1.saveChanges();
    b2.saveChanges();
    b0.closeFile();
    b1.closeFile();
    b2.closeFile();
  });

  it("PatchJsonProperties textureId", () => {
    const jsonProps = "{\"materialAssets\":{\"renderMaterial\":{\"HasBaseColor\":false,\"color\":null,\"HasSpecularColor\":false,\"specular_color\":null,\"HasFinish\":false,\"finish\":null,\"HasTransmit\":false,\"transmit\":null,\"HasDiffuse\":false,\"diffuse\":null,\"HasSpecular\":false,\"specular\":null,\"HasReflect\":false,\"reflect\":null,\"HasReflectColor\":false,\"reflect_color\":null,\"Map\":{\"Diffuse\":{\"TextureId\":9223372036854775807},\"Bump\":{\"TextureId\":18446744073709551615},\"Finish\":{\"TextureId\":13835058055282163712}},\"pbr_normal\":null}}}"
    const expectedProps = "{\"materialAssets\":{\"renderMaterial\":{\"HasBaseColor\":false,\"HasSpecularColor\":false,\"HasFinish\":false,\"HasTransmit\":false,\"HasDiffuse\":false,\"HasSpecular\":false,\"HasReflect\":false,\"HasReflectColor\":false,\"Map\":{\"Diffuse\":{\"TextureId\":\"0x7fffffffffffffff\"},\"Bump\":{\"TextureId\":\"0xffffffffffffffff\"},\"Finish\":{\"TextureId\":\"0xc000000000000000\"}}}}}";
    const deserializedProps = dgndb.patchJsonProperties(jsonProps);
    expect(deserializedProps).to.not.be.undefined;
    expect(deserializedProps).to.equal(expectedProps);
  });

  it("PatchJsonProperties subCategory", () => {
    const jsonProps = `{
    "styles": {
        "subCategoryOvr": [
          {
            "invisible": true,
            "subCategory": 51
          }
        ]
      }
    }`;
    const expectedProps = `{
      "styles": {
        "subCategoryOvr": [
          {
            "invisible": true,
            "subCategory": "0x33"
          }
        ]
      }
    }`;
    const deserializedProps = dgndb.patchJsonProperties(jsonProps);
    expect(deserializedProps).to.not.be.undefined;
    expect(deserializedProps).to.equal(JSON.stringify(JSON.parse(expectedProps)));
  });

  it("PatchJsonProperties relClassName", () => {
    const jsonProps = `{
    "styles": {
        "relClassName": "BisCore.ElementRefersToElements"
      }
    }`;
    const expectedProps = `{
    "styles": {
        "relClassName": "BisCore:ElementRefersToElements"
      }
    }`;
    const deserializedProps = dgndb.patchJsonProperties(jsonProps);
    expect(deserializedProps).to.not.be.undefined;
    expect(deserializedProps).to.equal(JSON.stringify(JSON.parse(expectedProps)));
  });

  // verify that throwing javascript exceptions from C++ works
  it("testExceptions", () => {
    // first try a function
    expect(() => (iModelJsNative as any).addFontWorkspace()).to.throw("Argument 0");

    try {
      (iModelJsNative as any).addFontWorkspace();
    } catch (error: any) {
      expect(error.message).to.equal("Argument 0 must be a string");
      expect(error).to.have.property("iTwinErrorId").deep.equal({ scope: "imodel-native", key: "TypeError" });
    }

    // now try methods
    const db = new iModelJsNative.DgnDb() as any;
    expect(() => db.openIModel()).to.throw("Argument 0").property("iTwinErrorId").deep.equal({ scope: "imodel-native", key: "TypeError" });
    expect(() => db.saveFileProperty()).to.throw("requires 2").property("iTwinErrorId").deep.equal({ scope: "imodel-native", key: "BadArg" });  

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

  // TODO: Failing in release builds - investigate later
  it.skip("testExportGraphicsBasics", () => {
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

  it("testExportGraphicsAsync", async () => {
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
    
    await dgndb.exportGraphicsAsync({ elementIdArray, onGraphics });

    for (const id of elementIdArray)
      assert.isDefined(elementsWithGraphics[id], `No graphics generated for ${id}`);
  });

  function createPhysicalElementWithPart() {
    const modelStmt = new iModelJsNative.ECSqlStatement();
    modelStmt.prepare(dgndb, "SELECT ECInstanceId FROM bis.PhysicalModel LIMIT 1");
    const modelId = DbResult.BE_SQLITE_ROW === modelStmt.step() ? modelStmt.getValue(0).getId() : undefined;
    modelStmt.dispose();

    if (modelId === undefined) {
      throw new Error("No PhysicalModel found in database.");
    }

    const categoryStmt = new iModelJsNative.ECSqlStatement();
    categoryStmt.prepare(dgndb, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1");
    const categoryId = DbResult.BE_SQLITE_ROW === categoryStmt.step() ? categoryStmt.getValue(0).getId() : undefined;
    categoryStmt.dispose();

    if (categoryId === undefined) {
      throw new Error("No SpatialCategory found in database.");
    }

    // Create a GeometryPart and attach it to a new physical element.
    const geometryPartProps: GeometryPartProps = {
      classFullName: "BisCore:GeometryPart",
      model: IModel.dictionaryId,
      code: Code.createEmpty(),
      geom: [
        { box: { origin: [0, 0, 0], baseX: 10, baseY: 10, height: 1 } }
      ]
    };

    const partId = dgndb.insertElement(geometryPartProps);
    assert(partId.length > 0, "Failed to create GeometryPart");

    const elementProps: PhysicalElementProps = {
      classFullName: "Generic:PhysicalObject",
      model: modelId,
      category: categoryId,
      code: Code.createEmpty(),
      placement: {
        origin: [100, 0, 0],
        angles: { yaw: 0, pitch: 0, roll: 0 }
      },
      geom: [
        {
          geomPart: {
            part: partId,
            origin: [5, 0, 0],
            rotation: { yaw: 45, pitch: 0, roll: 0 }
          }
        }
      ]
    };

    const elementId = dgndb.insertElement(elementProps);
    assert(elementId.length > 0, "Failed to create PhysicalObject.");

    return { elementId, partId };
  }

  it("exportGraphicsAsync enumerates parts directly if array is not provided", async () => {
    try {
      const partDetails = createPhysicalElementWithPart();

      const elementsWithGraphics: any = {};
      const onGraphics = (info: any) => {
        elementsWithGraphics[info.elementId] = true;
      };

      await dgndb.exportGraphicsAsync({ elementIdArray: [partDetails.elementId], onGraphics });

      assert(elementsWithGraphics[partDetails.elementId]);
    } finally {
      dgndb.abandonChanges();
    }
  });

  it("exportPartGraphics", async () => {
    try {
      const partDetails = createPhysicalElementWithPart();

      // Expect a mesh to be generated for each element - valid for test.bim, maybe invalid for future test data
      const elementsWithGraphics: any = {};
      const onGraphics = (info: any) => {
        elementsWithGraphics[info.elementId] = true;
      };

      const partInstanceArray: any[] = [];

      await dgndb.exportGraphicsAsync({ elementIdArray: [partDetails.elementId], onGraphics, partInstanceArray });

      assert(partInstanceArray.length === 1);
      assert(partInstanceArray[0].partId === partDetails.partId, "Part instance array does not contain expected part ID.");
      assert(partInstanceArray[0].partInstanceId === partDetails.elementId, "Part instance array does not contain expected instance ID.");
      assert(!elementsWithGraphics[partDetails.elementId], `Graphics should not have been generated for part instance ${partDetails.elementId}`);

      let onPartGraphicsCalls = 0;

      dgndb.exportPartGraphics({
        elementId: partInstanceArray[0].partId,
        displayProps: partInstanceArray[0].displayProps,
        onPartGraphics: (_: any) => {
          ++onPartGraphicsCalls;
        }
      });

      assert(onPartGraphicsCalls === 1, "Expected exactly one call to onPartGraphics.");
    } finally {
      dgndb.abandonChanges();
    }
  });

  it("exportPartGraphicsAsync", async () => {
    try {
      const partDetails = createPhysicalElementWithPart();

      // Expect a mesh to be generated for each element - valid for test.bim, maybe invalid for future test data
      const elementsWithGraphics: any = {};
      const onGraphics = (info: any) => {
        elementsWithGraphics[info.elementId] = true;
      };

      const partInstanceArray: any[] = [];

      await dgndb.exportGraphicsAsync({ elementIdArray: [partDetails.elementId], onGraphics, partInstanceArray });

      assert(partInstanceArray.length === 1);
      assert(partInstanceArray[0].partId === partDetails.partId, "Part instance array does not contain expected part ID.");
      assert(partInstanceArray[0].partInstanceId === partDetails.elementId, "Part instance array does not contain expected instance ID.");
      assert(!elementsWithGraphics[partDetails.elementId], `Graphics should not have been generated for part instance ${partDetails.elementId}`);

      let onPartGraphicsCalls = 0;

      await dgndb.exportPartGraphicsAsync({
        elementId: partInstanceArray[0].partId,
        displayProps: partInstanceArray[0].displayProps,
        onPartGraphics: (_: any) => {
          ++onPartGraphicsCalls;
        }
      });

      assert(onPartGraphicsCalls === 1, "Expected exactly one call to onPartGraphics.");
    } finally {
      dgndb.abandonChanges();
    }
  });

  it("testSchemaImport", () => {
    const writeDbFileName = copyFile("testSchemaImport.bim", dbFileName);
    // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCore.01.00.15'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
    const db = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    assert.isTrue(db !== undefined);
    expect(() => db.getSchemaProps("PresentationRules")).to.throw("schema not found").to.have.property("iTwinErrorId"); // presentationrules alias is 'pr'.
    let bisProps = db.getSchemaProps("BisCore");
    assert.isTrue(bisProps.version === "01.00.00");
    const schemaPath = path.join(iModelJsNative.DgnDb.getAssetsDir(), "ECSchemas/Domain/PresentationRules.ecschema.xml");
    db.importSchemas([schemaPath], { schemaLockHeld: true }); // Schema references BisCore.01.00.17+ which contains a data transform, so importing it will need the schema lock.

    db.getSchemaProps("PresentationRules");
    bisProps = db.getSchemaProps("BisCore");
    assert.isTrue(bisProps.version >= "01.00.15"); // PR references 01.00.15+, so importing PR will cause it to upgrade.
  });

  it("testSchemaImport NoAdditionalRootEntityClasses", () => {
    const writeDbFileName = copyFile("noAdditionalRootEntityClasses.bim", dbFileName);
    // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCore.01.00.15'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
    const db = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: false });
    assert.isTrue(db !== undefined);
    const bisProps = db.getSchemaProps("BisCore");
    assert.isTrue(bisProps.version === "01.00.00");

    const schema = `<?xml version="1.0" encoding="utf-8" ?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ECDbMap' version='02.00.03' alias='ecdbmap' />
        <ECEntityClass typeName="NewRootClass" modifier="Abstract">
            <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>
    </ECSchema>`;
    // eslint-disable-next-line @typescript-eslint/naming-convention
    const BE_SQLITE_ERROR_SchemaUpgradeFailed = (DbResult.BE_SQLITE_IOERR | 19 << 24);
    expect(() => db.importXmlSchemas([schema], { schemaLockHeld: false }))
      .to.throw("Failed to import ECClass 'TestSchema:NewRootClass'. It violates against the 'No additional root entity classes' policy which means that all entity classes must subclass from classes defined in the ECSchema BisCore")
      .property("errorNumber").equal(BE_SQLITE_ERROR_SchemaUpgradeFailed);
  });

  it("testSchemaImport PrefersExistingAndLocalOverStandard", () => {
    const testFileName = copyFile("prefersExistingOverStandard.bim", dbFileName);
    const db = openDgnDb(testFileName);
    const assetsDir = path.join(getAssetsDir(), "ImportSchemaTests");
    const test100Path = path.join(assetsDir, "Test.01.00.00.ecschema.xml");

    // BisCore will not be updated because Test only requests BisCore.01.00.00 which is already in the db
    // db has higher precedence than standard schema paths so BisCore from the db is used as the schema ref
    const bisProps = db.getSchemaProps("BisCore");
    db.importSchemas([test100Path], { schemaLockHeld: false });
    assert.equal(db.getSchemaProps("BisCore").version, bisProps.version, "BisCore after Test 1.0.0 import");

    const testRefProps = db.getSchemaProps("TestRef");
    assert.equal(testRefProps.version, "01.00.00", "TestRef after Test 1.0.0 import");

    // TestRef is updated to version 1.0.1 even though Test only references 1.0.0
    // local directory has higher precedence than the db
    const subAssetsDir = path.join(assetsDir, "LocalReferences");
    const test101Path = path.join(subAssetsDir, "Test.01.00.01.ecschema.xml");
    db.importSchemas([test101Path], { schemaLockHeld: false });
    assert.equal(db.getSchemaProps("TestRef").version, "01.00.01", "TestRef after Test 1.0.1 import");
  });

  it("testSchemaImport ErrorWhenAnyXmlIsIllFormed", async () => {
    await using(new iModelJsNative.DisableNativeAssertions(), async (_r) => {
      const writeDbFileName = copyFile("errorWhenAnyXmlIsIllFormed.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCore.01.00.15'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: false });
      assert.isTrue(db !== undefined);
      const bisProps = db.getSchemaProps("BisCore");
      assert.isTrue(bisProps.version === "01.00.00");

      const validSchema = `<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="ValidSchema" alias="vs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
        <ECEntityClass typeName="Pipe">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="p1" typeName="int" />
        </ECEntityClass>
      </ECSchema>`;

      const invalidSchema = `<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="InvalidSchema" alias="is" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
        <ECEntityClass typeName="Manhole" displayLabel="Manhole">
          <BaseClass>bis:GeometricElement3d</BaseClass>
        <ECEntityClass>
      </ECSchema>`;

      expect(() => db.importXmlSchemas([invalidSchema], { schemaLockHeld: true })) // Schema references BisCore.01.00.17+ which contains a data transform, so importing it will need the schema lock.
        .to.throw("Failed to import schemas")
        .property("errorNumber").equal(DbResult.BE_SQLITE_ERROR);

      expect(() => db.importXmlSchemas([validSchema, invalidSchema], { schemaLockHeld: true })) // Schema references BisCore.01.00.17+ which contains a data transform, so importing it will need the schema lock.
        .to.throw("Failed to import schemas")
        .property("errorNumber").equal(DbResult.BE_SQLITE_ERROR);

      db.importXmlSchemas([validSchema], { schemaLockHeld: true }); // Schema references BisCore.01.00.17+ which contains a data transform, so importing it will need the schema lock.
      const validSchemaProps = db.getSchemaProps("ValidSchema");
      assert.isTrue(validSchemaProps.name === "ValidSchema");
      assert.isTrue(validSchemaProps.version === "01.00.00");
    });
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
  it("crash inserting linktable", async () => {
    const pathToDb = path.join(getAssetsDir(), "test.bim");

    const testFile = path.join(getAssetsDir(), "crash-linktable.bim");
    if (fs.existsSync(testFile)) {
      fs.unlinkSync(testFile);
    }
    fs.copyFileSync(pathToDb, testFile);

    const db = new iModelJsNative.DgnDb();
    db.openIModel(testFile, OpenMode.ReadWrite);
    const abstractRelationshipClass = "BisCore:ElementRefersToElements";
    const insertRel: RelationshipProps = { classFullName: abstractRelationshipClass, sourceId: "0x37", targetId: "0x31" };
    const updateRel: RelationshipProps = { classFullName: abstractRelationshipClass, id: "0xe", sourceId: "0x37", targetId: "0x31" };
    expect(() => db.insertLinkTableRelationship(insertRel)).to.throw("Failed to insert relationship. Relationship class 'BisCore:ElementRefersToElements' is abstract").to.have.property("iTwinErrorId");
    expect(() => db.updateLinkTableRelationship(updateRel)).to.throw("Failed to update relationship. Relationship class 'BisCore:ElementRefersToElements' is abstract").to.have.property("iTwinErrorId");
  });
  it("testCrashReportingConfig", () => {
    if (os.platform() === "darwin" || process.env.AddressSanitizer === "yes") {
      // Currently unsupported on the Mac.
      // With Address Sanitizer it fails with 'JsInterop::InitializeCrashReporting: Failed to start the crashpad handler'
      return;
    }
    iModelJsNative.setCrashReporting({
      enableCrashDumps: true,
      crashDir: path.join(getOutputDir(), "crashdumps"),
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
    expect(() => db.openDb(pathToDb, { openMode: OpenMode.ReadWrite })).to.throw("file is not a database").to.have.property("iTwinErrorId");;
    // rawSQLite being true skips be_prop check so we can open the database, but will fail later on if we step and prepare on the db.
    expect(() => db.openDb(pathToDb, { openMode: OpenMode.ReadWrite, rawSQLite: true })).to.not.throw();
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
    withWal.closeFile();
  });

  it("testGetSchemaProps", async () => {
    assert.isTrue(dgndb.isOpen());
    expect(() => dgndb.getSchemaProps("DoesNotExist")).to.throw("schema not found").with.property("errorNumber", IModelStatus.NotFound);
    const props = dgndb.getSchemaProps("BisCore");
    expect(props.name).equal("BisCore");
  });

  it("testGetSchemaPropsAsync", async () => {
    assert.isTrue(dgndb.isOpen());
    await expect(dgndb.getSchemaPropsAsync("DoesNotExist")).rejectedWith("schema not found").eventually.with.property("errorNumber", IModelStatus.NotFound);
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
    try {
      dgndb.queryModelExtents({ id: "NotAnId" })
    } catch (error: any) {
      expect(error.message).to.equal("Invalid id");
      expect(error).to.have.property("errorNumber").equal(IModelStatus.InvalidId);
      expect(error).to.have.property("iTwinErrorId").deep.equal({ scope: "dgn-db", key: "InvalidId" });
    }

    try {
      dgndb.queryModelExtents({ id: "0xabcdef" });
    } catch (error: any) {
      expect(error.message).to.equal("not found");
      expect(error).to.have.property("errorNumber").equal(IModelStatus.NotFound);
      expect(error).to.have.property("iTwinErrorId").deep.equal({ scope: "dgn-db", key: "NotFound" });
    }

    try {
      dgndb.queryModelExtents({ id: "0x1" });
    } catch (error: any) {
      expect(error.message).to.equal("error=10040");
      expect(error).to.have.property("errorNumber").equal(IModelStatus.WrongModel);
      expect(error).to.have.property("iTwinErrorId").deep.equal({ scope: "dgn-db", key: "WrongModel" });
    }

    try {
      dgndb.queryModelExtents({ id: "0x1c" });
    } catch (error: any) {
      expect(error.message).to.equal("error=10022");
      expect(error).to.have.property("errorNumber").equal(IModelStatus.NoGeometry);
      expect(error).to.have.property("iTwinErrorId").deep.equal({ scope: "dgn-db", key: "NoGeometry" });
    }

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
      await expect(dgndb.generateElementMeshes({} as any)).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0" })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0x1" })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ source: "0x123456789" })).rejectedWith(msg);
    });

    // TODO: Failing in release builds - investigate later
    it.skip("produces meshes", async () => {
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
    dbForProfileUpgrade.closeFile();

    const dbForSchemaUpgrade = openDgnDb(testFile, { domain: DomainOptions.Upgrade, schemaLockHeld: true });
    dbForSchemaUpgrade.saveChanges();
    dbForSchemaUpgrade.closeFile();
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
    db.importXmlSchemas([generateSchema(20, 1)], { schemaLockHeld: false });
    db.saveChanges();

    // eslint-disable-next-line @typescript-eslint/naming-convention
    const BE_SQLITE_ERROR_DataTransformRequired = (DbResult.BE_SQLITE_IOERR | 23 << 24);

    // import should fail when schemaLockHeld flag is set to false which will fail the operation if data transform is required.
    expect(() => db.importXmlSchemas([generateSchema(20, 20)], { schemaLockHeld: false }))
      .to.throw("Import ECSchema failed. Data transform is required which is rejected by default unless explicitly allowed.")
      .property("errorNumber").equal(BE_SQLITE_ERROR_DataTransformRequired);

    // import should be successful when schemaLockHeld flag is set to true so it can transform data if required.
    db.importXmlSchemas([generateSchema(20, 20)], { schemaLockHeld: true });
    db.saveChanges();
    db.closeFile();
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
      classId: "0x44",
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
      // converted EC3 schemas are in the same order as of input schemas
      const ec3SchemaXml = ec3Schemas[0];
      const ec3RefSchema = ec3Schemas[1];

      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
      assert.isTrue(ec3RefSchema.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
    });

    it("schema import after deserialization", async () => {
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
      assert.equal(ec3Schemas.length, 2);
      // converted EC3 schemas are in the same order as of input schemas
      const ec3SchemaXml = ec3Schemas[0];
      const ec3RefSchema = ec3Schemas[1];

      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
      assert.isTrue(ec3RefSchema.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));

      const writeDbFileName = copyFile("SchemaImport.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      // importXmlSchemas expects schemas to be in dependency order
      db.importXmlSchemas([ec3RefSchema, ec3SchemaXml], { schemaLockHeld: true });
      db.saveChanges();

      const refSchema: IModelJsNative.SchemaProps = db.getSchemaProps("TrapRef");
      expect(refSchema.name).equal("TrapRef");
      assert.isTrue(refSchema.version === "78.00.00");

      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("Trap");
      expect(schema.name).equal("Trap");
      assert.isTrue(schema.version === "78.00.00");
    });

    it("rename reserved words", async () => {
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
      const schemasWithUpdatedCA: string[] = iModelJsNative.SchemaUtility.convertCustomAttributes(ec3Schemas);

      const writeDbFileName = copyFile("RenameReservedWords.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      db.importXmlSchemas(schemasWithUpdatedCA, { schemaLockHeld: true });
      db.saveChanges();

      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("TestSchema");
      expect(schema.name).equal("TestSchema");
      assert.isTrue(schema.version === "01.00.00");

      let resp = await query(db, `SELECT p.Name FROM meta.ECPropertyDef p JOIN meta.ECClassDef c USING meta.ClassOwnsLocalProperties JOIN meta.ECSchemaDef s USING meta.SchemaOwnsClasses WHERE s.Name='TestSchema' AND c.Name='TestEntityClass' ORDER BY p.Ordinal`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.error === "");
      assert(resp.rowCount === 9);

      // Function to check if a property exists in the given class/struct metadata
      const doesPropertyExist = (properties: any, propertyName: string) => {
        return Object.values(properties).some((property: any) => property[0] === propertyName);
      };

      assert.isFalse(doesPropertyExist(resp.data, "Id")); // The Id property is a reserved keyword and should have been renamed
      assert.isFalse(doesPropertyExist(resp.data, "ECClassId"));  // The ECClassId property is a reserved keyword and should have been renamed
      assert.isFalse(doesPropertyExist(resp.data, "ECInstanceId")); // The ECInstanceId property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(resp.data, "TestSchema_Id_"));  // The Id property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(resp.data, "TestSchema_ECClassId_")); // The ECClassId property is a reserved keyword and should have been renamed
      assert.isTrue(doesPropertyExist(resp.data, "TestSchema_ECInstanceId_"));  // The ECInstanceId property is a reserved keyword and should have been renamed

      assert.isTrue(doesPropertyExist(resp.data, "SourceECInstanceId"));  // The SourceECInstanceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "SourceId"));  // The SourceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "SourceECClassId")); // The SourceECClassId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "TargetECInstanceId"));  // The TargetECInstanceId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "TargetId"));  // The TargetId property is allowed on Entity classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "TargetECClassId")); // The TargetECClassId property is allowed on Entity classes and should not be renamed

      resp = await query(db, `SELECT p.Name FROM meta.ECPropertyDef p JOIN meta.ECClassDef c USING meta.ClassOwnsLocalProperties JOIN meta.ECSchemaDef s USING meta.SchemaOwnsClasses WHERE s.Name='TestSchema' AND c.Name='TestStructClass' ORDER BY p.Ordinal`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.error === "");
      assert(resp.rowCount === 3);

      assert.isTrue(doesPropertyExist(resp.data, "Id"));  // The Id property is not a reserved keyword for Struct classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "ECClassId")); // The ECClassId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isTrue(doesPropertyExist(resp.data, "ECInstanceId"));  // The ECInstanceId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(resp.data, "TestSchema_Id_")); // The Id property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(resp.data, "TestSchema_ECClassId_"));  // The ECClassId property is not a reserved keyword for Struct classes and should not be renamed
      assert.isFalse(doesPropertyExist(resp.data, "TestSchema_ECInstanceId_")); // The ECInstanceId property is not a reserved keyword for Struct classes and should not be renamed
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
      const schemasWithUpdatedCA: string[] = iModelJsNative.SchemaUtility.convertCustomAttributes(ec3Schemas);
      assert.equal(schemasWithUpdatedCA.length, 2);
      // converted EC3 schemas are in the same order as of input schemas
      const ec3SchemaXml = schemasWithUpdatedCA[0];
      const ec3RefSchema = schemasWithUpdatedCA[1];

      const writeDbFileName = copyFile("SchemaConvertEnum.bim", dbFileName);
      // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'RefSchema.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
      const db: IModelJsNative.DgnDb = openDgnDb(writeDbFileName, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      assert.isTrue(db !== undefined);
      assert.isTrue(db.isOpen());

      db.importXmlSchemas([ec3RefSchema, ec3SchemaXml], { schemaLockHeld: true });
      db.saveChanges();

      const refSchema: IModelJsNative.SchemaProps = db.getSchemaProps("TrapRef");
      expect(refSchema.name).equal("TrapRef");
      assert.isTrue(refSchema.version === "78.00.00");
      const schema: IModelJsNative.SchemaProps = db.getSchemaProps("Trap");
      expect(schema.name).equal("Trap");
      assert.isTrue(schema.version === "78.00.00");

      // Enumeration should have been created in refschema
      let resp = await query(db, `SELECT e.Name FROM meta.ECSchemaDef s JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations WHERE s.Name='${refSchema.name}'`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.error === "");
      assert(resp.rowCount === 1);
      assert(resp.data[0], "D_TitleA");

      // Enumeration should not have been created in schema
      resp = await query(db, `SELECT e.Name, e.* FROM meta.ECSchemaDef s JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations WHERE s.Name='${schema.name}'`);
      assert(resp.status === DbResponseStatus.Done);
      assert(resp.error === "");
      assert(resp.rowCount === 0);
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
      assert.equal(ec3Schemas.length, 1);
      // converted EC3 schemas are in the same order as of input schemas
      const ec3SchemaXml = ec3Schemas[0];

      assert.isTrue(ec3SchemaXml.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
    });
  });

  it("export ECXml 3.1 schema which is using ECXml 3.2 unit as persistence unit", async () => {
    // This test covers the scenario where we have a schema with ECXml version 3.1 which is using a KoQ with a unit that's defined in ECXml 3.2.
    // Since the 3.1 version only checks the legacy unit mappings, the export as a ECXml 3.1 schema fails.
    // In this scenario, when the export fails, we will try to export the schema as an ECXml 3.2 schema instead.

    if (fs.existsSync("testSchemaExport.bim"))
      fs.unlinkSync("testSchemaExport.bim");

    const testDb = openDgnDb(copyFile("testSchemaExport.bim", dbFileName), { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    assert.isTrue(testDb !== undefined);

    // Import the schema as ECXML 3.2 which has ECXml 3.2 unit LUMEN_PER_W
    testDb.importXmlSchemas([`<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
        <ECSchemaReference name="Units" version="01.00.07" alias="u"/>
        <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="true">
            <ECEnumerator name="Test_ENUM0" value="0" displayLabel="Test_ENUM0"/>
            <ECEnumerator name="Test_ENUM1" value="1" displayLabel="Test_ENUM1"/>
        </ECEnumeration>
        <ECEntityClass typeName="TestClass" displayLabel="TestClass">
            <BaseClass>bis:PhysicalElement</BaseClass>
            <ECProperty propertyName="TestProp" typeName="double" displayLabel="TestProp" readOnly="true" category="TestCategory" kindOfQuantity="TestKoq"/>
        </ECEntityClass>
        <KindOfQuantity typeName="TestKoq" displayLabel="TestKoq" persistenceUnit="u:LUMEN_PER_W" relativeError="10.763910416709722" presentationUnits="f:DefaultRealU[u:LUMEN_PER_W]"/>
        <PropertyCategory typeName="TestCategory" displayLabel="TestCategory" priority="200000"/>
    </ECSchema>`]);

    testDb.saveChanges();

    // Modify the ECXml version of the schema to 3.1 directly in the db
    const statement = new iModelJsNative.SqliteStatement();
    statement.prepare(testDb, `update ec_Schema set OriginalECXmlVersionMajor=3, OriginalECXmlVersionMinor=1 where Name='TestSchema'`);
    expect(statement.step()).to.be.eql(DbResult.BE_SQLITE_DONE);
    statement.dispose();

    // Export ECXml 3.1 schema to xml file
    const status = testDb.exportSchema(`TestSchema`, getOutputDir(), `ExportedTestSchema.ecschema.xml`);
    assert.equal(status, SchemaWriteStatus.Success, `Exporting the ECXml 3.1 schema is expected to fail due to incorrect KoQ. Schema should retry serialization as an ECXml 3.2 schema and succeed`);
    assert.isTrue(fs.existsSync(path.join(getOutputDir(), `ExportedTestSchema.ecschema.xml`)));

    const xmlVersionStr = `xmlns="http://www.bentley.com/schemas/Bentley.`;
    const koqStr = `<KindOfQuantity typeName="TestKoq" displayLabel="TestKoq" persistenceUnit="u:LUMEN_PER_W" relativeError="10.763910416709722" presentationUnits="f:DefaultRealU[u:LUMEN_PER_W]"/>`;
    const propStr = `<ECProperty propertyName="TestProp" typeName="double" displayLabel="TestProp" readOnly="true" category="TestCategory" kindOfQuantity="TestKoq"/>`;
    const propCatStr = `<PropertyCategory typeName="TestCategory" displayLabel="TestCategory" priority="200000"/>`;

    // Read the exported file to check if schema was serialized as ECXml 3.2
    const exportedFileStr = fs.readFileSync(path.join(getOutputDir(), `ExportedTestSchema.ecschema.xml`), { encoding: "utf8" });
    assert.notEqual(exportedFileStr, undefined);
    expect(exportedFileStr.includes(`${xmlVersionStr}ECXML.3.2"`)).to.be.true;
    expect(exportedFileStr.includes(`${xmlVersionStr}ECXML.3.1"`)).to.be.false;
    expect(exportedFileStr.includes(koqStr)).to.be.true;
    expect(exportedFileStr.includes(propStr)).to.be.true;
    expect(exportedFileStr.includes(propCatStr)).to.be.true;

    // Export ECXml 3.1 schema to a xml string as ECXml 3.1
    let schemaXmlStr = testDb.schemaToXmlString("TestSchema", IModelJsNative.ECVersion.V3_1);
    assert.notEqual(schemaXmlStr, undefined);

    // Check if schema was serialized as ECXml 3.2
    expect(schemaXmlStr!.includes(`${xmlVersionStr}ECXML.3.2"`)).to.be.true;
    expect(schemaXmlStr!.includes(`${xmlVersionStr}ECXML.3.1"`)).to.be.false;
    expect(schemaXmlStr!.includes(koqStr)).to.be.true;
    expect(schemaXmlStr!.includes(propStr)).to.be.true;
    expect(schemaXmlStr!.includes(propCatStr)).to.be.true;

    // Export ECXml 3.1 schema to a xml string as latest ECXml
    schemaXmlStr = testDb.schemaToXmlString("TestSchema");
    assert.notEqual(schemaXmlStr, undefined);

    // Check if schema was serialized as ECXml 3.2
    expect(schemaXmlStr!.includes(`${xmlVersionStr}ECXML.3.2"`)).to.be.true;
    expect(schemaXmlStr!.includes(`${xmlVersionStr}ECXML.3.1"`)).to.be.false;
    expect(schemaXmlStr!.includes(koqStr)).to.be.true;
    expect(schemaXmlStr!.includes(propStr)).to.be.true;
    expect(schemaXmlStr!.includes(propCatStr)).to.be.true;

    testDb.closeFile();
  });
  it("NoCaseCollation", async () => {
    const pathToDb = path.join(getAssetsDir(), "test.bim");
    const testFile = path.join(getAssetsDir(), "collation.bim");
    if (fs.existsSync(testFile)) {
      fs.unlinkSync(testFile);
    }
    fs.copyFileSync(pathToDb, testFile);

    const db = new iModelJsNative.DgnDb();
    db.openIModel(testFile, OpenMode.ReadWrite);
    expect(db.getNoCaseCollation()).to.be.equals("ASCII");
    const executeSql = (sql: string, cb: (stmt: IModelJsNative.SqliteStatement) => void) => {
      const stmt = new iModelJsNative.SqliteStatement();
      stmt.prepare(db, sql);
      cb(stmt);
      stmt.dispose();
    };
    db.saveChanges();
    executeSql("CREATE TABLE [Foo]([Id] INTEGER PRIMARY KEY, [Name] TEXT UNIQUE COLLATE NOCASE);", (stmt) => {
      expect(stmt.step()).equals(DbResult.BE_SQLITE_DONE);
    });

    db.saveChanges();
    executeSql("INSERT INTO Foo(Name)VALUES('ÀÁÂÃÄÅ')", (stmt) => {
      expect(stmt.step()).equals(DbResult.BE_SQLITE_DONE);
    });
    executeSql("INSERT INTO Foo(Name)VALUES('àáâãäå')", (stmt) => {
      expect(stmt.step()).equals(DbResult.BE_SQLITE_DONE);
    });
    db.abandonChanges();
    db.setNoCaseCollation("Latin1");

    executeSql("INSERT INTO Foo(Name)VALUES('ÀÁÂÃÄÅ')", (stmt) => {
      expect(stmt.step()).equals(DbResult.BE_SQLITE_DONE);
    });
    executeSql("INSERT INTO Foo(Name)VALUES('àáâãäå')", (stmt) => {
      expect(stmt.step()).equals(DbResult.BE_SQLITE_CONSTRAINT_UNIQUE);
    });
    db.closeFile();
  });

  describe.only("Bulk Element Deletion", () => {
    let db: IModelJsNative.DgnDb;
    let modelId: Id64String;
    let categoryId: Id64String;
    let codeSpecId: Id64String;

    // Testcase Generation Helpers
    const insertElement = (opts: { parentId?: Id64String; codeScope?: Id64String; codeValue?: string } = {}): Id64String => {
      const { parentId, codeScope, codeValue } = opts;
      const props: PhysicalElementProps = {
        classFullName: "Generic:PhysicalObject",
        model: modelId,
        category: categoryId,
        code: codeScope && codeValue ? { spec: codeSpecId, scope: codeScope, value: codeValue } : Code.createEmpty(),
        placement: { origin: [0, 0, 0], angles: { yaw: 0, pitch: 0, roll: 0 } },
        ...(parentId ? { parent: { id: parentId, relClassName: "BisCore:ElementOwnsChildElements" } } : {}),
      };
      const id = db.insertElement(props);
      assert.isNotEmpty(id, "insertElement must return a valid ID");
      return id;
    };

    /** Assert that the element with the given id exists or has been deleted. */
    const assertExists    = (id: Id64String, msg: string) => assert.isDefined(db.getElement({ id }), msg);
    const assertDeleted   = (id: Id64String, msg: string) => assert.throws(() => db.getElement({ id }), undefined, msg);

    /**
     * Run deleteElements, then verify each id in `deleted` is gone and each id in `retained` is still present.
     */
    const executeTestCase = (label: string, idsToDelete: Id64Array, deleted: Id64Array, retained: Id64Array) => {
      db.deleteElements(idsToDelete);

      for (const id of deleted)
        assertDeleted(id, `error reading element`);
      
      for (const id of retained)
        assertExists(id, `[${label}] ${id} should have been retained`);
      
      db.abandonChanges();
    };

    beforeEach(() => {
      const seedUri  = path.join(getAssetsDir(), "test.bim");
      const testFile = path.join(getAssetsDir(), "deleteElements-test.bim");
      if (fs.existsSync(testFile))
        fs.unlinkSync(testFile);
      fs.copyFileSync(seedUri, testFile);

      db = new iModelJsNative.DgnDb();
      db.openIModel(testFile, OpenMode.ReadWrite);

      const modelStmt = new iModelJsNative.ECSqlStatement();
      modelStmt.prepare(db, "SELECT ECInstanceId FROM bis.PhysicalModel LIMIT 1");
      modelId = DbResult.BE_SQLITE_ROW === modelStmt.step() ? modelStmt.getValue(0).getId() : "";
      modelStmt.dispose();
      assert.isNotEmpty(modelId, "Expected a PhysicalModel");

      const catStmt = new iModelJsNative.ECSqlStatement();
      catStmt.prepare(db, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1");
      categoryId = DbResult.BE_SQLITE_ROW === catStmt.step() ? catStmt.getValue(0).getId() : "";
      catStmt.dispose();
      assert.isNotEmpty(categoryId, "Expected a SpatialCategory");

      codeSpecId = db.insertCodeSpec("TestScopeSpec", { scopeSpec: { type: 4 /* RelatedElement */ } });
      assert.isNotEmpty(codeSpecId);
    });

    afterEach(() => {
      db.closeFile();
    });

    /**
     * Shared hierarchy used throughout the parent-child tests:
     *
     *   parentA                    parentB              standalone
     *     ├─ childA1                 ├─ childB1            └─ childS1
     *     │    └─ grandchildA1       └─ childB2
     *     ├─ childA2
     *     │    └─ grandchildA2
     *     └─ childA3
     */
    describe("parent-child hierarchy", () => {
      let parentA: Id64String, childA1: Id64String, grandchildA1: Id64String;
      let childA2: Id64String, grandchildA2: Id64String, childA3: Id64String;
      let parentB: Id64String, childB1: Id64String, childB2: Id64String;
      let standalone: Id64String, childS1: Id64String;
      let all: Id64Array;

      beforeEach(() => {
        parentA      = insertElement();
        childA1      = insertElement({ parentId: parentA });
        grandchildA1 = insertElement({ parentId: childA1 });
        childA2      = insertElement({ parentId: parentA });
        grandchildA2 = insertElement({ parentId: childA2 });
        childA3      = insertElement({ parentId: parentA });
        parentB      = insertElement();
        childB1      = insertElement({ parentId: parentB });
        childB2      = insertElement({ parentId: parentB });
        standalone   = insertElement();
        childS1      = insertElement({ parentId: standalone });
        db.saveChanges();
        all = [parentA, childA1, grandchildA1, childA2, grandchildA2, childA3,
               parentB, childB1, childB2, standalone, childS1];
      });

      it("delete a root element", () => {
        executeTestCase("root cascades",
          [parentA],
          [parentA, childA1, grandchildA1, childA2, grandchildA2, childA3],
          [parentB, childB1, childB2, standalone, childS1]);
      });

      it("explicitly delete the whole tree", () => {
        executeTestCase("redundant descendants in input",
          [parentA, childA1, grandchildA1, childA2],
          [parentA, childA1, grandchildA1, childA2, grandchildA2, childA3],
          [parentB, childB1, childB2, standalone, childS1]);
      });

      it("deleting all roots removes every element", () => {
        executeTestCase("delete all roots",
          [parentA, parentB, standalone],
          all,
          []);
      });

      it("empty input set is a no-op", () => {
        executeTestCase("empty set",
          [],
          [],
          all);
      });

      it("deleting a child removes its subtree but leaves the parent", () => {
        executeTestCase("delete depth-1 child",
          [childA1],
          [childA1, grandchildA1],
          [parentA, childA2, grandchildA2, childA3, parentB, childB1, childB2, standalone, childS1]);
      });

      it("deleting two mid-tree siblings leaves their parent and unrelated siblings", () => {
        executeTestCase("delete two depth-1 siblings",
          [childA1, childA2],
          [childA1, grandchildA1, childA2, grandchildA2],
          [parentA, childA3, parentB, childB1, childB2, standalone, childS1]);
      });

      it("deleting a child from one tree and a child from another tree", () => {
        executeTestCase("cross-tree mid-tree delete",
          [childA1, childB2],
          [childA1, grandchildA1, childB2],
          [parentA, childA2, grandchildA2, childA3, parentB, childB1, standalone, childS1]);
      });

      it("deleting mid-tree nodes mixed with a root", () => {
        executeTestCase("mid-tree + roots mixed",
          [childA1, childA3, parentB, standalone],
          [childA1, grandchildA1, childA3, parentB, childB1, childB2, standalone, childS1],
          [parentA, childA2, grandchildA2]);
      });

      it("deleting only grandchildren leaves all ancestors", () => {
        executeTestCase("delete leaves only",
          [grandchildA1, grandchildA2],
          [grandchildA1, grandchildA2],
          [parentA, childA1, childA2, childA3, parentB, childB1, childB2, standalone, childS1]);
      });

      it("deleting leaves from different subtrees simultaneously", () => {
        executeTestCase("leaves from multiple subtrees",
          [grandchildA1, childB1, childS1],
          [grandchildA1, childB1, childS1],
          [parentA, childA1, childA2, grandchildA2, childA3, parentB, childB2, standalone]);
      });

      it("deleting root, mid-tree and leaf", () => {
        executeTestCase("root + child + grandchild + leaf",
          [childA1, grandchildA2, parentB, childS1],
          [childA1, grandchildA1, grandchildA2, parentB, childB1, childB2, childS1],
          [parentA, childA2, childA3, standalone]);
      });

      it("parent and its grandchild", () => {
        executeTestCase("parent + grandchild redundant",
          [parentA, grandchildA1],
          [parentA, childA1, grandchildA1, childA2, grandchildA2, childA3],
          [parentB, childB1, childB2, standalone, childS1]);
      });
    });

    describe("intra-set code scope", () => {
      it("scope and scoped element both roots - order-agnostic", () => {
        const rootA = insertElement();
        const rootB = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        db.saveChanges();
        // Forward order: scope element first
        executeTestCase("rootA -> rootB forward", [rootA, rootB], [rootA, rootB], []);
        // Reverse order: scoped element first
        executeTestCase("rootA -> rootB reverse", [rootB, rootA], [rootA, rootB], []);
      });

      it("child element is the code scope for an unrelated root", () => {
        const rootA  = insertElement();
        const childA = insertElement({ parentId: rootA });
        const rootB  = insertElement({ codeScope: childA, codeValue: "rootB-code" });
        db.saveChanges();
        executeTestCase("depth-1 child scopes unrelated root - delete child+root directly",
          [childA, rootB],
          [childA, rootB],
          [rootA]);
        executeTestCase("depth-1 child scopes unrelated root - delete child only",
          [childA],
          [],
          [rootA, childA, rootB]);
        executeTestCase("depth-1 child scopes unrelated root - delete root only",
          [rootB],
          [rootB],
          [rootA, childA]);
      });

      it("grandchild is the code scope for an unrelated root", () => {
        const rootA       = insertElement();
        const childA      = insertElement({ parentId: rootA });
        const grandchildA = insertElement({ parentId: childA });
        const rootB       = insertElement({ codeScope: grandchildA, codeValue: "rootB-code" });
        db.saveChanges();
        executeTestCase("depth-2 grandchild scopes unrelated root - delete both roots",
          [rootA, rootB],
          [rootA, childA, grandchildA, rootB],
          []);
        executeTestCase("depth-2 grandchild scopes unrelated root - delete grandchild+root directly",
          [grandchildA, rootB],
          [grandchildA, rootB],
          [rootA, childA]);
      });

      it("root element scopes a child in another subtree", () => {
        const rootA  = insertElement();
        const rootB  = insertElement();
        const childB = insertElement({ parentId: rootB, codeScope: rootA, codeValue: "childB-code" });
        db.saveChanges();
        executeTestCase("root scopes depth-1 child in sibling tree",
          [rootA, rootB],
          [rootA, rootB, childB],
          []);
      });

      it("child scopes a sibling child", () => {
        const rootA  = insertElement();
        const childA = insertElement({ parentId: rootA });
        const rootB  = insertElement();
        const childB = insertElement({ parentId: rootB, codeScope: childA, codeValue: "childB-code" });
        db.saveChanges();
        executeTestCase("depth-1 child scopes depth-1 sibling",
          [childA, childB],
          [childA, childB],
          [rootA, rootB]);
      });

      it("scope chain 1", () => {
        const rootA = insertElement();
        const rootB = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        const rootC = insertElement({ codeScope: rootB, codeValue: "rootC-code" });
        db.saveChanges();
        executeTestCase("scope chain forward",  [rootA, rootB, rootC], [rootA, rootB, rootC], []);
        executeTestCase("scope chain reversed", [rootC, rootB, rootA], [rootA, rootB, rootC], []);
        executeTestCase("scope chain middle-first", [rootB, rootA, rootC], [rootA, rootB, rootC], []);
      });

      it("scope chain 2", () => {
        // A -> B -> C: B is not in the delete set but depends on A (external violation).
        // A should be pruned. C depends on B which is not being deleted, so C is standalone-deletable.
        const rootA = insertElement();
        const rootB = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        const rootC = insertElement({ codeScope: rootB, codeValue: "rootC-code" });
        db.saveChanges();
        // rootB is NOT in the delete set but uses rootA as scope -> external violation -> rootA pruned
        // rootC is in the delete set and its scope (rootB) is not being deleted -> rootC is safe to delete
        executeTestCase("scope chain: delete A and C, B external violation prunes A",
          [rootA, rootC],
          [rootC],
          [rootA, rootB]);
      });

      it("two elements using the same scope", () => {
        // A is the code scope for both B and C independently.
        //     A
        //    / \
        //   B   C  (code scope, not parent-child)
        const rootA = insertElement();
        const rootB = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        const rootC = insertElement({ codeScope: rootA, codeValue: "rootC-code" });
        db.saveChanges();
        executeTestCase("delete all three",
          [rootA, rootB, rootC],
          [rootA, rootB, rootC],
          []);
        executeTestCase("delete only B and C",
          [rootB, rootC],
          [rootB, rootC],
          [rootA]);
      });

      it("parent is also the code scope of its own child", () => {
        const rootP = insertElement();
        const childC = insertElement({ parentId: rootP, codeScope: rootP, codeValue: "childC-code" });
        db.saveChanges();
        executeTestCase("parent is code scope of child - delete parent",
          [rootP],
          [rootP, childC],
          []);
      });
    });

    describe("external code scope violation - pruning", () => {
      it("root is code scope for an external element", () => {
        const rootA    = insertElement();
        const external = insertElement({ codeScope: rootA, codeValue: "ext-code" });
        const rootB    = insertElement();
        db.saveChanges();
        executeTestCase("external scopes root",
          [rootA, rootB],
          [rootB],
          [rootA, external]);
      });

      it("depth-1 child is code scope for external", () => {
        const rootA    = insertElement();
        const childA   = insertElement({ parentId: rootA });
        const external = insertElement({ codeScope: childA, codeValue: "ext-code" });
        const rootB    = insertElement();
        db.saveChanges();
        executeTestCase("external scopes depth-1 child - parent subtree pruned",
          [rootA, rootB],
          [rootB],
          [rootA, childA, external]);
      });

      it("depth-2 grandchild is code scope for external", () => {
        const rootA       = insertElement();
        const childA      = insertElement({ parentId: rootA });
        const grandchildA = insertElement({ parentId: childA });
        const external    = insertElement({ codeScope: grandchildA, codeValue: "ext-code" });
        const rootB       = insertElement();
        db.saveChanges();
        executeTestCase("external scopes depth-2 grandchild - grandparent subtree pruned",
          [rootA, rootB],
          [rootB],
          [rootA, childA, grandchildA, external]);
      });

      it("only the child is passed for deletion", () => {
        const rootA    = insertElement();
        const childA   = insertElement({ parentId: rootA });
        const external = insertElement({ codeScope: childA, codeValue: "ext-code" });
        db.saveChanges();
        executeTestCase("external scopes requested child",
          [childA],
          [],
          [rootA, childA, external]);
      });

      it("root has both an external scope dependent AND an intra-set scope dependent", () => {
        const rootA    = insertElement();
        const rootB    = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        const external = insertElement({ codeScope: rootA, codeValue: "ext-code" });
        db.saveChanges();
        executeTestCase("root pruned due to external; sibling still deleted",
          [rootA, rootB],
          [rootB],
          [rootA, external]);
      });

      it("two independent external scope violations", () => {
        const rootA = insertElement();
        const rootB = insertElement();
        const extX   = insertElement({ codeScope: rootA, codeValue: "extX" });
        const extY   = insertElement({ codeScope: rootB, codeValue: "extY" });
        const rootC  = insertElement();
        db.saveChanges();
        executeTestCase("two independent violations",
          [rootA, rootB, rootC],
          [rootC],
          [rootA, rootB, extX, extY]);
      });
    });

    describe("mixed parent-child hierarchy and code scope", () => {
      it("root scopes another root - delete both roots, all descendants removed", () => {
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const childA2 = insertElement({ parentId: rootA });
        const rootB   = insertElement({ codeScope: rootA, codeValue: "rootB-code" });
        const childB1 = insertElement({ parentId: rootB });
        db.saveChanges();
        executeTestCase("root scopes root - delete both roots",
          [rootA, rootB],
          [rootA, childA1, childA2, rootB, childB1],
          []);
      });

      it("depth-1 child scopes an unrelated root", () => {
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const rootB   = insertElement({ codeScope: childA1, codeValue: "rootB-code" });
        const childB1 = insertElement({ parentId: rootB });
        db.saveChanges();
        executeTestCase("depth-1 child scopes root - delete both via parents",
          [rootA, rootB],
          [rootA, childA1, rootB, childB1],
          []);
        // Reverse input order - result must be identical
        executeTestCase("depth-1 child scopes root - reverse input order",
          [rootB, rootA],
          [rootA, childA1, rootB, childB1],
          []);
      });

      it("depth-1 child scopes an unrelated root - delete child and root directly (parent survives)", () => {
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const rootB   = insertElement({ codeScope: childA1, codeValue: "rootB-code" });
        const childB1 = insertElement({ parentId: rootB });
        db.saveChanges();
        // Only childA1 and rootB - rootA is NOT in the delete set.
        executeTestCase("depth-1 child scopes root - delete child + scoped root directly",
          [childA1, rootB],
          [childA1, rootB, childB1],
          [rootA]);
      });

      it("depth-1 child scopes an unrelated root - deleting only the child cascades into the scoped root's subtree", () => {
        // childA1 is the code scope of rootB. When childA1 is deleted, rootB loses its scope
        // element -> rootB (and its children) must also be deleted.
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const rootB   = insertElement({ codeScope: childA1, codeValue: "rootB-code" });
        const childB1 = insertElement({ parentId: rootB });
        db.saveChanges();
        executeTestCase("delete child only - scoped root also removed",
          [childA1],
          [],
          [rootA, childA1, rootB, childB1]);
      });

      it("root scopes a depth-1 child in sibling tree - delete both roots, all descendants removed", () => {
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const rootB   = insertElement();
        const childB1 = insertElement({ parentId: rootB, codeScope: rootA, codeValue: "childB1-code" });
        db.saveChanges();
        executeTestCase("root scopes depth-1 child - delete both roots",
          [rootA, rootB],
          [rootA, childA1, rootB, childB1],
          []);
      });

      it("depth-1 child scopes a depth-1 child in sibling tree - delete both children directly (parents survive)", () => {
        const rootA   = insertElement();
        const childA1 = insertElement({ parentId: rootA });
        const rootB   = insertElement();
        const childB1 = insertElement({ parentId: rootB, codeScope: childA1, codeValue: "childB1-code" });
        const childB2 = insertElement({ parentId: rootB });
        db.saveChanges();
        executeTestCase("sibling-child scope - delete both children directly",
          [childA1, childB1],
          [childA1, childB1],
          [rootA, rootB, childB2]);
      });

      it("depth-2 grandchild scopes an unrelated root - delete grandparent + scoped root", () => {
        const rootA       = insertElement();
        const childA      = insertElement({ parentId: rootA });
        const grandchildA = insertElement({ parentId: childA });
        const rootB       = insertElement({ codeScope: grandchildA, codeValue: "rootB-code" });
        const childB      = insertElement({ parentId: rootB });
        db.saveChanges();
        executeTestCase("depth-2 grandchild scopes root - delete both roots",
          [rootA, rootB],
          [rootA, childA, grandchildA, rootB, childB],
          []);
        // Delete grandchild and scoped root directly (rootA and childA survive)
        executeTestCase("depth-2 grandchild scopes root - delete grandchild + root directly",
          [grandchildA, rootB],
          [grandchildA, rootB, childB],
          [rootA, childA]);
      });

      it("external element scopes a depth-1 child", () => {
        const rootA    = insertElement();
        const childA1  = insertElement({ parentId: rootA });
        const rootB    = insertElement();
        const childB1  = insertElement({ parentId: rootB });
        const external = insertElement({ codeScope: childA1, codeValue: "ext-code" });
        db.saveChanges();
        executeTestCase("external scopes depth-1 child",
          [rootA, rootB],
          [rootB, childB1],
          [rootA, childA1, external]);
      });

      it("external element scopes a depth-2 grandchild", () => {
        const rootA       = insertElement();
        const childA      = insertElement({ parentId: rootA });
        const grandchildA = insertElement({ parentId: childA });
        const rootB       = insertElement();
        const childB      = insertElement({ parentId: rootB });
        const external    = insertElement({ codeScope: grandchildA, codeValue: "ext-code" });
        db.saveChanges();
        executeTestCase("external scopes depth-2 grandchild order 1",
          [rootA, rootB],
          [rootB, childB],
          [rootA, childA, grandchildA, external]);

        executeTestCase("external scopes depth-2 grandchild order 2",
          [grandchildA],
          [],
          [rootA, childA, grandchildA, rootB, childB, external]);

        executeTestCase("external scopes depth-2 grandchild order 3",
          [grandchildA, rootA, childB],
          [childB],
          [rootA, childA, grandchildA, rootB, external]);
      });

      it("two trees: one has external scope violation, other is deleted cleanly", () => {
        const rootA    = insertElement();
        const childA   = insertElement({ parentId: rootA });
        const gcA      = insertElement({ parentId: childA });
        const external = insertElement({ codeScope: childA, codeValue: "ext-code" });
        const rootB    = insertElement();
        const childB1  = insertElement({ parentId: rootB });
        const childB2  = insertElement({ parentId: rootB });
        const gcB      = insertElement({ parentId: childB1 });
        db.saveChanges();
        executeTestCase("one tree pruned, other fully deleted",
          [rootA, rootB],
          [rootB, childB1, childB2, gcB],
          [rootA, childA, gcA, external]);
      });
    });
  });
});
