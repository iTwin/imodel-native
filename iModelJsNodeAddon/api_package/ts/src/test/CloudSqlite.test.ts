/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { use as chaiuse, expect } from "chai";
import * as chaiAsPromised from "chai-as-promised";
import * as fs from "fs-extra";
import { join, normalize } from "path";
import { NativeCloudSqlite } from "../NativeCloudSqlite";
import { IModelJsNative } from "../NativeLibrary";
import { getAssetsDir, getOutputDir, iModelJsNative } from "./utils";
import { ChildProcess, spawn  } from "child_process";
import { BeDuration, Guid, OpenMode } from "@itwin/core-bentley";

chaiuse(chaiAsPromised);

const azuriteUser = "devstoreaccount1";
const azuriteKey = "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="; // default Azurite password

async function sleep(ms: number) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}
let azuriteExitedPromise: Promise<void>;
let azurite: ChildProcess;

describe("cloud sqlite", () => {
  let cache: IModelJsNative.CloudCache;

  const startAzurite = async () => {
    azurite = spawn(`node`, [`${normalize(join(__dirname, "..", "..", "node_modules", "azurite", "dist", "src", "azurite.js"))}`, "-l", join(__dirname)]);
    azuriteExitedPromise = new Promise((resolve) => {
      azurite.on("exit", () => {
        resolve();
      });
    });
    await sleep(2000); // give some time for azurite to start up.
  };

  const shutdownAzurite = async (deleteAzuriteDir = true) => {
    azurite.kill("SIGTERM");
    await azuriteExitedPromise;
    if (deleteAzuriteDir) {
      const rootDir = __dirname;
      const foldersOrFilesToSearch = ["__blobstorage__", "__queuestorage__", "__azurite_db_blob__.json", "__azurite_db_blob_extent__.json", "__azurite_db_queue__.json", "__azurite_db_queue_extent__.json", "__azurite_db_table__.json"];
      for (const f of foldersOrFilesToSearch) {
        const p = normalize(join(rootDir, f));
        if (fs.existsSync(p))
          fs.removeSync(p);
      }
    }
  };

  before(async () => {
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

    await startAzurite();
  });

  after(async () => {
    cache.destroy();
    await shutdownAzurite();
  });

  it("should query bcvHttpLog", async () => {
    const containerId = Guid.createValue();
    const containerProps: NativeCloudSqlite.ContainerAccessProps = {
      accessName: azuriteUser,
      storageType: "azure?emulator=127.0.0.1:10000&sas=0",
      containerId,
      accessToken: azuriteKey,
      clientIdentifier: "ContainerIdentifier",
      writeable: true,
    };
    const container = new iModelJsNative.CloudContainer(containerProps);
    container.initializeContainer();
    container.connect(cache);

    let rows = container.queryHttpLog();
    expect(rows.length).to.equal(2); // manifest and bcv_kv GETs.

    // endTime to exclude these first 2 entries in later queries. 
    await BeDuration.wait(100);
    const endTime = new Date().toISOString();

    rows = container.queryHttpLog({startFromId: 2});
    expect(rows.length).to.equal(1);
    expect(rows[0].id).to.equal(2);
    

    container.acquireWriteLock("test");
    const dbTransfer = new iModelJsNative.CloudDbTransfer("upload", container, {localFileName: join(getAssetsDir(), "test.bim"), dbName: "test.bim" });
    await dbTransfer.promise;
    container.releaseWriteLock();
    container.checkForChanges();

    // 6 entries added by grabbing the write lock and checking for changes.
    // 2 entries from before. Expect 6 total entries because we're filtering by endTime from before.
    rows = container.queryHttpLog({finishedAtOrAfterTime: endTime, startFromId: 1});
    expect(rows.length).to.equal(6);
    expect(rows.find((value) => {
      return value.id === 1 || value.id === 2;
    })).to.equal(undefined);

    rows = container.queryHttpLog({finishedAtOrAfterTime: endTime});
    expect(rows.length).to.equal(6);
    expect(rows.find((value) => {
      return value.id === 1 || value.id === 2;
    })).to.equal(undefined);

    rows = container.queryHttpLog({finishedAtOrAfterTime: endTime, startFromId: 1, showOnlyFinished: true});
    expect(rows.length).to.equal(6);
    expect(rows.find((value) => {
      return value.id === 1 || value.id === 2;
    })).to.equal(undefined);

    rows = container.queryHttpLog({showOnlyFinished: true});
    expect(rows.length).to.equal(8);

    rows = container.queryHttpLog({startFromId: 4, showOnlyFinished: true});
    expect(rows.length).to.equal(5);

  });

  it("should pass clientIdentifier through container to database", async () => {
    const containerId = Guid.createValue();
    const containerProps: NativeCloudSqlite.ContainerAccessProps = {
      accessName: azuriteUser,
      storageType: "azure?emulator=127.0.0.1:10000&sas=0",
      containerId,
      accessToken: azuriteKey,
      clientIdentifier: "ContainerIdentifier",
      writeable: true,
    };
    const container = new iModelJsNative.CloudContainer(containerProps);
    container.initializeContainer();
    container.connect(cache);

    container.acquireWriteLock("test");
    const dbTransfer = new iModelJsNative.CloudDbTransfer("upload", container, {localFileName: join(getAssetsDir(), "test.bim"), dbName: "test.bim" });
    await dbTransfer.promise;
    container.releaseWriteLock();
    container.checkForChanges();

    const db: IModelJsNative.DgnDb = new iModelJsNative.DgnDb();
    db.openIModel("test.bim", OpenMode.Readonly, undefined, undefined, container);
    const stmt = new iModelJsNative.SqliteStatement();
    stmt.prepare(db, "PRAGMA bcv_client");
    stmt.step();
    expect(stmt.getValueString(0)).equal(containerProps.clientIdentifier);
    stmt.dispose();
    db.closeIModel();
    container.disconnect();

    const containerProps2: NativeCloudSqlite.ContainerAccessProps = {
      accessName: azuriteUser,
      storageType: "azure?emulator=127.0.0.1:10000&sas=0",
      containerId,
      clientIdentifier: "",
      accessToken: azuriteKey,
      writeable: true,
    };
    const container2 = new iModelJsNative.CloudContainer(containerProps2);
    container2.connect(cache);

    container2.checkForChanges();

    db.openIModel("test.bim", OpenMode.Readonly, undefined, undefined, container2);
    stmt.prepare(db, "PRAGMA bcv_client");
    stmt.step();
    expect(stmt.getValueString(0)).equal(containerProps2.clientIdentifier);
    stmt.dispose();
    db.closeIModel();
    container2.disconnect();

    const containerProps3: NativeCloudSqlite.ContainerAccessProps = { // No client identifier, so undefined. Should be "" by default.
      accessName: azuriteUser,
      storageType: "azure?emulator=127.0.0.1:10000&sas=0",
      containerId,
      accessToken: azuriteKey,
      writeable: true,
    };
    const container3 = new iModelJsNative.CloudContainer(containerProps3);
    container3.connect(cache);

    container3.checkForChanges();

    db.openIModel("test.bim", OpenMode.Readonly, undefined, undefined, container3);
    stmt.prepare(db, "PRAGMA bcv_client");
    stmt.step();
    expect(stmt.getValueString(0)).equal("");
    stmt.dispose();
    db.closeIModel();
    container3.disconnect();
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
    // eslint-disable-next-line @typescript-eslint/promise-function-async
    expect(() => container.cleanDeletedBlocks()).to.throw(notAttached);
    expect(() => container.acquireWriteLock("test 1")).to.throw(notAttached);
    expect(() => container.releaseWriteLock()).to.throw(notAttached);
    // eslint-disable-next-line @typescript-eslint/promise-function-async
    expect(() => container.uploadChanges()).to.throw(notAttached);
    expect(() => container.checkForChanges()).to.throw(notAttached);
  });
});
