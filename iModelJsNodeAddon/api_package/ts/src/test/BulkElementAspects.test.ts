/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64, Id64String, IModelStatus } from "@itwin/core-bentley";
import type { ElementAspectProps, SubjectProps } from "@itwin/core-common";
import { expect } from "chai";
import { openDgnDb } from ".";
import { BulkElementAspectFailureReason, BulkElementAspectsStatus, IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, iModelJsNative } from "./utils";

const testSchemaXml = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BulkAspectTest" alias="bat" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
  <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
  <ECEntityClass typeName="TestUniqueAspect">
    <BaseClass>bis:ElementUniqueAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
    <ECProperty propertyName="IntProp" typeName="int"/>
  </ECEntityClass>
  <ECEntityClass typeName="OtherUniqueAspect">
    <BaseClass>bis:ElementUniqueAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="DerivedUniqueAspect">
    <BaseClass>TestUniqueAspect</BaseClass>
    <ECProperty propertyName="DerivedProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="TestMultiAspect">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
    <ECProperty propertyName="IntProp" typeName="int"/>
  </ECEntityClass>
  <ECEntityClass typeName="OtherMultiAspect">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="DerivedMultiAspect">
    <BaseClass>TestMultiAspect</BaseClass>
    <ECProperty propertyName="DerivedProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="AbstractUniqueAspect" modifier="Abstract">
    <BaseClass>bis:ElementUniqueAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="ConcreteUniqueAspect">
    <BaseClass>AbstractUniqueAspect</BaseClass>
  </ECEntityClass>
  <ECEntityClass typeName="AbstractMultiAspect" modifier="Abstract">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
  </ECEntityClass>
  <ECEntityClass typeName="ConcreteMultiAspect">
    <BaseClass>AbstractMultiAspect</BaseClass>
  </ECEntityClass>
