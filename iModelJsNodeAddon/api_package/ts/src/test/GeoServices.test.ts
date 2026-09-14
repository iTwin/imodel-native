/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert, expect } from "chai";
import * as fs from "fs";
import * as path from "path";
import { getOutputDir, iModelJsNative } from "./utils";

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
      transforms: [{ target: "WGS84", nullTransform: null }],
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
  }],
});

describe("GeoServices", () => {
  before(() => {
    const workspacePath = path.join(getOutputDir(), "GeoServices.itwin-workspace");
    fs.rmSync(workspacePath, { force: true });

    const workspaceDb = new iModelJsNative.SQLiteDb();
    workspaceDb.createDb(workspacePath);

    const statement = new iModelJsNative.SqliteStatement();
    statement.prepare(workspaceDb, "CREATE TABLE blobs(id TEXT PRIMARY KEY NOT NULL, value BLOB)");
    statement.step();
    statement.dispose();

    statement.prepare(workspaceDb, "INSERT INTO blobs(id,value) VALUES(?,?)");
    statement.bindString(1, "VerticalDatumDefinitions.json");
    statement.bindBlob(2, Buffer.from(verticalDatumDictionary));
    statement.step();
    statement.dispose();
    workspaceDb.saveChanges();
    workspaceDb.closeDb();

    expect(iModelJsNative.addGcsWorkspaceDb(workspacePath, undefined, 10000)).to.be.true;
  });

  it("enumerates vertical coordinate reference systems", () => {
    const verticalSystems = iModelJsNative.GeoServices.getListOfVerticalCRS();
    const egm96 = verticalSystems.find((entry) => entry.crsName === "EGM96 height");

    assert.isDefined(egm96);
    expect(egm96.id).to.equal("GEOID");
    expect(egm96.type).to.equal("GEOID");
    expect(egm96.unit).to.equal("meter");
    expect(egm96.extent).to.deep.equal({ low: [-180, -90], high: [180, 90] });
    expect(verticalSystems.some((entry) => entry.crsName === "Regional test height")).to.be.true;
  });

  it("interprets a named vertical coordinate reference system with its fallback id", () => {
    const response = iModelJsNative.GeoServices.getGeographicCRSInterpretation({
      format: "JSON",
      geographicCRSDef: JSON.stringify({
        horizontalCRS: { id: "LL84" },
        verticalCRS: { crsName: "EGM96 height", id: "GEOID" },
      }),
    });

    expect(response.status).to.equal(0);
    const verticalCRS = (response.geographicCRS as { verticalCRS?: { crsName?: string, id?: string } } | undefined)?.verticalCRS;
    assert.isDefined(verticalCRS);
    expect(verticalCRS.crsName).to.equal("EGM96 height");
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
});