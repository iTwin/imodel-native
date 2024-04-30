import { expect } from "chai";
import * as os from "os";
import {
  DbBlobRequest, DbBlobResponse, DbQueryRequest, DbQueryResponse, DbRequestKind, DbResponseStatus, DbResult,
  ECSqlReader, IModelError, QueryBinder, QueryOptions,
} from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";
import { openDgnDb } from "./";
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { dbFileName } from "./utils";

// Crash reporting on linux is gated by the presence of this env variable.
if (os.platform() === "linux")
  process.env.LINUX_MINIDUMP_ENABLED = "yes";

class ConcurrentQueryHelper {
  public static async executeQueryRequest(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, request: DbQueryRequest): Promise<DbQueryResponse> {
    return new Promise<DbQueryResponse>((resolve) => {
      request.kind = DbRequestKind.ECSql;
      conn.concurrentQueryExecute(request as any, (response: any) => {
        resolve(response as DbQueryResponse);
      });
    });
  }
  public static async executeBlobRequest(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, request: DbBlobRequest): Promise<DbBlobResponse> {
    return new Promise<DbBlobResponse>((resolve) => {
      request.kind = DbRequestKind.BlobIO;
      conn.concurrentQueryExecute(request as any, (response: any) => {
        resolve(response as DbBlobResponse);
      });
    });
  }
  public static createQueryReader(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, ecsql: string, params?: QueryBinder, config?: QueryOptions & { delay?: number }): ECSqlReader {
    if (!conn.isOpen()) {
      throw new IModelError(DbResult.BE_SQLITE_ERROR, "db not open");
    }
    const executor = {
      execute: async (request: DbQueryRequest) => {
        return ConcurrentQueryHelper.executeQueryRequest(conn, request);
      },
    };
    return new ECSqlReader(executor, ecsql, params, config as QueryOptions);
  }
  public static async * query(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, ecsql: string, params?: QueryBinder, options?: QueryOptions & { delay?: number }): AsyncIterableIterator<any> {
    const reader = this.createQueryReader(conn, ecsql, params, options);
    while (await reader.step()) {
      yield reader.formatCurrentRow();
    }
  }
  public static resetConfig(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, config?: IModelJsNative.QueryConfig): IModelJsNative.QueryConfig {
    return conn.concurrentQueryResetConfig(config);
  }
}

