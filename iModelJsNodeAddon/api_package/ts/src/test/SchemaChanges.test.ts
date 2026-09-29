/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { ProfileOptions } from "@itwin/core-common";
import { assert } from "chai";
import * as fs from "fs-extra";
import * as path from "path";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { copyFile, getOutputDir, iModelJsNative } from "./utils";

describe("Schema Changes Tests", () => {
  const seedUri = path.join(getOutputDir(), "SchemaChangesTestSeed.bim");
  const schemaDir = path.join(getOutputDir(), "SchemaChangesTest-schemas");
  let dgndb: IModelJsNative.DgnDb;

  before((done) => {
    const seedDb = new iModelJsNative.DgnDb();
    if (fs.existsSync(seedUri)) {
      fs.removeSync(seedUri);
    }
    seedDb.createIModel(seedUri, { rootSubject: { name: "test file" } });
    resetSchemaDirectory();
    createBasicSchemaSet();
    seedDb.importSchemas([path.join(schemaDir, "SchemaA.ecschema.xml"),
      path.join(schemaDir, "SchemaB.ecschema.xml"),
      path.join(schemaDir, "SchemaC.ecschema.xml"),
    ]);
    seedDb.saveChanges();
    seedDb.closeFile();
    done();
  })

  beforeEach((done) => {
    const testDbPath = copyFile("SchemaChangesTest.bim", seedUri);
    dgndb = openDgnDb(testDbPath, { profile: ProfileOptions.None, schemaLockHeld: true });
    resetSchemaDirectory();
    done();
  });

  afterEach((done) => {
    if (dgndb && dgndb.isOpen()) {
      dgndb.closeFile();
    }
    done();
  });

  const resetSchemaDirectory = (): string => {
    if (fs.existsSync(schemaDir)) {
      fs.removeSync(schemaDir);
    }
    fs.mkdirSync(schemaDir, { recursive: true });
    return schemaDir;
  };

  const writeToSchemaDirectory = (schemaXml: string, fileName: string): string => {
    const filePath = path.join(schemaDir, fileName);
    fs.writeFileSync(filePath, schemaXml);
    return filePath;
  };

  //base schemas to start with
  const schemaA105 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaA" alias="a" version="01.00.05" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="SchemaB" version="01.00.05" alias="b"/>
        <ECEntityClass typeName="ElementA">
            <BaseClass>b:ElementB</BaseClass>
            <ECProperty propertyName="PropertyA" typeName="string" />
        </ECEntityClass>
    </ECSchema>`;

    const schemaB105 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaB" alias="b" version="01.00.05" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="SchemaC" version="01.00.05" alias="c"/>
        <ECEntityClass typeName="ElementB">
            <BaseClass>c:ElementC</BaseClass>
            <ECProperty propertyName="PropertyB" typeName="string" />
        </ECEntityClass>
    </ECSchema>`;

    const schemaC105 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaC" alias="c" version="01.00.05" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
        <ECEntityClass typeName="ElementC">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="PropertyC" typeName="string" />
        </ECEntityClass>
    </ECSchema>`;

  const createBasicSchemaSet = (): void => {
    writeToSchemaDirectory(schemaA105, "SchemaA.ecschema.xml");
    writeToSchemaDirectory(schemaB105, "SchemaB.ecschema.xml");
    writeToSchemaDirectory(schemaC105, "SchemaC.ecschema.xml");
  };

  const schemaA106 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaA" alias="a" version="01.00.06" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="SchemaB" version="01.00.05" alias="b"/>
        <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
        <ECEntityClass typeName="ElementA">
            <BaseClass>b:ElementB</BaseClass>
            <ECProperty propertyName="PropertyA" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="ElementA2">
          <BaseClass>bis:GeometricElement2d</BaseClass>
        </ECEntityClass>
    </ECSchema>`;

  it("should import updated schemaA", () => {
    writeToSchemaDirectory(schemaA106, "SchemaA.ecschema.xml");

    let schemaProps = dgndb.getSchemaProps("SchemaB");
    assert.isTrue(schemaProps.version === "01.00.05");
    schemaProps = dgndb.getSchemaProps("SchemaA");
    assert.isTrue(schemaProps.version === "01.00.05");

    dgndb.importSchemas([path.join(schemaDir, "SchemaA.ecschema.xml")], { schemaLockHeld: true });
    // Nothing to assert, throws if it fails
    schemaProps = dgndb.getSchemaProps("SchemaA");
    assert.isTrue(schemaProps.version === "01.00.06");
  });

  const schemaA104 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaA" alias="a" version="01.00.04" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="SchemaB" version="01.00.05" alias="b"/>
        <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
    </ECSchema>`;

  it("schema downgrade", () => {
    writeToSchemaDirectory(schemaA104, "SchemaA.ecschema.xml");

    let schemaProps = dgndb.getSchemaProps("SchemaB");
    assert.isTrue(schemaProps.version === "01.00.05");
    schemaProps = dgndb.getSchemaProps("SchemaA");
    assert.isTrue(schemaProps.version === "01.00.05");

    // Attempting to downgrade a schema does not seem to be an error, we just do nothing (see SchemaWriter::Context::PreprocessSchemas)
    dgndb.importSchemas([path.join(schemaDir, "SchemaA.ecschema.xml")], { schemaLockHeld: true });
    // Nothing to assert, throws if it fails
    schemaProps = dgndb.getSchemaProps("SchemaA");
    assert.isTrue(schemaProps.version === "01.00.05");
  });

  const schemaC104 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaC" alias="c" version="01.00.04" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
    </ECSchema>`;

  it("should reject indirect schema downgrade", () => {
    // The referenced schemaC is downgraded, and takes priority when loading over the one inside ECDb. This should fail
    // We do not have to list SchemaC explicitly, its presence in the folder is enough
    writeToSchemaDirectory(schemaA105, "SchemaA.ecschema.xml");
    writeToSchemaDirectory(schemaC104, "SchemaC.ecschema.xml");

    // Attempting to downgrade a schema does not seem to be an error, we just do nothing (see SchemaWriter::Context::PreprocessSchemas)
    assert.throws(() => dgndb.importSchemas([path.join(schemaDir, "SchemaA.ecschema.xml")], { schemaLockHeld: true }), "Failed to import schemas");
    // Nothing to assert, throws if it fails
    const schemaProps = dgndb.getSchemaProps("SchemaC");
    assert.isTrue(schemaProps.version === "01.00.05");
  });

  const schemaB106 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaB" alias="b" version="01.00.06" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="SchemaC" version="01.00.06" alias="c"/>
        <ECEntityClass typeName="ElementB">
            <BaseClass>c:ElementC</BaseClass>
            <ECProperty propertyName="PropertyB" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="ElementB2">
            <BaseClass>c:ElementC</BaseClass>
        </ECEntityClass>
    </ECSchema>`;

  const schemaC106 = `<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="SchemaC" alias="c" version="01.00.06" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="BisCore" version="01.00.10" alias="bis" />
        <ECEntityClass typeName="ElementC">
          <BaseClass>bis:GeometricElement2d</BaseClass>
          <ECProperty propertyName="PropertyC" typeName="string" />
        </ECEntityClass>
        <UnitSystem typeName="KSJFASPFJEPW" />
    </ECSchema>`;

  it("Should handle OpenSite+ older imodel problem (08-2025)", () => {
    // There was a problem where we loaded an unmodified schema (SchemaA) first and didn't add the search path to the schema context.
    // What happens is:
    // - SchemaA is unmodified -> gets loaded from ECDB, including SchemaB and SchemaC, all from ECDb.
    // - SchemaB gets explicitly loaded (provided as parameter) and is updated, now requires SchemaC in a newer version, which also gets loaded
    // This setup eventually lead to an error of the SchemaWriter trying to pull the new UnitSystem on SchemaC from the wrong reference
    // This is a simplified version of the problem. Previously it logged many "unclean schema graph" warnings, and after the fix it does not.
    // However, this test fails to produce the original error where we fail the import. The original scenario involved 31 schemas which I don't want to include all here.
    writeToSchemaDirectory(schemaA105, "SchemaA.ecschema.xml");
    writeToSchemaDirectory(schemaB106, "SchemaB.ecschema.xml");
    writeToSchemaDirectory(schemaC106, "SchemaC.ecschema.xml");

    // Attempting to downgrade a schema does not seem to be an error, we just do nothing (see SchemaWriter::Context::PreprocessSchemas)
    dgndb.importSchemas([path.join(schemaDir, "SchemaA.ecschema.xml"), path.join(schemaDir, "SchemaB.ecschema.xml")], { schemaLockHeld: true });
    // Nothing to assert, throws if it fails
    let schemaProps = dgndb.getSchemaProps("SchemaA");
    assert.isTrue(schemaProps.version === "01.00.05");

    schemaProps = dgndb.getSchemaProps("SchemaB");
    assert.isTrue(schemaProps.version === "01.00.06");

    schemaProps = dgndb.getSchemaProps("SchemaC");
    assert.isTrue(schemaProps.version === "01.00.06");
  });

});