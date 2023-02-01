/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTests.h"
#include <numeric>
#include <algorithm>
#include <cctype>
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct Sync {
private:
    static DbResult GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
        Statement stmt;
        auto rc = stmt.Prepare(db, SqlPrintfString("pragma %s.pragma_table_info(?)", dbAlias).GetUtf8CP());
        if (BE_SQLITE_OK != rc)
            return rc;

        stmt.BindText(1, tableName, Statement::MakeCopy::No);
        while((rc = stmt.Step()) == BE_SQLITE_ROW) {
            columnNames.push_back(stmt.GetValueText(1));
        }
        return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
    }
    static DbResult GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames) {
        Statement stmt;
        auto rc = stmt.Prepare(db, SqlPrintfString("pragma %s.pragma_table_info(?)", dbAlias).GetUtf8CP());
        if (BE_SQLITE_OK != rc)
            return rc;

        stmt.BindText(1, tableName, Statement::MakeCopy::No);
        while((rc = stmt.Step()) == BE_SQLITE_OK) {
            if (stmt.GetValueInt(5) != 0) {
                columnNames.push_back(stmt.GetValueText(1));
            }
        }
        return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
    }
    static std::string Join(std::vector<std::string> const& list, std::string delimiter = ",", std::string prefix = "", std::string postfix = "") {
        return prefix + std::accumulate(
            std::next(list.begin()),
            std::end(list),
            std::string{list.front()},
            [&](std::string const& acc, const std::string& piece) {
                return acc + delimiter + piece;
            }
        ) + postfix;
    }
    static std::string ToLower(std::string const& val) {
		std::string out;
		std::for_each(val.begin(), val.end(), [&](char const& ch) {
			out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
		});
		return out;
    }

    static DbResult InsertOrUpdate(DbR conn, std::vector<std::string> const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main" ) {
        auto rc = conn.ExecuteSql("PRAGMA defer_foreign_keys=1");
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        for (auto& tbl : tables) {
            rc = InsertOrUpdate(conn, tbl.c_str(), sourceDbAlias, targetDbAlias);
            if (rc != BE_SQLITE_OK) {
                conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
                return rc;
            }
        }
        return conn.ExecuteSql("PRAGMA defer_foreign_keys=0");
    }
    static DbResult InsertOrUpdate(DbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main") {
        DbResult rc;
        auto sourceCols = std::vector<std::string>{};
        rc = GetColumnNames(conn, sourceDbAlias, tableName, sourceCols);
        if (BE_SQLITE_OK != rc)
            return rc;

        auto sourcePkCols = std::vector<std::string>{};
        rc = GetPrimaryKeyColumnNames(conn, sourceDbAlias, tableName, sourcePkCols);
        if (BE_SQLITE_OK != rc)
            return rc;

        auto targetCols = std::vector<std::string>{};
        rc = GetColumnNames(conn, targetDbAlias, tableName, targetCols);
        if (BE_SQLITE_OK != rc)
            return rc;

        auto targetPkCols = std::vector<std::string>{};
        rc = GetPrimaryKeyColumnNames(conn, targetDbAlias, tableName, targetPkCols);
        if (BE_SQLITE_OK != rc)
            return rc;

        std::sort(std::begin(sourceCols), std::end(sourceCols));
        std::sort(std::begin(sourcePkCols), std::end(sourcePkCols));
        std::sort(std::begin(targetCols), std::end(targetCols));
        std::sort(std::begin(targetPkCols), std::end(targetPkCols));

        const auto sourceColCount = sourceCols.size();
        const auto sourcePkColCount = sourcePkCols.size();

        if(sourceColCount != targetCols.size()) {
            return BE_SQLITE_SCHEMA;
        }
        if(sourcePkColCount != targetPkCols.size()) {
            return BE_SQLITE_SCHEMA;
        }
        for (auto i = 0; i < sourceColCount; ++i) {
            if (ToLower(sourceCols[i]) != ToLower(targetCols[i]))
                return BE_SQLITE_SCHEMA;
        }
        for (auto i = 0; i < sourcePkColCount; ++i) {
            if (ToLower(sourcePkCols[i]) != ToLower(targetPkCols[i]))
                return BE_SQLITE_SCHEMA;
        }

        const auto sourceTableSql = std::string {SqlPrintfString("[%s].[%s]", sourceDbAlias, tableName).GetUtf8CP()};
        const auto targetTableSql = std::string {SqlPrintfString("[%s].[%s]", targetDbAlias, tableName).GetUtf8CP()};
        const auto sourceColsSql = Join(sourceCols);
        const auto targetColsSql = Join(targetCols);
        const auto updateColsSql = Join(targetCols,"=excluded.");
        const auto targetPkColsSql = Join(targetPkCols);

        const auto sql = std::string{SqlPrintfString(
            "insert into %s(%s) on conflict(%s) do update set %s select %s from %s",
            targetTableSql.c_str(),
            targetColsSql.c_str(),
            targetPkColsSql.c_str(),
            updateColsSql.c_str(),
            sourceColsSql.c_str(),
            sourceTableSql.c_str()
        ).GetUtf8CP()};
    }
