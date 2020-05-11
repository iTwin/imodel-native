/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
type NativeSQLiteStatement = any; // import { NativeSQLiteStatement } from "@bentley/imodel-bank/native";
import { BentleyError, DbResult, IDisposable, Logger, StatusCodeWithMessage } from "@bentley/bentleyjs-core";
import { Db } from "./Db";
import { IModelBankLicensingNativeHost } from "./IModelBankLicensingNativeHost";

const LOGGING_CATEGORY = "iModelBank.Statement";

let statementVerbose = 0;
export function setStatementVerbose(v: number) {
  statementVerbose = v;
}

export class Statement implements IterableIterator<any>, IDisposable {
  private _stmt: NativeSQLiteStatement | undefined;
  private _isShared: boolean = false;

  /** @hidden - used by statement cache */
  public setIsShared(b: boolean) { this._isShared = b; }

  /** @hidden - used by statement cache */
  public get isShared(): boolean { return this._isShared; }

  /** Check if this statement has been prepared successfully or not */
  public get isPrepared(): boolean { return this._stmt !== undefined; }

  /** @hidden used internally only
   * Prepare this statement prior to first use.
   * @param db The DgnDb or ECDb to prepare the statement against
   * @param sql The ECSQL statement string to prepare
   * @throws [IModelError]($common) if the ECSQL statement cannot be prepared. Normally, prepare fails due to ECSQL syntax errors or references to tables or properties that do not exist.
   * The error.message property will provide details.
   */
  public prepare(db: Db, sql: string): void {
    if (statementVerbose > 1) {
      // tslint:disable-next-line:no-console
      Logger.logTrace(LOGGING_CATEGORY, sql);
    }
    if (this.isPrepared)
      throw new Error("SQLiteStatement is already prepared");
    this._stmt = new (IModelBankLicensingNativeHost.addon).NativeSQLiteStatement();
    const stat: StatusCodeWithMessage<DbResult> = this._stmt!.prepare(db.nativeDb, sql);
    if (stat.status !== DbResult.BE_SQLITE_OK)
      throw new BentleyError(stat.status, stat.message);
  }

  /** Reset this statement so that the next call to step will return the first row, if any.
   */
  public reset(): void {
    if (!this._stmt)
      throw new Error("SQLiteStatement is not prepared");

    this._stmt.reset();
  }

  public clearBindings() { this._stmt!.clearBindings(); }

  public bindValues(values: any | any[]): DbResult { return this._stmt!.bindValues(values); }

  public step(): DbResult { return this._stmt!.step(); }

  public getRow(): any { return this._stmt!.getRow(); }

  /** Call this function when finished with this statement. This releases the native resources held by the statement.
   * > Do not call this method directly on a statement that is being managed by a statement cache.
   */
  public dispose(): void {
    if (this.isShared)
      throw new Error("you can't dispose an SQLiteStatement that is shared with others (e.g., in a cache)");
    if (!this.isPrepared)
      return;
    this._stmt!.dispose(); // Tell the peer JS object to free its native resources immediately
    this._stmt = undefined; // discard the peer JS object as garbage
  }

  public next(): IteratorResult<any> {
    if (DbResult.BE_SQLITE_ROW === this.step()) {
      return {
        done: false,
        value: this.getRow(),
      };
    } else {
      return {
        done: true,
        value: undefined,
      };
    }
  }

  /** The iterator that will step through the results of this statement. */
  public [Symbol.iterator](): IterableIterator<any> { return this; }
}

export class CachedStatement {
  public statement: Statement;
  public useCount: number;

  /** @hidden - used by statement cache */
  public constructor(stmt: Statement) {
    this.statement = stmt;
    this.useCount = 1;
  }
}

export class StatementCache {
  private readonly _statements: Map<string, CachedStatement> = new Map<string, CachedStatement>();
  public readonly maxCount: number;

  public constructor(maxCount = 20) { this.maxCount = maxCount; }

  public add(str: string, stmt: Statement): void {
    const existing = this._statements.get(str);
    if (existing !== undefined) {
      throw new Error("you should only add a statement if all existing copies of it are in use.");
    }
    const cs = new CachedStatement(stmt);
    cs.statement.setIsShared(true);
    this._statements.set(str, cs);
  }

  public getCount(): number { return this._statements.size; }

  public find(str: string): CachedStatement | undefined {
    return this._statements.get(str);
  }

  public release(stmt: Statement): void {
    for (const cs of this._statements) {
      const css = cs[1];
      if (css.statement === stmt) {
        if (css.useCount > 0) {
          css.useCount--;
          if (css.useCount === 0) {
            css.statement.reset();
            css.statement.clearBindings();
          }
        } else {
          throw new Error("double-release of cached statement");
        }
        // leave the statement in the cache, even if its use count goes to zero. See removeUnusedStatements and clearOnClose.
        // *** TODO: we should remove it if it is a duplicate of another unused statement in the cache. The trouble is that we don't have the ecsql for the statement,
        //           so we can't check for other equivalent statements.
        break;
      }
    }
  }

  public removeUnusedStatementsIfNecessary(): void {
    if (this.getCount() <= this.maxCount)
      return;

    const keysToRemove = [];
    for (const cs of this._statements) {
      const css = cs[1];
      if (css.useCount === 0) {
        css.statement.setIsShared(false);
        css.statement.dispose();
        keysToRemove.push(cs[0]);
        if (keysToRemove.length >= this.maxCount)
          break;
      }
    }
    for (const k of keysToRemove) {
      this._statements.delete(k);
    }
  }

  public clear() {
    for (const cs of this._statements) {
      const stmt = cs[1].statement;
      if (stmt !== undefined) {
        stmt.setIsShared(false);
        stmt.dispose();
      }
    }
    this._statements.clear();
  }
}
