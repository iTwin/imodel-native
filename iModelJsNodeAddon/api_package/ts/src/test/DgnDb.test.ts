/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { dbFileName, iModelJsNative } from "./utils";
import { DbResult, Id64Array, IModelStatus } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { assert, expect } from "chai";
import { openDgnDb } from ".";
import * as os from "os";

// Crash reporting on linux is gated by the presence of this env variable.
if (os.platform() === "linux")
  process.env.LINUX_MINIDUMP_ENABLED = "yes";

describe("basic tests", () => {

  let dgndb: IModelJsNative.DgnDb;

  before((done) => {
    dgndb = openDgnDb(dbFileName);
    done();
  })

  after((done) => {
    dgndb.closeIModel();
    done();
  })

  // verify that throwing javascript exceptions from C++ works
  it("testExceptions", () => {
    // first try a function
    expect(() => (iModelJsNative as any).storeObjectInVault()).to.throw("Argument 0");

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
    const onGraphics = (info: any) => { elementsWithGraphics[info.elementId] = true; };
    const res = dgndb.exportGraphics({ elementIdArray, onGraphics });

    assert.equal(res, 0, `IModelDb.exportGraphics returned ${res}`);
    for (const id of elementIdArray)
      assert.isDefined(elementsWithGraphics[id], `No graphics generated for ${id}`);
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
    expect(() => dgndb.queryModelExtents({id: "NotAnId"})).to.throw("Invalid id").property("errorNumber").equal(IModelStatus.InvalidId);
    expect(() => dgndb.queryModelExtents({id: "0xabcdef"})).to.throw("not found").property("errorNumber").equal(IModelStatus.NotFound);
    expect(() => dgndb.queryModelExtents({id: "0x1"})).to.throw("error=10040").property("errorNumber").equal(IModelStatus.WrongModel);
    expect(() => dgndb.queryModelExtents({id: "0x1c"})).to.throw("error=10022").property("errorNumber").equal(IModelStatus.NoGeometry);

    expectExtents(dgndb.queryModelExtents({id: "0x23"}).modelExtents, { low: [-10, -16, -10], high: [14, 6, 10] });
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
      await expect(dgndb.generateElementMeshes({source: "NotAnId"})).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({ })).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({source: "0"})).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({source: "0x1"})).rejectedWith(msg);
      await expect(dgndb.generateElementMeshes({source: "0x123456789"})).rejectedWith(msg);
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

        const u32 = new Uint32Array(bytes.buffer);
        expect(u32[1]).to.equal(0);

        for (let i = 0; i < 4; i++)
          expect(bytes[8 + i]).to.equal("PLFC".charCodeAt(i));

        expect(u32[3]).least(8);
      }
    });
  });
});
