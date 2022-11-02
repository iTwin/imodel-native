/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { assert } from "chai";
import { Guid, GuidString, Id64Array, Id64String } from "@itwin/core-bentley";
import { ModelGeometryChangesProps, RelatedElementProps, RelationshipProps, SubjectProps } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName } from "./utils";
import { openDgnDb } from "./index";

export interface ModelIdAndGeometryGuid {
  /** The model's Id. */
  id: Id64String;
  /** A unique identifier for the current state of the model's geometry. If the guid differs between two revisions of the same iModel, it indicates that the geometry differs.
   * This is primarily an implementation detail used to determine whether [Tile]($frontend)s produced for one revision are compatible with another revision.
   */
  guid: GuidString;
}

let db: IModelJsNative.DgnDb;

class DependencyCallbackResults {
  public beforeOutputs: Id64Array = [];
  public allInputsHandled: Id64Array = [];
  public rootChanged: RelationshipProps[] = [];
  public deletedDependency: RelationshipProps[] = [];
  // public directChange: Id64Array = [];
  // public validateOutput: RelationshipProps[] = [];
};

class MockTxn {
  public dres = new DependencyCallbackResults();

  public fmtElem = (elClassName: string, elId: Id64String) => { return elClassName + "." + elId; };
  public fmtRel = (props: RelationshipProps) => { return this.fmtElem("", props.sourceId) + "->" + this.fmtElem("", props.targetId); }

  public resetDependencyResults() { this.dres = new DependencyCallbackResults(); }

  _onBeforeOutputsHandled(_elClassName: string, elId: Id64String): void {
    // console.log(`_onBeforeOutputsHandled ${this.fmtElem(elClassName,elId)}`);
    this.dres.beforeOutputs.push(elId);
  }
  _onAllInputsHandled(_elClassName: string, elId: Id64String): void {
    assert.isTrue(db.isIndirectChanges());
    // console.log(`_onAllInputsHandled ${this.fmtElem(elClassName,elId)}`);
    this.dres.allInputsHandled.push(elId);
  }
  _onRootChanged(props: RelationshipProps): void {
    assert.isTrue(db.isIndirectChanges());
    // console.log(`_onRootChanged ${this.fmtRel(props)}`);
    this.dres.rootChanged.push(props);
  }
  _onDeletedDependency(props: RelationshipProps): void {
    assert.isTrue(db.isIndirectChanges());
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
    assert.isFalse(db.isIndirectChanges());
    // console.log(`_onBeginValidate`);
  }
  _onEndValidate() {
    assert.isFalse(db.isIndirectChanges());
    // console.log(`_onEndValidate`);
  }
  _onGeometryChanged(_modelProps: ModelGeometryChangesProps[]) {
    // console.log(`_onGeometryChanged ${util.inspect(modelProps)}`);
  }
  _onGeometryGuidsChanged(_changes: ModelIdAndGeometryGuid[]): void {
    // console.log(`_onGeometryGuidsChanged ${util.inspect(changes)}`);
  }
  _onCommit() {
    assert.isFalse(db.isIndirectChanges());
    // console.log(`_onCommit`);
  }
  _onCommitted() {
    assert.isFalse(db.isIndirectChanges());
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
    classFullName: 'BisCore:Subject',
    code: { spec: '0x1f', scope: '0x1', value: codeValue },
    model: '0x1',
    parent: parent || { id: '0x1', relClassName: 'BisCore:ElementOwnsChildElements' },
  };
}

export interface ElementDrivesElementProps extends RelationshipProps {
  status: number;
  priority: number;
}

function makeEDE(sourceId: Id64String, targetId: Id64String): ElementDrivesElementProps {
  return {
    classFullName: 'BisCore:ElementDrivesElement',
    priority: 0,
    status: 0,
    sourceId,
    targetId,
  };
}

function updateElement(db: IModelJsNative.DgnDb, elid: Id64String, newLabel: string) {
  const ed2 = db.getElement({ id: elid });
  ed2.userLabel = newLabel;
  db.updateElement(ed2);
}

function assertRels(list: RelationshipProps[], rels: ElementDrivesElementProps[]) {
  assert.equal(list.length, rels.length);
  for (let i = 0; i < rels.length; ++i) {
    assert.equal(list[i].id, rels[i].id);
  }
}