describe("concurrent query tests", () => {
  let conn: IModelJsNative.DgnDb;
  beforeEach((done) => {
    conn = openDgnDb(dbFileName);
    done();
  });

  afterEach((done) => {
    conn.closeFile();
    done();
  });
  it("reset config", async () => {
    const defaultConf = ConcurrentQueryHelper.resetConfig(conn);
    expect(defaultConf.ignorePriority).eq(false);
    expect(defaultConf.ignoreDelay).eq(true);
    expect(defaultConf.requestQueueSize).eq(2000);
    expect(defaultConf.workerThreads).not.eq(0);
    expect(defaultConf.globalQuota?.memory).eq(0x800000);
    expect(defaultConf.globalQuota?.time).eq(60);
    const modifiedConf = ConcurrentQueryHelper.resetConfig(conn, {
      globalQuota: {
        time: 1,
        memory: 10000,
      },
      ignorePriority: true,
      ignoreDelay: false,
      requestQueueSize: 1000,
      workerThreads: 6,
    });
    expect(modifiedConf.ignorePriority).eq(true);
    expect(modifiedConf.ignoreDelay).eq(false);
    expect(modifiedConf.requestQueueSize).eq(1000);
    expect(modifiedConf.workerThreads).eq(6);
    expect(modifiedConf.globalQuota?.memory).eq(10000);
    expect(modifiedConf.globalQuota?.time).eq(1);

    const resetConf = ConcurrentQueryHelper.resetConfig(conn);
    expect(resetConf.ignorePriority).eq(false);
    expect(resetConf.ignoreDelay).eq(true);
    expect(resetConf.requestQueueSize).eq(2000);
    expect(resetConf.workerThreads).not.eq(0);
    expect(resetConf.globalQuota?.memory).eq(0x800000);
    expect(resetConf.globalQuota?.time).eq(60);
  });

  it("query timeout", async () => {
    // set max time for query to 1 sec
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { time: 1 }, ignoreDelay: false });

    // run a query with delay of 5 sec.
    const rc = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 5000,
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Timeout);
  });

  it("query max memory", async () => {
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { memory: 100 } });

    // run a query with delay of 5 sec.
    const rc = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Partial);
  });

  it("Test quota memory and time passed together through params", async () => {
    // Reset Config to defaults
    const resetConf = ConcurrentQueryHelper.resetConfig(conn);
    expect(resetConf.ignorePriority).eq(false);
    expect(resetConf.ignoreDelay).eq(true);
    expect(resetConf.requestQueueSize).eq(2000);
    expect(resetConf.workerThreads).not.eq(0);
    expect(resetConf.globalQuota?.memory).eq(0x800000);
    expect(resetConf.globalQuota?.time).eq(60);
    // Case 1: Set memory quota to 100 and time to 1, and ensure these limits are updated in the query response
    const rc1 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        memory: 100,
        time: 1,
      },
    } as DbQueryRequest);

    expect(rc1.stats.memLimit).eq(100);
    expect(rc1.stats.timeLimit).eq(1000);

    // Case 2: For no quota passed ensure global maximum limits are getting used
    const rc2 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
    } as DbQueryRequest);

    expect(rc2.stats.memLimit).eq(0x800000);
    expect(rc2.stats.timeLimit).eq(60000);

    // Case 3: For memory quota limits exceeding global maximum, ensure global maximum limit is used only for memoory
    const rc3 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        memory: 85464865465,
        time: 20,
      },
    } as DbQueryRequest);

    expect(rc3.stats.memLimit).eq(0x800000);
    expect(rc3.stats.timeLimit).eq(20000);

    // Case 4: For time quota limit exceeding global maximum, ensure global maximum limits is used only for time
    const rc4 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        memory: 1024,
        time: 6198,
      },
    } as DbQueryRequest);

    expect(rc4.stats.memLimit).eq(1024);
    expect(rc4.stats.timeLimit).eq(60000);

    // Case 5: For both quota limits exceeding global maximum, ensure both global maximum limits are getting used
    const rc5 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        memory: 85464865465,
        time: 6198,
      },
    } as DbQueryRequest);

    expect(rc5.stats.memLimit).eq(0x800000);
    expect(rc5.stats.timeLimit).eq(60000);
  });

  it("Test quota memory and time passed individually through params", async () => {
    // Reset Config with custom quota
    const resetConf = ConcurrentQueryHelper.resetConfig(conn, {
      globalQuota: {
        time: 30,
        memory: 10000,
      },
    });

    expect(resetConf.globalQuota?.memory).eq(10000);
    expect(resetConf.globalQuota?.time).eq(30);
    // Case 1: Set memory quota to 100 , and ensure only memory uses local quota, and time uses global
    const rc1 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        memory: 100,
      },
    } as DbQueryRequest);

    expect(rc1.stats.memLimit).eq(100);
    expect(rc1.stats.timeLimit).eq(30000);

    // Case 2: Set time to 12, and ensure only time uses local quota, and memory uses global
    const rc2 = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query:
        "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      quota: {
        time: 12,
      },
    } as DbQueryRequest);

    expect(rc2.stats.memLimit).eq(10000);
    expect(rc2.stats.timeLimit).eq(12000);
  });

  it("restart query", async () => {
    // set max time for query to 1 sec
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { time: 20 }, ignoreDelay: false });

    const q0 = ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 5000,
      restartToken: "token1",
    } as DbQueryRequest);

    const q1 = ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind: DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 0,
      restartToken: "token1",
    } as DbQueryRequest);

    const r0 = await q0;
    const r1 = await q1;
    expect(r0.status).eq(DbResponseStatus.Cancel);
    expect(r1.status).eq(DbResponseStatus.Done);
  });
});

