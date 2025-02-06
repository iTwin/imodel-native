/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import * as fs from "fs";
import { join } from "path";
import { NativeCloudSqlite } from "../NativeCloudSqlite";
import { IModelJsNative } from "../NativeLibrary";
import { getOutputDir, iModelJsNative } from "./utils";

describe("cloud sqlite", () => {
  let cache: IModelJsNative.CloudCache;

  before(() => {
    const rootDir = join(getOutputDir(), "cloud");
    if (!fs.existsSync(rootDir))
      fs.mkdirSync(rootDir);

    const cloudProps: NativeCloudSqlite.CacheProps = {
      rootDir,
      name: "testVFS",
      cacheSize: "10G",
    };
    cache = new iModelJsNative.CloudCache(cloudProps);
    expect(cache.name).equal(cloudProps.name);
    expect(cache.rootDir).equal(cloudProps.rootDir);
  });

  after(() => {
    cache.destroy();
  });

  it("container", async () => {
    const containerProps: NativeCloudSqlite.ContainerAccessProps = {
      baseUri: "http://127.0.0.1:10000/devstoreaccount1",
      storageType: "azure",
      containerId: "abc-123",
      accessToken: "bad",
      lockExpireSeconds: 1,
    };
    const container = new iModelJsNative.CloudContainer(containerProps);
    expect(container.isConnected).is.false;
    expect(container.hasWriteLock).is.false;
    expect(container.isPublic).is.false;
    expect(container.accessToken).equal(containerProps.accessToken);
    const newToken = "bad-token";
    container.accessToken = newToken; // test setter
    expect(container.accessToken).equal(newToken);

    const notAttached = "container not connected to cache";
    expect(() => container.acquireWriteLock("test 1")).to.throw(notAttached);
    expect(() => container.releaseWriteLock()).to.throw(notAttached);
    // eslint-disable-next-line @typescript-eslint/promise-function-async
    expect(() => container.uploadChanges()).to.throw(notAttached);
    expect(() => container.checkForChanges()).to.throw(notAttached);

    containerProps.isPublic = true;
    const c2 = new iModelJsNative.CloudContainer(containerProps);
    expect(c2.isPublic).is.true;
  });
});
