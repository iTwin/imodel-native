/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64, Id64String } from "@itwin/core-bentley";
import { Code, ElementProps, IModel } from "@itwin/core-common";
import { assert, expect } from "chai";
import * as fs from "fs";
import * as path from "path";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, getOutputDir, iModelJsNative } from "./utils";

const testSchemaXml = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="CloneTest" alias="ct" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
  <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
  <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
  <ECCustomAttributes>
    <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
  </ECCustomAttributes>
  <ECStructClass typeName="Info">
    <ECProperty propertyName="Name" typeName="string"/>
    <ECProperty propertyName="Count" typeName="int"/>
  </ECStructClass>
  <ECEntityClass typeName="TestPhysicalType">
    <BaseClass>bis:PhysicalType</BaseClass>
  </ECEntityClass>
  <ECEntityClass typeName="TestGraphicalType2d">
    <BaseClass>bis:GraphicalType2d</BaseClass>
  </ECEntityClass>
  <ECEntityClass typeName="TestPhysicalElement">
    <BaseClass>bis:PhysicalElement</BaseClass>
    <ECProperty propertyName="StringProp" typeName="string"/>
    <ECProperty propertyName="PointProp" typeName="point3d"/>
    <ECStructProperty propertyName="InfoProp" typeName="Info"/>
    <ECArrayProperty propertyName="TagsProp" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
    <ECStructArrayProperty propertyName="InfosProp" typeName="Info" minOccurs="0" maxOccurs="unbounded"/>
    <ECNavigationProperty propertyName="RefProp" relationshipName="TestPhysicalElementRefersToElement" direction="forward"/>
  </ECEntityClass>
  <ECRelationshipClass typeName="TestPhysicalElementRefersToElement" strength="referencing" modifier="Sealed">
    <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
      <Class class="TestPhysicalElement"/>
    </Source>
    <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
      <Class class="bis:Element"/>
    </Target>
  </ECRelationshipClass>
