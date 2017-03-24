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
            Utf8String m_name;
            int m_primaryTableColCount = 0;
            int m_secondaryTableColCount = 0;
            int m_ternaryTableColCount = 0;

            Scenario(Utf8CP name, int primaryTableColCount, int secondaryTableColCount, int ternaryTableColCount) : m_name(name), m_primaryTableColCount(primaryTableColCount), m_secondaryTableColCount(secondaryTableColCount > 0 ? secondaryTableColCount : 0), m_ternaryTableColCount(m_ternaryTableColCount > 0 ? m_ternaryTableColCount : 0) {}
            bool HasSecondaryTable() const { return m_secondaryTableColCount >= 0; }
            bool HasTernaryTable() const { return m_ternaryTableColCount >= 0; }
            int TableCount() const { if (!HasSecondaryTable()) return 1;  return HasTernaryTable() ? 3 : 2; }
            Utf8String ToString() const;
            };

        enum class SingleColumnMode
            {
            First,
            Middle,
            Last
            };

        static const int s_initialRowCount = 100000;
        static const int s_opCount = 50000;

    private:
        static int ComputeValue(int rowNo, int colNumber);
        static Utf8String GetColumnName(int colIndex);
        static int GetSingleColIndex(SingleColumnMode, int columnCount);
        void LogTiming(StopWatch&, Scenario const&, Utf8CP operation, int initialRowCount, int opCount) const;
        static void SetupTestDb(Db&, Scenario const&);

        static Utf8CP SingleColumnModeToString(SingleColumnMode);

    protected:
        void RunInsertAllCols(Scenario const&) const;
        void RunInsertSingleCol(Scenario const&, SingleColumnMode) const;
        void RunSelectSingleCol(Scenario const&, SingleColumnMode) const;
        void RunSelectWhereSingleCol(Scenario const&, SingleColumnMode, int opCount) const;
        void RunUpdateSingleCol(Scenario const&, SingleColumnMode) const;
        void RunUpdateAllCols(Scenario const&) const;
        void RunDelete(Scenario const&) const;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformanceOverflowTablesResearchTestFixture::RunInsertAllCols(Scenario const& scenario) const
    {
    ASSERT_GT(scenario.m_primaryTableColCount, 0) << scenario.m_name.c_str();

    Db db;
    SetupTestDb(db, scenario);
    ASSERT_TRUE(db.IsDbOpen());


    std::vector<Utf8String> sqls;

    Utf8String insertSql("INSERT INTO t1(");
    Utf8String insertValuesSql(") VALUES(");
    for (int i = 0; i < scenario.m_primaryTableColCount; i++)
        {
        if (i > 0)
            {
            insertSql.append(",");
            insertValuesSql.append(",");
            }

        insertSql.append(GetColumnName(i));
        insertValuesSql.append("?");
        }
    insertValuesSql.append(")");
    insertSql.append(insertValuesSql);
    sqls.push_back(insertSql);

    if (scenario.HasSecondaryTable())
        {
        insertSql.assign("INSERT INTO t2(Id");
        insertValuesSql.assign(") VALUES(?");

        for (int i = 0; i < scenario.m_secondaryTableColCount; i++)
            {
            insertSql.append(",").append(GetColumnName(i + scenario.m_primaryTableColCount));
            insertValuesSql.append(",").append("?");
            }

        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        sqls.push_back(insertSql);
        }

    if (scenario.HasTernaryTable())
        {
        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");

        for (int i = 0; i < scenario.m_ternaryTableColCount; i++)
            {
            insertSql.append(",").append(GetColumnName(i + scenario.m_primaryTableColCount + scenario.m_secondaryTableColCount));
            insertValuesSql.append(",").append("?");
            }

        insertValuesSql.append(")");
        insertSql.append(insertValuesSql);
        sqls.push_back(insertSql);
        }


    StopWatch timer(true);
    std::vector<std::unique_ptr<Statement>> stmts;
    for (Utf8StringCR sql : sqls)
        {
        std::unique_ptr<Statement> stmt = std::make_unique<Statement>();
        ASSERT_EQ(BE_SQLITE_OK, stmt->Prepare(db, sql.c_str())) << sql.c_str();
        stmts.push_back(std::move(stmt));
        }

    Statement& primaryStmt = *stmts[0];
    const size_t stmtCount = stmts.size();
    for (int i = 0; i < s_opCount; i++)
        {
        ASSERT_EQ(BE_SQLITE_DONE, primaryStmt.Step());
        const int64_t id = db.GetLastInsertRowId();
        primaryStmt.Reset();

        for (size_t j = 1; j < stmtCount; j++)
            {
            Statement& stmt = *stmts[j];
            ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt64(1, id));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Reset();
            stmt.ClearBindings();
            }
        }

    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "INSERT all cols", s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformanceOverflowTablesResearchTestFixture::GetSingleColIndex(SingleColumnMode mode, int columnCount)
    {
    switch (mode)
        {
            case SingleColumnMode::First:
                return 0;

            case SingleColumnMode::Last:
                return columnCount - 1;

            default:
                return columnCount / 2;
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
    Utf8String descr("Scenario: ");
    descr.Sprintf("%s - Scenario: %s (%s), initial row count %d", operation, scenario.ToString().c_str(), initialRowCount);

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP PerformanceOverflowTablesResearchTestFixture::SingleColumnModeToString(SingleColumnMode mode)
    {
    switch (mode)
        {
            case SingleColumnMode::First:
                return "First";
            case SingleColumnMode::Last:
                return "Last";
            case SingleColumnMode::Middle:
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
    fileName.Sprintf("perf_%s_initialrowcount%d_%d.db", scenario.m_name.c_str(), s_initialRowCount,
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
    for (int i = 0; i < scenario.m_primaryTableColCount; i++)
        {
        Utf8String colName = GetColumnName(i);
        if (i > 0)
            {
            createTableSql.append(",");
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

    if (scenario.m_secondaryTableColCount > 0)
        {
        createTableSql.assign("CREATE TABLE t2(Id INTEGER PRIMARY KEY REFERENCES t1(Id) ON CASCADE DELETE");
        insertSql.assign("INSERT INTO t2(Id");
        insertValuesSql.assign(") VALUES(?");
        for (int i = 0; i < scenario.m_secondaryTableColCount; i++)
            {
            Utf8String colName = GetColumnName(i + scenario.m_primaryTableColCount);
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

    if (scenario.m_ternaryTableColCount > 0)
        {
        createTableSql.assign("CREATE TABLE t3(Id INTEGER PRIMARY KEY REFERENCES t2(Id) ON CASCADE DELETE");
        insertSql.assign("INSERT INTO t3(Id");
        insertValuesSql.assign(") VALUES(?");
        for (int i = 0; i < scenario.m_ternaryTableColCount; i++)
            {
            Utf8String colName = GetColumnName(i + scenario.m_primaryTableColCount + scenario.m_secondaryTableColCount);
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
        ASSERT_EQ(BE_SQLITE_OK, seedDb.ExecuteSql(createTableSql.c_str())) << createTableSql.c_str();
        }

    std::vector<std::unique_ptr<Statement>> stmts;
    for (Utf8StringCR insertSql : insertSqls)
        {
        std::unique_ptr<Statement> stmt = std::make_unique<Statement>();
        ASSERT_EQ(BE_SQLITE_OK, stmt->Prepare(seedDb, insertSql.c_str())) << insertSql.c_str();
        stmts.push_back(std::move(stmt));
        }

    for (int i = 0; i < s_initialRowCount; i++)
        {
        ASSERT_EQ(BE_SQLITE_DONE, stmts[0]->Step()) << stmts[0]->GetSql() << " " << db.GetLastError().c_str();
        const int64_t id = db.GetLastInsertRowId();
        stmts[0]->Reset();

        for (size_t j = 1; j < stmts.size(); j++)
            {
            Statement& stmt = *stmts[j];
            ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt64(1, id)) << "Row#" << i + 1 << stmt.GetSql() << " " << db.GetLastError().c_str();
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Row#" << i + 1 << stmt.GetSql() << " " << db.GetLastError().c_str();
            stmt.ClearBindings();
            stmt.Reset();
            }
        }

    ASSERT_EQ(BE_SQLITE_OK, seedDb.SaveChanges());
    seedDb.CloseDb();

    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << filePath.GetNameUtf8().c_str();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
Utf8String PerformanceOverflowTablesResearchTestFixture::Scenario::ToString() const
    {
    Utf8String str;

    if (m_secondaryTableColCount < 0)
        {
        str.Sprintf("%s (table1: %d cols)", m_name.c_str(), m_primaryTableColCount);
        return str;
        }

    if (m_ternaryTableColCount < 0)
        {
        str.Sprintf("%s (table1: %d cols, table2: %d cols)", m_name.c_str(), m_primaryTableColCount, m_secondaryTableColCount);
        return str;
        }

    str.Sprintf("%s (table1: %d cols, table2: %d cols, table3: %d cols)", m_name.c_str(), m_primaryTableColCount, m_secondaryTableColCount, m_ternaryTableColCount);
    return str;
    }

END_ECDBUNITTESTS_NAMESPACE
