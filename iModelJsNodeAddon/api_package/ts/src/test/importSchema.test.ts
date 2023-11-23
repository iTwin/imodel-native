/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert, expect } from "chai";
import * as fs from "fs";
import * as path from "path";
import { getOutputDir, iModelJsNative } from "./utils";
import { Guid, OpenMode } from "@itwin/core-bentley";
import { Code, ElementProps } from "@itwin/core-common";
import { clearRegistry, loadMetaData } from "./loadMetaData";

const testSchemaXmlV10 = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="Test" alias="test" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="coreCA"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="TestInformationRecordElement">
        <BaseClass>bis:InformationRecordElement</BaseClass>
        <ECProperty propertyName="property1" typeName="string"/>
    </ECEntityClass>
</ECSchema>
`;

const testInformationRecordElementV10: ElementProps & { property1: string } = {
  classFullName: "Test:TestInformationRecordElement",
  model: "0x1",
  code: Code.createEmpty(),
  property1: "Prop1",
};

const testSchemaXmlV11 = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="Test" alias="test" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="coreCA"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="TestInformationRecordElement">
        <BaseClass>bis:InformationRecordElement</BaseClass>
        <ECProperty propertyName="property1" typeName="string"/>
        <ECProperty propertyName="property2" typeName="string"/>
    </ECEntityClass>
</ECSchema>
`;

const fooSchemaXmlV10 = `<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="Foo" alias="foo" version="09.08.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="coreCA"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
    </ECCustomAttributes>
    <ECEntityClass typeName="FooInformationRecordElement">
        <BaseClass>bis:InformationRecordElement</BaseClass>
        <ECProperty propertyName="propertyFoo" typeName="string"/>
        <ECProperty propertyName="propertyBar" typeName="string"/>
    </ECEntityClass>
</ECSchema>
`;

const testInformationRecordElementV11: ElementProps & { property1: string, property2: string } = {
  classFullName: "Test:TestInformationRecordElement",
  model: "0x1",
  code: Code.createEmpty(),
  property1: "Prop1",
  property2: "Prop2",
};

describe("ImportSchema", () => {

  it("should import schema into a locally changed briefcase", () => {
    const dbpath = path.join(getOutputDir(), "ImportSchemaWithChanges.bim");
    if (fs.existsSync(dbpath))
      fs.unlinkSync(dbpath);

    const db = new iModelJsNative.DgnDb();
    db.createIModel(dbpath, { rootSubject: { name: "ImportSchemaTest" } });
    db.saveLocalValue("StandaloneEdit", JSON.stringify({ txns: true }));
    db.setITwinId(Guid.empty);
    db.resetBriefcaseId(0);
    db.saveChanges();
    db.closeIModel();

    db.openIModel(dbpath, OpenMode.ReadWrite);

    db.importXmlSchemas([testSchemaXmlV10], { schemaLockHeld: true });

    db.saveChanges();

    const testInfoClassV10 = loadMetaData(db, "Test:TestInformationRecordElement");

    assert.equal(testInfoClassV10.baseClasses.length, 1);
    expect("BisCore:InformationRecordElement").eq(testInfoClassV10.baseClasses[0]);
    expect(testInfoClassV10.properties).hasOwnProperty("property1");
    expect(testInfoClassV10.properties).not.hasOwnProperty("property2");

    const el1Id = db.insertElement(testInformationRecordElementV10);
    const el1v10 = db.getElement({ id: el1Id });
    expect(el1v10).has.property("property1");
    expect(el1v10).not.has.property("property2");
    expect((el1v10 as any).property1).eq(testInformationRecordElementV10.property1);

    db.saveChanges();

    assert.isTrue(db.hasPendingTxns());

    // Import new version of test schema and the new foo schema.
    // Verify that the import is not rejected because of local changes.
    db.importXmlSchemas([testSchemaXmlV11, fooSchemaXmlV10], { schemaLockHeld: true });

    clearRegistry();

    // Verify that Test:TestInformationRecordElement was updated, specifically that property2 was added to it
    const testInfoClassV11 = loadMetaData(db, "Test:TestInformationRecordElement");

    assert.equal(testInfoClassV10.baseClasses.length, 1);
    expect("BisCore:InformationRecordElement").eq(testInfoClassV10.baseClasses[0]);
    expect(testInfoClassV11.properties).hasOwnProperty("property1");
    expect(testInfoClassV11.properties).hasOwnProperty("property2");

    const el1v11 = db.getElement({ id: el1Id });
    expect(el1v11).has.property("property1");
    expect(el1v11).not.has.property("property2");
    expect((el1v11 as any).property1).eq(testInformationRecordElementV10.property1);

    const el2Id = db.insertElement(testInformationRecordElementV11);
    const el2v11 = db.getElement({ id: el2Id });
    expect(el2v11).has.property("property1");
    expect(el2v11).has.property("property2");
    expect((el2v11 as any).property1).eq(testInformationRecordElementV11.property1);
    expect((el2v11 as any).property2).eq(testInformationRecordElementV11.property2);

    // Verify that Foo schema was added to the db
    const fooInfoClassV98 = loadMetaData(db, "Foo:FooInformationRecordElement");
    assert.equal(fooInfoClassV98.baseClasses.length, 1);
    expect("BisCore:InformationRecordElement").eq(fooInfoClassV98.baseClasses[0]);
    expect(fooInfoClassV98.properties).hasOwnProperty("propertyFoo");
    expect(fooInfoClassV98.properties).hasOwnProperty("propertyBar");

    db.saveChanges();
  });
});
