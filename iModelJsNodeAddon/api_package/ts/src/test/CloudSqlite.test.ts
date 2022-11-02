/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect, use as chaiuse } from "chai";
import * as chaiAsPromised from "chai-as-promised";
import * as fs from "fs";
import { join } from "path";
import { NativeCloudSqlite } from "../NativeCloudSqlite";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

chaiuse(chaiAsPromised);

describe("cloud sqlite", () => {
  let cache: IModelJsNative.CloudCache;

  before(() => {
    const rootDir =  join(getOutputDir(), "cloud");
    if (!fs.existsSync(rootDir))
      fs.mkdirSync(rootDir);

    const cloudProps: NativeCloudSqlite.CacheProps = {
      rootDir,
      name: "testVFS",
      cacheSize: "10G",
    }
    cache = new iModelJsNative.CloudCache(cloudProps);
    expect(cache.name).equal(cloudProps.name);
    expect(cache.rootDir).equal(cloudProps.rootDir);
  });

  after(() => {
    cache.destroy();
  });

  it("container", async () => {
    const containerProps: NativeCloudSqlite.ContainerAccessProps = {
      accessName: "account1",
      storageType: "azure",
      containerId: "abc-123",
      accessToken: "bad",
    };
    const container = new iModelJsNative.CloudContainer(containerProps);
    expect(container.isConnected).is.false;
    expect(container.hasWriteLock).is.false;
    expect(container.accessToken).equal(containerProps.accessToken);
    const newToken = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==";
    container.accessToken = newToken; // test setter
    expect(container.accessToken).equal(newToken);
    expect(() => container.connect(cache)).to.throw("attach error").property("errorNumber").equal(403);

    const notAttached = "container not connected to cache";
    expect(() => container.cleanDeletedBlocks()).to.throw(notAttached);
    expect(() => container.acquireWriteLock("test 1")).to.throw(notAttached);
    expect(() => container.releaseWriteLock()).to.throw(notAttached);
    expect(() => container.uploadChanges()).to.throw(notAttached);
    expect(() => container.checkForChanges()).to.throw(notAttached);
  });
});
