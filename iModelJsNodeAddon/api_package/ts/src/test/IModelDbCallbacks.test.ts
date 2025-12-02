/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert } from "chai";
import { DbChangeStage, DbOpcode, DbResult, GuidString, Id64Array, Id64String } from "@itwin/core-bentley";
import { type ModelGeometryChangesProps, ProfileOptions, type RelationshipProps, type SubjectProps } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, iModelJsNative } from "./utils";
import { openDgnDb } from "./index";
import { existsSync } from "node:fs";

/* eslint-disable @typescript-eslint/explicit-member-accessibility */
/* eslint-disable @typescript-eslint/naming-convention */

export interface ModelIdAndGeometryGuid {
  /** The model's Id. */
  id: Id64String;
  /** A unique identifier for the current state of the model's geometry. If the guid differs between two revisions of the same iModel, it indicates that the geometry differs.
   * This is primarily an implementation detail used to determine whether [Tile]($frontend)s produced for one revision are compatible with another revision.
   */
  guid: GuidString;
}

/**
 * The MockTxn block is needed to enable Txn, while we do not actually do anything with it, we still need to initialize
 */
class DependencyCallbackResults {
  public beforeOutputs: Id64Array = [];
  public allInputsHandled: Id64Array = [];
  public rootChanged: RelationshipProps[] = [];
  public deletedDependency: RelationshipProps[] = [];
}

class MockTxn {
  public dres = new DependencyCallbackResults();
  public db: IModelJsNative.DgnDb;

  constructor(db: IModelJsNative.DgnDb) {
    this.db = db;
  }

  public resetDependencyResults() { this.dres = new DependencyCallbackResults(); }

  _onBeforeOutputsHandled(_elClassName: string, elId: Id64String): void {
    this.dres.beforeOutputs.push(elId);
  }
  _onAllInputsHandled(_elClassName: string, elId: Id64String): void {
    this.dres.allInputsHandled.push(elId);
  }
  _onRootChanged(_props: RelationshipProps): void {
  }
  _onDeletedDependency(_props: RelationshipProps): void {
  }
  _onBeginValidate() {
  }
  _onEndValidate() {
  }
  _onGeometryChanged(_modelProps: ModelGeometryChangesProps[]) {
  }
  _onGeometryGuidsChanged(_changes: ModelIdAndGeometryGuid[]): void {
  }
  _onCommit() {
  }
  _onCommitted() {
  }
  _onChangesApplied() {
  }
  _onBeforeUndoRedo(_isUndo: boolean) {
  }
  _onAfterUndoRedo(_isUndo: boolean) {
  }
}

function makeSubject(codeValue: string): SubjectProps {
  return {
    classFullName: "BisCore:Subject",
    code: { spec: "0x1f", scope: "0x1", value: codeValue },
    model: "0x1",
    parent: { id: "0x1", relClassName: "BisCore:ElementOwnsChildElements" },
  };
}

type ChangeValueType = Uint8Array | number | string | null | undefined;

/**
 * Used to track calls to JS class handler methods
 */
interface IChange {
  tableName: string;
  op: "updated" | "inserted" | "deleted";
  after: ChangeValueType[] ;
}

function verifyChange(change: IChange, expectedTableName: string, expectedOp: "updated" | "inserted" | "deleted"): void {
  assert.equal(change.tableName, expectedTableName, `Expected table name ${expectedTableName}, got ${change.tableName}`);
  assert.equal(change.op, expectedOp, `Expected operation ${expectedOp}, got ${change.op}`);
};

// Helper function to assert call strings match expected patterns
function assertCallsMatch(expectedPatterns: string[]) {
  const calls = HandlerMethodCallsTracker.getCallStrings();
  
  assert.equal(calls.length, expectedPatterns.length, 
    `Expected ${expectedPatterns.length} calls, got ${calls.length}. Expected: ${JSON.stringify(expectedPatterns)}, Actual: ${JSON.stringify(calls)}`);
  
  expectedPatterns.forEach((expected, index) => {
    assert.equal(calls[index], expected, `Call ${index + 1} mismatch`);
  });
}

// Static calls cache for tracking all handler method calls
class HandlerMethodCallsTracker {
  static calls: { method: string; args: unknown[]; fullName: string }[] = [];

  static resetCalls(): void {
    HandlerMethodCallsTracker.calls = [];
  }

  static trackCall(method: string, args: unknown[], fullName: string): void {
    HandlerMethodCallsTracker.calls.push({ method, args, fullName });
  }

  // Convert calls to string array for easy assertion
  static getCallStrings(): string[] {
    return HandlerMethodCallsTracker.calls.map(call => {
      const classShortName = call.fullName.split(':')[1] || call.fullName; // Extract "Element" from "BisCore:Element"
      return `${classShortName}:${call.method}`;
    });
  }
}

