/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { DbResult, Guid } from "@itwin/core-bentley";
import { Code, PhysicalElementProps, ProfileOptions } from "@itwin/core-common";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, dbFileName, getOutputDir, iModelJsNative } from "./utils";

// Class, property and struct member names such as __proto__ and constructor are valid ECNames but also members of
// Object.prototype. Objects built through the N-API adapter must hold them as own properties, and must never write
// into Object.prototype or the Object function.
describe("Names that are members of Object.prototype", () => {
  const objectPrototypeMembers = Object.getOwnPropertyNames(Object.prototype);
  const objectFunctionMembers = Object.getOwnPropertyNames(Object);

  afterEach(() => {
    expect(Object.getOwnPropertyNames(Object.prototype)).to.deep.equal(objectPrototypeMembers);
    expect(Object.getOwnPropertyNames(Object)).to.deep.equal(objectFunctionMembers);
  });

  it("renders struct members as own properties of ECSql rows", () => {
    const outDir = getOutputDir();
    const fileName = path.join(outDir, "prototype-names.ecdb");
    if (fs.existsSync(fileName))
      fs.unlinkSync(fileName);
    const schemaPath = path.join(outDir, `${Guid.createValue()}.ecschema.xml`);
    fs.writeFileSync(schemaPath, `<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="PrototypeNames" alias="pn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECStructClass typeName="Inner">
          <ECProperty propertyName="Val" typeName="string"/>
        </ECStructClass>
        <ECStructClass typeName="Primitives">
          <ECProperty propertyName="__proto__" typeName="string"/>
          <ECProperty propertyName="constructor" typeName="string"/>
        </ECStructClass>
        <ECStructClass typeName="Nested">
          <ECStructProperty propertyName="__proto__" typeName="Inner"/>
        </ECStructClass>
        <ECEntityClass typeName="Thing">
          <ECStructProperty propertyName="A" typeName="Primitives"/>
          <ECStructProperty propertyName="B" typeName="Nested"/>
        </ECEntityClass>
      </ECSchema>`);

    const db = new iModelJsNative.ECDb();
    const insert = new iModelJsNative.ECSqlStatement();
    const select = new iModelJsNative.ECSqlStatement();
    try {
      db.createDb(fileName);
      db.importSchema(schemaPath);

      insert.prepare(db, "INSERT INTO pn.Thing(A, B) VALUES(?, ?)");
      insert.getBinder(1).bindMember("__proto__").bindString("first");
      insert.getBinder(1).bindMember("constructor").bindString("second");
      insert.getBinder(2).bindMember("__proto__").bindMember("Val").bindString("third");
      expect(insert.stepForInsert().status).to.equal(DbResult.BE_SQLITE_DONE);
      db.saveChanges();

      select.prepare(db, "SELECT A, B FROM pn.Thing");
      expect(select.step()).to.equal(DbResult.BE_SQLITE_ROW);
      const [a, b] = select.toRow({}).data;

      expect(Object.getPrototypeOf(a)).to.equal(Object.prototype);
      expect(Object.keys(a)).to.deep.equal(["__proto__", "constructor"]);
      expect(Object.getOwnPropertyDescriptor(a, "__proto__")?.value).to.equal("first");
      expect(Object.getOwnPropertyDescriptor(a, "constructor")?.value).to.equal("second");
      expect(Object.getPrototypeOf(b)).to.equal(Object.prototype);
      // eslint-disable-next-line @typescript-eslint/naming-convention
      expect(Object.getOwnPropertyDescriptor(b, "__proto__")?.value).to.deep.equal({ Val: "third" });
    } finally {
      insert.dispose();
      select.dispose();
      db.closeDb();
    }
  });

  describe("in an iModel", () => {
    let db: IModelJsNative.DgnDb;

    before(() => {
      db = openDgnDb(copyFile("PrototypeNames.bim", dbFileName), { profile: ProfileOptions.Upgrade, schemaLockHeld: true });
      db.importXmlSchemas([`<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="PrototypeNames" alias="pn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
          <ECEntityClass typeName="__proto__">
            <BaseClass>bis:PhysicalElement</BaseClass>
            <ECProperty propertyName="__proto__" typeName="string"/>
            <ECProperty propertyName="constructor" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="constructor"><BaseClass>__proto__</BaseClass></ECEntityClass>
          <ECStructClass typeName="toString"/>
        </ECSchema>`], { schemaLockHeld: true });
      db.saveChanges();
    });

    after(() => db.closeFile());

    it("returns schema props with all items as own properties", () => {
      const items = db.getSchemaProps("PrototypeNames").items!;

      expect(Object.getPrototypeOf(items)).to.equal(Object.prototype);
      expect(Object.keys(items).sort()).to.deep.equal(["__proto__", "constructor", "toString"]);
      expect(Object.getOwnPropertyDescriptor(items, "constructor")?.value).to.have.property("baseClass", "PrototypeNames.__proto__");
    });

    function firstId(ecsql: string) {
      const stmt = new iModelJsNative.ECSqlStatement();
      try {
        stmt.prepare(db, ecsql);
        expect(stmt.step()).to.equal(DbResult.BE_SQLITE_ROW);
        return stmt.getValue(0).getId();
      } finally {
        stmt.dispose();
      }
    }

    function newElementProps(): PhysicalElementProps {
      return {
        classFullName: "PrototypeNames:__proto__",
        model: firstId("SELECT ECInstanceId FROM bis.PhysicalModel LIMIT 1"),
        category: firstId("SELECT ECInstanceId FROM bis.SpatialCategory LIMIT 1"),
        code: Code.createEmpty(),
      };
    }

    it("returns class metadata with all properties as own properties", () => {
      const metaData = db.getECClassMetaData("PrototypeNames", "__proto__");
      expect(metaData.error).to.be.undefined;

      const properties: object = JSON.parse(metaData.result!).properties;
      expect(Object.getPrototypeOf(properties)).to.equal(Object.prototype);
      expect(Object.keys(properties).sort()).to.deep.equal(["__proto__", "constructor"]);
      expect(Object.getOwnPropertyDescriptor(properties, "__proto__")?.value).to.have.property("primitiveType");
    });

    it("inserts and loads element properties as own properties", () => {
      // Parsing JSON text makes `__proto__` an own key, and spreading keeps it as one.
      const props: PhysicalElementProps = {
        ...newElementProps(),
        ...JSON.parse(`{ "__proto__": "first", "constructor": "second" }`),
      };
      const id = db.insertElement(props);

      const element = db.getElement({ id });
      expect(Object.getPrototypeOf(element)).to.equal(Object.prototype);
      expect(Object.getOwnPropertyDescriptor(element, "__proto__")?.value).to.equal("first");
      expect(Object.getOwnPropertyDescriptor(element, "constructor")?.value).to.equal("second");
    });

    it("reads members that a class instance inherits from its class", () => {
      // Angles are read through their "degrees" member. Here it is a getter on the class, as on core-geometry's Angle.
      class AngleWithGetter {
        public get degrees() { return 90; }
      }
      const props: PhysicalElementProps = {
        ...newElementProps(),
        placement: { origin: [0, 0, 0], angles: { yaw: new AngleWithGetter() } },
      };
      const id = db.insertElement(props);

      const element = db.getElement({ id }) as PhysicalElementProps;
      expect(element.placement?.angles.yaw).to.be.closeTo(90, 1e-10);
    });

    it("updates an element from frozen props", () => {
      const id = db.insertElement(newElementProps());

      // updateElement writes classFullName and model into the props it receives. On a frozen object these writes are ignored.
      const props = { ...db.getElement({ id }), userLabel: "frozen" };
      db.updateElement(Object.freeze(props));
      expect(db.getElement({ id }).userLabel).to.equal("frozen");
    });
  });
});