</ECSchema>`;

const uniqueClass = "BulkAspectTest:TestUniqueAspect";
const otherUniqueClass = "BulkAspectTest:OtherUniqueAspect";
const derivedUniqueClass = "BulkAspectTest:DerivedUniqueAspect";
const multiClass = "BulkAspectTest:TestMultiAspect";
const otherMultiClass = "BulkAspectTest:OtherMultiAspect";
const derivedMultiClass = "BulkAspectTest:DerivedMultiAspect";
const abstractUniqueClass = "BulkAspectTest:AbstractUniqueAspect";
const concreteUniqueClass = "BulkAspectTest:ConcreteUniqueAspect";
const abstractMultiClass = "BulkAspectTest:AbstractMultiAspect";
const concreteMultiClass = "BulkAspectTest:ConcreteMultiAspect";

/** Spellings of `id` that are not valid Id64Strings, although a lenient parser could read them as an id. */
function malformedIds(id: Id64String): any[] {
  const hex = id.slice(2);
  return [`${id}garbage`, `0x0${hex}`, `0X${hex}`, `0x1${hex.padStart(16, "0")}`, "0xFFFFFF0", 42];
}

interface TestAspectProps extends ElementAspectProps {
  strProp?: string;
  intProp?: number;
  derivedProp?: string;
}

interface AspectRow {
  id: Id64String;
  className: string;
  element: Id64String;
  strProp?: string;
  intProp?: number;
}

interface HookCall {
  className: string;
  method: string;
  arg: any;
}

/** A mock JS IModelDb whose classes record every handler call. Bulk handlers ("onBulk*") are only exposed for the test aspect classes when `bulk` is set. */
function makeMockJsDb(opts: { bulk?: boolean, throwIn?: string, onCall?: (call: HookCall) => void } = {}) {
  const calls: HookCall[] = [];
  const mockJsDb: any = {
    getJsClass: (className: string) => new Proxy({}, {
      get: (_target, method) => {
        if (typeof method !== "string")
          return undefined;
        if (method.startsWith("onBulk") && !(opts.bulk && className.startsWith("BulkAspectTest:")))
          return undefined;
        return (arg: any) => {
          calls.push({ className, method, arg });
          opts.onCall?.({ className, method, arg });
          if (opts.throwIn === method)
            throw new Error(`${method} rejected`);
        };
      },
    }),
  };
  const aspectCalls = (method: string) => calls.filter((c) => c.method === method && c.className.startsWith("BulkAspectTest:"));
  const elementCalls = (method: string) => calls.filter((c) => c.method === method && c.className === "BisCore:Subject");
  return { mockJsDb, calls, aspectCalls, elementCalls };
}

describe("bulk element aspects", () => {
  let db: IModelJsNative.DgnDb;
  let subjectCounter = 0;

  before(() => {
    const fileName = copyFile("bulkElementAspects.bim", dbFileName);
    db = openDgnDb(fileName, { schemaLockHeld: true });
    db.importXmlSchemas([testSchemaXml], { schemaLockHeld: true });
    db.saveChanges();
    db.enableTxnTesting();
  });

  after(() => {
    db.setIModelDb(undefined);
    db.closeFile();
  });

  afterEach(() => {
    db.setIModelDb(undefined);
    db.abandonChanges();
  });

  function insertOwner(): Id64String {
    const props: SubjectProps = {
      classFullName: "BisCore:Subject",
      code: { spec: "0x1f", scope: "0x1", value: `BulkAspectOwner${++subjectCounter}` },
      model: "0x1",
      parent: { id: "0x1", relClassName: "BisCore:ElementOwnsChildElements" },
    };
    return db.insertElement(props);
  }

  function makeProps(classFullName: string, relClassName: string, element: Id64String, strProp: string, intProp?: number): TestAspectProps {
    const props: TestAspectProps = { classFullName, element: { id: element, relClassName }, strProp };
    if (intProp !== undefined)
      props.intProp = intProp;
    return props;
  }

  function uniqueProps(element: Id64String, strProp: string, intProp?: number, classFullName = uniqueClass): TestAspectProps {
    return makeProps(classFullName, "BisCore:ElementOwnsUniqueAspect", element, strProp, intProp);
  }

  function multiProps(element: Id64String, strProp: string, intProp?: number, classFullName = multiClass): TestAspectProps {
    return makeProps(classFullName, "BisCore:ElementOwnsMultiAspects", element, strProp, intProp);
  }

  function queryAspects(): AspectRow[] {
    const rows: AspectRow[] = [];
    for (const [className, hasIntProp] of [[uniqueClass, true], [otherUniqueClass, false], [multiClass, true], [otherMultiClass, false]] as Array<[string, boolean]>) {
      const stmt = new iModelJsNative.ECSqlStatement();
      const ecsql = `SELECT ECInstanceId, Element.Id, StrProp${hasIntProp ? ", IntProp" : ""} FROM ONLY ${className} ORDER BY ECInstanceId`;
      expect(stmt.prepare(db, ecsql).status).to.equal(DbResult.BE_SQLITE_OK);
      while (DbResult.BE_SQLITE_ROW === stmt.step()) {
        const row: AspectRow = { id: stmt.getValue(0).getId(), className, element: stmt.getValue(1).getId() };
        if (!stmt.getValue(2).isNull())
          row.strProp = stmt.getValue(2).getString();
        if (hasIntProp && !stmt.getValue(3).isNull())
          row.intProp = stmt.getValue(3).getInt();
        rows.push(row);
      }
      stmt.dispose();
    }
    return rows;
  }

  function findAspect(id: Id64String): AspectRow | undefined {
    return queryAspects().find((row) => row.id === id);
  }

  /** Reads the actual class and the given properties of an aspect, which may be of any class. */
  function readAspect(className: string, id: Id64String, propNames: string[]): any {
    const stmt = new iModelJsNative.ECSqlStatement();
    const ecsql = `SELECT ec_classname(ECClassId, 's:c'), ${propNames.join(", ")} FROM ${className.replace(":", ".")} WHERE ECInstanceId=?`;
    expect(stmt.prepare(db, ecsql).status).to.equal(DbResult.BE_SQLITE_OK);
    stmt.getBinder(1).bindId(id);
    expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
    const row: any = { className: stmt.getValue(0).getString() };
    propNames.forEach((name, i) => {
      const value = stmt.getValue(i + 1);
      if (!value.isNull())
        row[name] = name === "intProp" ? value.getInt() : value.getString();
    });
    stmt.dispose();
    return row;
  }

  /** Makes SQLite reject every `event` on the rows of `aspectClass` in `table` while `fn` runs, so that writing those aspects fails. */
  function withFailingAspectWrites(table: string, event: "UPDATE" | "DELETE", aspectClass: string, fn: () => void) {
    const [schemaName, className] = aspectClass.split(":");
    const classId = `(SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId WHERE s.Name='${schemaName}' AND c.Name='${className}')`;
    const trigger = `bulk_aspect_test_fail_${table}`;
    expect(db.executeSql(`CREATE TEMP TRIGGER ${trigger} BEFORE ${event} ON ${table} WHEN OLD.ECClassId=${classId} BEGIN SELECT RAISE(ABORT, 'rejected by test'); END`)).to.equal(DbResult.BE_SQLITE_OK);
    try {
      fn();
    } finally {
      expect(db.executeSql(`DROP TRIGGER temp.${trigger}`)).to.equal(DbResult.BE_SQLITE_OK);
    }
  }

  /** Mock handlers where the first `preMethod` call changes the label of e1, and then the onUpdated of e1 during the bulk write changes the label of e2. */
  function changeOwnersInHandlers(preMethod: string, e1: Id64String, e2: Id64String) {
    let preHandlerDone = false;
    let writing = false;
    let changedE2 = false;
    return makeMockJsDb({
      onCall: (call) => {
        if (!preHandlerDone && call.method === preMethod) {
          preHandlerDone = true;
          db.updateElement({ id: e1, userLabel: `${preMethod} changed e1` });
          writing = true;
        } else if (writing && !changedE2 && call.className === "BisCore:Subject" && call.method === "onUpdated" && call.arg.id === e1) {
          changedE2 = true;
          db.updateElement({ id: e2, userLabel: "onUpdated of e1 changed e2" });
        }
      },
    });
  }

  describe("insertElementAspects", () => {
    it("inserts unique and multi aspects of several classes on several owners", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const props = [
        uniqueProps(e1, "u1", 1),
        multiProps(e1, "m1a", 2),
        multiProps(e2, "m2a", 3),
        multiProps(e1, "m1b", 4),
        uniqueProps(e2, "u2", 5),
        uniqueProps(e1, "o1", undefined, otherUniqueClass),
        multiProps(e2, "om2", undefined, otherMultiClass),
      ];

      const result = db.insertElementAspects(props);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(result.failures).to.deep.equal([]);
      expect(result.ids.length).to.equal(props.length);
      result.ids.forEach((id) => expect(Id64.isValidId64(id)).to.be.true);
      expect(new Set(result.ids).size).to.equal(props.length);

      const expected: Array<[string, Id64String, string, number | undefined]> = [
        [uniqueClass, e1, "u1", 1],
        [multiClass, e1, "m1a", 2],
        [multiClass, e2, "m2a", 3],
        [multiClass, e1, "m1b", 4],
        [uniqueClass, e2, "u2", 5],
        [otherUniqueClass, e1, "o1", undefined],
        [otherMultiClass, e2, "om2", undefined],
      ];
      expected.forEach(([className, element, strProp, intProp], i) => {
        const row = findAspect(result.ids[i]);
        expect(row, `aspect ${i}`).to.not.be.undefined;
        expect(row!.className).to.equal(className);
        expect(row!.element).to.equal(element);
        expect(row!.strProp).to.equal(strProp);
        expect(row!.intProp).to.equal(intProp);
      });

      // The inserted aspects can be modified through the existing single-aspect API.
      db.updateElementAspect({ ...multiProps(e1, "m1a-single"), id: result.ids[1] });
      db.updateElementAspect(uniqueProps(e2, "u2-single"));
      expect(findAspect(result.ids[1])!.strProp).to.equal("m1a-single");
      expect(findAspect(result.ids[4])!.strProp).to.equal("u2-single");
    });

    it("multi aspects without relClassName use ElementOwnsMultiAspects", () => {
      const e1 = insertOwner();
      const result = db.insertElementAspects([{ classFullName: multiClass, element: { id: e1 }, strProp: "noRel" } as TestAspectProps]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);

      const stmt = new iModelJsNative.ECSqlStatement();
      stmt.prepare(db, "SELECT Element.Id, ec_classname(Element.RelECClassId, 's:c') FROM bat.TestMultiAspect WHERE ECInstanceId=?");
      stmt.getBinder(1).bindId(result.ids[0]);
      expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
      expect(stmt.getValue(0).getId()).to.equal(e1);
      expect(stmt.getValue(1).getString()).to.equal("BisCore:ElementOwnsMultiAspects");
      stmt.dispose();
    });

    it("a duplicate unique aspect in a batch is an upsert: last row wins and all rows report the same id", () => {
      const e1 = insertOwner();
      const result = db.insertElementAspects([uniqueProps(e1, "first", 1), uniqueProps(e1, "second", 2)]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(result.ids[0]).to.equal(result.ids[1]);

      const rows = queryAspects().filter((r) => r.element === e1);
      expect(rows.length).to.equal(1);
      expect(rows[0].strProp).to.equal("second");
      expect(rows[0].intProp).to.equal(2);

      // Inserting a unique aspect that already exists behaves like insertElementAspect (upsert).
      const again = db.insertElementAspects([uniqueProps(e1, "third", 3)]);
      expect(again.status).to.equal(BulkElementAspectsStatus.Success);
      expect(again.ids[0]).to.equal(result.ids[0]);
      expect(findAspect(again.ids[0])!.strProp).to.equal("third");
    });

    it("validates every row, and writes nothing and calls no handlers if any row is invalid", () => {
      const e1 = insertOwner();
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      const props: any[] = [
        multiProps(e1, "ok1"),
        multiProps("0", "invalidOwner"),
        multiProps("0xffffff0", "missingOwner"),
        { ...multiProps(e1, "badClass"), classFullName: "BulkAspectTest:DoesNotExist" },
        { ...multiProps(e1, "notAnAspect"), classFullName: "BisCore:Subject" },
        "not an object",
        { ...multiProps(e1, "noClass"), classFullName: undefined },
        uniqueProps(e1, "ok2"),
      ];

      const result = db.insertElementAspects(props);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([
        { index: 1, reason: BulkElementAspectFailureReason.InvalidId },
        { index: 2, reason: BulkElementAspectFailureReason.ElementNotFound },
        { index: 3, reason: BulkElementAspectFailureReason.ClassNotFound },
        { index: 4, reason: BulkElementAspectFailureReason.NotAnAspectClass },
        { index: 5, reason: BulkElementAspectFailureReason.InvalidProps },
        { index: 6, reason: BulkElementAspectFailureReason.InvalidProps },
      ]);
      expect(result.ids).to.deep.equal([]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects().filter((r) => r.element === e1)).to.deep.equal([]);
    });

    it("all rows failing reports ValidationFailed, and an empty array reports Success", () => {
      const all = db.insertElementAspects([multiProps("0", "a"), multiProps("0", "b")]);
      expect(all.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(all.failures.length).to.equal(2);
      expect(all.ids).to.deep.equal([]);

      const empty = db.insertElementAspects([]);
      expect(empty.status).to.equal(BulkElementAspectsStatus.Success);
      expect(empty.failures).to.deep.equal([]);
      expect(empty.ids).to.deep.equal([]);
    });

    it("throws if the argument is not an array", () => {
      expect(() => db.insertElementAspects({} as any)).to.throw();
    });

    it("rejects owner ids that are not valid Id64Strings, even when a lenient parser would read an id from them", () => {
      const e1 = insertOwner();
      const ids = malformedIds(e1);
      const result = db.insertElementAspects(ids.map((id) => ({ classFullName: multiClass, element: { id }, strProp: "x" })));
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal(ids.map((_id, index) => ({ index, reason: BulkElementAspectFailureReason.InvalidId })));
      expect(queryAspects().filter((r) => r.element === e1)).to.deep.equal([]);
    });

    it("finds the owner from the element member only; other spellings of it are ignored", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const result = db.insertElementAspects([
        // eslint-disable-next-line @typescript-eslint/naming-convention
        { ...multiProps(e1, "m1"), Element: { id: e2, relClassName: "BisCore:ElementOwnsMultiAspects" } } as TestAspectProps,
        // eslint-disable-next-line @typescript-eslint/naming-convention
        { ...uniqueProps(e1, "u1"), ELEMENT: { id: e2, relClassName: "BisCore:ElementOwnsUniqueAspect" } } as TestAspectProps,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(result.ids[0])).to.deep.include({ element: e1, strProp: "m1" });
      expect(findAspect(result.ids[1])).to.deep.include({ element: e1, strProp: "u1" });
    });

    it("throws at the first aspect that cannot be written, calling no post-insert handlers", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      expect(db.insertElementAspects([uniqueProps(e2, "o2", undefined, otherUniqueClass)]).status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "UPDATE", otherMultiClass, () => {
        expect(() => db.insertElementAspects([
          multiProps(e1, "m1"),
          multiProps(e1, "om1", undefined, otherMultiClass), // new multi aspect
          uniqueProps(e2, "o2-new", undefined, otherUniqueClass),
          uniqueProps(e1, "u1"),
        ])).to.throw(/insertElementAspects: unable to write the aspect at index 1/);
      });

      expect(mock.aspectCalls("onInsert").length).to.equal(4);
      expect(mock.aspectCalls("onInserted")).to.deep.equal([]);
    });

    it("throws if an existing unique aspect cannot be upserted", () => {
      const e1 = insertOwner();
      expect(db.insertElementAspects([uniqueProps(e1, "o1", undefined, otherUniqueClass)]).status).to.equal(BulkElementAspectsStatus.Success);
      withFailingAspectWrites("bis_ElementUniqueAspect", "UPDATE", otherUniqueClass, () => {
        expect(() => db.insertElementAspects([uniqueProps(e1, "o1-new", undefined, otherUniqueClass)])).to.throw(/unable to write the aspect at index 0/);
      });
    });

    it("rejects unique aspects of classes related by inheritance on the same owner, since writing one could replace the other", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const existing = db.insertElementAspects([uniqueProps(e2, "u2-derived", undefined, derivedUniqueClass)]);
      expect(existing.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const result = db.insertElementAspects([
        uniqueProps(e1, "u1-derived", undefined, derivedUniqueClass),
        uniqueProps(e1, "o1", undefined, otherUniqueClass),
        uniqueProps(e1, "u1"),
        uniqueProps(e2, "u2"),
        uniqueProps(e2, "u2-derived-new", undefined, derivedUniqueClass),
        multiProps(e1, "m1"),
        multiProps(e1, "m1-derived", undefined, derivedMultiClass),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([0, 2, 3, 4].map((index) => ({ index, reason: BulkElementAspectFailureReason.ConflictingAspects })));
      expect(result.ids).to.deep.equal([]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);
      expect(readAspect(uniqueClass, existing.ids[0], ["strProp"])).to.deep.equal({ className: derivedUniqueClass, strProp: "u2-derived" });

      // Related unique classes on different owners, and multi aspects of related classes on one owner, do not conflict
      const e3 = insertOwner();
      const props = [
        uniqueProps(e1, "u1-derived", undefined, derivedUniqueClass),
        uniqueProps(e3, "u3"),
        multiProps(e1, "m1"),
        multiProps(e1, "m1-derived", undefined, derivedMultiClass),
      ];
      const inserted = db.insertElementAspects(props);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(readAspect(uniqueClass, inserted.ids[0], ["strProp"])).to.deep.equal({ className: derivedUniqueClass, strProp: "u1-derived" });
      expect(readAspect(uniqueClass, inserted.ids[1], ["strProp"])).to.deep.equal({ className: uniqueClass, strProp: "u3" });
      expect(readAspect(multiClass, inserted.ids[2], ["strProp"])).to.deep.equal({ className: multiClass, strProp: "m1" });
      expect(readAspect(multiClass, inserted.ids[3], ["strProp"])).to.deep.equal({ className: derivedMultiClass, strProp: "m1-derived" });
    });

    it("keeps the changes that the pre-insert handlers and the callbacks of earlier owners make to an owning element", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const mock = changeOwnersInHandlers("onInsert", e1, e2);
      db.setIModelDb(mock.mockJsDb);

      const result = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e2, "m2")]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(db.getElement({ id: e1 }).userLabel).to.equal("onInsert changed e1");
      expect(db.getElement({ id: e2 }).userLabel).to.equal("onUpdated of e1 changed e2");
      expect(findAspect(result.ids[0])).to.deep.include({ element: e1, strProp: "u1" });
      expect(findAspect(result.ids[1])).to.deep.include({ element: e2, strProp: "m2" });
    });
  });

  describe("updateElementAspects", () => {
    it("updates unique and multi aspects on several owners, leaving untouched properties alone", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([
        uniqueProps(e1, "u1", 1),
        multiProps(e1, "m1", 2),
        multiProps(e2, "m2", 3),
        uniqueProps(e2, "o2", undefined, otherUniqueClass),
      ]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        uniqueProps(e1, "u1-new"),
        { ...multiProps(e1, "m1-new", 20), id: inserted.ids[1] },
        { classFullName: multiClass, element: { id: e2 }, id: inserted.ids[2], intProp: 30 } as TestAspectProps,
        uniqueProps(e2, "o2-new", undefined, otherUniqueClass),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(result.failures).to.deep.equal([]);

      expect(findAspect(inserted.ids[0])).to.deep.include({ strProp: "u1-new", intProp: 1 });
      expect(findAspect(inserted.ids[1])).to.deep.include({ strProp: "m1-new", intProp: 20 });
      expect(findAspect(inserted.ids[2])).to.deep.include({ strProp: "m2", intProp: 30 });
      expect(findAspect(inserted.ids[3])).to.deep.include({ strProp: "o2-new" });
    });

    it("validates every row, and writes nothing if any row is invalid or not found", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const e3 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e2, "u2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
        multiProps(e1, "noId"),
        { ...multiProps(e1, "wrongId"), id: "0xffffff0" },
        uniqueProps(e3, "noUniqueAspect"),
        uniqueProps("0xffffff0", "missingOwner"),
        uniqueProps(e2, "u2-new"),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([
        { index: 1, reason: BulkElementAspectFailureReason.InvalidId },
        { index: 2, aspectId: "0xffffff0", reason: BulkElementAspectFailureReason.AspectNotFound },
        { index: 3, reason: BulkElementAspectFailureReason.AspectNotFound },
        { index: 4, reason: BulkElementAspectFailureReason.ElementNotFound },
      ]);
      expect(queryAspects()).to.deep.equal(before);
    });

    it("ignores every spelling of element, so an aspect cannot be moved to another owner", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        // eslint-disable-next-line @typescript-eslint/naming-convention
        { ...multiProps(e1, "m1-new"), id: inserted.ids[0], Element: { id: e2, relClassName: "BisCore:ElementOwnsMultiAspects" } } as TestAspectProps,
        // eslint-disable-next-line @typescript-eslint/naming-convention
        { ...uniqueProps(e1, "u1-new"), Element: { id: e2, relClassName: "BisCore:ElementOwnsUniqueAspect" } } as TestAspectProps,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(inserted.ids[0])).to.deep.include({ element: e1, strProp: "m1-new" });
      expect(findAspect(inserted.ids[1])).to.deep.include({ element: e1, strProp: "u1-new" });
    });

    it("does not update a multi aspect owned by another element", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e2, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m2-moved"), id: inserted.ids[1] },
        { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([{ index: 0, aspectId: inserted.ids[1], reason: BulkElementAspectFailureReason.AspectNotFound }]);
      expect(findAspect(inserted.ids[1])).to.deep.include({ element: e2, strProp: "m2" });
      expect(findAspect(inserted.ids[0])).to.deep.include({ element: e1, strProp: "m1" });
    });

    it("throws at the first aspect that cannot be written, calling no post-update handlers", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "om1", undefined, otherMultiClass), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "UPDATE", otherMultiClass, () => {
        expect(() => db.updateElementAspects([
          { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
          { ...multiProps(e1, "om1-new", undefined, otherMultiClass), id: inserted.ids[1] },
          uniqueProps(e1, "u1-new"),
        ])).to.throw(/updateElementAspects: unable to write the aspect at index 1/);
      });

      expect(mock.aspectCalls("onUpdate").length).to.equal(3);
      expect(mock.aspectCalls("onUpdated")).to.deep.equal([]);
    });

    it("applies rows that name the same aspect in order", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([uniqueProps(e1, "u1", 1), multiProps(e1, "m1", 2)]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        { classFullName: uniqueClass, element: { id: e1 }, strProp: "u1-a" } as TestAspectProps,
        { classFullName: multiClass, element: { id: e1 }, id: inserted.ids[1], intProp: 20 } as TestAspectProps,
        { classFullName: uniqueClass, element: { id: e1 }, intProp: 10 } as TestAspectProps,
        { classFullName: multiClass, element: { id: e1 }, id: inserted.ids[1], strProp: "m1-b" } as TestAspectProps,
        { classFullName: uniqueClass, element: { id: e1 }, strProp: "u1-c" } as TestAspectProps,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(inserted.ids[0])).to.deep.include({ strProp: "u1-c", intProp: 10 });
      expect(findAspect(inserted.ids[1])).to.deep.include({ strProp: "m1-b", intProp: 20 });
    });

    it("updates a unique aspect of a derived class through its base class, keeping the derived properties", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([{ ...uniqueProps(e1, "d1", 1, "BulkAspectTest:DerivedUniqueAspect"), derivedProp: "derived" } as TestAspectProps]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([{ classFullName: uniqueClass, element: { id: e1 }, strProp: "d1-new" } as TestAspectProps]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);

      const stmt = new iModelJsNative.ECSqlStatement();
      expect(stmt.prepare(db, "SELECT ECInstanceId, ECClassId, StrProp, IntProp, DerivedProp FROM BulkAspectTest.DerivedUniqueAspect WHERE Element.Id=?").status).to.equal(DbResult.BE_SQLITE_OK);
      stmt.getBinder(1).bindId(e1);
      expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
      expect(stmt.getValue(0).getId()).to.equal(inserted.ids[0]);
      expect(stmt.getValue(1).getClassNameForClassId()).to.equal("BulkAspectTest.DerivedUniqueAspect");
      expect(stmt.getValue(2).getString()).to.equal("d1-new");
      expect(stmt.getValue(3).getInt()).to.equal(1);
      expect(stmt.getValue(4).getString()).to.equal("derived");
      expect(stmt.step()).to.equal(DbResult.BE_SQLITE_DONE);
      stmt.dispose();
    });

    it("the single-aspect API sees aspects written by the bulk API, and the other way around", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([uniqueProps(e1, "u1", 1), multiProps(e1, "m1", 2)]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      db.updateElementAspect({ classFullName: uniqueClass, element: { id: e1 }, intProp: 10 } as TestAspectProps);
      db.updateElementAspect({ classFullName: multiClass, element: { id: e1 }, id: inserted.ids[1], intProp: 20 } as TestAspectProps);

      const result = db.updateElementAspects([
        { classFullName: uniqueClass, element: { id: e1 }, strProp: "u1-bulk" } as TestAspectProps,
        { classFullName: multiClass, element: { id: e1 }, id: inserted.ids[1], strProp: "m1-bulk" } as TestAspectProps,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(inserted.ids[0])).to.deep.include({ strProp: "u1-bulk", intProp: 10 });
      expect(findAspect(inserted.ids[1])).to.deep.include({ strProp: "m1-bulk", intProp: 20 });

      db.updateElementAspect({ classFullName: uniqueClass, element: { id: e1 }, intProp: 100 } as TestAspectProps);
      db.updateElementAspect({ classFullName: multiClass, element: { id: e1 }, id: inserted.ids[1], intProp: 200 } as TestAspectProps);
      expect(findAspect(inserted.ids[0])).to.deep.include({ strProp: "u1-bulk", intProp: 100 });
      expect(findAspect(inserted.ids[1])).to.deep.include({ strProp: "m1-bulk", intProp: 200 });
    });

    it("rejects multi aspect ids that are not valid Id64Strings, even when a lenient parser would read an id from them", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();

      const ids = malformedIds(inserted.ids[0]);
      const result = db.updateElementAspects(ids.map((id) => ({ ...multiProps(e1, "m1-new"), id })));
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal(ids.map((_id, index) => ({ index, reason: BulkElementAspectFailureReason.InvalidId })));
      expect(queryAspects()).to.deep.equal(before);
    });

    it("updates a multi aspect of a derived class through its base class, keeping the derived properties", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([{ ...multiProps(e1, "d1", 1, derivedMultiClass), derivedProp: "derived" } as TestAspectProps]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([{ ...multiProps(e1, "d1-new"), id: inserted.ids[0] }]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(readAspect(derivedMultiClass, inserted.ids[0], ["strProp", "intProp", "derivedProp"])).to.deep.equal(
        { className: derivedMultiClass, strProp: "d1-new", intProp: 1, derivedProp: "derived" });
    });

    it("applies rows that name the same aspect through different classes in order", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([
        { ...multiProps(e1, "m", 1, derivedMultiClass), derivedProp: "d" } as TestAspectProps,
        { ...uniqueProps(e1, "u", 1, derivedUniqueClass), derivedProp: "d" } as TestAspectProps,
      ]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const result = db.updateElementAspects([
        { classFullName: multiClass, element: { id: e1 }, id: inserted.ids[0], strProp: "m-a", intProp: 2 },
        { classFullName: derivedMultiClass, element: { id: e1 }, id: inserted.ids[0], strProp: "m-b", derivedProp: "d-b" },
        { classFullName: multiClass, element: { id: e1 }, id: inserted.ids[0], intProp: 3 },
        { classFullName: uniqueClass, element: { id: e1 }, strProp: "u-a" },
        { classFullName: derivedUniqueClass, element: { id: e1 }, derivedProp: "d-b" },
      ] as TestAspectProps[]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(readAspect(derivedMultiClass, inserted.ids[0], ["strProp", "intProp", "derivedProp"])).to.deep.equal(
        { className: derivedMultiClass, strProp: "m-b", intProp: 3, derivedProp: "d-b" });
      expect(readAspect(derivedUniqueClass, inserted.ids[1], ["strProp", "intProp", "derivedProp"])).to.deep.equal(
        { className: derivedUniqueClass, strProp: "u-a", intProp: 1, derivedProp: "d-b" });
      expect(mock.aspectCalls("onUpdate").length).to.equal(5);
      expect(mock.aspectCalls("onUpdated").length).to.equal(5);
      expect(mock.elementCalls("onUpdated").length).to.equal(1);
    });

    it("rejects a row whose class is neither the class of the aspect nor a base class of it", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m1-new", undefined, derivedMultiClass), id: inserted.ids[0] },
        { ...multiProps(e1, "m1-new", undefined, otherMultiClass), id: inserted.ids[0] },
        uniqueProps(e1, "u1-new", undefined, derivedUniqueClass),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([
        { index: 0, aspectId: inserted.ids[0], reason: BulkElementAspectFailureReason.AspectNotFound },
        { index: 1, aspectId: inserted.ids[0], reason: BulkElementAspectFailureReason.AspectNotFound },
        { index: 2, reason: BulkElementAspectFailureReason.AspectNotFound },
      ]);
      expect(queryAspects()).to.deep.equal(before);
    });

    it("updates aspects through an abstract base class", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1", undefined, concreteMultiClass), uniqueProps(e1, "u1", undefined, concreteUniqueClass)]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m1-new", undefined, abstractMultiClass), id: inserted.ids[0] },
        uniqueProps(e1, "u1-new", undefined, abstractUniqueClass),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(readAspect(concreteMultiClass, inserted.ids[0], ["strProp"])).to.deep.equal({ className: concreteMultiClass, strProp: "m1-new" });
      expect(readAspect(concreteUniqueClass, inserted.ids[1], ["strProp"])).to.deep.equal({ className: concreteUniqueClass, strProp: "u1-new" });
    });

    it("rejects a unique aspect named through a base class that matches more than one aspect of the element", () => {
      const e1 = insertOwner();
      const base = db.insertElementAspects([uniqueProps(e1, "u-base")]);
      expect(base.status).to.equal(BulkElementAspectsStatus.Success);
      const derived = db.insertElementAspects([uniqueProps(e1, "u-derived", undefined, derivedUniqueClass)]);
      expect(derived.status).to.equal(BulkElementAspectsStatus.Success);
      const e2 = insertOwner();
      expect(db.insertElementAspects([uniqueProps(e2, "u2")]).status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();
      expect(readAspect(uniqueClass, base.ids[0], ["strProp"])).to.deep.equal({ className: uniqueClass, strProp: "u-base" });
      expect(readAspect(uniqueClass, derived.ids[0], ["strProp"])).to.deep.equal({ className: derivedUniqueClass, strProp: "u-derived" });
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const result = db.updateElementAspects([
        uniqueProps(e2, "u2-new"),
        uniqueProps(e1, "u-new"),
        uniqueProps(e1, "u-derived-new", undefined, derivedUniqueClass),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([{ index: 1, reason: BulkElementAspectFailureReason.AmbiguousAspect }]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);
      expect(readAspect(uniqueClass, derived.ids[0], ["strProp"])).to.deep.equal({ className: derivedUniqueClass, strProp: "u-derived" });

      // Naming the class of the aspect is not ambiguous
      expect(db.updateElementAspects([uniqueProps(e1, "u-derived-new", undefined, derivedUniqueClass)]).status).to.equal(BulkElementAspectsStatus.Success);
      expect(readAspect(uniqueClass, derived.ids[0], ["strProp"])).to.deep.equal({ className: derivedUniqueClass, strProp: "u-derived-new" });
      expect(readAspect(uniqueClass, base.ids[0], ["strProp"])).to.deep.equal({ className: uniqueClass, strProp: "u-base" });
    });

    it("loads the aspects after the pre-update handlers, keeping their changes to properties that the rows do not set", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1", 1)]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      let changed = false;
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (!changed && call.method === "onUpdate" && call.className === multiClass) {
            changed = true;
            db.updateElementAspect({ ...multiProps(e1, "m1-handler", 10), id: inserted.ids[0] });
          }
        },
      });
      db.setIModelDb(mock.mockJsDb);

      const result = db.updateElementAspects([{ classFullName: multiClass, element: { id: e1 }, id: inserted.ids[0], strProp: "m1-bulk" } as TestAspectProps]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(inserted.ids[0])).to.deep.include({ strProp: "m1-bulk", intProp: 10 });
    });

    it("throws if a pre-update handler deletes one of the aspects", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      let removed = false;
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (!removed && call.method === "onUpdate" && call.className === multiClass) {
            removed = true;
            db.deleteElementAspect(inserted.ids[1]);
          }
        },
      });
      db.setIModelDb(mock.mockJsDb);

      expect(() => db.updateElementAspects(inserted.ids.map((id, i) => ({ ...multiProps(e1, `m${i}-new`), id }))))
        .to.throw(`aspect ${inserted.ids[1]} can no longer be updated after the onUpdate handlers`);
      expect(mock.aspectCalls("onUpdated")).to.deep.equal([]);
      expect(findAspect(inserted.ids[0])!.strProp).to.equal("m1");
    });

    it("keeps the changes that the pre-update handlers and the callbacks of earlier owners make to an owning element", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e2, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = changeOwnersInHandlers("onUpdate", e1, e2);
      db.setIModelDb(mock.mockJsDb);

      const result = db.updateElementAspects([uniqueProps(e1, "u1-new"), { ...multiProps(e2, "m2-new"), id: inserted.ids[1] }]);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(db.getElement({ id: e1 }).userLabel).to.equal("onUpdate changed e1");
      expect(db.getElement({ id: e2 }).userLabel).to.equal("onUpdated of e1 changed e2");
      expect(findAspect(inserted.ids[0])!.strProp).to.equal("u1-new");
      expect(findAspect(inserted.ids[1])!.strProp).to.equal("m2-new");
    });
  });

  describe("deleteElementAspects", () => {
    it("deletes unique and multi aspects of several classes on several owners", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([
        uniqueProps(e1, "u1"),
        multiProps(e1, "m1a"),
        multiProps(e1, "m1b"),
        multiProps(e2, "m2"),
        uniqueProps(e2, "o2", undefined, otherUniqueClass),
        multiProps(e2, "om2", undefined, otherMultiClass),
      ]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const toDelete = [inserted.ids[0], inserted.ids[2], inserted.ids[4], inserted.ids[5]];
      const result = db.deleteElementAspects(toDelete);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(result.failures).to.deep.equal([]);

      const remaining = queryAspects().filter((r) => r.element === e1 || r.element === e2).map((r) => r.id).sort();
      expect(remaining).to.deep.equal([inserted.ids[1], inserted.ids[3]].sort());

      // The deleted aspects must not linger in any cache used by the single-aspect API.
      expect(() => db.updateElementAspect(uniqueProps(e1, "u1-new"))).to.throw();
      expect(() => db.updateElementAspect({ ...multiProps(e1, "m1b-new"), id: inserted.ids[2] })).to.throw();
      db.updateElementAspect({ ...multiProps(e1, "m1a-new"), id: inserted.ids[1] });
      expect(findAspect(inserted.ids[1])!.strProp).to.equal("m1a-new");
    });

    it("validates every id, and deletes nothing and calls no handlers if any id is invalid", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      const before = queryAspects();

      const result = db.deleteElementAspects([
        inserted.ids[0],
        "not an id",
        "0xffffff0",
        inserted.ids[0], // duplicate
        42 as any,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal([
        { index: 1, reason: BulkElementAspectFailureReason.InvalidId },
        { index: 4, reason: BulkElementAspectFailureReason.InvalidId },
      ]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);

      const empty = db.deleteElementAspects([]);
      expect(empty.status).to.equal(BulkElementAspectsStatus.Success);
    });

    it("ignores ids that do not identify an aspect, like deleteElements", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      // a missing id (twice), and an element id, which is not an aspect
      const result = db.deleteElementAspects(["0xffffff0", inserted.ids[0], "0xffffff0", e1]);
      expect(result).to.deep.equal({ status: BulkElementAspectsStatus.Success, failures: [] });
      expect(mock.aspectCalls("onDelete").map((c) => c.arg.aspectId)).to.deep.equal([inserted.ids[0]]);
      expect(mock.aspectCalls("onDeleted").map((c) => c.arg.aspectId)).to.deep.equal([inserted.ids[0]]);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);
      expect(() => db.getElement({ id: e1 })).to.not.throw();

      // only ids that are not aspects: nothing to do
      mock.calls.length = 0;
      expect(db.deleteElementAspects(["0xffffff0", inserted.ids[0]])).to.deep.equal({ status: BulkElementAspectsStatus.Success, failures: [] });
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);
    });

    it("deletes a duplicated id once", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      expect(db.deleteElementAspects([inserted.ids[0], inserted.ids[0]])).to.deep.equal({ status: BulkElementAspectsStatus.Success, failures: [] });
      expect(mock.aspectCalls("onDeleted").length).to.equal(1);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);
    });

    it("throws if the aspects cannot be deleted, calling no post-delete handlers", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "om1", undefined, otherMultiClass), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "DELETE", otherMultiClass, () => {
        expect(() => db.deleteElementAspects(inserted.ids)).to.throw(/deleteElementAspects: unable to delete the aspects/);
      });

      expect(mock.aspectCalls("onDelete").length).to.equal(3);
      expect(mock.aspectCalls("onDeleted")).to.deep.equal([]);
    });

    it("deletes the aspects of many owners and updates each owner once", () => {
      const owners = [insertOwner(), insertOwner(), insertOwner()];
      const props: TestAspectProps[] = [];
      owners.forEach((e, i) => props.push(
        multiProps(e, `m${i}a`),
        multiProps(e, `m${i}b`),
        uniqueProps(e, `u${i}`),
        multiProps(e, `om${i}`, undefined, otherMultiClass),
      ));
      const inserted = db.insertElementAspects(props);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const result = db.deleteElementAspects(inserted.ids);
      expect(result.status).to.equal(BulkElementAspectsStatus.Success);
      expect(queryAspects().filter((r) => owners.includes(r.element))).to.deep.equal([]);
      expect(mock.elementCalls("onUpdated").map((c) => c.arg.id)).to.deep.equal(owners);
      expect(mock.aspectCalls("onDeleted").map((c) => c.arg.aspectId)).to.have.members(inserted.ids);
    });

    it("deletes aspects that the single-aspect API has loaded, leaving none of them cached", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      db.updateElementAspect(uniqueProps(e1, "u1-new"));
      db.updateElementAspect({ ...multiProps(e1, "m1-new"), id: inserted.ids[1] });

      expect(db.deleteElementAspects([inserted.ids[0], inserted.ids[1]]).status).to.equal(BulkElementAspectsStatus.Success);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[2]]);
      expect(() => db.updateElementAspect(uniqueProps(e1, "u1-again"))).to.throw();
      expect(() => db.updateElementAspect({ ...multiProps(e1, "m1-again"), id: inserted.ids[1] })).to.throw();
      db.updateElementAspect({ ...multiProps(e1, "m2-new"), id: inserted.ids[2] });
      expect(findAspect(inserted.ids[2])!.strProp).to.equal("m2-new");
    });

    it("throws if a pre-delete handler removes one of the aspects", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2"), multiProps(e1, "m3")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      let removed = false;
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (!removed && call.method === "onDelete" && call.arg.aspectId === inserted.ids[0]) {
            removed = true;
            db.deleteElementAspect(inserted.ids[1]);
          }
        },
      });
      db.setIModelDb(mock.mockJsDb);

      // The missing aspect is detected by the delete itself, so the other aspects may have been deleted: the caller must abandon the changes.
      expect(() => db.deleteElementAspects(inserted.ids)).to.throw(/can no longer be deleted after the onDelete handlers/);
      // only the single-aspect delete made by the handler calls onDeleted
      expect(mock.aspectCalls("onDeleted").map((c) => c.arg.aspectId)).to.deep.equal([inserted.ids[1]]);
    });

    it("throws if a pre-delete handler deletes the owning element", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e2, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      let deleted = false;
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (!deleted && call.method === "onDelete") {
            deleted = true;
            db.deleteElement(e2);
          }
        },
      });
      db.setIModelDb(mock.mockJsDb);

      expect(() => db.deleteElementAspects(inserted.ids)).to.throw(/can no longer be deleted after the onDelete handlers/);
      expect(mock.aspectCalls("onDeleted")).to.deep.equal([]);
    });

    it("keeps the changes that a pre-delete handler makes to the owning element", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      let updated = false;
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (!updated && call.method === "onDelete") {
            updated = true;
            db.updateElement({ id: e1, userLabel: "changed by onDelete" });
          }
        },
      });
      db.setIModelDb(mock.mockJsDb);

      expect(db.deleteElementAspects(inserted.ids)).to.deep.equal({ status: BulkElementAspectsStatus.Success, failures: [] });
      expect(queryAspects().filter((r) => r.element === e1)).to.deep.equal([]);
      expect(db.getElement({ id: e1 }).userLabel).to.equal("changed by onDelete");
    });

    it("rejects ids that are not valid Id64Strings, even when a lenient parser would read an id from them", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();

      const ids = malformedIds(inserted.ids[0]);
      const result = db.deleteElementAspects(ids);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures).to.deep.equal(ids.map((_id, index) => ({ index, reason: BulkElementAspectFailureReason.InvalidId })));
      expect(queryAspects()).to.deep.equal(before);
    });

    it("deletes the aspects of each owner during that owner's update, after its onUpdated and before its parent's onChildUpdated", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e1, "u1"), multiProps(e2, "m2"), uniqueProps(e2, "u2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const remaining = () => queryAspects().filter((r) => r.element === e1 || r.element === e2).map((r) => r.id).sort();
      const all = [...inserted.ids].sort();
      const ofE2 = [inserted.ids[2], inserted.ids[3]].sort();
      const snapshots: Array<{ method: string, owner: Id64String, remaining: Id64String[] }> = [];
      const mock = makeMockJsDb({
        onCall: (call) => {
          if (call.className === "BisCore:Subject" && call.method === "onUpdated")
            snapshots.push({ method: call.method, owner: call.arg.id, remaining: remaining() });
          else if (call.className === "BisCore:Subject" && call.method === "onChildUpdated")
            snapshots.push({ method: call.method, owner: call.arg.childId, remaining: remaining() });
        },
      });
      db.setIModelDb(mock.mockJsDb);

      expect(db.deleteElementAspects(inserted.ids).status).to.equal(BulkElementAspectsStatus.Success);
      expect(snapshots).to.deep.equal([
        { method: "onUpdated", owner: e1, remaining: all },
        { method: "onChildUpdated", owner: e1, remaining: ofE2 },
        { method: "onUpdated", owner: e2, remaining: ofE2 },
        { method: "onChildUpdated", owner: e2, remaining: [] },
      ]);
    });
  });

  describe("JS handlers", () => {
    it("falls back to the per-aspect handlers and updates each owning element once", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const inserted = db.insertElementAspects([
        uniqueProps(e1, "u1"),
        multiProps(e1, "m1a"),
        multiProps(e1, "m1b"),
        multiProps(e2, "m2"),
      ]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onInsert").length).to.equal(4);
      expect(mock.aspectCalls("onInserted").length).to.equal(4);
      expect(mock.aspectCalls("onInsert").map((c) => c.arg.props.strProp)).to.deep.equal(["u1", "m1a", "m1b", "m2"]);
      mock.aspectCalls("onInsert").forEach((c) => {
        expect(c.arg.iModel).to.equal(mock.mockJsDb);
        expect(c.arg.model).to.equal("0x1");
      });
      // Each owning element is updated exactly once, no matter how many aspects it gets. Native element updates only fire
      // the post-update handler; the element pre-update handler is called by TypeScript, not by native code.
      expect(mock.elementCalls("onUpdated").length).to.equal(2);
      expect(mock.elementCalls("onUpdate").length).to.equal(0);

      mock.calls.length = 0;
      const updated = db.updateElementAspects([
        uniqueProps(e1, "u1-new"),
        { ...multiProps(e1, "m1a-new"), id: inserted.ids[1] },
        { ...multiProps(e2, "m2-new"), id: inserted.ids[3] },
      ]);
      expect(updated.status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onUpdate").length).to.equal(3);
      expect(mock.aspectCalls("onUpdated").length).to.equal(3);
      expect(mock.elementCalls("onUpdated").length).to.equal(2);

      mock.calls.length = 0;
      const deleted = db.deleteElementAspects(inserted.ids);
      expect(deleted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onDelete").length).to.equal(4);
      expect(mock.aspectCalls("onDeleted").length).to.equal(4);
      expect(mock.aspectCalls("onDelete").map((c) => c.arg.aspectId)).to.deep.equal(inserted.ids);
      expect(mock.aspectCalls("onDelete").map((c) => c.arg.element)).to.deep.equal([e1, e1, e1, e2]);
      expect(mock.elementCalls("onUpdated").length).to.equal(2);
    });

    it("calls the bulk handlers once per class when they exist", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const mock = makeMockJsDb({ bulk: true });
      db.setIModelDb(mock.mockJsDb);

      const inserted = db.insertElementAspects([
        multiProps(e1, "m1"),
        uniqueProps(e1, "u1"),
        multiProps(e2, "m2"),
      ]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onInsert").length).to.equal(0);
      const bulkInsert = mock.aspectCalls("onBulkInsert");
      expect(bulkInsert.map((c) => c.className)).to.deep.equal([multiClass, uniqueClass]);
      expect(bulkInsert[0].arg.iModel).to.equal(mock.mockJsDb);
      expect(bulkInsert[0].arg.aspects.map((a: any) => a.props.strProp)).to.deep.equal(["m1", "m2"]);
      expect(bulkInsert[1].arg.aspects.map((a: any) => a.props.strProp)).to.deep.equal(["u1"]);
      for (const c of bulkInsert)
        c.arg.aspects.forEach((a: any) => expect(a.iModel).to.equal(mock.mockJsDb));
      expect(mock.aspectCalls("onBulkInserted").length).to.equal(2);

      mock.calls.length = 0;
      expect(db.updateElementAspects([{ ...multiProps(e1, "m1-new"), id: inserted.ids[0] }]).status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onBulkUpdate").length).to.equal(1);
      expect(mock.aspectCalls("onBulkUpdated").length).to.equal(1);
      expect(mock.aspectCalls("onUpdate").length).to.equal(0);

      mock.calls.length = 0;
      expect(db.deleteElementAspects(inserted.ids).status).to.equal(BulkElementAspectsStatus.Success);
      expect(mock.aspectCalls("onBulkDelete").map((c) => c.arg.aspects.length)).to.deep.equal([2, 1]);
      mock.aspectCalls("onBulkDelete").forEach((c) => c.arg.aspects.forEach((a: any) => expect(a.iModel).to.equal(mock.mockJsDb)));
      expect(mock.aspectCalls("onBulkDeleted").length).to.equal(2);
      expect(mock.aspectCalls("onDelete").length).to.equal(0);
    });

    it("calls the post-handlers in input order when owners are interleaved", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const mock = makeMockJsDb({ bulk: true });
      db.setIModelDb(mock.mockJsDb);
      const strProps = (method: string) => mock.aspectCalls(method).map((c) => c.arg.aspects.map((a: any) => a.props?.strProp ?? a.aspectId));

      const inserted = db.insertElementAspects([multiProps(e1, "a"), multiProps(e2, "b"), multiProps(e1, "c")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(strProps("onBulkInsert")).to.deep.equal([["a", "b", "c"]]);
      expect(strProps("onBulkInserted")).to.deep.equal([["a", "b", "c"]]);

      const updated = db.updateElementAspects(inserted.ids.map((id, i) => ({ ...multiProps(i === 1 ? e2 : e1, `${i}`), id })));
      expect(updated.status).to.equal(BulkElementAspectsStatus.Success);
      expect(strProps("onBulkUpdated")).to.deep.equal([["0", "1", "2"]]);

      const deleted = db.deleteElementAspects(inserted.ids);
      expect(deleted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(strProps("onBulkDeleted")).to.deep.equal([inserted.ids]);
    });

    it("an exception from a pre-handler aborts the whole call before anything is written", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const existing = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e2, "u2")]);
      expect(existing.status).to.equal(BulkElementAspectsStatus.Success);
      const before = queryAspects();

      const mock = makeMockJsDb({ throwIn: "onInsert" });
      db.setIModelDb(mock.mockJsDb);
      expect(() => db.insertElementAspects([multiProps(e1, "new1"), multiProps(e2, "new2")])).to.throw("onInsert rejected");
      expect(mock.aspectCalls("onInsert").length).to.equal(1);
      expect(mock.elementCalls("onUpdated").length).to.equal(0);
      expect(queryAspects()).to.deep.equal(before);

      db.setIModelDb(makeMockJsDb({ throwIn: "onUpdate" }).mockJsDb);
      expect(() => db.updateElementAspects([{ ...multiProps(e1, "m1-new"), id: existing.ids[0] }])).to.throw("onUpdate rejected");
      expect(queryAspects()).to.deep.equal(before);

      db.setIModelDb(makeMockJsDb({ throwIn: "onDelete" }).mockJsDb);
      expect(() => db.deleteElementAspects(existing.ids)).to.throw("onDelete rejected");
      expect(queryAspects()).to.deep.equal(before);
    });
  });

  it("an empty array reports Success", () => {
    for (const result of [db.insertElementAspects([]), db.updateElementAspects([]), db.deleteElementAspects([])])
      expect(result).to.deep.include({ status: BulkElementAspectsStatus.Success, failures: [] });
  });
  it("bulk operations can be undone and redone", () => {
    const e1 = insertOwner();
    const e2 = insertOwner();
    db.saveChanges();
    const aspectsOf = () => queryAspects().filter((r) => r.element === e1 || r.element === e2).map((r) => `${r.className}:${r.strProp}`).sort();

    const inserted = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e1, "m1"), multiProps(e2, "m2")]);
    expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
    db.saveChanges();
    const afterInsert = aspectsOf();

    expect(db.updateElementAspects([uniqueProps(e1, "u1-new"), { ...multiProps(e2, "m2-new"), id: inserted.ids[2] }]).status).to.equal(BulkElementAspectsStatus.Success);
    db.saveChanges();
    const afterUpdate = aspectsOf();

    expect(db.deleteElementAspects([inserted.ids[0], inserted.ids[2]]).status).to.equal(BulkElementAspectsStatus.Success);
    db.saveChanges();
    const afterDelete = aspectsOf();
    expect(afterDelete).to.deep.equal([`${multiClass}:m1`]);

    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal(afterUpdate);
    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal(afterInsert);
    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal([]);

    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal(afterInsert);
    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal(afterUpdate);
    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(aspectsOf()).to.deep.equal(afterDelete);
  });
});
