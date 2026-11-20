/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { DbResult, Id64, Id64String, IModelStatus } from "@itwin/core-bentley";
import type { ElementAspectProps, SubjectProps } from "@itwin/core-common";
import { expect } from "chai";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, iModelJsNative } from "./utils";

const testSchemaXml = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BulkAspectTest" alias="bat" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
  <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
  <ECEntityClass typeName="TestUniqueAspect">
    <BaseClass>bis:ElementUniqueAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
    <ECProperty propertyName="IntProp" typeName="int"/>
  </ECEntityClass>
  <ECEntityClass typeName="DerivedUniqueAspect">
    <BaseClass>TestUniqueAspect</BaseClass>
    <ECProperty propertyName="DerivedProp" typeName="string"/>
  </ECEntityClass>
  <ECStructClass typeName="TestStruct">
    <ECProperty propertyName="Label" typeName="string"/>
    <ECProperty propertyName="Num" typeName="long"/>
  </ECStructClass>
  <ECEntityClass typeName="TestMultiAspect">
    <BaseClass>bis:ElementMultiAspect</BaseClass>
    <ECProperty propertyName="StrProp" typeName="string"/>
    <ECProperty propertyName="IntProp" typeName="int"/>
    <ECProperty propertyName="LongProp" typeName="long"/>
    <ECProperty propertyName="BinProp" typeName="binary"/>
    <ECProperty propertyName="GuidProp" typeName="binary" extendedTypeName="BeGuid"/>
    <ECStructArrayProperty propertyName="StructArr" typeName="TestStruct"/>
    <ECStructProperty propertyName="StructProp" typeName="TestStruct"/>
    <ECArrayProperty propertyName="LongArr" typeName="long"/>
  </ECEntityClass>
  <ECEntityClass typeName="DerivedMultiAspect">
    <BaseClass>TestMultiAspect</BaseClass>
    <ECProperty propertyName="DerivedProp" typeName="string"/>
  </ECEntityClass>
