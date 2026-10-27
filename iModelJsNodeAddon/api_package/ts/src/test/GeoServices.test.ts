/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert, expect } from "chai";
import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { IModelStatus, OpenMode } from "@itwin/core-bentley";
import { GeoCoordStatus } from "@itwin/core-common";
import { Point3d, Range3d } from "@itwin/core-geometry";
import { getLocalBuildOfAddonPath, getOutputDir, iModelJsNative } from "./utils";

const verticalDatumDictionary = JSON.stringify({
  version: 1,
  definitions: [{
    verticalCRS: {
      crsName: "EGM96 height",
      datumName: "EGM96 geoid",
      type: "GEOID",
      units: "meter",
      description: "EGM96 height",
      deprecated: false,
      extent: {
        southWest: { latitude: -90, longitude: -180 },
        northEast: { latitude: 90, longitude: 180 },
      },
      transforms: [{
        target: "WGS84",
        geoidSeparationGrid: {
          direction: "Direct",
          format: "GRD",
          files: ["./World/WW15MGH.GRD"],
        },
      }],
    },
  }, {
    verticalCRS: {
      crsName: "Regional test height",
      datumName: "Regional test datum",
      type: "GEOID",
      units: "ussurveyfoot",
      description: "Regional test height",
      deprecated: false,
      extent: {
        southWest: { latitude: 25, longitude: -125 },
        northEast: { latitude: 50, longitude: -65 },
      },
      transforms: [{ target: "WGS84", nullTransform: null }],
    },
  }, {
    verticalCRS: {
      crsName: "NAVD88 height",
      datumName: "North American Vertical Datum 1988",
      type: "GEOID",
      units: "meter",
      description: "NAVD88 height",
      deprecated: false,
      extent: {
        southWest: { latitude: 14, longitude: -170 },
        northEast: { latitude: 72, longitude: -60 },
      },
      transforms: [{ target: "WGS84", nullTransform: null }],
    },
  }, {
    verticalCRS: {
      crsName: "NGVD29 height",
      datumName: "National Geodetic Vertical Datum 1929",
      epsg: 7968,
      type: "GEOID",
      units: "meter",
      description: "NGVD29 height",
      deprecated: false,
      extent: {
        southWest: { latitude: 14, longitude: -170 },
        northEast: { latitude: 72, longitude: -60 },
      },
      transforms: [{ target: "NAVD88 height", nullTransform: null }],
    },
  }, {
    verticalCRS: {
      crsName: "WGS84",
      datumName: "WGS_1984",
      epsg: 4326,
      type: "ELLIPSOID",
      units: "meter",
      description: "WGS84 ellipsoid",
      deprecated: false,
      extent: {
        southWest: { latitude: -90, longitude: -180 },
        northEast: { latitude: 90, longitude: 180 },
      },
    },
  }, {
    verticalCRS: {
      crsName: "ITRF2008",
      datumName: "International Terrestrial Reference Frame 2008",
      epsg: 5332,
      type: "ELLIPSOID",
      units: "meter",
      description: "ITRF2008 ellipsoid",
      deprecated: false,
      extent: {
        southWest: { latitude: -90, longitude: -180 },
        northEast: { latitude: 90, longitude: 180 },
      },
    },
  }],
});