public:
    static DbResult InsertOrUpdate(Utf8CP sourceDb, Utf8CP targetDb, std::function<bool(const Utf8String&)> tableFilter,bool createMissingTables = false, bool useNestedSafePoint = false) {
        const auto kSourceDbAlias = "source_db";
        BeFileName sourceDbFile(sourceDb);
        BeFileName targetDbFile(targetDb);
        if (!sourceDbFile.DoesPathExist()) {
            return BE_SQLITE_ERROR;
        }
        if (!targetDbFile.DoesPathExist()) {
            return BE_SQLITE_ERROR;
        }
        if (sourceDbFile.EqualsI(targetDbFile)) {
            return BE_SQLITE_ERROR;
        }

        Db target;
        auto rc = target.OpenBeSQLiteDb(targetDbFile, Db::OpenParams(Db::OpenMode::ReadWrite));
        if (rc != BE_SQLITE_OK) {
            return rc;
        }

        rc = target.AttachDb(sourceDbFile.GetNameUtf8().c_str(), kSourceDbAlias);
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        std::unique_ptr<Savepoint> savePoint;
        if (useNestedSafePoint) {
            savePoint = std::make_unique<Savepoint>(target, "sync-db");
        }

        if (createMissingTables) {
            const auto enumMissingTblSql = std::string{
                SqlPrintfString(R"sql(
                    SELECT
                        [source].[sql]
                    FROM   [%s].[sqlite_master] [source]
                        LEFT JOIN [%s].[sqlite_master] [target] ON [target].[name] = [source].[name]
                                AND [target].[tbl_name] = [source].[tbl_name]
                                AND [target].[type] = [source].[type]
                    WHERE  [source].[sql] IS NOT NULL
            )sql", kSourceDbAlias, "main").GetUtf8CP()};

            Statement stmt;
            rc = stmt.Prepare(target, enumMissingTblSql.c_str());
            if (rc != BE_SQLITE_OK) {
                return rc;
            }
            while(stmt.Step() != BE_SQLITE_ROW) {
                rc = target.ExecuteSql(stmt.GetValueText(0));
                if (rc != BE_SQLITE_OK) {
                    return rc;
                }
            }
        }
        const auto enumTblSql = std::string{
            SqlPrintfString(R"sql(
                SELECT [target].[tbl_name]
                FROM   [%s].[sqlite_master] [target], [%s].[sqlite_master] [source]
                WHERE  [target].[type] = 'table'
                        AND [target].[rootpage] != 0
                        AND [target].[tbl_name] NOT LIKE 'sqlite_%'
                        AND [source].[type] = 'table'
                        AND [source].[rootpage] != 0
                        AND [source].[tbl_name] NOT LIKE 'sqlite_%'
                        AND [source].[tbl_name] = [target].[tbl_name]
                ORDER  BY [target].[tbl_name];
        )sql", "main" , kSourceDbAlias).GetUtf8CP()};

        std::vector<std::string> tables;
        Statement stmt;
        rc = stmt.Prepare(target, enumTblSql.c_str());
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
        while(stmt.Step() != BE_SQLITE_ROW) {
            Utf8String tbl = stmt.GetValueText(0);
            if (tableFilter(tbl)) {
                tables.push_back(tbl);
            }
        }
        return InsertOrUpdate(target, tables, kSourceDbAlias, "main");
    }
};
//************************************************************************************
//ECIssueListener
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDbIssue ECIssueListener::GetIssue() const
    {
    if (!m_issue.has_value())
        return m_issue;

    ECDbIssue copy(m_issue);
    //reset cached issue before returning
    m_issue = ECDbIssue();
    return copy;
    }

//************************************************************************************
//ECDbTestLogger
//************************************************************************************
NativeLogging::CategoryLogger ECDbTestLogger::Get()  {
    return NativeLogging::CategoryLogger("ECDbTests");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ToString(JsonECSqlSelectAdapter::FormatOptions const& options)
    {
    Utf8String str("JsonECSqlSelectAdapter::FormatOptions(");
    switch (options.GetMemberCasingMode())
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                str.append("MemberNameCasing::KeepOriginal");
                break;
            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                str.append("MemberNameCasing::LowerFirstChar");
                break;

            default:
                return Utf8String("Unhandled JsonECSqlSelectAdapter::MemberNameCasing. Adjust ECDb ATP ToString method");
        }

    str.append(",");
    switch (options.GetInt64Format())
        {
            case ECJsonInt64Format::AsNumber:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsNumber));
                break;
            case ECJsonInt64Format::AsDecimalString:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsDecimalString));
                break;
            case ECJsonInt64Format::AsHexadecimalString:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsHexadecimalString));
                break;
            default:
                return Utf8String("Unhandled ECJsonInt64Format. Adjust ECDb ATP ToString method");
        }

    str.append(")");
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ToString(JsonUpdater::Options const& options)
    {
    Utf8String str("JsonUpdater::Options(");
    switch (options.GetSystemPropertiesOption())
        {
            case JsonUpdater::SystemPropertiesOption::Fail:
                str.append("SystemPropertiesOption::Fail");
                break;
            case JsonUpdater::SystemPropertiesOption::Ignore:
                str.append("SystemPropertiesOption::Ignore");
                break;

            default:
                return Utf8String("Unhandled ReadonlyPropertiesOption. Adjust ECDb ATP ToString method");
        }

    str.append(", ");

    switch (options.GetReadonlyPropertiesOption())
        {
            case JsonUpdater::ReadonlyPropertiesOption::Fail:
                str.append("ReadonlyPropertiesOption::Fail");
                break;
            case JsonUpdater::ReadonlyPropertiesOption::Ignore:
                str.append("ReadonlyPropertiesOption::Ignore");
                break;

            case JsonUpdater::ReadonlyPropertiesOption::Update:
                str.append("ReadonlyPropertiesOption::Update");
                break;

            default:
                return Utf8String("Unhandled ReadonlyPropertiesOption. Adjust ECDb ATP ToString method");
        }

    str.append(", ECSQLOPTIONS: ").append(options.GetECSqlOptions());
    return str;
    }

