/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { OpenMode, DbResult, IDisposable, Logger } from "@bentley/bentleyjs-core";
import { Statement, StatementCache } from "./Statement";
import { IModelBankLicensingNative } from "./IModelBankLicensingNative";
import { IModelBankLicensingNativeHost } from "./IModelBankLicensingNativeHost";

const loggingCategory = "iModelBank.Db";

/** The type of default transaction to run. */
export enum DefaultTxn { None = 0, Deferred = 1, Immediate = 2, Exclusive = 3 }

/** A SQLite Db */
export class Db implements IDisposable {
  private _nativeDb: IModelBankLicensingNative.NativeSQLiteDb;
  private _statementCache: StatementCache;

  public constructor() {
    this._nativeDb = new (IModelBankLicensingNativeHost.addon).NativeSQLiteDb();
    this._statementCache = new StatementCache();
  }

  /** Call this function when finished with this Db object. This releases the native resources held by the
   *  Db object.
   */
  public dispose(): void {
    if (this.isOpen)
      this.close();
  }

  public get nativeDb(): IModelBankLicensingNative.NativeSQLiteDb { return this._nativeDb; }

  public create(path: string, defaultTxn: DefaultTxn = DefaultTxn.Deferred): void {
    const res: DbResult = this._nativeDb.createDb(path, defaultTxn as number);
    if (DbResult.BE_SQLITE_OK !== res)
      throw IModelBankLicensingNativeHost.makeDbError(res, "Creating database failed.");
  }

  public open(path: string, mode: OpenMode, defaultTxn: DefaultTxn = DefaultTxn.Deferred): void {
    const res: DbResult = this._nativeDb.openDb(path, mode, defaultTxn as number);
    if (DbResult.BE_SQLITE_OK !== res)
      throw IModelBankLicensingNativeHost.makeDbError(res, "Opening database failed.");
  }

  public get isOpen(): boolean { return this._nativeDb.isOpen(); }
  public close(): void {
    this._statementCache.clear();
    this._nativeDb.closeDb();
  }

  public saveChanges(): void {
    const res = this._nativeDb.saveChanges();
    if (DbResult.BE_SQLITE_OK !== res)
      throw IModelBankLicensingNativeHost.makeDbError(res, "Saving changes failed.");
  }

  public abandonChanges(): void {
    const res = this._nativeDb.abandonChanges();
    if (DbResult.BE_SQLITE_OK !== res)
      throw IModelBankLicensingNativeHost.makeDbError(res, "Abandoning changes failed.");
  }

  public getDbGuid(): string {
    return this._nativeDb.getDbGuid();
  }

  public createTable(tableName: string, ddl: string): void {
    const res = this._nativeDb.createTable(tableName, ddl);
    if (DbResult.BE_SQLITE_OK !== res)
      throw IModelBankLicensingNativeHost.makeDbError(res, "Create table failed.");
  }

  private getPreparedStatement(sql: string): Statement {
    const cachedStatement = this._statementCache.find(sql);
    if (cachedStatement !== undefined && cachedStatement.useCount === 0) {  // we can only recycle a previously cached statement if nobody is currently using it.
      cachedStatement.useCount++;
      return cachedStatement.statement;
    }

    this._statementCache.removeUnusedStatementsIfNecessary();
    const stmt = new Statement();
    // Logger.logTrace(loggingCategory, `Preparing statement: ${sql}`);
    stmt.prepare(this, sql);
    this._statementCache.add(sql, stmt);
    return stmt;
  }
  private releasePreparedStatement(stmt: Statement): void { this._statementCache.release(stmt); }

  public withPreparedStatement<T>(sql: string, callback: (stmt: Statement) => T): T {
    const stmt = this.getPreparedStatement(sql);
    try {
      const val = callback(stmt);
      this.releasePreparedStatement(stmt);
      return val;
    } catch (err) {
      this.releasePreparedStatement(stmt); // always release statement
      Logger.logError(loggingCategory, err.toString());
      throw err;
    }
  }
}