// Base template with all the handler method names
const handlerMethodNames = [
  'onInsert', 'onInserted', 'onInsertElement', 'onInsertedElement',
  'onUpdate', 'onUpdated', 'onUpdateElement', 'onUpdatedElement', 
  'onDelete', 'onDeleted', 'onDeleteElement', 'onDeletedElement',
  'onChildDelete', 'onChildDeleted', 'onChildInsert', 'onChildInserted',
  'onChildUpdate', 'onChildUpdated', 'onChildAdd', 'onChildAdded',
  'onChildDrop', 'onChildDropped'
];


describe("Handler method calls during insert/update/delete subject", () => {
  const testFileName = "callbackTests.bim";
  let db: IModelJsNative.DgnDb;
  let mockTxn: MockTxn;
  let mockIModelDb: any;

  before(async() => {
    // create a test file
    const testFilePath = copyFile(testFileName, dbFileName);
    db = openDgnDb(testFilePath, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    assert.isTrue(db !== undefined, "Failed to create test iModel");
    
    // Set up mocks
    mockTxn = new MockTxn(db);
    const getJsClass = (fullName: string) => {
      // Dynamically create a class with all handler methods
      const dynamicClass: any = {};
      
      // Add each handler method to the dynamic class
      handlerMethodNames.forEach(methodName => {
        dynamicClass[methodName] = (arg: unknown) => {
          HandlerMethodCallsTracker.trackCall(methodName, [arg], fullName);
        };
      });
      
      return dynamicClass;
    };

    mockIModelDb = { txns: mockTxn, getJsClass };
    db.setIModelDb(mockIModelDb);
    db.enableTxnTesting();
    db.saveChanges();
  });

  after(() => {
    if (db) {
      db.saveChanges();
      db.closeFile();
      db = undefined!;
    }
  });

  it("insert, update and delete subjects", () => {
    const createChangesetAndCollectChanges = (description: string) => {
      assert.equal(DbResult.BE_SQLITE_OK, db.saveChanges(), "Failed to save changes");
      
      const changesetProps = db.startCreateChangeset();
      changesetProps.description = description;
      const index = changesetProps.index;
      const changesetFilePath = copyFile(`${description.replace(/\s+/g, '_')}.changeset`, changesetProps.pathname);
      db.completeCreateChangeset({index});
      assert.isTrue(existsSync(changesetFilePath));

      const reader = new iModelJsNative.ChangesetReader();
      reader.openFile(changesetFilePath, false);
      const changes: IChange[] = [];
      while (reader.step()) {
        changes.push({
          tableName: reader.getTableName(),
          op: reader.getOpCode() === DbOpcode.Delete ? "deleted" : (reader.getOpCode() === DbOpcode.Update ? "updated" : "inserted"),
          after: reader.getRow(DbChangeStage.New),
        });
      }
      reader.close();
      return changes;
    };

    // 1. Create subject
    db.deleteAllTxns();
    HandlerMethodCallsTracker.resetCalls();
    let beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    
    const directSubjectId = db.insertElement(makeSubject("directSubject"));
    assert.isDefined(directSubjectId, "Failed to insert direct subject");
    
    let endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    
    assertCallsMatch([
      "Subject:onInsert",
      "RepositoryModel:onInsertElement",
      "Subject:onChildInsert",
      "Subject:onInserted",
      "RepositoryModel:onInsertedElement",
      "Subject:onChildInserted",
    ]);
    HandlerMethodCallsTracker.resetCalls();
    const changes1 = createChangesetAndCollectChanges("create subject directly");
    assert.equal(changes1.length, 2, "Expected one change in changeset");
    verifyChange(changes1[0], "bis_Element", "inserted");
    verifyChange(changes1[1], "bis_InformationReferenceElement", "inserted");

    // 2. Update subject
    db.deleteAllTxns();
    HandlerMethodCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    
    const directElement = db.getElement({ id: directSubjectId });
    directElement.userLabel = "updatedDirectly";
    db.updateElement(directElement);
    
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    
    assertCallsMatch([
    "Subject:onUpdate",
    "RepositoryModel:onUpdateElement",
    "Subject:onChildUpdate",
    "Subject:onUpdated",
    "RepositoryModel:onUpdatedElement",
    "Subject:onChildUpdated",
    ]);

    HandlerMethodCallsTracker.resetCalls();
    
    const changes2 = createChangesetAndCollectChanges("update subject");
    verifyChange(changes2[0], "bis_Element", "updated");

    // 3. Delete subject
    db.deleteAllTxns();
    HandlerMethodCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    db.deleteElement(directSubjectId);
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    assertCallsMatch([
      "Subject:onDelete",
      "RepositoryModel:onDeleteElement",
      "Subject:onChildDelete",
      "Subject:onDeleted",
      "RepositoryModel:onDeletedElement",
      "Subject:onChildDeleted",
    ]);
    HandlerMethodCallsTracker.resetCalls();
    const changes3 = createChangesetAndCollectChanges("delete subject directly");
    verifyChange(changes3[0], "bis_Element", "deleted");
    verifyChange(changes3[1], "bis_InformationReferenceElement", "deleted");
  });
});
