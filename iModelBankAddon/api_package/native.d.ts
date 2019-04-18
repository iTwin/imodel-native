/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 *--------------------------------------------------------------------------------------------*/
import { BentleyStatus, DbResult, OpenMode, StatusCodeWithMessage } from "@bentley/bentleyjs-core";

declare interface NativeChangeSetCounts {
  inserts: number;
  deletes: number;
  updates: number;
}

declare interface NativeApplyResult {
  status: DbResult;
  containsSchemaChanges: boolean;
  element: NativeChangeSetCounts;
  model: NativeChangeSetCounts;
  aspect: NativeChangeSetCounts;
}

export enum NativeSQLiteDefaultTxn { None = 0, Deferred = 1, Immediate = 2, Exclusive = 3 }

/* The NativeSQLiteDb class that is projected by the addon. */
declare class NativeSQLiteDb {
  constructor();
  /**
  * Create a new SQLiteDb.
  * @param dbname The full path to the SQLiteDb in the local file system
  * @param defaultTxn SQLite txn mode to use
  * @return non-zero error status if operation failed.
  */
  createDb(dbname: string, defaultTxn: NativeSQLiteDefaultTxn): DbResult;

  /** Open a existing SQLiteDb.
  * @param dbname The full path to the SQLiteDb in the local file system
  * @param mode The open mode
  * @param defaultTxn SQLite txn mode to use
  * @return non-zero error status if operation failed.
  */
  openDb(dbname: string, mode: OpenMode, defaultTxn: NativeSQLiteDefaultTxn): DbResult;

  /** Check to see if connection to SQLiteDb is open or not.
  * @return true if connection is open
  */
  isOpen(): boolean;

  /** Check to see if connection to SQLiteDb is open or not.
  * @return true if connection was closed
  */
  closeDb(): void;

  /** Query the DbGuid property of this Db. */
  getDbGuid(): string;

  createTable(tableName: string, ddl: string): DbResult;

  /** Save changes to SQLiteDb
  * @param changesetName The name of the operation that generated these changes. If transaction tracking is enabled.
  * @return non-zero error status if operation failed.
  */
  saveChanges(changesetName?: string): DbResult;

  /** Abandon changes
  * @return non-zero error status if operation failed.
  */
  abandonChanges(): DbResult;

  /** Apply a changeset, which is contained in a series of "block" files. */
  applyChangeSet(blockFileNames: string[]): NativeApplyResult;

}

declare class NativeSQLiteStatement {
  constructor();
  prepare(db: NativeSQLiteDb, sql: string): StatusCodeWithMessage<DbResult>;
  reset(): void;
  clearBindings(): void;
  dispose(): void;
  step(): DbResult;
  bindValues(values: any | any[]): DbResult;
  getRow(): any;
}