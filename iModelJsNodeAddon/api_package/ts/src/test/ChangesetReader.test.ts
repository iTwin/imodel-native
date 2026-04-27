/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult } from "@itwin/core-bentley";
import { Code, CodeScopeSpec, IModel, PhysicalElementProps } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("ChangesetReader", () => {
  const outDir = getOutputDir();

  describe("ChangesetReader with EC metadata", () => {
    let db: IModelJsNative.DgnDb;

    before(() => {
      const dbPath = path.join(outDir, "changeset-reader-ec.bim");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      db = new iModelJsNative.DgnDb();
      db.createIModel(dbPath, { rootSubject: { name: "EC Changeset Reader" } });
      db.enableTxnTesting();

      const schema = `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="CrTest" alias="crt" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="Widget">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="WidgetName" typeName="string"/>
          <ECProperty propertyName="Weight" typeName="double"/>
        </ECEntityClass>
      </ECSchema>`;
      db.importXmlSchemas([schema], { schemaLockHeld: true });
      db.saveChanges("import schema");
    });

    after(() => {
      db.closeFile();
    });

    it("openLocalChanges reads uncommitted changes", () => {
      // Create category and model
      const stmt = new iModelJsNative.ECSqlStatement();
      let categoryId = "";
      stmt.prepare(db, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1");
      if (stmt.step() === DbResult.BE_SQLITE_ROW)
        categoryId = stmt.getValue(0).getId();
      stmt.dispose();

      if (!categoryId) {
        categoryId = db.insertElement({
          classFullName: "BisCore:SpatialCategory",
          model: IModel.dictionaryId,
          code: { spec: db.insertCodeSpec("bis.CategoryDefinition", { scopeSpec: { type: CodeScopeSpec.Type.Repository } }), scope: IModel.dictionaryId, value: "CrCategory" },
        });
      }

      const partitionId = db.insertElement({
        classFullName: "BisCore:PhysicalPartition",
        model: IModel.repositoryModelId,
        code: Code.createEmpty(),
        parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
      });
      const modelId = db.insertModel({
        classFullName: "BisCore:PhysicalModel",
        modeledElement: { id: partitionId },
        isPrivate: false,
      });

      // Insert an element
      const elemProps: PhysicalElementProps = {
        classFullName: "CrTest:Widget",
        model: modelId,
        category: categoryId,
        code: Code.createEmpty(),
      };
      (elemProps as any).widgetName = "TestWidget";
      (elemProps as any).weight = 42.5;
      db.insertElement(elemProps);
      db.saveChanges("insert widget");

      // Read the changes using the ChangesetReader
      const reader = new iModelJsNative.ChangesetReader();
      reader.openLocalChanges(db, true, false, 0);

      let hasChanges = false;
      while (reader.step()) {
        hasChanges = true;
        const metadata = reader.getChangeMetadata();
        assert.isDefined(metadata);
      }
      reader.close();

      assert.isTrue(hasChanges, "should have found changes");

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });

    it("openTxn reads changes from a specific transaction", () => {
      const partitionId = db.insertElement({
        classFullName: "BisCore:PhysicalPartition",
        model: IModel.repositoryModelId,
        code: Code.createEmpty(),
        parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
      });
      const modelId = db.insertModel({
        classFullName: "BisCore:PhysicalModel",
        modeledElement: { id: partitionId },
        isPrivate: false,
      });

      let categoryId = "";
      const catStmt = new iModelJsNative.ECSqlStatement();
      catStmt.prepare(db, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1");
      if (catStmt.step() === DbResult.BE_SQLITE_ROW)
        categoryId = catStmt.getValue(0).getId();
      catStmt.dispose();

      if (!categoryId) {
        categoryId = db.insertElement({
          classFullName: "BisCore:SpatialCategory",
          model: IModel.dictionaryId,
          code: { spec: db.insertCodeSpec("bis.CategoryDefinition", { scopeSpec: { type: CodeScopeSpec.Type.Repository } }), scope: IModel.dictionaryId, value: "TxnCat" },
        });
      }

      db.insertElement({
        classFullName: "CrTest:Widget",
        model: modelId,
        category: categoryId,
        code: Code.createEmpty(),
      } as any);

      db.saveChanges("insert for txn reader");
      const txnId = db.queryPreviousTxnId(db.getCurrentTxnId());

      const reader = new iModelJsNative.ChangesetReader();
      reader.openTxn(db, txnId, false, 0);

      let changeCount = 0;
      while (reader.step())
        changeCount++;
      reader.close();

      assert.isAbove(changeCount, 0);

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("ChangesetReader filters", () => {
    let db: IModelJsNative.DgnDb;

    before(() => {
      const dbPath = path.join(outDir, "changeset-filter.bim");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      db = new iModelJsNative.DgnDb();
      db.createIModel(dbPath, { rootSubject: { name: "Filter Tests" } });
      db.enableTxnTesting();

      const schema = `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="FilterTest" alias="ft" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="TypeA">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="AName" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="TypeB">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="BName" typeName="string"/>
        </ECEntityClass>
      </ECSchema>`;
      db.importXmlSchemas([schema], { schemaLockHeld: true });
      db.saveChanges("import schema");
    });

    after(() => {
      db.closeFile();
    });

    it("setClassNameFilters limits results to specific classes", () => {
      let categoryId = "";
      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1");
      if (stmt.step() === DbResult.BE_SQLITE_ROW)
        categoryId = stmt.getValue(0).getId();
      stmt.dispose();

      if (!categoryId) {
        categoryId = db.insertElement({
          classFullName: "BisCore:SpatialCategory",
          model: IModel.dictionaryId,
          code: { spec: db.insertCodeSpec("bis.CategoryDefinition", { scopeSpec: { type: CodeScopeSpec.Type.Repository } }), scope: IModel.dictionaryId, value: "FilterCat" },
        });
      }

      const partitionId = db.insertElement({
        classFullName: "BisCore:PhysicalPartition",
        model: IModel.repositoryModelId,
        code: Code.createEmpty(),
        parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
      });
      const modelId = db.insertModel({
        classFullName: "BisCore:PhysicalModel",
        modeledElement: { id: partitionId },
        isPrivate: false,
      });

      // Insert both types
      db.insertElement({
        classFullName: "FilterTest:TypeA",
        model: modelId,
        category: categoryId,
        code: Code.createEmpty(),
      } as any);
      db.insertElement({
        classFullName: "FilterTest:TypeB",
        model: modelId,
        category: categoryId,
        code: Code.createEmpty(),
      } as any);
      db.saveChanges("insert mixed");

      // Read all changes
      const allReader = new iModelJsNative.ChangesetReader();
      allReader.openLocalChanges(db, true, false, 0);
      let allCount = 0;
      while (allReader.step())
        allCount++;
      allReader.close();

      // Read with class name filter
      const filteredReader = new iModelJsNative.ChangesetReader();
      filteredReader.openLocalChanges(db, true, false, 0);
      filteredReader.setClassNameFilters(["FilterTest:TypeA"]);
      let filteredCount = 0;
      while (filteredReader.step())
        filteredCount++;
      filteredReader.close();

      assert.isAbove(allCount, filteredCount, "filtered should have fewer changes");

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("SqliteChangesetReader writeToFile", () => {
    it("can write changeset data to a new file", () => {
      const dbPath = path.join(outDir, "cs-write.bim");
      if (fs.existsSync(dbPath))
        fs.unlinkSync(dbPath);

      const db = new iModelJsNative.DgnDb();
      db.createIModel(dbPath, { rootSubject: { name: "Write CS" } });
      db.enableTxnTesting();

      db.saveFileProperty({ namespace: "test", name: "prop" }, "value", undefined);
      db.saveChanges("add prop");

      // Read local changes via SqliteChangesetReader and write to file
      const reader = new iModelJsNative.SqliteChangesetReader();
      reader.openLocalChanges(db, false, false);

      const outCsPath = path.join(outDir, "written-changeset.cs");
      if (fs.existsSync(outCsPath))
        fs.unlinkSync(outCsPath);

      reader.writeToFile(outCsPath, false, true);
      reader.close();

      assert.isTrue(fs.existsSync(outCsPath));
      assert.isAbove(fs.statSync(outCsPath).size, 0);

      // Verify the written file can be read back
      const verifyReader = new iModelJsNative.SqliteChangesetReader();
      verifyReader.openFile(outCsPath, false);
      let count = 0;
      while (verifyReader.step())
        count++;
      verifyReader.close();
      assert.isAbove(count, 0);

      db.closeFile();

      // Cleanup
      if (fs.existsSync(outCsPath))
        fs.unlinkSync(outCsPath);
    });
  });
});
