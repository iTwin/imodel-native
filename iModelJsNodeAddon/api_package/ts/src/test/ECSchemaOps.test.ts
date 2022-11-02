/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { expect } from "chai";
import { join } from "path";
import { iModelJsNative, getAssetsDir } from "./utils";

describe("ECSchemaOps", () => {

  it("computeSchemaChecksum", () => {
    const assetsDir = join(getAssetsDir(), 'ECSchemaOps');
    const schemaXmlPath = join(assetsDir, "SchemaA.ecschema.xml");
    let referencePaths = [assetsDir];
    let sha1 = iModelJsNative.computeSchemaChecksum({ schemaXmlPath, referencePaths });
    expect(sha1).equal("3ac6578060902aa0b8426b61d62045fdf7fa0b2b");

    referencePaths = [join(assetsDir, "exact-match")];
    sha1 = iModelJsNative.computeSchemaChecksum({ schemaXmlPath, referencePaths, exactMatch: true });
    expect(sha1).equal("2a618664fbba1df7c05f27d7c0e8f58de250003b");
  });
});
