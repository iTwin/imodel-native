/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <functional>
#include <optional>
#include <vector>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! The action codes passed to the SQLite authorizer. Values match the SQLITE_* action
//! codes of sqlite3_set_authorizer.
//! @see https://www.sqlite.org/c3ref/c_alter_table.html
// @bsiclass
//=======================================================================================
enum class AuthorizerAction : int
    {
    Copy = 0,               //!< Legacy, never issued by current SQLite versions
    CreateIndex = 1,        //!< arg1: index, arg2: table
    CreateTable = 2,        //!< arg1: table
    CreateTempIndex = 3,    //!< arg1: index, arg2: table
    CreateTempTable = 4,    //!< arg1: table
    CreateTempTrigger = 5,  //!< arg1: trigger, arg2: table
    CreateTempView = 6,     //!< arg1: view
    CreateTrigger = 7,      //!< arg1: trigger, arg2: table
    CreateView = 8,         //!< arg1: view
    Delete = 9,             //!< arg1: table
    DropIndex = 10,         //!< arg1: index, arg2: table
    DropTable = 11,         //!< arg1: table
    DropTempIndex = 12,     //!< arg1: index, arg2: table
    DropTempTable = 13,     //!< arg1: table
    DropTempTrigger = 14,   //!< arg1: trigger, arg2: table
    DropTempView = 15,      //!< arg1: view
    DropTrigger = 16,       //!< arg1: trigger, arg2: table
    DropView = 17,          //!< arg1: view
    Insert = 18,            //!< arg1: table
    Pragma = 19,            //!< arg1: pragma name, arg2: argument or nullptr
    Read = 20,              //!< arg1: table, arg2: column ("" for count(*))
    Select = 21,            //!< no arguments
    Transaction = 22,       //!< arg1: BEGIN, COMMIT or ROLLBACK
    Update = 23,            //!< arg1: table, arg2: column
    Attach = 24,            //!< arg1: filename or nullptr
    Detach = 25,            //!< arg1: schema alias
    AlterTable = 26,        //!< arg1: schema, arg2: table; database slot holds the column for DROP COLUMN
    Reindex = 27,           //!< arg1: index
    Analyze = 28,           //!< arg1: table
    CreateVtable = 29,      //!< arg1: table, arg2: module
    DropVtable = 30,        //!< arg1: table, arg2: module
    Function = 31,          //!< arg2: function name
    Savepoint = 32,         //!< arg1: BEGIN, RELEASE or ROLLBACK, arg2: savepoint name
    Recursive = 33,         //!< no arguments
    };

//=======================================================================================
//! The decision returned by an authorizer callback.
// @bsiclass
//=======================================================================================
enum class AuthorizerResult : int
    {
    Allow = 0,  //!< Permit the request
    Ignore = 1, //!< Honored only for Read/Function/Select/Recursive under ReadOnly(); otherwise treated as Deny
    Deny = 2,   //!< Reject the request; the statement fails with BE_SQLITE_AUTH (BE_SQLITE_ERROR for Function)
    };

//=======================================================================================
//! A single authorization request. Strings may be nullptr and are valid only during
//! the callback.
// @bsiclass
//=======================================================================================
struct AuthorizerRequest
    {
    AuthorizerAction m_action;
    Utf8CP m_arg1;
    Utf8CP m_arg2;
    Utf8CP m_database; //!< Schema name ("main", "temp", alias) or nullptr
    Utf8CP m_source;   //!< Innermost trigger or view responsible, or nullptr
    };

typedef std::function<AuthorizerResult(AuthorizerRequest const&)> AuthorizerCallback;

//=======================================================================================
//! A copy of the most recent request denied by a ScopedPolicy, for diagnostics.
// @bsiclass
//=======================================================================================
struct AuthorizerDenial
    {
    AuthorizerAction m_action = AuthorizerAction::Copy;
    std::optional<Utf8String> m_arg1;
    std::optional<Utf8String> m_arg2;
    std::optional<Utf8String> m_database;
    std::optional<Utf8String> m_source;
    bool m_callbackFailed = false; //!< A custom callback threw or returned an invalid result
    BE_SQLITE_EXPORT Utf8String ToString() const;
    };

