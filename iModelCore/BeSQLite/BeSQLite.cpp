/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#define ZLIB_INTERNAL

#define SQLITE_ENABLE_SESSION 1
#define SQLITE_ENABLE_NORMALIZE 1
#include <BeSQLite/ChangeSet.h>
#include <BeSQLite/BeLzma.h>
#include "SQLite/sqlite3.h"
#include <Bentley/BeFileName.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/Logging.h>
#include "snappy/snappy.h"
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <string>
#include "BeSQLiteProfileManager.h"
#include <prg.h>
#include <unordered_map>
#include <list>
#include <re2/re2.h>

static NativeLogging::CategoryLogger LOG("BeSQLite");
static NativeLogging::CategoryLogger NativeSqliteLog("SQLite");

#define RUNONCE_CHECK(var,stat) {if (var) return stat; var=true;}
using namespace re2;
using namespace std;
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

#if !defined (NDEBUG)
extern "C" int checkNoActiveStatements(SqlDbP db);
#endif

BEGIN_BENTLEY_SQLITE_NAMESPACE

// NB: "repository" here really means "briefcase", but we don't want to break existing DgnDbs.
#define BEDB_BRIEFCASELOCALVALUE_BriefcaseId "be_repositoryid"

//=======================================================================================
// @bsiclass
// See https://github.com/google/re2/wiki/Syntax
// No need make this threadsafe. RE2 are already threadsafe and this function registered
// per db connection and cache is local to a single db connection.
//=======================================================================================
struct RegExpFunc final : ScalarFunction {
    private:
        const int kMaxMemUsedBySingleRegexp = 1024*1024*10;
        const int kMaxMemUsedByCachedRegexp = 1024*1024*20;
        const int kMaxCachedSize = 100;

        std::unordered_map<std::string,RE2>  m_regexpCache;
        int m_memoryUsed;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override {
            if (nArgs != 2) {
                ctx.SetResultError("expecting two argument regexp(Y,X)");
                return;
            }
            auto regexp = args[0];
            auto data = args[1];
            if (!regexp.IsValid() || !data.IsValid() || regexp.IsNull() || data.IsNull()) {
                return;
            }
            if (regexp.GetValueType () != DbValueType::TextVal) {
                ctx.SetResultError("string expression expected for regexp");
                return;
            }
            if (data.GetValueType () != DbValueType::TextVal) {
                return;
            }
            auto cRegExp = regexp.GetValueText();
            auto cData = data.GetValueText();
            auto it = m_regexpCache.find(cRegExp);
            if (it == m_regexpCache.end()) {
                // Clear all cache if we exceeds any limit
                if (m_regexpCache.size() > kMaxCachedSize || m_memoryUsed > kMaxMemUsedByCachedRegexp) {
                    m_regexpCache.clear();
                    m_memoryUsed = 0;
                }
                RE2::Options opts;
                opts.set_never_capture(true); // disable capture support
                opts.set_max_mem(kMaxMemUsedBySingleRegexp); // limit memory that can be used by single regex
                opts.set_log_errors(false); // disable logging.
                it = m_regexpCache.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(cRegExp),
                    std::forward_as_tuple(cRegExp, opts)).first;
                if (!it->second.ok()) {
                    ctx.SetResultError(("regex:" + it->second.error()).c_str());
                    m_regexpCache.erase(it);
                    return;
                }
                // count size of memory used by new compiled regex
                m_memoryUsed += it->second.ProgramSize() + it->second.ReverseProgramSize();
            }

            //Perform partial match similar to LIKE op
            const auto isMatch = RE2::PartialMatch(cData, it->second);
            ctx.SetResultInt(isMatch ? 1 : 0);
        }
    public:
        explicit RegExpFunc() : ScalarFunction("REGEXP", 2, DbValueType::IntegerVal),m_memoryUsed(0){}
        ~RegExpFunc() {}
        static std::unique_ptr<ScalarFunction> Create() { return std::make_unique<RegExpFunc>();}
};

//=======================================================================================
// @bsiclass
// See https://github.com/google/re2/wiki/Syntax
// No need make this threadsafe. RE2 are already threadsafe and this function registered
// per db connection and cache is local to a single db connection.
//=======================================================================================
struct RegExpExtractFunc final : ScalarFunction {
    private:
        const int kMaxMemUsedBySingleRegexp = 1024*1024*10;
        const int kMaxMemUsedByCachedRegexp = 1024*1024*20;
        const int kMaxCachedSize = 100;

        std::unordered_map<std::string,RE2>  m_regexpCache;
        int m_memoryUsed;
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override {
            if (nArgs > 3 || nArgs < 2) {
                ctx.SetResultError("expecting two or three argument regexp_extract(D,R[,W])");
                return;
            }
            auto regexp = args[1];
            auto data = args[0];
            auto rewrite = nArgs == 3 ? &(args[2]) : nullptr;
            if (!regexp.IsValid() || !data.IsValid() || regexp.IsNull() || data.IsNull()) {
                return;
            }
            if (regexp.GetValueType () != DbValueType::TextVal) {
                ctx.SetResultError("string expression expected for R in regexp_extract(D,R,W)");
                return;
            }
            if (rewrite != nullptr && rewrite->GetValueType () != DbValueType::TextVal) {
                ctx.SetResultError("string expression expected for W in regexp_extract(D,R,W)");
                return;
            }
            if (data.GetValueType () != DbValueType::TextVal) {
                return;
            }
            auto cRegExp = regexp.GetValueText();
            auto cData = data.GetValueText();
            auto cReWrite = rewrite != nullptr ? rewrite->GetValueText() : R"(\0)"; // default capture.
            auto it = m_regexpCache.find(cRegExp);
            if (it == m_regexpCache.end()) {
                // Clear all cache if we exceeds any limit
                if (m_regexpCache.size() > kMaxCachedSize || m_memoryUsed > kMaxMemUsedByCachedRegexp) {
                    m_regexpCache.clear();
                    m_memoryUsed = 0;
                }
                RE2::Options opts;
                opts.set_max_mem(kMaxMemUsedBySingleRegexp); // limit memory that can be used by single regex
                opts.set_log_errors(false); // disable logging.
                it = m_regexpCache.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(cRegExp),
                    std::forward_as_tuple(cRegExp, opts)).first;
                if (!it->second.ok()) {
                    ctx.SetResultError(("regexp_extract:" + it->second.error()).c_str());
                    m_regexpCache.erase(it);
                    return;
                }
                // count size of memory used by new compiled regex
                m_memoryUsed += it->second.ProgramSize() + it->second.ReverseProgramSize();
            }

