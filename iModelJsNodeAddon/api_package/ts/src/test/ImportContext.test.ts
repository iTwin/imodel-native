/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { Code, DbQueryRequest, DbQueryResponse, DbRequestKind, GeometricElement3dProps, ProfileOptions} from "@itwin/core-common";
import { expect } from "chai";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { openDgnDb } from ".";
import { IModelJsNative } from "../NativeLibrary";
import { dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";

// Crash reporting on linux is gated by the presence of this env variable.
if (os.platform() === "linux")
  process.env.LINUX_MINIDUMP_ENABLED = "yes";

describe("ImportContext", () => {
  const sourceDbPath = path.join(getOutputDir(), "testSource.bim");
  const targetDbPath = path.join(getOutputDir(), "testTarget.bim");
  beforeEach(async () => {
    if (fs.existsSync(sourceDbPath))
      fs.removeSync(sourceDbPath);
    if (fs.existsSync(targetDbPath))
      fs.removeSync(targetDbPath);

    fs.copyFileSync(dbFileName, sourceDbPath);
    fs.copyFileSync(dbFileName, targetDbPath);
  });

  it("should cloneElement properly on a spatialCategory within the same database", async () => {
    const sourceDb = openDgnDb(sourceDbPath, { profile: ProfileOptions.Upgrade, schemaLockHeld: true});
    const elementProps = sourceDb.getElement({id: "0x2d"});
    expect(elementProps.classFullName).to.be.equal("BisCore:SpatialCategory");
    const nativeContext = new iModelJsNative.ImportContext(sourceDb, sourceDb);
    const targetElementProps = nativeContext.cloneElement(elementProps.id!);
    // cloneElement doesn't set the id.
    expect(targetElementProps.id).to.be.equal("0");
    targetElementProps.id = "0x2d";

    // If ImportContext is not between dbs, code does not get copied over.
    // The default DgnCode() constructor is different from Code.createEmpty() in TS, so I can't use it here.
    expect(Code.equalCodes(Code.fromJSON({spec: "0", scope: "", value: ""}), targetElementProps.code)).to.be.true;
    targetElementProps.code = elementProps.code;

    // importContext creates a fedGuid by default, but our source element doesn't have one.
    delete targetElementProps.federationGuid;
    expect(targetElementProps).to.deep.equal(elementProps);

    sourceDb.closeFile();
  });

  it("should successfully change type of property during ImportContext.cloneElement even though the two schemas have the same classId within their own database", async () => {

    // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCustomAttributes.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
    const sourceDb = openDgnDb(sourceDbPath, { profile: ProfileOptions.Upgrade, schemaLockHeld: true});
    const pathToSchema = `${path.join(getAssetsDir(), "sourceDbSchema.ecschema.xml")}`;
    sourceDb.importSchemas([pathToSchema]);

    // insert element of type Bracket.Bracket introduced by sourceDbSchema.ecschema.xml
    const sourceId = sourceDb.insertElement(
      {classFullName: "Bracket.Bracket",
        code: Code.createEmpty(),
        model: "0x1c",
        allocation: [{
          name: "B70-B14-001",
          quantity: 1,
        }],
        category: "0x2d",
        parent: undefined,
      } as GeometricElement3dProps);

    const sourceElementProps = sourceDb.getElement({id: sourceId});
    expect((sourceElementProps as any).allocation[0].quantity).to.equal(1);

    // Without ProfileOptions.Upgrade, we get: Error | ECDb | Failed to import schema 'BisCustomAttributes.01.00.00'. Current ECDb profile version (4.0.0.1) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.
    const targetDb = openDgnDb(targetDbPath, {profile: ProfileOptions.Upgrade, schemaLockHeld: true});
    targetDb.importSchemas([`${path.join(getAssetsDir(), "targetDbSchema.ecschema.xml")}`]);

    // Assert the same ECClassId across the two databases for the Bracket class, and they should because source and target are clones of eachother, which then both had a schema imported.
    const request: DbQueryRequest = {
      kind: DbRequestKind.ECSql,
      query: "SELECT ECClassId FROM ECDbMeta.ECClassDef WHERE Name LIKE 'Bracket'",
      delay: 5000,  // Set delay to a value > timeout
    };
    const sourceDbQuery = await new Promise<DbQueryResponse>((resolve) => {
      sourceDb.concurrentQueryExecute(request as any, (response: any) => {
        resolve(response as DbQueryResponse);
      });
    });
    const schemaIdSource = [...sourceDbQuery.data];
    expect(schemaIdSource.length).to.be.equal(1);
    expect(schemaIdSource[0].length).to.be.equal(1);

    const targetDbQuery = await new Promise<DbQueryResponse>((resolve) => {
      targetDb.concurrentQueryExecute(request as any, (response: any) => {
        resolve(response as DbQueryResponse);
      });
    });
    const schemaIdTarget = [...targetDbQuery.data];
    expect(schemaIdTarget.length).to.be.equal(1);
    expect(schemaIdTarget[0].length).to.be.equal(1);
    expect(schemaIdSource[0][0]).to.be.equal(schemaIdTarget[0][0]);

    const nativeContext = new IModelJsNative.ImportContext(sourceDb, targetDb);
    // Helps cloneElement code find the intended target model, which since our target starts off as a copy of source is the same id.
    nativeContext.addElementId("0x1c", "0x1c");

    // Clone the element with targetDb as the target, it has a nearly identical schema with the exception of the type of the Quantity property which is string instead of int.
    const targetElementProps = nativeContext.cloneElement(sourceId);
    /**
     * If sameClass is true in DgnElement::_CopyFrom then this would've stayed as an int.
     * Previously sameClass would have been true because we were only checking that GetElementClassId of the source and target were the same and not also asserting that the dbs are the same.
     */
    expect((targetElementProps as any).allocation[0].quantity).to.equal("1");
    sourceDb.closeFile();
    targetDb.closeFile();
  });
});
