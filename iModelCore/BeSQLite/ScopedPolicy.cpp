/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/ScopedPolicy.h>
#include "ScopedPolicyInternal.h"
#include "SQLite/sqlite3.h"
#include <Bentley/BeAssert.h>
#include <algorithm>

BEGIN_BENTLEY_SQLITE_NAMESPACE

#define BE_CHECK_ACTION(name, code) static_assert((int) AuthorizerAction::name == (code), "AuthorizerAction::" #name " does not match " #code)
BE_CHECK_ACTION(CreateIndex, SQLITE_CREATE_INDEX);
BE_CHECK_ACTION(CreateTable, SQLITE_CREATE_TABLE);
BE_CHECK_ACTION(CreateTempIndex, SQLITE_CREATE_TEMP_INDEX);
BE_CHECK_ACTION(CreateTempTable, SQLITE_CREATE_TEMP_TABLE);
BE_CHECK_ACTION(CreateTempTrigger, SQLITE_CREATE_TEMP_TRIGGER);
BE_CHECK_ACTION(CreateTempView, SQLITE_CREATE_TEMP_VIEW);
BE_CHECK_ACTION(CreateTrigger, SQLITE_CREATE_TRIGGER);
BE_CHECK_ACTION(CreateView, SQLITE_CREATE_VIEW);
BE_CHECK_ACTION(Delete, SQLITE_DELETE);
BE_CHECK_ACTION(DropIndex, SQLITE_DROP_INDEX);
BE_CHECK_ACTION(DropTable, SQLITE_DROP_TABLE);
BE_CHECK_ACTION(DropTempIndex, SQLITE_DROP_TEMP_INDEX);
BE_CHECK_ACTION(DropTempTable, SQLITE_DROP_TEMP_TABLE);
BE_CHECK_ACTION(DropTempTrigger, SQLITE_DROP_TEMP_TRIGGER);
BE_CHECK_ACTION(DropTempView, SQLITE_DROP_TEMP_VIEW);
BE_CHECK_ACTION(DropTrigger, SQLITE_DROP_TRIGGER);
BE_CHECK_ACTION(DropView, SQLITE_DROP_VIEW);
BE_CHECK_ACTION(Insert, SQLITE_INSERT);
BE_CHECK_ACTION(Pragma, SQLITE_PRAGMA);
BE_CHECK_ACTION(Read, SQLITE_READ);
BE_CHECK_ACTION(Select, SQLITE_SELECT);
BE_CHECK_ACTION(Transaction, SQLITE_TRANSACTION);
BE_CHECK_ACTION(Update, SQLITE_UPDATE);
BE_CHECK_ACTION(Attach, SQLITE_ATTACH);
BE_CHECK_ACTION(Detach, SQLITE_DETACH);
BE_CHECK_ACTION(AlterTable, SQLITE_ALTER_TABLE);
BE_CHECK_ACTION(Reindex, SQLITE_REINDEX);
BE_CHECK_ACTION(Analyze, SQLITE_ANALYZE);
BE_CHECK_ACTION(CreateVtable, SQLITE_CREATE_VTABLE);
BE_CHECK_ACTION(DropVtable, SQLITE_DROP_VTABLE);
BE_CHECK_ACTION(Function, SQLITE_FUNCTION);
BE_CHECK_ACTION(Savepoint, SQLITE_SAVEPOINT);
BE_CHECK_ACTION(Recursive, SQLITE_RECURSIVE);
#undef BE_CHECK_ACTION

