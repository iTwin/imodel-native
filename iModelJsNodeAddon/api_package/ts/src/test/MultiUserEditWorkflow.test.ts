/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert } from "chai";
import { GuidString, Id64Array, Id64String, Logger, LogLevel } from "@itwin/core-bentley";
import { type ModelGeometryChangesProps, ProfileOptions, type RelatedElementProps, type RelationshipProps, type SubjectProps } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName } from "./utils";
import { openDgnDb } from "./index";

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
    // console.log(`_onBeforeOutputsHandled ${this.fmtElem(elClassName,elId)}`);
    this.dres.beforeOutputs.push(elId);
  }
  _onAllInputsHandled(_elClassName: string, elId: Id64String): void {
    assert.isTrue(this.db.isIndirectChanges());
    // console.log(`_onAllInputsHandled ${this.fmtElem(elClassName,elId)}`);
    this.dres.allInputsHandled.push(elId);
  }
  _onRootChanged(props: RelationshipProps): void {
    assert.isTrue(this.db.isIndirectChanges());
    // console.log(`_onRootChanged ${this.fmtRel(props)}`);
    this.dres.rootChanged.push(props);
  }
  _onDeletedDependency(props: RelationshipProps): void {
    assert.isTrue(this.db.isIndirectChanges());
    // console.log(`_onDeletedDependency ${this.fmtRel(props)}`);
    this.dres.deletedDependency.push(props);
  }
  // _onDirectChange(elClassName: string, elId: Id64String): void {
  //   console.log(`_onDirectChange ${this.fmtElem(elClassName,elId)}`);
  //   this.dres.directChange.push(elId);
  // }
  // _onValidateOutput(props: RelationshipProps): void {
  //   console.log(`_onValidateOutput ${this.fmtRel(props)}`);
  //   this.dres.validateOutput.push(props);
  // }
  _onBeginValidate() {
    assert.isFalse(this.db.isIndirectChanges());
    // console.log(`_onBeginValidate`);
  }
  _onEndValidate() {
    assert.isFalse(this.db.isIndirectChanges());
    // console.log(`_onEndValidate`);
  }
  _onGeometryChanged(_modelProps: ModelGeometryChangesProps[]) {
    // console.log(`_onGeometryChanged ${util.inspect(modelProps)}`);
  }
  _onGeometryGuidsChanged(_changes: ModelIdAndGeometryGuid[]): void {
    // console.log(`_onGeometryGuidsChanged ${util.inspect(changes)}`);
  }
  _onCommit() {
    assert.isFalse(this.db.isIndirectChanges());
    // console.log(`_onCommit`);
  }
  _onCommitted() {
    assert.isFalse(this.db.isIndirectChanges());
    // console.log(`_onCommitted`);
  }
  _onChangesApplied() {
    // console.log(`_onChangesApplied`);
  }
  _onBeforeUndoRedo(_isUndo: boolean) {
    // console.log(`_onBeforeUndoRedo ${isUndo}`);
  }
  _onAfterUndoRedo(_isUndo: boolean) {
    // console.log(`_onAfterUndoRedo ${isUndo}`);
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

function _makeEDE(sourceId: Id64String, targetId: Id64String): ElementDrivesElementProps {
  return {
    classFullName: "BisCore:ElementDrivesElement",
    priority: 0,
    status: 0,
    sourceId,
    targetId,
  };
}

function _updateElement(db1: IModelJsNative.DgnDb, elid: Id64String, newLabel: string) {
  const ed2 = db1.getElement({ id: elid });
  ed2.userLabel = newLabel;
  db1.updateElement(ed2);
}

describe.only("multi user edit workflow", () => {

  const seedFileName = "multiUserEditSeed.bim";
  const user1FileName = "multiUserEdit1.bim";
  let db: IModelJsNative.DgnDb;
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
  });

  after(() => {
    db.closeFile();
    db = undefined!;
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

    mockTxn.fmtElem = (_cn: string, id: Id64String) => {
      return db.getElement({ id }).code.value!;
    };

    const subject1Id = db.insertElement(makeSubject("subject1"), { indirect: false});
    assert.isDefined(subject1Id, "Failed to insert element subject1");
    const subjectWithRel1Id = db.insertElement(makeSubject("subjectWithRel1", { id: subject1Id, relClassName: "BisCore.ElementOwnsChildElements" }));
    assert.isDefined(subjectWithRel1Id, "Failed to insert element subjectWithRel1");

    const subject3Id = db.insertElement(makeSubject("subject3"), { indirect: true});
    assert.isDefined(subject3Id, "Failed to insert element subject3");
    db.saveChanges(); // get the elements into the iModel

    /*const ede_1_2 = makeEDE(rel1Id, rel2Id);
    const ede_2_3 = makeEDE(rel2Id, e3id);
    const ede_p2_p3 = makeEDE(subject1Id, subject2Id);
    for (const rel of [ede_1_2, ede_2_3, ede_p2_p3])
      rel.id = db.insertLinkTableRelationship(rel);

    assert.equal(db.addChildPropagatesChangesToParentRelationship("BisCore", "ElementOwnsChildElements"), 0);

    // db.writeFullElementDependencyGraphToFile(writeDbFileName + ".dot");

    // The full graph:
    //     .-parent-> p2 -EDE-> p3
    /     /;
    //  e1 -EDE-> e2 -EDE-> e3
    //
    mockTxn.resetDependencyResults();
    db.saveChanges(); // this will react to EDE inserts only.
    // assert.deepEqual(mockTxn.dres.directChange, []);
    assert.deepEqual(mockTxn.dres.beforeOutputs, []); // only roots get this callback, and only if they have been directly changed.
    assert.deepEqual(mockTxn.dres.allInputsHandled, []); // No input elements have changed
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3, ede_p2_p3]); // we send out this callback even if only the relationship itself is new or changed.
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    updateElement(db, rel1Id, "change e1");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements
    assert.deepEqual(mockTxn.dres.beforeOutputs, [rel1Id]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [rel2Id, subject1Id, e3id, subject2Id]);
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3, ede_p2_p3]);*/
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    // db.writeAffectedElementDependencyGraphToFile(writeDbFileName + ".dot", [e1id]);
  });
});
