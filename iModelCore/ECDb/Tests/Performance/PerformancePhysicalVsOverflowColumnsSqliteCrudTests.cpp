/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformancePhysicalVsOverflowColumnsSqliteCrudTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define TESTTABLE_NAME "test"
#define TESTTABLE_OVERFLOWCOL "overflow"

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
struct PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture : ECDbTestFixture
    {
    protected:
        enum class Scenario
            {
            Physical,
            Overflow
            };

        static const int s_initialRowCount = 100000;
        static const int s_opCount = 50000;

    private:
        static int ComputeValue(int rowNo, int colNumber);
        static Utf8String GetColumnName(int colIndex);

        void LogTiming(StopWatch& timer, Scenario scenario, Utf8CP operation, int colCount, int initialRowCount, int opCount) const;
        static void SetupTestDb(Db& db, Scenario scenario, int colCount);

    protected:
        void RunInsertAllCols(Scenario scenario, int colCount) const;
        void RunInsertSingleCol(Scenario scenario, int colCount) const;
        void RunSelectSingleCol(Scenario scenario, int colCount) const;
        void RunSelectWhereSingleCol(Scenario scenario, int colCount, int opCount) const;
        void RunUpdateSingleCol(Scenario scenario, int colCount) const;
        void RunUpdateAllCols(Scenario scenario, int colCount) const;
        void RunDelete(Scenario scenario, int colCount) const;

        static std::vector<int> GetTestColumnCounts() { return std::vector<int> {1, 10, 30, 60}; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, InsertAllColumns)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunInsertAllCols(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunInsertAllCols(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, InsertSingleColumn)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunInsertSingleCol(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunInsertSingleCol(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, SelectSingleColumn)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunSelectSingleCol(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunSelectSingleCol(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, SelectWhereSingleColumn)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunSelectWhereSingleCol(Scenario::Physical, colCount, 5);
        }

    for (int colCount : testColCounts)
        {
        RunSelectWhereSingleCol(Scenario::Overflow, colCount, 5);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, UpdateAllColumns)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunUpdateAllCols(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunUpdateAllCols(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, UpdateSingleColumn)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunUpdateSingleCol(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunUpdateSingleCol(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture, Delete)
    {
    std::vector<int> testColCounts = GetTestColumnCounts();
    for (int colCount : testColCounts)
        {
        RunDelete(Scenario::Physical, colCount);
        }

    for (int colCount : testColCounts)
        {
        RunDelete(Scenario::Overflow, colCount);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunInsertAllCols(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    Utf8String insertSql("INSERT INTO " TESTTABLE_NAME "(");

    switch (scenario)
        {
            case Scenario::Overflow:
            {
            insertSql.append(TESTTABLE_OVERFLOWCOL).append(") VALUES(json_object(");

            for (int i = 0; i < colCount; i++)
                {
                if (i > 0)
                    insertSql.append(",");

                insertSql.append("'").append(GetColumnName(i)).append("',?");
                }

            insertSql.append("))");
            break;
            }

            case Scenario::Physical:
            {
            Utf8String insertValuesSql("VALUES(");
            for (int i = 0; i < colCount; i++)
                {
                if (i > 0)
                    {
                    insertSql.append(",");
                    insertValuesSql.append(",");
                    }

                insertSql.append(GetColumnName(i));
                insertValuesSql.append("?");
                }
            insertSql.append(") ").append(insertValuesSql).append(")");
            break;
            }

            default:
                FAIL();
        }


    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
    for (int i = 0; i < s_opCount; i++)
        {
        for (int j = 0; j < colCount; j++)
            {
            const int value = ComputeValue(i, j);
            stmt.BindInt(j + 1, value);
            }

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "INSERT all cols", colCount, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunInsertSingleCol(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    const int insertColNo = colCount / 2;
    Utf8String insertSql;
    switch (scenario)
        {
            case Scenario::Overflow:
            {
            insertSql.append("INSERT INTO " TESTTABLE_NAME "(").append(TESTTABLE_OVERFLOWCOL).append(") VALUES(json_object(");

            for (int i = 0; i < colCount; i++)
                {
                if (i > 0)
                    insertSql.append(",");

                insertSql.append("'").append(GetColumnName(i)).append("',");

                if (i == insertColNo)
                    insertSql.append("?");
                else
                    insertSql.append("null");
                }

            insertSql.append("))");
            break;
            }

            case Scenario::Physical:
            {
            insertSql.Sprintf("INSERT INTO " TESTTABLE_NAME "(%s) VALUES(?)", GetColumnName(insertColNo).c_str());
            break;
            }

            default:
                FAIL();
                break;
        }


    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, insertSql.c_str())) << insertSql.c_str();
    for (int i = 0; i < s_opCount; i++)
        {
        const int value = ComputeValue(i, insertColNo);
        stmt.BindInt(1, value);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    LogTiming(timer, scenario, "INSERT single col", colCount, s_initialRowCount, s_opCount);
    db.AbandonChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunSelectSingleCol(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    const int insertColNo = colCount / 2;
    Utf8String insertColname = GetColumnName(insertColNo);
    Utf8String sql;
    switch (scenario)
        {
            case Scenario::Overflow:
            {
            sql.Sprintf("SELECT json_extract(" TESTTABLE_OVERFLOWCOL ",'$.%s') FROM " TESTTABLE_NAME " WHERE rowid=?",
                        insertColname.c_str());
            break;
            }
            case Scenario::Physical:
            {
            sql.Sprintf("SELECT %s FROM " TESTTABLE_NAME " WHERE rowid=?", insertColname.c_str());
            break;
            }

            default:
                FAIL();
                break;
        }

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str();
    for (int i = 0; i < s_opCount; i++)
        {
        stmt.BindInt(1, i + 1);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        stmt.GetValueInt(0);
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(timer, scenario, "SELECT single col", colCount, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunSelectWhereSingleCol(Scenario scenario, int colCount, int opCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    const int insertColNo = colCount / 2;
    Utf8String insertColname = GetColumnName(insertColNo);
    Utf8String sql;
    switch (scenario)
        {
            case Scenario::Overflow:
            {
            sql.Sprintf("SELECT rowid FROM " TESTTABLE_NAME " WHERE json_extract(" TESTTABLE_OVERFLOWCOL ",'$.%s')=?",
                        insertColname.c_str());
            break;
            }
            case Scenario::Physical:
            {
            sql.Sprintf("SELECT rowid FROM " TESTTABLE_NAME " WHERE %s=?", insertColname.c_str());
            break;
            }

            default:
                FAIL();
                break;
        }

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str() << " " << db.GetLastError().c_str();
    for (int i = 0; i < opCount; i++)
        {
        const int value = ComputeValue(i, insertColNo);
        stmt.BindInt(1, value);
        stmt.Step();
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(timer, scenario, "SELECT WHERE single col", colCount, s_initialRowCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunUpdateSingleCol(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    const int insertColNo = colCount / 2;
    Utf8String insertColname = GetColumnName(insertColNo);
    Utf8String sql;
    switch (scenario)
        {
            case Scenario::Overflow:
            {
            sql.Sprintf("UPDATE " TESTTABLE_NAME " SET " TESTTABLE_OVERFLOWCOL "=json_set(" TESTTABLE_OVERFLOWCOL ",'$.%s',?) WHERE rowid=?",
                        insertColname.c_str());
            break;
            }
            case Scenario::Physical:
            {
            sql.Sprintf("UPDATE " TESTTABLE_NAME " SET %s=? WHERE rowId=?", insertColname.c_str());
            break;
            }

            default:
                FAIL();
                break;
        }

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str();
    for (int i = 0; i < s_opCount; i++)
        {
        stmt.BindInt(1, ComputeValue(i, insertColNo) + 1);
        stmt.BindInt(2, i + 1);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LogTiming(timer, scenario, "UPDATE single col", colCount, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunUpdateAllCols(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    Utf8String sql("UPDATE " TESTTABLE_NAME " SET ");

    switch (scenario)
        {
            case Scenario::Overflow:
            {
            sql.append(TESTTABLE_OVERFLOWCOL "=json_set(" TESTTABLE_OVERFLOWCOL);
            for (int i = 0; i < colCount; i++)
                {
                sql.append(", '$.").append(GetColumnName(i)).append("',?");
                }

            sql.append(") WHERE rowId=?");
            break;
            }

            case Scenario::Physical:
            {
            for (int i = 0; i < colCount; i++)
                {
                if (i > 0)
                    sql.append(",");

                sql.append(GetColumnName(i)).append("=?");
                }

            sql.append(" WHERE rowId=?");
            break;
            }

            default:
                FAIL();
        }


    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql.c_str())) << sql.c_str();
    for (int i = 0; i < s_opCount; i++)
        {
        for (int j = 0; j < colCount; j++)
            {
            const int value = ComputeValue(i, j);
            stmt.BindInt(j + 1, value);
            }

        stmt.BindInt(colCount + 1, i + 1); //where rowid=?
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "UPDATE all cols", colCount, s_initialRowCount, s_opCount);
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                  Krischan.Eberle     01/2017
    //---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::RunDelete(Scenario scenario, int colCount) const
    {
    Db db;
    SetupTestDb(db, scenario, colCount);
    ASSERT_TRUE(db.IsDbOpen());

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "DELETE FROM " TESTTABLE_NAME " WHERE rowid=?"));
    for (int i = 0; i < s_opCount; i++)
        {
        stmt.BindInt(1, i + 1); //where rowid=?
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    db.AbandonChanges();
    LogTiming(timer, scenario, "DELETE", colCount, s_initialRowCount, s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
int PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::ComputeValue(int rowNo, int colNumber)
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
Utf8String PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::GetColumnName(int colIndex)
    {
    Utf8String colName;
    colName.Sprintf("col%d", colIndex + 1);
    return colName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::LogTiming(StopWatch& timer, Scenario scenario, Utf8CP operation, int colCount, int initialRowCount, int opCount) const
    {
    Utf8String descr;
    descr.Sprintf("%s, %s, %d columns, initial row count %d", operation, scenario == Scenario::Overflow ? "Overflow" : "Physical",
                  colCount, initialRowCount);

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), descr.c_str(), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
void PerformancePhysicalVsOverflowColumnsSqliteCrudTestFixture::SetupTestDb(Db& db, Scenario scenario, int colCount)
    {
    Initialize();

    Utf8String fileName;
    fileName.Sprintf("perf_%s_colcount%d_initialrowcount%d_%d.db", scenario == Scenario::Overflow ? "overflow" : "physical", colCount, s_initialRowCount,
                     DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName filePath = ECDbTestUtility::BuildECDbPath(fileName.c_str());

    if (!filePath.DoesPathExist())
        {
        Utf8String createTableSql, insertSql;
        switch (scenario)
            {
                case Scenario::Overflow:
                {
                createTableSql.assign("CREATE TABLE " TESTTABLE_NAME "(Id INTEGER PRIMARY KEY, " TESTTABLE_OVERFLOWCOL " TEXT)");
                insertSql.assign("INSERT INTO " TESTTABLE_NAME "(" TESTTABLE_OVERFLOWCOL ") VALUES(json_object(");

                for (int i = 0; i < colCount; i++)
                    {
                    Utf8String colName = GetColumnName(i);
                    if (i > 0)
                        insertSql.append(",");

                    insertSql.append("'").append(GetColumnName(i)).append("', random()");
                    }
                insertSql.append("))");
                break;
                }

                case Scenario::Physical:
                {
                createTableSql.assign("CREATE TABLE " TESTTABLE_NAME "(Id INTEGER PRIMARY KEY,");
                insertSql.assign("INSERT INTO " TESTTABLE_NAME "(");
                Utf8String insertValuesSql("VALUES(");

                for (int i = 0; i < colCount; i++)
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
                    insertValuesSql.append("random()");
                    }

                createTableSql.append(")");
                insertSql.append(") ").append(insertValuesSql).append(")");
                break;
                }

                default:
                    FAIL();
            }

        Db seedDb;
        ASSERT_EQ(BE_SQLITE_OK, seedDb.CreateNewDb(filePath)) << filePath.GetNameUtf8().c_str();
        ASSERT_EQ(BE_SQLITE_OK, seedDb.ExecuteSql(createTableSql.c_str())) << createTableSql.c_str();

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(seedDb, insertSql.c_str())) << insertSql.c_str();
        for (int i = 0; i < s_initialRowCount; i++)
            {
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Row #" << i << " Error: " << seedDb.GetLastError().c_str();
            stmt.Reset();
            stmt.ClearBindings();
            }

        stmt.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, seedDb.SaveChanges());
        seedDb.CloseDb();
        }

    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite))) << filePath.GetNameUtf8().c_str();
    }

END_ECDBUNITTESTS_NAMESPACE