describe("elementDependency", () => {
  it("should invokeCallbacks through parents", () => {
    const writeDbFileName = copyFile("elementDependencyThroughParents.bim", dbFileName);
    db = openDgnDb(writeDbFileName);
    assert.isTrue(db !== undefined);

    // Logger.setLevelDefault(LogLevel.Info);
    // Logger.setLevel('ElementDependencyGraph', LogLevel.Trace);
    // Logger.setLevel('ECObjectsNative', LogLevel.Error);

    const mockTxn = new MockTxn();
    const mockJsDb = { txns: mockTxn };
    db.setIModelDb(mockJsDb);

    db.enableTxnTesting();

    mockTxn.fmtElem = (_cn: string, id: Id64String) => { return db.getElement({ id }).code.value!; };

    const p2id = db.insertElement(makeSubject("p2"));
    const p3id = db.insertElement(makeSubject("p3"));
    const e1id = db.insertElement(makeSubject("e1", { id: p2id, relClassName: "BisCore.ElementOwnsChildElements" }));
    const e2id = db.insertElement(makeSubject("e2"));
    const e3id = db.insertElement(makeSubject("e3"));
    db.saveChanges(); // get the elements into the iModel

    const ede_1_2 = makeEDE(e1id, e2id);
    const ede_2_3 = makeEDE(e2id, e3id);
    const ede_p2_p3 = makeEDE(p2id, p3id);
    for (const rel of [ede_1_2, ede_2_3, ede_p2_p3])
      rel.id = db.insertLinkTableRelationship(rel);

    assert.equal(db.addChildPropagatesChangesToParentRelationship("BisCore", "ElementOwnsChildElements"), 0);

    // db.writeFullElementDependencyGraphToFile(writeDbFileName + ".dot");

    // The full graph:
    //     .-parent-> p2 -EDE-> p3
    /     /
    //  e1 -EDE-> e2 -EDE-> e3
    //
    mockTxn.resetDependencyResults();
    db.saveChanges(); // this will react to EDE inserts only.
    // assert.deepEqual(mockTxn.dres.directChange, []);
    assert.deepEqual(mockTxn.dres.beforeOutputs, []); // only roots get this callback, and only if they have been directly changed.
    assert.deepEqual(mockTxn.dres.allInputsHandled, []); // No input elements have changed
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3, ede_p2_p3]); // we send out this callback even if only the relationship itself is new or changed.
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    updateElement(db, e1id, "change e1");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e1id]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e2id, p2id, e3id, p3id]);
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3, ede_p2_p3]);
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    // db.writeAffectedElementDependencyGraphToFile(writeDbFileName + ".dot", [e1id]);
  });

  it("should invokeCallbacks through parents - geomodeler schema", () => {
    const writeDbFileName = copyFile("elementDependencyThroughParentsGM.bim", dbFileName);
    db = openDgnDb(writeDbFileName);
    assert.isTrue(db !== undefined);

    const mockTxn = new MockTxn();
    const mockJsDb = { txns: mockTxn };
    db.setIModelDb(mockJsDb);

    db.enableTxnTesting();

    mockTxn.fmtElem = (_cn: string, id: Id64String) => { return db.getElement({ id }).code.value!; };

    // The full graph:
    //                                        BoreholeSource -EDE-> GroundGeneration
    //                                        / parent
    //                                  Borehole
    //                                  / parent
    // Material -EDE-> MaterialDepthRange
    //

    const boreholeSource = db.insertElement(makeSubject("BoreholeSource"));
    const borehole = db.insertElement(makeSubject("Borehole", { id: boreholeSource, relClassName: "BisCore.ElementOwnsChildElements" }));
    const materialDepthRange = db.insertElement(makeSubject("MaterialDepthRange", { id: borehole, relClassName: "BisCore.ElementOwnsChildElements" }));
    const material = db.insertElement(makeSubject("Material"));
    const groundGeneration = db.insertElement(makeSubject("GroundGeneration"));
    db.saveChanges(); // get the elements into the iModel

    const ede_material_materialDepthRange = makeEDE(material, materialDepthRange);
    const ede_boreholeSource_groundGeneration = makeEDE(boreholeSource, groundGeneration);
    for (const rel of [ede_material_materialDepthRange, ede_boreholeSource_groundGeneration])
      rel.id = db.insertLinkTableRelationship(rel);

    assert.equal(db.addChildPropagatesChangesToParentRelationship("BisCore", "ElementOwnsChildElements"), 0);

    // db.writeFullElementDependencyGraphToFile(writeDbFileName + ".dot");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    updateElement(db, material, "change material");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements
    assert.deepEqual(mockTxn.dres.beforeOutputs, [material]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [materialDepthRange, borehole, boreholeSource, groundGeneration]);
    assertRels(mockTxn.dres.rootChanged, [ede_material_materialDepthRange, ede_boreholeSource_groundGeneration]);
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    // db.writeAffectedElementDependencyGraphToFile(writeDbFileName + ".dot", [material]);
  });

  it("should invokeCallbacks 1-2-3", () => {
    const writeDbFileName = copyFile("elementDependency123.bim", dbFileName);
    db = openDgnDb(writeDbFileName);
    assert.isTrue(db !== undefined);

    const mockTxn = new MockTxn();
    const mockJsDb = { txns: mockTxn };
    db.setIModelDb(mockJsDb);

    db.enableTxnTesting();

    mockTxn.fmtElem = (_cn: string, id: Id64String) => { return db.getElement({ id }).code.value!; };

    const e1id = db.insertElement(makeSubject("e1"));
    const e2id = db.insertElement(makeSubject("e2"));
    const e3id = db.insertElement(makeSubject("e3"));
    db.saveChanges(); // get the elements into the iModel

    const ede_1_2 = makeEDE(e1id, e2id);
    const ede_2_3 = makeEDE(e2id, e3id);
    for (const rel of [ede_1_2, ede_2_3])
      rel.id = db.insertLinkTableRelationship(rel);

    // The full graph:
    //  e1 --> e2 --> e3

    mockTxn.resetDependencyResults();
    db.saveChanges(); // this will react to EDE inserts only.
    // assert.deepEqual(mockTxn.dres.directChange, []);
    assert.deepEqual(mockTxn.dres.beforeOutputs, []); // only roots get this callback, and only if they have been directly changed.
    assert.deepEqual(mockTxn.dres.allInputsHandled, []); // No input elements have changed
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3]); // we send out this callback even if only the relationship itself is new or changed.
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed

    updateElement(db, e1id, "change e1");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e1id]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e2id, e3id]);
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3]);
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that share an output with another rel or whose output is directly changed
  });

  it("should invokeCallbacks", () => {
    const writeDbFileName = copyFile("elementDependency.bim", dbFileName);
    db = openDgnDb(writeDbFileName);
    assert.isTrue(db !== undefined);

    const mockTxn = new MockTxn();
    const mockJsDb = { txns: mockTxn };
    db.setIModelDb(mockJsDb);

    db.enableTxnTesting();

    mockTxn.fmtElem = (_cn: string, id: Id64String) => { return db.getElement({ id }).code.value!; };

    const e1id = db.insertElement(makeSubject("e1"));
    const e1 = db.getElement({ id: e1id });
    assert.isTrue(Guid.isGuid(e1.federationGuid!)); // if you don't supply a federationGuid, one is created

    const e11props = makeSubject("e11");
    e11props.federationGuid = Guid.empty;
    const e11id = db.insertElement(e11props);
    const e11 = db.getElement({ id: e11id });
    assert.isUndefined(e11.federationGuid); // if you supply empty federationGuid, it is saved as null and returned as undefined

    const e2props = makeSubject("e2");
    e2props.federationGuid = Guid.createValue();
    const e2id = db.insertElement(e2props);
    const e2 = db.getElement({ id: e2id });
    assert.strictEqual(e2.federationGuid, e2props.federationGuid); // you may supply your own federationGuid

    const e21props = makeSubject("e21");
    e21props.federationGuid = "bad guid";
    const e21id = db.insertElement(e21props);
    const e21 = db.getElement({ id: e21id });
    assert.isUndefined(e21.federationGuid); // if you supply an invalid federationGuid, it is saved as null and returned as undefined

    const e3props = makeSubject("e3");
    e3props.federationGuid = undefined;
    const e3id = db.insertElement(e3props);
    const e3 = db.getElement({ id: e3id });
    assert.isTrue(Guid.isGuid(e3.federationGuid!)); // if you supply "undefined" for federationGuid, one is created.

    const e4props = makeSubject("e4");
    e4props.federationGuid = "";
    const e4id = db.insertElement(e4props);
    const e4 = db.getElement({ id: e4id });
    assert.isUndefined(e4.federationGuid); // if you supply an blank federationGuid, it is saved as null and returned as undefined

    const ede_1_2 = makeEDE(e1id, e2id);
    const ede_11_2 = makeEDE(e11id, e2id);
    const ede_2_3 = makeEDE(e2id, e3id);
    const ede_21_3 = makeEDE(e21id, e3id);
    const ede_3_4 = makeEDE(e3id, e4id);
    for (const rel of [ede_1_2, ede_11_2, ede_2_3, ede_21_3, ede_3_4])
      rel.id = db.insertLinkTableRelationship(rel);

    // The full graph:
    //        e21
    //            \
    //  e1 --> e2 --> e3 --> e4
    //      /
    //  e11

    // On the very first validation, everything is new and is considered directly changed
    // resulting graph:
    //        e21
    //            \
    //  e1 --> e2 --> e3 --> e4
    //      /
    //  e11
    mockTxn.resetDependencyResults();
    db.saveChanges();
    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements that have no directly changed inputs
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e1id, e11id, e21id]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e2id, e3id, e4id]);
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_11_2, ede_2_3, ede_21_3, ede_3_4]);
    // assertRels(mockTxn.dres.validateOutput, []); // this callback is made only on rels that not in the graph but share an output with another rel or have an output that was directly changed

    // modify e4 directly. That is a leaf. None of its inputs are changed.
    // resulting subgraph:
    //                      *
    //                      e4
    //
    //
    updateElement(db, e4id, "change e4");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, [e4id]); // only called on directly changed non-root elements
    assert.deepEqual(mockTxn.dres.beforeOutputs, []); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, []);
    assertRels(mockTxn.dres.rootChanged, []);
    // assertRels(mockTxn.dres.validateOutput, [ede_3_4]);

    // modify e3 directly.
    // resulting subgraph:
    //
    //
    //               *
    //               e3 --> e4
    //
    //
    updateElement(db, e3id, "change e3");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, [e3id]); // only called on directly changed non-root elements.
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e3id]); // only called on directly changed root elements.
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e4id]);
    assertRels(mockTxn.dres.rootChanged, [ede_3_4]);
    // assertRels(mockTxn.dres.validateOutput, [ede_2_3, ede_21_3]); // this callback is made only on rels that not in the graph but share an output with another rel or have an output that was directly changed

    // modify e2 directly. That is a node in middle of the graph. None of its inputs is modified.
    // resulting subgraph:
    //
    //         *
    //         e2 --> e3 --> e4
    //
    //
    //
    updateElement(db, e2id, "change e2");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, [e2id]); // only called on directly changed non-root elements that have no directly changed inputs
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e2id]); // only called on directly changed root elements
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e3id, e4id],);
    assertRels(mockTxn.dres.rootChanged, [ede_2_3, ede_3_4]);
    // assertRels(mockTxn.dres.validateOutput, [ede_1_2, ede_11_2, ede_21_3]); // this callback is made only on rels that not in the graph but share an output with another rel or have an output that was directly changed

    // Modify e1 directly. That should propagate to the rest of the nodes. Each should get an _onAllInputsHandled callback
    // resulting graph:
    //
    //    *
    //    e1 --> e2 --> e3 --> e4
    //
    updateElement(db, e1id, "change e1");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements that have no directly changed inputs
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e1id]); // only called on directly changed root elements
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e2id, e3id, e4id]);
    assertRels(mockTxn.dres.rootChanged, [ede_1_2, ede_2_3, ede_3_4]);
    // assertRels(mockTxn.dres.validateOutput, [ede_11_2, ede_21_3]); // this callback is made only on rels that not in the graph but share an output with another rel or have an output that was directly changed

    // Modify e11 directly. That should propagate to the rest of the nodes. Each should get an _onAllInputsHandled callback
    // resulting graph:
    //
    //       > e2 --> e3 --> e4
    //      /
    //  e11
    //  *
    //
    // Note that the e1 -> e2 and e21 -> e3 edges are NOT in the sub-graph. These edges should be validated, nevertheless -- TBD
    updateElement(db, e11id, "change e11");

    mockTxn.resetDependencyResults();
    db.saveChanges();

    // assert.deepEqual(mockTxn.dres.directChange, []); // only called on directly changed non-root elements that have no directly changed inputs
    assert.deepEqual(mockTxn.dres.beforeOutputs, [e11id]); // only called on directly changed root elements
    assert.deepEqual(mockTxn.dres.allInputsHandled, [e2id, e3id, e4id]);
    assertRels(mockTxn.dres.rootChanged, [ede_11_2, ede_2_3, ede_3_4]);
    // assertRels(mockTxn.dres.validateOutput, [ede_1_2, ede_21_3]); // this callback is made only on rels that not in the graph but share an output with another rel or have an output that was directly changed
  });
});
