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

describe("Transactions", () => {
  const outDir = getOutputDir();
  let db: IModelJsNative.DgnDb;
  let physicalModelId: string;
  let categoryId: string;

  function queryFirstId(ecsql: string): string | undefined {
    const stmt = new iModelJsNative.ECSqlStatement();
    stmt.prepare(db, ecsql);
    const id = stmt.step() === DbResult.BE_SQLITE_ROW ? stmt.getValue(0).getId() : undefined;
    stmt.dispose();
    return id;
  }

  before(() => {
    const dbPath = path.join(outDir, "transactions.bim");
    if (fs.existsSync(dbPath))
      fs.unlinkSync(dbPath);

    db = new iModelJsNative.DgnDb();
    db.createIModel(dbPath, { rootSubject: { name: "Transaction Tests" } });
    db.enableTxnTesting();

    const schema = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TxnTest" alias="tt" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
      <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
      <ECEntityClass typeName="TxnElement">
        <BaseClass>bis:PhysicalElement</BaseClass>
        <ECProperty propertyName="Label" typeName="string"/>
      </ECEntityClass>
    </ECSchema>`;
    db.importXmlSchemas([schema], { schemaLockHeld: true });

    // Create category
    categoryId = queryFirstId("SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1") ?? (() => {
      return db.insertElement({
        classFullName: "BisCore:SpatialCategory",
        model: IModel.dictionaryId,
        code: { spec: db.insertCodeSpec("bis.CategoryDefinition", { scopeSpec: { type: CodeScopeSpec.Type.Repository } }), scope: IModel.dictionaryId, value: "TxnCategory" },
      });
    })();

    // Create physical model
    const partitionId = db.insertElement({
      classFullName: "BisCore:PhysicalPartition",
      model: IModel.repositoryModelId,
      code: Code.createEmpty(),
      parent: { id: "0x1", relClassName: "BisCore:SubjectOwnsPartitionElements" },
    });
    physicalModelId = db.insertModel({
      classFullName: "BisCore:PhysicalModel",
      modeledElement: { id: partitionId },
      isPrivate: false,
    });

    db.saveChanges("setup");
    db.restartTxnSession();
  });

  after(() => {
    db.closeFile();
  });

  function insertTestElement(label: string): string {
    const props: PhysicalElementProps = {
      classFullName: "TxnTest:TxnElement",
      model: physicalModelId,
      category: categoryId,
      code: Code.createEmpty(),
    };
    (props as any).label = label;
    return db.insertElement(props);
  }

  function elementExists(id: string): boolean {
    try {
      db.getElement({ id });
      return true;
    } catch {
      return false;
    }
  }

  describe("saveChanges and abandonChanges", () => {
    it("saveChanges persists element", () => {
      const id = insertTestElement("persisted");
      db.saveChanges("insert persisted element");
      assert.isTrue(elementExists(id));

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });

    it("abandonChanges discards element", () => {
      const id = insertTestElement("discarded");
      assert.isTrue(elementExists(id));
      db.abandonChanges();
      assert.isFalse(elementExists(id));
    });
  });

  describe("hasPendingTxns and isUndoPossible", () => {
    it("isUndoPossible returns false when no session changes", () => {
      assert.isFalse(db.isUndoPossible());
    });

    it("returns true after element insert", () => {
      insertTestElement("pending");
      db.saveChanges("pending txn");
      assert.isTrue(db.isUndoPossible());

      // Cleanup: reverse pending changes
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("undo and redo", () => {
    it("reverseTxns undoes the last change", () => {
      const id = insertTestElement("to-undo");
      db.saveChanges("insert to-undo");
      assert.isTrue(elementExists(id));
      assert.isTrue(db.isUndoPossible());

      const undoResult = db.reverseTxns(1);
      assert.equal(undoResult, 0); // IModelStatus.Success
      assert.isFalse(elementExists(id));

      // Redo should be possible
      assert.isTrue(db.isRedoPossible());
      const redoResult = db.reinstateTxn();
      assert.equal(redoResult, 0); // IModelStatus.Success
      assert.isTrue(elementExists(id));

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });

    it("reverseAll undoes all pending changes", () => {
      insertTestElement("all-1");
      db.saveChanges("insert all-1");
      insertTestElement("all-2");
      db.saveChanges("insert all-2");

      assert.isTrue(db.isUndoPossible());
      db.reverseAll();
      assert.isFalse(db.isUndoPossible());

      db.saveChanges();
    });

    it("getUndoString and getRedoString return descriptions", () => {
      insertTestElement("undo-desc");
      db.saveChanges("my description");

      const undoStr = db.getUndoString();
      assert.isString(undoStr);
      assert.equal(undoStr, "my description");

      db.reverseTxns(1);
      const redoStr = db.getRedoString();
      assert.isString(redoStr);
      assert.equal(redoStr, "my description");

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("getCurrentTxnId", () => {
    it("returns a txn id after save", () => {
      insertTestElement("txn-id-test");
      db.saveChanges("txn-id-test");

      const txnId = db.getCurrentTxnId();
      assert.isString(txnId);
      assert.isTrue(txnId.length > 0);

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("multi-txn operations", () => {
    it("beginMultiTxnOperation groups changes as single undo", () => {
      db.beginMultiTxnOperation();

      insertTestElement("multi-1");
      db.saveChanges("multi-1");
      insertTestElement("multi-2");
      db.saveChanges("multi-2");

      db.endMultiTxnOperation();

      // Should undo both in one reverseTxns(1)
      assert.isTrue(db.isUndoPossible());
      db.reverseTxns(1);
      assert.isFalse(db.isUndoPossible());

      db.saveChanges();
    });
  });

  describe("transaction session", () => {
    it("restartTxnSession starts a new session", () => {
      insertTestElement("session-test");
      db.saveChanges("session-test");
      assert.isTrue(db.isUndoPossible());

      db.restartTxnSession();
      // After restart, pending txns from previous session should be gone from undo stack
      assert.isFalse(db.isUndoPossible());
    });
  });

  describe("cancelTo", () => {
    it("cancel to a specific txn id", () => {
      const _beforeId = db.getCurrentTxnId();

      insertTestElement("cancel-1");
      db.saveChanges("cancel-1");
      const afterFirstId = db.getCurrentTxnId();

      insertTestElement("cancel-2");
      db.saveChanges("cancel-2");

      // Cancel back to after-first state
      db.cancelTo(afterFirstId);
      // The second insert should be undone
      assert.equal(db.getCurrentTxnId(), afterFirstId);

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });

  describe("getTxnDescription", () => {
    it("returns description for a txn", () => {
      insertTestElement("desc-test");
      const txnId = db.getCurrentTxnId();
      db.saveChanges("a specific description");
      const desc = db.getTxnDescription(txnId);
      assert.isString(desc);
      assert.equal(desc, "a specific description");

      // Cleanup
      db.reverseAll();
      db.saveChanges();
    });
  });
});
