/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/


import { assert } from "chai";
import { DbChangeStage, DbOpcode, DbResult, GuidString, Id64Array, Id64String, Logger, LogLevel } from "@itwin/core-bentley";
import { type ModelGeometryChangesProps, ProfileOptions, type RelatedElementProps, type RelationshipProps, type SubjectProps } from "@itwin/core-common";
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
  // public directChange: Id64Array = [];
  // public validateOutput: RelationshipProps[] = [];
}

class MockTxn {
  public dres = new DependencyCallbackResults();

  public db: IModelJsNative.DgnDb;

  constructor(db: IModelJsNative.DgnDb) {
    this.db = db;
  }

  public fmtElem = (elClassName: string, elId: Id64String) => { return `${elClassName}.${elId}`; };
  public fmtRel = (props: RelationshipProps) => { return `${this.fmtElem("", props.sourceId)}->${this.fmtElem("", props.targetId)}`; };

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

function makeSubject(codeValue: string, parent?: RelatedElementProps): SubjectProps {
  return {
    classFullName: "BisCore:Subject",
    code: { spec: "0x1f", scope: "0x1", value: codeValue },
    model: "0x1",
    parent: parent || { id: "0x1", relClassName: "BisCore:ElementOwnsChildElements" },
  };
}

export interface ElementDrivesElementProps extends RelationshipProps {
  status: number;
  priority: number;
}

function _updateElement(db1: IModelJsNative.DgnDb, elid: Id64String, newLabel: string) {
  const ed2 = db1.getElement({ id: elid });
  ed2.userLabel = newLabel;
  db1.updateElement(ed2);
}

type ChangeValueType = Uint8Array | number | string | null | undefined;

interface IChange {
  tableName: string;
  op: "updated" | "inserted" | "deleted";
  after: ChangeValueType[] ;
  isIndirect: boolean;
}

describe.only("multi user edit workflow", () => {
  const seedFileName = "multiUserEditSeed.bim";
  const user1FileName = "multiUserEdit1.bim";
  const user2FileName = "multiUserEdit2.bim";
  const changesetFileName = "multiUserEdit.changeset";
  let db: IModelJsNative.DgnDb;
  let db2: IModelJsNative.DgnDb;
  before(async() => {
    // create a seed file
    const seedFilePath = copyFile(seedFileName, dbFileName);
    db = openDgnDb(seedFilePath, { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
    assert.isTrue(db !== undefined, "Failed to create seed iModel");
    db.saveChanges();
    db.closeFile();

    const user1FilePath = copyFile(user1FileName, seedFilePath);
    db = openDgnDb(user1FilePath);
    assert.isTrue(db !== undefined, "Failed to open iModel");

    const user2FilePath = copyFile(user2FileName, seedFilePath);
    db2 = openDgnDb(user2FilePath);
    assert.isTrue(db2 !== undefined, "Failed to open iModel");
  });

  after(() => {
    db.closeFile();
    db = undefined!;
    db2.closeFile();
    db2 = undefined!;
  });

  it("create direct and indirect elements", () => {
    assert.isTrue(db !== undefined);

    Logger.setLevelDefault(LogLevel.Info);
    Logger.setLevel('ECDb', LogLevel.Warning);
    Logger.setLevel('ECObjectsNative', LogLevel.Warning);

    const mockTxn = new MockTxn(db);
    const mockJsDb = { txns: mockTxn };
    db.setIModelDb(mockJsDb);
    db.enableTxnTesting();

    db.deleteAllTxns();

    const beginResult = db.beginMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, beginResult, "Failed to begin multi user edit operation");
    const txnId = db.getCurrentTxnId();
    assert.isTrue(txnId !== undefined, "Failed to get current transaction id");

    const subject1Id = db.insertElement(makeSubject("subject1"), { indirect: false});
    assert.isDefined(subject1Id, "Failed to insert element subject1");
    const subject2Id = db.insertElement(makeSubject("subjectWithRel1", { id: subject1Id, relClassName: "BisCore.ElementOwnsChildElements" }));
    assert.isDefined(subject2Id, "Failed to insert element subjectWithRel1");

    const subject3Id = db.insertElement(makeSubject("subject3"), { indirect: true});
    assert.isDefined(subject3Id, "Failed to insert element subject3");

    const endResult = db.endMultiTxnOperation();
    assert.equal(DbResult.BE_SQLITE_OK, endResult, "Failed to end multi user edit operation");

    const saveResult = db.saveChanges();
    assert.equal(DbResult.BE_SQLITE_OK, saveResult, "Failed to save changes");
    const changesetProps = db.startCreateChangeset();
    changesetProps.description = "create initial subjects";
    const index = changesetProps.index;
    const changesetFilePath = copyFile(changesetFileName, changesetProps.pathname);
    db.completeCreateChangeset({index});
    assert.isTrue(existsSync(changesetFilePath));
    
    assert.isTrue(db !== undefined);
    const mockTxn2 = new MockTxn(db2);
    const mockJsDb2 = { txns: mockTxn2};
    db2.setIModelDb(mockJsDb2);
    db2.enableTxnTesting();
    db2.deleteAllTxns();
    const changesetPropsCopy = JSON.parse(JSON.stringify(changesetProps));
    changesetPropsCopy.pathname = changesetFilePath;
    db2.applyChangeset(changesetPropsCopy, true);
    const appliedElement = db2.getElement({ id: subject1Id });
    assert.isDefined(appliedElement, "Failed to apply changeset to user2 iModel");
    assert.equal(appliedElement.code.value, "subject1", "Applied element code value does not match expected value");
    const appliedElement2 = db2.getElement({ id: subject2Id });
    assert.isDefined(appliedElement2, "Failed to apply changeset to user2 iModel");
    assert.equal(appliedElement2.code.value, "subjectWithRel1", "Applied element code value does not match expected value");
    const appliedElement3 = db2.getElement({ id: subject3Id });
    assert.isDefined(appliedElement3, "Failed to apply changeset to user2 iModel");
    assert.equal(appliedElement3.code.value, "subject3", "Applied element code value does not match expected value");

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
  });
});
