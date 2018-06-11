/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { BentleyStatus, DbResult, OpenMode, StatusCodeWithMessage } from "@bentley/bentleyjs-core";

/* The NativeSQLiteDb class that is projected by the addon. */
declare class NativeSQLiteDb {
  constructor();
  /**
  * Create a new SQLiteDb.
  * @param dbname The full path to the SQLiteDb in the local file system
  * @return non-zero error status if operation failed.
  */
  createDb(dbname: string): DbResult;

  /** Open a existing SQLiteDb.
  * @param dbname The full path to the SQLiteDb in the local file system
  * @param mode The open mode
  * @return non-zero error status if operation failed.
  */
  openDb(dbname: string, mode: OpenMode): DbResult;

  /** Check to see if connection to SQLiteDb is open or not.
  * @return true if connection is open
  */
  isOpen(): boolean;

  /** Check to see if connection to SQLiteDb is open or not.
  * @return true if connection was closed
  */
  closeDb(): void;

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
}

declare class NativeSQLiteStatement {
  constructor();
  prepare(db: NativeSQLiteDb, sql: string): StatusCodeWithMessage<DbResult>;
  reset(): void;
  dispose(): void;
  step(): DbResult;
  getRow(): any;
}