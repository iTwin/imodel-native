/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert, expect } from "chai";
import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { OpenMode } from "@itwin/core-bentley";
import { GeoCoordStatus } from "@itwin/core-common";
import { Point3d } from "@itwin/core-geometry";
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
      units: "meter",
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
    expect(egm96.unit).to.equal("meter");
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

  it("uses the fallback id when the named vertical coordinate reference system is unknown", () => {
    const response = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify({
        horizontalCRS: { id: "LL84" },
        verticalCRS: { crsName: "Unknown test height", id: "GEOID" },
      }),
    });

    expect(response.status).to.equal(0);
    const verticalCRS = (response.geographicCRS as { verticalCRS?: { id?: string } } | undefined)?.verticalCRS;
    assert.isDefined(verticalCRS);
    expect(verticalCRS.id).to.equal("GEOID");
  });

  it("filters vertical coordinate reference systems by point", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { longitude: 23.7, latitude: 37.9 },
    });

    expect(verticalSystems.some((entry) => entry.crsName === "EGM96 height")).to.be.true;
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.false;
  });

  it("finds all vertical coordinate reference systems containing a point", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { longitude: -100, latitude: 40 },
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

  it("rejects conflicting vertical coordinate reference system filters", () => {
    expect(() => iModelJsNative.GeoServices.getListOfVerticalCRS({
      point: { longitude: 0, latitude: 0 },
      extent: { low: { x: -1, y: -1 }, high: { x: 1, y: 1 } },
    })).to.throw("point and extent are mutually exclusive");
  });

  it("rejects malformed vertical coordinate reference system filters", () => {
    const getListOfVerticalCRS = (props?: unknown) =>
      (iModelJsNative.GeoServices.getListOfVerticalCRS as (value?: unknown) => unknown)(props);

    expect(() => getListOfVerticalCRS({ point: "invalid" })).to.throw("point must be an object");
    expect(() => getListOfVerticalCRS({ point: { longitude: 0 } })).to.throw("point must contain numeric longitude and latitude");
    expect(() => getListOfVerticalCRS({ point: { longitude: Number.NaN, latitude: 0 } })).to.throw("point longitude and latitude must be finite");
    expect(() => getListOfVerticalCRS({ extent: "invalid" })).to.throw("extent must be an object");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0 }, high: { x: 1, y: 1 } } })).to.throw("extent must contain numeric low and high points");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0, y: 0 }, high: { x: Number.POSITIVE_INFINITY, y: 1 } } })).to.throw("extent coordinates must be finite");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: 0, y: 2 }, high: { x: 1, y: 1 } } })).to.throw("extent low latitude must not exceed high latitude");
    expect(() => getListOfVerticalCRS({ extent: { low: { x: -181, y: 0 }, high: { x: 1, y: 1 } } })).to.throw();
    expect(() => getListOfVerticalCRS({ includeIntersecting: "yes" })).to.throw("includeIntersecting must be a boolean");
  });
});