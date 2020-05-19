/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { assert } from "chai";
import * as path from "path";
import { ecSchemaOpsNative, getTestAssetsDir} from "./utils";

const ECSchemaOps = ecSchemaOpsNative.ECSchemaOps;

describe("ECSchemaOps", () => {
  let assetsDir: string;
  let schemaUtil: any;

  beforeEach(async ()  => {
    assetsDir = getTestAssetsDir();
    schemaUtil = new ECSchemaOps();
  });

  afterEach(async () => {
  });

  it("computeChecksum", async () => {
    const schemaPath = path.join(assetsDir, "SchemaA.ecschema.xml");
    const refPath = path.dirname(schemaPath);
    const sha1 = schemaUtil.computeChecksum(schemaPath, [refPath]);
    assert.isDefined(sha1);
    assert.equal(sha1, "2a618664fbba1df7c05f27d7c0e8f58de250003b", "Expected sha1 hash values to match");
  });
});