describe("GeoServices", () => {
  before(() => {
    const workspacePath = path.join(getOutputDir(), "GeoServices.itwin-workspace");
    fs.rmSync(workspacePath, { force: true });

    assert.isDefined(process.env.SrcRoot);
    const upackDir = path.join(process.env.SrcRoot, "upack");
    const csMapDataPackage = fs.readdirSync(upackDir).find((entry) => entry.startsWith("csmap_data."));
    assert.isDefined(csMapDataPackage);
    const csMapDataDir = path.join(upackDir, csMapDataPackage, "Dictionaries");

    const workspaceDb = new iModelJsNative.SQLiteDb();
    workspaceDb.createDb(workspacePath);

    const statement = new iModelJsNative.SqliteStatement();
    statement.prepare(workspaceDb, "CREATE TABLE blobs(id TEXT PRIMARY KEY NOT NULL, value BLOB)");
    statement.step();
    statement.dispose();

    statement.prepare(workspaceDb, "INSERT INTO blobs(id,value) VALUES(?,?)");
    const egm96Grid = fs.readFileSync(path.join(csMapDataDir, "WW15MGH._96"));
    const resources = new Map<string, Buffer>([
      ["VerticalDatumDefinitions.json", Buffer.from(verticalDatumDictionary)],
      ["World/WW15MGH._96", egm96Grid],
      ...["coordsys.dty", "datum.dty", "ellipsoid.dty", "GeodeticTransform.dty", "GeodeticPath.dty"]
        .map((fileName): [string, Buffer] => [fileName, fs.readFileSync(path.join(csMapDataDir, fileName))]),
    ]);
    for (const [fileName, contents] of resources) {
      statement.reset();
      statement.bindString(1, fileName);
      statement.bindBlob(2, contents);
      statement.step();
    }
    statement.dispose();
    workspaceDb.saveChanges();
    workspaceDb.closeDb();

    expect(iModelJsNative.addGcsWorkspaceDb(workspacePath, undefined, 10000)).to.be.true;
  });

  it("reports a missing vertical datum dictionary", () => {
    const script = `
      const addon = require(process.argv[1]);
      try {
        addon.GeoServices.getListOfVerticalCRS();
        process.exitCode = 2;
      } catch (error) {
        process.stdout.write(error.message);
      }
    `;
    const result = spawnSync(process.execPath, ["-e", script, getLocalBuildOfAddonPath()], { encoding: "utf8" });

    expect(result.status, result.stderr).to.equal(0);
    expect(result.stdout).to.equal("unable to query vertical coordinate reference systems (status 16427)");
  });

  it("enumerates vertical coordinate reference systems", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS();
    const egm96 = verticalSystems.find((entry) => entry.crsName === "EGM96 height");

    assert.isDefined(egm96);
    expect(egm96.id).to.equal("GEOID");
    expect(egm96.description).to.equal("EGM96 height");
    expect(egm96.deprecated).to.be.false;
    expect(egm96.type).to.equal("GEOID");
    expect(egm96.unit).to.equal("Meter");
    expect(egm96.extent).to.deep.equal({ low: [-180, -90], high: [180, 90] });
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.true;

    for (const expected of [
      { crsName: "EGM96 height", id: "GEOID", type: "GEOID", epsg: undefined },
      { crsName: "NAVD88 height", id: "NAVD88", type: "GEOID", epsg: undefined },
      { crsName: "NGVD29 height", id: "NGVD29", type: "GEOID", epsg: 7968 },
      { crsName: "WGS84", id: "ELLIPSOID", type: "ELLIPSOID", epsg: 4326 },
      { crsName: "ITRF2008", id: "ELLIPSOID", type: "ELLIPSOID", epsg: 5332 },
    ]) {
      const entry = verticalSystems.find((candidate) => candidate.crsName === expected.crsName);
      assert.isDefined(entry);
      expect(entry.id).to.equal(expected.id);
      expect(entry.type).to.equal(expected.type);
      expect(entry.epsg).to.equal(expected.epsg);
    }
  });

  it("prefers a named vertical coordinate reference system over a conflicting fallback id", () => {
    const response = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify({
        horizontalCRS: { id: "LL84" },
        verticalCRS: { crsName: "EGM96 height", id: "ELLIPSOID" },
      }),
    });

    expect(response.status).to.equal(0);
    const verticalCRS = (response.geographicCRS as { verticalCRS?: { crsName?: string, id?: string } } | undefined)?.verticalCRS;
    assert.isDefined(verticalCRS);
    expect(verticalCRS.crsName).to.equal("EGM96 height");
    expect(verticalCRS.id).to.equal("GEOID");
  });

  it("round-trips a complete named vertical coordinate reference system", () => {
    const firstResponse = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify({
        horizontalCRS: { id: "LL84" },
        verticalCRS: { crsName: "EGM96 height", id: "GEOID" },
      }),
    });
    assert.isDefined(firstResponse.geographicCRS);

    const secondResponse = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify(firstResponse.geographicCRS),
    });

    expect(secondResponse.status).to.equal(0);
    expect(secondResponse.geographicCRS).to.deep.equal(firstResponse.geographicCRS);
  });

  it("restores a named vertical coordinate reference system when its fallback id conflicts", () => {
    const iModelPath = path.join(getOutputDir(), "ConflictingVerticalCrsId.bim");
    fs.rmSync(iModelPath, { force: true });

    const iModelDb = new iModelJsNative.DgnDb();
    try {
      iModelDb.createIModel(iModelPath, { rootSubject: { name: "Conflicting Vertical CRS id" } });
      const geographicCoordinateSystem = {
        horizontalCRS: { id: "LL84" },
        verticalCRS: { id: "GEOID" as const, crsName: "EGM96 height" },
      };
      iModelDb.updateIModelProps({
        rootSubject: { name: "Conflicting Vertical CRS id: IGNORED BY updateIModelProps" },
        geographicCoordinateSystem,
      });

      const storedProperty = iModelDb.queryFileProperty(
        { namespace: "dgn_Db", name: "DgnGCSVerticalCRS" },
        true,
      );
      if (typeof storedProperty !== "string")
        assert.fail("Expected the named vertical CRS file property to exist as a string");

      const storedVerticalCrs = JSON.parse(storedProperty);
      storedVerticalCrs.verticalCRS.id = "ELLIPSOID";
      iModelDb.saveFileProperty(
        { namespace: "dgn_Db", name: "DgnGCSVerticalCRS" },
        JSON.stringify(storedVerticalCrs),
        undefined,
      );
      iModelDb.saveChanges();
      iModelDb.closeFile();
      iModelDb.openIModel(iModelPath, OpenMode.ReadWrite);

      const verticalCRS = iModelDb.getIModelProps().geographicCoordinateSystem?.verticalCRS as { crsName?: string, id?: string } | undefined;
      expect(verticalCRS?.crsName).to.equal("EGM96 height");
      expect(verticalCRS?.id).to.equal("GEOID");
    } finally {
      iModelDb.closeFile();
      fs.rmSync(iModelPath, { force: true });
    }
  });

  it("converts a named vertical coordinate reference system with a meaningful elevation", () => {
    const iModelPath = path.join(getOutputDir(), "NamedVerticalCrsConversion.bim");
    fs.rmSync(iModelPath, { force: true });

    const iModelDb = new iModelJsNative.DgnDb();
    try {
      iModelDb.createIModel(iModelPath, { rootSubject: { name: "Named Vertical CRS conversion" } });
      const geographicCoordinateSystem = {
        horizontalCRS: { id: "LL84" },
        verticalCRS: { id: "GEOID" as const, crsName: "EGM96 height" },
      };
      iModelDb.updateIModelProps({
        rootSubject: { name: "Named Vertical CRS conversion: IGNORED BY updateIModelProps" },
        geographicCoordinateSystem,
      });
      iModelDb.saveChanges();
      iModelDb.closeFile();
      iModelDb.openIModel(iModelPath, OpenMode.ReadWrite);

      const storedVerticalCRS = (iModelDb.getIModelProps().geographicCoordinateSystem?.verticalCRS as { crsName?: string } | undefined);
      expect(storedVerticalCRS?.crsName).to.equal("EGM96 height");

      const response = iModelDb.getGeoCoordinatesFromIModelCoordinates({
        target: JSON.stringify({
          horizontalCRS: { id: "LL84" },
          verticalCRS: { id: "ELLIPSOID", crsName: "WGS84" },
        }),
        iModelCoords: [{ x: 23.700523, y: 37.944210, z: 0 }],
      });

      expect(response.geoCoords).to.have.lengthOf(1);
      expect(response.geoCoords[0].s).to.equal(GeoCoordStatus.Success);
      const convertedPoint = Point3d.fromJSON(response.geoCoords[0].p);
      expect(convertedPoint.x).to.be.closeTo(23.700523, 1.0e-8);
      expect(convertedPoint.y).to.be.closeTo(37.944210, 1.0e-8);
      expect(convertedPoint.z).to.be.closeTo(38.3, 0.5);
    } finally {
      iModelDb.closeFile();
      fs.rmSync(iModelPath, { force: true });
    }
  });

  it("ignores named vertical coordinate reference system metadata after Type 66 changes", () => {
    const iModelPath = path.join(getOutputDir(), "StaleNamedVerticalCrs.bim");
    fs.rmSync(iModelPath, { force: true });

    const iModelDb = new iModelJsNative.DgnDb();
    try {
      iModelDb.createIModel(iModelPath, { rootSubject: { name: "Stale named Vertical CRS" } });
      const geographicCoordinateSystem = {
        horizontalCRS: { id: "LL84" },
        verticalCRS: { id: "GEOID" as const, crsName: "EGM96 height" },
      };
      iModelDb.updateIModelProps({
        rootSubject: { name: "Stale named Vertical CRS: IGNORED BY updateIModelProps" },
        geographicCoordinateSystem,
      });
      const verticalCrsProperty = iModelDb.queryFileProperty(
        { namespace: "dgn_Db", name: "DgnGCSVerticalCRS" },
        true,
      );
      if (typeof verticalCrsProperty !== "string")
        assert.fail("Expected the named vertical CRS file property to exist as a string");

      iModelDb.updateIModelProps({
        rootSubject: { name: "Stale named Vertical CRS: IGNORED BY updateIModelProps" },
        geographicCoordinateSystem: {
          horizontalCRS: { id: "LL84" },
          verticalCRS: { id: "ELLIPSOID" },
        },
      });
      iModelDb.saveFileProperty(
        { namespace: "dgn_Db", name: "DgnGCSVerticalCRS" },
        verticalCrsProperty,
        undefined,
      );
      iModelDb.saveChanges();
      iModelDb.closeFile();
      iModelDb.openIModel(iModelPath, OpenMode.ReadWrite);

      const legacyVerticalCRS = (iModelDb.getIModelProps().geographicCoordinateSystem?.verticalCRS as { crsName?: string, id?: string } | undefined);
      expect(legacyVerticalCRS?.crsName).to.equal("WGS84");
      expect(legacyVerticalCRS?.id).to.equal("ELLIPSOID");
    } finally {
      iModelDb.closeFile();
      fs.rmSync(iModelPath, { force: true });
    }
  });

  it("rejects invalid GCS updates without changing geolocation or persisted properties", () => {
    const iModelPath = path.join(getOutputDir(), "InvalidGcsUpdate.bim");
    fs.rmSync(iModelPath, { force: true });
    const iModelDb = new iModelJsNative.DgnDb();
    try {
      iModelDb.createIModel(iModelPath, { rootSubject: { name: "Invalid GCS update" } });
      const initialGcs = {
        horizontalCRS: { id: "LL84" },
        verticalCRS: { id: "GEOID" as const, crsName: "EGM96 height" },
      };
      iModelDb.updateIModelProps({
        rootSubject: { name: "ignored" },
        geographicCoordinateSystem: initialGcs,
      });
      iModelDb.saveChanges();
      const originalProps = iModelDb.getIModelProps();
      const property = (name: string) => iModelDb.queryFileProperty({ namespace: "dgn_Db", name }, true);
      const originalUnits = property("Units");
      const originalGcs = iModelDb.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCS" }, false);
      const originalVerticalCrs = property("DgnGCSVerticalCRS");
      assert.isString(originalUnits);
      assert.instanceOf(originalGcs, Uint8Array);
      assert.isString(originalVerticalCrs);

      for (const geographicCoordinateSystem of [
        {},
        { horizontalCRS: { id: "Unknown horizontal CRS" } },
        { horizontalCRS: { id: "LL84" }, verticalCRS: { id: "GEOID" as const, crsName: "Unknown test height" } },
        { horizontalCRS: { id: "LL84" }, verticalCRS: { id: "GEOID" as const, epsg: 999999 } },
      ]) {
        expect(() => iModelDb.updateIModelProps({
          rootSubject: { name: "ignored" },
          geographicCoordinateSystem,
          projectExtents: { low: { x: -100, y: -200, z: -300 }, high: { x: 400, y: 500, z: 600 } },
          globalOrigin: { x: 100, y: 200, z: 300 },
          ecefLocation: { origin: { x: 400, y: 500, z: 600 }, orientation: { yaw: 0, pitch: 0, roll: 0 } },
        })).to.throw(/Invalid geographic coordinate system: .+/).with.property("errorNumber", IModelStatus.BadArg);
        expect(iModelDb.getIModelProps()).to.deep.equal(originalProps);
        expect(property("Units")).to.equal(originalUnits);
        expect(iModelDb.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCS" }, false)).to.deep.equal(originalGcs);
        expect(property("DgnGCSVerticalCRS")).to.equal(originalVerticalCrs);
        iModelDb.saveChanges();
        iModelDb.closeFile();
        iModelDb.openIModel(iModelPath, OpenMode.ReadWrite);
        expect(iModelDb.getIModelProps()).to.deep.equal(originalProps);
        expect(property("Units")).to.equal(originalUnits);
        expect(iModelDb.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCS" }, false)).to.deep.equal(originalGcs);
        expect(property("DgnGCSVerticalCRS")).to.equal(originalVerticalCrs);
      }
    } finally {
      iModelDb.closeFile();
      fs.rmSync(iModelPath, { force: true });
    }
  });

  it("persists intentional GCS replacements with other geolocation updates", () => {
    const iModelPath = path.join(getOutputDir(), "ValidGcsUpdate.bim");
    fs.rmSync(iModelPath, { force: true });
    const iModelDb = new iModelJsNative.DgnDb();
    try {
      iModelDb.createIModel(iModelPath, { rootSubject: { name: "Valid GCS update" } });
      for (const verticalCRS of [
        { id: "GEOID" as const, crsName: "EGM96 height" },
        { id: "ELLIPSOID" as const },
      ]) {
        const projectExtents = { low: { x: -100, y: -200, z: -300 }, high: { x: 400, y: 500, z: 600 } };
        const globalOrigin = { x: 100, y: 200, z: 300 };
        const ecefLocation = { origin: { x: 400, y: 500, z: 600 }, orientation: { yaw: 0, pitch: 0, roll: 0 } };
        iModelDb.updateIModelProps({
          rootSubject: { name: "ignored" },
          geographicCoordinateSystem: { horizontalCRS: { id: "LL84" }, verticalCRS },
          projectExtents, globalOrigin, ecefLocation,
        });
        const storedUnits = iModelDb.queryFileProperty({ namespace: "dgn_Db", name: "Units" }, true);
        if (typeof storedUnits !== "string")
          assert.fail("Expected the geolocation Units file property to exist as a string");
        const units = JSON.parse(storedUnits);
        expect(Point3d.fromJSON(units.globalOrigin).isAlmostEqual(Point3d.fromJSON(globalOrigin))).to.be.true;
        expect(Point3d.fromJSON(units.ecefLocation.origin).isAlmostEqual(Point3d.fromJSON(ecefLocation.origin))).to.be.true;
        const updatedProps = iModelDb.getIModelProps();
        expect(Range3d.fromJSON(updatedProps.projectExtents).isAlmostEqual(Range3d.fromJSON(projectExtents))).to.be.true;
        expect(Point3d.fromJSON(updatedProps.globalOrigin).isAlmostEqual(Point3d.fromJSON(globalOrigin))).to.be.true;
        assert.isDefined(updatedProps.ecefLocation);
        expect(updatedProps.geographicCoordinateSystem?.verticalCRS?.id).to.equal(verticalCRS.id);
        iModelDb.saveChanges();
        iModelDb.closeFile();
        iModelDb.openIModel(iModelPath, OpenMode.ReadWrite);
        expect(iModelDb.getIModelProps()).to.deep.equal(updatedProps);
      }
    } finally {
      iModelDb.closeFile();
      fs.rmSync(iModelPath, { force: true });
    }
  });

  it("does not publish rejected custom vertical transforms before the saved GCS is loaded", () => {
          const workspacePath = path.join(getOutputDir(), "GeoServices.itwin-workspace");
          const iModelPath = path.join(getOutputDir(), "RejectedCustomVerticalTransform.bim");
          fs.rmSync(iModelPath, { force: true });
          const geographicCoordinateSystem = {
            horizontalCRS: {
              id: "RejectedUpdateLL",
              datumId: "WGS84",
              unit: "Degree" as const,
              projection: { method: "None" as const },
              extent: { southWest: { latitude: -1, longitude: -1 }, northEast: { latitude: 1, longitude: 1 } },
            },
            verticalCRS: {
              id: "GEOID" as const,
              crsName: "Rejected update test height",
              datumName: "Rejected update test height",
              type: "GEOID",
              units: "meter",
              extent: { southWest: { latitude: -90, longitude: -180 }, northEast: { latitude: 90, longitude: 180 } },
              transforms: [{ target: "WGS84", verticalOffset: { offset: 10, units: "meter" } }],
            },
          };
          const iModelDb = new iModelJsNative.DgnDb();
          try {
            iModelDb.createIModel(iModelPath, { rootSubject: { name: "Rejected custom vertical transform" } });
            iModelDb.updateIModelProps({
              rootSubject: { name: "ignored" },
              geographicCoordinateSystem,
              projectExtents: { low: { x: -0.5, y: -0.5, z: 0 }, high: { x: 0.5, y: 0.5, z: 100 } },
            });
            const unitsProperty = { namespace: "dgn_Db", name: "Units" };
            const unitsString = iModelDb.queryFileProperty(unitsProperty, true);
            if (typeof unitsString !== "string")
              assert.fail("Expected the geolocation Units file property to exist as a string");
            const units = JSON.parse(unitsString);
            // Exercise lazy GCS loading with no saved ECEF in each fresh process.
            delete units.ecefLocation;
            iModelDb.saveFileProperty(unitsProperty, JSON.stringify(units), undefined);
            iModelDb.saveChanges();
            iModelDb.closeFile();

            const rejectedVerticalCRS = {
              ...geographicCoordinateSystem.verticalCRS,
              transforms: [{ target: "WGS84", verticalOffset: { offset: 1000, units: "meter" } }],
            };
            const script = `
              const assert = require("node:assert/strict");
              const addon = require(process.argv[1]);
              const { IModelStatus, OpenMode } = require("@itwin/core-bentley");
              assert.equal(addon.addGcsWorkspaceDb(process.argv[2], undefined, 10000), true);
              const db = new addon.DgnDb();
              try {
                db.openIModel(process.argv[3], OpenMode.ReadWrite);
                const units = db.queryFileProperty({ namespace: "dgn_Db", name: "Units" }, true);
                assert.equal(JSON.parse(units).ecefLocation, undefined);
                const vertical = db.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCSVerticalCRS" }, true);
                const gcs = db.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCS" }, false);
                assert.throws(() => db.updateIModelProps({
                  rootSubject: { name: "ignored" },
                  projectExtents: { low: { x: -1, y: -1, z: 0 }, high: { x: 1, y: 1, z: 100 } },
                  geographicCoordinateSystem: JSON.parse(process.argv[4]),
                }), (error) => error.errorNumber === IModelStatus.BadArg && /Invalid geographic coordinate system/.test(error.message));
                assert.equal(db.queryFileProperty({ namespace: "dgn_Db", name: "Units" }, true), units);
                assert.equal(db.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCSVerticalCRS" }, true), vertical);
                assert.deepEqual(db.queryFileProperty({ namespace: "dgn_Db", name: "DgnGCS" }, false), gcs);
                const response = db.getGeoCoordinatesFromIModelCoordinates({
                  target: JSON.stringify({ horizontalCRS: { id: "LL84" }, verticalCRS: { id: "ELLIPSOID", crsName: "WGS84" } }),
                  iModelCoords: [{ x: 0, y: 0, z: 100 }],
                });
                assert.equal(response.geoCoords.length, 1);
                assert.equal(response.geoCoords[0].s, 0);
                assert.ok(Math.abs(response.geoCoords[0].p[2] - 110) < 1e-8, JSON.stringify(response));
              } finally {
                db.closeFile();
              }
            `;
            for (const [label, replacement] of [
              ["invalid additional transform", {
                horizontalCRS: geographicCoordinateSystem.horizontalCRS,
                verticalCRS: rejectedVerticalCRS,
                additionalTransform: {},
              }],
              ["invalid vertical transform", {
                horizontalCRS: geographicCoordinateSystem.horizontalCRS,
                verticalCRS: { ...rejectedVerticalCRS, transforms: [...rejectedVerticalCRS.transforms, { target: "WGS84", verticalOffset: {} }] },
              }],
              ["invalid vertical transform path", {
                horizontalCRS: geographicCoordinateSystem.horizontalCRS,
                verticalCRS: { ...rejectedVerticalCRS, transformPaths: [{ target: "WGS84", path: ["Wrong source", "WGS84"] }] },
              }],
              ["non-overlapping vertical extent", {
                horizontalCRS: geographicCoordinateSystem.horizontalCRS,
                verticalCRS: {
                  ...rejectedVerticalCRS,
                  extent: { southWest: { latitude: 20, longitude: 20 }, northEast: { latitude: 30, longitude: 30 } },
                },
              }],
            ] as const) {
              const result = spawnSync(process.execPath, ["-e", script, getLocalBuildOfAddonPath(), workspacePath, iModelPath, JSON.stringify(replacement)], {
                encoding: "utf8", timeout: 60000,
              });
              expect(result.status, `${label}: ${result.error?.message ?? ""}\n${result.stderr}\n${result.stdout}`).to.equal(0);
            }
          } finally {
            iModelDb.closeFile();
            fs.rmSync(iModelPath, { force: true });
          }
        });
  it("rejects an unknown or invalid named vertical coordinate reference system instead of using the fallback id", () => {
    for (const verticalCRS of [
      { crsName: "Unknown test height", id: "GEOID" },
      { crsName: "", id: "GEOID" },
      { epsg: 999999, id: "GEOID" },
      { epsg: 0, id: "GEOID" },
      { epsg: -1, id: "GEOID" },
      { epsg: "5773", id: "GEOID" },
      { crsName: "Unknown test height", epsg: 999999, id: "GEOID" },
    ]) {
      const response = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
        format: "JSON",
        geographicCRSDef: JSON.stringify({ horizontalCRS: { id: "LL84" }, verticalCRS }),
      });

      expect(response.status, JSON.stringify(verticalCRS)).to.not.equal(0);
      expect(response.geographicCRS, JSON.stringify(verticalCRS)).to.be.undefined;
    }
  });

  it("omits epsg from a vertical coordinate reference system that has no EPSG code", () => {
    const response = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify({
        horizontalCRS: { id: "LL84" },
        verticalCRS: { crsName: "NAVD88 height", id: "NAVD88" },
      }),
    });

    expect(response.status).to.equal(0);
    const verticalCRS = (response.geographicCRS as { verticalCRS?: { crsName?: string, epsg?: number } } | undefined)?.verticalCRS;
    assert.isDefined(verticalCRS);
    expect(verticalCRS.crsName).to.equal("NAVD88 height");
    expect(verticalCRS).to.not.have.property("epsg");
  });

  it("filters vertical coordinate reference systems by point", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { x: 23.7, y: 37.9 },
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.false;
  });

  it("finds all vertical coordinate reference systems containing a point", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { x: -100, y: 40 },
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.true;
  });

  it("excludes vertical coordinate reference systems that only partially overlap an extent", () => {
    // The regional CRS starts at longitude -125, so this extent overlaps it without being fully contained.
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      extent: { low: { x: -130, y: 39 }, high: { x: -120, y: 41 } },
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.false;
  });

  it("includes vertical coordinate reference systems that intersect an extent", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      extent: { low: { x: -130, y: 39 }, high: { x: -120, y: 41 } },
      includeIntersecting: true,
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.true;
  });

  it("finds all vertical coordinate reference systems containing an extent", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      extent: { low: { x: -101, y: 39 }, high: { x: -99, y: 41 } },
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.true;
  });

  it("filters vertical coordinate reference systems by canonical unit name case-insensitively", () => {
    const matchingSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({ unit: "mEtEr" });
    expect(matchingSystems).not.to.be.empty;
    expect(matchingSystems.every((entry) => entry.unit === "Meter")).to.be.true;

    const surveyFootSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({ unit: "ussurveyfoot" });
    expect(surveyFootSystems.map((entry) => entry.crsName)).to.deep.equal(["Regional test height"]);
    expect(surveyFootSystems[0].unit).to.equal("USSurveyFoot");

    expect(iModelJsNative.GeoServices.getListOfVerticalCRS({ unit: "Degree" })).to.be.empty;
  });

  it("rejects conflicting vertical coordinate reference system filters", () => {
    expect(() => iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { x: 0, y: 0 },
      extent: { low: { x: -1, y: -1 }, high: { x: 1, y: 1 } },
    })).to.throw("point and extent are mutually exclusive");
  });

  it("rejects malformed vertical coordinate reference system filters", () => {
    const getListOfVerticalCRS = (props?: unknown) =>
      (iModelJsNative.GeoServices.getListOfVerticalCRS as (value?: unknown) => unknown)(props);

    expect(() => getListOfVerticalCRS({ point: "invalid" })).to.throw("point must be an object");
    expect(() => getListOfVerticalCRS({ point: { x: 0 } })).to.throw("point must contain numeric x and y coordinates");
    expect(() => getListOfVerticalCRS({ point: { x: Number.NaN, y: 0 } })).to.throw("point coordinates must be finite");
    expect(() => getListOfVerticalCRS({ extent: "invalid" })).to.throw("extent must be an object");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0 }, high: { x: 1, y: 1 } } })).to.throw("extent must contain numeric low and high points");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0, y: 0 }, high: { x: Number.POSITIVE_INFINITY, y: 1 } } })).to.throw("extent coordinates must be finite");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0, y: 2 }, high: { x: 1, y: 1 } } })).to.throw("extent low latitude must not exceed high latitude");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: -181, y: 0 }, high: { x: 1, y: 1 } } })).to.throw();
    expect(() => getListOfVerticalCRS({ includeIntersecting: "yes" })).to.throw("includeIntersecting must be a boolean");
    expect(() => getListOfVerticalCRS({ unit: 1 })).to.throw("unit must be a string");
  });
});