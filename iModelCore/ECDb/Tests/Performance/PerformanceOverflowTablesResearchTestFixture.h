/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
            int m_secondaryTableMaxSharedColCount = 0;
            int m_maxClassColCount = 0;
            int m_eightyPercentClassColCount = 0;

        public:
            Scenario(Utf8CP name, int primaryTableColCount, int secondaryTableUnsharedColCount, int secondaryTableMaxSharedColCount, int maxClassColCount, int eightyPercentClassColCount);

            bool IsValid() const { return m_primaryTableColCount > 0; }

            Utf8StringCR GetName() const { return m_name; }
            int PrimaryTableColCount() const { return m_primaryTableColCount; }
            int EightyPercentPrimaryTableColCount() const { return (m_eightyPercentClassColCount < m_primaryTableColCount) ? m_eightyPercentClassColCount : m_primaryTableColCount; }
            int TotalSharedColCount() const { return m_maxClassColCount - (m_primaryTableColCount + m_secondaryTableUnsharedColCount); }
            bool HasSecondaryTable() const { return m_secondaryTableUnsharedColCount > 0; }
            int SecondaryTableUnsharedColCount() const { return m_secondaryTableUnsharedColCount; }
            int SecondaryTableMaxSharedColCount() const { return m_secondaryTableMaxSharedColCount; }
            int MaxClassColCount() const { return m_maxClassColCount; }
            int EightyPercentClassColCount() const { return m_eightyPercentClassColCount; }

            int SecondaryTableColCount() const;
            int EightyPercentSecondaryTableColCount() const { return (m_eightyPercentClassColCount < SecondaryTableColCount()) ? m_eightyPercentClassColCount - m_primaryTableColCount : SecondaryTableColCount(); }
            int TernaryTableColCount() const;
            int EightyPercentTernaryTableColCount() const { return (m_eightyPercentClassColCount < TernaryTableColCount()) ? m_eightyPercentClassColCount - m_primaryTableColCount - SecondaryTableColCount() : TernaryTableColCount(); }
            bool HasTernaryTable() const { return TernaryTableColCount() > 0; }

            int GetTableForColumn(int colIx) const 
                {
                if (colIx < m_primaryTableColCount)
                    return 0;

                if (colIx < (m_primaryTableColCount + SecondaryTableColCount()))
                    return 1;

                return 2;
                }

            int TableCount() const { if (!HasSecondaryTable()) return 1;  return HasTernaryTable() ? 3 : 2; }
            Utf8String ToCsvString() const;
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

        static const int s_initialRowCount = 1000;
        static const int s_opCount = 500;

    private:
        static int ComputeValue(int rowId, int colIndex);
        static Utf8String GetColumnName(int colIndex);
        static int GetSingleColIndex(ColumnMode, Scenario const&);
        void LogTiming(StopWatch&, Scenario const&, Utf8CP operation, int initialRowCount, int opCount) const;
        static void SetupTestDb(Db&, Scenario const&);

        static DbResult BindValues(StatementInfo&, int rowId);
        static Utf8CP ColumnModeToString(ColumnMode);

    protected:
        void RunInsertAllCols(Scenario const&) const;
        void RunInsertSingleCol(Scenario const&, ColumnMode) const;
        void RunSelectAllCols(Scenario const&) const;
        void RunUpdateAllCols(Scenario const&) const;
        void RunDelete(Scenario const&) const;
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunInsertAllCols(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToCsvString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentPrimaryTableColCount());

    Utf8String insertSql("INSERT INTO t1(");
    Utf8String insertValuesSql(") VALUES(");
    int paramIndex = 0;
    for (int i = 0; i < stmt->m_colCount; i++)
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

    //if has secondary table but it would only be filled with the id, we will not create the row
    if (scenario.HasSecondaryTable() && scenario.EightyPercentSecondaryTableColCount() > 0)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentSecondaryTableColCount());
        stmt->m_idParamIndex = 1;
        insertSql.assign("INSERT INTO t2(Id");
        insertValuesSql.assign(") VALUES(?");

        const int colOffset = scenario.PrimaryTableColCount();
        paramIndex = 1;
        for (int i = 0; i < stmt->m_colCount; i++)
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

    //if has ternary table but it would only be filled with the id, we will not create the row
    if (scenario.HasTernaryTable() && scenario.EightyPercentTernaryTableColCount() > 0)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentTernaryTableColCount());
        stmt->m_idParamIndex = 1;
        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");

        const int colOffset = scenario.PrimaryTableColCount() + scenario.SecondaryTableColCount();
        paramIndex = 1;
        for (int i = 0; i < stmt->m_colCount; i++)
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

    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        int rowId = s_initialRowCount + opNo + 1;
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowId)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(1, db.GetModifiedRowCount()) << stmt->GetSql() << scenario.ToFileNameString().c_str();

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
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToCsvString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    const int colIx = GetSingleColIndex(colMode, scenario);
    const int tableIx = scenario.GetTableForColumn(colIx);
    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.EightyPercentPrimaryTableColCount());
    stmt->m_idParamIndex = 1;
    Utf8String insertSql;
    if (tableIx == 0)
        {
        insertSql.Sprintf("INSERT INTO t1(Id,%s) VALUES(?,?)", GetColumnName(colIx).c_str());
        stmt->m_colIxParamIxMap[colIx] = 2;
        }
    else
        insertSql.assign("INSERT INTO t1(Id) VALUES(?)");

    ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();
    stmts.push_back(std::move(stmt));

    if (scenario.HasSecondaryTable() && scenario.EightyPercentSecondaryTableColCount() > 0 && tableIx > 0)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.EightyPercentSecondaryTableColCount());
        stmt->m_idParamIndex = 1;

        if (tableIx == 1)
            {
            stmt->m_colIxParamIxMap[colIx] = 2;
            insertSql.Sprintf("INSERT INTO t2(Id,%s) VALUES(?,?)", GetColumnName(colIx).c_str());
            }
        else
            insertSql.assign("INSERT INTO t2(Id) VALUES(?)");

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << scenario.ToFileNameString().c_str() << " " << insertSql.c_str() << " " << db.GetLastError().c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable() && scenario.EightyPercentTernaryTableColCount() > 0 && tableIx == 2)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(colMode, scenario.EightyPercentTernaryTableColCount());
        stmt->m_idParamIndex = 1;
        stmt->m_colIxParamIxMap[colIx] = 2;

        insertSql.Sprintf("INSERT INTO t3(Id,%s) VALUES(?,?)", GetColumnName(colIx).c_str());

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();
        stmts.push_back(std::move(stmt));
        }


    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        int rowId = s_initialRowCount + opNo + 1;
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowId)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(1, db.GetModifiedRowCount()) << stmt->GetSql() << scenario.ToFileNameString().c_str();

            stmt->m_stmt.Reset();
            stmt->m_stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    Utf8String opStr;
    opStr.Sprintf("INSERT %s col", ColumnModeToString(colMode));
    LogTiming(timer, scenario, opStr.c_str(), s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunUpdateAllCols(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToCsvString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);

    std::vector<std::unique_ptr<StatementInfo>> stmts;

    std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentPrimaryTableColCount());
    Utf8String sql("UPDATE t1 SET ");
    int paramIndex = 0;
    for (int i = 0; i < stmt->m_colCount; i++)
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

    if (scenario.HasSecondaryTable() && scenario.EightyPercentSecondaryTableColCount() > 0)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentSecondaryTableColCount());

        sql.assign("UPDATE t2 SET ");
        const int colOffset = scenario.PrimaryTableColCount();
        paramIndex = 0;
        for (int i = 0; i < stmt->m_colCount; i++)
            {
            if (i > 0)
                sql.append(",");

            sql.append(GetColumnName(colOffset + i)).append("=?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        sql.append(" WHERE Id=?");
        stmt->m_idParamIndex = paramIndex + 1;

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, sql.c_str())) << sql.c_str();
        stmts.push_back(std::move(stmt));
        }

    if (scenario.HasTernaryTable() && scenario.EightyPercentTernaryTableColCount() > 0)
        {
        std::unique_ptr<StatementInfo> stmt = std::make_unique<StatementInfo>(ColumnMode::AllColumns, scenario.EightyPercentTernaryTableColCount());

        sql.assign("UPDATE t3 SET ");
        const int colOffset = scenario.PrimaryTableColCount() + scenario.SecondaryTableColCount();
        paramIndex = 0;
        for (int i = 0; i < stmt->m_colCount; i++)
            {
            if (i > 0)
                sql.append(",");

            sql.append(GetColumnName(colOffset + i)).append("=?");
            paramIndex++;
            stmt->m_colIxParamIxMap[i] = paramIndex;
            }

        sql.append(" WHERE Id=?");
        stmt->m_idParamIndex = paramIndex + 1;

        ASSERT_EQ(BE_SQLITE_OK, stmt->m_stmt.Prepare(db, sql.c_str())) << sql.c_str();
        stmts.push_back(std::move(stmt));
        }

    const int rowIdIncrement = s_initialRowCount / s_opCount;
    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        const int rowId = opNo * rowIdIncrement + 1;
        for (std::unique_ptr<StatementInfo>& stmt : stmts)
            {
            ASSERT_EQ(BE_SQLITE_OK, BindValues(*stmt, rowId)) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt->m_stmt.Step()) << stmt->GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(1, db.GetModifiedRowCount()) << stmt->GetSql() << scenario.ToFileNameString().c_str();
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
void PerformanceOverflowTablesResearchTestFixture::RunSelectAllCols(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToCsvString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    Utf8String selectClause("SELECT ");
    Utf8String fromClause(" FROM t1");
    Utf8String whereClause(" WHERE ");
    for (int i = 0; i < scenario.EightyPercentPrimaryTableColCount(); i++)
        {
        if (i > 0)
            selectClause.append(",");
        selectClause.append("t1.").append(GetColumnName(i));
        }

    if (scenario.HasSecondaryTable())
        {
        int colOffset = scenario.PrimaryTableColCount();
        for (int i = 0; i < scenario.EightyPercentSecondaryTableColCount(); i++)
            {
            selectClause.append(",t2.").append(GetColumnName(colOffset + i));
            }

        fromClause.append(",t2");
        whereClause.append("t1.Id=t2.Id");

        if (scenario.HasTernaryTable())
            {
            int colOffset = scenario.PrimaryTableColCount() + scenario.SecondaryTableColCount();
            for (int i = 0; i < scenario.EightyPercentTernaryTableColCount(); i++)
                {
                selectClause.append(",t3.").append(GetColumnName(colOffset + i));
                }

            fromClause.append(",t3");
            whereClause.append(" AND t2.Id=t3.Id");
            }
        }

    whereClause.append(" AND t1.Id=?");
    Utf8String sql(selectClause);
    sql.append(fromClause).append(whereClause);

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str();

    const int rowIdIncrement = s_initialRowCount / s_opCount;
    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        const int rowId = opNo * rowIdIncrement + 1;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(1, rowId)) << "Id: " << rowId << " SQL: " << sql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Id: " << rowId << " SQL: " << sql.c_str();
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(timer, scenario, "SELECT all columns from each table", s_initialRowCount, s_opCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunDelete(Scenario const& scenario) const
    {
    ASSERT_TRUE(scenario.IsValid()) << scenario.ToCsvString().c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "DELETE FROM t1 WHERE Id=?"));

    const int rowIdIncrement = s_initialRowCount / s_opCount;
    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        const int rowId = opNo * rowIdIncrement + 1;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(1, rowId));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        ASSERT_EQ(1, db.GetModifiedRowCount());
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "DELETE WHERE Id=?", s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
DbResult PerformanceOverflowTablesResearchTestFixture::BindValues(StatementInfo& stmt, int rowId)
    {
    if (stmt.m_idParamIndex >= 0)
        {
        DbResult stat = stmt.m_stmt.BindInt(stmt.m_idParamIndex, rowId);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    for (bpair<int, int> const& kvPair : stmt.m_colIxParamIxMap)
        {
        const int value = ComputeValue(rowId, kvPair.first);
        DbResult stat = stmt.m_stmt.BindInt(kvPair.second, value);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearchTestFixture::GetSingleColIndex(ColumnMode colMode, Scenario const& scenario)
    {
    switch (colMode)
        {
            case ColumnMode::First:
                return 0;

            case ColumnMode::Middle:
                return scenario.EightyPercentClassColCount() / 2;

            case ColumnMode::Last:
                return scenario.EightyPercentClassColCount() - 1;

            default:
                BeAssert(false && "ColumnMode::All not allowed for GetSingleColIndex");
                return -1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearchTestFixture::ComputeValue(int rowId, int colIx)
    {
    switch (rowId % 3)
        {
            case 0:
                return 14352 * (colIx + 1);

            case 1:
                return -4553232 / (colIx + 1);

            case 2:
                return 9832423 + (1400305 + colIx);

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
    descr.Sprintf("%s,%s,%d", operation, scenario.ToCsvString().c_str(), initialRowCount);
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
    fileName.Sprintf("perfoverflowtable_%s_initialrowcount%d_%d.db", scenario.ToFileNameString().c_str(), s_initialRowCount,
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
    for (int i = 0; i < scenario.PrimaryTableColCount(); i++)
        {
        createTableSql.append(",");
        Utf8String colName = GetColumnName(i);
        if (i > 0)
            {
            insertSql.append(",");
            insertValuesSql.append(",");
            }

        createTableSql.append(colName).append(" INTEGER");
        insertSql.append(colName);
        if (i < scenario.EightyPercentClassColCount())
            insertValuesSql.append("random()/1000");
        else
            insertValuesSql.append("NULL");
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
        for (int i = 0; i < scenario.SecondaryTableColCount(); i++)
            {
            Utf8String colName = GetColumnName(i + scenario.PrimaryTableColCount());
            createTableSql.append(",").append(colName).append(" INTEGER");
            insertSql.append(",").append(colName);
            if ((i + scenario.PrimaryTableColCount()) < scenario.EightyPercentClassColCount())
                insertValuesSql.append(",random()/1000");
            else
                insertValuesSql.append(",NULL");
            }

        createTableSql.append(")");
        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        createTableSqls.push_back(createTableSql);
        insertSqls.push_back(insertSql);
        }

    if (scenario.TernaryTableColCount() > 0)
        {
        createTableSql.assign("CREATE TABLE t3(Id INTEGER PRIMARY KEY REFERENCES t2(Id) ON DELETE CASCADE");

        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");
        for (int i = 0; i < scenario.TernaryTableColCount(); i++)
            {
            Utf8String colName = GetColumnName(i + scenario.PrimaryTableColCount() + scenario.SecondaryTableColCount());
            createTableSql.append(",").append(colName).append(" INTEGER");
            insertSql.append(",").append(colName);
            if ((i + scenario.PrimaryTableColCount() + scenario.SecondaryTableColCount()) < scenario.EightyPercentClassColCount())
                insertValuesSql.append(",random()/1000");
            else
                insertValuesSql.append(",NULL");
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
PerformanceOverflowTablesResearchTestFixture::Scenario::Scenario(Utf8CP name, int primaryTableColCount, int secondaryTableUnsharedColCount, int secondaryTableMaxSharedColCount, int maxClassColCount, int eightyPercentClassColCount) : m_name(name)
    {
    const int neededSharedColCount = maxClassColCount - (primaryTableColCount + secondaryTableUnsharedColCount);
    if (neededSharedColCount < 0)
        {
        BeAssert(false && "invalid scenario");
        return;
        }

    m_primaryTableColCount = primaryTableColCount;
    m_secondaryTableUnsharedColCount = secondaryTableUnsharedColCount;
    m_secondaryTableMaxSharedColCount = secondaryTableMaxSharedColCount;
    m_maxClassColCount = maxClassColCount;
    m_eightyPercentClassColCount = eightyPercentClassColCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
int PerformanceOverflowTablesResearchTestFixture::Scenario::SecondaryTableColCount() const
    {
    if (!HasSecondaryTable())
        return 0;

    if (TotalSharedColCount() > m_secondaryTableMaxSharedColCount)
        return m_secondaryTableUnsharedColCount + m_secondaryTableMaxSharedColCount;

    return m_secondaryTableUnsharedColCount + TotalSharedColCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
int PerformanceOverflowTablesResearchTestFixture::Scenario::TernaryTableColCount() const
    {
    const int excessSharedColCount = TotalSharedColCount() - m_secondaryTableMaxSharedColCount;
    if (excessSharedColCount <= 0)
        return 0;

    return excessSharedColCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8String PerformanceOverflowTablesResearchTestFixture::Scenario::ToCsvString() const
    {
    Utf8String str;
    str.Sprintf("%s,%d,%d,%d,%d,%d,%d,%d",
                m_name.c_str(), m_primaryTableColCount, m_secondaryTableUnsharedColCount, m_secondaryTableMaxSharedColCount, m_maxClassColCount, m_eightyPercentClassColCount, SecondaryTableColCount(), TernaryTableColCount());
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8String PerformanceOverflowTablesResearchTestFixture::Scenario::ToFileNameString() const
    {
    Utf8String str;
    str.Sprintf("%s_%d_%d_%d_%d_%d",
                m_name.c_str(), m_primaryTableColCount, m_secondaryTableUnsharedColCount, m_secondaryTableMaxSharedColCount, m_maxClassColCount, m_eightyPercentClassColCount);
    return str;
    }


//*******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
struct PerformanceOverflowTablesResearch_NullColumnsTestFixture : ECDbTestFixture
    {
    protected:
        struct Scenario final
            {
            enum class Mode
                {
                FirstColumn,
                MiddleColumn,
                LastColumn,
                FirstEightyPercentColumns,
                AllColumns
                };

            int m_colCount = -1;
            Mode m_mode;
            
            Scenario(int colCount, Mode mode) : m_colCount(colCount), m_mode(mode) {}

            int GetOperationColCount() const 
                {
                switch (m_mode)
                    {
                        case Mode::FirstColumn:
                        case Mode::LastColumn:
                        case Mode::MiddleColumn:
                            return 1;

                        case Mode::AllColumns:
                            return m_colCount;

                        case Mode::FirstEightyPercentColumns:
                            return m_colCount * 80 / 100;

                        default:
                            BeAssert(false);
                            return -1;
                    }
                }
            bool IsSingleColumnMode() const { return m_mode == Mode::FirstColumn || m_mode == Mode::LastColumn || m_mode == Mode::MiddleColumn; }
            int GetSingleColIndex() const;
            Utf8CP ModeToString() const;
            };

    private:
        static const int s_initialRowCount = 1000;
        static const int s_opCount = 500;

        static void SetupTestDb(Db&, Scenario const&);
        static Utf8String GetColumnName(int colIndex);
        void LogTiming(StopWatch&, Utf8CP operation, Scenario const&, uint64_t fileSizeBefore, uint64_t fileSizeAfter, int initialRowCount, int opCount) const;

    protected:
        static std::vector<Scenario> GetTestScenarios()
            { 
            std::vector<Scenario> scenarios;
            for (int colCount : {30, 50, 80, 130, 210, 340, 550})
                {
                scenarios.push_back(Scenario(colCount, Scenario::Mode::MiddleColumn));
                scenarios.push_back(Scenario(colCount, Scenario::Mode::FirstEightyPercentColumns));
                scenarios.push_back(Scenario(colCount, Scenario::Mode::AllColumns));
                scenarios.push_back(Scenario(colCount, Scenario::Mode::FirstColumn));
                scenarios.push_back(Scenario(colCount, Scenario::Mode::LastColumn));
                }

            return scenarios;
            }

        void RunInsert(Scenario const&) const;
        void RunUpdate(Scenario const&) const;
        void RunDelete(Scenario const&) const;

    };


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearch_NullColumnsTestFixture::RunInsert(Scenario const& scenario) const
    {
    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    BeFileName testFilePath(db.GetDbFileName());
    uint64_t fileSizeBefore = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeBefore));

    Utf8String insertSql;
    if (scenario.IsSingleColumnMode())
        {
        const int colIx = scenario.GetSingleColIndex();
        insertSql.Sprintf("INSERT INTO t(%s) VALUES(random()/1000)", GetColumnName(colIx).c_str());
        }
    else
        {
        insertSql.assign("INSERT INTO t(");
        Utf8String valuesClause(") VALUES(");
        for (int i = 0; i < scenario.GetOperationColCount(); i++)
            {
            if (i > 0)
                {
                insertSql.append(",");
                valuesClause.append(",");
                }

            insertSql.append(GetColumnName(i));
            valuesClause.append("random()/1000");
            }

        insertSql.append(valuesClause).append(")");
        }

    StopWatch timer(true);
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Begin());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str() << " " << db.GetLastError().c_str();

    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetSql() << " " << db.GetLastError().c_str();

        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Commit());
    ASSERT_EQ(BE_SQLITE_OK, db.TryExecuteSql("VACUUM"));
    db.CloseDb();
    uint64_t fileSizeAfter = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeAfter));

    LogTiming(timer, "INSERT", scenario, fileSizeBefore, fileSizeAfter, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearch_NullColumnsTestFixture::RunUpdate(Scenario const& scenario) const
    {
    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    BeFileName testFilePath(db.GetDbFileName());
    uint64_t fileSizeBefore = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeBefore));

    Utf8String sql;
    if (scenario.IsSingleColumnMode())
        {
        const int colIx = scenario.GetSingleColIndex();
        sql.Sprintf("UPDATE t SET %s=random()/1000 WHERE Id=?", GetColumnName(colIx).c_str());
        }
    else
        {
        sql.assign("UPDATE t SET ");
        for (int i = 0; i < scenario.GetOperationColCount(); i++)
            {
            if (i > 0)
                sql.append(",");

            sql.append(GetColumnName(i)).append("=random()/1000");
            }

        sql.append(" WHERE Id=?");
        }

    StopWatch timer(true);
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Begin());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str() << " " << db.GetLastError().c_str();

    const int rowIdIncrement = s_initialRowCount / s_opCount;
    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        const int rowId = opNo * rowIdIncrement + 1;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(1, rowId)) << stmt.GetSql() << " " << db.GetLastError().c_str();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetSql() << " " << db.GetLastError().c_str();
        ASSERT_EQ(1, db.GetModifiedRowCount()) << "Col Count: " << scenario.m_colCount << " Column Mode: " << scenario.ModeToString() << " " << stmt.GetSql();
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Commit());
    ASSERT_EQ(BE_SQLITE_OK, db.TryExecuteSql("VACUUM"));
    db.CloseDb();
    uint64_t fileSizeAfter = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeAfter));
    LogTiming(timer, "UPDATE WHERE rowid=?", scenario, fileSizeBefore, fileSizeAfter, s_initialRowCount, s_opCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearch_NullColumnsTestFixture::RunDelete(Scenario const& scenario) const
    {
    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());

    BeFileName testFilePath(db.GetDbFileName());
    uint64_t fileSizeBefore = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeBefore));

    StopWatch timer(true);
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Begin());
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "DELETE FROM t WHERE Id=?")) << db.GetLastError().c_str();

    const int rowIdIncrement = s_initialRowCount / s_opCount;
    for (int opNo = 0; opNo < s_opCount; opNo++)
        {
        const int rowId = opNo * rowIdIncrement + 1;
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(1, rowId)) << stmt.GetSql() << " " << db.GetLastError().c_str();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetSql() << " " << db.GetLastError().c_str();
        ASSERT_EQ(1, db.GetModifiedRowCount()) << stmt.GetSql() << " " << db.GetLastError().c_str();
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, db.GetDefaultTransaction()->Commit());
    ASSERT_EQ(BE_SQLITE_OK, db.TryExecuteSql("VACUUM"));
    db.CloseDb();
    uint64_t fileSizeAfter = INT64_C(0);
    ASSERT_EQ(BeFileNameStatus::Success, testFilePath.GetFileSize(fileSizeAfter));
    LogTiming(timer, "DELETE WHERE rowid=?", scenario, fileSizeBefore, fileSizeAfter, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearch_NullColumnsTestFixture::LogTiming(StopWatch& timer, Utf8CP operation, Scenario const& scenario, uint64_t fileSizeBefore, uint64_t fileSizeAfter, int initialRowCount, int opCount) const
    {
    Utf8String descr;
    descr.Sprintf("%s,%d,%s,%" PRIu64 ",%" PRIu64 ",%d", operation, scenario.m_colCount, scenario.ModeToString(), fileSizeBefore, fileSizeAfter, initialRowCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
Utf8String PerformanceOverflowTablesResearch_NullColumnsTestFixture::GetColumnName(int colIndex)
    {
    Utf8String colName;
    colName.Sprintf("col%d", colIndex + 1);
    return colName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
//static
void PerformanceOverflowTablesResearch_NullColumnsTestFixture::SetupTestDb(Db& db, Scenario const& scenario)
    {
    Initialize();

    Utf8String fileName;
    fileName.Sprintf("perfnullcols_%d_%s_initialrowcount%d_%d.db", scenario.m_colCount, scenario.ModeToString(), s_initialRowCount,
                     DateTime::GetCurrentTimeUtc().GetDayOfYear());
    Utf8String seedFileName("seed_");
    seedFileName.append(fileName);

    BeFileName filePath = BuildECDbPath(fileName.c_str());
    BeFileName seedFilePath = BuildECDbPath(seedFileName.c_str());
    if (!seedFilePath.DoesPathExist())
        {
        Utf8String createTableSql("CREATE TABLE t(Id INTEGER PRIMARY KEY");
        for (int i = 0; i < scenario.m_colCount; i++)
            {
            createTableSql.append(",");
            Utf8String colName = GetColumnName(i);
            createTableSql.append(colName).append(" INTEGER");
            }

        createTableSql.append(")");

        Utf8String insertSql;
        if (scenario.IsSingleColumnMode())
            insertSql.Sprintf("INSERT INTO t(%s) VALUES(random()/1000)", GetColumnName(scenario.GetSingleColIndex()).c_str());
        else
            {
            insertSql.assign("INSERT INTO t(");
            Utf8String valuesClause(") VALUES(");
            for (int i = 0; i < scenario.GetOperationColCount(); i++)
                {
                if (i > 0)
                    {
                    insertSql.append(",");
                    valuesClause.append(",");
                    }

                Utf8String colName = GetColumnName(i);
                insertSql.append(colName);
                valuesClause.append("random()/1000");
                }

            insertSql.append(valuesClause).append(")");
            }

        Db seedDb;
        Db::CreateParams createParams;
        //we VACUUM the file after the set up is completed. Vacuuming requires that no transaction is active
        createParams.SetStartDefaultTxn(DefaultTxn::No);
        ASSERT_EQ(BE_SQLITE_OK, seedDb.CreateNewDb(seedFilePath, BeGuid(), createParams)) << seedFilePath.GetNameUtf8().c_str();

        seedDb.GetDefaultTransaction()->Begin();
        ASSERT_EQ(BE_SQLITE_OK, seedDb.ExecuteSql(createTableSql.c_str())) << createTableSql.c_str() << " " << seedDb.GetLastError().c_str();
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(seedDb, insertSql.c_str())) << insertSql.c_str() << " " << seedDb.GetLastError().c_str();

        for (int i = 0; i < s_initialRowCount; i++)
            {
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetSql() << " " << seedDb.GetLastError().c_str();
            stmt.Reset();
            }

        stmt.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, seedDb.GetDefaultTransaction()->Commit());
        ASSERT_FALSE(seedDb.GetDefaultTransaction()->IsActive());
        ASSERT_EQ(BE_SQLITE_OK, seedDb.TryExecuteSql("VACUUM")) << seedDb.GetLastError().c_str();
        seedDb.CloseDb();
        }

    if (filePath.DoesPathExist())
        {
        ASSERT_EQ(BeFileNameStatus::Success, filePath.BeDeleteFile()) << filePath.GetNameUtf8().c_str();
        }

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, filePath)) << filePath.GetNameUtf8().c_str();
    //we VACUUM the file after the set up is completed. Vacuuming requires that no transaction is active
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No))) << filePath.GetNameUtf8().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearch_NullColumnsTestFixture::Scenario::GetSingleColIndex() const
    {
    switch (m_mode)
        {
            case Mode::FirstColumn:
                return 0;

            case Mode::MiddleColumn:
                return m_colCount / 2;

            case Mode::LastColumn:
                return m_colCount - 1;

            default:
                BeAssert(false);
                return -1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8CP PerformanceOverflowTablesResearch_NullColumnsTestFixture::Scenario::ModeToString() const
    {
    switch (m_mode)
        {
            case Mode::FirstColumn:
                return "FirstColumn";
            case Mode::FirstEightyPercentColumns:
                return "FirstEightyPercent";
            case Mode::LastColumn:
                return "LastColumn";
            case Mode::MiddleColumn:
                return "MiddleColumn";
            case Mode::AllColumns:
                return "AllColumns";

            default:
                BeAssert(false);
                return "";
        }
    }
END_ECDBUNITTESTS_NAMESPACE
