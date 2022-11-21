import { expect } from 'chai';
import * as os from 'os';

import {
    DbBlobRequest, DbBlobResponse, DbQueryRequest, DbQueryResponse, DbRequestKind, DbResponseStatus,
    DbResult, ECSqlReader, IModelError, QueryBinder, QueryLimit, QueryOptions, QueryOptionsBuilder,
    QueryQuota, QueryRowFormat
} from '@itwin/core-common';

import { IModelJsNative } from '../NativeLibrary';
import { openDgnDb } from './';
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
import { dbFileName } from './utils';

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
  public static resetConfig(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, config?: IModelJsNative.QueryConfig) : IModelJsNative.QueryConfig {
    return conn.concurrentQueryResetConfig(config);
  }

}

describe("concurrent query tests", () => {
  let conn: IModelJsNative.DgnDb;
  beforeEach((done) => {
    conn = openDgnDb(dbFileName);
    done();
  })

  afterEach((done) => {
    conn.closeIModel();
    done();
  })
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
        memory: 10000
      },
      ignorePriority: true,
      ignoreDelay: false,
      requestQueueSize: 1000,
      workerThreads: 6
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
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 5000,
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Timeout);
  });

  it("query max memory", async () => {
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { memory: 100 } });

    // run a query with delay of 5 sec.
    const rc = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 1000 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Partial);
  });

  it("restart query", async () => {
    // set max time for query to 1 sec
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { time: 20 }, ignoreDelay: false });

    const q0 = ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 5000,
      restartToken:"token1"
    } as DbQueryRequest);

    const q1 = ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 0,
      restartToken:"token1"
    } as DbQueryRequest);

    const r0 = await q0;
    const r1 = await q1;
    expect(r0.status).eq(DbResponseStatus.Cancel);
    expect(r1.status).eq(DbResponseStatus.Done);
  });
});