//=======================================================================================
//! RAII guard that installs an authorization policy on a Db connection using
//! sqlite3_set_authorizer. Constraints can be declared at any time while the guard is
//! alive; the destructor removes them and restores the previous policy (if any) along
//! with any hardening settings this guard changed.
//!
//! - Guards nest in LIFO order. Only the innermost guard is evaluated: an inner guard
//!   temporarily replaces, rather than combines with, the outer policy.
//! - Built-in constraints run first, then custom callbacks in insertion order. The first
//!   Deny wins; otherwise any Ignore yields Ignore; otherwise Allow.
//! - AuthorizerResult::Ignore is honored only for Read, Function, Select and Recursive
//!   while ReadOnly() is active and no ChangeTracker/ChangeStream operation is running.
//!   Every other Ignore is treated as Deny, because SQLite turns an ignored write or DDL
//!   into a silent no-op or a corrupt result.
//! - Built-in transaction rules deny only starting a transaction or savepoint; COMMIT,
//!   ROLLBACK and RELEASE are never denied by built-ins so open transactions can end.
//! - Helpers are strict: they also deny requests SQLite issues for its own bookkeeping
//!   (e.g. DenyInsert() blocks CREATE TABLE, DenyAttach() blocks VACUUM).
//! - Errors never throw. The first error is reported by GetStatus(); an error after the
//!   guard became active (failed registration, invalid callback, hardening failure)
//!   puts the guard into fail-closed mode where every request is denied.
//! - Every scope entry, exit and constraint addition expires prepared statements; they
//!   are re-authorized when next stepped.
//!
//! Limits: authorization happens when SQL is compiled. Expressions stored in the schema
//! (VIRTUAL generated columns, CHECK, expression and partial indexes) are not authorized
//! at use time; use TrustedSchema(false). BlobIO::Open is checked against the policy but
//! BlobIO::ReOpen is not. Do not call sqlite3_set_authorizer directly on a guarded Db.
//! Closing the Db while guards are alive detaches them; their destructors then do nothing.
//! The policy is connection-wide and also applies to SQL run internally by BeSQLite
//! (e.g. property and briefcase-local-value writes during SaveChanges).
//!
//! @code
//! {
//! ScopedPolicy policy(db);
//! policy.ReadOnly().DenyTable("secrets").DenyColumn("users", "password").Defensive();
//! EXPECT_EQ(BE_SQLITE_OK, policy.GetStatus());
//! // ... queries; writes fail with BE_SQLITE_AUTH
//! } // previous authorizer and settings restored
//! @endcode
// @bsiclass
//=======================================================================================
struct ScopedPolicy : NonCopyableClass
{
    friend struct BlobIO;
    friend struct DbFile;

private:
    struct SavedSetting { bool m_isLimit; int m_id; int m_value; };

    DbFile* m_file = nullptr;
    ScopedPolicy* m_prev = nullptr;
    std::vector<AuthorizerCallback> m_builtins;
    std::vector<AuthorizerCallback> m_callbacks;
    std::vector<SavedSetting> m_saved;
    std::optional<AuthorizerDenial> m_lastDenial;
    DbResult m_status = BE_SQLITE_OK;
    bool m_readOnly = false;
    bool m_failClosed = false;

    void SetError(DbResult);
    void Refresh();
    AuthorizerResult Evaluate(AuthorizerRequest const&, bool& callbackFailed) const;
    void RecordDenial(AuthorizerRequest const&, bool callbackFailed);
    static int AuthorizerShim(void*, int, char const*, char const*, char const*, char const*);
    ScopedPolicy& AddBuiltin(AuthorizerCallback);
    ScopedPolicy& SetDbConfig(int op, bool on);
    bool RequireActive();
    static DbResult CheckBlobAccess(DbR db, Utf8CP schema, Utf8CP table, Utf8CP column, bool writable);

public:
    //! Install an empty (allow-all) policy on @p db. If @p db is not open the guard is
    //! inert and GetStatus() returns BE_SQLITE_ERROR_NOTOPEN.
    BE_SQLITE_EXPORT explicit ScopedPolicy(DbR db);
    //! Install a policy with an initial custom callback.
    BE_SQLITE_EXPORT ScopedPolicy(DbR db, AuthorizerCallback callback);
    //! Restore the hardening settings changed by this guard and the previous policy.
    BE_SQLITE_EXPORT ~ScopedPolicy();

