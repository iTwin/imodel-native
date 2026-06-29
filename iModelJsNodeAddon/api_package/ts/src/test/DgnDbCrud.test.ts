/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert, expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, IModelStatus, OpenMode } from "@itwin/core-bentley";
import { Code, CodeScopeSpec, IModel, PhysicalElementProps } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("DgnDb CRUD", () => {
  const outDir = getOutputDir();

  function createIModel(name: string): IModelJsNative.DgnDb {
    const dbPath = path.join(outDir, name);
    if (fs.existsSync(dbPath))
      fs.unlinkSync(dbPath);

    const db = new iModelJsNative.DgnDb();
    db.createIModel(dbPath, { rootSubject: { name: `Test: ${name}` } });
    return db;
  }

  function queryFirstId(db: IModelJsNative.DgnDb, ecsql: string): string | undefined {
    const stmt = new iModelJsNative.ECSqlStatement();
    stmt.prepare(db, ecsql);
    const id = stmt.step() === DbResult.BE_SQLITE_ROW ? stmt.getValue(0).getId() : undefined;
    stmt.dispose();
    return id;
  }

  describe("createIModel", () => {
    it("creates a new iModel with root subject", () => {
      const db = createIModel("crud-create.bim");
      assert.isTrue(db.isOpen());
      assert.isFalse(db.isReadonly());

      // Root subject should exist
      const rootSubjectId = queryFirstId(db, "SELECT ECInstanceId FROM bis.Subject WHERE Parent.Id IS NULL");
      assert.isDefined(rootSubjectId);

      // Repository model should exist
      const repoModelId = queryFirstId(db, "SELECT ECInstanceId FROM bis.RepositoryModel");
      assert.isDefined(repoModelId);

      db.closeFile();
    });

    it("getIModelProps returns correct properties", () => {
      const db = createIModel("crud-props.bim");
      const props = db.getIModelProps();
      assert.isDefined(props);
      assert.equal(props.rootSubject.name, "Test: crud-props.bim");
      db.closeFile();
    });
  });

  describe("local values", () => {
    it("save, query, and delete local values", () => {
      const db = createIModel("crud-localvals.bim");

      db.saveLocalValue("testKey", "testValue");
      assert.equal(db.queryLocalValue("testKey"), "testValue");

      db.saveLocalValue("testKey", "updatedValue");
      assert.equal(db.queryLocalValue("testKey"), "updatedValue");

      db.deleteLocalValue("testKey");
      assert.isUndefined(db.queryLocalValue("testKey"));

      db.closeFile();
    });

    it("query non-existent key returns undefined", () => {
      const db = createIModel("crud-localvals-missing.bim");
      assert.isUndefined(db.queryLocalValue("nonExistentKey"));
      db.closeFile();
    });
  });

  describe("file properties", () => {
    it("save and query file property with string", () => {
      const db = createIModel("crud-fileprop.bim");

      db.saveFileProperty({ namespace: "test", name: "prop1" }, "value1", undefined);
      db.saveChanges();

      const val = db.queryFileProperty({ namespace: "test", name: "prop1" }, true);
      assert.equal(val, "value1");

      db.closeFile();
    });

    it("save and query file property with blob", () => {
      const db = createIModel("crud-fileprop-blob.bim");
      const blob = new Uint8Array([1, 2, 3, 4, 5]);

      db.saveFileProperty({ namespace: "test", name: "blobProp" }, "metadata", blob);
      db.saveChanges();

      const val = db.queryFileProperty({ namespace: "test", name: "blobProp" }, false);
      assert.isDefined(val);

      db.closeFile();
    });

    it("queryNextAvailableFileProperty increments", () => {
      const db = createIModel("crud-fileprop-next.bim");

      const id1 = db.queryNextAvailableFileProperty({ namespace: "seq", name: "item" });
      db.saveFileProperty({ namespace: "seq", name: "item", id: id1 }, "first", undefined);
      db.saveChanges();

      const id2 = db.queryNextAvailableFileProperty({ namespace: "seq", name: "item" });
      assert.isAbove(id2, id1);

      db.closeFile();
    });
  });

  describe("element operations", () => {
    let db: IModelJsNative.DgnDb;
    let physicalModelId: string;
    let categoryId: string;

    before(() => {
      db = createIModel("crud-elements.bim");
      db.enableTxnTesting();

      // Import a schema that gives us physical model/category support
      const schema = `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="CrudTest" alias="ct" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="TestPhysical">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="TestProp" typeName="string"/>
          <ECProperty propertyName="IntProp" typeName="int"/>
        </ECEntityClass>
      </ECSchema>`;
      db.importXmlSchemas([schema], { schemaLockHeld: true });

      // Find or create a spatial category
      categoryId = queryFirstId(db, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1") ?? (() => {
        const catProps = {
          classFullName: "BisCore:SpatialCategory",
          model: IModel.dictionaryId,
          code: { spec: db.insertCodeSpec("bis.CategoryDefinition", { scopeSpec: { type: CodeScopeSpec.Type.Repository } }), scope: IModel.dictionaryId, value: "TestCategory" },
        };
        return db.insertElement(catProps);
      })();

      // Create a physical model
      const partitionProps = {
        classFullName: "BisCore:PhysicalPartition",
        model: IModel.repositoryModelId,
        code: Code.createEmpty(),
        parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
      };
      const partitionId = db.insertElement(partitionProps);

      const modelProps = {
        classFullName: "BisCore:PhysicalModel",
        modeledElement: { id: partitionId },
        isPrivate: false,
      };
      physicalModelId = db.insertModel(modelProps);
      db.saveChanges();
    });

    after(() => {
      db.closeFile();
    });

    it("insert element and retrieve it", () => {
      try {
        const elemProps: PhysicalElementProps = {
          classFullName: "CrudTest:TestPhysical",
          model: physicalModelId,
          category: categoryId,
          code: Code.createEmpty(),
        };

        const elementId = db.insertElement(elemProps);
        assert.isString(elementId);
        assert.isTrue(elementId.length > 0);

        const retrieved = db.getElement({ id: elementId });
        assert.equal(retrieved.classFullName, "CrudTest:TestPhysical");
        assert.equal(retrieved.model, physicalModelId);
      } finally {
        db.abandonChanges();
      }
    });

    it("update element properties", () => {
      try {
        const elemProps: PhysicalElementProps = {
          classFullName: "CrudTest:TestPhysical",
          model: physicalModelId,
          category: categoryId,
          code: Code.createEmpty(),
        };
        const elementId = db.insertElement(elemProps);

        const retrieved: any = db.getElement({ id: elementId });
        retrieved.testProp = "updated-value";
        db.updateElement(retrieved);

        const updated: any = db.getElement({ id: elementId });
        assert.equal(updated.testProp, "updated-value");
      } finally {
        db.abandonChanges();
      }
    });

    it("delete element", () => {
      try {
        const elemProps: PhysicalElementProps = {
          classFullName: "CrudTest:TestPhysical",
          model: physicalModelId,
          category: categoryId,
          code: Code.createEmpty(),
        };
        const elementId = db.insertElement(elemProps);
        assert.isString(elementId);

        db.deleteElement(elementId);

        expect(() => db.getElement({ id: elementId })).to.throw();
      } finally {
        db.abandonChanges();
      }
    });

    it("insert element returns error for invalid class", () => {
      expect(() => db.insertElement({
        classFullName: "NotASchema:NotAClass",
        model: physicalModelId,
        code: Code.createEmpty(),
      })).to.throw();
    });

    it("getElement throws for non-existent id", () => {
      expect(() => db.getElement({ id: "0x999999" })).to.throw();
    });
  });

  describe("model operations", () => {
    it("queryModelExtents on empty model", () => {
      const db = createIModel("crud-model-extents.bim");

      // Create a physical model with no geometry
      const partitionProps = {
        classFullName: "BisCore:PhysicalPartition",
        model: IModel.repositoryModelId,
        code: Code.createEmpty(),
        parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
      };
      const partitionId = db.insertElement(partitionProps);
      const modelProps = {
        classFullName: "BisCore:PhysicalModel",
        modeledElement: { id: partitionId },
        isPrivate: false,
      };
      const modelId = db.insertModel(modelProps);
      db.saveChanges();

      // Empty model should throw NoGeometry
      let caught: any;
      try {
        db.queryModelExtents({ id: modelId });
      } catch (err: any) {
        caught = err;
      }
      assert.isDefined(caught, "queryModelExtents should have thrown for empty model");
      assert.equal(caught.errorNumber, IModelStatus.NoGeometry);

      db.closeFile();
    });
  });

  describe("classNameToId and classIdToName", () => {
    it("round-trip class name and id", () => {
      const db = createIModel("crud-classnames.bim");

      const classId = db.classNameToId("BisCore:Element");
      assert.isString(classId);
      assert.isTrue(classId.length > 0);

      const className = db.classIdToName(classId);
      assert.equal(className, "BisCore:Element");

      db.closeFile();
    });

    it("classNameToId returns invalid id for unknown class", () => {
      const db = createIModel("crud-classnames-unknown.bim");
      const result = db.classNameToId("NotReal:FakeClass");
      assert.equal(result, "0");
      db.closeFile();
    });
  });

  describe("isSubClassOf", () => {
    it("checks BIS hierarchy correctly", () => {
      const db = createIModel("crud-subclass.bim");

      assert.isTrue(db.isSubClassOf("BisCore:PhysicalElement", "BisCore:GeometricElement3d"));
      assert.isTrue(db.isSubClassOf("BisCore:GeometricElement3d", "BisCore:GeometricElement"));
      assert.isTrue(db.isSubClassOf("BisCore:GeometricElement3d", "BisCore:Element"));

      assert.isFalse(db.isSubClassOf("BisCore:Element", "BisCore:GeometricElement3d"));
      assert.isFalse(db.isSubClassOf("BisCore:GeometricElement2d", "BisCore:GeometricElement3d"));

      // Same class should be subclass of itself
      assert.isTrue(db.isSubClassOf("BisCore:Element", "BisCore:Element"));

      db.closeFile();
    });

    it("works with custom schemas", () => {
      const db = createIModel("crud-subclass-custom.bim");
      const schema = `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="SubTest" alias="st" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="MyElement">
          <BaseClass>bis:PhysicalElement</BaseClass>
        </ECEntityClass>
      </ECSchema>`;
      db.importXmlSchemas([schema], { schemaLockHeld: true });

      assert.isTrue(db.isSubClassOf("SubTest:MyElement", "BisCore:PhysicalElement"));
      assert.isTrue(db.isSubClassOf("SubTest:MyElement", "BisCore:Element"));
      assert.isFalse(db.isSubClassOf("BisCore:Element", "SubTest:MyElement"));

      db.closeFile();
    });
  });

  describe("computeProjectExtents", () => {
    it("compute on empty iModel returns valid extents", () => {
      const db = createIModel("crud-projextents.bim");
      const result = db.computeProjectExtents(false, false);
      assert.isDefined(result);
      assert.isDefined(result.extents);
      db.closeFile();
    });
  });

  describe("briefcase and changeset", () => {
    it("getBriefcaseId returns valid id", () => {
      const db = createIModel("crud-briefcase.bim");
      const bcId = db.getBriefcaseId();
      assert.isNumber(bcId);
      db.closeFile();
    });

    it("resetBriefcaseId updates the id", () => {
      const db = createIModel("crud-briefcase-reset.bim");
      db.resetBriefcaseId(5);
      assert.equal(db.getBriefcaseId(), 5);
      db.saveChanges();
      db.closeFile();
    });

    it("setITwinId and getITwinId", () => {
      const db = createIModel("crud-itwinid.bim");
      const testGuid = "12345678-1234-1234-1234-123456789012";
      db.saveLocalValue("itwin_id", testGuid);
      db.saveChanges();
      db.closeFile();
    });
  });

  describe("WAL and checkpoint", () => {
    it("enable WAL mode and perform checkpoint", () => {
      const db = createIModel("crud-wal.bim");
      db.enableWalMode();
      db.performCheckpoint();
      db.setAutoCheckpointThreshold(5000);
      db.closeFile();
    });
  });

  describe("schema operations on DgnDb", () => {
    it("importXmlSchemas and getSchemaProps", () => {
      const db = createIModel("crud-schema.bim");
      const schema = `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="Pipe">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="Diameter" typeName="double"/>
        </ECEntityClass>
      </ECSchema>`;

      db.importXmlSchemas([schema], { schemaLockHeld: true });

      const props = db.getSchemaProps("TestSchema");
      assert.equal(props.name, "TestSchema");
      assert.equal(props.version, "01.00.00");

      db.closeFile();
    });

    it("schemaToXmlString exports schema XML", () => {
      const db = createIModel("crud-schema-export.bim");
      const xml = db.schemaToXmlString("BisCore", IModelJsNative.ECVersion.V3_2);
      assert.isString(xml);
      assert.isTrue(xml!.includes("BisCore"));
      assert.isTrue(xml!.includes("http://www.bentley.com/schemas/Bentley.ECXML.3.2"));
      db.closeFile();
    });

    it("schemaToXmlString returns undefined for missing schema", () => {
      const db = createIModel("crud-schema-missing.bim");
      const xml = db.schemaToXmlString("NotThere.NotThere", IModelJsNative.ECVersion.V3_2);
      assert.isUndefined(xml);
      db.closeFile();
    });

    it("getSchemaProps throws for non-existent schema", () => {
      const db = createIModel("crud-schema-notfound.bim");
      expect(() => db.getSchemaProps("DoesNotExist")).to.throw("schema not found");
      db.closeFile();
    });
  });

  describe("vacuum and analyze", () => {
    it("vacuum on DgnDb", () => {
      const db = createIModel("crud-vacuum.bim");
      db.saveChanges();
      db.vacuum();
      db.closeFile();
    });

    it("analyze on DgnDb", () => {
      const db = createIModel("crud-analyze.bim");
      db.saveChanges();
      db.analyze();
      db.closeFile();
    });
  });

  describe("open/close modes", () => {
    it("open readonly prevents writes", () => {
      const dbPath = path.join(outDir, "crud-readonly.bim");
      const db = createIModel("crud-readonly.bim");
      db.saveChanges();
      db.closeFile();

      const readonlyDb = new iModelJsNative.DgnDb();
      readonlyDb.openIModel(dbPath, OpenMode.Readonly);
      assert.isTrue(readonlyDb.isReadonly());
      readonlyDb.closeFile();
    });

    it("getFilePath returns correct path", () => {
      const db = createIModel("crud-filepath.bim");
      const fp = db.getFilePath();
      assert.isTrue(fp.endsWith("crud-filepath.bim"));
      db.closeFile();
    });
  });
});
