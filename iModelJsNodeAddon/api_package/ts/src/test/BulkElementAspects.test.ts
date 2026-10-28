/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64, Id64String, IModelStatus } from "@itwin/core-bentley";
import type { ElementAspectProps, SubjectProps } from "@itwin/core-common";
import { expect } from "chai";
import { openDgnDb } from ".";
import { BulkElementAspectsMode, BulkElementAspectsStatus, IModelJsNative } from "../NativeLibrary";
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
  <ECEntityClass typeName="TestMultiAspect">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
    <ECProperty propertyName="IntProp" typeName="int"/>
  </ECEntityClass>
  <ECEntityClass typeName="OtherMultiAspect">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
  </ECEntityClass>
</ECSchema>`;

const uniqueClass = "BulkAspectTest:TestUniqueAspect";
const otherUniqueClass = "BulkAspectTest:OtherUniqueAspect";
const multiClass = "BulkAspectTest:TestMultiAspect";
const otherMultiClass = "BulkAspectTest:OtherMultiAspect";

interface TestAspectProps extends ElementAspectProps {
  strProp?: string;
  intProp?: number;
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

    it("reports per-row failures and still inserts the valid rows", () => {
      const e1 = insertOwner();
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
      expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
      expect(result.failures.map((f) => f.index)).to.deep.equal([1, 2, 3, 4, 5, 6]);
      expect(result.failures.map((f) => f.status)).to.deep.equal([
        IModelStatus.InvalidId,
        IModelStatus.MissingId,
        IModelStatus.BadSchema,
        IModelStatus.WrongClass,
        IModelStatus.BadArg,
        IModelStatus.BadArg,
      ]);
      result.failures.forEach((f) => expect(f.message).to.be.a("string").and.not.empty);

      expect(Id64.isValidId64(result.ids[0])).to.be.true;
      expect(Id64.isValidId64(result.ids[7])).to.be.true;
      [1, 2, 3, 4, 5, 6].forEach((i) => expect(result.ids[i]).to.equal("0"));
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.strProp).sort()).to.deep.equal(["ok1", "ok2"]);
    });

    it("all rows failing reports Failed, and an empty array reports Success", () => {
      const all = db.insertElementAspects([multiProps("0", "a"), multiProps("0", "b")]);
      expect(all.status).to.equal(BulkElementAspectsStatus.Failed);
      expect(all.failures.length).to.equal(2);

      const empty = db.insertElementAspects([]);
      expect(empty.status).to.equal(BulkElementAspectsStatus.Success);
      expect(empty.failures).to.deep.equal([]);
      expect(empty.ids).to.deep.equal([]);
    });

    it("throws if the argument is not an array", () => {
      expect(() => db.insertElementAspects({} as any)).to.throw();
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

    it("reports aspects that cannot be written, still writing the owner's other aspects", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      expect(db.insertElementAspects([uniqueProps(e2, "o2", undefined, otherUniqueClass)]).status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "UPDATE", otherMultiClass, () => {
        withFailingAspectWrites("bis_ElementUniqueAspect", "UPDATE", otherUniqueClass, () => {
          const result = db.insertElementAspects([
            multiProps(e1, "m1"),
            multiProps(e1, "om1", undefined, otherMultiClass), // new multi aspect
            uniqueProps(e1, "ou1", undefined, otherUniqueClass), // new unique aspect
            uniqueProps(e2, "o2-new", undefined, otherUniqueClass), // upsert of an existing unique aspect
            uniqueProps(e1, "u1"),
          ]);
          expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
          expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([
            [1, IModelStatus.WriteError],
            [2, IModelStatus.WriteError],
            [3, IModelStatus.WriteError],
          ]);
          expect(result.ids.map((id) => Id64.isValidId64(id))).to.deep.equal([true, false, false, false, true]);
        });
      });

      expect(mock.aspectCalls("onInserted").map((c) => c.arg.props.strProp)).to.deep.equal(["m1", "u1"]);
      // Nothing is rolled back, so the failed new aspects may be left without their properties; the existing unique aspect is unchanged.
      const aspects = queryAspects().filter((r) => (r.element === e1 || r.element === e2) && r.strProp !== undefined).map((r) => `${r.className}:${r.strProp}`).sort();
      expect(aspects).to.deep.equal([`${multiClass}:m1`, `${otherUniqueClass}:o2`, `${uniqueClass}:u1`].sort());
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

    it("reports per-row failures; a failing row does not affect rows of other owners", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const e3 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e2, "u2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
        multiProps(e1, "noId"),
        { ...multiProps(e1, "wrongId"), id: "0xffffff0" },
        uniqueProps(e3, "noUniqueAspect"),
        uniqueProps("0xffffff0", "missingOwner"),
        uniqueProps(e2, "u2-new"),
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([
        [1, IModelStatus.InvalidId],
        [2, IModelStatus.NotFound],
        [3, IModelStatus.NotFound],
        [4, IModelStatus.MissingId],
      ]);

      expect(findAspect(inserted.ids[0])!.strProp).to.equal("m1-new");
      expect(findAspect(inserted.ids[1])!.strProp).to.equal("u2-new");
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
      expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
      expect(result.failures).to.deep.equal([{ index: 0, status: IModelStatus.NotFound, message: "aspect not found on the owning element" }]);
      expect(findAspect(inserted.ids[1])).to.deep.include({ element: e2, strProp: "m2" });
      expect(findAspect(inserted.ids[0])).to.deep.include({ element: e1, strProp: "m1-new" });
    });

    it("reports aspects that cannot be written; the other aspects of the same owner are still written", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "om1", undefined, otherMultiClass), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "UPDATE", otherMultiClass, () => {
        const result = db.updateElementAspects([
          { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
          { ...multiProps(e1, "om1-new", undefined, otherMultiClass), id: inserted.ids[1] },
          uniqueProps(e1, "u1-new"),
        ]);
        expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
        expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.WriteError]]);
      });

      expect(mock.aspectCalls("onUpdated").map((c) => c.arg.props.strProp)).to.deep.equal(["m1-new", "u1-new"]);
      expect(findAspect(inserted.ids[0])!.strProp).to.equal("m1-new");
      expect(findAspect(inserted.ids[1])!.strProp).to.equal("om1");
      expect(findAspect(inserted.ids[2])!.strProp).to.equal("u1-new");
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

    it("reports per-row failures for invalid, missing and non-aspect ids, and deletes duplicates once", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      const result = db.deleteElementAspects([
        inserted.ids[0],
        "not an id",
        "0xffffff0",
        e1, // an element, not an aspect
        inserted.ids[0], // duplicate
        42 as any,
      ]);
      expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([
        [1, IModelStatus.InvalidId],
        [2, IModelStatus.NotFound],
        [3, IModelStatus.NotFound],
        [5, IModelStatus.BadArg],
      ]);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);

      const empty = db.deleteElementAspects([]);
      expect(empty.status).to.equal(BulkElementAspectsStatus.Success);
    });

    it("reports aspects that cannot be deleted; the other aspects of the same owner are still deleted", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "om1", undefined, otherMultiClass), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      withFailingAspectWrites("bis_ElementMultiAspect", "DELETE", otherMultiClass, () => {
        const result = db.deleteElementAspects(inserted.ids);
        expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
        expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.WriteError]]);
      });

      expect(mock.aspectCalls("onDeleted").map((c) => c.arg.aspectId)).to.deep.equal([inserted.ids[0], inserted.ids[2]]);
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);
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

    it("reports aspects removed by a pre-delete handler as not found", () => {
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

      const result = db.deleteElementAspects(inserted.ids);
      expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.NotFound]]);
      expect(queryAspects().filter((r) => r.element === e1)).to.deep.equal([]);
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

  describe("FailFast mode", () => {
    const failFast = { mode: BulkElementAspectsMode.FailFast };

    it("insert writes nothing and calls no handlers if any row is invalid", () => {
      const e1 = insertOwner();
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      const before = queryAspects();

      const result = db.insertElementAspects([multiProps(e1, "ok"), multiProps("0", "invalidOwner"), { ...uniqueProps(e1, "badClass"), classFullName: "BulkAspectTest:DoesNotExist" }], failFast);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.InvalidId], [2, IModelStatus.BadSchema]]);
      expect(result.ids).to.deep.equal(["0", "0", "0"]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);
    });

    it("update writes nothing and calls no handlers if any row is invalid or not found", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e2, "u2")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      const before = queryAspects();

      const result = db.updateElementAspects([
        { ...multiProps(e1, "m1-new"), id: inserted.ids[0] },
        uniqueProps(e2, "u2-new"),
        { ...multiProps(e2, "wrongOwner"), id: inserted.ids[0] },
        { ...multiProps(e1, "missing"), id: "0xffffff0" },
      ], failFast);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[2, IModelStatus.NotFound], [3, IModelStatus.NotFound]]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);
    });

    it("delete writes nothing and calls no handlers if any id is invalid or not found", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e1, "u1")]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);
      const before = queryAspects();

      const result = db.deleteElementAspects([inserted.ids[0], "0xffffff0", "bad", inserted.ids[1]], failFast);
      expect(result.status).to.equal(BulkElementAspectsStatus.ValidationFailed);
      expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.NotFound], [2, IModelStatus.InvalidId]]);
      expect(mock.calls).to.deep.equal([]);
      expect(queryAspects()).to.deep.equal(before);
    });

    it("behaves like BestEffort when every row is valid", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const mock = makeMockJsDb();
      db.setIModelDb(mock.mockJsDb);

      const inserted = db.insertElementAspects([multiProps(e1, "m1"), uniqueProps(e2, "u2")], failFast);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(inserted.failures).to.deep.equal([]);
      inserted.ids.forEach((id) => expect(Id64.isValidId64(id)).to.be.true);

      const updated = db.updateElementAspects([{ ...multiProps(e1, "m1-new"), id: inserted.ids[0] }, uniqueProps(e2, "u2-new")], failFast);
      expect(updated.status).to.equal(BulkElementAspectsStatus.Success);
      expect(findAspect(inserted.ids[0])!.strProp).to.equal("m1-new");
      expect(findAspect(inserted.ids[1])!.strProp).to.equal("u2-new");

      const deleted = db.deleteElementAspects(inserted.ids, failFast);
      expect(deleted.status).to.equal(BulkElementAspectsStatus.Success);
      expect(inserted.ids.map((id) => findAspect(id))).to.deep.equal([undefined, undefined]);
      expect(mock.aspectCalls("onInserted").length).to.equal(2);
      expect(mock.aspectCalls("onUpdated").length).to.equal(2);
      expect(mock.aspectCalls("onDeleted").length).to.equal(2);
    });

    it("an empty array reports Success", () => {
      expect(db.insertElementAspects([], failFast).status).to.equal(BulkElementAspectsStatus.Success);
      expect(db.updateElementAspects([], failFast).status).to.equal(BulkElementAspectsStatus.Success);
      expect(db.deleteElementAspects([], failFast).status).to.equal(BulkElementAspectsStatus.Success);
    });

    it("failures while writing are still reported per row, with the other rows written", () => {
      const e1 = insertOwner();
      const inserted = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "om1", undefined, otherMultiClass)]);
      expect(inserted.status).to.equal(BulkElementAspectsStatus.Success);

      withFailingAspectWrites("bis_ElementMultiAspect", "DELETE", otherMultiClass, () => {
        const result = db.deleteElementAspects(inserted.ids, failFast);
        expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
        expect(result.failures.map((f) => [f.index, f.status])).to.deep.equal([[1, IModelStatus.WriteError]]);
      });
      expect(queryAspects().filter((r) => r.element === e1).map((r) => r.id)).to.deep.equal([inserted.ids[1]]);
    });

    it("BestEffort is the default", () => {
      const e1 = insertOwner();
      const rows = () => [multiProps(e1, "ok"), multiProps("0", "invalidOwner")];
      for (const options of [undefined, {}, { mode: undefined }, { mode: BulkElementAspectsMode.BestEffort }]) {
        const result = db.insertElementAspects(rows(), options);
        expect(result.status).to.equal(BulkElementAspectsStatus.PartialSuccess);
        expect(Id64.isValidId64(result.ids[0])).to.be.true;
      }
    });

    it("throws for an invalid mode or options", () => {
      const e1 = insertOwner();
      for (const options of [{ mode: 2 }, { mode: -1 }, { mode: 0.5 }, { mode: "1" }, { mode: null }, 5, "FailFast"] as any[]) {
        expect(() => db.insertElementAspects([multiProps(e1, "a")], options)).to.throw(TypeError);
        expect(() => db.updateElementAspects([uniqueProps(e1, "a")], options)).to.throw(TypeError);
        expect(() => db.deleteElementAspects(["0x1"], options)).to.throw(TypeError);
      }
      expect(queryAspects().filter((r) => r.element === e1)).to.deep.equal([]);
    });
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