END_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//GTest PrintTo customizations
//************************************************************************************

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BentleyStatus stat, std::ostream* os)
    {
    switch (stat)
        {
            case SUCCESS:
                *os << "SUCCESS";
                break;

            case ERROR:
                *os << "ERROR";
                break;

            default:
                *os << "Unhandled BentleyStatus. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) {  *os << id.GetValueUnchecked();  }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime const& dt, std::ostream* os) { *os << dt.ToString().c_str(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime::Info const& dtInfo, std::ostream* os)
    {
    if (!dtInfo.IsValid())
        {
        *os << "<invalid DateTime::Info>";
        return;
        }

    switch (dtInfo.GetComponent())
        {
        case DateTime::Component::Date:
            *os << "Date";
            return;

        case DateTime::Component::DateAndTime:
        {
        *os << "TimeStamp [";
        switch (dtInfo.GetKind())
            {
            case DateTime::Kind::Local:
                *os << "Local";
                break;
            case DateTime::Kind::Unspecified:
                *os << "Unspecified";
                break;
            case DateTime::Kind::Utc:
                *os << "Utc";
                break;
            default:
                *os << "Unhandled DateTime::Kind. Adjust the PrintTo method";
                break;

            }
        *os << "]";
        return;
        }

        case DateTime::Component::TimeOfDay:
            *os << "TimeOfDay";
            return;

        default:
            *os << "Unhandled DateTime::Component. Adjust the PrintTo method";
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8CP> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8CP str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str;
        isFirstItem = false;
        }
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8String> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8StringCR str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str.c_str();
        isFirstItem = false;
        }
    *os << "}";
    }
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECClassId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECValue const& val, std::ostream* os) {  *os << val.ToString().c_str(); }

