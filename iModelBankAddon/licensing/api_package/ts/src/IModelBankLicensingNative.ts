/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 *--------------------------------------------------------------------------------------------*/
import { Logger, DbResult, OpenMode, StatusCodeWithMessage } from "@bentley/bentleyjs-core";

export declare namespace IModelBankLicensingNative {

  const version: string;
  let logger: Logger;

  /**
   * Verify that the specified license file is valid and not expired.
   * @param licenseFileName Full path to a checked-out license file.
   */
  function checkEntitlement(iModelId: string, activityId: string, time: string): string;
  function setup(rootDir: string, licensePath: string, deploymentId: string): number;

  interface NativeChangeSetCounts {
    inserts: number;
    deletes: number;
    updates: number;
  }

  interface NativeApplyResult {
    status: DbResult;
    containsSchemaChanges: boolean;
    element: NativeChangeSetCounts;
    model: NativeChangeSetCounts;
    aspect: NativeChangeSetCounts;
  }

  enum NativeSQLiteDefaultTxn { None = 0, Deferred = 1, Immediate = 2, Exclusive = 3 }

  /* The native SQLite Db class that is projected by the addon. */
  class NativeSQLiteDb {
    constructor();
    /**
     * Create a new SQLiteDb.
     * @param dbname The full path to the SQLiteDb in the local file system
     * @param defaultTxn SQLite txn mode to use
     * @return non-zero error status if operation failed.
     */
    public createDb(dbname: string, defaultTxn: NativeSQLiteDefaultTxn): DbResult;

    /** Open a existing SQLiteDb.
     * @param dbname The full path to the SQLiteDb in the local file system
     * @param mode The open mode
     * @param defaultTxn SQLite txn mode to use
     * @return non-zero error status if operation failed.
     */
    public openDb(dbname: string, mode: OpenMode, defaultTxn: NativeSQLiteDefaultTxn): DbResult;

    /** Check to see if connection to SQLiteDb is open or not.
     * @return true if connection is open
     */
    // tslint:disable-next-line:prefer-get
    public isOpen(): boolean;

    /** Check to see if connection to SQLiteDb is open or not.
     * @return true if connection was closed
     */
    public closeDb(): void;

    /** Query the DbGuid property of this Db. */
    public getDbGuid(): string;

    public createTable(tableName: string, ddl: string): DbResult;

    /** Save changes to SQLiteDb
     * @param changesetName The name of the operation that generated these changes. If transaction tracking is enabled.
     * @return non-zero error status if operation failed.
     */
    public saveChanges(changesetName?: string): DbResult;

    /** Abandon changes
     * @return non-zero error status if operation failed.
     */
    public abandonChanges(): DbResult;

    /** Apply a changeset, which is contained in a series of "block" files. */
    public applyChangeSet(blockFileNames: string[]): NativeApplyResult;

  }

  /* The native SQLite Statment class that is projected by the addon. */
  class NativeSQLiteStatement {
    constructor();
    public prepare(db: NativeSQLiteDb, sql: string): StatusCodeWithMessage<DbResult>;
    public reset(): void;
    public clearBindings(): void;
    public dispose(): void;
    public step(): DbResult;
    public bindValues(values: any | any[]): DbResult;
    public getRow(): any;
  }

}