namespace {

typedef AuthorizerAction Action;
typedef AuthorizerResult Result;

char AsciiLower(char c) {return (c >= 'A' && c <= 'Z') ? (char) (c - 'A' + 'a') : c;}

bool IEquals(Utf8CP a, Utf8CP b)
    {
    if (nullptr == a || nullptr == b)
        return false;
    for (; *a && *b; ++a, ++b)
        {
        if (AsciiLower(*a) != AsciiLower(*b))
            return false;
        }
    return *a == *b;
    }

// The authorizer reports sqlite_master/sqlite_temp_master for the sqlite_schema aliases.
Utf8String CanonicalTable(Utf8CP name)
    {
    if (IEquals(name, "sqlite_schema"))
        return "sqlite_master";
    if (IEquals(name, "sqlite_temp_schema"))
        return "sqlite_temp_master";
    return name;
    }

bool TableMatches(Utf8CP reported, Utf8StringCR rule)
    {
    if (IEquals(reported, "sqlite_schema"))
        reported = "sqlite_master";
    else if (IEquals(reported, "sqlite_temp_schema"))
        reported = "sqlite_temp_master";
    return IEquals(reported, rule.c_str());
    }

// An empty rule schema matches any request; a request without a schema matches any rule (fail closed).
bool SchemaMatches(Utf8CP reported, Utf8StringCR rule) {return rule.empty() || nullptr == reported || IEquals(reported, rule.c_str());}

bool IsStarting(AuthorizerRequest const& r) {return IEquals(r.m_arg1, "BEGIN");}
bool IsEnding(AuthorizerRequest const& r) {return (r.m_action == Action::Transaction || r.m_action == Action::Savepoint) && !IsStarting(r);}
bool IsRedactable(Action a) {return a == Action::Read || a == Action::Function || a == Action::Select || a == Action::Recursive;}

Utf8CP TableArg(AuthorizerRequest const& r)
    {
    switch (r.m_action)
        {
        case Action::CreateTable: case Action::CreateTempTable:
        case Action::CreateView: case Action::CreateTempView:
        case Action::DropTable: case Action::DropTempTable:
        case Action::DropView: case Action::DropTempView:
        case Action::Insert: case Action::Update: case Action::Delete: case Action::Read:
        case Action::Analyze: case Action::CreateVtable: case Action::DropVtable:
            return r.m_arg1;
        case Action::CreateIndex: case Action::CreateTempIndex:
        case Action::DropIndex: case Action::DropTempIndex:
        case Action::CreateTrigger: case Action::CreateTempTrigger:
        case Action::DropTrigger: case Action::DropTempTrigger:
        case Action::AlterTable:
            return r.m_arg2;
        default:
            return nullptr;
        }
    }

// ALTER TABLE reports the schema in arg1.
Utf8CP SchemaArg(AuthorizerRequest const& r) {return r.m_action == Action::AlterTable ? r.m_arg1 : r.m_database;}

bool IsReadOnlyPragma(Utf8CP name)
    {
    static Utf8CP const s_allowed[] = {"table_info", "table_xinfo", "index_list", "index_info", "index_xinfo", "foreign_key_list", "data_version"};
    return std::any_of(std::begin(s_allowed), std::end(s_allowed), [name](Utf8CP p) {return IEquals(name, p);});
    }

Utf8CP ActionName(Action a)
    {
    static Utf8CP const s_names[] = {"Copy", "CreateIndex", "CreateTable", "CreateTempIndex", "CreateTempTable", "CreateTempTrigger",
        "CreateTempView", "CreateTrigger", "CreateView", "Delete", "DropIndex", "DropTable", "DropTempIndex", "DropTempTable",
        "DropTempTrigger", "DropTempView", "DropTrigger", "DropView", "Insert", "Pragma", "Read", "Select", "Transaction", "Update",
        "Attach", "Detach", "AlterTable", "Reindex", "Analyze", "CreateVtable", "DropVtable", "Function", "Savepoint", "Recursive"};
    int i = (int) a;
    return (i < 0 || i >= (int) (sizeof(s_names) / sizeof(s_names[0]))) ? "Unknown" : s_names[i];
    }

std::optional<Utf8String> Copy(Utf8CP s) {return s ? std::optional<Utf8String>(Utf8String(s)) : std::nullopt;}

thread_local ForbidAuthorizerIgnore* t_forbidIgnoreTop = nullptr;

} // anonymous namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ForbidAuthorizerIgnore::ForbidAuthorizerIgnore(SqlDbP db) : m_db(db), m_prev(t_forbidIgnoreTop) {t_forbidIgnoreTop = this;}
ForbidAuthorizerIgnore::~ForbidAuthorizerIgnore() {BeAssert(t_forbidIgnoreTop == this); t_forbidIgnoreTop = m_prev;}
bool ForbidAuthorizerIgnore::IsActive(SqlDbP db)
    {
    for (auto f = t_forbidIgnoreTop; nullptr != f; f = f->m_prev)
        {
        if (f->m_db == db)
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AuthorizerDenial::ToString() const
    {
    Utf8PrintfString out("%s(%d)", ActionName(m_action), (int) m_action);
    auto field = [&out](Utf8CP label, std::optional<Utf8String> const& v)
        {
        if (v)
            out.append(Utf8PrintfString(" %s=\"%s\"", label, v->c_str()));
        };
    field("arg1", m_arg1);
    field("arg2", m_arg2);
    field("database", m_database);
    field("source", m_source);
    if (m_callbackFailed)
        out.append(" [callback failed]");
    return out;
    }

/*---------------------------------------------------------------------------------**//**
* Built-in constraints first, then callbacks. The first Deny wins, otherwise any Ignore.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AuthorizerResult ScopedPolicy::Evaluate(AuthorizerRequest const& r, bool& callbackFailed) const
    {
    callbackFailed = false;
    if (m_failClosed)
        return Result::Deny;

    try
        {
        for (auto const& rule : m_builtins)
            {
            if (rule(r) == Result::Deny)
                return Result::Deny;
            }

        Result combined = Result::Allow;
        for (auto const& cb : m_callbacks)
            {
            Result res = cb(r);
            if (res == Result::Deny)
                return Result::Deny;
            if (res == Result::Ignore)
                combined = Result::Ignore;
            else if (res != Result::Allow)
                {
                callbackFailed = true;
                return Result::Deny;
                }
            }
        return combined;
        }
    catch (...)
        {
        // Never let an exception unwind through SQLite.
        callbackFailed = true;
        return Result::Deny;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedPolicy::RecordDenial(AuthorizerRequest const& r, bool callbackFailed)
    {
    try
        {
        AuthorizerDenial d;
        d.m_action = r.m_action;
        d.m_arg1 = Copy(r.m_arg1);
        d.m_arg2 = Copy(r.m_arg2);
        d.m_database = Copy(r.m_database);
        d.m_source = Copy(r.m_source);
        d.m_callbackFailed = callbackFailed;
        m_lastDenial = std::move(d);
        }
    catch (...)
        {
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ScopedPolicy::AuthorizerShim(void* user, int code, char const* arg1, char const* arg2, char const* dbName, char const* source)
    {
    auto& policy = *static_cast<ScopedPolicy*>(user);
    AuthorizerRequest r {(Action) code, arg1, arg2, dbName, source};
    bool callbackFailed = false;
    Result res = policy.Evaluate(r, callbackFailed);
    if (res == Result::Ignore)
        {
        // Ignore on anything but a read under ReadOnly() turns into silent no-ops or corrupt results.
        SqlDbP db = policy.m_file ? policy.m_file->m_sqlDb : nullptr;
        if (policy.m_readOnly && IsRedactable(r.m_action) && !ForbidAuthorizerIgnore::IsActive(db))
            return SQLITE_IGNORE;
        res = Result::Deny;
        }
    if (res == Result::Allow)
        return SQLITE_OK;

    policy.RecordDenial(r, callbackFailed);
    return SQLITE_DENY;
    }

/*---------------------------------------------------------------------------------**//**
* Synthesized Read (and Update when writable) check for BlobIO::Open, which never runs SQL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ScopedPolicy::CheckBlobAccess(DbR db, Utf8CP schema, Utf8CP table, Utf8CP column, bool writable)
    {
    if (!db.IsDbOpen())
        return BE_SQLITE_OK;

    ScopedPolicy* policy = db.GetDbFile()->m_policy;
    if (nullptr == policy)
        return BE_SQLITE_OK;

    for (Action action : {Action::Read, Action::Update})
        {
        if (action == Action::Update && !writable)
            break;
        AuthorizerRequest r {action, table, column, schema ? schema : "main", nullptr};
        bool callbackFailed = false;
        if (policy->Evaluate(r, callbackFailed) != Result::Allow) // Ignore is not meaningful for blobs
            {
            policy->RecordDenial(r, callbackFailed);
            return BE_SQLITE_AUTH;
            }
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy::ScopedPolicy(DbR db)
    {
    if (!db.IsDbOpen())
        {
        m_status = BE_SQLITE_ERROR_NOTOPEN;
        return;
        }

    DbFile* file = db.GetDbFile();
    int rc = sqlite3_set_authorizer(file->m_sqlDb, AuthorizerShim, this);
    if (SQLITE_OK != rc)
        {
        // Not published: the previous policy stays in effect.
        m_status = (DbResult) rc;
        return;
        }
    m_file = file;
    m_prev = file->m_policy;
    file->m_policy = this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy::ScopedPolicy(DbR db, AuthorizerCallback callback) : ScopedPolicy(db)
    {
    Add(std::move(callback));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy::~ScopedPolicy()
    {
    if (nullptr == m_file) // never activated, or the Db was closed first
        return;

    if (m_file->m_policy != this)
        {
        BeAssert(false && "ScopedPolicy guards must be destroyed in LIFO order");
        // Unlink so the inner guard does not restore a dangling pointer.
        for (ScopedPolicy* p = m_file->m_policy; nullptr != p; p = p->m_prev)
            {
            if (p->m_prev == this)
                {
                p->m_prev = m_prev;
                break;
                }
            }
        return;
        }

    SqlDbP h = m_file->m_sqlDb;
    for (auto it = m_saved.rbegin(); it != m_saved.rend(); ++it)
        {
        if (it->m_isLimit)
            sqlite3_limit(h, it->m_id, it->m_value);
        else
            sqlite3_db_config(h, it->m_id, it->m_value, (int*) nullptr);
        }

    m_file->m_policy = m_prev;
    int rc = m_prev ? sqlite3_set_authorizer(h, AuthorizerShim, m_prev) : sqlite3_set_authorizer(h, nullptr, nullptr);
    UNUSED_VARIABLE(rc);
    BeAssert(SQLITE_OK == rc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScopedPolicy::IsActive() const {return nullptr != m_file && m_file->m_policy == this;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedPolicy::SetError(DbResult rc)
    {
    if (BE_SQLITE_OK == m_status)
        m_status = rc;
    if (nullptr != m_file)
        m_failClosed = true;
    }

/*---------------------------------------------------------------------------------**//**
* Re-registering expires prepared statements so they are re-authorized at their next step.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScopedPolicy::Refresh()
    {
    if (!IsActive())
        return;
    int rc = sqlite3_set_authorizer(m_file->m_sqlDb, AuthorizerShim, this);
    if (SQLITE_OK != rc)
        SetError((DbResult) rc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::AddBuiltin(AuthorizerCallback rule)
    {
    m_builtins.push_back(std::move(rule));
    Refresh();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::Add(AuthorizerCallback callback)
    {
    if (!callback)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    m_callbacks.push_back(std::move(callback));
    Refresh();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::Deny(AuthorizerAction action)
    {
    if (action == Action::Transaction || action == Action::Savepoint)
        return AddBuiltin([action](AuthorizerRequest const& r) {return (r.m_action == action && IsStarting(r)) ? Result::Deny : Result::Allow;});

    return AddBuiltin([action](AuthorizerRequest const& r) {return r.m_action == action ? Result::Deny : Result::Allow;});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::DenyTable(Utf8CP table, Utf8CP schema)
    {
    if (nullptr == table)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    return AddBuiltin([name = CanonicalTable(table), schema = Utf8String(schema ? schema : "")](AuthorizerRequest const& r)
        {
        if (!TableMatches(TableArg(r), name))
            return Result::Allow;
        // A TEMP trigger reports "temp" even when it targets another schema.
        bool anySchema = r.m_action == Action::CreateTempTrigger || r.m_action == Action::DropTempTrigger;
        return (anySchema || SchemaMatches(SchemaArg(r), schema)) ? Result::Deny : Result::Allow;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::DenyColumn(Utf8CP table, Utf8CP column, Utf8CP schema)
    {
    if (nullptr == table || nullptr == column)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    return AddBuiltin([table = CanonicalTable(table), column = Utf8String(column), schema = Utf8String(schema ? schema : "")](AuthorizerRequest const& r)
        {
        bool hit = false;
        if (r.m_action == Action::Read || r.m_action == Action::Update)
            hit = TableMatches(r.m_arg1, table) && IEquals(r.m_arg2, column.c_str()) && SchemaMatches(r.m_database, schema);
        else if (r.m_action == Action::AlterTable) // DROP COLUMN and ALTER COLUMN ... NOT NULL report the column in the database slot
            hit = TableMatches(r.m_arg2, table) && IEquals(r.m_database, column.c_str()) && SchemaMatches(r.m_arg1, schema);
        return hit ? Result::Deny : Result::Allow;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::DenyFunction(Utf8CP name)
    {
    if (nullptr == name)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    return AddBuiltin([name = Utf8String(name)](AuthorizerRequest const& r)
        {
        return (r.m_action == Action::Function && IEquals(r.m_arg2, name.c_str())) ? Result::Deny : Result::Allow;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::DenyPragma(Utf8CP name)
    {
    if (nullptr == name)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    return AddBuiltin([name = Utf8String(name)](AuthorizerRequest const& r)
        {
        return (r.m_action == Action::Pragma && IEquals(r.m_arg1, name.c_str())) ? Result::Deny : Result::Allow;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::ReadOnly()
    {
    m_readOnly = true;
    return AddBuiltin([](AuthorizerRequest const& r)
        {
        switch (r.m_action)
            {
            case Action::Read:
            case Action::Select:
            case Action::Function:
            case Action::Recursive:
            case Action::Transaction:
            case Action::Savepoint:
                return Result::Allow;
            case Action::Pragma:
                return IsReadOnlyPragma(r.m_arg1) ? Result::Allow : Result::Deny;
            default:
                return Result::Deny;
            }
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::AllowOnly(std::vector<AuthorizerAction> actions)
    {
    return AddBuiltin([actions = std::move(actions)](AuthorizerRequest const& r)
        {
        if (IsEnding(r) || std::find(actions.begin(), actions.end(), r.m_action) != actions.end())
            return Result::Allow;
        return Result::Deny;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScopedPolicy::RequireActive()
    {
    if (nullptr == m_file)
        return false; // inert guard, status already reports the reason
    if (!IsActive())
        {
        SetError(BE_SQLITE_MISUSE);
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::SetDbConfig(int op, bool on)
    {
    if (!RequireActive())
        return *this;

    SqlDbP h = m_file->m_sqlDb;
    int current = 0;
    int rc = sqlite3_db_config(h, op, -1, &current);
    if (SQLITE_OK != rc)
        {
        SetError((DbResult) rc);
        return *this;
        }

    bool first = std::none_of(m_saved.begin(), m_saved.end(), [op](SavedSetting const& s) {return !s.m_isLimit && s.m_id == op;});
    if (first)
        m_saved.reserve(m_saved.size() + 1); // so push_back below cannot fail after the change

    int wanted = on ? 1 : 0;
    int applied = -1;
    rc = sqlite3_db_config(h, op, wanted, &applied);
    if (SQLITE_OK != rc || applied != wanted)
        {
        sqlite3_db_config(h, op, current, (int*) nullptr);
        SetError(SQLITE_OK != rc ? (DbResult) rc : BE_SQLITE_ERROR);
        return *this;
        }
    if (first)
        m_saved.push_back({false, op, current});
    return *this;
    }

ScopedPolicy& ScopedPolicy::Defensive(bool on) {return SetDbConfig(SQLITE_DBCONFIG_DEFENSIVE, on);}
ScopedPolicy& ScopedPolicy::TrustedSchema(bool on) {return SetDbConfig(SQLITE_DBCONFIG_TRUSTED_SCHEMA, on);}
ScopedPolicy& ScopedPolicy::TriggersEnabled(bool on) {return SetDbConfig(SQLITE_DBCONFIG_ENABLE_TRIGGER, on);}
ScopedPolicy& ScopedPolicy::ViewsEnabled(bool on) {return SetDbConfig(SQLITE_DBCONFIG_ENABLE_VIEW, on);}
ScopedPolicy& ScopedPolicy::AttachCreate(bool on) {return SetDbConfig(SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE, on);}
ScopedPolicy& ScopedPolicy::AttachWrite(bool on) {return SetDbConfig(SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE, on);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedPolicy& ScopedPolicy::MaxAttached(int n)
    {
    if (n < 0)
        {
        SetError(BE_SQLITE_MISUSE);
        return *this;
        }
    if (!RequireActive())
        return *this;

    bool first = std::none_of(m_saved.begin(), m_saved.end(), [](SavedSetting const& s) {return s.m_isLimit && s.m_id == SQLITE_LIMIT_ATTACHED;});
    if (first)
        m_saved.reserve(m_saved.size() + 1);
    int previous = sqlite3_limit(m_file->m_sqlDb, SQLITE_LIMIT_ATTACHED, n);
    if (first)
        m_saved.push_back({true, SQLITE_LIMIT_ATTACHED, previous});
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* Called by ~DbFile: the connection is going away, so detach every guard still alive.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DetachPolicies()
    {
    if (nullptr == m_policy)
        return;
    for (ScopedPolicy* p = m_policy; nullptr != p; p = p->m_prev)
        p->m_file = nullptr;
    m_policy = nullptr;
    if (nullptr != m_sqlDb)
        sqlite3_set_authorizer(m_sqlDb, nullptr, nullptr);
    }

END_BENTLEY_SQLITE_NAMESPACE