END_BENTLEY_ECOBJECT_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DbResult r, std::ostream* os) { *os << Db::InterpretDbResult(r); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeBriefcaseBasedId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ProfileState state, std::ostream* os)
    {
    if (state.IsError())
        {
        *os << "ProfileState(Error)";
        return;
        }

    *os << "ProfileState(";
    switch (state.GetAge())
        {
            case ProfileState::Age::Older:
                *os << "Age::Older";
                break;
            case ProfileState::Age::UpToDate:
                *os << "Age::UpToDate";
                break;
            case ProfileState::Age::Newer:
                *os << "Age::Newer";
                break;
            default:
                *os << "Invalid ProfileState::Age";
                return;
        }

    *os << ",";
    switch (state.GetCanOpen())
        {
            case ProfileState::CanOpen::No:
                *os << "CanOpen::No";
                break;
            case ProfileState::CanOpen::Readonly:
                *os << "CanOpen::Readonly";
                break;
            case ProfileState::CanOpen::Readwrite:
                *os << "CanOpen::Readwrite";
                break;
            default:
                *os << "Invalid ProfileState::CanOpen";
                return;
        }

    if (state.GetAge() == ProfileState::Age::Older)
        *os << ",Upgradable: " << state.IsUpgradable() ? "yes" : "no";

    *os << ")";
    }

END_BENTLEY_SQLITE_NAMESPACE

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceKey const& key, std::ostream* os)
    {
    *os << "{ECInstanceId:";
    PrintTo(key.GetInstanceId(), os);
    *os << ",ECClassId:";
    PrintTo(key.GetClassId(), os);
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECSqlStatus stat, std::ostream* os)
    {
    if (stat.IsSuccess())
        {
        *os << "ECSqlStatus::Success";
        return;
        }

    if (stat.IsSQLiteError())
        {
        *os << "ECSqlStatus::SQLiteError " << stat.GetSQLiteError();
        return;
        }

    switch (stat.Get())
        {
            case ECSqlStatus::Status::Error:
                *os << "ECSqlStatus::Error";
                return;

            case ECSqlStatus::Status::InvalidECSql:
                *os << "ECSqlStatus::InvalidECSql";
                return;

            default:
                *os << "Unhandled ECSqlStatus. Adjust the PrintTo method";
                break;
        }
    }
void PrintTo(SchemaImportResult rc, std::ostream* os)
    {
    switch ((SchemaImportResult::Status)rc)
        {
            case SchemaImportResult::ERROR:
                *os << "SchemaImportResult::ERROR";
                return;

            case SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED:
                *os << "SchemaImportResult::ERROR_IMODEL_LOCK_FAILED";
                return;

            case SchemaImportResult::ERROR_READONLY:
                *os << "SchemaImportResult::ERROR_READONLY";
                return;

            case SchemaImportResult::OK:
                *os << "SchemaImportResult::OK";
                return;

            default:
                *os << "Unhandled SchemaImportResult::Status. Adjust the PrintTo method";
                break;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
