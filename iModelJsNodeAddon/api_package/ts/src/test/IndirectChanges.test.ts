/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*
 * PARAMETER EXTRACTION SETUP:
 * 
 * The extractEventArgs function now correctly extracts the indirect flag from:
 * obj.options.indirect (boolean) -> "indirect: true" or "indirect: false"
 * 
 * Expected string format: "Subject:onInsert indirect:false"
 * - Class name is extracted from fullName (e.g., "Subject" from "BisCore:Subject")
 * - Method name comes from the handler method called
 * - Indirect flag comes from the options.indirect property
 */


import { assert } from "chai";
import { DbChangeStage, DbOpcode, DbResult, GuidString, Id64Array, Id64String, Logger, LogLevel } from "@itwin/core-bentley";
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

interface IChange {
  tableName: string;
  op: "updated" | "inserted" | "deleted";
  after: ChangeValueType[] ;
  isIndirect: boolean;
}

function verifyChange(change: IChange, expectedTableName: string, expectedOp: "updated" | "inserted" | "deleted", expectedIndirect: boolean): void {
  assert.equal(change.tableName, expectedTableName, `Expected table name ${expectedTableName}, got ${change.tableName}`);
  assert.equal(change.op, expectedOp, `Expected operation ${expectedOp}, got ${change.op}`);
  assert.equal(change.isIndirect, expectedIndirect, `Expected indirect flag ${expectedIndirect}, got ${change.isIndirect}`);
};

// Helper function to extract parameters from handler method arguments
interface EventArgument {
  props?: {
    classFullName?: string;
    parent?: {
      id?: string;
      relClassName?: string;
    };
  };
  options?: {
    indirect?: boolean;
  };
  iModel?: unknown;
}

function extractEventArgs(arg: unknown): string {
  if (!arg || typeof arg !== 'object') {
    return "indirect: unknown";
  }

  const obj = arg as EventArgument;
  
  const indirect = obj?.options?.indirect !== undefined 
                 ? obj.options.indirect.toString()
                 : "undefined";
  
  return `indirect: ${indirect}`;
}

// Helper function to assert call strings match expected patterns
function assertCallsMatch(expectedPatterns: string[]) {
  const calls = MockCallsTracker.getCallStrings();
  
  assert.equal(calls.length, expectedPatterns.length, 
    `Expected ${expectedPatterns.length} calls, got ${calls.length}. Expected: ${JSON.stringify(expectedPatterns)}, Actual: ${JSON.stringify(calls)}`);
  
  expectedPatterns.forEach((expected, index) => {
    assert.equal(calls[index], expected, `Call ${index + 1} mismatch`);
  });
}

// Static calls cache for tracking all handler method calls
class MockCallsTracker {
  static calls: { method: string; args: unknown[]; fullName: string }[] = [];

  static resetCalls(): void {
    MockCallsTracker.calls = [];
  }

  static addCall(method: string, args: unknown[], fullName: string): void {
    MockCallsTracker.calls.push({ method, args, fullName });
  }

  // Convert calls to string array for easy assertion
  static getCallStrings(): string[] {
    return MockCallsTracker.calls.map(call => {
      const classShortName = call.fullName.split(':')[1] || call.fullName; // Extract "Element" from "BisCore:Element"
      
      // Extract parameters from the args object
      const params = extractEventArgs(call.args[0]);
      return `${classShortName}:${call.method} ${params}`;
    });
  }
}

// Base template with all the handler method names
// To add new handler methods, simply add them to this array - no code duplication needed!
const handlerMethods = [
  'onInsert', 'onInserted', 'onInsertElement', 'onInsertedElement',
  'onUpdate', 'onUpdated', 'onUpdateElement', 'onUpdatedElement', 
  'onDelete', 'onDeleted', 'onDeleteElement', 'onDeletedElement',
  'onChildDelete', 'onChildDeleted', 'onChildInsert', 'onChildInserted',
  'onChildUpdate', 'onChildUpdated', 'onChildAdd', 'onChildAdded',
  'onChildDrop', 'onChildDropped'
];