</ECSchema>`;

const uniqueClass = "BulkAspectTest:TestUniqueAspect";
const derivedUniqueClass = "BulkAspectTest:DerivedUniqueAspect";
const multiClass = "BulkAspectTest:TestMultiAspect";
const derivedMultiClass = "BulkAspectTest:DerivedMultiAspect";

interface TestAspectProps extends ElementAspectProps {
  strProp?: string;
  intProp?: number;
  derivedProp?: string;
  longProp?: number | string;
  structArr?: Array<{ label: string } | null>;
  structProp?: { label?: string, num?: number | string };
  longArr?: Array<number | string>;
}

interface HookCall {
  className: string;
  method: string;
  arg: any;
}

/** A mock JS IModelDb whose classes record every handler call, pass it to `onCall`, and throw from `throwIn`. */
function makeMockJsDb(throwIn?: string, onCall?: (call: HookCall) => void) {
  const calls: HookCall[] = [];
  const mockJsDb: any = {
    getJsClass: (className: string) => new Proxy({}, {
      get: (_target, method) => {
        if (typeof method !== "string")
          return undefined;
        return (arg: any) => {
          calls.push({ className, method, arg });
          onCall?.({ className, method, arg });
          if (throwIn === method)
            throw new Error(`${method} rejected`);
        };
      },
    }),
  };
  return { mockJsDb, calls };
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

  function uniqueProps(element: Id64String, strProp: string, intProp?: number, classFullName = uniqueClass): TestAspectProps {
    return { classFullName, element: { id: element }, strProp, ...(intProp !== undefined ? { intProp } : {}) };
  }

  function multiProps(element: Id64String, strProp: string, intProp?: number, classFullName = multiClass): TestAspectProps {
    return { classFullName, element: { id: element }, strProp, ...(intProp !== undefined ? { intProp } : {}) };
  }

  /** Reads an aspect of any test class, or returns undefined if it does not exist. */
  function readAspect(id: Id64String): any {
    const stmt = new iModelJsNative.ECSqlStatement();
    const ecsql = "SELECT ec_classname(ECClassId, 's:c'), Element.Id, ec_classname(Element.RelECClassId, 's:c'), StrProp, IntProp FROM bat.TestUniqueAspect WHERE ECInstanceId=:id"
      + " UNION ALL SELECT ec_classname(ECClassId, 's:c'), Element.Id, ec_classname(Element.RelECClassId, 's:c'), StrProp, IntProp FROM bat.TestMultiAspect WHERE ECInstanceId=:id";
    expect(stmt.prepare(db, ecsql).status).to.equal(DbResult.BE_SQLITE_OK);
    stmt.getBinder("id").bindId(id);
    let row: any;
    if (DbResult.BE_SQLITE_ROW === stmt.step()) {
      row = { className: stmt.getValue(0).getString(), element: stmt.getValue(1).getId(), relClassName: stmt.getValue(2).getString() };
      if (!stmt.getValue(3).isNull())
        row.strProp = stmt.getValue(3).getString();
      if (!stmt.getValue(4).isNull())
        row.intProp = stmt.getValue(4).getInt();
    }
    stmt.dispose();
    return row;
  }

  function readDerivedProp(className: string, id: Id64String): string | undefined {
    const stmt = new iModelJsNative.ECSqlStatement();
    expect(stmt.prepare(db, `SELECT DerivedProp FROM ${className.replace(":", ".")} WHERE ECInstanceId=?`).status).to.equal(DbResult.BE_SQLITE_OK);
    stmt.getBinder(1).bindId(id);
    expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
    const value = stmt.getValue(0).isNull() ? undefined : stmt.getValue(0).getString();
    stmt.dispose();
    return value;
  }

  function lastMod(elementId: Id64String): string {
    const stmt = new iModelJsNative.ECSqlStatement();
    expect(stmt.prepare(db, "SELECT LastMod FROM bis.Element WHERE ECInstanceId=?").status).to.equal(DbResult.BE_SQLITE_OK);
    stmt.getBinder(1).bindId(elementId);
    expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
    const value = stmt.getValue(0).getDateTime();
    stmt.dispose();
    return value;
  }

  describe("insertElementAspects", () => {
    it("inserts unique and multi aspects and returns their ids in input order", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const ids = db.insertElementAspects([uniqueProps(e1, "u1", 1), multiProps(e1, "m1"), multiProps(e2, "m2", 2), multiProps(e1, "m1b")]);
      expect(ids.length).to.equal(4);
      ids.forEach((id) => expect(Id64.isValidId64(id)).to.be.true);
      expect(new Set(ids).size).to.equal(4);
      expect(readAspect(ids[0])).to.deep.equal({ className: uniqueClass, element: e1, relClassName: "BisCore:ElementOwnsUniqueAspect", strProp: "u1", intProp: 1 });
      expect(readAspect(ids[1])).to.deep.equal({ className: multiClass, element: e1, relClassName: "BisCore:ElementOwnsMultiAspects", strProp: "m1" });
      expect(readAspect(ids[2])).to.deep.equal({ className: multiClass, element: e2, relClassName: "BisCore:ElementOwnsMultiAspects", strProp: "m2", intProp: 2 });
      expect(readAspect(ids[3])).to.deep.equal({ className: multiClass, element: e1, relClassName: "BisCore:ElementOwnsMultiAspects", strProp: "m1b" });
    });

    it("an empty array does nothing", () => {
      expect(db.insertElementAspects([])).to.deep.equal([]);
    });

    it("a unique aspect replaces the existing one, keeping its id when the class is the same", () => {
      const e1 = insertOwner();
      const [first] = db.insertElementAspects([uniqueProps(e1, "first", 1)]);
      const [second] = db.insertElementAspects([uniqueProps(e1, "second")]);
      expect(second).to.equal(first);
      expect(readAspect(first)).to.deep.include({ strProp: "second" }).and.not.have.property("intProp");

      // A subclass aspect is replaced by an aspect of its base class, which gets a new id.
      const e2 = insertOwner();
      const [derived] = db.insertElementAspects([uniqueProps(e2, "derived", undefined, derivedUniqueClass)]);
      const [base] = db.insertElementAspects([uniqueProps(e2, "base")]);
      expect(base).to.not.equal(derived);
      expect(readAspect(derived)).to.be.undefined;
      expect(readAspect(base)).to.deep.include({ className: uniqueClass, strProp: "base" });

      // The same unique aspect twice in one call: the last props win.
      const e3 = insertOwner();
      const twice = db.insertElementAspects([uniqueProps(e3, "a"), uniqueProps(e3, "b")]);
      expect(twice[1]).to.equal(twice[0]);
      expect(readAspect(twice[0])).to.deep.include({ strProp: "b" });
    });

    it("throws on the first invalid props, before writing any aspect", () => {
      const e1 = insertOwner();
      db.saveChanges();
      const expectThrows = (bad: any, message: RegExp) => {
        expect(() => db.insertElementAspects([multiProps(e1, "ok"), bad, multiProps(e1, "never")])).to.throw(message);
        const stmt = new iModelJsNative.ECSqlStatement();
        stmt.prepare(db, "SELECT StrProp FROM bat.TestMultiAspect WHERE Element.Id=?");
        stmt.getBinder(1).bindId(e1);
        const values: string[] = [];
        while (DbResult.BE_SQLITE_ROW === stmt.step())
          values.push(stmt.getValue(0).getString());
        stmt.dispose();
        expect(values).to.deep.equal([]);
        db.abandonChanges();
      };
      expectThrows(42, /insertElementAspects: props\[1\] is not an object/);
      expectThrows({ ...multiProps(e1, "x"), classFullName: "BisCore:Subject" }, /props\[1\]\.classFullName is not an ElementMultiAspect or ElementUniqueAspect class/);
      expectThrows({ ...multiProps(e1, "x"), classFullName: "BulkAspectTest:Missing" }, /props\[1\]\.classFullName/);
      expectThrows({ ...multiProps(e1, "x"), element: { id: "not an id" } }, /props\[1\]\.element is not a valid element id/);
      expectThrows(multiProps("0xfffffff", "x"), /owning element 0xfffffff of input 1 does not exist/);
    });

    it("throws for an argument that is not an array", () => {
      expect(() => db.insertElementAspects({} as any)).to.throw("Invalid argument given to insertElementAspects");
    });

    it("accepts a number or a decimal or hex string for a long property, on insert and update", () => {
      const e1 = insertOwner();
      const readLong = (id: Id64String) => {
        const stmt = new iModelJsNative.ECSqlStatement();
        expect(stmt.prepare(db, "SELECT LongProp FROM bat.TestMultiAspect WHERE ECInstanceId=?").status).to.equal(DbResult.BE_SQLITE_OK);
        stmt.getBinder(1).bindId(id);
        expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
        const value = stmt.getValue(0).getInt64();
        stmt.dispose();
        return value;
      };
      const ids = db.insertElementAspects([1, "0x2d", "123"].map((longProp) => ({ ...multiProps(e1, "m"), longProp })));
      expect(ids.map(readLong)).to.deep.equal([1, 45, 123]);
      db.updateElementAspects([{ ...multiProps(e1, "m"), id: ids[0], longProp: "0x30" } as TestAspectProps]);
      expect(readLong(ids[0])).to.equal(48);
    });

    it("accepts a Uint8Array or a base64 string for a binary property", () => {
      const e1 = insertOwner();
      const bytes = new Uint8Array([2, 4, 6, 8]);
      const guid = Uint8Array.from({ length: 16 }, (_, i) => i);
      const toBase64 = (b: Uint8Array) => `encoding=base64;${Buffer.from(b).toString("base64")}`;
      const props = [
        { ...multiProps(e1, "m"), binProp: bytes, guidProp: guid },
        { ...multiProps(e1, "m"), binProp: toBase64(bytes), guidProp: toBase64(guid) },
      ];
      const ids = db.insertElementAspects(props);
      const stmt = new iModelJsNative.ECSqlStatement();
      expect(stmt.prepare(db, "SELECT BinProp, GuidProp FROM bat.TestMultiAspect WHERE ECInstanceId=?").status).to.equal(DbResult.BE_SQLITE_OK);
      for (const id of ids) {
        stmt.reset();
        stmt.getBinder(1).bindId(id);
        expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
        expect(Array.from(stmt.getValue(0).getBlob())).to.deep.equal(Array.from(bytes));
        expect(Array.from(stmt.getValue(1).getBlob())).to.deep.equal(Array.from(guid));
      }
      stmt.dispose();
    });

    it("keeps a null struct array entry as a null element", () => {
      const e1 = insertOwner();
      const props: TestAspectProps = { ...multiProps(e1, "m"), structArr: [{ label: "a" }, null, { label: "c" }] };
      const [id] = db.insertElementAspects([props]);
      const stmt = new iModelJsNative.ECSqlStatement();
      expect(stmt.prepare(db, "SELECT StructArr FROM bat.TestMultiAspect WHERE ECInstanceId=?").status).to.equal(DbResult.BE_SQLITE_OK);
      stmt.getBinder(1).bindId(id);
      expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
      const labels: Array<string | null> = [];
      const elements = stmt.getValue(0).getArrayIterator();
      while (elements.moveNext()) {
        const element = elements.getCurrent();
        const members = element.getStructIterator();
        labels.push(element.isNull() || !members.moveNext() ? null : members.getCurrent().getString());
      }
      stmt.dispose();
      expect(labels).to.deep.equal(["a", null, "c"]);
    });

    it("accepts strings for longs in structs and arrays, and with incremental keeps the struct members that are not in the props", () => {
      const e1 = insertOwner();
      const read = (aspectId: Id64String) => {
        const stmt = new iModelJsNative.ECSqlStatement();
        expect(stmt.prepare(db, "SELECT StructProp.Label, StructProp.Num, LongArr FROM bat.TestMultiAspect WHERE ECInstanceId=?").status).to.equal(DbResult.BE_SQLITE_OK);
        stmt.getBinder(1).bindId(aspectId);
        expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
        const longArr: number[] = [];
        const elements = stmt.getValue(2).getArrayIterator();
        while (elements.moveNext())
          longArr.push(elements.getCurrent().getInt64());
        const value = { label: stmt.getValue(0).getString(), num: stmt.getValue(1).getInt64(), longArr };
        stmt.dispose();
        return value;
      };
      const [id] = db.insertElementAspects([{ ...multiProps(e1, "m"), structProp: { label: "a", num: "0x2d" }, longArr: ["0x1", "-5", 7] } as TestAspectProps]);
      expect(read(id)).to.deep.equal({ label: "a", num: 45, longArr: [1, -5, 7] });
      db.updateElementAspects([{ ...multiProps(e1, "m"), id, structProp: { label: "b" } } as TestAspectProps], true);
      expect(read(id)).to.deep.equal({ label: "b", num: 45, longArr: [1, -5, 7] });
    });

    it("ignores the EC name Element for the owner, like the single-aspect functions", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      // eslint-disable-next-line @typescript-eslint/naming-convention
      const [id] = db.insertElementAspects([{ ...multiProps(e1, "m"), element: { id: e1, relClassName: "BisCore:ElementOwnsMultiAspects" }, Element: { id: e2 } } as TestAspectProps]);
      expect(readAspect(id)).to.deep.include({ element: e1, relClassName: "BisCore:ElementOwnsMultiAspects" });
      // eslint-disable-next-line @typescript-eslint/naming-convention
      db.updateElementAspects([{ ...multiProps(e1, "m-new"), id, Element: { id: e2 } } as TestAspectProps]);
      expect(readAspect(id)).to.deep.include({ element: e1, strProp: "m-new" });
    });
  });

  describe("updateElementAspects", () => {
    it("sets the properties that are not in the props to null and keeps the owner", () => {
      const e1 = insertOwner();
      const ids = db.insertElementAspects([uniqueProps(e1, "u1", 1), multiProps(e1, "m1", 2)]);
      db.updateElementAspects([uniqueProps(e1, "u1-new"), { classFullName: multiClass, element: { id: e1 }, id: ids[1], intProp: 20 } as TestAspectProps]);
      expect(readAspect(ids[0])).to.deep.equal({ className: uniqueClass, element: e1, relClassName: "BisCore:ElementOwnsUniqueAspect", strProp: "u1-new" });
      expect(readAspect(ids[1])).to.deep.equal({ className: multiClass, element: e1, relClassName: "BisCore:ElementOwnsMultiAspects", intProp: 20 });
    });

    it("with incremental, updates only the given properties of multi aspects (by id) and unique aspects (by element)", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const ids = db.insertElementAspects([uniqueProps(e1, "u1", 1), multiProps(e1, "m1", 2), multiProps(e2, "m2", 3)]);
      db.updateElementAspects([
        uniqueProps(e1, "u1-new"),
        { ...multiProps(e1, "m1-new"), id: ids[1] },
        { classFullName: multiClass, element: { id: e2 }, id: ids[2], intProp: 30 } as TestAspectProps,
      ], true);
      expect(readAspect(ids[0])).to.deep.include({ strProp: "u1-new", intProp: 1 });
      expect(readAspect(ids[1])).to.deep.include({ strProp: "m1-new", intProp: 2 });
      expect(readAspect(ids[2])).to.deep.include({ strProp: "m2", intProp: 30 });
    });

    it("accepts a base class of the stored aspect", () => {
      const e1 = insertOwner();
      const ids = db.insertElementAspects([
        { ...uniqueProps(e1, "u", undefined, derivedUniqueClass), derivedProp: "du" } as TestAspectProps,
        { ...multiProps(e1, "m", undefined, derivedMultiClass), derivedProp: "dm" } as TestAspectProps,
      ]);
      db.updateElementAspects([uniqueProps(e1, "u-new"), { ...multiProps(e1, "m-new"), id: ids[1] }], true);
      expect(readAspect(ids[0])).to.deep.include({ className: derivedUniqueClass, strProp: "u-new" });
      expect(readAspect(ids[1])).to.deep.include({ className: derivedMultiClass, strProp: "m-new" });
      expect(readDerivedProp(derivedUniqueClass, ids[0])).to.equal("du");
      expect(readDerivedProp(derivedMultiClass, ids[1])).to.equal("dm");
    });

    it("throws on the first aspect that cannot be found", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const [multiId] = db.insertElementAspects([multiProps(e1, "m1")]);
      db.saveChanges();
      expect(() => db.updateElementAspects([{ ...multiProps(e1, "m1-new"), id: multiId }, uniqueProps(e1, "u1")]))
        .to.throw(/updateElementAspects: the aspect of props\[1\] does not exist on element/);
      expect(readAspect(multiId)).to.deep.include({ strProp: "m1" }); // nothing is written, but the pre-handlers ran, so the caller must abandon the txn
      db.abandonChanges();

      expect(() => db.updateElementAspects([{ ...multiProps(e1, "x"), id: e2 }])).to.throw(/the aspect of props\[0\] does not exist/);
      expect(() => db.updateElementAspects([multiProps(e1, "x")])).to.throw(/props\[0\]\.id is not a valid aspect id/);
      expect(() => db.updateElementAspects([{ ...multiProps(e1, "x"), id: `${multiId}garbage` }])).to.throw(/props\[0\]\.id is not a valid aspect id/);
      expect(readAspect(multiId)).to.deep.include({ strProp: "m1" });
    });

    it("keeps a multi aspect on its owner and updates the element in the props when they differ, like the single-aspect update", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const [multiId] = db.insertElementAspects([multiProps(e1, "m1", 1)]);
      db.saveChanges();
      const before = lastMod(e2);
      for (const start = Date.now(); Date.now() - start < 5;) {
        // wait, so that LastMod can change
      }
      db.updateElementAspects([{ ...multiProps(e2, "m1-new"), id: multiId }], true);
      expect(readAspect(multiId)).to.deep.include({ element: e1, strProp: "m1-new", intProp: 1 });
      expect(lastMod(e2)).to.not.equal(before);
    });

    it("throws for an aspect with a native handler, like the single-aspect update", () => {
      const e1 = insertOwner();
      const props = { classFullName: "BisCore:TextAnnotationData", element: { id: e1 } };
      db.insertElementAspects([props]);
      expect(() => db.updateElementAspects([props])).to.throw(/updateElementAspects: the aspect of props\[0\] has a native handler/);
    });
  });

  describe("deleteElementAspects", () => {
    it("deletes unique and multi aspects", () => {
      const e1 = insertOwner();
      const ids = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e1, "m1"), multiProps(e1, "m2")]);
      db.deleteElementAspects([ids[0], ids[2]]);
      expect(readAspect(ids[0])).to.be.undefined;
      expect(readAspect(ids[1])).to.not.be.undefined;
      expect(readAspect(ids[2])).to.be.undefined;
    });

    it("throws on the first id that is invalid or does not identify an aspect", () => {
      const e1 = insertOwner();
      const ids = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      db.saveChanges();
      expect(() => db.deleteElementAspects([ids[0], ids[0]])).to.throw(new RegExp(`deleteElementAspects: aspect ${ids[0]} of ids\\[1\\] does not exist`));
      expect(readAspect(ids[0])).to.not.be.undefined; // nothing is deleted, but the pre-handlers ran, so the caller must abandon the txn
      db.abandonChanges();

      for (const bad of [`${ids[1]}garbage`, `0x0${ids[1].slice(2)}`, "0x0", 42])
        expect(() => db.deleteElementAspects([bad as any])).to.throw(/ids\[0\] is not a valid aspect id/);
      expect(() => db.deleteElementAspects([e1])).to.throw(/does not exist/);
      expect(readAspect(ids[0])).to.not.be.undefined;
      expect(readAspect(ids[1])).to.not.be.undefined;
    });
  });

  describe("handlers", () => {
    it("calls the pre-handlers, then updates each owner once, then calls the post-handlers", () => {
      const e1 = insertOwner();
      const e2 = insertOwner();
      const { mockJsDb, calls } = makeMockJsDb();
      const relevant = () => calls.filter((c) => c.className.startsWith("BulkAspectTest:") || (c.className === "BisCore:Subject" && c.method === "onUpdated"))
        .map((c) => `${c.className === "BisCore:Subject" ? c.arg.id : c.className}.${c.method}`);

      db.setIModelDb(mockJsDb);
      const ids = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e2, "m2"), multiProps(e1, "m3")]);
      expect(relevant()).to.deep.equal([
        `${multiClass}.onInsert`, `${multiClass}.onInsert`, `${multiClass}.onInsert`,
        `${e1}.onUpdated`, `${e2}.onUpdated`,
        `${multiClass}.onInserted`, `${multiClass}.onInserted`, `${multiClass}.onInserted`,
      ]);
      const insertArg = calls.find((c) => c.method === "onInsert")!.arg;
      expect(insertArg.props.strProp).to.equal("m1");
      expect(insertArg.model).to.equal("0x1");
      expect(insertArg.iModel).to.equal(mockJsDb);

      calls.length = 0;
      db.updateElementAspects([{ ...multiProps(e2, "m2-new"), id: ids[1] }]);
      expect(relevant()).to.deep.equal([`${multiClass}.onUpdate`, `${e2}.onUpdated`, `${multiClass}.onUpdated`]);

      calls.length = 0;
      db.deleteElementAspects([ids[2], ids[0]]);
      expect(relevant()).to.deep.equal([`${multiClass}.onDelete`, `${multiClass}.onDelete`, `${e1}.onUpdated`, `${multiClass}.onDeleted`, `${multiClass}.onDeleted`]);
      expect(calls.find((c) => c.method === "onDelete")!.arg).to.deep.include({ aspectId: ids[2], element: e1, model: "0x1" });
    });

    it("writes the aspects of an owner after its onUpdated handler, like the update of an element with staged aspects", () => {
      const e1 = insertOwner();
      const [id] = db.insertElementAspects([multiProps(e1, "old")]);
      const seen: string[] = [];
      const { mockJsDb } = makeMockJsDb(undefined, (c) => {
        if (c.method === "onUpdated")
          seen.push(`${c.className === "BisCore:Subject" ? "owner" : "aspect"}:${readAspect(id).strProp}`);
      });
      db.setIModelDb(mockJsDb);
      db.updateElementAspects([{ ...multiProps(e1, "new"), id }]);
      expect(seen).to.deep.equal(["owner:old", "aspect:new"]);
    });

    it("an exception from a pre-handler propagates and stops the operation", () => {
      const e1 = insertOwner();
      const [id] = db.insertElementAspects([multiProps(e1, "m1")]);
      db.setIModelDb(makeMockJsDb("onInsert").mockJsDb);
      expect(() => db.insertElementAspects([multiProps(e1, "m2")])).to.throw("onInsert rejected");
      db.setIModelDb(makeMockJsDb("onUpdate").mockJsDb);
      expect(() => db.updateElementAspects([{ ...multiProps(e1, "m1-new"), id }])).to.throw("onUpdate rejected");
      db.setIModelDb(makeMockJsDb("onDelete").mockJsDb);
      expect(() => db.deleteElementAspects([id])).to.throw("onDelete rejected");
      expect(readAspect(id)).to.deep.include({ strProp: "m1" });
    });

    it("throws if a pre-handler deletes an aspect to be updated or deleted", () => {
      const e1 = insertOwner();
      const ids = db.insertElementAspects([multiProps(e1, "m1"), multiProps(e1, "m2")]);
      db.saveChanges();
      // The handler of the second aspect deletes the first one, which has already been found.
      const deleteFirstFrom = (method: string) => makeMockJsDb(undefined, (c) => {
        if (c.method === method && (c.arg.aspectId ?? c.arg.props?.id) === ids[1])
          db.deleteElementAspects([ids[0]]);
      }).mockJsDb;

      db.setIModelDb(deleteFirstFrom("onUpdate"));
      expect(() => db.updateElementAspects([{ ...multiProps(e1, "x"), id: ids[0] }, { ...multiProps(e1, "y"), id: ids[1] }]))
        .to.throw(new RegExp(`updateElementAspects: aspect ${ids[0]} of props\\[0\\] no longer exists`));
      db.abandonChanges();

      db.setIModelDb(deleteFirstFrom("onDelete"));
      expect(() => db.deleteElementAspects([ids[0], ids[1]])).to.throw(new RegExp(`deleteElementAspects: aspect ${ids[0]} of ids\\[0\\] no longer exists`));
    });
  });

  it("updates the LastMod of each owning element", () => {
    const e1 = insertOwner();
    db.saveChanges();
    const before = lastMod(e1);
    for (const start = Date.now(); Date.now() - start < 5;) {
      // wait, so that LastMod can change
    }
    db.insertElementAspects([multiProps(e1, "m1")]);
    expect(lastMod(e1)).to.not.equal(before);
  });

  it("can be undone and redone", () => {
    const e1 = insertOwner();
    const e2 = insertOwner();
    db.saveChanges();
    const aspectsOf = (aspectIds: Id64String[]) => aspectIds.map((id) => readAspect(id)?.strProp);

    const ids = db.insertElementAspects([uniqueProps(e1, "u1"), multiProps(e1, "m1"), multiProps(e2, "m2")]);
    db.saveChanges();
    db.updateElementAspects([uniqueProps(e1, "u1-new"), { ...multiProps(e2, "m2-new"), id: ids[2] }]);
    db.saveChanges();
    db.deleteElementAspects([ids[0], ids[2]]);
    db.saveChanges();
    expect(aspectsOf(ids)).to.deep.equal([undefined, "m1", undefined]);

    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf(ids)).to.deep.equal(["u1-new", "m1", "m2-new"]);
    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf(ids)).to.deep.equal(["u1", "m1", "m2"]);
    expect(db.reverseTxns(1)).to.equal(IModelStatus.Success);
    expect(aspectsOf(ids)).to.deep.equal([undefined, undefined, undefined]);

    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(db.reinstateTxn()).to.equal(IModelStatus.Success);
    expect(aspectsOf(ids)).to.deep.equal([undefined, "m1", undefined]);
  });
});