</ECSchema>`;

/** Ids of the elements that both copies of the seed file share. */
interface SeedIds {
  physicalModelId: Id64String;
  spatialCategoryId: Id64String;
  drawingModelId: Id64String;
  drawingCategoryId: Id64String;
}

/** Ids of the elements that a fully-populated element refers to. */
interface ReferencedIds {
  physicalParentId: Id64String;
  physicalTypeId: Id64String;
  refTargetId: Id64String;
  drawingParentId: Id64String;
  graphicalType2dId: Id64String;
}

describe("ImportContext.cloneElement explicitClears", () => {
  let sourceDb: IModelJsNative.DgnDb;
  let targetDb: IModelJsNative.DgnDb;
  let seed: SeedIds;
  let importContext: IModelJsNative.ImportContext;

  const queryOne = <T>(db: IModelJsNative.DgnDb, ecsql: string, read: (stmt: IModelJsNative.ECSqlStatement) => T, id?: Id64String): T | undefined => {
    const stmt = new iModelJsNative.ECSqlStatement();
    stmt.prepare(db, ecsql);
    try {
      if (undefined !== id)
        stmt.getBinder(1).bindId(id);
      return stmt.step() === DbResult.BE_SQLITE_ROW ? read(stmt) : undefined;
    } finally {
      stmt.dispose();
    }
  };

  const createSeedFile = (): string => {
    const seedFileName = copyFile("CloneElementSeed.bim", dbFileName);
    const db = openDgnDb(seedFileName, { schemaLockHeld: true });
    try {
      db.importXmlSchemas([testSchemaXml], { schemaLockHeld: true });
      const documentListModelId = queryOne(db, "SELECT ECInstanceId FROM bis.DocumentListModel LIMIT 1", (s) => s.getValue(0).getId());
      assert.isDefined(documentListModelId);
      const drawingId = db.insertElement({ classFullName: "BisCore:Drawing", model: documentListModelId, code: Code.createEmpty() });
      db.insertModel({ classFullName: "BisCore:DrawingModel", modeledElement: { id: drawingId } });
      db.saveChanges();
    } finally {
      db.closeFile();
    }
    return seedFileName;
  };

  const openCopy = (seedFileName: string, name: string): IModelJsNative.DgnDb => {
    const fileName = path.join(getOutputDir(), name);
    if (fs.existsSync(fileName))
      fs.unlinkSync(fileName);
    fs.copyFileSync(seedFileName, fileName);
    return openDgnDb(fileName, { schemaLockHeld: true });
  };

  /** Inserts the elements that a fully-populated element refers to. */
  const insertReferencedElements = (db: IModelJsNative.DgnDb): ReferencedIds => ({
    physicalParentId: db.insertElement({ classFullName: "Generic:PhysicalObject", model: seed.physicalModelId, category: seed.spatialCategoryId, code: Code.createEmpty() } as ElementProps),
    physicalTypeId: db.insertElement({ classFullName: "CloneTest:TestPhysicalType", model: IModel.dictionaryId, code: Code.createEmpty() }),
    refTargetId: db.insertElement({ classFullName: "Generic:PhysicalObject", model: seed.physicalModelId, category: seed.spatialCategoryId, code: Code.createEmpty() } as ElementProps),
    drawingParentId: db.insertElement({ classFullName: "BisCore:DrawingGraphic", model: seed.drawingModelId, category: seed.drawingCategoryId, code: Code.createEmpty() } as ElementProps),
    graphicalType2dId: db.insertElement({ classFullName: "CloneTest:TestGraphicalType2d", model: IModel.dictionaryId, code: Code.createEmpty() }),
  });

  const fullPhysicalElementProps = (refs: ReferencedIds): ElementProps => ({
    classFullName: "CloneTest:TestPhysicalElement",
    model: seed.physicalModelId,
    category: seed.spatialCategoryId,
    code: Code.createEmpty(),
    parent: { id: refs.physicalParentId, relClassName: "BisCore:PhysicalElementAssemblesElements" },
    userLabel: "label",
    jsonProperties: { foo: "bar" },
    typeDefinition: { id: refs.physicalTypeId, relClassName: "BisCore:PhysicalElementIsOfType" },
    placement: { origin: [1, 2, 3], angles: { yaw: 10, pitch: 20, roll: 30 } },
    geom: [{ box: { origin: [0, 0, 0], baseX: 1, baseY: 1, height: 1 } }],
    stringProp: "value",
    pointProp: { x: 1, y: 2, z: 3 },
    infoProp: { name: "info", count: 3 },
    tagsProp: ["a", "b"],
    infosProp: [{ name: "first", count: 1 }],
    refProp: { id: refs.refTargetId },
  } as ElementProps);

  const fullDrawingGraphicProps = (refs: ReferencedIds): ElementProps => ({
    classFullName: "BisCore:DrawingGraphic",
    model: seed.drawingModelId,
    category: seed.drawingCategoryId,
    code: Code.createEmpty(),
    parent: { id: refs.drawingParentId, relClassName: "BisCore:ElementOwnsChildElements" },
    userLabel: "label",
    jsonProperties: { foo: "bar" },
    typeDefinition: { id: refs.graphicalType2dId, relClassName: "BisCore:GraphicalElement2dIsOfType" },
    placement: { origin: [1, 2], angle: 30 },
    geom: [{ lineSegment: [[0, 0, 0], [5, 5, 0]] }],
  } as ElementProps);

  /** Clones the source element and updates the mapped target element with the result. */
  const cloneAndUpdate = (sourceId: Id64String, targetId: Id64String, options: IModelJsNative.CloneElementOptions): ElementProps => {
    importContext.addElementId(sourceId, targetId);
    const props = importContext.cloneElement(sourceId, options);
    props.id = targetId;
    targetDb.updateElement(props);
    return props;
  };

  const getElement = (db: IModelJsNative.DgnDb, id: Id64String): any => db.getElement({ id });

  /** Each clone gets a new federationGuid, so leave it out when comparing two clones of the same element. */
  const withoutFederationGuid = (props: ElementProps) => {
    const { federationGuid: _federationGuid, ...rest } = props;
    return rest;
  };

  /** Reads which nullable columns of a geometric element are NULL. */
  const queryNullColumns = (db: IModelJsNative.DgnDb, id: Id64String, is3d: boolean) => {
    const ecsql = is3d
      ? "SELECT GeometryStream, Origin, Yaw, Pitch, Roll, BBoxLow, BBoxHigh FROM bis.GeometricElement3d WHERE ECInstanceId=?"
      : "SELECT GeometryStream, Origin, Rotation, BBoxLow, BBoxHigh FROM bis.GeometricElement2d WHERE ECInstanceId=?";
    const result = queryOne(db, ecsql, (stmt) => {
      const isNull = (i: number) => stmt.getValue(i).isNull();
      return is3d
        ? { geometryStream: isNull(0), origin: isNull(1), angles: isNull(2) && isNull(3) && isNull(4), bbox: isNull(5) && isNull(6) }
        : { geometryStream: isNull(0), origin: isNull(1), angles: isNull(2), bbox: isNull(3) && isNull(4) };
    }, id);
    assert.isDefined(result);
    return result;
  };

  before(() => {
    const seedFileName = createSeedFile();
    sourceDb = openCopy(seedFileName, "CloneElementSource.bim");
    targetDb = openCopy(seedFileName, "CloneElementTarget.bim");

    const physicalModelId = queryOne(sourceDb, "SELECT ECInstanceId FROM bis.PhysicalModel LIMIT 1", (s) => s.getValue(0).getId());
    const spatialCategoryId = queryOne(sourceDb, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1", (s) => s.getValue(0).getId());
    const drawingModelId = queryOne(sourceDb, "SELECT ECInstanceId FROM bis.DrawingModel LIMIT 1", (s) => s.getValue(0).getId());
    const drawingCategoryId = queryOne(sourceDb, "SELECT ECInstanceId FROM bis.DrawingCategory LIMIT 1", (s) => s.getValue(0).getId());
    assert.isDefined(physicalModelId);
    assert.isDefined(spatialCategoryId);
    assert.isDefined(drawingModelId);
    assert.isDefined(drawingCategoryId);
    seed = { physicalModelId, spatialCategoryId, drawingModelId, drawingCategoryId };
  });

  beforeEach(() => {
    importContext = new iModelJsNative.ImportContext(sourceDb, targetDb);
    // Both files are copies of the same seed, so the shared elements and code specs keep their ids.
    for (const id of Object.values(seed))
      importContext.addElementId(id, id);
    importContext.addCodeSpecId("0x1", "0x1");
  });

  afterEach(() => {
    importContext.dispose();
    sourceDb.abandonChanges();
    targetDb.abandonChanges();
  });

  after(() => {
    sourceDb?.closeFile();
    targetDb?.closeFile();
  });

  it("clears the members of a 3d element that are empty in the source", () => {
    const targetId = targetDb.insertElement(fullPhysicalElementProps(insertReferencedElements(targetDb)));
    const sourceId = sourceDb.insertElement({
      classFullName: "CloneTest:TestPhysicalElement",
      model: seed.physicalModelId,
      category: seed.spatialCategoryId,
      code: Code.createEmpty(),
      infoProp: { name: "kept" },
    } as ElementProps);

    const before = getElement(targetDb, targetId);
    for (const member of ["parent", "userLabel", "jsonProperties", "typeDefinition", "placement", "stringProp", "pointProp", "tagsProp", "infosProp", "refProp"])
      expect(before, member).to.have.property(member);
    expect(before.infoProp).to.deep.equal({ name: "info", count: 3 });
    expect(queryNullColumns(targetDb, targetId, true)).to.deep.equal({ geometryStream: false, origin: false, angles: false, bbox: false });

    const props: any = cloneAndUpdate(sourceId, targetId, { explicitClears: true });
    // The markers must be real JS null, not undefined, so that they survive serialization.
    for (const member of ["parent", "userLabel", "typeDefinition", "stringProp", "pointProp", "tagsProp", "infosProp", "refProp"])
      expect(props[member], member).to.be.null;
    expect(props.jsonProperties).to.deep.equal({});
    expect(props.placement).to.deep.equal({});
    expect(props.infoProp).to.deep.equal({ name: "kept", count: null });
    expect(props.geomBinary).to.be.instanceOf(Uint8Array);
    expect(props.geomBinary.length).to.equal(0);

    const after = getElement(targetDb, targetId);
    for (const member of ["parent", "userLabel", "typeDefinition", "placement", "stringProp", "pointProp", "tagsProp", "infosProp", "refProp"])
      expect(after, member).not.to.have.property(member);
    expect(after.jsonProperties ?? {}).to.deep.equal({});
    expect(after.infoProp).to.deep.equal({ name: "kept" });
    expect(queryNullColumns(targetDb, targetId, true)).to.deep.equal({ geometryStream: true, origin: true, angles: true, bbox: true });
  });

  it("clears the members of a 2d element that are empty in the source", () => {
    const targetId = targetDb.insertElement(fullDrawingGraphicProps(insertReferencedElements(targetDb)));
    const sourceId = sourceDb.insertElement({
      classFullName: "BisCore:DrawingGraphic",
      model: seed.drawingModelId,
      category: seed.drawingCategoryId,
      code: Code.createEmpty(),
    } as ElementProps);

    const before = getElement(targetDb, targetId);
    for (const member of ["parent", "userLabel", "jsonProperties", "typeDefinition", "placement"])
      expect(before, member).to.have.property(member);
    expect(queryNullColumns(targetDb, targetId, false)).to.deep.equal({ geometryStream: false, origin: false, angles: false, bbox: false });

    cloneAndUpdate(sourceId, targetId, { explicitClears: true });

    const after = getElement(targetDb, targetId);
    for (const member of ["parent", "userLabel", "typeDefinition", "placement"])
      expect(after, member).not.to.have.property(member);
    expect(after.jsonProperties ?? {}).to.deep.equal({});
    expect(queryNullColumns(targetDb, targetId, false)).to.deep.equal({ geometryStream: true, origin: true, angles: true, bbox: true });
  });

  for (const binaryGeometry of [false, true]) {
    it(`clears geometry but keeps the placement origin (binaryGeometry: ${binaryGeometry})`, () => {
      const targetId = targetDb.insertElement(fullPhysicalElementProps(insertReferencedElements(targetDb)));
      const sourceId = sourceDb.insertElement({
        classFullName: "CloneTest:TestPhysicalElement",
        model: seed.physicalModelId,
        category: seed.spatialCategoryId,
        code: Code.createEmpty(),
        placement: { origin: [4, 5, 6], angles: { yaw: 0, pitch: 0, roll: 0 } },
      } as ElementProps);

      const props: any = cloneAndUpdate(sourceId, targetId, { explicitClears: true, binaryGeometry });
      expect(props).not.to.have.property("geom");
      expect(props.geomBinary.length).to.equal(0);

      expect(queryNullColumns(targetDb, targetId, true)).to.deep.equal({ geometryStream: true, origin: false, angles: false, bbox: true });
      const origin = getElement(targetDb, targetId).placement.origin;
      expect(Array.isArray(origin) ? origin : [origin.x, origin.y, origin.z]).to.deep.equal([4, 5, 6]);
    });
  }

  it("does not change members that have values", () => {
    const sourceRefs = insertReferencedElements(sourceDb);
    const targetRefs = insertReferencedElements(targetDb);
    for (const key of Object.keys(sourceRefs) as (keyof ReferencedIds)[])
      importContext.addElementId(sourceRefs[key], targetRefs[key]);

    for (const sourceId of [sourceDb.insertElement(fullPhysicalElementProps(sourceRefs)), sourceDb.insertElement(fullDrawingGraphicProps(sourceRefs))]) {
      for (const binaryGeometry of [false, true]) {
        const withClears = importContext.cloneElement(sourceId, { binaryGeometry, explicitClears: true });
        const withoutClears = importContext.cloneElement(sourceId, { binaryGeometry });
        expect(withoutFederationGuid(withClears)).to.deep.equal(withoutFederationGuid(withoutClears));
      }
    }
  });

  it("leaves the output unchanged when explicitClears is not set", () => {
    const sourceId = sourceDb.insertElement({
      classFullName: "CloneTest:TestPhysicalElement",
      model: seed.physicalModelId,
      category: seed.spatialCategoryId,
      code: Code.createEmpty(),
    } as ElementProps);

    const props: any = importContext.cloneElement(sourceId);
    expect(withoutFederationGuid(importContext.cloneElement(sourceId, {}))).to.deep.equal(withoutFederationGuid(props));
    expect(withoutFederationGuid(importContext.cloneElement(sourceId, { explicitClears: false }))).to.deep.equal(withoutFederationGuid(props));
    for (const member of ["parent", "userLabel", "jsonProperties", "typeDefinition", "placement", "geom", "geomBinary", "stringProp", "pointProp", "infoProp", "tagsProp", "infosProp", "refProp"])
      expect(props, member).not.to.have.property(member);

    // Without the markers, an update keeps the target's old values.
    const targetId = targetDb.insertElement(fullPhysicalElementProps(insertReferencedElements(targetDb)));
    cloneAndUpdate(sourceId, targetId, {});
    const after = getElement(targetDb, targetId);
    expect(after.userLabel).to.equal("label");
    expect(after.stringProp).to.equal("value");
    expect(Id64.isValidId64(after.parent.id)).to.be.true;
    expect(queryNullColumns(targetDb, targetId, true).geometryStream).to.be.false;
  });
});