    //! BE_SQLITE_OK, or the first error encountered by this guard.
    DbResult GetStatus() const {return m_status;}
    //! True when this guard is the innermost policy of an open connection.
    BE_SQLITE_EXPORT bool IsActive() const;
    //! True when an error has put this guard into deny-all mode.
    bool IsFailClosed() const {return m_failClosed;}
    //! The most recent request denied while this guard was active, or nullptr.
    AuthorizerDenial const* GetLastDenial() const {return m_lastDenial ? &*m_lastDenial : nullptr;}
    void ClearLastDenial() {m_lastDenial.reset();}

    //! @name Constraints (chainable)
    //! @{
    //! Add a custom callback. An empty callback is a misuse and puts the guard in fail-closed mode.
    BE_SQLITE_EXPORT ScopedPolicy& Add(AuthorizerCallback callback);
    //! Deny every request with @p action (Transaction/Savepoint: only BEGIN).
    BE_SQLITE_EXPORT ScopedPolicy& Deny(AuthorizerAction action);
    //! Deny every request naming @p table (case-insensitive). @p schema nullptr matches any schema;
    //! a request without a schema matches any rule schema. sqlite_schema is treated as sqlite_master.
    BE_SQLITE_EXPORT ScopedPolicy& DenyTable(Utf8CP table, Utf8CP schema = nullptr);
    //! Deny Read/Update of @p table.@p column, and ALTER TABLE forms that report the column.
    BE_SQLITE_EXPORT ScopedPolicy& DenyColumn(Utf8CP table, Utf8CP column, Utf8CP schema = nullptr);
    //! Deny calls to the SQL function @p name (case-insensitive).
    BE_SQLITE_EXPORT ScopedPolicy& DenyFunction(Utf8CP name);
    //! Deny PRAGMA @p name (case-insensitive), including pragma_* table-valued functions.
    BE_SQLITE_EXPORT ScopedPolicy& DenyPragma(Utf8CP name);
    //! Permit only queries, transaction control, savepoints and the introspection pragmas
    //! table_info, table_xinfo, index_list, index_info, index_xinfo, foreign_key_list and
    //! data_version. Enables Ignore-based read redaction from custom callbacks.
    BE_SQLITE_EXPORT ScopedPolicy& ReadOnly();
    //! Deny every action not in @p actions (transaction/savepoint end requests are always allowed).
    //! Multiple calls intersect.
    BE_SQLITE_EXPORT ScopedPolicy& AllowOnly(std::vector<AuthorizerAction> actions);

