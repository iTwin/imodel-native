/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceOverflowTablesResearchTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "PerformanceTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
struct PerformanceOverflowTablesResearchTestFixture : ECDbTestFixture
    {
    protected:
        struct Scenario final
            {
        private:
            Utf8String m_name;
            //col counts exclude the PK column of each table
            int m_primaryTableColCount = 0;
            int m_secondaryTableUnsharedColCount = 0;
            int m_sharedColCount = 0;
            int m_maxClassColCount = 0;

        public:
            Scenario(Utf8CP name, int primaryTableColCount, int secondaryTableUnsharedColCount, int sharedColCount, int maxClassColCount);

            bool IsValid() const { return m_primaryTableColCount > 0; }

            Utf8StringCR GetName() const { return m_name; }
            int GetPrimaryTableCount() const { return m_primaryTableColCount; }
            int GetTotalSharedColCount() const { return m_maxClassColCount - (m_primaryTableColCount + m_secondaryTableUnsharedColCount); }
            bool HasSecondaryTable() const { return m_secondaryTableUnsharedColCount > 0; }
            int GetSecondaryTableCount() const;
            int GetTernaryTableCount() const;
            bool HasTernaryTable() const { return GetTernaryTableCount() > 0; }
            int TableCount() const { if (!HasSecondaryTable()) return 1;  return HasTernaryTable() ? 3 : 2; }
            Utf8String ToString() const;
            Utf8String ToFileNameString() const;
            };

        enum class ColumnMode
            {
            AllColumns,
            First,
            Middle,
            Last
            };

        struct StatementInfo final
            {
            Statement m_stmt;
            ColumnMode m_colMode = ColumnMode::First;
            int m_colCount = 0;
            int m_idParamIndex = -1;
            bmap<int, int> m_colIxParamIxMap;

            StatementInfo() {}
            StatementInfo(ColumnMode mode, int colCount) : m_colMode(mode), m_colCount(colCount) {}

            Utf8CP GetSql() const { return m_stmt.GetSql(); }
            };

        static const int s_initialRowCount = 100000;
        static const int s_opCount = 50000;

    private:
        static int ComputeValue(int rowNo, int colNumber);
        static Utf8String GetColumnName(int colIndex);
        static int GetSingleColIndex(StatementInfo&);
        void LogTiming(StopWatch&, Scenario const&, Utf8CP operation, int initialRowCount, int opCount) const;
        static void SetupTestDb(Db&, Scenario const&);

        static DbResult BindValues(StatementInfo&, int rowNumber, int64_t id);
        static Utf8CP ColumnModeToString(ColumnMode);

    protected:
        void RunInsertAllCols(Scenario const&) const;
        void RunInsertSingleCol(Scenario const&, ColumnMode) const;
        void RunSelectSingleCol(Scenario const&, ColumnMode) const;
        void RunSelectWhereSingleCol(Scenario const&, ColumnMode, int opCount) const;
        void RunUpdateSingleCol(Scenario const&, ColumnMode) const;
        void RunUpdateAllCols(Scenario const&) const;
        void RunDelete(Scenario const&) const;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunInsertAllCols(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetPrimaryTableCount());

    Utf8String insertSql("INSERT INTO t1(");
    Utf8String insertValuesSql(") VALUES(");
    int paramIndex = 0;
    for (int i = 0; i < scenario.GetPrimaryTableCount(); i++)
        {
        if (i > 0)
            {
            insertSql.append(",");
            insertValuesSql.append(",");
            }

        insertSql.append(GetColumnName(i));
        insertValuesSql.append("?");
        paramIndex++;

        stmt->m_colIxParamIxMap[i] = paramIndex;
        }
    insertValuesSql.append(")");
    insertSql.append(insertValuesSql);

    ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
    stmts.push_back(std::move(stmt));

    if (scenario.HasSecondaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetSecondaryTableCount());
        stmt->m_idParamIndex = 1;
        insertSql.assign("INSERT INTO t2(Id");
        insertValuesSql.assign(") VALUES(?");

        const int colOffset = scenario.GetPrimaryTableCount();
        paramIndex = 0;
        for (int i = 0; i < scenario.GetSecondaryTableCount(); i++)
            {
            insertSql.append(",").append(GetColumnName(colOffset + i));
            insertValuesSql.append(",").append("?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetTernaryTableCount());
        stmt->m_idParamIndex = 1;
        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");

        const int colOffset = scenario.GetPrimaryTableCount() + scenario.GetSecondaryTableCount();
        paramIndex = 0;
        for (int i = 0; i < scenario.GetTernaryTableCount(); i++)
            {
            insertSql.append(",").append(GetColumnName(colOffset + i));
            insertValuesSql.append(",").append("?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
        stmts.push_back(std::move(stmt));
        }

    for (int rowCount = 0; rowCount < s_opCount; rowCount++)
        {
        int64_t id = INT64_C(-1);
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowCount, id)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();

            if (id < 0)
                id = db.GetLastInsertRowId();

            stmt->m_stmt.Reset();
            stmt->m_stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "INSERT all cols", s_initialRowCount, s_opCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunInsertSingleCol(Scenario const& scenario, ColumnMode colMode) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetPrimaryTableCount());
    int colIx = GetSingleColIndex(*stmt);
    Utf8String insertSql;
    insertSql.Sprintf("INSERT INTO t1(%s) VALUES(?)", GetColumnName(colIx).c_str());
    stmt->m_colIxParamIxMap[colIx] = 1;
    ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();
    stmts.push_back(std::move(stmt));

    if (scenario.HasSecondaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetSecondaryTableCount());
        colIx = GetSingleColIndex(*stmt);
        stmt->m_idParamIndex = 1;
        stmt->m_colIxParamIxMap[colIx] = 2;

        insertSql.Sprintf("INSERT INTO t2(Id,%s) VALUES(?,?)",
                          GetColumnName(scenario.GetPrimaryTableCount() + colIx).c_str());


        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetTernaryTableCount());
        colIx = GetSingleColIndex(*stmt);
        stmt->m_idParamIndex = 1;
        stmt->m_colIxParamIxMap[colIx] = 2;

        insertSql.Sprintf("INSERT INTO t3(Id,%s) VALUES(?,?)",
                          GetColumnName(scenario.GetPrimaryTableCount() + scenario.GetSecondaryTableCount() + colIx).c_str());

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();
        stmts.push_back(std::move(stmt));
        }


    for (int rowCount = 0; rowCount < s_opCount; rowCount++)
        {
        int64_t id = INT64_C(-1);
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowCount, id)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();

            if (id < 0)
                id = db.GetLastInsertRowId();

            stmt->m_stmt.Reset();
            stmt->m_stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    Utf8String opStr;
    opStr.Sprintf("INSERT %s col in each table", ColumnModeToString(colMode));
    LogTiming(timer, scenario, opStr.c_str(), s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunUpdateAllCols(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetPrimaryTableCount());
    Utf8String sql("UPDATE t1 SET");
    int paramIndex = 0;
    for (int i = 0; i < scenario.GetPrimaryTableCount(); i++)
        {
        if (i > 0)
            sql.append(",");

        sql.append(GetColumnName(i)).append("=?");
        paramIndex++;
        stmt->m_colIxParamIxMap[i] = paramIndex;
        }

    sql.append(" WHERE Id=?");
    stmt->m_idParamIndex = paramIndex + 1;
    ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, sql.c_str())) << sql.c_str();
    stmts.push_back(std::move(stmt));

    if (scenario.HasSecondaryTable())
        {
        sql.assign("UPDATE t2 SET");
        const int colOffset = scenario.GetPrimaryTableCount();
        paramIndex = 0;
        for (int i = 0; i < scenario.GetSecondaryTableCount(); i++)
            {
            if (i > 0)
                sql.append(",");

            sql.append(GetColumnName(colOffset + i)).append("=?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        sql.append(" WHERE Id=?");
        stmt->m_idParamIndex = paramIndex + 1;

        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetSecondaryTableCount());
        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, sql.c_str())) << sql.c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable())
        {
        sql.assign("UPDATE t3 SET");

        const int colOffset = scenario.GetPrimaryTableCount() + scenario.GetSecondaryTableCount();
        paramIndex = 0;
        for (int i = 0; i < scenario.GetTernaryTableCount(); i++)
            {
            if (i > 0)
                sql.append(",");

            sql.append(GetColumnName(colOffset + i)).append("=?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        sql.append(" WHERE Id=?");
        stmt->m_idParamIndex = paramIndex + 1;

        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.GetTernaryTableCount());
        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, sql.c_str())) << sql.c_str();
        stmts.push_back(std::move(stmt));
        }

    for (int rowCount = 0; rowCount < s_opCount; rowCount++)
        {
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowCount, rowCount + 1)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();

            stmt->m_stmt.Reset();
            stmt->m_stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "UPDATE all cols", s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunUpdateSingleCol(Scenario const& scenario, ColumnMode colMode) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetPrimaryTableCount());
    int colIndex = GetSingleColIndex(*stmt);
    stmt->m_colIxParamIxMap[colIndex] = 1;
    stmt->m_idParamIndex = 2;
    Utf8String insertSql;
    insertSql.Sprintf("UPDATE t1 SET %s=? WHERE Id=?", GetColumnName(colIndex).c_str());
    ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
    stmts.push_back(std::move(stmt));

    if (scenario.HasSecondaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetSecondaryTableCount());
        int colIndex = GetSingleColIndex(*stmt);
        stmt->m_colIxParamIxMap[colIndex] = 1;
        stmt->m_idParamIndex = 2;

        insertSql.Sprintf("UPDATE t2 SET %s=? WHERE Id=?",
                          GetColumnName(scenario.GetPrimaryTableCount() + colIndex).c_str());

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable())
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.GetTernaryTableCount());
        int colIndex = GetSingleColIndex(*stmt);
        stmt->m_colIxParamIxMap[colIndex] = 1;
        stmt->m_idParamIndex = 2;

        insertSql.Sprintf("UPDATE t3 SET %s=? WHERE Id=?",
                          GetColumnName(scenario.GetPrimaryTableCount() + scenario.GetSecondaryTableCount() + colIndex).c_str());

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
        stmts.push_back(std::move(stmt));
        }


    for (int rowCount = 0; rowCount < s_opCount; rowCount++)
        {
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            const int colNo = GetSingleColIndex(*stmt);

            ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.BindInt(1, ComputeValue(rowCount, colNo) + 1));
            ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.BindInt(stmt->m_idParamIndex, rowCount + 1));
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step());
            ASSERT_EQ(1, db.GetModifiedRowCount());
            stmt->m_stmt.Reset();
            stmt->m_stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    Utf8String opStr;
    opStr.Sprintf("UPDATE %s col in each table", ColumnModeToString(colMode));
    LogTiming(timer, scenario, opStr.c_str(), s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
DbResult PerformanceOverflowTablesResearchTestFixture::BindValues(StatementInfo& stmt, int rowNumber, int64_t id)
    {
    if (stmt.m_idParamIndex >= 1 && id <= 0)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    int firstParamIndex = 1;
    if (id > 0)
        {
        DbResult stat = stmt.m_stmt.BindInt64(firstParamIndex, id);
        if (BE_SQLITE_OK != stat)
            return stat;

        firstParamIndex++;
        }

    if (stmt.m_colMode == ColumnMode::AllColumns)
        {
        for (int i = 0; i < stmt.m_colCount; i++)
            {
            const int value = ComputeValue(rowNumber, i);
            DbResult stat = stmt.m_stmt.BindInt(firstParamIndex + i, value);
            if (BE_SQLITE_OK != stat)
                return stat;
            }
        }
    else
        {
        const int colIndex = GetSingleColIndex(stmt);
        const int value = ComputeValue(rowNumber, colIndex);

        DbResult stat = stmt.m_stmt.BindInt(firstParamIndex, value);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearchTestFixture::GetSingleColIndex(StatementInfo& stmtInfo)
    {
    switch (stmtInfo.m_colMode)
        {
            case ColumnMode::First:
                return 0;

            case ColumnMode::Middle:
                return stmtInfo.m_colCount / 2;

            case ColumnMode::Last:
                return stmtInfo.m_colCount - 1;

            default:
                BeAssert(false && "ColumnMode::All not allowed for GetSingleColIndex");
                return -1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearchTestFixture::ComputeValue(int rowNo, int colNumber)
    {
    switch (rowNo % 3)
        {
            case 0:
                return 14352 * (colNumber + 1);

            case 1:
                return -4553232 / (colNumber + 1);

            case 2:
                return 9832423 + (1400305 + colNumber);

            default:
                BeAssert(false);
                return 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
Utf8String PerformanceOverflowTablesResearchTestFixture::GetColumnName(int colIndex)
    {
    Utf8String colName;
    colName.Sprintf("col%d", colIndex + 1);
    return colName;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::LogTiming(StopWatch& timer, Scenario const& scenario, Utf8CP operation, int initialRowCount, int opCount) const
    {
    Utf8String descr;
    descr.Sprintf("%s,%s,%d,%d,%d,%d", operation, scenario.GetName().c_str(), scenario.GetPrimaryTableCount(), scenario.GetSecondaryTableCount(), scenario.GetTernaryTableCount(), initialRowCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP PerformanceOverflowTablesResearchTestFixture::ColumnModeToString(ColumnMode mode)
    {
    switch (mode)
        {
            case ColumnMode::First:
                return "First";
            case ColumnMode::Last:
                return "Last";
            case ColumnMode::Middle:
                return "Middle";

            default:
                BeAssert(false);
                return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
//static
void PerformanceOverflowTablesResearchTestFixture::SetupTestDb(Db& db, Scenario const& scenario)
    {
    Initialize();

    Utf8String fileName;
    fileName.Sprintf("perf_%s_initialrowcount%d_%d.db", scenario.ToFileNameString().c_str(), s_initialRowCount,
                     DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName filePath = BuildECDbPath(fileName.c_str());

    if (filePath.DoesPathExist())
        {
        ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << filePath.GetNameUtf8().c_str();
        return;
        }

    std::vector<Utf8String> createTableSqls, insertSqls;

    Utf8String createTableSql("CREATE TABLE t1 (Id INTEGER PRIMARY KEY");
    Utf8String insertSql("INSERT INTO t1 (");
    Utf8String insertValuesSql(") VALUES(");
    for (int i = 0; i < scenario.GetPrimaryTableCount(); i++)
        {
        createTableSql.append(",");
        Utf8String colName = GetColumnName(i);
        if (i > 0)
            {
            insertSql.append(",");
            insertValuesSql.append(",");
            }

        createTableSql.append(colName).append(" BLOB");
        insertSql.append(colName);
        insertValuesSql.append("random() / 1000");
        }

    createTableSql.append(")");
    insertValuesSql.append(")");
    insertSql.append(insertValuesSql);
    createTableSqls.push_back(createTableSql);
    insertSqls.push_back(insertSql);

    if (scenario.HasSecondaryTable())
        {
        createTableSql.assign("CREATE TABLE t2(Id INTEGER PRIMARY KEY REFERENCES t1(Id) ON DELETE CASCADE");
        insertSql.assign("INSERT INTO t2(Id");
        insertValuesSql.assign(") VALUES(?");
        for (int i = 0; i < scenario.GetSecondaryTableCount(); i++)
            {
            Utf8String colName = GetColumnName(i + scenario.GetPrimaryTableCount());
            createTableSql.append(",").append(colName).append(" BLOB");
            insertSql.append(",").append(colName);
            insertValuesSql.append(",random() / 1000");
            }

        createTableSql.append(")");
        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        createTableSqls.push_back(createTableSql);
        insertSqls.push_back(insertSql);
        }

    if (scenario.GetTernaryTableCount() > 0)
        {
        createTableSql.assign("CREATE TABLE t3(Id INTEGER PRIMARY KEY REFERENCES t2(Id) ON DELETE CASCADE");
        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");
        for (int i = 0; i < scenario.GetTernaryTableCount(); i++)
            {
            Utf8String colName = GetColumnName(i + scenario.GetPrimaryTableCount() + scenario.GetSecondaryTableCount());
            createTableSql.append(",").append(colName).append(" BLOB");
            insertSql.append(",").append(colName);
            insertValuesSql.append(",random() / 1000");
            }

        createTableSql.append(")");
        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        createTableSqls.push_back(createTableSql);
        insertSqls.push_back(insertSql);
        }

    Db seedDb;
    ASSERT_EQ(BE_SQLITE_OK, seedDb.CreateNewDb(filePath)) << filePath.GetNameUtf8().c_str();

    for (Utf8StringCR createTableSql : createTableSqls)
        {
        ASSERT_EQ(BE_SQLITE_OK, seedDb.ExecuteSql(createTableSql.c_str())) << createTableSql.c_str() << " " << seedDb.GetLastError().c_str();
        }

    std::vector<std::unique_ptr<Statement>> stmts;
    for (Utf8StringCR insertSql : insertSqls)
        {
        std::unique_ptr<Statement> stmt = std::make_unique<Statement>();
        ASSERT_EQ(BE_SQLITE_OK, stmt->Prepare(seedDb, insertSql.c_str())) << insertSql.c_str() << " " << seedDb.GetLastError().c_str();
        stmts.push_back(std::move(stmt));
        }

    for (int i = 0; i < s_initialRowCount; i++)
        {
        ASSERT_EQ(BE_SQLITE_DONE, stmts[0]->Step()) << stmts[0]->GetSql() << " " << seedDb.GetLastError().c_str();
        const int64_t id = seedDb.GetLastInsertRowId();
        stmts[0]->Reset();

        for (size_t j = 1; j < stmts.size(); j++)
            {
            Statement& stmt = *stmts[j];
            ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt64(1, id)) << "Row#" << i + 1 << stmt.GetSql() << " " << seedDb.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Row#" << i + 1 << stmt.GetSql() << " " << seedDb.GetLastError().c_str();
            stmt.ClearBindings();
            stmt.Reset();
            }
        }

    stmts.clear();

    ASSERT_EQ(BE_SQLITE_OK, seedDb.SaveChanges());
    seedDb.CloseDb();

    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << filePath.GetNameUtf8().c_str();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
PerformanceOverflowTablesResearchTestFixture::Scenario::Scenario(Utf8CP name, int primaryTableColCount, int secondaryTableUnsharedColCount, int sharedColCount, int maxClassColCount) : m_name(name)
    {
    const int neededSharedColCount = maxClassColCount - (primaryTableColCount + secondaryTableUnsharedColCount);
    if (neededSharedColCount < 0)
        {
        BeAssert(false && "invalid scenario");
        return;
        }

    m_primaryTableColCount = primaryTableColCount;
    m_secondaryTableUnsharedColCount = secondaryTableUnsharedColCount;
    m_sharedColCount = sharedColCount;
    m_maxClassColCount = maxClassColCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
int PerformanceOverflowTablesResearchTestFixture::Scenario::GetSecondaryTableCount() const
    {
    if (!HasSecondaryTable())
        return 0;

    if (GetTotalSharedColCount() > m_sharedColCount)
        return m_secondaryTableUnsharedColCount + m_sharedColCount;

    return m_secondaryTableUnsharedColCount + GetTotalSharedColCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
int PerformanceOverflowTablesResearchTestFixture::Scenario::GetTernaryTableCount() const
    {
    const int excessSharedColCount = GetTotalSharedColCount() - m_sharedColCount;
    if (excessSharedColCount <= 0)
        return 0;

    return excessSharedColCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8String PerformanceOverflowTablesResearchTestFixture::Scenario::ToString() const
    {
    Utf8String str;
    str.Sprintf("%s (Primary Table: %d cols, Secondary Table: %d unshared cols, %d shared cols, Max class col count: %d)",
                m_name.c_str(), m_primaryTableColCount, m_secondaryTableUnsharedColCount, m_sharedColCount, m_maxClassColCount);
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8String PerformanceOverflowTablesResearchTestFixture::Scenario::ToFileNameString() const
    {
    Utf8String str;
    str.Sprintf("%s_%d_%d_%d_%d",
                m_name.c_str(), m_primaryTableColCount, m_secondaryTableUnsharedColCount, m_sharedColCount, m_maxClassColCount);
    return str;
    }

END_ECDBUNITTESTS_NAMESPACE