describe("indirect changes flag", () => {
  const testFileName = "indirectChangesTest.bim";
  let db: IModelJsNative.DgnDb;
  let mockTxn: MockTxn;

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
      handlerMethods.forEach(methodName => {
        dynamicClass[methodName] = (arg: unknown) => {
          MockCallsTracker.addCall(methodName, [arg], fullName);
        };
      });
      
      return dynamicClass;
    };
    const mockJsDb = { txns: mockTxn, getJsClass };
    db.setIModelDb(mockJsDb);
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

  it("insert, update and delete direct and indirect elements", () => {
    Logger.setLevelDefault(LogLevel.Info);
    Logger.setLevel('ECDb', LogLevel.Warning);
    Logger.setLevel('ECObjectsNative', LogLevel.Warning);

    // Helper function to create changeset and verify indirect flag
    const createChangesetAndCollectChanges = (description: string) => {
      assert.equal(DbResult.BE_SQLITE_OK, db.saveChanges(), "Failed to save changes");
      
      const changesetProps = db.startCreateChangeset();
      changesetProps.description = description;
      const index = changesetProps.index;
      const changesetFilePath = copyFile(`${description.replace(/\s+/g, '_')}.changeset`, changesetProps.pathname);
      db.completeCreateChangeset({index});
      assert.isTrue(existsSync(changesetFilePath));

      // Read changeset and verify indirect flag
      const reader = new iModelJsNative.ChangesetReader();
      reader.openFile(changesetFilePath, false);
      const changes: IChange[] = [];
      while (reader.step()) {
        changes.push({
          tableName: reader.getTableName(),
          op: reader.getOpCode() === DbOpcode.Delete ? "deleted" : (reader.getOpCode() === DbOpcode.Update ? "updated" : "inserted"),
          after: reader.getRow(DbChangeStage.New),
          isIndirect: reader.isIndirectChange(),
        });
      }
      reader.close();
      return changes;
    };

    // 1. Create subject directly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    let beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    
    const directSubjectId = db.insertElement(makeSubject("directSubject"), { indirect: false });
    assert.isDefined(directSubjectId, "Failed to insert direct subject");
    
    let endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    
    // Verify handler calls with proper string assertions
    assertCallsMatch([
      "Subject:onInsert indirect: false",
      "RepositoryModel:onInsertElement indirect: false",
      "Subject:onChildInsert indirect: false",
      "Subject:onInserted indirect: undefined", // TODO: needs to be fixed
      "RepositoryModel:onInsertedElement indirect: false",
      "Subject:onChildInserted indirect: false",
    ]);
    MockCallsTracker.resetCalls();
    const changes1 = createChangesetAndCollectChanges("create subject directly");
    assert.equal(changes1.length, 2, "Expected one change in changeset");
    verifyChange(changes1[0], "bis_Element", "inserted", false);
    verifyChange(changes1[1], "bis_InformationReferenceElement", "inserted", false);

    // 2. Create subject indirectly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    
    const indirectSubjectId = db.insertElement(makeSubject("indirectSubject"), { indirect: true });
    assert.isDefined(indirectSubjectId, "Failed to insert indirect subject");
    
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    
    // Verify handler calls with proper string assertions
    assertCallsMatch([
      "Subject:onInsert indirect: true",
      "RepositoryModel:onInsertElement indirect: true",
      "Subject:onChildInsert indirect: true",
      "Subject:onInserted indirect: undefined", // TODO: needs to be fixed
      "RepositoryModel:onInsertedElement indirect: true",
      "Subject:onChildInserted indirect: true",
    ]);
    MockCallsTracker.resetCalls();
    
    const changes2 = createChangesetAndCollectChanges("create subject indirectly");
    assert.equal(changes2.length, 2, "Expected one change in changeset");
    verifyChange(changes2[0], "bis_Element", "inserted", true);
    verifyChange(changes2[1], "bis_InformationReferenceElement", "inserted", true);

    // 3. Update subject directly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    
    const directElement = db.getElement({ id: directSubjectId });
    directElement.userLabel = "updatedDirectly";
    db.updateElement(directElement, { indirect: false });
    
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    
    // Verify handler calls with proper string assertions
    assertCallsMatch([
    "Subject:onUpdate indirect: false",
    "RepositoryModel:onUpdateElement indirect: false",
    "Subject:onChildUpdate indirect: false",
    "Subject:onUpdated indirect: false",
    "RepositoryModel:onUpdatedElement indirect: false",
    "Subject:onChildUpdated indirect: false",
    ]);

    MockCallsTracker.resetCalls();
    
    const changes3 = createChangesetAndCollectChanges("update subject directly");
    verifyChange(changes3[0], "bis_Element", "updated", false);

    // 4. Update subject indirectly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    const indirectElement = db.getElement({ id: indirectSubjectId });
    indirectElement.userLabel = "updatedIndirectly";
    db.updateElement(indirectElement, { indirect: true });
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    // Verify handler calls with proper string assertions
    assertCallsMatch([
      "Subject:onUpdate indirect: true",
      "RepositoryModel:onUpdateElement indirect: true",
      "Subject:onChildUpdate indirect: true",
      "Subject:onUpdated indirect: true",
      "RepositoryModel:onUpdatedElement indirect: true",
      "Subject:onChildUpdated indirect: true",
    ]);
    MockCallsTracker.resetCalls();
    const changes4 = createChangesetAndCollectChanges("update subject indirectly");
    verifyChange(changes4[0], "bis_Element", "updated", true);

    // 5. Delete subject directly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    db.deleteElement(directSubjectId, { indirect: false });
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    // Verify handler calls with proper string assertions
    assertCallsMatch([
      "Subject:onDelete indirect: false",
      "RepositoryModel:onDeleteElement indirect: false",
      "Subject:onChildDelete indirect: false",
      "Subject:onDeleted indirect: false",
      "RepositoryModel:onDeletedElement indirect: false",
      "Subject:onChildDeleted indirect: false",
    ]);
    MockCallsTracker.resetCalls();
    const changes5 = createChangesetAndCollectChanges("delete subject directly");
    verifyChange(changes5[0], "bis_Element", "deleted", false);
    verifyChange(changes5[1], "bis_InformationReferenceElement", "deleted", true); // TODO: clarify, why is this indirect?

    // 6. Delete subject indirectly
    db.deleteAllTxns();
    MockCallsTracker.resetCalls();
    beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult);
    db.deleteElement(indirectSubjectId, { indirect: true });
    endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult);
    // Verify handler calls with proper string assertions
    assertCallsMatch([
      "Subject:onDelete indirect: true",
      "RepositoryModel:onDeleteElement indirect: true",
      "Subject:onChildDelete indirect: true",
      "Subject:onDeleted indirect: true",
      "RepositoryModel:onDeletedElement indirect: true",
      "Subject:onChildDeleted indirect: true",
    ]);
    MockCallsTracker.resetCalls();
    const changes6 = createChangesetAndCollectChanges("delete subject indirectly");
    verifyChange(changes6[0], "bis_Element", "deleted", true);
    verifyChange(changes6[1], "bis_InformationReferenceElement", "deleted", true);
  });
});