    ScopedPolicy& DenyCopy() {return Deny(AuthorizerAction::Copy);}
    ScopedPolicy& DenyCreateIndex() {return Deny(AuthorizerAction::CreateIndex);}
    ScopedPolicy& DenyCreateTable() {return Deny(AuthorizerAction::CreateTable);}
    ScopedPolicy& DenyCreateTempIndex() {return Deny(AuthorizerAction::CreateTempIndex);}
    ScopedPolicy& DenyCreateTempTable() {return Deny(AuthorizerAction::CreateTempTable);}
    ScopedPolicy& DenyCreateTempTrigger() {return Deny(AuthorizerAction::CreateTempTrigger);}
    ScopedPolicy& DenyCreateTempView() {return Deny(AuthorizerAction::CreateTempView);}
    ScopedPolicy& DenyCreateTrigger() {return Deny(AuthorizerAction::CreateTrigger);}
    ScopedPolicy& DenyCreateView() {return Deny(AuthorizerAction::CreateView);}
    ScopedPolicy& DenyDelete() {return Deny(AuthorizerAction::Delete);}
    ScopedPolicy& DenyDropIndex() {return Deny(AuthorizerAction::DropIndex);}
    ScopedPolicy& DenyDropTable() {return Deny(AuthorizerAction::DropTable);}
    ScopedPolicy& DenyDropTempIndex() {return Deny(AuthorizerAction::DropTempIndex);}
    ScopedPolicy& DenyDropTempTable() {return Deny(AuthorizerAction::DropTempTable);}
    ScopedPolicy& DenyDropTempTrigger() {return Deny(AuthorizerAction::DropTempTrigger);}
    ScopedPolicy& DenyDropTempView() {return Deny(AuthorizerAction::DropTempView);}
    ScopedPolicy& DenyDropTrigger() {return Deny(AuthorizerAction::DropTrigger);}
    ScopedPolicy& DenyDropView() {return Deny(AuthorizerAction::DropView);}
    ScopedPolicy& DenyInsert() {return Deny(AuthorizerAction::Insert);}
    ScopedPolicy& DenyPragma() {return Deny(AuthorizerAction::Pragma);}
    ScopedPolicy& DenyRead() {return Deny(AuthorizerAction::Read);}
    ScopedPolicy& DenySelect() {return Deny(AuthorizerAction::Select);}
    ScopedPolicy& DenyTransaction() {return Deny(AuthorizerAction::Transaction);}
    ScopedPolicy& DenyUpdate() {return Deny(AuthorizerAction::Update);}
    ScopedPolicy& DenyAttach() {return Deny(AuthorizerAction::Attach);}
    ScopedPolicy& DenyDetach() {return Deny(AuthorizerAction::Detach);}
    ScopedPolicy& DenyAlterTable() {return Deny(AuthorizerAction::AlterTable);}
    ScopedPolicy& DenyReindex() {return Deny(AuthorizerAction::Reindex);}
    ScopedPolicy& DenyAnalyze() {return Deny(AuthorizerAction::Analyze);}
    ScopedPolicy& DenyCreateVtable() {return Deny(AuthorizerAction::CreateVtable);}
    ScopedPolicy& DenyDropVtable() {return Deny(AuthorizerAction::DropVtable);}
    ScopedPolicy& DenyFunction() {return Deny(AuthorizerAction::Function);}
    ScopedPolicy& DenySavepoint() {return Deny(AuthorizerAction::Savepoint);}
    ScopedPolicy& DenyRecursive() {return Deny(AuthorizerAction::Recursive);}
    //! @}

    //! @name Hardening (chainable, innermost active guard only; restored by the destructor)
    //! Calling these on a guard that is not active records BE_SQLITE_MISUSE and fails closed.
    //! @{
    BE_SQLITE_EXPORT ScopedPolicy& Defensive(bool on = true);       //!< SQLITE_DBCONFIG_DEFENSIVE
    BE_SQLITE_EXPORT ScopedPolicy& TrustedSchema(bool on);          //!< SQLITE_DBCONFIG_TRUSTED_SCHEMA
    BE_SQLITE_EXPORT ScopedPolicy& TriggersEnabled(bool on);        //!< SQLITE_DBCONFIG_ENABLE_TRIGGER (TEMP triggers unaffected)
    BE_SQLITE_EXPORT ScopedPolicy& ViewsEnabled(bool on);           //!< SQLITE_DBCONFIG_ENABLE_VIEW (TEMP views unaffected)
    BE_SQLITE_EXPORT ScopedPolicy& AttachCreate(bool on);           //!< SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE
    BE_SQLITE_EXPORT ScopedPolicy& AttachWrite(bool on);            //!< SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE
    BE_SQLITE_EXPORT ScopedPolicy& MaxAttached(int n);              //!< SQLITE_LIMIT_ATTACHED
    //! @}
};

END_BENTLEY_SQLITE_NAMESPACE