            //Perform partial match similar to LIKE op
            std::string out;
            if (RE2::Extract(cData, it->second, cReWrite, &out))
                ctx.SetResultText(out.c_str(), (int)out.length(), Context::CopyData::Yes);
        }
    public:
        explicit RegExpExtractFunc() : ScalarFunction("REGEXP_EXTRACT", -1, DbValueType::TextVal),m_memoryUsed(0){}
        ~RegExpExtractFunc() {}
        static std::unique_ptr<ScalarFunction> Create() { return std::make_unique<RegExpExtractFunc>();}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CachedPropertyKey
    {
    Utf8String  m_namespace;
    Utf8String  m_name;
    uint64_t    m_id;
    uint64_t    m_subId;

    void Init(Utf8CP nameSpace, Utf8CP name, uint64_t id, uint64_t subId)  {m_id = id;m_subId = subId;m_namespace.assign(nameSpace);m_name.assign(name);}
    CachedPropertyKey(){}
    CachedPropertyKey(Utf8CP nameSpace, Utf8CP name, uint64_t id, uint64_t subId) {Init(nameSpace, name, id, subId);}
    CachedPropertyKey(PropertySpecCR spec, uint64_t id, uint64_t subId) {Init(spec.GetNamespace(), spec.GetName(), id, subId);}
    bool operator< (CachedPropertyKey const& other) const
        {
        int val = m_namespace.CompareTo(other.m_namespace);
        if (val == 0)
            {
            val = m_name.CompareTo(other.m_name);
            if (val == 0)
                return (m_id != other.m_id) ? (m_id < other.m_id) : (m_subId < other.m_subId);
            }
        return val<0;
        }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CachedPropertyValue
    {
    bool          m_dirty;
    bool          m_compressed;
    Utf8String    m_strVal;
    bvector<Byte> m_value;

    CachedPropertyValue() {m_dirty=m_compressed=false;}
    void ChangeValue(Utf8CP strVal, uint32_t valSize, Byte const* value, bool dirty, bool compressed)
        {
        m_dirty = dirty;
        m_compressed = compressed;
        if (strVal)
            m_strVal.assign(strVal);
        if (value)
            m_value.assign(value, value+valSize);
        }
    uint32_t GetSize() const {return (uint32_t) m_value.size();}
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CachedPropertyMap : bmap<CachedPropertyKey, CachedPropertyValue>
    {
    bool Delete(PropertySpecCR spec, uint64_t id, uint64_t subId) {return 0 != erase(CachedPropertyKey(spec, id, subId));}
    CachedPropertyValue* Find(PropertySpecCR spec, uint64_t id, uint64_t subId)
        {
        iterator it = find(CachedPropertyKey(spec, id, subId));
        return (end() == it) ? nullptr : &it->second;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SQLiteTraceScope::SQLiteTraceScope(DbTrace categories, DbCR db, Utf8CP loggerName) {
    NativeLogging::CategoryLogger logger(loggerName);
    m_cancelCbList.push_back(db.GetTraceProfileEvent().AddListener([=](TraceContext const& ctx, int64_t nanoseconds) {
        logger.messagev(NativeLogging::LOG_DEBUG, "[PROF] %s (%lld ns)", ctx.GetExpandedSql().c_str(), nanoseconds);
    }));
    m_cancelCbList.push_back(db.GetTraceStmtEvent().AddListener([=](TraceContext const& ctx, Utf8CP sql) {
        logger.messagev(NativeLogging::LOG_DEBUG, "[STMT] %s",ctx.GetExpandedSql().c_str());
    }));
    m_cancelCbList.push_back(db.GetTraceRowEvent().AddListener([=](TraceContext const& ctx) {
        logger.messagev(NativeLogging::LOG_DEBUG, "[ROW] %s", ctx.GetExpandedSql().c_str());
    }));
    m_cancelCbList.push_back(db.GetTraceCloseEvent().AddListener([=](SqlDbP, Utf8CP fileName) {
        logger.messagev(NativeLogging::LOG_DEBUG, "[CLOSE] %s", fileName);
    }));
    db.ConfigTraceEvents(categories, true);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SQLiteTraceScope::~SQLiteTraceScope() {
    // remove callbacks
    for(auto& cb : m_cancelCbList) {
        cb();
    }
}

END_BENTLEY_SQLITE_NAMESPACE

/**
 * Used by operations that require there be no active transaction. If there is an active
 * default transaction, this class commits it in the ctor and restarts it in the dtor.
 */
struct SuspendDefaultTxn {
    Savepoint* m_savepoint = nullptr;
    SuspendDefaultTxn(Db& db) {
        if (db.GetCurrentSavepointDepth() > 1)
            return; // let operation fail

        m_savepoint = db.GetSavepoint(0);
        if (m_savepoint)
            m_savepoint->Commit();
    }
    ~SuspendDefaultTxn() {
        if (m_savepoint)
            m_savepoint->Begin();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeGuid::Create()
    {
    sqlite3_randomness(sizeof(BeGuid), m_guid.u);
    m_guid.b[6] = (m_guid.b[6] & 0x0f) | 0x40;   // see http://en.wikipedia.org/wiki/Universally_unique_identifier, use Version 4 (random)
    m_guid.b[8] = (m_guid.b[8] & 0x3f) | 0x80;   // set 2-bit flags to 0b10
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeGuid::ToString() const
    {
	char guidStr[40];
	snprintf(guidStr, sizeof(guidStr), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    m_guid.b[0], m_guid.b[1], m_guid.b[2], m_guid.b[3], m_guid.b[4], m_guid.b[5], m_guid.b[6], m_guid.b[7],
                    m_guid.b[8], m_guid.b[9], m_guid.b[10], m_guid.b[11], m_guid.b[12], m_guid.b[13], m_guid.b[14], m_guid.b[15]);

    return guidStr;
    }

/*---------------------------------------------------------------------------------**//**
* Copied from apr_guid.c
* convert a pair of hex digits to an integer value [0,255]
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static unsigned char parse_hexpair(Utf8CP s)
    {
    int result = s[0] - '0';
    if (result > 48)
        result = (result - 39) << 4;
    else if (result > 16)
        result = (result - 7) << 4;
    else
        result = result << 4;

    int temp = s[1] - '0';
    if (temp > 48)
        result |= temp - 39;
    else if (temp > 16)
        result |= temp - 7;
    else
        result |= temp;

    return (unsigned char)result;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from apr_guid.c
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeGuid::FromString(Utf8CP uuid_str)
    {
    Invalidate(); // clear existing value for error case

    if (uuid_str == nullptr)
        return ERROR;

    for (int i = 0; i < 36; ++i)
        {
        char c = uuid_str[i];
        if (c==0 || !isxdigit(c) && !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
            return ERROR;       /* ### need a better value */
        }

    if (uuid_str[36] != '\0')
        return ERROR; /* ### need a better value */

    m_guid.b[0] = parse_hexpair(&uuid_str[0]);
    m_guid.b[1] = parse_hexpair(&uuid_str[2]);
    m_guid.b[2] = parse_hexpair(&uuid_str[4]);
    m_guid.b[3] = parse_hexpair(&uuid_str[6]);
    m_guid.b[4] = parse_hexpair(&uuid_str[9]);
    m_guid.b[5] = parse_hexpair(&uuid_str[11]);
    m_guid.b[6] = parse_hexpair(&uuid_str[14]);
    m_guid.b[7] = parse_hexpair(&uuid_str[16]);
    m_guid.b[8] = parse_hexpair(&uuid_str[19]);
    m_guid.b[9] = parse_hexpair(&uuid_str[21]);

    for (int i = 6; i--;)
        m_guid.b[10 + i] = parse_hexpair(&uuid_str[i*2+24]);

    return SUCCESS;
    }

#ifdef NDEBUG
 #define LOG_STATEMENT_DIAGNOSTICS(sql, prepareStat, dbFile, suppressDiagnostics)
#else
 #define LOG_STATEMENT_DIAGNOSTICS(sql, prepareStat, dbFile, suppressDiagnostics) logStatementDiagnostics(sql, prepareStat, dbFile, suppressDiagnostics)

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BEGIN_UNNAMED_NAMESPACE
static bool s_diagIsEnabled = true;

NativeLogging::CategoryLogger s_diagnosticsPrepareLogger(DIAGNOSTICS_PREPARE_LOGGER_NAME);
NativeLogging::CategoryLogger s_diagnosticsQueryPlanLogger(DIAGNOSTICS_QUERYPLAN_LOGGER_NAME);
NativeLogging::CategoryLogger s_diagnosticsQueryPlanWithTableScanLogger(DIAGNOSTICS_QUERYPLANWITHTABLESCANS_LOGGER_NAME);

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static NativeLogging::CategoryLogger* getDiagnosticsPrepareLoggerIfEnabled()
    {
    if (s_diagnosticsPrepareLogger.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        return &s_diagnosticsPrepareLogger;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static NativeLogging::CategoryLogger* getDiagnosticsQueryPlanLoggerIfEnabled()
    {
    if (s_diagnosticsQueryPlanLogger.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        return &s_diagnosticsQueryPlanLogger;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static NativeLogging::CategoryLogger* getDiagnosticsQueryPlanWithTableScanLoggerIfEnabled()
    {
    if (s_diagnosticsQueryPlanWithTableScanLogger.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        return &s_diagnosticsQueryPlanWithTableScanLogger;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool excludeSqlFromExplainQuery(Utf8CP sql)
    {
    return BeStringUtilities::Strnicmp("explain", sql, 7) == 0 || BeStringUtilities::Strnicmp("pragma", sql, 6) == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void logStatementDiagnostics(Utf8CP sql, DbResult prepareStat, DbFileCR dbFile, bool suppressDiagnostics)
    {
    if (!s_diagIsEnabled || suppressDiagnostics)
        return;

    auto prepareLogger = getDiagnosticsPrepareLoggerIfEnabled();
    if (prepareLogger != nullptr)
        prepareLogger->message(NativeLogging::LOG_DEBUG, sql);

    //only do query plan diagnostics if preparation succeeded
    if (BE_SQLITE_OK == prepareStat)
        {
        auto queryPlanLogger = getDiagnosticsQueryPlanLoggerIfEnabled();
        auto queryPlanWithTableScansLogger = getDiagnosticsQueryPlanWithTableScanLoggerIfEnabled();
        if (queryPlanLogger == nullptr && queryPlanWithTableScansLogger == nullptr)
            return;

        if (!excludeSqlFromExplainQuery(sql))
            {
            Utf8String queryPlan = dbFile.ExplainQuery(sql, true, true);

            const size_t truncatedSqlSize = 150;
            Utf8String truncatedSql(sql);
            if (truncatedSql.size() > truncatedSqlSize)
                {
                truncatedSql = truncatedSql.substr(0, truncatedSqlSize - 4);
                truncatedSql.append(" ...");
                }

            if (queryPlanLogger != nullptr)
                queryPlanLogger->messagev(NativeLogging::LOG_DEBUG, "%s | %s", truncatedSql.c_str(), queryPlan.c_str());

            if (queryPlanWithTableScansLogger != nullptr && queryPlan.find("SCAN TABLE") != queryPlan.npos)
                {
                queryPlanWithTableScansLogger->messagev(NativeLogging::LOG_DEBUG, "%s | %s", truncatedSql.c_str(), queryPlan.c_str());
                }
            }
        }
    }

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StatementDiagnostics::LogComment(Utf8CP comment)
    {
    if (!s_diagIsEnabled || Utf8String::IsNullOrEmpty(comment))
        return;

    bvector<NativeLogging::CategoryLogger*> loggers;
    loggers.push_back(getDiagnosticsPrepareLoggerIfEnabled());
    loggers.push_back(getDiagnosticsQueryPlanLoggerIfEnabled());
    loggers.push_back(getDiagnosticsQueryPlanWithTableScanLoggerIfEnabled());

    for (auto logger : loggers)
        {
        if (logger != nullptr)
            logger->message(NativeLogging::LOG_DEBUG, comment);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementDiagnostics::SetIsEnabled(bool isEnabled) {s_diagIsEnabled=isEnabled;}

#endif

void        Statement::Finalize() {if (m_stmt) {sqlite3_finalize(m_stmt); m_stmt = nullptr;}}
DbResult    Statement::Step() {return m_stmt ? (DbResult) sqlite3_step(m_stmt) : BE_SQLITE_ERROR;}
DbResult    Statement::Reset() {return (DbResult)sqlite3_reset(m_stmt);}
DbResult    Statement::ClearBindings() {return m_stmt ? (DbResult)sqlite3_clear_bindings(m_stmt): BE_SQLITE_ERROR;}
DbResult    Statement::BindInt(int col, int val)        {return (DbResult)sqlite3_bind_int(m_stmt, col, val);}
DbResult    Statement::BindInt64(int col, int64_t val)  {return (DbResult)sqlite3_bind_int64(m_stmt, col, val);}
DbResult    Statement::BindDouble(int col, double val)  {return (DbResult)sqlite3_bind_double(m_stmt, col, val);}
DbResult    Statement::BindText(int col, Utf8CP val, MakeCopy makeCopy, int nBytes) {return (DbResult)sqlite3_bind_text(m_stmt, col, val, nBytes, makeCopy==MakeCopy::Yes ? SQLITE_TRANSIENT : SQLITE_STATIC);}
DbResult    Statement::BindZeroBlob(int col, int size) {
    return (size < 0) ?  BE_SQLITE_TOOBIG :  (DbResult)sqlite3_bind_zeroblob(m_stmt, col, size);
}
DbResult    Statement::BindBlob(int col, void const* val, int size, MakeCopy makeCopy) {return (DbResult)sqlite3_bind_blob(m_stmt, col, val, size, makeCopy==MakeCopy::Yes ? SQLITE_TRANSIENT : SQLITE_STATIC);}
DbResult    Statement::BindNull(int col) {return (DbResult)sqlite3_bind_null(m_stmt, col);}
DbResult    Statement::BindVirtualSet(int col, VirtualSet const& intSet) {return BindInt64(col, (int64_t) &intSet);}
DbResult    Statement::BindDbValue(int col, struct DbValue const& dbVal) {return (DbResult) sqlite3_bind_value(m_stmt, col, dbVal.GetSqlValueP());}

DbValueType Statement::GetColumnType(int col)   {return (DbValueType) sqlite3_column_type(m_stmt, col);}
Utf8CP      Statement::GetColumnDeclaredType(int col) { return sqlite3_column_decltype(m_stmt, col); }
Utf8CP      Statement::GetColumnTableName(int col) { return sqlite3_column_table_name(m_stmt, col); }
int         Statement::GetColumnCount()         {return sqlite3_column_count(m_stmt);}
int         Statement::GetColumnBytes(int col)  {return sqlite3_column_bytes(m_stmt, col);}
int         Statement::GetColumnBytes16(int col){return sqlite3_column_bytes16(m_stmt, col);}
Utf8CP      Statement::GetColumnName(int col)   {return sqlite3_column_name(m_stmt, col);}
void const* Statement::GetValueBlob(int col)    {return sqlite3_column_blob(m_stmt, col);}
Utf8CP      Statement::GetValueText(int col)    {return (Utf8CP) sqlite3_column_text(m_stmt, col);}
int         Statement::GetValueInt(int col)     {return sqlite3_column_int(m_stmt, col);}
int64_t     Statement::GetValueInt64(int col)   {return sqlite3_column_int64(m_stmt, col);}
double      Statement::GetValueDouble(int col)  {return sqlite3_column_double(m_stmt, col);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Statement::BindGuid(int col, BeGuidCR guid)
    {
    if (guid.IsValid())
        return (DbResult)sqlite3_bind_blob(m_stmt, col, &guid, sizeof(guid), SQLITE_TRANSIENT);

    return BindNull(col);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid Statement::GetValueGuid(int col)
    {
    if (IsColumnNull(col))
        return BeGuid();

    int columnBytes = GetColumnBytes(col);
    if (columnBytes != sizeof(BeGuid))
        return BeGuid();

    BeGuid guid;
    memcpy(&guid, GetValueBlob(col), sizeof(guid));
    return guid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbDupValue  Statement::GetDbValue(int col)
    {
    DbDupValue value(sqlite3_column_value(m_stmt, col));
    return value;
    }

int         Statement::GetParameterIndex(Utf8CP name) {return sqlite3_bind_parameter_index(m_stmt, name);}
int         Statement::GetParameterCount() { return sqlite3_bind_parameter_count(m_stmt); }
Utf8CP      Statement::GetSql() const                {return sqlite3_sql(m_stmt);}

DbValueType DbValue::GetValueType() const             {return (DbValueType) sqlite3_value_type(m_val);}
int         DbValue::GetValueBytes() const            {return sqlite3_value_bytes(m_val);}
void const* DbValue::GetValueBlob() const             {return sqlite3_value_blob(m_val);}
Utf8CP      DbValue::GetValueText() const             {return (Utf8CP)sqlite3_value_text(m_val);}
int         DbValue::GetValueInt() const              {return sqlite3_value_int(m_val);}
int64_t     DbValue::GetValueInt64() const            {return sqlite3_value_int64(m_val);}
double      DbValue::GetValueDouble() const           {return sqlite3_value_double(m_val);}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BusyRetry::_OnBusy(int count) const {
    if (count >= m_retries) {
        LOG.error("Busy retry failed, returning BE_SQLITE_BUSY error.");
        return 0;
    }

    LOG.infov("Busy retry %d",count);
    BeThreadUtilities::BeSleep(m_timeout);
    return 1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::TraceCallback(unsigned t, void* data, void* p, void* x) {
    auto db = reinterpret_cast<DbFile*>(data);
    if (!db) return 0;
    auto& traceEvents = db->GetTraceEvents();
    if (t == static_cast<int>(DbTrace::Profile)) {
        TraceContext ctx(reinterpret_cast<SqlStatementP>(p));
        auto elapsed = *(reinterpret_cast<int64_t*>(x));
        traceEvents.m_traceProfileEvent.RaiseEvent(ctx, elapsed);
    } else if (t == static_cast<int>(DbTrace::Row)) {
        TraceContext ctx(reinterpret_cast<SqlStatementP>(p));
        traceEvents.m_traceRowEvent.RaiseEvent(ctx);
    } else if (t == static_cast<int>(DbTrace::Stmt)) {
        TraceContext ctx(reinterpret_cast<SqlStatementP>(p));
        auto sql = reinterpret_cast<Utf8CP>(x);
        traceEvents.m_traceStmtEvent.RaiseEvent(ctx, sql);
    } else if (t == static_cast<int>(DbTrace::Close)) {
        auto sqlDb = reinterpret_cast<SqlDbP>(p);
        std::string fileName = sqlite3_db_filename(sqlDb, nullptr);
        traceEvents.m_traceCloseEvent.RaiseEvent(sqlDb, fileName.c_str());
    };
    return 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbFile::TraceEvents& DbFile::GetTraceEvents() const{
    if (m_traceEvents == nullptr) {
        m_traceEvents = std::make_unique<TraceEvents>();
        m_traceEvents->m_traceFlags = DbTrace::None;
    }
    return *m_traceEvents;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbTrace DbFile::ConfigTraceEvents(DbTrace flags, bool enable) const {
    auto& traceEvents = GetTraceEvents();
    const auto newVal = static_cast<int>(flags);
    const auto curVal = static_cast<int>(traceEvents.m_traceFlags);
    const auto traceFlags = static_cast<DbTrace>(enable ? (curVal | newVal) : (curVal & ~newVal));

    if (traceFlags == traceEvents.m_traceFlags) {
        return traceFlags;
    }

    traceEvents.m_traceFlags = traceFlags;

    if (traceEvents.m_traceFlags != DbTrace::None) {
        sqlite3_trace_v2(m_sqlDb, static_cast<unsigned int>(traceEvents.m_traceFlags), DbFile::TraceCallback, (void*)this);
    } else {
        DisableAllTraceEvents();
    }
    return traceEvents.m_traceFlags;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DisableAllTraceEvents() const {
    GetTraceEvents().m_traceFlags = DbTrace::None;
    sqlite3_trace_v2(m_sqlDb, 0, nullptr, nullptr);
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Statement::GetNormalizedSql() const
    {
    return sqlite3_normalized_sql(m_stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Statement::GetExpandedSql() const
    {
    char * sql = sqlite3_expanded_sql(m_stmt);
    Utf8String expendedSql = sql;
    sqlite3_free(sql);
    return expendedSql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP TraceContext::GetNormalizedSql() const
    {
    return sqlite3_normalized_sql(m_stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TraceContext::GetExpandedSql() const
    {
    char * sql = sqlite3_expanded_sql(m_stmt);
    Utf8String expendedSql = sql;
    sqlite3_free(sql);
    return expendedSql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP TraceContext::GetSql() const
    {
    return sqlite3_sql(m_stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TraceContext::IsReadonly() const
    {
    return 0 != sqlite3_stmt_readonly(m_stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid DbValue::GetValueGuid() const
    {
    int valueBytes = GetValueBytes();
    if (valueBytes != sizeof(BeGuid))
        return BeGuid();

    BeGuid guid;
    memcpy(&guid, GetValueBlob(), sizeof(guid));
    return guid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbDupValue::DbDupValue(SqlValueP val) : DbValue(nullptr) {m_val = sqlite3_value_dup(val);}
DbDupValue::~DbDupValue() {sqlite3_value_free(m_val);}
DbDupValue& DbDupValue::operator = (DbDupValue&& other)
    {
    if (this != &other)
        {
        if (m_val)
            sqlite3_value_free(m_val);
        m_val = other.m_val;
        other.m_val = nullptr;
        }
    return *this;
    }

TraceRowEvent& Db::GetTraceRowEvent() const { return m_dbFile->GetTraceEvents().m_traceRowEvent; }
TraceStmtEvent& Db::GetTraceStmtEvent() const{ return m_dbFile->GetTraceEvents().m_traceStmtEvent; }
TraceProfileEvent& Db::GetTraceProfileEvent() const { return m_dbFile->GetTraceEvents().m_traceProfileEvent; }
TraceCloseEvent& Db::GetTraceCloseEvent() const {return m_dbFile->GetTraceEvents().m_traceCloseEvent;}

SqlDbP   Db::GetSqlDb() const {return m_dbFile->m_sqlDb;}
BeGuid   Db::GetDbGuid() const {return m_dbFile->m_dbGuid;}
int32_t  Db::GetCurrentSavepointDepth() const {return (int32_t) m_dbFile->m_txns.size();}
Utf8String Db::GetLastError(DbResult* lastResult) const {return IsDbOpen() ? m_dbFile->GetLastError(lastResult) : "Not opened";}
void Db::Interrupt() const {return sqlite3_interrupt(GetSqlDb());}

int64_t  Db::GetLastInsertRowId() const {return sqlite3_last_insert_rowid(GetSqlDb());}
int      Db::GetModifiedRowCount() const {return sqlite3_changes(GetSqlDb());}
int      Db::GetTotalModifiedRowCount() const { return sqlite3_total_changes(GetSqlDb()); }
void     SnappyFromBlob::Finish() {m_blobIO.Close();}

Utf8String ProfileVersion::ToJson() const { return ToString("{\"major\":%" PRIu16 ",\"minor\":%" PRIu16 ",\"sub1\":%" PRIu16 ",\"sub2\":%" PRIu16 "}"); }
DbResult Db::FreeMemory() const { return (DbResult)sqlite3_db_release_memory(m_dbFile->m_sqlDb); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfileVersion::FromJson(Utf8CP val)
    {
    FromString(val, "{\"major\":%d,\"minor\":%d,\"sub1\":%d,\"sub2\":%d}");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Statement::TryPrepare(DbFileCR dbFile, Utf8CP sql)
    {
    DbResult stat = DoPrepare(dbFile, sql);
    LOG_STATEMENT_DIAGNOSTICS(sql, stat, dbFile, false);
    return stat;
    }

DbResult Statement::TryPrepare(DbCR db, Utf8CP sql) { return TryPrepare(*db.m_dbFile, sql);}
DbResult Statement::Prepare(DbCR db, Utf8CP sql) {return Prepare(*db.m_dbFile, sql);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Statement::Prepare(DbFileCR dbFile, Utf8CP sql, bool suppressDiagnostics)
    {
    DbResult stat = DoPrepare(dbFile, sql);
    if (stat != BE_SQLITE_OK)
        {
        Utf8String lastError = dbFile.GetLastError(nullptr); // keep on separate line for debugging
        LOG.errorv("Error \"%s\" preparing SQL: %s", lastError.c_str(), sql);
        }

    LOG_STATEMENT_DIAGNOSTICS(sql, stat, dbFile, suppressDiagnostics);
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Statement::DoPrepare(DbFileCR dbFile, Utf8CP sql)
    {
    SqlDbP dbHdl = dbFile.GetSqlDb();
    return (nullptr != m_stmt) ? BE_SQLITE_MISUSE : (DbResult) sqlite3_prepare_v2(dbHdl, sql, -1, &m_stmt, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Statement::IsReadonly() const
    {
    if (!IsPrepared())
        {
        BeAssert(IsPrepared() && "Can call Statement::IsReadonly only on prepared statements");
        return true; // pick true in error case, as an unprepared statement cannot change the DB either
        }

    return sqlite3_stmt_readonly(m_stmt) != 0;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Statement::DumpResults()
    {
    printf("Dumping SQL: %s\n", GetSql());

    bool firstTime = true;
    while (BE_SQLITE_ROW == Step())
        {
        if (firstTime)
            {
            firstTime = false;
            Utf8String header;
            for (int i = 0; i < GetColumnCount(); ++i)
                {
                if (i > 0)
                    header.append(", ");

                header.append(GetColumnName(i));
                }
            printf("%s\n", header.c_str());
            }

        Utf8String values;
        for (int i = 0; i < GetColumnCount(); ++i)
            {
            values.append(i > 0 ? ", " : "  ");
            auto text = GetValueText(i);
            values.append(nullptr != text ? text : "<NULL>");
            }
        printf("%s\n", values.c_str());
        }

    Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SqlPrintfString::SqlPrintfString(Utf8CP fmt, ...)
    {
    va_list vl;
    va_start(vl,fmt);
    m_str = sqlite3_vmprintf(fmt, vl);
    va_end(vl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SqlPrintfString::~SqlPrintfString() {sqlite3_free(m_str);}

#ifdef _MSC_VER
    #pragma warning(disable:4355)
#endif

//-------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static int besqliteBusyHandler(void* retry, int count) {return ((BusyRetry const*) retry)->_OnBusy(count);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbFile::DbFile(SqlDbP sqlDb, BusyRetry* retry, BeSQLiteTxnMode defaultTxnMode) : m_sqlDb(sqlDb), m_cachedProps(nullptr), m_blvCache(*this),
            m_defaultTxn(*this, "default", defaultTxnMode), m_statements(10), m_regexFunc(RegExpFunc::Create()),m_regexExtractFunc(RegExpExtractFunc::Create())
    {
    m_inCommit = false;
    m_allowImplicitTxns = false;
    m_dataVersion = 0;
    m_readonly = false;
    m_retry = retry ? retry : new BusyRetry();
    sqlite3_busy_handler(sqlDb, besqliteBusyHandler, m_retry.get());
    AddFunction(*m_regexFunc);
    AddFunction(*m_regexExtractFunc);
    }

BriefcaseLocalValueCache& DbFile::GetBLVCache() {return m_blvCache;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP getStartTxnSql(BeSQLiteTxnMode mode)
    {
    switch (mode)
        {
        case BeSQLiteTxnMode::Immediate: return "BEGIN IMMEDIATE";
        case BeSQLiteTxnMode::Exclusive: return "BEGIN EXCLUSIVE";
        }
    return  "BEGIN";
    }

static int savepointCommitHook(void* arg) {return ((DbFile*) arg)->OnCommit();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeSQLiteLib::s_throwExceptionOnUnexpectedAutoCommit = false;

/*---------------------------------------------------------------------------------**//**
* Ensure that all commits and rollbacks are done using BeSQLite api, not through SQL directly
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::OnCommit()
    {
    if (m_inCommit || m_txns.empty())
        return  0;

    // Sqlite initiate auto rollback in case of SQLITE_FULL, SQLITE_IOERR, SQLITE_NOMEM, SQLITE_BUSY, and SQLITE_INTERRUPT
    // We force a crash if COMMIT/ROLLBACK is called outside BeSQLite api.
    if (sqlite3_get_autocommit(m_sqlDb) != 0 ) {
        LOG.error("Sqlite initiated autocommit due to a fatal error.");
        if (BeSQLiteLib::s_throwExceptionOnUnexpectedAutoCommit) {
             LOG.error("Runtime debug option to throw exception on unexpected autocommit is set to *true*. Caller must handle exception and then delete the briefcase/db afterword.");
            throw std::runtime_error("sqlite initiated autocommit due to a fatal error");
        }
        return 0;
    }

    BeAssert(0);     // !!! USE BeSQLite API !!!
#ifdef __clang__
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wnull-dereference"
#endif

    *((int*)0) = 100; // force a crash - this must be illegal.

#ifdef __clang__
   #pragma clang diagnostic pop
#endif

    return  1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::StartSavepoint(Savepoint& txn, BeSQLiteTxnMode txnMode)
    {
    BeAssert(nullptr != m_sqlDb);

    if (txn.IsActive())
        return BE_SQLITE_ERROR;

    if (m_tracker.IsValid() && 0 < m_txns.size()) // if a change tracker is active, it is illegal to start a nested transaction
        {
        BeAssert(false);
        return BE_SQLITE_ERROR_ChangeTrackError;
        }

    // we need to save the cached properties/rlvs in case nested txn is cancelled
    if (m_txns.size() > 0)
        {
        SaveCachedProperties(true);
        SaveCachedBlvs(true);
        }

    void* old = sqlite3_commit_hook(m_sqlDb, savepointCommitHook, this);
    UNUSED_VARIABLE (old);
    BeAssert(old == nullptr || old == this);

    DbResult rc = (DbResult) sqlite3_exec(m_sqlDb,
               (0 == m_txns.size()) ? getStartTxnSql(txnMode) : SqlPrintfString("SAVEPOINT \"%s\"", txn.GetName()).GetUtf8CP(), nullptr, nullptr, nullptr);

    if (BE_SQLITE_OK != rc)
        {
        txn.m_depth = -1;
        return rc;
        }

    // If we're starting a new (non-default) transaction, make sure the file wasn't changed by another process. Note that this is only possible
    // with DefaultTxn_No, since with a default transaction BeSQLite::Db holds the lock on the file. The default txn has m_db=nullptr;
    if (txn.m_db && 0 == m_txns.size())
        {
        Statement stmt;
        stmt.TryPrepare(*txn.m_db, "PRAGMA data_version");
        rc = stmt.Step();
        BeAssert(BE_SQLITE_ROW == rc);

        uint64_t newDataVersion = stmt.GetValueInt64(0);
        if (0 != m_dataVersion && newDataVersion != m_dataVersion) // don't call this on the first Savepoint
            txn.m_db->_OnDbChangedByOtherConnection(); // let subclasses know they may need to flush caches, etc.

        m_dataVersion = newDataVersion;
        }

    m_txns.push_back(&txn);
    txn.m_depth = (int32_t) m_txns.size() - 1;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Savepoint*>::iterator FindSavepointPosition(bvector<Savepoint*>& txns, Savepoint& txn) {
    if (!txn.IsActive()) // not active? forget it.
        return txns.end();

    // make sure the txn is really active for this DbFile
    auto thisPos = txns.begin() + txn.GetDepth();
    if (*thisPos != &txn) {
        BeAssert(false);
        return txns.end();
    }

    return thisPos;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeactivateSavepoints(DbTxnIter pos, bool isCommit) {
    // deactivate this and all nested lower txns.
    for (DbTxnIter it = pos; it != m_txns.end(); ++it)
        (*it)->_OnDeactivate(isCommit);

    m_txns.erase(pos, m_txns.end());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::StopSavepoint(Savepoint& txn, bool isCommit, Utf8CP operation) {
    DbTxnIter thisPos = FindSavepointPosition(m_txns, txn);
    if (thisPos == m_txns.end())
        return BE_SQLITE_ERROR;

    SaveCachedProperties(isCommit);
    SaveCachedBlvs(isCommit);

    m_inCommit = true;

    // Don't check m_tracker->HasChanges - may have dynamic changes to rollback
    ChangeTracker::OnCommitStatus trackerStatus = (m_tracker.IsValid()) ? m_tracker->_OnCommit(isCommit, operation) : ChangeTracker::OnCommitStatus::Commit;
    if (trackerStatus == ChangeTracker::OnCommitStatus::Abort) {
        // Abort is considered fatal and application must quit.
        // We do not allocate memory or attempt to log as this is only happens when sqlite returns NOMEM.
        m_inCommit = false;
        if (m_tracker.IsValid()) {
            m_tracker->EndTracking();
            m_tracker = nullptr;
        }
        if (0 != m_txns.size()) {
            m_txns[0]->Cancel();
            m_txns.clear();
        }
        throw std::runtime_error("commit error");
    }

    // save briefcase local values again after _OnCommit. It can cause additional changes
    SaveCachedBlvs(isCommit);

    // attempt the commit/release or rollback
    DbResult rc = BE_SQLITE_OK;
    if (trackerStatus == ChangeTracker::OnCommitStatus::Commit || trackerStatus == ChangeTracker::OnCommitStatus::NoChanges) {
        if (0 == txn.GetDepth()) {
            rc = (DbResult)sqlite3_exec(m_sqlDb, isCommit ? "COMMIT" : "ROLLBACK", nullptr, nullptr, nullptr);
        } else {
            Utf8String sql;
            if (!isCommit) // to cancel a nested transaction, we need to roll it back and then release it.
                sql.append(SqlPrintfString("ROLLBACK TO \"%s\";", txn.GetName()));

            sql.append(SqlPrintfString("RELEASE \"%s\"", txn.GetName()));
            rc = (DbResult)sqlite3_exec(m_sqlDb, sql.c_str(), nullptr, nullptr, nullptr);
        }
    }

    m_inCommit = false;

    // BE_SQLITE_INTERRUPT means the transaction was cancelled. Anything other than BE_SQLITE_OK means the transaction is still open.
    if (rc != BE_SQLITE_OK && rc != BE_SQLITE_INTERRUPT) {
        BeAssert(BE_SQLITE_OK == checkNoActiveStatements(m_sqlDb)); // a common problem is to try to commit while statements are active. Help find that error.
        return rc;
    }

    if (rc == BE_SQLITE_INTERRUPT) {
        if (isCommit)
            isCommit = false; // we tried to commit this savepoint, but it was rollled back externally
        else
            rc = BE_SQLITE_OK; // we were trying to cancel anyway. This is not an error.
    }

    DeactivateSavepoints(thisPos, isCommit);

    if (m_tracker.IsValid() && trackerStatus == ChangeTracker::OnCommitStatus::Commit) // notify trackers that commit/cancel operation finished
        m_tracker->_OnCommitted(isCommit, operation);

    return rc; // either BE_SQLITE_OK or BE_SQLITE_INTERRUPT
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeactivateSavepoint(Savepoint& txn) {
    auto pos = FindSavepointPosition(m_txns, txn);
    if (pos != m_txns.end())
        DeactivateSavepoints(pos, false);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbFile::ExplainQuery(Utf8CP sql, bool explainPlan, bool suppressDiagnostics) const
    {
    Statement queryPlan;
    if (BE_SQLITE_OK != queryPlan.Prepare(*this, Utf8PrintfString("EXPLAIN %s %s", explainPlan ? "QUERY PLAN" : "", sql).c_str(), suppressDiagnostics))
        return GetLastError(nullptr);

    Utf8CP fmt = explainPlan ? "%s %s %s %s %s %s %s %s" : "%-3s %-12s %-4s %-4s %s %s %s %s\n";
    Utf8String plan;
    bool isFirstRow = true;
    while (BE_SQLITE_ROW == queryPlan.Step())
        {
        if (!isFirstRow)
            plan.append(";");

        plan.append(Utf8PrintfString(fmt,
                                     queryPlan.GetValueText(0), queryPlan.GetValueText(1), queryPlan.GetValueText(2), queryPlan.GetValueText(3),
                                     queryPlan.GetValueText(4), queryPlan.GetValueText(5), queryPlan.GetValueText(6), queryPlan.GetValueText(7)
                                    ));
        isFirstRow = false;
        }
    return plan;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Db::Db() : m_embeddedFiles(*this), m_dbFile(nullptr), m_statements(35){}
Db::~Db() {DoCloseDb();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Savepoint::Savepoint(DbR db, Utf8CP name, bool beginTxn, BeSQLiteTxnMode txnMode) : m_dbFile(db.m_dbFile), m_name(name), m_txnMode(txnMode)
    {
    m_db = &db;
    m_depth = -1;

    if (beginTxn)
        Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Savepoint::~Savepoint() {
    DbResult rc = Commit(nullptr);
    if (rc != BE_SQLITE_OK && rc != BE_SQLITE_INTERRUPT && m_dbFile) {
        // on error, StopSavepoint doesn't deactivate and/or remove the savepoint from m_txns, but
        // we need to make that happen since the savepoint is getting destroyed
        m_dbFile->DeactivateSavepoint(*this);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Savepoint::_Begin(BeSQLiteTxnMode mode)  {return m_dbFile ? m_dbFile->StartSavepoint(*this, mode) : BE_SQLITE_ERROR;}
DbResult Savepoint::_Cancel() {return m_dbFile ? m_dbFile->StopSavepoint(*this, false, nullptr) : BE_SQLITE_ERROR;}
DbResult Savepoint::_Commit(Utf8CP operation) {return m_dbFile ? m_dbFile->StopSavepoint(*this, true, operation) : BE_SQLITE_ERROR;}
DbResult Savepoint::Begin(BeSQLiteTxnMode mode)  {return _Begin(mode);}
DbResult Savepoint::Commit(Utf8CP operation) {return _Commit(operation);}
DbResult Savepoint::Save(Utf8CP operation)
    {
    DbResult res = Commit(operation);
    if (BE_SQLITE_BUSY == res) {
        return res;
    }
    return Begin();
    }
DbResult Savepoint::Cancel() {return _Cancel();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Savepoint* Db::GetSavepoint(int32_t depth) const
    {
    return (depth<0 || depth>=GetCurrentSavepointDepth()) ? nullptr : m_dbFile->m_txns[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ExecuteDdl(Utf8CP ddl) const
    {
    DbResult result = ExecuteSql(ddl);
    if (result != BE_SQLITE_OK)
        {
        BeAssert(false);
        return result;
        }

    if (ChangeTracker* tracker = m_dbFile->m_tracker.get())
        result = tracker->RecordDbSchemaChange(ddl);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ExecuteSql(Utf8CP sql, int (*callback)(void*,int,CharP*,CharP*),void* arg,CharP* errmsg) const
    {
    if (!m_dbFile->CheckImplicitTxn())
        {
        BeAssert(false);
        return  BE_SQLITE_ERROR_NoTxnActive;
        }

    DbResult rc = (DbResult) sqlite3_exec(GetSqlDb(), sql, callback, arg, errmsg);
    if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
        {
        Utf8String lastError = GetLastError(); // keep on separate line for debugging
        LOG.errorv("Error \"%s\" SQL: %s", lastError.c_str(), sql);

        if (BE_SQLITE_LOCKED == rc)
            {
            sqlite3_stmt* stmt = nullptr;
            while (nullptr != (stmt = sqlite3_next_stmt(m_dbFile->m_sqlDb, stmt)))
                {
                if (sqlite3_stmt_busy(stmt))
                    {
                    Utf8String openStatement(sqlite3_sql(stmt));
                    if (openStatement.empty())
                        LOG.errorv("Transaction is active.");
                    else
                        LOG.errorv("Busy: '%s'", openStatement.c_str());
                    }
                }
            }

        BeAssert(false);  // If you EXPECT failures to be non-exceptional, call TryExecuteSql
        }
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::TryExecuteSql(Utf8CP sql, int (*callback)(void*,int,CharP*,CharP*),void* arg,CharP* errmsg) const
    {
    return (DbResult) sqlite3_exec(GetSqlDb(), sql, callback, arg, errmsg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Db::ExplainQuery(Utf8CP sql, bool explainPlan) const {return m_dbFile->ExplainQuery(sql, explainPlan, false);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::CreatePropertyTable(Utf8CP tablename, Utf8CP ddl)
    {
    return (DbResult) sqlite3_exec(m_sqlDb, SqlPrintfString("CREATE TABLE %s (%s)",tablename, ddl), nullptr, nullptr, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedPropertyMap& DbFile::GetCachedPropMap() const
    {
    if (nullptr == m_cachedProps)
        m_cachedProps = new CachedPropertyMap();

    CachedPropertyMap& map =*((CachedPropertyMap*) m_cachedProps);
    return  map;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedPropertyValue& DbFile::GetCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    return GetCachedPropMap()[CachedPropertyKey(spec.GetNamespace(), spec.GetName(), id, subId)];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedPropertyValue* DbFile::FindCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    return GetCachedPropMap().Find(spec, id, subId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeleteCachedProperty(PropertySpecCR spec, uint64_t id, uint64_t subId)
    {
    if (nullptr != m_cachedProps)
        GetCachedPropMap().erase(CachedPropertyKey(spec.GetNamespace(), spec.GetName(), id, subId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SaveCachedProperties(bool isCommit)
    {
    if (nullptr == m_cachedProps)
        return;

    if (!isCommit)
        {
        DeleteCachedPropertyMap(); // only delete cached property map on cancel, not commit.
        return;
        }

    CachedPropertyMap& map = GetCachedPropMap();
    for (CachedPropertyMap::iterator it=map.begin(); it!=map.end(); ++it)
        {
        CachedPropertyKey const& key = it->first;
        CachedPropertyValue& val = it->second;
        if (val.m_dirty)
            {
            PropertySpec spec(key.m_name.c_str(), key.m_namespace.c_str(), PropertySpec::Mode::Normal, val.m_compressed ? PropertySpec::Compress::Yes : PropertySpec::Compress::No);
            SaveProperty(spec, val.m_strVal.length()>0 ? val.m_strVal.c_str() : nullptr, val.m_value.size()>0 ? val.m_value.data() : nullptr, (uint32_t) val.m_value.size(),
                          key.m_id, key.m_subId);
            val.m_dirty=false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::SaveCachedBlvs(bool isCommit)
    {
    if (!isCommit)
        {
        m_blvCache.Clear();
        return; // only clear cached RLVs on cancel, not commit
        }

    CachedStatementPtr stmt;
    for (CachedBLV const& rlv : m_blvCache.m_cache)
        {
        if (!rlv.IsUnset() && rlv.IsDirty())
            {
            if (!stmt.IsValid())
                {
                DbResult rc = m_statements.GetPreparedStatement(stmt, *this, "INSERT OR REPLACE INTO " BEDB_TABLE_Local " (Name,Val) VALUES(?,?)");
                BeAssert(rc == BE_SQLITE_OK);
                UNUSED_VARIABLE(rc);
                }

            stmt->BindText(1, rlv.GetName(), Statement::MakeCopy::No);
            const uint64_t val = rlv.GetValue();
            stmt->BindUInt64(2, val);
            DbResult rc = stmt->Step();
            BeAssert(BE_SQLITE_DONE == rc);
            UNUSED_VARIABLE(rc);

            stmt->Reset();
            rlv.SetIsNotDirty();
            }
        }
    }

#define PROPERTY_TABLE_DDL "Namespace CHAR NOT NULL,Name CHAR NOT NULL,Id INT NOT NULL,SubId INT NOT NULL,TxnMode Int NOT NULL,StrData CHAR,RawSize INT,Data BLOB,PRIMARY KEY(Namespace,Name,Id,SubId)"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::SaveProperty(PropertySpecCR spec, Utf8CP stringData, void const* value, uint32_t size, uint64_t id, uint64_t subId)
    {
    if (nullptr == value && nullptr == stringData && !spec.SaveIfNull())
        {
        DeleteProperty(spec, id);
        return  BE_SQLITE_OK;
        }

    if (spec.IsCached())
        {
        GetCachedProperty(spec, id, subId).ChangeValue(stringData, size, const_cast<Byte*>((Byte const*)value), true, spec.IsCompress());
        return  BE_SQLITE_OK;
        }

    DbResult rc;
    CachedStatementPtr stmt;
    rc = m_statements.GetPreparedStatement(stmt, *this, "INSERT OR REPLACE INTO " BEDB_TABLE_Property " (Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData) VALUES(?,?,?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    stmt->BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt->BindText(2, spec.GetName(), Statement::MakeCopy::No);
    stmt->BindInt64(3, id);
    stmt->BindInt64(4, subId);
    stmt->BindInt(5, 0);
    stmt->BindText(8, stringData, Statement::MakeCopy::No);

    bool doCompress = spec.IsCompress();
    bvector<Byte> compressed; // use bvector just so destructor will free make sure this is outside if!
    if (nullptr != value)
        {
        if (size <= 100) // too small to be worth trying
            doCompress = false;

        if (doCompress)
            {
            unsigned long compressedSize= (uint32_t) (size*1.01) + 12;
            compressed.resize(compressedSize);
            if (Z_OK != compress2(compressed.data(), &compressedSize, (Byte const*) value, size, DefaultCompressionLevel) || (compressedSize >= size))
                doCompress = false;
            else
                {
                stmt->BindInt(6, size);     // this is the uncompressed size, when compressed
                stmt->BindBlob(7, compressed.data(), compressedSize, Statement::MakeCopy::No);
                }
            }

        if (!doCompress) // dont use "else" here, value of compress can change if zip fails!
            {
            stmt->BindNull(6);
            stmt->BindBlob(7, value, size, Statement::MakeCopy::No);
            }
        }

    rc = stmt->Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryPropertySize(uint32_t& size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        {
        HasProperty(spec, id, subId); // make sure its cached

        CachedPropertyValue* cachedProp = FindCachedProperty(spec, id, subId);
        if (nullptr == cachedProp)
            return BE_SQLITE_ERROR;

        size = cachedProp->GetSize();
        return  BE_SQLITE_ROW;
        }

    Statement stmt;
    DbResult rc = stmt.Prepare(*this, "SELECT RawSize,length(Data) FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?");
    stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
    stmt.BindInt64(3, id);
    stmt.BindInt64(4, subId);
    rc = stmt.Step();

    if (rc != BE_SQLITE_ROW)
        {
        size = 0;
        return BE_SQLITE_ERROR;
        }

    size = stmt.GetValueInt(0);
    if (0 == size)
        size = stmt.GetValueInt(1);

    return  BE_SQLITE_ROW;
    }

#define FROM_PROPERTY_TABLE_SQL " FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=? AND SubId=?"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryCachedProperty(Utf8String* strval, void** value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    BeAssert(spec.IsCached());
    CachedPropertyValue* cachedProp = FindCachedProperty(spec, id, subId);
    if (nullptr == cachedProp)
        {
        CachedStatementPtr stmt;
        DbResult rc = m_statements.GetPreparedStatement(stmt, *this, "SELECT RawSize,Data,StrData" FROM_PROPERTY_TABLE_SQL);
        if (BE_SQLITE_OK == rc)
            {
            stmt->BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
            stmt->BindText(2, spec.GetName(), Statement::MakeCopy::No);
            stmt->BindInt64(3, id);
            stmt->BindInt64(4, subId);
            rc = stmt->Step();
            }

        if (rc != BE_SQLITE_ROW)
            return BE_SQLITE_ERROR;

        cachedProp = &GetCachedProperty(spec, id, subId);

        Utf8CP strVal = stmt->GetValueText(2);
        if (strVal)
            cachedProp->m_strVal = strVal;

        uint32_t compressedBytes = stmt->GetValueInt(0);
        uint32_t blobsize = stmt->GetColumnBytes(1);

        uint32_t bytes = (0==compressedBytes) ? blobsize : compressedBytes;

        cachedProp->m_value.resize(bytes);

        void const* blobdata = stmt->GetValueBlob(1);

        if (compressedBytes > 0)
            {
            unsigned long actuallyRead = size;
            uncompress((Byte*)cachedProp->m_value.data(), &actuallyRead, (Byte const*) blobdata, blobsize);
            if (actuallyRead != size)
                return BE_SQLITE_MISMATCH;
            }
        else
            {
            if (blobsize < size)
                return BE_SQLITE_MISMATCH;

            if (!cachedProp->m_value.empty())
                memcpy((Byte*)cachedProp->m_value.data(), blobdata, bytes);
            }
        }

    if (strval)
        *strval = cachedProp->m_strVal;

    if (size>cachedProp->GetSize())
        size = cachedProp->GetSize();

    if (value && !cachedProp->m_value.empty())
        memcpy(*value, cachedProp->m_value.data(), size);

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryProperty(void* value, uint32_t size, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return QueryCachedProperty(nullptr, &value, size, spec, id, subId);

    CachedStatementPtr stmt;
    DbResult rc = m_statements.GetPreparedStatement(stmt, *this, "SELECT RawSize,Data" FROM_PROPERTY_TABLE_SQL);
    if (BE_SQLITE_OK == rc)
        {
        stmt->BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
        stmt->BindText(2, spec.GetName(), Statement::MakeCopy::No);
        stmt->BindInt64(3, id);
        stmt->BindInt64(4, subId);
        rc = stmt->Step();
        }

    if (rc != BE_SQLITE_ROW)
        {
        stmt = nullptr; // we have to release this stmt, so it will get reset on recursive call
        return BE_SQLITE_ERROR;
        }

    uint32_t compressedBytes = stmt->GetValueInt(0);
    uint32_t blobsize = stmt->GetColumnBytes(1);

    uint32_t bytes = (0==compressedBytes) ? blobsize : compressedBytes;
    if (size>bytes)
        size=bytes;

    void const* blobdata = stmt->GetValueBlob(1);

    if (compressedBytes > 0)
        {
        unsigned long actuallyRead = size;
        uncompress((Byte*)value, &actuallyRead, (Byte const*) blobdata, blobsize);
        if (actuallyRead != size)
            return BE_SQLITE_MISMATCH;
        }
    else
        {
        if (blobsize < size)
            return BE_SQLITE_MISMATCH;

        memcpy(value, blobdata, size);
        }

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::QueryProperty(Utf8StringR value, PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return QueryCachedProperty(&value, nullptr, 0, spec, id, subId);

    CachedStatementPtr stmt;
    DbResult rc = m_statements.GetPreparedStatement(stmt, *this, "SELECT StrData" FROM_PROPERTY_TABLE_SQL);
    if (BE_SQLITE_OK == rc)
        {
        stmt->BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
        stmt->BindText(2, spec.GetName(), Statement::MakeCopy::No);
        stmt->BindInt64(3, id);
        stmt->BindInt64(4, subId);
        rc = stmt->Step();
        }

    if (rc != BE_SQLITE_ROW)
        {
        stmt = nullptr; // we have to release this stmt, so it will get reset on recursive call
        value.clear();
        return  BE_SQLITE_ERROR;
        }

    value.AssignOrClear(stmt->GetValueText(0));
    return  BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbFile::HasProperty(PropertySpecCR spec, uint64_t id, uint64_t subId) const
    {
    if (spec.IsCached())
        return BE_SQLITE_ROW==QueryCachedProperty(nullptr, nullptr, 0, spec, id, subId);

    Statement stmt;
    DbResult rc = stmt.Prepare(*this, "SELECT 1" FROM_PROPERTY_TABLE_SQL);
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
        stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
        stmt.BindInt64(3, id);
        stmt.BindInt64(4, subId);
        rc = stmt.Step();
        }

    if (rc == BE_SQLITE_ROW)
        return  true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::DeleteProperty(PropertySpecCR spec, uint64_t id, uint64_t subId)
    {
    if (spec.IsCached())
        DeleteCachedProperty(spec, id, subId);

    Statement stmt;
    DbResult rc = stmt.Prepare(*this, "DELETE" FROM_PROPERTY_TABLE_SQL);
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
        stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
        stmt.BindInt64(3, id);
        stmt.BindInt64(4, subId);
        rc = stmt.Step();
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbFile::DeleteProperties(PropertySpecCR spec, uint64_t* id)
    {
    Utf8String sql("DELETE FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=? ");
    if (id)
        sql.append("AND Id=?");

    Statement stmt;
    DbResult rc = stmt.Prepare(*this, sql.c_str());
    if (BE_SQLITE_OK == rc)
        {
        stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
        stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
        if (id)
            stmt.BindInt64(3, *id);

        rc = stmt.Step();
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveBeDbGuid()
    {
    if (!m_dbFile->m_dbGuid.IsValid())
        m_dbFile->m_dbGuid.Create();

    DbResult rc = SaveProperty(Properties::DbGuid(), (void*) &m_dbFile->m_dbGuid, sizeof(m_dbFile->m_dbGuid));
    if (BE_SQLITE_OK != rc)
        {
        LOG.warningv("Could not save GUID: %s", BeSQLiteLib::GetErrorName(rc));
        BeAssert(false);
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveBriefcaseId()
    {
    if (!m_dbFile->m_briefcaseId.IsValid())
        {
        BeAssert(false && "Invalid BeBriefcaseId");
        return BE_SQLITE_ERROR;
        }

    Statement stmt;
    DbResult stat = stmt.Prepare(*this, "INSERT OR REPLACE INTO " BEDB_TABLE_Local "(Name,Val) VALUES('" BEDB_BRIEFCASELOCALVALUE_BriefcaseId "',?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    int64_t briefcaseId = m_dbFile->m_briefcaseId.GetValue();
    stmt.BindBlob(1, &briefcaseId, (int) sizeof(briefcaseId), Statement::MakeCopy::No);
    stat = stmt.Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ChangeDbGuid(BeGuid guid) {
    if (IsReadonly()) {
        BeAssert(false && "must be a writeable db");
        return BE_SQLITE_READONLY;
    }
    if (!guid.IsValid())
        guid.Create();

    _OnDbGuidChange(guid);

    m_dbFile->m_dbGuid = guid;
    return SaveBeDbGuid();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ResetBriefcaseId(BeBriefcaseId id) {
    // perform this even if id is already correct. May be changing txn state
    if (IsReadonly()) {
        BeAssert(false && "must be a writeable db");
        return BE_SQLITE_READONLY;
    }
    if (!id.IsValid()) {
        BeAssert(false && "Id must be valid");
        return BE_SQLITE_ERROR;
    }

    SaveChanges(); // make sure there are no pending unsaved changes

    _OnBeforeSetBriefcaseId(id);

    // NOTE: we used to clear the whole table here. That's not necessary and a bad idea
    m_dbFile->m_briefcaseId = id;
    SaveBriefcaseId();

    _OnAfterSetBriefcaseId();
    return SaveChanges();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeBriefcaseBasedId::BeBriefcaseBasedId(Db& db, Utf8CP tableName, Utf8CP colName)
    {
    SqlPrintfString sql("SELECT max(%s) FROM %s WHERE %s>=? AND %s<?", colName, tableName, colName, colName);

    Statement stmt;
    stmt.Prepare(db, sql);

    BeBriefcaseId myRepo = db.GetBriefcaseId();
    BeBriefcaseBasedId firstId(myRepo, 1);
    BeBriefcaseBasedId lastId(myRepo.GetNextBriefcaseId(), 0);

    stmt.BindInt64(1, firstId.GetValueUnchecked());
    stmt.BindInt64(2, lastId.GetValueUnchecked());
    stmt.Step();
    m_id = stmt.IsColumnNull(0) ? firstId.GetValueUnchecked() : stmt.GetValueInt64(0)+1;
    BeAssert(m_id < lastId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeBriefcaseBasedId::UseNext(Db& db)
    {
    if (!IsValid())
        return;

    BeBriefcaseId myRepo = db.GetBriefcaseId();
    BeAssert(myRepo == GetBriefcaseId());
    ++m_id;

    BeBriefcaseBasedId lastId(myRepo.GetNextBriefcaseId(), 0);
    if (m_id < lastId.GetValueUnchecked())
        return; // ok

    BeAssert(false); // We're out of ids for this briefcaseid!
    Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::GetServerIssuedId(BeServerIssuedId& value, Utf8CP tableName, Utf8CP colName, Utf8CP json)
    {
    if (value.IsValid())
        value = BeServerIssuedId(value.GetValue() + 1);
    else
        {
        Utf8String sql(SqlPrintfString("SELECT max(%s) FROM %s", colName, tableName, colName));

        Statement stmt;
        stmt.Prepare(*this, sql.c_str());

        DbResult result = stmt.Step();
        BeAssert(result == BE_SQLITE_ROW);
        if (result != BE_SQLITE_ROW)
            return  result;

        value = BeServerIssuedId(stmt.GetValueUInt64(0) + 1);
        }

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::AddFunction(DbFunction& func) const
    {
    int stat = m_dbFile->AddFunction(func);
    return (stat != 0) ? stat : _OnAddFunction(func);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::AddRTreeMatchFunction(RTreeMatchFunction& func) const
    {
    int stat =m_dbFile->AddRTreeMatchFunction(func);
    return (stat != 0) ? stat : _OnAddFunction(func);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::RemoveFunction(DbFunction& func) const
    {
    _OnRemoveFunction(func);
    return m_dbFile->RemoveFunction(func);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Db::AddDataUpdateCallback(DataUpdateCallback& callback) const { m_dbFile->AddDataUpdateCallback(callback); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Db::RemoveDataUpdateCallback() const { m_dbFile->RemoveDataUpdateCallback();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::SaveProjectGuid(BeGuid projectGuid)
    {
    // Note: allow invalid guids - that's how you make a "standalone" iModel.
    SaveProperty(Properties::ProjectGuid(), (void*) &projectGuid, sizeof(projectGuid));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid Db::QueryProjectGuid() const
    {
    BeGuid projectGuid(false);
    QueryProperty(&projectGuid, sizeof(projectGuid), Properties::ProjectGuid());
    return  projectGuid;
    }

/** get the uri parameter (or just the dbName) to open a database */
static Utf8String getDbUri(Utf8CP dbName, Db::OpenParams const& params) {
    if (params.m_queryParams.empty())
        return dbName;

    Utf8String uri = "file:";
    Utf8String dbUri(dbName);

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    if (dbUri.StartsWith("\\\\?\\")) // uri open doesn't work correctly with long path prefix on Windows
        dbUri = dbUri.substr(4);
#endif

    uri += dbUri;
    uri += "?";
    bool first = true;
    for (auto const& param : params.m_queryParams) {
        if (!first)
            uri.append("&");
        first = false;
        uri.append(param);
    }
    return uri;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisableBloomFilter(SqlDbP db) {
    const int SQLITE_BloomFilter =  0x00080000;
    sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS, db, SQLITE_BloomFilter);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateNewDb(Utf8CP inName, CreateParams const& params, BeGuid dbGuid) {
    if (IsDbOpen())
        return BE_SQLITE_ERROR_AlreadyOpen;

    Utf8String dbName = getDbUri(inName, params);

    OpenMode openMode = OpenMode::Create;
    if (!dbName.Equals(BEDB_MemoryDb) && !params.m_skipFileCheck) {
        BeFileName dbFileName(dbName);
        if (dbFileName.DoesPathExist()) {
            if (params.m_failIfDbExists)
                return BE_SQLITE_ERROR_FileExists;

            openMode = OpenMode::ReadWrite;
        }
    }

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    static const size_t JOURNAL_SUFFIX_LENGTH = 8; // "-journal"
    if ((dbName.length() + 1 + JOURNAL_SUFFIX_LENGTH) > MAX_PATH)
        return BE_SQLITE_CANTOPEN;
#endif

    SqlDbP sqlDb;
    DbResult rc = (DbResult)sqlite3_open_v2(dbName.c_str(), &sqlDb, (int)openMode, nullptr);
    if (BE_SQLITE_OK != rc) {
        sqlite3_close_v2(sqlDb);
        return rc;
    }

    // Bloom filter introduce in 3.38 has issue with certain queries and fix was in ANALYZE
    // command which had a rounding error. We need to keep bloom filter disabled for now until
    // we are able to run ANALYZE on all new checkpoints and old one if necessary.
    DisableBloomFilter(sqlDb);

    sqlite3_extended_result_codes(sqlDb, 1); // turn on extended error codes
    m_dbFile = new DbFile(sqlDb, params.m_busyRetry, (BeSQLiteTxnMode)params.m_startDefaultTxn);
    m_isCloudDb = params.m_fromContainer;

    m_dbFile->m_defaultTxn.Begin();
    m_dbFile->m_dbGuid = dbGuid;
    m_dbFile->m_briefcaseId = BeBriefcaseId(BeBriefcaseId::Standalone());
    ExecuteSql(SqlPrintfString("PRAGMA page_size=%d;PRAGMA encoding=\"%s\";PRAGMA user_version=%d;PRAGMA application_id=%lld;PRAGMA locking_mode=\"%s\";", params.m_pagesize,
                               params.m_encoding == Encoding::Utf8 ? "UTF-8" : "UTF-16le", BeSQLite::DbUserVersion, params.m_applicationId,
                               params.m_startDefaultTxn == DefaultTxn::Exclusive ? "EXCLUSIVE" : "NORMAL"));

    if (!params.m_rawSQLite) {
        rc = m_dbFile->CreatePropertyTable(BEDB_TABLE_Property, PROPERTY_TABLE_DDL);
        if (BE_SQLITE_OK != rc)
            return rc;

        // NOTE: this table purposely has no primary key so it won't be tracked / merged. It is meant to hold values that are
        // local to the briefcase and never in a changeset.
        rc = CreateTable(BEDB_TABLE_Local, "Name CHAR NOT NULL COLLATE NOCASE UNIQUE,Val BLOB");
        if (BE_SQLITE_OK != rc)
            return rc;

        rc = SaveBeDbGuid();
        if (BE_SQLITE_OK != rc)
            return rc;

        rc = SaveBriefcaseId();
        if (BE_SQLITE_OK != rc)
            return rc;

        rc = EmbeddedFiles().CreateTable();
        if (rc != BE_SQLITE_OK)
            return rc;

        if (params.m_expirationDate.IsValid())
            SaveExpirationDate(params.m_expirationDate);

        rc = BeSQLiteProfileManager::AssignProfileVersion(*this);
        if (rc != BE_SQLITE_OK)
            return rc;

        // create sqlite_stat1 so it can be tracked later on.
        rc = ExecuteSql("analyze;");
        if (BE_SQLITE_OK != rc)
            return rc;

        // make sure its empty we just want sqlite to add the tables
        rc = ExecuteSql("delete from sqlite_stat1;");
        if (BE_SQLITE_OK != rc)
            return rc;
    }

    rc = _OnDbCreated(params);
    if (BE_SQLITE_OK != rc)
        return rc;

    if (DefaultTxn::No == params.m_startDefaultTxn)
        m_dbFile->m_defaultTxn.Commit(nullptr);

    LOG.infov("Created file '%s'", GetDbFileName());
    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AttachDb(Utf8CP filename, Utf8CP alias) const {
    Savepoint* txn = GetSavepoint(0);
    bool wasActive = (txn != nullptr) && (BE_SQLITE_OK == txn->Commit(nullptr));

    DbResult rc = (DbResult)sqlite3_exec(GetSqlDb(), SqlPrintfString("ATTACH \"%s\" AS %s", filename, alias), nullptr, nullptr, nullptr);

    if (rc != BE_SQLITE_OK) {
        BeAssert(false);
        Utf8String lastError = GetLastError(nullptr); // keep on separate line for debuggging
        LOG.errorv("AttachDb failed: \"%s\" filename:[%s], alias:[%s]", lastError.c_str(), filename, alias);
    }

    if (wasActive)
        txn->Begin();

    if (BE_SQLITE_OK == rc)
        return _OnDbAttached(filename, alias);

    return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DetachDb(Utf8CP alias) const
    {
    Savepoint* txn = GetSavepoint(0);
    bool wasActive = (nullptr != txn) && (BE_SQLITE_OK == txn->Commit(nullptr));

    DbResult rc = (DbResult) sqlite3_exec(GetSqlDb(), SqlPrintfString("DETACH %s", alias), nullptr, nullptr, nullptr);
    if (rc != BE_SQLITE_OK)
        LOG.errorv("DetachDb failed: \"%s\" alias:[%s]", GetLastError(nullptr).c_str(), alias);

    if (wasActive)
        txn->Begin();

    if (BE_SQLITE_OK == rc)
        return _OnDbDetached(alias);

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
*
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BriefcaseLocalValueCache::Register(size_t& index, Utf8CP name)
    {
    BeMutexHolder lock(m_mutex);

    size_t existingIndex = 0;
    if (TryGetIndex(existingIndex, name))
        return BE_SQLITE_ERROR;

    m_cache.push_back(CachedBLV(name));
    index = m_cache.size() - 1;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BriefcaseLocalValueCache::TryGetIndex(size_t& index, Utf8CP name)
    {
    BeMutexHolder lock(m_mutex);

    const size_t size = m_cache.size();
    for (size_t i = 0; i < size; i++)
        {
        if (strcmp(name, m_cache[i].GetName()) == 0)
            {
            index = i;
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BriefcaseLocalValueCache::Clear()
    {
    BeMutexHolder lock(m_mutex);

    for (CachedBLV& val : m_cache)
        val.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BriefcaseLocalValueCache::SaveValue(size_t rlvIndex, uint64_t value)
    {
    BeMutexHolder lock(m_mutex);

    if (rlvIndex >= m_cache.size())
        return BE_SQLITE_NOTFOUND;

    m_cache[rlvIndex].ChangeValue(value);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BriefcaseLocalValueCache::QueryValue(uint64_t& value, size_t rlvIndex)
    {
    BeMutexHolder lock(m_mutex);

    if (rlvIndex >= m_cache.size())
        return BE_SQLITE_NOTFOUND;

    CachedBLV* cachedRlv = nullptr;
    if (!TryQuery(cachedRlv, rlvIndex))
        return BE_SQLITE_ERROR;

    value = cachedRlv->GetValue();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BriefcaseLocalValueCache::IncrementValue(uint64_t& newValue, size_t rlvIndex)
    {
    BeMutexHolder lock(m_mutex);

    CachedBLV* cachedRlv = nullptr;
    if (!TryQuery(cachedRlv, rlvIndex))
        return BE_SQLITE_ERROR;

    newValue = cachedRlv->Increment();
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BriefcaseLocalValueCache::CheckMaxValue(uint64_t value, size_t rlvIndex)
    {
    BeMutexHolder lock(m_mutex);

    CachedBLV* cachedRlv = nullptr;
    if (!TryQuery(cachedRlv, rlvIndex))
        return BE_SQLITE_ERROR;

    if (value > cachedRlv->GetValue())
        cachedRlv->ChangeValue(value, false);

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool BriefcaseLocalValueCache::TryQuery(CachedBLV*& value, size_t rlvIndex)
    {
    BeMutexHolder lock(m_mutex);

    if (rlvIndex >= m_cache.size())
        return false;

    CachedBLV& cachedRlv = m_cache[rlvIndex];
    if (cachedRlv.IsUnset())
        {
        Statement stmt;
        stmt.Prepare(m_dbFile, "SELECT Val FROM " BEDB_TABLE_Local " WHERE Name=?");
        stmt.BindText(1, cachedRlv.GetName(), Statement::MakeCopy::No);

        const DbResult rc = stmt.Step();
        if (rc != BE_SQLITE_ROW)
            return false;

        const uint64_t val = stmt.GetValueUInt64(0);
        cachedRlv.ChangeValue(val, true);
        }

    BeAssert(!cachedRlv.IsUnset());
    value = &cachedRlv;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CachedBLV::CachedBLV(Utf8CP name) : m_name(name) {BeAssert(!Utf8String::IsNullOrEmpty(name)); Reset();}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CachedBLV::ChangeValue(uint64_t value, bool initializing)
    {
    m_isUnset = false; m_dirty = !initializing; m_value = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t CachedBLV::Increment()
    {
    BeAssert(!m_isUnset); m_dirty = true; m_value++; return m_value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CachedBLV::SetIsNotDirty() const {BeAssert(!m_isUnset); m_dirty = false;}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CachedBLV::Reset() {m_isUnset = true; m_dirty = false; m_value = 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::SetupBlvSaveStmt(Statement& stmt, Utf8CP name)
    {
    DbResult rc = stmt.Prepare(*this, "INSERT OR REPLACE INTO " BEDB_TABLE_Local " (Name,Val) VALUES(?,?)");
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);

    stmt.BindText(1, name, Statement::MakeCopy::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveBriefcaseLocalValue(Utf8CP name, Utf8StringCR value)
    {
    Statement stmt;
    SetupBlvSaveStmt(stmt, name);

    stmt.BindText(2, value, Statement::MakeCopy::No);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveBriefcaseLocalValue(Utf8CP name, uint64_t value)
    {
    Statement stmt;
    SetupBlvSaveStmt(stmt, name);

    stmt.BindUInt64(2, value);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::ExecBlvQueryStmt(Statement& stmt, Utf8CP name) const
    {
    DbResult rc = stmt.Prepare(*this, "SELECT Val FROM " BEDB_TABLE_Local " WHERE Name=?");
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);

    stmt.BindText(1, name, Statement::MakeCopy::No);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryBriefcaseLocalValue(Utf8StringR value, Utf8CP name) const
    {
    Statement stmt;
    DbResult rc = ExecBlvQueryStmt(stmt, name);
    if (rc != BE_SQLITE_ROW)
        return rc;

    value = stmt.GetValueText(0);
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryBriefcaseLocalValue(uint64_t& value, Utf8CP name) const
    {
    Statement stmt;
    DbResult rc = ExecBlvQueryStmt(stmt, name);
    if (rc != BE_SQLITE_ROW)
        return rc;

    value = stmt.GetValueUInt64(0);
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DeleteBriefcaseLocalValue(Utf8CP name)
    {
    Statement stmt;
    DbResult rc = stmt.Prepare(*this, "DELETE FROM " BEDB_TABLE_Local " WHERE Name=?");
    UNUSED_VARIABLE (rc);
    BeAssert(rc == BE_SQLITE_OK);

    stmt.BindText(1, name, Statement::MakeCopy::No);
    return stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isMainTableOrIndex(Utf8CP tableName)
    {
    return (nullptr == ::strchr(tableName, '.') || 0 == BeStringUtilities::Strnicmp(tableName, "main.", 5));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateTable(Utf8CP tableName, Utf8CP ddl) const
    {
    SqlPrintfString sql("CREATE TABLE %s (%s)", tableName, ddl);

    DbResult result = ExecuteSql(sql);
    if (result != BE_SQLITE_OK)
        return result;

    ChangeTracker* tracker = m_dbFile->m_tracker.get();
    if (tracker && isMainTableOrIndex(tableName))
        result = tracker->RecordDbSchemaChange(sql.GetUtf8CP());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateTableIfNotExists(Utf8CP tableName, Utf8CP ddl) const
{
	SqlPrintfString sql("CREATE TABLE IF NOT EXISTS %s (%s)", tableName, ddl);

	DbResult result = ExecuteSql(sql);
	if (result != BE_SQLITE_OK)
		return result;

	ChangeTracker* tracker = m_dbFile->m_tracker.get();
	if (tracker && isMainTableOrIndex(tableName))
		result = tracker->RecordDbSchemaChange(sql.GetUtf8CP());

	return result;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DropTable(Utf8CP tableName, bool requireExists) const
    {
    ChangeTracker* tracker = m_dbFile->m_tracker.get();
    if (tracker && tracker->IsTracking() && isMainTableOrIndex(tableName))
        {
        BeAssert(false && "Cannot make arbitrary schema changes when changes are being tracked");
        return BE_SQLITE_ERROR;
        }

    DbResult rc = TryExecuteSql(SqlPrintfString("DROP TABLE %s", tableName));
    if (!requireExists && BE_SQLITE_ERROR == rc)
        rc = BE_SQLITE_OK;

    BeAssert(rc == BE_SQLITE_OK);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AddColumnToTable(Utf8CP tableName, Utf8CP columnName, Utf8CP columnDetails)
    {
    SqlPrintfString sql("ALTER TABLE %s ADD COLUMN %s %s", tableName, columnName, columnDetails);

    DbResult result = ExecuteSql(sql);
    if (result != BE_SQLITE_OK)
        return result;

    ChangeTracker* tracker = m_dbFile->m_tracker.get();
    if (tracker && isMainTableOrIndex(tableName))
        result = tracker->RecordDbSchemaChange(sql.GetUtf8CP());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::CreateIndex(Utf8CP indexName, Utf8CP tableName, bool isUnique, Utf8CP columnName1, Utf8CP columnName2 /*=nullptr*/)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(indexName) && !Utf8String::IsNullOrEmpty(tableName) && !Utf8String::IsNullOrEmpty(columnName1));

    // e.g.,  "CREATE UNIQUE INDEX ix_ec_Class_SchemaId_Name ON ec_Class(SchemaId, Name);"
    SqlPrintfString sql("CREATE %s INDEX %s ON %s(%s%s%s)", isUnique ? "UNIQUE" : "", indexName, tableName, columnName1, Utf8String::IsNullOrEmpty(columnName2) ? "" : ",", columnName2 ? columnName2 : "");

    DbResult result = ExecuteSql(sql);
    if (result != BE_SQLITE_OK)
        return result;

    ChangeTracker* tracker = m_dbFile->m_tracker.get();
    if (tracker && isMainTableOrIndex(indexName))
        result = tracker->RecordDbSchemaChange(sql.GetUtf8CP());

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::TruncateTable(Utf8CP tableName) const
    {
    DbResult rc = TryExecuteSql(SqlPrintfString("DELETE FROM %s", tableName));
    BeAssert(rc == BE_SQLITE_OK);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::TableExists(Utf8CP tableName) const
    {
    Statement statement;
    return BE_SQLITE_OK == statement.TryPrepare(*this, SqlPrintfString("SELECT NULL FROM %s", tableName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
void Db::FlushPageCache()
    {
    sqlite3_db_release_memory(m_dbFile->m_sqlDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::ColumnExists(Utf8CP tableName, Utf8CP columnName) const
    {
    Statement sql;
    return BE_SQLITE_OK == sql.TryPrepare(*this, SqlPrintfString("SELECT [%s] FROM %s", columnName, tableName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::GetColumns(bvector<Utf8String>& columns, Utf8CP tableName) const
    {
    Statement statement;
    DbResult status =  statement.TryPrepare(*this, SqlPrintfString("SELECT * FROM %s LIMIT 0", tableName));
    if (status != BE_SQLITE_OK)
        return false;

    columns.clear();
    for (int nColumn = 0; nColumn < statement.GetColumnCount(); nColumn++)
        columns.push_back(statement.GetColumnName(nColumn));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::RenameTable(Utf8CP tableName, Utf8CP newTableName)
    {
    ChangeTracker* tracker = m_dbFile->m_tracker.get();
    if (tracker && tracker->IsTracking() && isMainTableOrIndex(tableName))
        {
        BeAssert(false && "Cannot make arbitrary schema changes when changes are being tracked");
        return BE_SQLITE_ERROR;
        }

    return ExecuteSql(SqlPrintfString("ALTER TABLE %s RENAME TO %s", tableName, newTableName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFile::DeleteCachedPropertyMap()
    {
    if (nullptr == m_cachedProps)
        return;

    delete (CachedPropertyMap*) m_cachedProps;
    m_cachedProps = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbTxnState DbFile::GetTxnState(Utf8CP schema) const {
    return (DbTxnState)sqlite3_txn_state(m_sqlDb, nullptr);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbFile::~DbFile() {
    if (nullptr == m_sqlDb)
        return;

    //! IMPORTANT we need to make sure if tracker was set and has changes we should *ROLLBACK* instead of *COMMIT*
    const bool isChangeTrackerIsEnabled = m_tracker.IsValid();
    const bool isChangeTrackerHasChanges = isChangeTrackerIsEnabled ? m_tracker->HasChanges() : false;
    const bool isInMemoryDb = strcmp (sqlite3_db_filename(m_sqlDb, nullptr), "") == 0;
    m_tracker = nullptr;

    if (0 != m_txns.size()) {
        DbResult stat = BE_SQLITE_OK;
        if (isChangeTrackerIsEnabled) {
            // We only want to assert only when there was changes.
            if (isChangeTrackerHasChanges) {
                BeAssert(false && "Local changes will be lost, call SaveChanges to save them.");
            }
            stat = m_txns[0]->Cancel();
        } else {
            if (!m_readonly && GetTxnState() == BE_SQLITE_TXN_WRITE && !isInMemoryDb) {
                if (m_defaultTxn.GetTxnMode() == BeSQLiteTxnMode::Deferred) {
                    BeAssert(false && "Auto commit on ~Db() behaviour will be removed in future. All code must call SaveChanges() on writable connection to ensure changes are written to disk.");
                }
                LOG.warning("Auto commit on ~Db() behaviour will be removed in future. All code must call SaveChanges() on writable connection to ensure changes are written to disk.");
            }
            stat = m_txns[0]->Commit(nullptr);
        }
        UNUSED_VARIABLE(stat);
        BeAssert(BE_SQLITE_OK==stat);
        m_txns.clear();
    }

    BeAssert(!m_defaultTxn.IsActive()); // should have been committed above
    m_defaultTxn.m_depth = -1;  // just in case commit failed. Otherwise destructor will attempt to access this DbFile.
    m_statements.Empty();       // No more statements will be prepared and cached.
    DeleteCachedPropertyMap();
    m_blvCache.Clear();

    BeAssert(m_txns.empty());
    BeAssert(m_statements.IsEmpty());
    BeAssert(nullptr == m_cachedProps);
    RemoveFunction(*m_regexFunc);
    RemoveFunction(*m_regexExtractFunc);
    DbResult rc = (DbResult) sqlite3_close(m_sqlDb);

    if (BE_SQLITE_OK != rc) {
        sqlite3_stmt* stmt = nullptr;
        while (nullptr != (stmt = sqlite3_next_stmt(m_sqlDb, stmt))) {
            Utf8String openStatement(sqlite3_sql(stmt)); // keep as separate line for debugging
            LOG.errorv("Statement not closed: '%s'", openStatement.c_str());
        };

        LOG.errorv("Cannot close database '%s'", sqlite3_db_filename(m_sqlDb, "main"));
        BeAssert(false);
    }

    m_sqlDb = 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::IsDbOpen() const
    {
    return  (nullptr != m_dbFile) && (nullptr != m_dbFile->m_sqlDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::DoCloseDb()
    {
    if (!IsDbOpen())
        return;

    m_appData.Clear();

    m_statements.Empty();
    DELETE_AND_CLEAR(m_dbFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::CloseDb()
    {
    m_appData.Clear();
    _OnDbClose();
    DoCloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Db::AppDataCollection::Drop(AppData::Key const& key)
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_map.find(&key);
    if (m_map.end() == iter)
        return ERROR;

    // Note: We don't want to appdata object to be destroyed while it's being
    // removed from the map - in certain cases, where appdata's destructor is used
    // for observing db close events, we had a situation where we could still find
    // this object (with 0 ref count) in the map. To avoid it, we keep a ref while
    // removing and the object gets destroyed when we return from this method.
    AppDataPtr value = iter->second;
    m_map.erase(iter);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Db::DropAppData(AppData::Key const& key) const
    {
    return m_appData.Drop(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::AppDataCollection::AddInternal(AppData::Key const& key, AppData* data)
    {
    auto entry = m_map.Insert(&key, data);
    if (!entry.second)
        {
        // we already had appdata for this key. Clean up old and save new.
        entry.first->second = data;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::AddAppData(AppData::Key const& key, AppData* appData) const
    {
    m_appData.Add(key, appData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Db::AppData* Db::AppDataCollection::FindInternal(AppData::Key const& key)
    {
    auto entry = m_map.find(&key);
    return m_map.end() == entry ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Db::AppDataPtr Db::FindAppData(AppData::Key const& key) const
    {
    return m_appData.Find(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::SetLimit(DbLimits id, int newVal) const { return sqlite3_limit(m_dbFile->m_sqlDb, (int)id, newVal); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Db::GetLimit(DbLimits id) const { return sqlite3_limit(m_dbFile->m_sqlDb, (int)id, -1); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Db::GetDbFileName() const
    {
    return  (m_dbFile && m_dbFile->m_sqlDb) ? sqlite3_db_filename(m_dbFile->m_sqlDb, "main") : nullptr;
    }

#define IS_SQLITE_FILE_SIGNATURE(toCheck) (0 == memcmp((Utf8CP)header, toCheck, strlen(toCheck)))

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult isValidDbFile(Utf8CP filename) {
    BeFile tester;
    BeFileName filenameW(filename, BentleyCharEncoding::Utf8);
    BeFileStatus status = tester.Open(filenameW.c_str(), BeFileAccess::Read);

    switch (status) {
    case BeFileStatus::Success:
        break;
    case BeFileStatus::FileNotFoundError:
        return BE_SQLITE_ERROR_FileNotFound;
    default:
        return BE_SQLITE_ERROR;
    }

    uint32_t bytesRead;
    uint8_t header[100] = "";
    tester.Read(header, &bytesRead, sizeof(header));
    tester.Close();

    if (IS_SQLITE_FILE_SIGNATURE("SQLite format 3")) {
        if (bytesRead != sizeof(header))
            return BE_SQLITE_CORRUPT;

        return BE_SQLITE_OK;
    }

    return BE_SQLITE_NOTADB;
}

#ifdef TRACE_ALL_SQLITE_STATEMENTS
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void tracefunc(void*, char const* stmt)
    {
    printf("SQL>%s\n", stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void printLog(void *pArg, int iErrCode, Utf8CP zMsg)
    {
    char const* sev = (iErrCode == SQLITE_NOTICE)? "trace":
                      (iErrCode == SQLITE_WARNING)? "warn": "error";
    printf("SQL %s %d %s\n", sev, iErrCode, zMsg);
    }
#endif

/** return true if this database is using WAL mode. */
bool Db::IsWalMode() const {
    // CloudSqlite always uses WAL mode (and produces an error if you attempt to query the journal mode)
    if (m_isCloudDb)
        return true;

    Statement stmt;
    stmt.Prepare(*this, "pragma journal_mode");
    stmt.Step();
    return 0 == strncmp("wal", stmt.GetValueText(0), 3);
};

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::DoOpenDb(Utf8CP inName, OpenParams const& params) {
    if (IsDbOpen())
        return BE_SQLITE_ERROR_AlreadyOpen;

    DbResult rc = BE_SQLITE_OK;
    if (!params.m_skipFileCheck) {
        rc = isValidDbFile(inName);
        if (BE_SQLITE_OK != rc)
            return rc;
    }

    for (auto const& param : params.m_queryParams)
        m_openQueryParams.push_back(param);

    Utf8String uri = getDbUri(inName, params);
    SqlDbP sqlDb = nullptr;
    rc = (DbResult)sqlite3_open_v2(uri.c_str(), &sqlDb, (int)params.m_openMode, nullptr);
    if (BE_SQLITE_OK != rc)
        return rc;

    // Bloom filter introduce in 3.38 has issue with certain queries and fix was in ANALYZE
    // command which had a rounding error. We need to keep bloom filter disabled for now until
    // we are able to run ANALYZE on all new checkpoints and old one if necessary.
    DisableBloomFilter(sqlDb);

#ifdef TRACE_ALL_SQLITE_STATEMENTS
    sqlite3_config(SQLITE_CONFIG_LOG, printLog, nullptr);
    sqlite3_trace(sqlDb, tracefunc, nullptr);
#endif

    // set the tempfileBase from open params
    m_tempfileBase = params.m_tempfileBase;
    m_isCloudDb = params.m_fromContainer;

    m_dbFile = new DbFile(sqlDb, params.m_busyRetry, (BeSQLiteTxnMode)params.m_startDefaultTxn);
    m_dbFile->m_readonly = ((int)params.m_openMode & (int)OpenMode::Readonly) == (int)OpenMode::Readonly;
    sqlite3_extended_result_codes(sqlDb, 1); // turn on extended error codes

    // for writeable databases in WAL mode with DEFERRED defaultTxn mode, promote it to IMMEDIATE mode so there can only
    // be one writer. Without WAL mode we can't do that because the (single) writer would block readers.
    if (IsWriteable() && m_dbFile->m_defaultTxn.GetTxnMode() == BeSQLiteTxnMode::Deferred && IsWalMode()) {
        m_dbFile->m_defaultTxn.SetTxnMode(BeSQLiteTxnMode::Immediate);
    } else if (m_dbFile->m_defaultTxn.GetTxnMode() == BeSQLiteTxnMode::Exclusive) {
        rc = TryExecuteSql("PRAGMA locking_mode=\"EXCLUSIVE\"");
        BeAssert(rc == BE_SQLITE_OK);
    }

    rc = m_dbFile->m_defaultTxn.Begin();
    if (BE_SQLITE_OK != rc)
        return rc;

    if (!params.m_rawSQLite) {
        // make sure that the be_Prop table exists and has the right columns before continuing
        Statement statement;
        rc = statement.TryPrepare(*this, "SELECT Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData FROM " BEDB_TABLE_Property);
        if (BE_SQLITE_OK != rc)
            return rc == BE_SQLITE_ERROR ? BE_SQLITE_ERROR_NoPropertyTable : rc;

        rc = _OnDbOpening();
    }

    if (params.m_startDefaultTxn == DefaultTxn::No)
        m_dbFile->m_defaultTxn.Commit(nullptr);

    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::DoProfileUpgrade() {
    DbResult rc = _OnBeforeProfileUpgrade();
    if (BE_SQLITE_OK != rc) {
        AbandonChanges();
        CloseDb();
        return rc;
    }

    rc = _UpgradeProfile();
    if (BE_SQLITE_OK != rc) {
        AbandonChanges();
        CloseDb();
        return rc;
    }

    rc = _OnAfterProfileUpgrade();
    if (BE_SQLITE_OK != rc) {
        AbandonChanges();
        CloseDb();
        return rc;
    }

    // re-check profile version. Should be up-to-date now, but in case a profile upgrader is missing something
    // we want to make sure we now know the new profile state
    ProfileState newProfileState = _CheckProfileVersion();
    rc = newProfileState.ToDbResult();
    if (BE_SQLITE_OK != rc) {
        AbandonChanges();
        CloseDb();
        return rc;
    }
    return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::OpenBeSQLiteDb(Utf8CP dbName, OpenParams const& params) {
    if (params.GetProfileUpgradeOptions() == ProfileUpgradeOptions::Upgrade && params.IsReadonly())
        return BE_SQLITE_READONLY;

    DbResult rc = DoOpenDb(dbName, params);
    if (rc != BE_SQLITE_OK || params.m_rawSQLite) {
        if (rc != BE_SQLITE_OK)
            CloseDb();
        return rc;
    }

    // ensure that profile upgraders have an active transaction even if DefaultTxn::No was passed
    if (params.m_startDefaultTxn == DefaultTxn::No && !m_dbFile->m_defaultTxn.IsActive())
        m_dbFile->m_defaultTxn.Begin();

    const ProfileState profileState = _CheckProfileVersion();
    const bool doUpgrade = profileState.IsUpgradable() && params.GetProfileUpgradeOptions() == ProfileUpgradeOptions::Upgrade;

    if (profileState.IsError() ||
        (!doUpgrade && (profileState.GetCanOpen() == ProfileState::CanOpen::No ||
                        (profileState.GetCanOpen() == ProfileState::CanOpen::Readonly && !params.IsReadonly())))) {
        CloseDb();
        return profileState.ToDbResult();
    }

    if (doUpgrade) {
        rc = DoProfileUpgrade();
        if (BE_SQLITE_OK != rc) {
            return rc;
        }
    }

    rc = _OnDbOpened(params);
    if (BE_SQLITE_OK != rc) {
        AbandonChanges();
        CloseDb();
        return rc;
    }

    // if DefaultTxn::No was passed, commit the txn as it was started by BeSQlite
    if (params.m_startDefaultTxn == DefaultTxn::No && m_dbFile->m_defaultTxn.IsActive())
        m_dbFile->m_defaultTxn.Commit(nullptr);

    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::OpenSecondaryConnection(Db& newConnection, OpenParams params) const
    {
    if (!IsDbOpen())
        return BE_SQLITE_ERROR_NOTOPEN;

    // NOTE: We don't currently pass in any query params in cases where we need to open a secondary connection.
    // If we ever find the need to do so, we may need to modify this loop so that the queryParamsAtOpen and queryParams passed into opensecondaryconnection don't step on eachother.
    for (auto const& param : m_openQueryParams) {
        if (param.Contains("vfs"))
            params.m_skipFileCheck = true;
        params.AddQueryParam(param.c_str());
    }

    return newConnection.OpenBeSQLiteDb(GetDbFileName(), params);
    }

/** Perform a checkpoint operation if this database is in WAL mode. */
DbResult Db::PerformCheckpoint(WalCheckpointMode mode, int* pnLog, int* pnCkpt) {
    SuspendDefaultTxn noDefaultTxn(*this); // no transactions may be active to perform a checkpoint
    return (DbResult) sqlite3_wal_checkpoint_v2(GetSqlDb(), "main", (int)mode, pnLog, pnCkpt);
}

/** Set the auto checkpoint threshold */
DbResult Db::SetAutoCheckpointThreshold(int frames) {
    return (DbResult) sqlite3_wal_autocheckpoint(GetSqlDb(), frames);
}

/** Turn on or off WAL journal mode */
DbResult Db::EnableWalMode(bool yesNo) {
    SuspendDefaultTxn noDefaultTxn(*this);
    auto modeStr = yesNo ? "wal" : "delete";
    auto sql = Utf8String("pragma journal_mode=").append(modeStr);
    Statement stmt;
    stmt.Prepare(*this, sql.c_str());
    auto rc = stmt.Step();
    if (rc != BE_SQLITE_ROW)
        return rc;
    auto mode = stmt.GetValueText(0);
    return 0 == strncmp(modeStr, mode, strlen(modeStr)) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState Db::_CheckProfileVersion() const { return BeSQLiteProfileManager::CheckProfileVersion(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult Db::_UpgradeProfile() { return BeSQLiteProfileManager::UpgradeProfile(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryExpirationDate(DateTime& expirationDate) const
    {
    Utf8String dateStr;
    DbResult rc = QueryProperty(dateStr, Properties::ExpirationDate());
    if (BE_SQLITE_ROW != rc)
        return BE_SQLITE_NOTFOUND;   // expiration date is optional

    expirationDate = DateTime::FromString(dateStr.c_str());
    if (!expirationDate.IsValid())    {
        BeDataAssert(false && "invalid value stored for expiration date property");
        return BE_SQLITE_ERROR;
    }

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveExpirationDate(DateTime const& expirationDate)
    {
    if (!expirationDate.IsValid())
        return BE_SQLITE_ERROR;

    if (expirationDate.GetInfo().GetKind() != DateTime::Kind::Utc)
        return BE_SQLITE_ERROR;

    Utf8String xdateString(expirationDate.ToString());
    if (xdateString.empty())
        {
        BeAssert(false); // a valid DateTime generated an empty string?
        return BE_SQLITE_ERROR;
        }

    return SavePropertyString(Properties::ExpirationDate(), xdateString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::IsExpired() const
    {
    DateTime expirationDate;
    if (BE_SQLITE_ROW != QueryExpirationDate(expirationDate))
        return false;

    return DateTime::Compare(DateTime::GetCurrentTimeUtc(), expirationDate) != DateTime::CompareResult::EarlierThan;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveChanges(Utf8CP operation)
    {
    Savepoint* txn = GetSavepoint(0);
    return txn ? txn->Save(operation) : BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::AbandonChanges()
    {
    Savepoint* txn = GetSavepoint(0);
    if (nullptr == txn)
        return  BE_SQLITE_ERROR;

    txn->Cancel();
    return txn->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::_OnDbChangedByOtherConnection()
    {
    m_dbFile->DeleteCachedPropertyMap();
    m_dbFile->m_blvCache.Clear();
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState Db::CheckProfileVersion(ProfileVersion const& expectedProfileVersion, ProfileVersion const& fileProfileVersion, ProfileVersion const& minimumUpgradableProfileVersion, Utf8CP profileName)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(profileName));
    BeAssert(minimumUpgradableProfileVersion.GetSub1() == 0 && minimumUpgradableProfileVersion.GetSub2() == 0 && "Minimum upgradable version's sub1 and sub2 must be 0.");

    const int fileVersionComp = fileProfileVersion.CompareTo(expectedProfileVersion, ProfileVersion::VERSION_All);

    if (fileVersionComp == 0)
        return ProfileState::UpToDate();

    const int fileVersionMajorComp = fileProfileVersion.CompareTo(expectedProfileVersion, ProfileVersion::VERSION_Major);
    const int fileVersionMinorComp = fileProfileVersion.CompareTo(expectedProfileVersion, ProfileVersion::VERSION_Minor);

    //File is newer than software
    if (fileVersionComp > 0)
        {
        if (fileVersionMajorComp > 0)
            {
            LOG.errorv("Cannot open file: The file's %s profile (%s) is too new. Expected version: %s.",
                       profileName, fileProfileVersion.ToString().c_str(), expectedProfileVersion.ToString().c_str());
            return ProfileState::Newer(ProfileState::CanOpen::No);
            }

        if (fileVersionMinorComp > 0)
            {
            return ProfileState::Newer(ProfileState::CanOpen::Readonly);
            }

        return ProfileState::Newer(ProfileState::CanOpen::Readwrite);
        }

    BeAssert(fileVersionComp < 0 && "At this point file version must be older than expected");

    const bool isUpgradable = fileProfileVersion.CompareTo(minimumUpgradableProfileVersion) >= 0;

    if (fileVersionMajorComp < 0)
        {
        LOG.errorv("Cannot open file: The file's %s profile (%s) is too old. Expected version: %s.",
                   profileName, fileProfileVersion.ToString().c_str(), expectedProfileVersion.ToString().c_str());
        return ProfileState::Older(ProfileState::CanOpen::No, isUpgradable);
        }

    if (fileVersionMinorComp < 0)
        return ProfileState::Older(ProfileState::CanOpen::Readonly, isUpgradable);

    return ProfileState::Older(ProfileState::CanOpen::Readwrite, isUpgradable);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState ProfileState::Merge(ProfileState const& rhs) const
    {
    if (*this == rhs)
        return *this;

    if (IsError() || rhs.IsError())
        return *this;

    if (m_age == Age::UpToDate)
        return rhs;

    if (rhs.m_age == Age::UpToDate)
        return *this;

    if (m_age != rhs.m_age)
        {
        BeAssert(false && "One profile cannot be older when the other is newer");
        return Error();
        }

    const int thisCanOpenInt = (int) m_canOpen;
    const int rhsCanOpenInt = (int) rhs.m_canOpen;
    const CanOpen finalCanOpen = (CanOpen) std::max(thisCanOpenInt, rhsCanOpenInt);

    if (m_age == Age::Newer)
        return ProfileState::Newer(finalCanOpen);

    bool finalIsUpgradable = m_isUpgradable || rhs.m_isUpgradable;
    if ((m_canOpen == CanOpen::No && !m_isUpgradable) || (rhs.m_canOpen == CanOpen::No && !rhs.m_isUpgradable))
        finalIsUpgradable = false;

    return ProfileState::Older(finalCanOpen, finalIsUpgradable);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ProfileState::ToDbResult() const
    {
    if (IsError())
        return BE_SQLITE_ERROR_InvalidProfileVersion;

    if (m_age == Age::UpToDate)
        return BE_SQLITE_OK;

    if (m_age == Age::Newer)
        {
        switch (m_canOpen)
            {
                case CanOpen::Readwrite:
                    return BE_SQLITE_OK;
                case CanOpen::Readonly:
                    return BE_SQLITE_ERROR_ProfileTooNewForReadWrite;
                case CanOpen::No:
                    return BE_SQLITE_ERROR_ProfileTooNew;

                default:
                    BeAssert(false && "Unhandled enum value");
                    return BE_SQLITE_ERROR_InvalidProfileVersion;
            }
        }

    switch (m_canOpen)
        {
            case CanOpen::Readwrite:
                return BE_SQLITE_OK;
            case CanOpen::Readonly:
                return BE_SQLITE_ERROR_ProfileTooOldForReadWrite;
            case CanOpen::No:
                return BE_SQLITE_ERROR_ProfileTooOld;

            default:
                BeAssert(false && "Unhandled enum value");
                return BE_SQLITE_ERROR_InvalidProfileVersion;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::QueryDbIds()
    {
    DbResult rc = QueryProperty(&m_dbFile->m_dbGuid, sizeof(m_dbFile->m_dbGuid), Properties::DbGuid());
    if (BE_SQLITE_ROW != rc)
        return  BE_SQLITE_ERROR_NoPropertyTable;

    // BriefcaseId is a 32 bit number, but for historical reasons is saved as a 64 number in the db.
    Statement stmt;
    rc = stmt.Prepare(*this, "SELECT Val FROM " BEDB_TABLE_Local " WHERE Name='" BEDB_BRIEFCASELOCALVALUE_BriefcaseId "'");
    if (BE_SQLITE_OK != rc || BE_SQLITE_ROW != stmt.Step())
        return BE_SQLITE_ERROR;

    int64_t val = 0LL;
    if (stmt.GetColumnBytes(0) >= (int) sizeof(int64_t))
        memcpy(&val, stmt.GetValueBlob(0), sizeof(val));

    m_dbFile->m_briefcaseId = BeBriefcaseId((uint32_t) val);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbFunction::Context::SetResultBlob(void const* value, int length, CopyData doCopy) {sqlite3_result_blob((sqlite3_context*) this, value, length, (sqlite3_destructor_type) doCopy);}
void DbFunction::Context::SetResultDouble(double val){sqlite3_result_double((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultError(Utf8CP val, int len){sqlite3_result_error((sqlite3_context*) this, val, len);}
void DbFunction::Context::SetResultError_toobig(){sqlite3_result_error_toobig((sqlite3_context*) this);}
void DbFunction::Context::SetResultError_nomem(){sqlite3_result_error_nomem((sqlite3_context*) this);}
void DbFunction::Context::SetResultError_code(int val){sqlite3_result_error_code((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultInt(int val){sqlite3_result_int((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultInt64(int64_t val){sqlite3_result_int64((sqlite3_context*) this, val);}
void DbFunction::Context::SetResultNull(){sqlite3_result_null((sqlite3_context*) this);}
void DbFunction::Context::SetResultText(Utf8CP val, int length, CopyData doCopy){sqlite3_result_text((sqlite3_context*) this, val, length,(sqlite3_destructor_type) doCopy);}
void DbFunction::Context::SetResultZeroblob(int length){sqlite3_result_zeroblob((sqlite3_context*)this, length);}
void DbFunction::Context::SetResultValue(DbValue val){sqlite3_result_value((sqlite3_context*)this, val.GetSqlValueP());}
void* DbFunction::Context::GetAggregateContext(int nBytes) {return sqlite3_aggregate_context((sqlite3_context*)this, nBytes);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void aggregateStep(sqlite3_context* context, int nArgs, sqlite3_value** args){((AggregateFunction*)sqlite3_user_data(context))->_StepAggregate((DbFunction::Context&) *context, nArgs, (DbValue*)args);}
static void aggregateFinal(sqlite3_context* context) {((AggregateFunction*) sqlite3_user_data(context))->_FinishAggregate((DbFunction::Context&) *context);}
static void scalarFunc(sqlite3_context* context, int nArgs, sqlite3_value** args) {((ScalarFunction*)sqlite3_user_data(context))->_ComputeScalar((DbFunction::Context&) *context, nArgs, (DbValue*)args);}
static int  rTreeMatch(RTreeMatchFunction::QueryInfo* info){return ((RTreeMatchFunction*) info->m_context)->_TestRange(*info);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::AddFunction(DbFunction& function) const
    {
    bool isAgg = function._IsAggregate();
    return sqlite3_create_function_v2(m_sqlDb, function.GetName(), function.GetNumArgs(), SQLITE_UTF8 | SQLITE_DETERMINISTIC, &function,
            isAgg ? nullptr        : scalarFunc,
            isAgg ? aggregateStep  : nullptr,
            isAgg ? aggregateFinal : nullptr,
            nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::AddRTreeMatchFunction(RTreeMatchFunction& func) const
    {
    return sqlite3_rtree_query_callback(m_sqlDb, func.GetName(), (int(*)(sqlite3_rtree_query_info*)) rTreeMatch, &func, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int DbFile::RemoveFunction(DbFunction& function) const
    {
    return sqlite3_create_function_v2(m_sqlDb, function.GetName(), function.GetNumArgs(), SQLITE_UTF8 | SQLITE_DETERMINISTIC, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static void updateHookCallback(void* arg, int sqlTypeInt, Utf8CP dbName, Utf8CP tableName, sqlite3_int64 rowid)
    {
    DataUpdateCallback::SqlType sqlType = DataUpdateCallback::SqlType::Insert;
    switch (sqlTypeInt)
        {
            case SQLITE_INSERT:
                sqlType = DataUpdateCallback::SqlType::Insert;
                break;

            case SQLITE_UPDATE:
                sqlType = DataUpdateCallback::SqlType::Update;
                break;
            case SQLITE_DELETE:
                sqlType = DataUpdateCallback::SqlType::Delete;
                break;

            default:
                BeAssert(false && "Unexpected SQL type passed to call back in sqlite3_update_hook");
                return;
        }

    ((DataUpdateCallback*) arg)->_OnRowModified(sqlType, dbName, tableName, BeInt64Id((uint64_t) rowid));
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void DbFile::AddDataUpdateCallback(DataUpdateCallback& updateHook) const
    {
    void* previousHook = sqlite3_update_hook(m_sqlDb, updateHookCallback, &updateHook);
    if (previousHook != nullptr)
        {
        BeAssert(false && "Previous SQLite data change notification callback was overridden by call to Db::AddDataUpdateCallback");
        LOG.debugv("SQLite data change notification callback was overridden by call to Db::AddDataUpdateCallback.");
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void DbFile::RemoveDataUpdateCallback() const { sqlite3_update_hook(m_sqlDb, nullptr, nullptr); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::SetChangeTracker(ChangeTracker* tracker)
    {
    if (m_dbFile->m_tracker.get() == tracker)
        return;

    if (tracker != nullptr)
        tracker->SetDb(this);

    m_dbFile->m_tracker = tracker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbFile::GetLastError(DbResult* lastResult) const
    {
    DbResult ALLOW_NULL_OUTPUT(status, lastResult);
    if (nullptr == m_sqlDb)
        {
        status = BE_SQLITE_ERROR_NOTOPEN;
        return BeSQLiteLib::GetErrorString(status);
        }

    status = (DbResult) sqlite3_errcode(m_sqlDb);
    Utf8String msg = (Utf8CP)sqlite3_errmsg(m_sqlDb);
    msg.append(" (").append(BeSQLiteLib::GetErrorName(status)).append(")");
    return msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP Db::InterpretDbResult(DbResult result)
    {
    return BeSQLiteLib::GetErrorName(result);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BlobIO::Open(DbR db, Utf8CP tableName, Utf8CP columnName, uint64_t row, bool writable, Utf8CP dbName)
    {
    return (DbResult) sqlite3_blob_open(db.GetSqlDb(), dbName ? dbName : "main", tableName, columnName, row, writable, &m_blob);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BlobIO::Close()
    {
    SqlDbBlobP blob = m_blob;
    m_blob  = 0;
    return blob ? (DbResult) sqlite3_blob_close(blob) : BE_SQLITE_OK;
    }

DbResult BlobIO::ReOpen(uint64_t row) {return (DbResult) sqlite3_blob_reopen(m_blob, row);}
DbResult BlobIO::Read(void* data, int numBytes, int offset) {return (DbResult) sqlite3_blob_read(m_blob, data, numBytes, offset);}
DbResult BlobIO::Write(const void* data, int numBytes, int offset) {return (DbResult) sqlite3_blob_write(m_blob, data, numBytes, offset);}
int BlobIO::GetNumBytes() const {return sqlite3_blob_bytes(m_blob);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromBlob::SnappyFromBlob()
    {
    m_uncompressed = nullptr;
    m_uncompressAvail = 0;

    // make m_blobData size the max we could generate from SnappyToBlob
    const uint32_t blobBufferSize = (uint32_t) snappy::MaxCompressedLength(32*1024);
    enum {uncompressedSize=SNAPPY_UNCOMPRESSED_BUFFER_SIZE};
    m_blobBufferSize = blobBufferSize;
    m_blobData = (Byte*) malloc(blobBufferSize);
    m_uncompressed = (Byte*) malloc(uncompressedSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromBlob::~SnappyFromBlob()
    {
    Finish();
    free(m_blobData);
    free(m_uncompressed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::ReadNextChunk()
    {
    BeAssert(0==m_uncompressAvail);

    if (0 == m_blobBytesLeft)
        return  ZIP_ERROR_BLOB_READ_ERROR;

    uint16_t chunkSize;
    if (BE_SQLITE_OK != m_blobIO.Read(&chunkSize, 2, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert(0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    if (chunkSize<=2 || chunkSize>m_blobBytesLeft)
        {
        m_blobBytesLeft = 0;
        BeAssert(0);
        return ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobOffset += 2;
    m_blobBytesLeft -= 2;
    chunkSize -= 2;
    BeAssert(chunkSize <= m_blobBufferSize);

    if (chunkSize > m_blobBufferSize || BE_SQLITE_OK != m_blobIO.Read(m_blobData, chunkSize, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert(0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobOffset     += chunkSize;
    m_blobBytesLeft  -= chunkSize;

    size_t uncompressSize;
    bool bstat = snappy::GetUncompressedLength((char const*) m_blobData, chunkSize, &uncompressSize);
    UNUSED_VARIABLE(bstat);
    BeAssert(uncompressSize<=SNAPPY_UNCOMPRESSED_BUFFER_SIZE && uncompressSize>0);
    BeAssert(bstat);

    bstat = snappy::RawUncompress((char const*) m_blobData, chunkSize, (char*) m_uncompressed);
    BeAssert(bstat);

    m_uncompressCurr = m_uncompressed;
    m_uncompressAvail = (uint16_t) uncompressSize;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::Init(DbCR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    m_blobOffset = 0;
    m_uncompressAvail = 0;

    if (nullptr != m_blobIO.GetBlobP())
        {
        if (BE_SQLITE_OK != m_blobIO.ReOpen(rowId))
            {
            m_blobBytesLeft = 0;
            return  ZIP_ERROR_BLOB_READ_ERROR;
            }
        }
    else if (BE_SQLITE_OK != m_blobIO.Open(const_cast<DbR>(db), tableName, column, rowId, 0))
        {
        m_blobBytesLeft = 0;
        LOG.errorv("sqlite3_open_blob failed: %s", db.GetLastError(nullptr).c_str());
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    m_blobBytesLeft = m_blobIO.GetNumBytes();
    return ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::_Read(Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead) {
    // point output pointer to their buffer
    bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != bufSize) {
        // if there's no more uncompressed data, read from the blob
        if (0 >= m_uncompressAvail) {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadNextChunk())
                return ZIP_ERROR_COMPRESSION_ERROR;
        }

        uint32_t readSize = (bufSize > m_uncompressAvail) ? m_uncompressAvail : bufSize;

        memcpy(data, m_uncompressCurr, readSize);
        data += readSize;
        m_uncompressCurr += readSize;
        bytesActuallyRead += readSize;
        bufSize -= readSize;
        m_uncompressAvail -= readSize;
    }

    return ZIP_SUCCESS;
}

/*---------------------------------------------------------------------------------**/ /**
  Seek to an offset in the chunked array.
  @note the entries are *not* the same size so this has to be done by walking through them.
  @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
int ChunkedArray::Reader::Seek(int64_t pos, bool fromBeginning) {
    if (fromBeginning) {
        m_chunkNum = 0;
        m_offset = 0;
        m_currPos = 0;
    }
    while (pos >= 0) {
        int64_t thisSize = m_array.m_chunks[m_chunkNum].size();
        if (pos < thisSize) {
            m_currPos += (pos - m_offset);
            m_offset = pos;
            return 0;
        }
        pos -= thisSize;
        m_currPos += thisSize;
        if (++m_chunkNum >= m_array.m_chunks.size())
            break;
    }
    return -1; // attempt to seek past end or negative
}

/*---------------------------------------------------------------------------------**/ /**
  @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedArray::Reader::Read(Byte* data, int* pSize) {
    int copied = 0;
    int nLeft = *pSize; // on input, this is the requested size. On output it is the actual read size.
    Byte* pOut = data;
    while (nLeft > 0) {
        auto currChunk = m_array.m_chunks.begin() + m_chunkNum;
        if (currChunk >= m_array.m_chunks.end())
            break;
        int leftInChunk = std::min((int)(currChunk->size() - m_offset), nLeft);
        if (leftInChunk > 0) {
            auto start = currChunk->begin() + m_offset;
            memcpy(pOut, &*start, leftInChunk);
            m_currPos += leftInChunk;
            nLeft -= leftInChunk;
            m_offset += leftInChunk;
            copied += leftInChunk;
            pOut += leftInChunk;
        } else {
            m_offset = 0;
            ++m_chunkNum;
        }
    }
    *pSize = copied;
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromBlob::ReadToChunkedArray(ChunkedArray& array, uint32_t bufSize) {
    array.Clear();

    // while there's still more data requested
    while (0 != bufSize) {
        // if there's no more uncompressed data, read from the blob
        if (0 >= m_uncompressAvail) {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadNextChunk()) {
                Finish();
                return ZIP_ERROR_COMPRESSION_ERROR;
            }
        }

        uint32_t readSize = (bufSize > m_uncompressAvail) ? m_uncompressAvail : bufSize;
        array.Append(m_uncompressCurr, readSize);
        m_uncompressCurr += readSize;
        bufSize -= readSize;
        m_uncompressAvail -= readSize;
    }

    Finish();
    return ZIP_SUCCESS;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromMemory::SnappyFromMemory(void*uncompressedBuffer, uint32_t uncompressedBufferSize)
    {
    BeAssert(SNAPPY_UNCOMPRESSED_BUFFER_SIZE == uncompressedBufferSize);

    m_uncompressed = (Byte*)uncompressedBuffer;
    m_uncompressAvail = 0;
    m_uncompressSize = uncompressedBufferSize;

    m_blobData = nullptr;
    m_blobOffset = 0;
    m_blobBytesLeft = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyFromMemory::Init(void*blobBuffer, uint32_t blobBufferSize)
    {
    m_uncompressAvail = 0;

    m_blobData = (Byte*) blobBuffer;
    m_blobOffset = 0;
    m_blobBytesLeft = blobBufferSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::TransferFromBlob(void* data, uint32_t numBytes, int offset)
    {
    //  Verify that it does not go beyond the end of the buffer
    memcpy(data, m_blobData + offset, numBytes);
    return ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::ReadNextChunk()
    {
    BeAssert(0==m_uncompressAvail);

    if (0 == m_blobBytesLeft)
        return  ZIP_ERROR_BLOB_READ_ERROR;

    uint16_t chunkSize;
    if (ZIP_SUCCESS != TransferFromBlob(&chunkSize, 2, m_blobOffset))
        {
        m_blobBytesLeft = 0;
        BeAssert(0);
        return  ZIP_ERROR_BLOB_READ_ERROR;
        }

    BeAssert(chunkSize <= m_blobBytesLeft);

    m_blobOffset += 2;
    m_blobBytesLeft -= 2;
    chunkSize -= 2;

    Byte*   currentData = m_blobData + m_blobOffset;

    m_blobOffset     += chunkSize;
    m_blobBytesLeft  -= chunkSize;

    size_t uncompressSize;
    bool bstat = snappy::GetUncompressedLength((char const*) currentData, chunkSize, &uncompressSize);
    UNUSED_VARIABLE(bstat);
    BeAssert(uncompressSize<=32*1024 && uncompressSize>0);
    BeAssert(bstat);

    bstat = snappy::RawUncompress((char const*) currentData, chunkSize, (char*) m_uncompressed);
    BeAssert(bstat);

    m_uncompressCurr = m_uncompressed;
    m_uncompressAvail = (uint16_t) uncompressSize;
    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors SnappyFromMemory::_Read(Byte* data, uint32_t bufSize, uint32_t& bytesActuallyRead)
    {
    // point output pointer to their buffer
    bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != bufSize)
        {
        // if there's no more uncompressed data, read from the blob
        if (0 >= m_uncompressAvail)
            {
            // get more data from the blob
            if (ZIP_SUCCESS != ReadNextChunk())
                return ZIP_ERROR_COMPRESSION_ERROR;
            }

        uint32_t readSize = (bufSize > m_uncompressAvail) ? m_uncompressAvail : bufSize;

        memcpy(data, m_uncompressCurr, readSize);
        data += readSize;
        m_uncompressCurr += readSize;
        bytesActuallyRead += readSize;
        bufSize -= readSize;
        m_uncompressAvail -= readSize;
        }

    return  ZIP_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyToBlob::SnappyToBlob()
    {
    enum {bufsize = 32*1024};
    m_rawBuf = (Byte*) malloc(bufsize);
    m_rawSize = bufsize;
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Init()
    {
    m_unsnappedSize = m_rawCurr = 0;
    m_rawAvail = m_rawSize;
    m_currChunk = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyToBlob::~SnappyToBlob()
    {
    for (SnappyChunk* chunk : m_chunks)
        delete chunk;

    m_chunks.clear();
    free(m_rawBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Write(Byte const* data, uint32_t inSize)
    {
    m_unsnappedSize += inSize;
    while (inSize>0)
        {
        uint32_t thisSize = (inSize>m_rawAvail) ? m_rawAvail : inSize;
        memcpy(m_rawBuf+m_rawCurr, data, thisSize);

        inSize -= thisSize;
        data   += thisSize;

        m_rawCurr  += thisSize;
        m_rawAvail -= thisSize;

        if (0 >= m_rawAvail)
            Finish();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SnappyToBlob::GetCompressedSize()
    {
    Finish();
    uint32_t total = 0;

    for (uint32_t i=0; i<m_currChunk; ++i)
        total += m_chunks[i]->GetChunkSize();

    return  total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnappyToBlob::Finish()
    {
    if (0 == m_rawCurr)
        return;

    if (m_chunks.size() <= m_currChunk)
        m_chunks.push_back(new SnappyChunk((uint32_t) snappy::MaxCompressedLength(m_rawSize)));

    unsigned int compressedBytes;
    snappy::RawCompress((char const*) m_rawBuf, m_rawCurr, (char*) &m_chunks[m_currChunk]->m_data[1], &compressedBytes);
    BeAssert((compressedBytes+2) < 64*1024);
    m_chunks[m_currChunk]->m_data[0] = (uint16_t) compressedBytes + 2; // add 2 because compressed data starts 2 bytes into buffer

    ++m_currChunk;
    m_rawCurr = 0;
    m_rawAvail = m_rawSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SnappyToBlob::SaveToRow(DbR db, Utf8CP tableName, Utf8CP column, int64_t rowId)
    {
    BlobIO blobIO;
    DbResult status = blobIO.Open(db, tableName, column, rowId, true);
    if (BE_SQLITE_OK != status)
        {
        BeAssert(false);
        return status;
        }

    return SaveToRow(blobIO);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SnappyToBlob::SaveToRow(BlobIO& io)
    {
    DbResult status = BE_SQLITE_ERROR;
    if (!io.IsValid())
        return status;

    Finish();

    int offset = 0;
    for (uint32_t i = 0; i< m_currChunk; ++i)
        {
        status = io.Write(m_chunks[i]->m_data, m_chunks[i]->GetChunkSize(), offset);
        if (BE_SQLITE_OK != status)
            {
            BeAssert(false);
            return status;
            }
        offset += m_chunks[i]->GetChunkSize();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeDbMutex::BeDbMutex(MutexType mutexType) {m_mux = sqlite3_mutex_alloc((int) mutexType);}
BeDbMutex::~BeDbMutex()   {sqlite3_mutex_free((sqlite3_mutex*)m_mux);}
void BeDbMutex::Enter() {sqlite3_mutex_enter((sqlite3_mutex*)m_mux);}
void BeDbMutex::Leave() {sqlite3_mutex_leave((sqlite3_mutex*)m_mux);}

#ifndef NDEBUG
bool BeDbMutex::IsHeld() {return 0!=sqlite3_mutex_held((sqlite3_mutex*)m_mux);}
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSqliteDbMutex::BeSqliteDbMutex(Db& db)
    {
    m_mux = sqlite3_db_mutex(db.GetDbFile()->GetSqlDb());
    }

void BeSqliteDbMutex::Enter() {sqlite3_mutex_enter((sqlite3_mutex*)m_mux);}
void BeSqliteDbMutex::Leave() {sqlite3_mutex_leave((sqlite3_mutex*)m_mux);}

#ifndef NDEBUG
bool BeSqliteDbMutex::IsHeld() {return 0!=sqlite3_mutex_held((sqlite3_mutex*)m_mux);}
#endif

/*---------------------------------------------------------------------------------**//**
* CachedStatements hold a reference to their cache. That is so they can use the cache's mutex for release.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatement::CachedStatement(Utf8CP sql, StatementCache const& myCache) : m_myCache(myCache), m_inCache(true)
    {
    size_t len = strlen(sql) + 1;
    m_sql = (Utf8P) sqlite3_malloc((int)len);
    memcpy(const_cast<char*>(m_sql), sql, len);
    }

CachedStatement::~CachedStatement() {sqlite3_free((void*)m_sql);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CachedStatement::Release()
    {
    // Since statements can be referenced from multiple threads, and since we want to reset the statement
    // when it is only held by the StatementCache, we need to hold the cache's mutex for the entire scope of this
    // method. However, the reference count member must still be atomic since we don't acquire the mutex for AddRef.
    BeMutexHolder holder(*m_myCache.m_mutex);

    bool inCache = m_inCache; // hold this in a local before we decrement the refcount in case another thread deletes us
    uint32_t countWas = m_refCount.DecrementAtomicPost();
    if (1 == countWas)
        {
        delete this;
        return 0;
        }

    // SharedStatements are always held in a StatementCache, so the ref count will be 1 if no
    // one else is pointing to this instance. That means that the statement is no longer in use and
    // we should reset it so sqlite won't keep it in the list of active vdbe's. Also, clear its bindings so
    // the next user won't accidentally inherit them.
    if (inCache && 2==countWas)
        {
        Reset(); // this is safe because we know this statement is now ONLY held by the StatementCache.
        ClearBindings();
        }

    return countWas-1;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatementCache::StatementCache(uint32_t size, BeMutex* inheritedMutex )
    :m_size(size), m_mutex(inheritedMutex == nullptr ? new BeMutex() : inheritedMutex), m_ownMutex(inheritedMutex == nullptr)
    {}


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatementCache::~StatementCache()
    {
    Empty();
    if (m_ownMutex)
        {
        delete m_mutex;
        }

    m_mutex = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::Empty()
    {
    BeMutexHolder _v_v(*m_mutex);

    for (auto& entry : m_entries)
        {
        entry->m_inCache = false;
        BeAssert(entry->GetRefCount() == 1); // someone is still holding a reference to this statement?
        }

    m_entries.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::Dump() const
    {
    for (auto it=m_entries.begin(); it!=m_entries.end(); ++it)
        {
        printf("%s\n", (*it)->GetSQL());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatementCache::Entries::iterator StatementCache::FindEntry(Utf8CP sql) const
    {
    for (auto it=m_entries.begin(), end=m_entries.end(); it!=end; ++it)
        {
        if (0==strcmp((*it)->GetSQL(), sql))
            {
            if (1 < (*it)->GetRefCount()) // this statement is currently in use, we can't share it
                continue;

            return  it;
            }
        }

    return  m_entries.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::AddStatement(CachedStatementPtr& newEntry, Utf8CP sql) const
    {
    BeMutexHolder _v_v(*m_mutex);

    if (m_entries.size() >= m_size) // if cache is full, remove oldest entry
        {
        m_entries.back()->m_inCache = false; // this statement is no longer managed by this cache, don't let Release method call Reset/ClearBindings anymore
        m_entries.pop_back();
        }

    newEntry = new CachedStatement(sql, *this);
    m_entries.push_front(newEntry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult StatementCache::GetPreparedStatement(CachedStatementPtr& stmt, DbFile const& dbFile, Utf8CP sqlString, bool logError) const
    {
    FindStatement(stmt, sqlString);
    if (stmt.IsValid())
        return BE_SQLITE_OK;

    AddStatement(stmt, sqlString);
    return logError ? stmt->Prepare(dbFile, sqlString) : stmt->TryPrepare(dbFile, sqlString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StatementCache::FindStatement(CachedStatementPtr& stmt, Utf8CP sql) const
    {
    BeMutexHolder _v_v(*m_mutex);

    auto entry = FindEntry(sql);
    if (entry == m_entries.end())
        {
        stmt = nullptr;
        return;
        }

    m_entries.splice(m_entries.begin(), m_entries, entry); // move this most-recently-accessed statement to front
    stmt = *entry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::GetCachedStatement(CachedStatementPtr& stmt, Utf8CP sqlString, bool logError) const
    {
    stmt = nullptr;  // just in case caller is already holding a pointer to a valid statement. Otherwise we may not share it.
    return GetStatementCache().GetPreparedStatement(stmt, *m_dbFile, sqlString, logError);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NamedParams::SetWhere(Utf8CP where)
    {
    if (nullptr == where)
        {
        m_where.clear();
        return;
        }

    m_where.assign(where);
    m_where.Trim();
    if (0 == strncmp(m_where.c_str(), "WHERE ", 6))
        m_where.erase(0,6);
    else if (0 == strncmp(m_where.c_str(), "AND ", 4))
        m_where.erase(0,4);
    }

struct StringParameter : NamedParams::SqlParameter
{
    Utf8String m_value;
    StringParameter(Utf8CP name, Utf8CP value) : SqlParameter(name), m_value(value) {}
    virtual void _Bind(Statement& stmt) {stmt.BindText(stmt.GetParameterIndex(m_name.c_str()), m_value, Statement::MakeCopy::No);}
};
struct IntegerParameter : NamedParams::SqlParameter
{
    uint64_t m_value;
    IntegerParameter(Utf8CP name, uint64_t value) : SqlParameter(name), m_value(value) {}
    virtual void _Bind(Statement& stmt) {stmt.BindInt64(stmt.GetParameterIndex(m_name.c_str()), m_value);}
};
struct DoubleParameter : NamedParams::SqlParameter
{
    double m_value;
    DoubleParameter(Utf8CP name, double value) : SqlParameter(name), m_value(value) {}
    virtual void _Bind(Statement& stmt) {stmt.BindDouble(stmt.GetParameterIndex(m_name.c_str()), m_value);}
};
struct BlobParameter : NamedParams::SqlParameter
{
    void const* m_data;
    int         m_size;
    Statement::MakeCopy m_copy;
    BlobParameter(Utf8CP name, void const* data, int size, Statement::MakeCopy copy) : SqlParameter(name), m_data(data), m_size(size), m_copy(copy){}
    virtual void _Bind(Statement& stmt) {stmt.BindBlob(stmt.GetParameterIndex(m_name.c_str()), m_data, m_size, m_copy);}
};

void NamedParams::AddStringParameter(Utf8CP name, Utf8CP val) {AddSqlParameter(new StringParameter(name, val));}
void NamedParams::AddIntegerParameter(Utf8CP name, uint64_t val) {AddSqlParameter(new IntegerParameter(name, val));}
void NamedParams::AddDoubleParameter(Utf8CP name, double val) {AddSqlParameter(new DoubleParameter(name, val));}
void NamedParams::AddBlobParameter(Utf8CP name, void const* data, int size, Statement::MakeCopy copy) {AddSqlParameter(new BlobParameter(name, data, size, copy));}
void NamedParams::Bind(Statement& stmt) const {for (auto param : m_params) param->_Bind(stmt);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::DumpSqlResults(Utf8CP sql)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(*this, sql))
        return;

    stmt.DumpResults();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbTableIterator::MakeSqlString(Utf8CP sql, bool hasWhere) const
    {
    Utf8String sqlString(sql);

    if (!m_params.GetWhere().empty())
        {
        if (0 == strncmp(m_params.GetWhere().c_str(), "ORDER BY", 8))
            sqlString.append(" ");
        else
            sqlString.append(hasWhere ? " AND " : " WHERE ");

        sqlString.append(m_params.GetWhere());
        }

    return sqlString;
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PropertyBlobInStream : ILzmaInputStream
{
private:
    Db&                 m_db;
    BeBriefcaseBasedId m_id;
    Byte*               m_buffer;
    uint32_t            m_bufferSize;
    uint32_t            m_nBytesInBuffer;
    uint32_t            m_bufferOffset;
    uint32_t            m_nextChunk;

public:
    PropertyBlobInStream(Db&db, BeBriefcaseBasedId id) : m_db(db), m_id(id), m_buffer(nullptr), m_bufferSize(0), m_nBytesInBuffer(0), m_bufferOffset(0), m_nextChunk(0) {}
    ~PropertyBlobInStream() {free(m_buffer);}

    //  The LZMA2 multithreading ensures that calls to _Read are sequential and do not overlap, so this code does not need to
    //  be concerned with preventing race conditions
    ZipErrors _Read(void* data, uint32_t size, uint32_t& actuallyRead) override
        {
        for (actuallyRead = 0; actuallyRead < size; )
            {
            if (m_nBytesInBuffer == m_bufferOffset)
                {
                m_bufferOffset = 0;
                DbResult rc = m_db.QueryPropertySize(m_nBytesInBuffer, Properties::EmbeddedFileBlob(), m_id.GetValue(), m_nextChunk);
                if (rc != BE_SQLITE_ROW)
                    return ZIP_ERROR_BAD_DATA;

                if (m_nBytesInBuffer > m_bufferSize)
                    {
                    m_bufferSize = m_nBytesInBuffer;
                    m_buffer = (Byte*)realloc(m_buffer, m_bufferSize);
                    _Analysis_assume_(m_buffer != nullptr);
                    }

                rc = m_db.QueryProperty(m_buffer, m_nBytesInBuffer, Properties::EmbeddedFileBlob(), m_id.GetValue(), m_nextChunk++);
                if (rc != BE_SQLITE_ROW)
                    return ZIP_ERROR_BAD_DATA;
                }

            uint32_t bytesToTransfer = std::min(m_nBytesInBuffer - m_bufferOffset, size - actuallyRead);
            memcpy((Byte*)data + actuallyRead, m_buffer + m_bufferOffset, bytesToTransfer);
            actuallyRead += bytesToTransfer;
            m_bufferOffset += bytesToTransfer;
            }

        return ZIP_SUCCESS;
        }

    //  This is not needed since PropertyBlobInStream is only used for decompressing.  It is the LZMA compression code that needs it.
    uint64_t _GetSize() override {BeAssert(false && "PropertyBlobInStream::_GetSize not implemented");  return 0;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  PropertyBlobOutStream : ILzmaOutputStream
{
private:
    Db&                 m_db;
    BeBriefcaseBasedId m_id;
    bvector<Byte>       m_buffer;
    uint32_t            m_chunkSize;
    uint32_t            m_bufferOffset;
    uint32_t            m_nextChunk;
    bool                m_alwaysFlush;

public:
    DbResult Flush()
        {
        if (0 == m_bufferOffset)
            return BE_SQLITE_OK;

        DbResult rc = m_db.SaveProperty(Properties::EmbeddedFileBlob(), m_buffer.data(), m_bufferOffset, m_id.GetValue(), m_nextChunk++);
        if (BE_SQLITE_OK != rc)
            LOG.errorv("PropertyBlobOutStream::Flush is returning %d", rc);
        m_bufferOffset = 0;
        return rc;
        }

    PropertyBlobOutStream(Db&db, BeBriefcaseBasedId id, uint64_t chunkSize) : m_db(db), m_id(id), m_chunkSize(static_cast <uint32_t> (chunkSize)), m_bufferOffset(0), m_nextChunk(0), m_alwaysFlush(false)
        {
        m_buffer.resize(m_chunkSize);
        }

    //  Setting m_alwaysFlush to true force PropertyBlobOutStream to create a new embedded blob for each write. Since LZMA2 calls
    //  _Write whenever it finishes processing a a block this creates a one-to-one mapping from input blocks to embedded blobs.
    //  That makes it possible to read and expand any given block, randomly accessing the blocks.
    void _SetAlwaysFlush(bool flushOnEveryWrite) override
        {
        m_alwaysFlush = flushOnEveryWrite;
        Flush();
        }

    ~PropertyBlobOutStream()
        {
        //  We should always flush and check that status, so m_bufferOffset should be 0.
        BeAssert(0 == m_bufferOffset);
        Flush();
        }

    ZipErrors _Write(void const* data, uint32_t size, uint32_t& bytesWritten) override
        {
        bytesWritten = 0;
        while (bytesWritten < size)
            {
            uint32_t bytesToTransfer = size;  //  Assume chunk size applies to input.
            if (!m_alwaysFlush)
                //  Chunk size applies to output.
                bytesToTransfer = std::min(m_chunkSize - m_bufferOffset, size - bytesWritten);

            if (bytesToTransfer > 0)
                memcpy(m_buffer.data() + m_bufferOffset, (CharCP)data + bytesWritten, bytesToTransfer);

            m_bufferOffset += bytesToTransfer;
            if (m_alwaysFlush)
                {
                if (BE_SQLITE_OK != Flush())
                    return ZIP_ERROR_WRITE_ERROR;
                }
            else if (m_bufferOffset == m_chunkSize)
                {
                if (BE_SQLITE_OK != Flush())
                    return ZIP_ERROR_WRITE_ERROR;
                }
            bytesWritten += bytesToTransfer;
            }

        return ZIP_SUCCESS;
        }
};


#define EMBEDDED_LZMA_MARKER   "EmLzma"
//=======================================================================================
// The first EmbeddedFileBlob for an embedded file starts with this structure.  If
// m_compressionType is NO_COMPRESSION then the image is not compressed.
// @bsiclass
//=======================================================================================
struct   EmbeddedLzmaHeader
{
private:
    uint16_t m_sizeOfHeader;
    char    m_idString[10];
    uint16_t m_formatVersionNumber;
    uint16_t m_compressionType;

public:
    static const int formatVersionNumber = 0x10;
    enum CompressionType : uint16_t
        {
        NO_COMPRESSION = 0,
        LZMA2 = 2
        };

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    EmbeddedLzmaHeader(CompressionType compressionType)
        {
        CharCP idString = EMBEDDED_LZMA_MARKER;
        BeAssert((strlen(idString) + 1) <= sizeof(m_idString));
        memset(this, 0, sizeof(*this));
        m_sizeOfHeader = (uint16_t)sizeof(EmbeddedLzmaHeader);
        strcpy(m_idString, idString);
        m_compressionType = compressionType;
        m_formatVersionNumber = formatVersionNumber;
        }

    int GetVersion() { return m_formatVersionNumber; }
    bool IsLzma2() { return LZMA2 == m_compressionType; }
    bool IsUncompressed() { return NO_COMPRESSION == m_compressionType; }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    bool IsValid()
        {
        if (m_sizeOfHeader != sizeof(EmbeddedLzmaHeader))
            return false;

        if (strcmp(m_idString, EMBEDDED_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2 || m_compressionType == NO_COMPRESSION;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbEmbeddedFileTable& Db::EmbeddedFiles() {return m_embeddedFiles;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void removeFileBlobs(Db& db, BeBriefcaseBasedId id)
    {
    Statement stmt;
    stmt.Prepare(db, "DELETE FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=? AND Id=?");
    stmt.BindText(1, Properties::EmbeddedFileBlob().GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(2, Properties::EmbeddedFileBlob().GetName(), Statement::MakeCopy::No);
    stmt.BindId(3, id);
    stmt.Step();
    }

//---------------------------------------------------------------------------------------
//  This is based on LZMA2_DIC_SIZE_FROM_PROP.  This is the size of the dictionary
//  they allocate when decompressing.  It makes sense for us to use same
//  boundaries as the LZMA library.
// @bsimethod
//---------------------------------------------------------------------------------------
static uint32_t getDictionarySize(uint32_t dictionarySize)
    {
    for (unsigned i = 0; i < 40; i++)
        {
        uint32_t computed = (2 | (i & 1)) << (i / 2 + 11);
        if (dictionarySize <= computed)
            return computed;
        }

    return dictionarySize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DbResult compressAndEmbedFileImage(Db& db, uint32_t& chunkSize, BeBriefcaseBasedId id, void const* data, uint32_t const size, bool supportRandomAccess)
    {
    if (supportRandomAccess)
        chunkSize = getDictionarySize(chunkSize);

    MemoryLzmaInStream inStream(data, size);
    PropertyBlobOutStream outStream(db, id, chunkSize);

    uint32_t dictionarySize = std::min(size, chunkSize);

    LzmaEncoder encoder(LzmaEncoder::LzmaParams(dictionarySize, false));

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t bytesWritten;
    outStream._Write(&header, sizeof (header), bytesWritten);
    if (bytesWritten != sizeof (header))
        return BE_SQLITE_IOERR;

    if (encoder.CompressStream(outStream, inStream) != ZIP_SUCCESS)
        return BE_SQLITE_IOERR;

    DbResult rc = outStream.Flush();

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DbResult translateZipErrorToSQLiteError(ZipErrors zipError)
    {
    switch (zipError)
        {
        case ZIP_SUCCESS:
            return BE_SQLITE_OK;
        case ZIP_ERROR_READ_ERROR:
            return BE_SQLITE_IOERR_READ;
        case ZIP_ERROR_WRITE_ERROR:
            return BE_SQLITE_IOERR_WRITE;
        case ZIP_ERROR_ABORTED:
            return BE_SQLITE_ABORT;
        case ZIP_ERROR_END_OF_DATA:
            return BE_SQLITE_IOERR_SHORT_READ;
        }

    return BE_SQLITE_ERROR;
    }

// Copied from sqlite3-1.c
#define SQLITE_PENDING_BYTE     (0x40000000)
#define SQLITE_RESERVED_BYTE    (PENDING_BYTE+1)
#define SQLITE_SHARED_FIRST     (PENDING_BYTE+2)
#define SQLITE_SHARED_SIZE      510

//---------------------------------------------------------------------------------------
// On Windows SQLite locks files by locking bytes starting at SQLITE_PENDING_BYTE (1gig)
// when there are pending writes.  We test for it here because:
//  --  it allows us to detect if the program has uncommitted changes
//  --  it allows this code to abort early instead of after processing a gig of data.
// @bsimethod
//---------------------------------------------------------------------------------------
static bool isFileLockedBySQLite(BeFile& testfile)
    {
    if (testfile.SetPointer(SQLITE_PENDING_BYTE, BeFileSeekOrigin::Begin) == BeFileStatus::Success)
        {
        uint8_t buffer[512];
        if (testfile.Read(buffer, nullptr, sizeof (buffer)) == BeFileStatus::SharingViolationError)
            {
            testfile.SetPointer(0, BeFileSeekOrigin::Begin);
            LOG.errorv("SQLite has this file locked.");
            return true;
            }
        }

    testfile.SetPointer(0, BeFileSeekOrigin::Begin);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult compressAndEmbedFile(Db& db, uint64_t& filesize, uint32_t& chunkSize, BeBriefcaseBasedId id, Utf8CP filespec, bool supportRandomAccess)
    {
    if (supportRandomAccess)
        chunkSize = getDictionarySize(chunkSize);

    BeFileLzmaInStream inStream;
    PropertyBlobOutStream outStream(db, id, chunkSize);

    WString filespecW(filespec, true);
    BeFileName filename(filespecW.c_str());

    if (inStream.OpenInputFile(filename) != BSISUCCESS)
        return BE_SQLITE_ERROR_FileNotFound;

    filesize = inStream._GetSize();

    if (isFileLockedBySQLite(inStream.GetBeFile()))
        return BE_SQLITE_BUSY;

    uint32_t dictionarySize = static_cast <uint32_t> (std::min(filesize, (uint64_t) chunkSize));

    LzmaEncoder encoder(LzmaEncoder::LzmaParams(dictionarySize, supportRandomAccess));

    if (supportRandomAccess)
        encoder.SetBlockSize(dictionarySize); // Forces LZMA to process input in chunks that correspond to block size.

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t bytesWritten;
    outStream._Write(&header, sizeof (header), bytesWritten);
    if (bytesWritten != sizeof (header))
        return BE_SQLITE_IOERR;

    ZipErrors compressResult = encoder.CompressStream(outStream, inStream);
    if (compressResult != ZIP_SUCCESS)
        {
        LOG.errorv("LzmaEncoder::Compress returned %d", compressResult);

        return translateZipErrorToSQLiteError(compressResult);
        }

    if (inStream.GetBytesRead() != filesize)
        {
        LOG.errorv("LzmaEncoder::Compress succeeded but read the wrong number of bytes: expected %lld, actual %lld", filesize, inStream.GetBytesRead());
        return BE_SQLITE_IOERR;
        }

    return outStream.Flush();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DbResult embedFileImageWithoutCompressing(Db& db, void const*data, uint64_t size, uint32_t chunkSize, BeBriefcaseBasedId id)
    {
    int32_t nChunks=0;
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::NO_COMPRESSION);
    DbResult rc = db.SaveProperty(Properties::EmbeddedFileBlob(), &header, sizeof(header), id.GetValue(), nChunks++);
    if (rc != BE_SQLITE_OK)
        return  rc;

    for (int64_t nLeft=(int64_t)size; nLeft>0; nLeft-=chunkSize)
        {
        uint32_t thisSize = (uint32_t) std::min((int64_t) chunkSize, nLeft);

        //  This relies on EmbeddedFileBlob returning PropertySpec::COMPRESS_PROPERTY_No to prevent trying
        //  to compress the blobs
        rc = db.SaveProperty(Properties::EmbeddedFileBlob(), data, thisSize, id.GetValue(), nChunks++);
        if (rc != BE_SQLITE_OK)
            return  rc;

        data  = const_cast<char*>((char const*)data) + thisSize;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DbResult embedFileWithoutCompressing(Db& db, uint64_t&filesize, uint32_t chunkSize, BeBriefcaseBasedId id, Utf8CP filespec)
    {
    WString filespecW(filespec, true);
    BeFileName filename(filespecW.c_str());
    BeFile      inputFile;

    if (inputFile.Open(filename, BeFileAccess::Read) != BeFileStatus::Success)
        return BE_SQLITE_ERROR_FileNotFound;

    if (isFileLockedBySQLite(inputFile))
        return BE_SQLITE_BUSY;

    int32_t nChunks=0;
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::NO_COMPRESSION);
    DbResult rc = db.SaveProperty(Properties::EmbeddedFileBlob(), &header, (uint32_t)sizeof(header), id.GetValue(), nChunks++);
    if (rc != BE_SQLITE_OK)
        return  rc;

    filename.GetFileSize(filesize);
    bvector<Byte>    buffer;
    buffer.resize(chunkSize);
    uint64_t    totalBytesRead = 0;
    while (true)
        {
        uint32_t bytesRead;
        inputFile.Read(&buffer[0], &bytesRead, chunkSize);
        if (0 == bytesRead)
            break;

        totalBytesRead += bytesRead;
        rc = db.SaveProperty(Properties::EmbeddedFileBlob(), &buffer[0], bytesRead, id.GetValue(), nChunks++);
        if (rc != BE_SQLITE_OK)
            return  rc;
        }

    if (totalBytesRead != filesize)
        return BE_SQLITE_IOERR_READ;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult addEmbedFile(Db& db, Utf8CP name, Utf8CP type, Utf8CP description, BeBriefcaseBasedId id, uint64_t fileSize, DateTime const* lastModified, uint32_t chunkSize)
    {
    Statement stmt;
    stmt.Prepare(db, "INSERT INTO " BEDB_TABLE_EmbeddedFile " (Name,Descr,Type,Id,Size,Chunk,LastModified) VALUES(?,?,?,?,?,?,?)");

    stmt.BindText(1, name, Statement::MakeCopy::No);
    stmt.BindText(2, description, Statement::MakeCopy::No);
    if (type)
        stmt.BindText(3, type, Statement::MakeCopy::No);
    stmt.BindId(4, id);
    stmt.BindInt64(5, fileSize);
    stmt.BindInt(6, chunkSize);
    if (lastModified != nullptr && lastModified->IsValid())
        {
        double lastModifiedJulianDay = 0.0;
        lastModified->ToJulianDay(lastModifiedJulianDay);
        stmt.BindDouble(7, lastModifiedJulianDay);
        }

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeBriefcaseBasedId DbEmbeddedFileTable::GetNextEmbedFileId() const
    {
    BeAssert(m_db.TableExists(BEDB_TABLE_EmbeddedFile));
    return BeBriefcaseBasedId(m_db, BEDB_TABLE_EmbeddedFile, "Id");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeBriefcaseBasedId DbEmbeddedFileTable::ImportDbFile(DbResult& stat, Utf8CP name, Utf8CP filespec, Utf8CP type, Utf8CP description, DateTime const* lastModified, uint32_t chunkSize, bool supportRandomAccess)
    {
    stat = isValidDbFile(filespec);
    //  We aren't going to use the VFS to read it so it better not require a VFS.
    if (BE_SQLITE_OK != stat)
        return BeBriefcaseBasedId();

    return Import(&stat, false, name, filespec, lastModified, type, description, chunkSize, supportRandomAccess);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeBriefcaseBasedId DbEmbeddedFileTable::Import(DbResult* stat, bool compress, Utf8CP name, Utf8CP localFileName, DateTime const* lastModified,
        Utf8CP typeStr, Utf8CP description, uint32_t chunkSize, bool supportRandomAccess)
    {
    BeAssert(m_db.IsTransactionActive());

    // make sure name is unique before continuing
    BeBriefcaseBasedId fileId = QueryFile(name);
    if (fileId.IsValid())
        {
        if (stat != nullptr)
            *stat = BE_SQLITE_CONSTRAINT;

        return BeBriefcaseBasedId();
        }

    BeBriefcaseBasedId newId = GetNextEmbedFileId();

    uint64_t fileSize;
    DbResult rc = compress ? compressAndEmbedFile(m_db, fileSize, chunkSize, newId, localFileName, supportRandomAccess) :
                             embedFileWithoutCompressing(m_db, fileSize, chunkSize, newId, localFileName);
    if (BE_SQLITE_OK == rc)
        rc = addEmbedFile(m_db, name, typeStr, description, newId, fileSize, lastModified, chunkSize);

    if (stat != nullptr)
        *stat = rc;

    return (rc == BE_SQLITE_OK) ? newId : BeBriefcaseBasedId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::AddEntry(Utf8CP name, Utf8CP type, Utf8CP description, DateTime const* lastModified)
    {
    return addEmbedFile(m_db, name, type, description, GetNextEmbedFileId(), 0, lastModified, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult updateEmbedFile(Db& db, BeBriefcaseBasedId id, uint64_t fileSize, DateTime const* lastModified, uint32_t chunkSize)
    {
    const bool updateLastModified = lastModified != nullptr && lastModified->IsValid();

    Statement stmt;
    Utf8CP sql = updateLastModified ? "UPDATE " BEDB_TABLE_EmbeddedFile " SET Size=?,Chunk=?,LastModified=? WHERE Id=?" : "UPDATE " BEDB_TABLE_EmbeddedFile " SET Size=?,Chunk=? WHERE Id=?";
    stmt.Prepare(db, sql);

    int parameterIndex = 1;
    stmt.BindInt64(parameterIndex, fileSize);

    parameterIndex++;
    stmt.BindInt(parameterIndex, chunkSize);

    if (updateLastModified)
        {
        parameterIndex++;
        double lastModifiedJd = 0.0;
        lastModified->ToJulianDay(lastModifiedJd);
        stmt.BindDouble(parameterIndex, lastModifiedJd);
        }

    parameterIndex++;
    stmt.BindId(parameterIndex, id);

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Replace(Utf8CP name, Utf8CP filespec, DateTime const* lastModified, uint32_t chunkSize)
    {
    BeAssert(m_db.IsTransactionActive());
    BeBriefcaseBasedId id = QueryFile(name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    bool isLzma2 = true;
    {
    PropertyBlobInStream    inStream(m_db, id);
    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;
    isLzma2 = header.IsLzma2();
    }

    removeFileBlobs(m_db, id);

    if (isLzma2)
        {
        uint64_t fileSize;
        DbResult rc = compressAndEmbedFile(m_db, fileSize, chunkSize, id, filespec, false);
        return (BE_SQLITE_OK != rc) ? rc : updateEmbedFile(m_db, id, fileSize, lastModified, chunkSize);
        }

    uint64_t fileSize;
    DbResult rc = embedFileWithoutCompressing(m_db, fileSize, chunkSize, id, filespec);
    return (BE_SQLITE_OK != rc) ? rc : updateEmbedFile(m_db, id, fileSize, lastModified, chunkSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Save(void const* data, uint64_t size, Utf8CP name, DateTime const* lastModified, bool compress, uint32_t chunkSize)
    {
    BeAssert(m_db.IsTransactionActive());
    BeBriefcaseBasedId id = QueryFile(name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    removeFileBlobs(m_db, id);

    if (nullptr != lastModified)
        {
        Statement stmt(m_db, "UPDATE " BEDB_TABLE_EmbeddedFile " SET LastModified=? WHERE Id=?");

        int parameterIndex = 1;
        double lastModifiedJd = 0.0;
        lastModified->ToJulianDay(lastModifiedJd);
        stmt.BindDouble(parameterIndex, lastModifiedJd);

        parameterIndex++;
        stmt.BindId(parameterIndex, id);

        DbResult rc = stmt.Step();
        if (BE_SQLITE_DONE != rc)
            return rc;
        }

    if (compress)
        {
        compressAndEmbedFileImage(m_db, chunkSize, id, data, (uint32_t)size, true);
        return updateEmbedFile(m_db, id, size, nullptr, chunkSize);
        }

    embedFileImageWithoutCompressing(m_db, data, size, chunkSize, id);
    return updateEmbedFile(m_db, id, size, nullptr, chunkSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Export(Utf8CP filespec, Utf8CP name, ICompressProgressTracker* progress)
    {
    BeAssert(m_db.IsTransactionActive());
    uint64_t expectedFileSize;
    uint32_t chunkSize;
    BeBriefcaseBasedId id = QueryFile(name, &expectedFileSize, nullptr, &chunkSize);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR;

    PropertyBlobInStream    inStream(m_db, id);
    BeFileLzmaOutStream     outStream;

    EmbeddedLzmaHeader  header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;

    WString filespecW(filespec, true);
    BeFileName outName(filespecW.c_str());

    if (outStream.CreateOutputFile(outName, false) != BeFileStatus::Success)
        return  BE_SQLITE_ERROR_FileExists;

    if (header.IsLzma2())
        {
        LzmaDecoder decoder;
        decoder.SetProgressTracker(progress);

        ZipErrors result = decoder.DecompressStream(outStream, inStream);
        if (ZIP_SUCCESS != result)
            {
            outName.BeDeleteFile();
            //  LZMA does not pass through any detail on write errors so we try to pull the detail from the BeFile.
            if (result == ZIP_ERROR_WRITE_ERROR && outStream.GetBeFile().GetLastError() == BeFileStatus::DiskFull)
                return BE_SQLITE_FULL;
            return  translateZipErrorToSQLiteError(result);
            }

        uint64_t  sizeOfNewFile;
        outName.GetFileSize(sizeOfNewFile);
        if (expectedFileSize != sizeOfNewFile)
            {
            LOG.errorv("DbEmbeddedFileTable::Export: exported file is not the expected size");
            outName.BeDeleteFile();
            return BE_SQLITE_IOERR_READ;
            }

        BeAssert(expectedFileSize == outStream.GetBytesWritten());

        return BE_SQLITE_OK;
        }

    bvector<Byte>   buffer;
    buffer.resize(20000);
    uint64_t totalRead = 0;
    while (true)
        {
        inStream._Read(&buffer[0], (uint32_t)buffer.size(), actuallyRead);
        if (0 == actuallyRead)
            break;
        totalRead += actuallyRead;
        uint32_t bytesWritten = 0;
        outStream._Write(&buffer[0], actuallyRead, bytesWritten);
        if (bytesWritten != actuallyRead)
            {
            if (outStream.GetBeFile().GetLastError() == BeFileStatus::DiskFull)
                return BE_SQLITE_FULL;
            return BE_SQLITE_IOERR_WRITE;
            }
        }

    //  Verify that the file size matches.
    if (totalRead != expectedFileSize)
        return BE_SQLITE_IOERR_READ;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult DbEmbeddedFileTable::ExportDbFile(Utf8CP localFileName, Utf8CP name, ICompressProgressTracker* progress)
    {
    DbResult status = Export(localFileName, name, progress);
    if (BE_SQLITE_OK != status)
        return status;

    return isValidDbFile(localFileName);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChunkedOutStream : ILzmaOutputStream {
    ChunkedArray& m_buffer;
    ChunkedOutStream(ChunkedArray& v) : m_buffer(v) {}
    virtual ~ChunkedOutStream() {}
    ZipErrors _Write(void const* data, uint32_t writeSize, uint32_t& bytesWritten) override {
        m_buffer.Append((Byte const*)data, writeSize);
        bytesWritten = writeSize;
        return ZipErrors::ZIP_SUCCESS;
    }
    void _SetAlwaysFlush(bool) override {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Read(ChunkedArray& callerBuffer, Utf8CP name) {
    BeAssert(m_db.IsTransactionActive());
    uint64_t actualSize;
    uint32_t chunkSize;
    BeBriefcaseBasedId id = QueryFile(name, &actualSize, nullptr, &chunkSize);
    if (!id.IsValid())
        return BE_SQLITE_ERROR;

    PropertyBlobInStream inStream(m_db, id);
    ChunkedOutStream outStream(callerBuffer);

    EmbeddedLzmaHeader header(EmbeddedLzmaHeader::LZMA2);
    uint32_t actuallyRead;
    inStream._Read(&header, sizeof(header), actuallyRead);
    if (actuallyRead != sizeof(header) || !header.IsValid())
        return BE_SQLITE_MISMATCH;

    if (header.IsLzma2()) {
        LzmaDecoder decoder;
        ZipErrors result = decoder.DecompressStream(outStream, inStream);
        BeAssert(callerBuffer.m_size == actualSize);
        return ZIP_SUCCESS == result ? BE_SQLITE_OK : BE_SQLITE_IOERR_READ;
    }

    char tempBuffer[2000];
    while (true) {
        inStream._Read(tempBuffer, (uint32_t)sizeof(tempBuffer), actuallyRead);
        if (0 == actuallyRead)
            break;

        uint32_t bytesWritten = 0;
        outStream._Write(&tempBuffer[0], actuallyRead, bytesWritten);
        if (bytesWritten != actuallyRead)
            return BE_SQLITE_IOERR_WRITE;
    }

    //  Verify that the file size matches.
    if (callerBuffer.m_size != actualSize) {
        BeAssert(false);
        return BE_SQLITE_IOERR_READ;
    }

    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
BeBriefcaseBasedId DbEmbeddedFileTable::QueryFile(Utf8CP name, uint64_t* size, DateTime* lastModified, uint32_t* chunkSize, Utf8StringP descr, Utf8StringP typestr)
    {
    // embedded file table is created on demand, so may not exist yet
    if (!m_db.TableExists(BEDB_TABLE_EmbeddedFile))
        return BeBriefcaseBasedId(); // invalid id indicates an error

    Statement stmt;
    stmt.Prepare(m_db, "SELECT Descr,Type,Id,Size,Chunk,LastModified FROM " BEDB_TABLE_EmbeddedFile " WHERE Name=?");
    stmt.BindText(1, name, Statement::MakeCopy::No);

    DbResult rc = stmt.Step();
    if (rc != BE_SQLITE_ROW)
        return BeBriefcaseBasedId(); // invalid id indicates an error

    if (descr)
        descr->AssignOrClear(stmt.GetValueText(0));
    if (typestr)
        typestr->AssignOrClear(stmt.GetValueText(1));
    if (size)
        *size = stmt.GetValueInt64(3);
    if (chunkSize)
        *chunkSize = stmt.GetValueInt(4);
    if (lastModified != nullptr)
        {
        if (stmt.IsColumnNull(5))
            *lastModified = DateTime();
        else
            {
            double lastModifiedJulianDay = stmt.GetValueDouble(5);
            *lastModified = DateTime::FromJulianDay(lastModifiedJulianDay, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
            }
        }

    return BeBriefcaseBasedId(stmt.GetValueUInt64(2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::Remove(Utf8CP name)
    {
    BeAssert(m_db.IsTransactionActive());
    BeBriefcaseBasedId id = QueryFile(name);
    if (!id.IsValid())
        return  BE_SQLITE_ERROR_FileNotFound;

    // this isn't really necessary for files created after we added the trigger, but do it for older dbs before we had the trigger.
    removeFileBlobs(m_db, id);

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " BEDB_TABLE_EmbeddedFile " WHERE Id=?");
    stmt.BindId(1, id);

    DbResult rc = stmt.Step();
    return (BE_SQLITE_DONE==rc) ? BE_SQLITE_OK : rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DbEmbeddedFileTable::CreateTable() const
    {
    if (m_db.TableExists(BEDB_TABLE_EmbeddedFile))
        return BE_SQLITE_OK;

    DbResult rc = m_db.CreateTable(BEDB_TABLE_EmbeddedFile,
            "Id INTEGER PRIMARY KEY,Name CHAR NOT NULL COLLATE NOCASE,Descr CHAR,Type CHAR,Size INT,Chunk INT,LastModified TIMESTAMP, CONSTRAINT nameAndType UNIQUE(Name,Type)");

    if (BE_SQLITE_OK != rc)
        return rc;

    return m_db.ExecuteSql("CREATE TRIGGER delete_embeddedFiles AFTER DELETE ON " BEDB_TABLE_EmbeddedFile
            " BEGIN DELETE FROM " BEDB_TABLE_Property
            " WHERE Namespace=\"" BEDB_PROPSPEC_NAMESPACE "\" AND NAME=\"" BEDB_PROPSPEC_EMBEDBLOB_NAME "\" AND Id=OLD.Id; END");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbEmbeddedFileTable::Iterator::Entry DbEmbeddedFileTable::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        if (!m_db->TableExists(BEDB_TABLE_EmbeddedFile))
            return Entry(nullptr, false);

        Utf8String sqlString = MakeSqlString("SELECT Name,Descr,Type,Id,Size,Chunk,LastModified FROM " BEDB_TABLE_EmbeddedFile);

        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetNameUtf8() const {return m_sql->GetValueText(0);}
Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetDescriptionUtf8() const {return m_sql->GetValueText(1);}
Utf8CP DbEmbeddedFileTable::Iterator::Entry::GetTypeUtf8() const {return m_sql->GetValueText(2);}
uint64_t DbEmbeddedFileTable::Iterator::Entry::GetFileSize() const {return m_sql->GetValueInt64(4);}
uint32_t DbEmbeddedFileTable::Iterator::Entry::GetChunkSize() const {return m_sql->GetValueInt(5);}
DateTime DbEmbeddedFileTable::Iterator::Entry::GetLastModified() const {
    return DateTime::FromJulianDay(m_sql->GetValueDouble(6), DateTime::Info::CreateForDateTime(DateTime::Kind::Utc));
}

BeBriefcaseBasedId DbEmbeddedFileTable::Iterator::Entry::GetId() const {return BeBriefcaseBasedId(m_sql->GetValueUInt64(3));}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t DbEmbeddedFileTable::Iterator::QueryCount() const
    {
    // embedded file table is created on demand, so may not exist yet
    if (!m_db->TableExists(BEDB_TABLE_EmbeddedFile))
        return 0;

    Utf8String sqlString = "SELECT COUNT(*) FROM " BEDB_TABLE_EmbeddedFile;
    if (!m_params.GetWhere().empty())
        {
        sqlString += " WHERE ";
        sqlString += m_params.GetWhere();
        }

    Statement statement;
    statement.Prepare(*m_db, sqlString.c_str());
    m_params.Bind(statement);

    return ((BE_SQLITE_ROW != statement.Step()) ? 0 : statement.GetValueInt(0));
    }

/*---------------------------------------------------------------------------------**//**
* implementation of SQL "InVirtualSet" function. Returns 1 if the value is contained in the set.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void isInVirtualSet(sqlite3_context* ctx, int nArgs, sqlite3_value** args)
    {
    if (2 > nArgs)
        {
        sqlite3_result_error(ctx, "Not enough arguments to InVirtualSet", -1);
        return;
        }

    // the first argument must be the set to test against.
    VirtualSet const* vSet = (VirtualSet const*) sqlite3_value_int64(args[0]);
    if (nullptr==vSet)
        sqlite3_result_int(ctx, 0); //0 means false -> if no virtual set is bound, we treat it as binding an empty virtual set
    else
        {
        // skip the first argument - we used it above.
        sqlite3_result_int(ctx, vSet->_IsInSet(nArgs - 1, (DbValue const*) args + 1));
        }
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we override the LOWER and UPPER scalar functions to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod
//---------------------------------------------------------------------------------------
static void caseCallback(sqlite3_context* context, int numArgs, sqlite3_value** args)
    {
    // Largely a copy of icuCaseFunc16 in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.

    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (nullptr == languageSupport)
        {
        BeAssert(false);
        return;
        }

    if (1 != numArgs)
        {
        BeAssert(false);
        return;
        }

    Utf16CP source = (Utf16CP)sqlite3_value_text16(args[0]);
    if (nullptr == source)
        return;

    int sourceSize = sqlite3_value_bytes16(args[0]);
    if (sourceSize <= 0)
        return;

    int resultSize = (2 * sourceSize) * sizeof(uint16_t);
    Utf16P result = (Utf16P)sqlite3_malloc(resultSize);
    if (nullptr == result)
        {
        BeAssert(false);
        return;
        }

    if (0 != sqlite3_user_data(context))
        languageSupport->_Upper(source, sourceSize / sizeof(uint16_t), result, resultSize / sizeof(uint16_t));
    else
        languageSupport->_Lower(source, sourceSize / sizeof(uint16_t), result, resultSize / sizeof(uint16_t));

    sqlite3_result_text16(context, result, -1, sqlite3_free);
    }

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod
//---------------------------------------------------------------------------------------
#define U8_NEXT_UNSAFE(s, i, c) \
    {\
    (c) = (uint8_t)(s)[(i)++]; \
    if ((c) >= 0x80) {\
        if ((c)<0xe0) {\
                (c) = (((c)& 0x1f) << 6) | ((s)[(i)++] & 0x3f); \
            } else if ((c)<0xf0) {\
                /* no need for (c&0xf) because the upper bits are truncated after <<12 in the cast to (UChar) */ \
                (c) = (unsigned char)(((c) << 12) | (((s)[i] & 0x3f) << 6) | ((s)[(i)+1] & 0x3f)); \
                (i) += 2; \
            } else {\
                (c) = (((c)& 7) << 18) | (((s)[i] & 0x3f) << 12) | (((s)[(i)+1] & 0x3f) << 6) | ((s)[(i)+2] & 0x3f); \
                (i) += 3; \
            } \
        } \
    }

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod
//---------------------------------------------------------------------------------------
#define U8_COUNT_TRAIL_BYTES_UNSAFE(leadByte) (((leadByte)>=0xc0)+((leadByte)>=0xe0)+((leadByte)>=0xf0))

//---------------------------------------------------------------------------------------
// Copied from utf8.h in ICU.
// Supports our LIKE operator implementation.
// @bsimethod
//---------------------------------------------------------------------------------------
#define U8_FWD_1_UNSAFE(s, i) \
    {\
    (i) += 1 + U8_COUNT_TRAIL_BYTES_UNSAFE((uint8_t)(s)[i]); \
    }

//---------------------------------------------------------------------------------------
// Actual comparison logic for our custom LIKE operator.
// @see likeCallback
// @bsimethod
//---------------------------------------------------------------------------------------
static int likeCompare(unsigned char const* patternString, unsigned char const* matchString, uint32_t escapeChar, BeSQLiteLib::ILanguageSupport* languageSupport)
    {
    // Largely a copy of icuLikeCompare in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.
    // See also patternCompare in sqlite3/src/func.c... though that also supports globs and can use other nice internal utility functions that we can't, so copying is limited.

    static const uint32_t MATCH_ONE = (uint32_t)'_';
    static const uint32_t MATCH_ALL = (uint32_t)'%';

    int iPattern = 0; // Current byte index in patternString
    int iMatch = 0; // Current byte index in matchString
    bool wasPreviousCharEscape = 0; // True if the previous character was escapeChar

    while (0 != patternString[iPattern])
        {
        // Read (and consume) the next character from the input pattern.
        uint32_t currPatternChar;
        U8_NEXT_UNSAFE(patternString, iPattern, currPatternChar);
        BeAssert(0 != currPatternChar);

        // There are now 4 possibilities:
        //  1. currPatternChar is an unescaped match-all character "%"
        //  2. currPatternChar is an unescaped match-one character "_"
        //  3. currPatternChar is an unescaped escape character
        //  4. currPatternChar is to be handled as an ordinary character

        if (!wasPreviousCharEscape && (MATCH_ALL == currPatternChar))
            {
            // Case 1.
            uint8_t peekPatternChar;

            // Skip any MATCH_ALL or MATCH_ONE characters that follow a MATCH_ALL. For each MATCH_ONE, skip one character in the test string.
            while ((MATCH_ALL == (peekPatternChar = patternString[iPattern])) || (MATCH_ONE == peekPatternChar))
                {
                if (MATCH_ONE == peekPatternChar)
                    {
                    if (0 == matchString[iMatch])
                        return 0;

                    U8_FWD_1_UNSAFE(matchString, iMatch);
                    }

                ++iPattern;
                }

            if (0 == patternString[iPattern])
                return 1;

            while (0 != matchString[iMatch])
                {
                if (likeCompare(&patternString[iPattern], &matchString[iMatch], escapeChar, languageSupport))
                    return 1;

                U8_FWD_1_UNSAFE(matchString, iMatch);
                }

            return 0;
            }
        else if (!wasPreviousCharEscape && (MATCH_ONE == currPatternChar))
            {
            // Case 2.
            if (0 == matchString[iMatch])
                return 0;

            U8_FWD_1_UNSAFE(matchString, iMatch);
            }
        else if (!wasPreviousCharEscape && (currPatternChar == escapeChar))
            {
            // Case 3.
            wasPreviousCharEscape = true;
            }
        else{
            // Case 4.
            uint32_t currMatchChar;
            U8_NEXT_UNSAFE(matchString, iMatch, currMatchChar);

            currMatchChar = languageSupport->_FoldCase(currMatchChar);
            currPatternChar = languageSupport->_FoldCase(currPatternChar);

            if (currMatchChar != currPatternChar)
                return 0;

            wasPreviousCharEscape = false;
            }
        }

    return (0 == matchString[iMatch]);
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we override the LIKE operator to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod
//---------------------------------------------------------------------------------------
static void likeCallback(sqlite3_context* context, int numArgs, sqlite3_value** args)
    {
    // Largely a copy of icuLikeFunc in sqlite3/ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.
    // See also likeFunc in sqlite3/src/func.c... though that can use other nice internal utility functions that we can't, so copying is limited.

    auto languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (nullptr == languageSupport)
        {
        BeAssert(false);
        return;
        }

    if ((numArgs < 2) || (numArgs > 3))
        {
        BeAssert(false);
        return;
        }

    auto patternString = sqlite3_value_text(args[0]);
    auto matchString = sqlite3_value_text(args[1]);

    if ((nullptr == patternString) || (nullptr == matchString))
        return;

    // Limit the length of the LIKE or GLOB pattern to avoid problems of deep recursion and N*N behavior in likeCompare.
    auto maxPatternLen = sqlite3_limit(sqlite3_context_db_handle(context), SQLITE_LIMIT_LIKE_PATTERN_LENGTH, -1);
    if (sqlite3_value_bytes(args[0]) > maxPatternLen)
        {
        BeAssert(false);
        return;
        }

    uint32_t escapeChar = 0;
    if (3 == numArgs)
        {
        // The escape character string must consist of a single UTF-8 character. Otherwise, return an error.
        auto escapeCharStr = sqlite3_value_text(args[2]);
        if (nullptr == escapeCharStr)
            {
            BeAssert(false);
            return;
            }

        auto escapeCharNumBytes = sqlite3_value_bytes(args[2]);
        int iNextChar = 0;
        U8_NEXT_UNSAFE(escapeCharStr, iNextChar, escapeChar);

        if (iNextChar != escapeCharNumBytes)
            {
            BeAssert(false);
            return;
            }
        }

    sqlite3_result_int(context, likeCompare(patternString, matchString, escapeChar, languageSupport));
    }

//---------------------------------------------------------------------------------------
// Direct sqlite callback when we add custom collations to delegate to BeSQLiteLib::ILanguageSupport.
// @bsimethod
//---------------------------------------------------------------------------------------
static int collateCallback(void* userData, int lhsSize, void const* lhs, int rhsSize, void const* rhs)
    {
    // Largely a copy of icuCollationColl in ext/icu/icu.c, but follows our coding standards and redirects the actual ICU call to the host.

    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (nullptr == languageSupport)
        {
        BeAssert(false);
        return 0;
        }

    return languageSupport->_Collate((Utf16CP)lhs, lhsSize / sizeof(uint16_t), (Utf16CP)rhs, rhsSize / sizeof(uint16_t), userData);
    }

//---------------------------------------------------------------------------------------
// Registers overrides and additions to be able to delegate language-aware string processing to BeSQLiteLib::ILanguageSupport.
// @bsimethod
//---------------------------------------------------------------------------------------
static void initLanguageSupportOnDb(sqlite3* db)
    {
    BeSQLiteLib::ILanguageSupport* languageSupport = BeSQLiteLib::GetLanguageSupport();
    if (nullptr == languageSupport)
        return;

    int rc;
    UNUSED_VARIABLE(rc);

    // The ICU sample from the sqlite folks overrides scalar functions for both SQLITE_UTF8 and SQLITE_UTF16, but only provides SQLITE_UTF8 versions for operators and collations...
    // I'm not sure why, but following their example until proven otherwise.

    rc = sqlite3_create_function_v2(db, "lower", 1, SQLITE_UTF8, (void*)0, caseCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "lower", 1, SQLITE_UTF16, (void*)0, caseCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "upper", 1, SQLITE_UTF8, (void*)1, caseCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "upper", 1, SQLITE_UTF16, (void*)1, caseCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "like", 2, SQLITE_UTF8, (void*)0, likeCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    rc = sqlite3_create_function_v2(db, "like", 3, SQLITE_UTF8, (void*)0, likeCallback, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    bvector<BeSQLiteLib::ILanguageSupport::CollationEntry> collationEntries;
    BeSQLiteLib::ILanguageSupport::CollationUserDataFreeFunc collatorFreeFunc = nullptr;
    languageSupport->_InitCollation(collationEntries, collatorFreeFunc);

    for (auto const& collationEntry : collationEntries)
        {
        rc = sqlite3_create_collation_v2(db, collationEntry.m_name.c_str(), SQLITE_UTF16, collationEntry.m_collator, collateCallback, collatorFreeFunc);
        BeAssert(BE_SQLITE_OK == rc);
        }
    }

extern "C" int sqlite3_closure_init(sqlite3* db, char** pzErrMsg, struct sqlite3_api_routines const* pApi);

/*---------------------------------------------------------------------------------**//**
* this function is called for every new database connection.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int besqlite_db_init(sqlite3* db, char** pzErrMsg, struct sqlite3_api_routines const* pApi)
    {
    // install the "closure" virtual table
    int rc = sqlite3_closure_init(db, pzErrMsg, pApi);
    UNUSED_VARIABLE(rc);
    BeAssert(BE_SQLITE_OK == rc);

    // and the "InVirtualSet" SQL function. It requires at least two arguments: the address of the VirtualSet and the value(s) to test
    rc = sqlite3_create_function_v2(db, "InVirtualSet", -1, SQLITE_UTF8, nullptr, &isInVirtualSet, nullptr, nullptr, nullptr);
    BeAssert(BE_SQLITE_OK == rc);

    // Register language-aware callbacks if necessary.
    initLanguageSupportOnDb(db);

    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void logCallback(void* pArg, int rc, Utf8CP zMsg) {
    USING_NAMESPACE_BENTLEY_LOGGING
    SEVERITY sev = LOG_ERROR;
    switch (rc & 0xff) { // mask low byte to check for error level
    case BE_SQLITE_NOTICE:
        sev = LOG_TRACE;
        break;
    case BE_SQLITE_WARNING:
    case BE_SQLITE_SCHEMA: // automatically recovered, usually not a problem
    case BE_SQLITE_ERROR:  // often expected for testing whether columns exist
        sev = LOG_WARNING;
        break;
    }
    NativeSqliteLog.messagev(sev, "rc=%d, %s", rc, zMsg);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BeSQLiteLib::EnableSharedCache(bool enabled)
    {
    return (DbResult)sqlite3_enable_shared_cache(enabled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeSQLiteLib::GetLogError(DbResult rc) {
    return SqlPrintfString("%s (%s)", BeSQLiteLib::GetErrorString(rc), BeSQLiteLib::GetErrorName(rc)).GetUtf8CP();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BeSQLiteLib::GetErrorName(DbResult code) {
    switch (code) {
        case BE_SQLITE_ERROR_FileExists:                    return "BE_SQLITE_ERROR_FileExists";
        case BE_SQLITE_ERROR_AlreadyOpen:                   return "BE_SQLITE_ERROR_AlreadyOpen";
        case BE_SQLITE_ERROR_NoPropertyTable:               return "BE_SQLITE_ERROR_NoPropertyTable";
        case BE_SQLITE_ERROR_FileNotFound:                  return "BE_SQLITE_ERROR_FileNotFound";
        case BE_SQLITE_ERROR_NoTxnActive:                   return "BE_SQLITE_ERROR_NoTxnActive";
        case BE_SQLITE_ERROR_BadDbProfile:                  return "BE_SQLITE_ERROR_BadDbProfile";
        case BE_SQLITE_ERROR_InvalidProfileVersion:         return "BE_SQLITE_ERROR_InvalidProfileVersion";
        case BE_SQLITE_ERROR_ProfileUpgradeFailed:          return "BE_SQLITE_ERROR_ProfileUpgradeFailed";
        case BE_SQLITE_ERROR_ProfileTooOld:                 return "BE_SQLITE_ERROR_ProfileTooOld";
        case BE_SQLITE_ERROR_ProfileTooNewForReadWrite:     return "BE_SQLITE_ERROR_ProfileTooNewForReadWrite";
        case BE_SQLITE_ERROR_ProfileTooNew:                 return "BE_SQLITE_ERROR_ProfileTooNew";
        case BE_SQLITE_ERROR_ChangeTrackError:              return "BE_SQLITE_ERROR_ChangeTrackError";
        case BE_SQLITE_ERROR_InvalidRevisionVersion:        return "BE_SQLITE_ERROR_InvalidRevisionVersion";
        case BE_SQLITE_ERROR_SchemaUpgradeRequired:         return "BE_SQLITE_ERROR_SchemaUpgradeRequired";
        case BE_SQLITE_ERROR_SchemaUpgradeRecommended:      return "BE_SQLITE_ERROR_SchemaUpgradeRecommended";
        case BE_SQLITE_ERROR_SchemaTooNew:                  return "BE_SQLITE_ERROR_SchemaTooNew";
        case BE_SQLITE_ERROR_SchemaTooOld:                  return "BE_SQLITE_ERROR_SchemaTooOld";
        case BE_SQLITE_ERROR_SchemaLockFailed:              return "BE_SQLITE_ERROR_ChangeTrackError";
        case BE_SQLITE_ERROR_SchemaUpgradeFailed:           return "BE_SQLITE_ERROR_SchemaUpgradeFailed";
        case BE_SQLITE_ERROR_SchemaImportFailed:            return "BE_SQLITE_ERROR_SchemaImportFailed";
        case BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes:   return "BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes";
        case BE_SQLITE_ERROR_NOTOPEN:                       return "BE_SQLITE_ERROR_NOTOPEN";
    };

    int rc = (int)code;
    const char *zName = 0;
    int i, origRc = rc;
    for(i=0; i<2 && zName==0; i++, rc &= 0xff){
        switch( rc ){
            case BE_SQLITE_OK:                 zName = "BE_SQLITE_OK";                break;
            case BE_SQLITE_ERROR:              zName = "BE_SQLITE_ERROR";             break;
            case BE_SQLITE_ERROR_SNAPSHOT:     zName = "BE_SQLITE_ERROR_SNAPSHOT";    break;
            case BE_SQLITE_INTERNAL:           zName = "BE_SQLITE_INTERNAL";          break;
            case BE_SQLITE_PERM:               zName = "BE_SQLITE_PERM";              break;
            case BE_SQLITE_ABORT:              zName = "BE_SQLITE_ABORT";             break;
            case BE_SQLITE_ABORT_ROLLBACK:     zName = "BE_SQLITE_ABORT_ROLLBACK";    break;
            case BE_SQLITE_BUSY:               zName = "BE_SQLITE_BUSY";              break;
            case BE_SQLITE_BUSY_RECOVERY:      zName = "BE_SQLITE_BUSY_RECOVERY";     break;
            case BE_SQLITE_BUSY_SNAPSHOT:      zName = "BE_SQLITE_BUSY_SNAPSHOT";     break;
            case BE_SQLITE_LOCKED:             zName = "BE_SQLITE_LOCKED";            break;
            case BE_SQLITE_LOCKED_SHAREDCACHE: zName = "BE_SQLITE_LOCKED_SHAREDCACHE";break;
            case BE_SQLITE_NOMEM:              zName = "BE_SQLITE_NOMEM";             break;
            case BE_SQLITE_READONLY:           zName = "BE_SQLITE_READONLY";          break;
            case BE_SQLITE_READONLY_RECOVERY:  zName = "BE_SQLITE_READONLY_RECOVERY"; break;
            case BE_SQLITE_READONLY_CANTINIT:  zName = "BE_SQLITE_READONLY_CANTINIT"; break;
            case BE_SQLITE_READONLY_ROLLBACK:  zName = "BE_SQLITE_READONLY_ROLLBACK"; break;
            case BE_SQLITE_READONLY_DBMOVED:   zName = "BE_SQLITE_READONLY_DBMOVED";  break;
            case BE_SQLITE_READONLY_DIRECTORY: zName = "BE_SQLITE_READONLY_DIRECTORY";break;
            case BE_SQLITE_INTERRUPT:          zName = "BE_SQLITE_INTERRUPT";         break;
            case BE_SQLITE_IOERR:              zName = "BE_SQLITE_IOERR";             break;
            case BE_SQLITE_IOERR_READ:         zName = "BE_SQLITE_IOERR_READ";        break;
            case BE_SQLITE_IOERR_SHORT_READ:   zName = "BE_SQLITE_IOERR_SHORT_READ";  break;
            case BE_SQLITE_IOERR_WRITE:        zName = "BE_SQLITE_IOERR_WRITE";       break;
            case BE_SQLITE_IOERR_FSYNC:        zName = "BE_SQLITE_IOERR_FSYNC";       break;
            case BE_SQLITE_IOERR_DIR_FSYNC:    zName = "BE_SQLITE_IOERR_DIR_FSYNC";   break;
            case BE_SQLITE_IOERR_TRUNCATE:     zName = "BE_SQLITE_IOERR_TRUNCATE";    break;
            case BE_SQLITE_IOERR_FSTAT:        zName = "BE_SQLITE_IOERR_FSTAT";       break;
            case BE_SQLITE_IOERR_UNLOCK:       zName = "BE_SQLITE_IOERR_UNLOCK";      break;
            case BE_SQLITE_IOERR_RDLOCK:       zName = "BE_SQLITE_IOERR_RDLOCK";      break;
            case BE_SQLITE_IOERR_DELETE:       zName = "BE_SQLITE_IOERR_DELETE";      break;
            case BE_SQLITE_IOERR_NOMEM:        zName = "BE_SQLITE_IOERR_NOMEM";       break;
            case BE_SQLITE_IOERR_ACCESS:       zName = "BE_SQLITE_IOERR_ACCESS";      break;
            case BE_SQLITE_IOERR_CHECKRESERVEDLOCK:
                                        zName = "BE_SQLITE_IOERR_CHECKRESERVEDLOCK"; break;
            case BE_SQLITE_IOERR_LOCK:         zName = "BE_SQLITE_IOERR_LOCK";        break;
            case BE_SQLITE_IOERR_CLOSE:        zName = "BE_SQLITE_IOERR_CLOSE";       break;
            case BE_SQLITE_IOERR_DIR_CLOSE:    zName = "BE_SQLITE_IOERR_DIR_CLOSE";   break;
            case BE_SQLITE_IOERR_SHMOPEN:      zName = "BE_SQLITE_IOERR_SHMOPEN";     break;
            case BE_SQLITE_IOERR_SHMSIZE:      zName = "BE_SQLITE_IOERR_SHMSIZE";     break;
            case BE_SQLITE_IOERR_SHMLOCK:      zName = "BE_SQLITE_IOERR_SHMLOCK";     break;
            case BE_SQLITE_IOERR_SHMMAP:       zName = "BE_SQLITE_IOERR_SHMMAP";      break;
            case BE_SQLITE_IOERR_SEEK:         zName = "BE_SQLITE_IOERR_SEEK";        break;
            case BE_SQLITE_IOERR_DELETE_NOENT: zName = "BE_SQLITE_IOERR_DELETE_NOENT";break;
            case BE_SQLITE_IOERR_MMAP:         zName = "BE_SQLITE_IOERR_MMAP";        break;
            case BE_SQLITE_IOERR_GETTEMPPATH:  zName = "BE_SQLITE_IOERR_GETTEMPPATH"; break;
            case BE_SQLITE_IOERR_CONVPATH:     zName = "BE_SQLITE_IOERR_CONVPATH";    break;
            case BE_SQLITE_CORRUPT:            zName = "BE_SQLITE_CORRUPT";           break;
            case BE_SQLITE_CORRUPT_VTAB:       zName = "BE_SQLITE_CORRUPT_VTAB";      break;
            case BE_SQLITE_NOTFOUND:           zName = "BE_SQLITE_NOTFOUND";          break;
            case BE_SQLITE_FULL:               zName = "BE_SQLITE_FULL";              break;
            case BE_SQLITE_CANTOPEN:           zName = "BE_SQLITE_CANTOPEN";          break;
            case BE_SQLITE_CANTOPEN_NOTEMPDIR: zName = "BE_SQLITE_CANTOPEN_NOTEMPDIR";break;
            case BE_SQLITE_CANTOPEN_ISDIR:     zName = "BE_SQLITE_CANTOPEN_ISDIR";    break;
            case BE_SQLITE_CANTOPEN_FULLPATH:  zName = "BE_SQLITE_CANTOPEN_FULLPATH"; break;
            case BE_SQLITE_CANTOPEN_CONVPATH:  zName = "BE_SQLITE_CANTOPEN_CONVPATH"; break;
            case BE_SQLITE_CANTOPEN_SYMLINK:   zName = "BE_SQLITE_CANTOPEN_SYMLINK";  break;
            case BE_SQLITE_PROTOCOL:           zName = "BE_SQLITE_PROTOCOL";          break;
            case BE_SQLITE_EMPTY:              zName = "BE_SQLITE_EMPTY";             break;
            case BE_SQLITE_SCHEMA:             zName = "BE_SQLITE_SCHEMA";            break;
            case BE_SQLITE_TOOBIG:             zName = "BE_SQLITE_TOOBIG";            break;
            case BE_SQLITE_CONSTRAINT:         zName = "BE_SQLITE_CONSTRAINT";        break;
            case BE_SQLITE_CONSTRAINT_UNIQUE:  zName = "BE_SQLITE_CONSTRAINT_UNIQUE"; break;
            case BE_SQLITE_CONSTRAINT_TRIGGER: zName = "BE_SQLITE_CONSTRAINT_TRIGGER";break;
            case BE_SQLITE_CONSTRAINT_FOREIGNKEY:
                                        zName = "BE_SQLITE_CONSTRAINT_FOREIGNKEY";   break;
            case BE_SQLITE_CONSTRAINT_CHECK:   zName = "BE_SQLITE_CONSTRAINT_CHECK";  break;
            case BE_SQLITE_CONSTRAINT_PRIMARYKEY:
                                        zName = "BE_SQLITE_CONSTRAINT_PRIMARYKEY";   break;
            case BE_SQLITE_CONSTRAINT_NOTNULL: zName = "BE_SQLITE_CONSTRAINT_NOTNULL";break;
            case BE_SQLITE_CONSTRAINT_COMMITHOOK:
                                        zName = "BE_SQLITE_CONSTRAINT_COMMITHOOK";   break;
            case BE_SQLITE_CONSTRAINT_VTAB:    zName = "BE_SQLITE_CONSTRAINT_VTAB";   break;
            case BE_SQLITE_CONSTRAINT_FUNCTION:
                                        zName = "BE_SQLITE_CONSTRAINT_FUNCTION";     break;
            case BE_SQLITE_CONSTRAINT_ROWID:   zName = "BE_SQLITE_CONSTRAINT_ROWID";  break;
            case BE_SQLITE_MISMATCH:           zName = "BE_SQLITE_MISMATCH";          break;
            case BE_SQLITE_MISUSE:             zName = "BE_SQLITE_MISUSE";            break;
            case BE_SQLITE_NOLFS:              zName = "BE_SQLITE_NOLFS";             break;
            case BE_SQLITE_AUTH:               zName = "BE_SQLITE_AUTH";              break;
            case BE_SQLITE_FORMAT:             zName = "BE_SQLITE_FORMAT";            break;
            case BE_SQLITE_RANGE:              zName = "BE_SQLITE_RANGE";             break;
            case BE_SQLITE_NOTADB:             zName = "BE_SQLITE_NOTADB";            break;
            case BE_SQLITE_ROW:                zName = "BE_SQLITE_ROW";               break;
            case BE_SQLITE_NOTICE:             zName = "BE_SQLITE_NOTICE";            break;
            case BE_SQLITE_NOTICE_RECOVER_WAL: zName = "BE_SQLITE_NOTICE_RECOVER_WAL";break;
            case BE_SQLITE_NOTICE_RECOVER_ROLLBACK:
                                        zName = "BE_SQLITE_NOTICE_RECOVER_ROLLBACK"; break;
            case BE_SQLITE_WARNING:            zName = "BE_SQLITE_WARNING";           break;
            case BE_SQLITE_WARNING_AUTOINDEX:  zName = "BE_SQLITE_WARNING_AUTOINDEX"; break;
            case BE_SQLITE_DONE:               zName = "BE_SQLITE_DONE";              break;
        }
    }
    if( zName==0 ){
        static char zBuf[50];
        sqlite3_snprintf(sizeof(zBuf), zBuf, "BE_SQLITE_UNKNOWN(%d)", origRc);
        zName = zBuf;
    }
    return zName;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BeSQLiteLib::GetErrorString(DbResult rc) {
    switch(rc) {
        case BE_SQLITE_ERROR_FileExists:                    return "attempt to create a new file when a file by that name already exists";
        case BE_SQLITE_ERROR_AlreadyOpen:                   return "attempt to open a BeSQLite::Db that is already in use somewhere";
        case BE_SQLITE_ERROR_NoPropertyTable:               return "attempt to open a BeSQLite::Db that doesn't have a property table";
        case BE_SQLITE_ERROR_FileNotFound:                  return "file not found";
        case BE_SQLITE_ERROR_NoTxnActive:                   return "there is no transaction active and the database was opened with AllowImplicitTransactions=false";
        case BE_SQLITE_ERROR_BadDbProfile:                  return "wrong BeSQLite profile version";
        case BE_SQLITE_ERROR_InvalidProfileVersion:         return "profile of file could not be determined.";
        case BE_SQLITE_ERROR_ProfileUpgradeFailed:          return "upgrade of profile of file failed";
        case BE_SQLITE_ERROR_ProfileTooOld:                 return "profile of file is too old";
        case BE_SQLITE_ERROR_ProfileTooNewForReadWrite:     return "profile of file is too new for read-write access";
        case BE_SQLITE_ERROR_ProfileTooNew:                 return "profile of file is too new";
        case BE_SQLITE_ERROR_ChangeTrackError:              return "attempt to commit with active changetrack";
        case BE_SQLITE_ERROR_InvalidRevisionVersion:        return "invalid version of the changeset file";
        case BE_SQLITE_ERROR_SchemaUpgradeRequired:         return "the schemas found in the database need to be upgraded";
        case BE_SQLITE_ERROR_SchemaUpgradeRecommended:      return "recommended that the schemas found in the database be upgraded";
        case BE_SQLITE_ERROR_SchemaTooNew:                  return "the schemas found in the database are too new, upgrade application";
        case BE_SQLITE_ERROR_SchemaTooOld:                  return "the schema found in the database are too old";
        case BE_SQLITE_ERROR_SchemaLockFailed:              return "failed to acquire schema lock for upgrade";
        case BE_SQLITE_ERROR_SchemaUpgradeFailed:           return "failed to upgrade schemas";
        case BE_SQLITE_ERROR_SchemaImportFailed:            return "failed to import schemas";
        case BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes:   return "error acquiring locks or codes";
        case BE_SQLITE_ERROR_NOTOPEN:                       return "db not open";
    };
    return sqlite3_errstr(rc);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BeSQLiteLib::GetMemoryUsed(int64_t& current, int64_t& high, bool reset)
    {
    return (DbResult)sqlite3_status64(SQLITE_STATUS_MEMORY_USED, (sqlite3_int64*)&current, (sqlite3_int64*)&high, reset?1:0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BeSQLiteLib::Initialize(BeFileNameCR tempDir, LogErrors logErrors)
    {
    static bool s_done = false;
    RUNONCE_CHECK(s_done,BE_SQLITE_OK);

#ifndef NO_LOG_CALLBACK
    if (LogErrors::No != logErrors)
        sqlite3_config(SQLITE_CONFIG_LOG, logCallback, nullptr);
#endif
    sqlite3_initialize();
    sqlite3_auto_extension((void(*)(void))&besqlite_db_init);

    Utf8String tempDirUtf8 = tempDir.GetNameUtf8();
    if (!tempDir.DoesPathExist())
        {
        LOG.errorv("BeSQLiteLib::Initialize failed: Temporary directory '%s' does not exist", tempDirUtf8.c_str());
        BeAssert(false && "Error in BeSQLiteLib::Initialize: Temp dir does not exist!");
        return BE_SQLITE_CANTOPEN_NOTEMPDIR;
        }

    sqlite3_temp_directory = (char*) sqlite3_malloc((int) (tempDirUtf8.size()) + 1);
PUSH_DISABLE_DEPRECATION_WARNINGS
    strcpy(sqlite3_temp_directory, tempDirUtf8.c_str());
POP_DISABLE_DEPRECATION_WARNINGS

    //Set streaming chuck size to 64KiB
    const int streamChuckSize = 64 * 1024;
    sqlite3session_config (SQLITE_SESSION_CONFIG_STRMSIZE, (void*)&streamChuckSize);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BeSQLiteLib::CloseSqlDb(void* p) {return sqlite3_close((sqlite3*) p);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSet::FromReadableString(Utf8StringCR str)
    {
    BeAssert(!str.empty()); // checked in FromString()...

    Utf8CP curr = str.c_str();
    while (true)
        {
        uint64_t startRange, endRange;
        int converted = Utf8String::Sscanf_safe(curr, "%" SCNu64 "-%" SCNu64, &startRange, &endRange);
        if (0 == converted)
            return;

        if (converted < 2)
            endRange = startRange;
        else if (endRange < startRange)
            std::swap(startRange, endRange);

        for (; startRange<=endRange; ++startRange)
            insert((BeInt64Id) startRange);

        curr = strchr(curr, ',');
        if (curr == nullptr)
            return;

        ++curr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isHexDigit(Utf8Char ch)
    {
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F');
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t convertChar(Utf8Char ch)
    {
    return ch >= 'A' ? (ch - 'A' + 10) : (ch - '0');
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t parseUInt64(Utf8String::const_iterator& iter, Utf8String::const_iterator end)
    {
    // NB: 0 is not a valid return value - we will never encounter it
    uint64_t value = 0;
    while (iter < end)
        {
        Utf8Char ch = *iter;
        if (!isHexDigit(ch))
            break;

        value <<= 4;
        value |= convertChar(ch);
        ++iter;
        }

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t insertRange(BeIdSet& ids, uint64_t value, uint64_t increment, uint64_t mult)
    {
    for (uint64_t i = 0; i < mult; i++)
        {
        value += increment;
        ids.insert(BeInt64Id(static_cast<int64_t>(value)));
        }

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSet::FromCompactString(Utf8StringCR str)
    {
    auto iter = str.begin();
    auto strEnd = str.end();
    BeAssert(iter != strEnd && '+' == *iter);    // verified in FromString()...

    uint64_t prevValue = 0;

    ++iter;
    while (iter < strEnd)
        {
        uint64_t mult = 1;
        uint64_t increment = parseUInt64(iter, strEnd);
        BeAssert(0 != increment);
        if (0 == increment)
            return;

        if (iter != strEnd)
            {
            switch (*iter++)
                {
                case '*':
                    mult = parseUInt64(iter, strEnd);
                    BeAssert(0 != mult);
                    if (0 == mult)
                        return;
                    else if (iter != strEnd && *(iter++) != '+')
                        return;

                    break;
                case '+':
                    break;
                default:
                    BeAssert(false);
                    return;
                }
            }

        prevValue = insertRange(*this, prevValue, increment, mult);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSet::FromString(Utf8StringCR str)
    {
    if (str.empty())
        return;

    if ('+' == *str.begin())
        return FromCompactString(str);

    return FromReadableString(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void saveRange(bool& valid, Utf8StringR str, int64_t start, int64_t end)
    {
    if (!valid)
        return;

    valid = false;
    if (!str.empty())
        str.append(",");

    char tmp[100];
    if (start < end)
        BeStringUtilities::Snprintf(tmp, _countof(tmp), "%lld-%lld", start, end);
    else
        BeStringUtilities::Snprintf(tmp, _countof(tmp), "%lld", start);

    str.append(tmp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSet::SaveCompactRange(Utf8StringR str, int64_t increment, int64_t len)
    {
    BeAssert(0 != len);
    Utf8Char buf[0x12] = "+";
    BeStringUtilities::FormatUInt64(buf+1, _countof(buf)-1, static_cast<uint64_t>(increment), HexFormatOptions::Uppercase);
    buf[0x11] = 0; // FormatUInt64 takes care of null terminator, but static analyzer doesn't know that.
    str.append(buf);
    if (1 < len)
        {
        *buf = '*';
        BeStringUtilities::FormatUInt64(buf+1, _countof(buf)-1, static_cast<uint64_t>(len), HexFormatOptions::Uppercase);
        str.append(buf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* convert an IdSet to a Json string. This looks for ranges of contiguous values and uses "n-m" syntax.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeIdSet::ToReadableString() const
    {
    Utf8String str;
    int64_t start=0, end=0;
    bool valid=false;
    for (auto val : *this)
        {
        if (!val.IsValid())
            continue;

        int64_t curr = val.GetValue();
        if (valid &&(curr == end+1))
            {
            end = curr;
            continue;
            }

        saveRange(valid, str, start, end);
        valid = true;
        start = end = curr;
        }

    saveRange(valid, str, start, end);
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeIdSet::ToString() const
    {
    return ToReadableString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Db::SaveCreationDate()
    {
    SavePropertyString(Properties::BeSQLiteBuild(), REL_V "." MAJ_V "." MIN_V "." SUBMIN_V);
    return SavePropertyString(Properties::CreationDate(), DateTime::GetCurrentTimeUtc().ToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Db::QueryCreationDate(DateTime& creationDate) const
    {
    Utf8String date;
    DbResult rc = QueryProperty(date, Properties::CreationDate());
    if (BE_SQLITE_ROW != rc)
        return rc;

    creationDate = DateTime::FromString(date.c_str());
    return creationDate.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_MISMATCH;
    }

#define BE_LOCAL_StandaloneEdit "StandaloneEdit"
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Db::QueryStandaloneEditFlags(BeJsValue out) const {
    Utf8String val;
    if (BE_SQLITE_ROW == QueryBriefcaseLocalValue(val, BE_LOCAL_StandaloneEdit))
        out.From(BeJsDocument(val));
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::SaveStandaloneEditFlags(BeJsConst val) {
    return val.isNull() ? DeleteBriefcaseLocalValue(BE_LOCAL_StandaloneEdit) : SaveBriefcaseLocalValue(BE_LOCAL_StandaloneEdit, val.Stringify());
}

void BeSQLiteLib::Randomness(int numBytes, void* random) {sqlite3_randomness(numBytes, random);}
void* BeSQLiteLib::MallocMem(int sz) {return sqlite3_malloc(sz);}
void* BeSQLiteLib::ReallocMem(void* p, int sz) {return sqlite3_realloc(p,sz);}
void BeSQLiteLib::FreeMem(void* p) {sqlite3_free(p);}

BeSQLiteLib::ILanguageSupport* s_languageSupport;
void BeSQLiteLib::SetLanguageSupport(ILanguageSupport* value) {s_languageSupport = value;}
BeSQLiteLib::ILanguageSupport* BeSQLiteLib::GetLanguageSupport() {return s_languageSupport;}

#define DB_LZMA_MARKER   "LzmaDgnDb"

//=======================================================================================
//  A compressed Bim file starts with DbLzmaHeader.  This is not the same as an
//  EmbeddedLzmaHeader.  A EmbeddedLzmaHeader is used for any type of embedded file.
//  If a compressed Bim file is stored as an embedded file, it also gets a
//  EmbeddedLzmaHeader as part of the embedded stream.
// @bsiclass
//=======================================================================================
struct DbLzmaHeader
{
private:
    uint16_t        m_sizeOfHeader;
    char            m_idString[10];
    uint16_t        m_formatVersionNumber;
    uint16_t        m_compressionType;
    uint64_t        m_sourceSize;

public:
    static const int formatVersionNumber = 0x10;
    enum CompressionType
        {
        LZMA2 = 2
        };

    DbLzmaHeader(CharCP idString, CompressionType compressionType, uint64_t sourceSize)
        {
        BeAssert((strlen(idString) + 1) <= sizeof(m_idString));
        memset(this, 0, sizeof(*this));
        m_sizeOfHeader = (uint16_t)sizeof(DbLzmaHeader);
        strcpy(m_idString, idString);
        m_compressionType = compressionType;
        m_formatVersionNumber = formatVersionNumber;
        m_sourceSize = sourceSize;
        }

    DbLzmaHeader()
        {
        memset(this, 0, sizeof(*this));
        }

    int GetVersion() { return m_formatVersionNumber; }
    bool IsLzma2() { return true; }
    bool IsValid()
        {
        if (strcmp(m_idString, DB_LZMA_MARKER))
            return false;

        if (formatVersionNumber != m_formatVersionNumber)
            return false;

        return m_compressionType == LZMA2;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ZipErrors LzmaUtility::CompressDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progressTracker, uint32_t dictionarySize, bool supportRandomAccess)
    {
    if (!BeFileName::DoesPathExist(sourceFile.GetName()))
        return ZIP_ERROR_FILE_DOES_NOT_EXIST;

    BeFileLzmaInStream  inStream;
    if (BSISUCCESS != inStream.OpenInputFile(sourceFile))
        return ZIP_ERROR_CANNOT_OPEN_INPUT;

    BeFileLzmaOutStream outStream;
    if (BeFileStatus::Success != outStream.CreateOutputFile(targetFile))
        return ZIP_ERROR_CANNOT_OPEN_OUTPUT;

    DbLzmaHeader  dbLzmaHeader(DB_LZMA_MARKER, DbLzmaHeader::LZMA2, inStream._GetSize());
    uint32_t bytesWritten;
    if (outStream._Write(&dbLzmaHeader, sizeof(dbLzmaHeader), bytesWritten) != ZIP_SUCCESS)
        return ZIP_ERROR_WRITE_ERROR;

    LzmaEncoder encoder(LzmaEncoder::LzmaParams(dictionarySize, supportRandomAccess));
    encoder.SetProgressTracker(progressTracker);
    return encoder.CompressStream(outStream, inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ZipErrors LzmaUtility::DecompressDb(BeFileNameCR targetFile, BeFileNameCR sourceFile, ICompressProgressTracker* progress)
    {
    BeFileLzmaInStream  inStream;
    if (inStream.OpenInputFile(sourceFile) != BSISUCCESS)
        return ZIP_ERROR_CANNOT_OPEN_INPUT;

    BeFileLzmaOutStream outStream;
    if (outStream.CreateOutputFile(targetFile) != BeFileStatus::Success)
        return ZIP_ERROR_CANNOT_OPEN_OUTPUT;

    //  Advance past header
    DbLzmaHeader     lzmaDbHeader;

    uint32_t readSize = sizeof(lzmaDbHeader);
    inStream._Read(&lzmaDbHeader, readSize, readSize);
    if (readSize != sizeof(lzmaDbHeader) || !lzmaDbHeader.IsValid())
        return ZIP_ERROR_BAD_DATA;

    BeAssert(lzmaDbHeader.IsLzma2());

    LzmaDecoder decoder;
    decoder.SetProgressTracker(progress);
    return decoder.DecompressStream(outStream, inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ZipErrors LzmaUtility::DecompressEmbeddedBlob(bvector<Byte>&out, uint32_t expectedSize, void const*inputBuffer, uint32_t inputSize, Byte*header, uint32_t headerSize)
    {
    MemoryLzmaInStream  inStream(inputBuffer, inputSize);

    if (headerSize != sizeof(EmbeddedLzmaHeader) + 1)
        return ZIP_ERROR_BAD_DATA;

    EmbeddedLzmaHeader  dgndbHeader(EmbeddedLzmaHeader::LZMA2);
    memcpy(&dgndbHeader, header, sizeof(EmbeddedLzmaHeader));
    if (!dgndbHeader.IsValid())
        return ZIP_ERROR_BAD_DATA;

    inStream.SetHeaderData(header + sizeof(EmbeddedLzmaHeader), 1);

    MemoryLzmaOutStream outStream(out);

    LzmaDecoder decoder;
    return decoder.DecompressStream(outStream, inStream);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int integrityCheckCallback(void* callbackArg, int numColumns, CharP* columnValues, CharP* columnNames)
    {
    bool& anyFailures = *(bool*)callbackArg;

    // We only care if any failures were ever encountered.
    if (anyFailures)
        return 0;

    // According to docs, if the checks pass, a single row will be returned with a single column with the text "ok".
    // Treat anything else as a failure.

    if ((1 != numColumns) || (nullptr == columnValues[0]) || (0 != strcmp("ok", columnValues[0])))
        anyFailures = true;

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Db::CheckDbIntegrity(BeFileNameCR dbFileName)
    {
    Utf8String dbName(dbFileName.c_str());
    SqlDbP sqlDb;
    DbResult rc = (DbResult)sqlite3_open_v2(dbName.c_str(), &sqlDb, (int)Db::OpenMode::Readonly, nullptr);
    if (BE_SQLITE_OK != rc)
        return rc;

    // Pass '1' as the maximum number of failures to detect... we don't currently need to know details, just whether /anything/ is wrong.
    bool anyFailures = false;
    rc = (DbResult)sqlite3_exec(sqlDb, "PRAGMA integrity_check(1)", integrityCheckCallback, &anyFailures, nullptr);

    DbResult closeRc = (DbResult)sqlite3_close(sqlDb);
    if (BE_SQLITE_OK != closeRc)
        {
        BeAssert(false);
        return closeRc;
        }

    if ((BE_SQLITE_OK != rc) || anyFailures)
        return BE_SQLITE_CORRUPT;

    return BE_SQLITE_OK;
    }

/** vacuum this database. */
DbResult Db::Vacuum(int newPageSizeInBytes) {
    SuspendDefaultTxn noDefault(*this);
    if (newPageSizeInBytes > 0) {
        if (newPageSizeInBytes % 2 != 0 && newPageSizeInBytes < 512 && newPageSizeInBytes > 64 * 1024)
            return BE_SQLITE_ERROR;
        auto rc = TryExecuteSql(SqlPrintfString("pragma page_size=%" PRId32, newPageSizeInBytes));
        if (rc != BE_SQLITE_OK)
            return rc;
    }

    return TryExecuteSql("vacuum");
}

/** vacuum this database into a different destination file. */
DbResult Db::VacuumInto(Utf8CP newFileName) {
    SuspendDefaultTxn noDefault(*this);
    return TryExecuteSql(SqlPrintfString("vacuum into '%s'", newFileName));
}

/**
 * Commit and then re-start current Txn. This may only happen when no nested transactions are active.
 * It is useful for readonly connections to load changes made by other connections.
 */
DbResult Db::RestartDefaultTxn() {
    if (GetCurrentSavepointDepth() != 1) {
        LOG.error("Cannot restart default txn from within nested transaction");
        return BE_SQLITE_ERROR;
    }
    auto savepoint = GetSavepoint(0);
    if (savepoint) {
        savepoint->Commit();
        savepoint->Begin();
    }

    return BE_SQLITE_OK;
}

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ThreadLocalSnappyFromMemory
{
    Byte m_buffer[SnappyReader::SNAPPY_UNCOMPRESSED_BUFFER_SIZE];
    SnappyFromMemory m_snappy;

    ThreadLocalSnappyFromMemory() : m_snappy(m_buffer, _countof(m_buffer)) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void deleteTlsSnappy(void* obj)
    {
    auto snappy = reinterpret_cast<T*>(obj);
    delete snappy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static T& getTlsSnappy(BeThreadLocalStorage& tls)
    {
    auto obj = reinterpret_cast<T*>(tls.GetValueAsPointer());
    if (nullptr == obj)
        tls.SetValueAsPointer(obj = new T());

    return *obj;
    }

static BeThreadLocalStorage s_tlsSnappyFromMemory(deleteTlsSnappy<ThreadLocalSnappyFromMemory>);
static BeThreadLocalStorage s_tlsSnappyFromBlob(deleteTlsSnappy<SnappyFromBlob>);

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromMemory& SnappyFromMemory::GetForThread()
    {
    return getTlsSnappy<ThreadLocalSnappyFromMemory>(s_tlsSnappyFromMemory).m_snappy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnappyFromBlob& SnappyFromBlob::GetForThread()
    {
    return getTlsSnappy<SnappyFromBlob>(s_tlsSnappyFromBlob);
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DbBuffer::SqliteMSize(SqlMemP buff) {
    if (buff == 0) {
        return 0;
    }
    return sqlite3_msize(buff);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::SqliteFree(SqlMemP buff) {
    if (buff != 0) {
        sqlite3_free(buff);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::SqlMemP DbBuffer::SqliteAlloc(uint64_t size){
    SqlMemP buff = sqlite3_malloc64(size);
    if (buff == nullptr) {
        throw std::bad_alloc();
    }
    return buff;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::SqlMemP DbBuffer::SqliteRealloc(SqlMemP buff, uint64_t newSize) {
    SqlMemP relBuff = sqlite3_realloc64(buff, newSize);
    if (relBuff == nullptr) {
        throw std::bad_alloc();
    }
    return relBuff;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Realloc(uint64_t size) {
    m_buff = DbBuffer::SqliteRealloc(m_buff, size);
    m_size = size;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::TakeOwnership(SqlMemP buff, uint64_t size ){
    if (buff == m_buff &&  size == m_size) {
        return;
    }
    Free();
    m_buff = buff;
    m_size = size;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Alloc(uint64_t size) {
    Free();
    m_buff = DbBuffer::SqliteAlloc(size);
    m_size = size;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::EnsureCapacity(uint64_t requiredSize) {
    if (requiredSize < Capacity() && requiredSize > 0) {
        Realloc(requiredSize);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Zero() {
    if (m_buff != nullptr) {
        memset(m_buff, 0, m_size);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::MoveFrom(DbBuffer& from) {
    if (&from == this) {
        return;
    }
    EnsureCapacity(from.m_size);
    m_buff = std::move(from.m_buff);
    m_size = std::move(from.m_size);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DbBuffer::Capacity() const  {
    return  DbBuffer::SqliteMSize(m_buff);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::CopyFrom(SqlMemCP fromBuff, uint64_t fromSize) {
    if (m_buff == fromBuff && m_size == fromSize) {
        return;
    }
    EnsureCapacity(fromSize);
    memcpy(m_buff, fromBuff, fromSize);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Free() {
    DbBuffer::SqliteFree(m_buff);
    m_buff = nullptr;
    m_size = 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer Db::Serialize(const char *zSchema ) const {
    if (!IsDbOpen()) {
        return DbBuffer();
    }
    sqlite3_int64 spiSize;
    auto buff = sqlite3_serialize(GetSqlDb(), zSchema,  &spiSize, 0);
    return DbBuffer((DbBuffer::SqlMemP)buff, spiSize);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::pair<DbBuffer::SqlMemP, uint64_t>  DbBuffer::Detach() {
    auto pair = std::make_pair(m_buff, m_size);
    m_buff = nullptr;
    m_size = 0;
    return pair;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::Deserialize(DbBuffer& buffer, DbR db, DbDeserializeOptions opts, const char *zSchema , std::function<void(DbR)> beforeDefaultTxnStarts) {
    if (db.IsDbOpen()) {
        return BE_SQLITE_ERROR;
    }
    Db::CreateParams param;
    param.m_skipFileCheck = true;
    auto rc = db.CreateNewDb(BEDB_MemoryDb, param);
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    rc = db.SaveChanges();
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    db.m_statements.Empty();
    if (db.m_dbFile->m_defaultTxn.IsActive()) {
        db.m_dbFile->m_defaultTxn.Commit(nullptr);
    }

    rc = (DbResult)sqlite3_deserialize(
        db.GetSqlDb(),
        zSchema,
        (unsigned char*)buffer.DataP(),
        buffer.Size(),
        buffer.Capacity(),static_cast<unsigned int>(opts));

    if (rc == BE_SQLITE_OK && (opts & DbDeserializeOptions::FreeOnClose) == DbDeserializeOptions::FreeOnClose) {
        buffer.Detach();
    }
    if (rc == BE_SQLITE_OK && db.m_dbFile->m_defaultTxn.GetTxnMode() != BeSQLiteTxnMode::None) {
        if (beforeDefaultTxnStarts != nullptr) {
            beforeDefaultTxnStarts(db);
        }
        rc= db.m_dbFile->m_defaultTxn.Begin();
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return rc;
}

