/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeSQLite/Profiler.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeAssert.h>
#include <Bentley/Logging.h>
#include <Bentley/BeStringUtilities.h>
#include <string>
#include <unordered_map>
#include <list>

#define LOG (NativeLogging::CategoryLogger("BeSQLite"))

using namespace std;
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE


BEGIN_BENTLEY_SQLITE_NAMESPACE

static Db::AppData::Key s_key;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Profiler::Scope::CmpChar::operator()(Utf8CP lhs, Utf8CP rhs) const {
    return strcmp(lhs, rhs) == 0;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t Profiler::Scope::HashChar::operator()(Utf8CP str) const {
    size_t l = str == nullptr ? 0 : strlen(str);
    // fnv 64bit hash
    size_t hash = 14695981039346656037ull;
    hash ^= l;
    hash *= 1099511628211ull;
    for (auto i = 0; i < l ;++i) {
            hash ^= (size_t)str[i];
            hash *= 1099511628211ull;
        }
    return hash;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RefCountedPtr<Profiler::Scope> Profiler::Scope::Create(DbCR db, Utf8CP scopeName, Utf8CP sessionName, Profiler::Params param) {
    return new Scope(db, scopeName, sessionName, param);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Profiler::Scope::Scope(DbCR db, Utf8CP scopeName, Utf8CP scenarioName, Profiler::Params param)
    : m_db(db), m_running(false),m_paused(false),m_param(param), m_scopeName(scopeName),m_scenarioName(scenarioName),m_scopeId(0) {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t Profiler::Scope::GetElapsedTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timepoint).count();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::Pause() const {
    if (!m_running) {
        return BE_SQLITE_ERROR;
    }
    if (m_paused) {
        return BE_SQLITE_OK;
    }
    m_pauseTimePoint = std::chrono::steady_clock::now();
    m_paused = true;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::Resume() const {
    if (!m_running) {
        return BE_SQLITE_ERROR;
    }
    if (!m_paused) {
        return BE_SQLITE_OK;
    }
    // append paused time to start so we do not count any skipped time in scope.
    m_timepoint += (std::chrono::steady_clock::now() - m_pauseTimePoint);
    m_paused = false;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::Start() const {
    DbResult rc;
    m_fileName.AppendUtf8(m_db.GetDbFileName());
    if (!m_scenarioName.empty()) {
        m_fileName.AppendUtf8("-");
        m_fileName.AppendUtf8(m_scenarioName.c_str());
    }
    m_fileName.AppendUtf8("-profile.db");
    if (m_fileName.DoesPathExist() && m_param.OverrideProfileDb()) {
        m_fileName.BeDeleteFile();
    }
    if (!m_fileName.DoesPathExist()) {
        rc = m_profileDb.CreateNewDb(m_fileName);
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        if (InitProfileDb() != BE_SQLITE_OK) {
            if (m_profileDb.IsDbOpen())
                m_profileDb.CloseDb();

            m_fileName.BeDeleteFile();
            return BE_SQLITE_ERROR;
        }
    } else {
       rc = m_profileDb.OpenBeSQLiteDb(m_fileName, Db::OpenParams(Db::OpenMode::ReadWrite));
       if (BE_SQLITE_OK != rc) {
           return rc;
        }
    }
    rc = BeginSession();
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    return InstallListener();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::InstallListener() const {
    m_cancelCb = m_db.GetTraceProfileEvent().AddListener(
        [&](TraceContext const &ctx, int64_t nanoseconds) {
            if (m_paused) {
                return;
            }
            auto sql = ctx.GetSql();
            auto it = m_profile.find(sql);
            if (it == m_profile.end()) {
                Utf8CP cachedSql = m_sqlList.insert(m_sqlList.end(), sql)->c_str();
                QueryStats& stats = m_profile.insert(std::make_pair(cachedSql, QueryStats())).first->second;
                stats.m_count = 1;
                stats.m_elapsed = nanoseconds;
                stats.m_scans = -1;
                if (m_param.ComputeExecutionPlan()) {
                    ComputeExecutionPlan(sql, stats.m_plan, stats.m_scans);
                }
            } else {
                it->second.m_count++;
                it->second.m_elapsed += nanoseconds;
            }
        });

    m_db.ConfigTraceEvents(DbTrace::Profile, true);
    m_paused = false;
    m_running = true;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Profiler::Scope::RemoveListener() const {
    if (m_cancelCb) {
        m_cancelCb();
        m_cancelCb = nullptr;
    }
    m_running = false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::InitProfileDb() const {
    if (m_profileDb.TryExecuteSql(R"(
        CREATE TABLE [sql_list](
            [id] integer PRIMARY KEY,
            [sql] text,
            [plan] text,
            [scan] integer);
        CREATE TABLE [sql_scope](
            [id] integer PRIMARY KEY,
            [name] TEXT,
            [file_guid] TEXT,
            [elapsed_ms] INTEGER,
            [memory_diff] INTEGER,
            [memory_high] INTEGER,
            [file_path] TEXT);
        CREATE TABLE [sql_profile](
            [id] integer PRIMARY KEY,
            [scope_id] integer REFERENCES [sql_scope]([id]) ON DELETE CASCADE,
            [elapsed_ns] integer,
            [row_count] integer,
            [sql_id] integer REFERENCES [sql_list]([id]) ON DELETE CASCADE);
        CREATE VIEW [v_perf_scope] AS
        SELECT
            *,
            ROUND ((1 - ([scope_elapsed_sec] - [sql_elapsed_sec]) / [scope_elapsed_sec]) * 100, 0) [sql_time_%],
            ROUND (([scope_elapsed_sec] / SUM ([scope_elapsed_sec]) OVER ()) * 100, 3) [total_time_%]
        FROM   (SELECT
                    [id],
                    [name],
                    (CASE WHEN [elapsed_sec] = 0 THEN [sql_elapsed_sec] ELSE [elapsed_sec] END) [scope_elapsed_sec],
                    [sql_elapsed_sec],
                    [memory_diff],
                    [memory_high]
                FROM   (SELECT
                            [id],
                            [name],
                            [elapsed_ms] / 1000 [elapsed_sec],
                            [memory_diff],
                            [memory_high],
                            [file_path],
                            (SELECT SUM ([elapsed_ns]) / 1000000000.0
                            FROM   [sql_profile]
                            WHERE  [sql_profile].[scope_id] = [sql_scope].[id]) [sql_elapsed_sec]
                        FROM   [sql_scope]
                        WHERE  [elapsed_ms] IS NOT NULL));

        CREATE VIEW [v_perf_all] AS
        SELECT
            [scope_time_min],
            [sql_time_min],
            ROUND ((1 - ([scope_time_min] - [sql_time_min]) / [scope_time_min]) * 100, 2) [sql_time_%]
        FROM   (SELECT
                    ROUND (SUM ([scope_elapsed_sec]) / (60), 4) [scope_time_min],
                    ROUND (SUM ([sql_elapsed_sec]) / (60), 4) [sql_time_min]
                FROM   [v_perf_scope]);

        CREATE VIEW [v_perf_profile] AS
        SELECT
            [scope_id],
            [sql_id],
            ROUND (SUM ([elapsed_ns] / 1000000000.0), 2) [elapsed_sec],
            SUM ([row_count]) [row_count],
            ROUND (SUM ([row_count]) / SUM ([elapsed_ns] / 1000000000.0), 2) [speed]
        FROM   [sql_profile]
        WHERE  [elapsed_ns] > 0
        GROUP  BY [sql_id];

        CREATE VIEW [v_perf_sql] AS
        SELECT
            *,
            SUBSTR ([sql], 0, 8) [op],
            ROUND (([elapsed_sec] / SUM ([elapsed_sec]) OVER ()) * 100, 2) [total_time_%]
        FROM   (SELECT
                    [v_perf_profile].*,
                    [sql_list].[sql]
                FROM   [v_perf_profile],
                    [sql_list]
                WHERE  [v_perf_profile].[sql_id] = [sql_list].[id]
                        AND [sql] NOT LIKE 'PRAGMA%')
        ORDER  BY [elapsed_sec] DESC;
        CREATE INDEX [idx_sql_list_list] ON [sql_list]([sql]);
        CREATE INDEX [idx_sql_profile_scope_id] ON [sql_profile]([scope_id]);
        CREATE INDEX [idx_sql_profile_sql_id] ON [sql_profile]([sql_id]);)") != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        m_profileDb.CloseDb();
        m_fileName.BeDeleteFile();
        return BE_SQLITE_ERROR;
        }
    return m_profileDb.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::BeginSession() const {
    m_timepoint = std::chrono::steady_clock::now();
    Statement stmt;
    DbResult rc;
    rc = stmt.Prepare(m_profileDb, "insert into sql_scope([id], name, file_guid, elapsed_ms, memory_diff, memory_high, file_path) values(null,?, ?, ?, ?, null,?)");
    if(rc != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        return rc;
    }

    int64_t currentMem, highMem;
    BeSQLiteLib::GetMemoryUsed(currentMem, highMem, true);
    if (!m_scopeName.empty()) {
        stmt.BindText(1, m_scopeName, Statement::MakeCopy::No);
    }
    stmt.BindInt64(2, m_timepoint.time_since_epoch().count());
    stmt.BindText(3, m_db.GetDbGuid().ToString(), Statement::MakeCopy::Yes);
    stmt.BindInt64(4, currentMem);
    stmt.BindText(5, m_profileDb.GetDbFileName(), Statement::MakeCopy::No);

    rc = stmt.Step();
    m_scopeId = m_profileDb.GetLastInsertRowId();
    if (rc == BE_SQLITE_DONE) {
        return m_profileDb.SaveChanges();
    }
    return BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::EndSession() const{
    auto elapsedTime = GetElapsedTime();
    Statement stmt;
    DbResult rc = stmt.Prepare(m_profileDb, "update sql_scope set elapsed_ms=?, memory_diff=?-memory_diff, memory_high=? where id=?");
    if(rc != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        return rc;
    }

    int64_t currentMem, highMem;
    BeSQLiteLib::GetMemoryUsed(currentMem, highMem);
    stmt.BindInt64(1, elapsedTime);
    stmt.BindInt64(2, currentMem);
    stmt.BindInt64(3, highMem);
    stmt.BindInt64(4, m_scopeId);
    return stmt.Step() == BE_SQLITE_DONE ? BE_SQLITE_OK : BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::Scope::Stop() const {
    DbResult rc;
    RemoveListener();

    if (!m_profileDb.IsDbOpen()) {
        return BE_SQLITE_ERROR;
    }

    rc = EndSession();
    if (rc != BE_SQLITE_OK)
        return rc;

    Statement profileStmt;
    rc = profileStmt.Prepare(m_profileDb, "insert into sql_profile([id], [scope_id], [elapsed_ns], [row_count], [sql_id]) values(null,?, ?,?,?)");
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        return rc;
    }

    Statement sqlStmt;
    rc = sqlStmt.Prepare(m_profileDb, "insert into sql_list([id], [sql], [plan], [scan]) values(null,?,?,?)");
    if (rc != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        return rc;
    }

    Statement idStmt;
    rc = idStmt.Prepare(m_profileDb, "select [id] from [sql_list] where  [sql]=?");
    if(rc != BE_SQLITE_OK) {
        m_lastError = m_profileDb.GetLastError();
        return rc;
    }

    for (auto& sql: m_sqlList) {
        QueryStats& stats = m_profile[sql.c_str()];
        int64_t sqlId = 0;
        idStmt.ClearBindings();
        idStmt.Reset();
        idStmt.BindText(1, sql, Statement::MakeCopy::No);
        if (idStmt.Step() == BE_SQLITE_ROW) {
            sqlId = idStmt.GetValueInt64(0);
        } else {
            sqlStmt.ClearBindings();
            sqlStmt.Reset();
            sqlStmt.BindText(1, sql, Statement::MakeCopy::No);
            if (!stats.m_plan.empty()) {
                sqlStmt.BindText(2, stats.m_plan, Statement::MakeCopy::No);
                sqlStmt.BindInt(3, stats.m_scans);
            }

            rc = sqlStmt.Step();
            if (rc == BE_SQLITE_DONE) {
                sqlId = m_profileDb.GetLastInsertRowId();
            } else {
                m_lastError = m_profileDb.GetLastError();
                return rc;
            }
        }

        profileStmt.ClearBindings();
        profileStmt.Reset();
        profileStmt.BindInt64(1, m_scopeId);
        profileStmt.BindInt64(2, stats.m_elapsed);
        profileStmt.BindInt64(3, stats.m_count);
        profileStmt.BindInt64(4, sqlId);
        rc = profileStmt.Step();
        if (rc != BE_SQLITE_DONE) {
            m_lastError = m_profileDb.GetLastError();
            return rc;
        }
    }
    return m_profileDb.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Profiler::Scope::~Scope() {
    if (IsRunning()) {
        Stop();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Profiler::Scope::ComputeExecutionPlan(Utf8CP sql, Utf8StringR plan, int& scans) const {
    bool oldPausedValue = m_paused;
    m_paused = true;
    Utf8String explainQuery = " EXPLAIN QUERY PLAN ";
    explainQuery.append(sql);
    Statement stmt;
    scans = 0;
    plan.clear();
    if (stmt.Prepare(m_db, explainQuery.c_str()) == BE_SQLITE_OK) {
        while(stmt.Step() == BE_SQLITE_ROW) {
            Utf8String str = stmt.GetValueText(3);
            plan.append(str).append("\n");
            if(plan.StartsWithIAscii("SCAN"))
                scans++;
        }
    }
    m_paused = oldPausedValue;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Profiler::InitScope(DbCR db, Utf8CP scopeName, Utf8CP sessionName, Profiler::Params param) {
    auto currentScope = GetScope(db);
    if (currentScope && (currentScope->IsRunning() || currentScope->IsPaused())) {
        LOG.error("There is already a profiling scope active. Call Stop() on it and then start a different scope.");
        return BE_SQLITE_ERROR;
    } else {
        db.DropAppData(s_key);
    }
    auto newScope = Scope::Create(db, scopeName, sessionName, param);
    db.AddAppData(s_key, newScope.get());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Profiler::Scope const* Profiler::GetScope(DbCR db) {
    BeSQLite::Db::AppDataPtr appdata = db.FindAppData(s_key);
        return static_cast<Scope*>(appdata.get());
    return nullptr;
}


END_BENTLEY_SQLITE_NAMESPACE
