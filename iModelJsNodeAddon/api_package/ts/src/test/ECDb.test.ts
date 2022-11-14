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
  public static resetConfig(conn: IModelJsNative.ECDb | IModelJsNative.DgnDb, config: IModelJsNative.QueryConfig) {
    conn.concurrentQueryResetConfig(config);
  }

}

describe("concurrent query tests", () => {
  let conn: IModelJsNative.DgnDb;
  before((done) => {
    conn = openDgnDb(dbFileName);
    done();
  })

  after((done) => {
    conn.closeIModel();
    done();
  })

  it("query timeout", async () => {
    // set max time for query to 1 sec
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { time: 1 } });

    // run a query with delay of 5 sec.
    const rc = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x from cnt",
      delay: 5000,
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Timeout);
  });

  it("query max memory", async () => {
    // set max time for query to 1 sec
    ConcurrentQueryHelper.resetConfig(conn, { globalQuota: { memory: 1024 } });

    // run a query with delay of 5 sec.
    const rc = await ConcurrentQueryHelper.executeQueryRequest(conn, {
      kind :DbRequestKind.ECSql,
      query: "with cnt(x) as (values(0) union select x+1 from cnt where x < 10 ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
      delay: 0,
    } as DbQueryRequest);

    expect(rc.status).eq(DbResponseStatus.Partial);
  });

});

