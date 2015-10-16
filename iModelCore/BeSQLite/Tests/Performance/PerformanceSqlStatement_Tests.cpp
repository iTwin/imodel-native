/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSqlStatement_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#ifdef WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS

#if defined (BENTLEY_WIN32)
    #include <Windows.h>
#endif

#include "BeSQLitePerformanceTests.h"

#include <Bentley/bvector.h>
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

#define TESTTABLE_ID_COLUMNNAME "id"
#define TESTTABLE_STRINGCOL_COLUMNNAME "stringcol"
#define TESTTABLE_DOUBLECOL_COLUMNNAME "doublecol"
#define TESTTABLE_INTCOL_COLUMNNAME "intcol"
#define TESTTABLE_BLOBCOL_COLUMNNAME "blobcol"

struct PerformanceSqlStatementTests : public PerformanceTestFixtureBase
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void InsertRows(Db& db, Utf8CP tableName, int rowCount)
    {
        if (rowCount == 0)
        {
            return;
        }

        Utf8String sql;
        sql.Sprintf("INSERT INTO %s (" TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME ") VALUES (?, ?, ?, ?)", tableName);

        Statement stmt;
        DbResult stat = stmt.Prepare(db, sql.c_str());
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing statement " << sql.c_str() << " failed.";
        for (int i = 0; i < rowCount; i++)
        {
            stmt.Reset();
            stmt.ClearBindings();
            stmt.BindInt64(1, static_cast<int64_t> (i));
            stmt.BindText(2, "blabla", Statement::MakeCopy::Yes);
            stmt.BindDouble(3, i * 3.1415);
            stmt.BindInt(4, i * 123);

            stat = stmt.Step();
            ASSERT_EQ(BE_SQLITE_DONE, stat) << "Executing Statement " << stmt.GetSql() << " failed.";
        }
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   03/13
    //+---------------+---------------+---------------+---------------+---------------+------
    void CreateTables
        (
        bvector<Utf8String>& tableNames,
        Db& db,
        int tableCount,
        int rowCount,
        bool addBlobColumn,
        bool createPK,
        bool createAdditionalIndices
        )
    {
        for (int i = 0; i < tableCount; i++)
        {
            Utf8String tableNameStr;
            tableNameStr.Sprintf("testtable%d", i);
            Utf8CP const tableName = tableNameStr.c_str();

            Utf8CP blobColumnExpression = addBlobColumn ? ", " TESTTABLE_BLOBCOL_COLUMNNAME " BLOB" : "";
            Utf8CP pkExpression = createPK ? "PRIMARY KEY" : "";
            Utf8String sql;
            sql.Sprintf("CREATE TABLE %s (" TESTTABLE_ID_COLUMNNAME " INTEGER %s, " TESTTABLE_STRINGCOL_COLUMNNAME " TEXT, " TESTTABLE_DOUBLECOL_COLUMNNAME " REAL, " TESTTABLE_INTCOL_COLUMNNAME " INTEGER%s);", tableName, pkExpression, blobColumnExpression);

            DbResult stat = db.TryExecuteSql(sql.c_str());
            ASSERT_EQ(BE_SQLITE_OK, stat) << "Creating test table " << tableName << " failed. Error code: DbResult::" << stat;
            tableNames.push_back(tableNameStr);

            InsertRows(db, tableName, rowCount);

            if (createAdditionalIndices)
            {
                sql.Sprintf("CREATE INDEX idx_%s ON %s (" TESTTABLE_INTCOL_COLUMNNAME ");", tableName, tableName);
                stat = db.TryExecuteSql(sql.c_str());
                ASSERT_EQ(BE_SQLITE_OK, stat) << "Creating index on test table " << tableName << " failed. Error code: DbResult::" << stat;
            }
        }

        //shuffle the table names in the list so that the tests don't process them in the same order as they were created in the db.
        std::random_shuffle(tableNames.begin(), tableNames.end());

        ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   03/13
    //+---------------+---------------+---------------+---------------+---------------+------
    void CreateTestDgnDb
        (
        Utf8String& dbPath,
        bvector<Utf8String>& testTableNames,
        WCharCP dbName,
        int testTableCount,
        int rowCount,
        bool addBlobColumn,
        bool createPK,
        bool createAdditionalIndices
        )
    {
        BeFileName dbFileName;
        BeTest::GetHost().GetOutputRoot(dbFileName);
        dbFileName.AppendToPath(dbName);
        if (BeFileName::DoesPathExist(dbFileName))
        {
            BeFileName::BeDeleteFile(dbFileName);
        }

        Utf8String dbFilePath = dbFileName.GetNameUtf8();
        Db db;
        DbResult stat = db.CreateNewDb(dbFilePath.c_str());
        ASSERT_EQ(BE_SQLITE_OK, stat) << L"Creation of test DgnDb failed";

        CreateTables(testTableNames, db, testTableCount, rowCount, addBlobColumn, createPK, createAdditionalIndices);

        db.CloseDb();
        dbPath = dbFilePath;

#if defined (BENTLEY_WIN32)
        //raises current process' priority which might give more consistent performance test results
        HANDLE hCurrentProcess = GetCurrentProcess();
        SetPriorityClass(hCurrentProcess, ABOVE_NORMAL_PRIORITY_CLASS);
#endif
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void CreateTestDgnDb
        (
        Utf8String& dbPath,
        bvector<Utf8String>& testTableNames,
        WCharCP dbName,
        int testTableCount,
        int rowCount,
        bool createPK,
        bool createAdditionalIndices
        )
    {
        CreateTestDgnDb(dbPath, testTableNames, dbName, testTableCount, rowCount, false, createPK, createAdditionalIndices);
    }

    //**************************** Performance of prepare statement *************************************************
    //---------------------------------------------------------------------------------------
    // @bsimethod                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimePrepareStatement
        (
        DbR db,
        Utf8CP sqlTemplate,
        bvector<Utf8String>& testTableNames,
        int repetitionsPerTable,
        DbResult expectedResult
        )
    {
        Utf8String sql;
        m_stopwatch.Start();
        for (int i = 0; i < repetitionsPerTable; i++)
        {
            FOR_EACH(Utf8StringCR testTableName, testTableNames)
            {
                sql.Sprintf(sqlTemplate, testTableName.c_str());
                Statement stmt;
                DbResult stat = stmt.TryPrepare(db, sql.c_str());
                EXPECT_EQ(expectedResult, stat) << "Unexpected result of preparing '" << sql.c_str() << "'.";
            }
        }

        m_stopwatch.Stop();
        const int64_t totalExecutionCount = repetitionsPerTable * testTableNames.size();
        sql.Sprintf(sqlTemplate, "Foo");
        PERFORMANCELOG.infov("Preparing '%s' %lld times took %.4f secs.", sql.c_str(), totalExecutionCount, m_stopwatch.GetElapsedSeconds());
    }
    //************************ Determine emptiness of tables Tests ************************************************
    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimeExecuteSelectCountStatement
        (
        Db& db,
        bvector<Utf8String>& testTableNames,
        int rowCount,
        Utf8CP sqlTemplate,
        int repetitions,
        Utf8String& logMessageHeader
        )
    {
        const bool expectedIsEmpty = rowCount == 0;
        m_stopwatch.Start();
        Utf8String sql;
        for (int i = 0; i < repetitions; i++)
        {
            FOR_EACH(Utf8String& tableName, testTableNames)
            {
                sql.Sprintf(sqlTemplate, tableName.c_str());
                Statement stmt;
                DbResult stat = stmt.Prepare(db, sql.c_str());
                ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing SQL failed for " << sql.c_str();

                stat = stmt.Step();
                ASSERT_EQ(BE_SQLITE_ROW, stat) << "Executing '" << sql.c_str() << "' failed";
                const bool actualIsEmpty = (stmt.GetValueInt(0) == 0);
                EXPECT_EQ(expectedIsEmpty, actualIsEmpty) << "Table " << tableName.c_str();
            }
        }

        m_stopwatch.Stop();
        Utf8String sampleSql;
        sampleSql.Sprintf(sqlTemplate, "Foo");

        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sampleSql, repetitions);
        PERFORMANCELOG.infov("%hs%10.4f   %s", logMessageHeader.c_str(), m_stopwatch.GetElapsedSeconds(), sampleSql.c_str());
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimeExecuteSelectLimit1Statement
        (
        Db& db,
        bvector<Utf8String>& testTableNames,
        int rowCount,
        Utf8CP sqlTemplate,
        int repetitions,
        Utf8String& logMessageHeader
        )
    {
        m_stopwatch.Start();
        Utf8String sql;
        for (int i = 0; i < repetitions; i++)
        {
            FOR_EACH(Utf8String& tableName, testTableNames)
            {
                sql.Sprintf(sqlTemplate, tableName.c_str());
                Statement stmt;
                DbResult stat = stmt.Prepare(db, sql.c_str());
                ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing SQL failed for " << sql.c_str();

                stat = stmt.Step();
                if (rowCount == 0)
                {
                    EXPECT_EQ(BE_SQLITE_DONE, stat) << "Table " << tableName.c_str() << " is expected to be empty";
                }
                else
                {
                    EXPECT_EQ(BE_SQLITE_ROW, stat) << "Table " << tableName.c_str() << " is expected to be non-empty";;
                }
            }
        }

        m_stopwatch.Stop();

        Utf8String sampleSql;
        sampleSql.Sprintf(sqlTemplate, "Foo");
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sampleSql, repetitions);
        PERFORMANCELOG.infov("%hs%10.4f   %s", logMessageHeader.c_str(), m_stopwatch.GetElapsedSeconds(), sampleSql.c_str());
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void RunCheckEmptiness
        (
        int tableCount,
        int rowCount,
        bool createPK,
        bool createIndex
        )
    {
        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        BeSQLiteLib::Initialize(temporaryDir);

        Utf8String dbPath;
        bvector<Utf8String> testTableNames;
        CreateTestDgnDb(dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, createPK, createIndex);

        ASSERT_EQ((size_t)tableCount, testTableNames.size()) << "Count of test tables created doesn't match the expected value.";

        Db db;
        DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

        bvector<Utf8String> selectCountSqls;
        selectCountSqls.push_back("SELECT count(*) FROM %s;");
        selectCountSqls.push_back("SELECT count(rowid) FROM %s;");
        selectCountSqls.push_back("SELECT count(doublecol) FROM %s;");
        selectCountSqls.push_back("SELECT count(intcol) FROM %s;");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM %s);");
        selectCountSqls.push_back("SELECT EXISTS (select 0 FROM %s);");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM %s LIMIT 1);");
        //with where clause
        selectCountSqls.push_back("SELECT count(*) FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000;");
        selectCountSqls.push_back("SELECT count(rowid) FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000;");
        selectCountSqls.push_back("SELECT count(intcol) FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000;");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000);");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000 LIMIT 1);");

        bvector<Utf8String> selectLimit1Sqls;
        selectLimit1Sqls.push_back("SELECT NULL FROM %s LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT 0 FROM %s LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT rowid FROM %s LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT intcol FROM %s LIMIT 1;");
        //with where clause
        selectLimit1Sqls.push_back("SELECT NULL FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000 LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT 0 FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000 LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT rowid FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000 LIMIT 1;");
        selectLimit1Sqls.push_back("SELECT intcol FROM %s WHERE " TESTTABLE_INTCOL_COLUMNNAME " < 1000 LIMIT 1;");

        bvector<int> repetitionsList;
        repetitionsList.push_back(1);
        repetitionsList.push_back(100);

        FOR_EACH(int repetitions, repetitionsList)
        {
            LOG.info("Tables - Rows - PK - Index - Repetitions per table - Timing [s] - SQL");
            Utf8String logMessageHeader;
            logMessageHeader.Sprintf("%6d   %4d    %d   %5d   %17d     ", testTableNames.size(), rowCount, createPK, createIndex, repetitions);
            FOR_EACH(Utf8String& sql, selectCountSqls)
            {
                TimeExecuteSelectCountStatement(db, testTableNames, rowCount, sql.c_str(), repetitions, logMessageHeader);
            }

            FOR_EACH(Utf8String& sql, selectLimit1Sqls)
            {
                TimeExecuteSelectLimit1Statement(db, testTableNames, rowCount, sql.c_str(), repetitions, logMessageHeader);
            }
        }
    }

    //************************ Table existence Tests ************************************************
    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void CreateListOfTablesToCheckForExistence
        (
        bvector<Utf8String>& tablesToCheck,
        bvector<Utf8String>& existingTables,
        bool checkForExistingTables
        )
    {
        if (checkForExistingTables)
        {
            tablesToCheck = existingTables;
        }
        else
        {
            FOR_EACH(Utf8String& existingTable, existingTables)
            {
                Utf8String nonExistingTable;
                nonExistingTable.Sprintf("%s_notexisting", existingTable.c_str());
                tablesToCheck.push_back(nonExistingTable);
            }
        }
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimeExecuteSelectLimit1StatementForTableExistence
        (
        Db& db,
        bool checkForExistingTables,
        bvector<Utf8String>& existingTables,
        Utf8CP sql,
        int repetitions,
        Utf8String& logMessageHeader
        )
    {
        bvector<Utf8String> tablesToCheck;
        CreateListOfTablesToCheckForExistence(tablesToCheck, existingTables, checkForExistingTables);

        int64_t executionCount = 0LL;

        m_stopwatch.Start();
        Statement stmt;
        DbResult stat = stmt.Prepare(db, sql);
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing SQL failed for " << sql;

        for (int i = 0; i < repetitions; i++)
        {
            FOR_EACH(Utf8String& tableName, tablesToCheck)
            {
                stmt.BindText(1, tableName.c_str(), Statement::MakeCopy::No);
                stat = stmt.Step();
                stmt.Reset();
                stmt.ClearBindings();

                executionCount++;
                if (checkForExistingTables)
                {
                    EXPECT_EQ(BE_SQLITE_ROW, stat) << "Table " << tableName.c_str() << " is expected to exist";
                }
                else
                {
                    EXPECT_EQ(BE_SQLITE_DONE, stat) << "Table " << tableName.c_str() << " is not expected to exist";;
                }
            }
        }

        m_stopwatch.Stop();
        const size_t tableCount = tablesToCheck.size();
        EXPECT_EQ(tableCount * repetitions, executionCount) << "Unexpected execution count.";
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sql, repetitions);
        PERFORMANCELOG.infov("%hs%10.4f   %s", logMessageHeader.c_str(), m_stopwatch.GetElapsedSeconds(), sql);
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimeExecuteSelectCountStatementForTableExistence
        (
        Db& db,
        bool checkForExistingTables,
        bvector<Utf8String>& existingTables,
        Utf8CP sql,
        int repetitions,
        Utf8String& logMessageHeader
        )
    {
        bvector<Utf8String> tablesToCheck;
        CreateListOfTablesToCheckForExistence(tablesToCheck, existingTables, checkForExistingTables);

        const int expectedRowCount = checkForExistingTables ? 1 : 0;

        int64_t executionCount = 0LL;

        m_stopwatch.Start();
        Statement stmt;
        DbResult stat = stmt.Prepare(db, sql);
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing SQL failed for " << sql;

        for (int i = 0; i < repetitions; i++)
        {
            FOR_EACH(Utf8String& tableName, tablesToCheck)
            {
                stmt.BindText(1, tableName.c_str(), Statement::MakeCopy::No);
                stat = stmt.Step();
                ASSERT_EQ(BE_SQLITE_ROW, stat) << "Executing '" << sql << "' for table " << tableName.c_str() << " failed";
                const int actualRowCount = stmt.GetValueInt(0);
                EXPECT_EQ(expectedRowCount, actualRowCount) << "Unexpected result for '" << sql << "' for table " << tableName.c_str();
                stmt.Reset();
                stmt.ClearBindings();
                executionCount++;
            }
        }

        m_stopwatch.Stop();
        const size_t tableCount = tablesToCheck.size();
        EXPECT_EQ(tableCount * repetitions, executionCount) << "Unexpected execution count.";
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sql, repetitions);
        PERFORMANCELOG.infov("%s%10.4f   %s", logMessageHeader.c_str(), m_stopwatch.GetElapsedSeconds(), sql);
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void TimePrepareFailureApproachForTableExistence
        (
        Db& db,
        bool checkForExistingTables,
        bvector<Utf8String>& existingTables,
        Utf8CP sqlTemplate,
        int repetitions,
        Utf8String& logMessageHeader
        )
    {
        bvector<Utf8String> tablesToCheck;
        CreateListOfTablesToCheckForExistence(tablesToCheck, existingTables, checkForExistingTables);

        const DbResult expectedPrepareStat = checkForExistingTables ? BE_SQLITE_OK : BE_SQLITE_ERROR;

        Utf8String sql;

        int64_t executionCount = 0LL;

        m_stopwatch.Start();
        for (int i = 0; i < repetitions; i++)
        {
            FOR_EACH(Utf8String& tableName, tablesToCheck)
            {
                sql.Sprintf(sqlTemplate, tableName.c_str());
                LOG.tracev("TryPrepare on %s.", sql.c_str());
                Statement stmt;
                DbResult actualStat = stmt.TryPrepare(db, sql.c_str());
                executionCount++;

                ASSERT_EQ(expectedPrepareStat, actualStat) << "Unexpected return value from Prepare for " << sql.c_str();
            }
        }

        m_stopwatch.Stop();

        const size_t tableCount = tablesToCheck.size();
        EXPECT_EQ(tableCount * repetitions, executionCount) << "Unexpected execution count.";

        Utf8String sampleSql;
        sampleSql.Sprintf(sqlTemplate, "Foo");
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sql, repetitions);
        PERFORMANCELOG.infov("%s%10.4f   %s", logMessageHeader.c_str(), m_stopwatch.GetElapsedSeconds(), sampleSql.c_str());
    }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                     Krischan.Eberle                  12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void RunCheckTableExistence
        (
        int tableCount,
        int rowCount,
        bool checkForExistingTables
        )
    {
        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        BeSQLiteLib::Initialize(temporaryDir);

        Utf8String dbPath;
        bvector<Utf8String> testTableNames;
        CreateTestDgnDb(dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, true, false);

        ASSERT_EQ((size_t)tableCount, testTableNames.size()) << "Count of test tables created doesn't match the expected value.";

        Db db;
        DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

        bvector<Utf8String> selectCountSqls;
        selectCountSqls.push_back("SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM sqlite_master WHERE type = 'table' AND name = ?);");
        selectCountSqls.push_back("SELECT EXISTS (select null FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1);");

        bvector<Utf8String> selectLimit1Sqls;
        selectLimit1Sqls.push_back("SELECT NULL FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1");
        selectLimit1Sqls.push_back("SELECT 0 FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1");

        bvector<Utf8String> prepareFailsSqls;
        prepareFailsSqls.push_back("SELECT NULL FROM %s");
        prepareFailsSqls.push_back("SELECT NULL FROM %s LIMIT 1");

        bvector<int> repetitionsList;
        repetitionsList.push_back(1);
        repetitionsList.push_back(100);

        FOR_EACH(int repetitions, repetitionsList)
        {
            LOG.info("Tables - Tables exist - Repetitions per table - Timing [s] - SQL");
            Utf8String logMessageHeader;
            logMessageHeader.Sprintf("%6d   %12d    %17d     ", testTableNames.size(), checkForExistingTables, repetitions);

            FOR_EACH(Utf8String& sql, selectCountSqls)
            {
                TimeExecuteSelectCountStatementForTableExistence(db, checkForExistingTables, testTableNames, sql.c_str(), repetitions, logMessageHeader);
            }

            FOR_EACH(Utf8String& sql, selectLimit1Sqls)
            {
                TimeExecuteSelectLimit1StatementForTableExistence(db, checkForExistingTables, testTableNames, sql.c_str(), repetitions, logMessageHeader);
            }

            FOR_EACH(Utf8String& sql, prepareFailsSqls)
            {
                TimePrepareFailureApproachForTableExistence(db, checkForExistingTables, testTableNames, sql.c_str(), repetitions, logMessageHeader);
            }
        }
    }
    //************************ DROP TABLE Tests ************************************************

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void RunExecuteDropTablesWithMultipleCallsTest
        (
        int tableCount,
        int rowCount,
        bool createIndices
        )
    {
        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        BeSQLiteLib::Initialize(temporaryDir);

        Utf8String dbPath;
        bvector<Utf8String> testTableNames;
        CreateTestDgnDb(dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, true, createIndices);

        bvector<Utf8String> dropTableSqlList;
        FOR_EACH(Utf8String& tableName, testTableNames)
        {
            SqlPrintfString sql("DROP TABLE %s;", tableName.c_str());
            dropTableSqlList.push_back(Utf8String(sql.GetUtf8CP()));
        }

        Db db;
        DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

        m_stopwatch.Start();
        Savepoint savepoint(db, "MultipleExecution");
        FOR_EACH(Utf8String& sql, dropTableSqlList)
        {
            stat = db.TryExecuteSql(sql.c_str());
            ASSERT_EQ(BE_SQLITE_OK, stat) << "SQL '" << sql.c_str() << "' failed.";
        }

        savepoint.Commit();
        m_stopwatch.Stop();
        db.CloseDb();
        if (createIndices)
        {
            PERFORMANCELOG.infov("Dropping %d tables [%d rows; one index besides PK] with multiple calls took %.4f seconds.", tableCount, rowCount, m_stopwatch.GetElapsedSeconds());
            LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
        }
        else
        {
            PERFORMANCELOG.infov("Dropping %d tables [%d rows; no index besides PK] with multiple calls took %.4f seconds.", tableCount, rowCount, m_stopwatch.GetElapsedSeconds());
            LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
        }
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Krischan.Eberle                   12/12
    //+---------------+---------------+---------------+---------------+---------------+------
    void RunExecuteDropTablesWithSingleCallsTest
        (
        int tableCount,
        int rowCount,
        bool createIndices
        )
    {
        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        BeSQLiteLib::Initialize(temporaryDir);

        Utf8String dbPath;
        bvector<Utf8String> testTableNames;
        CreateTestDgnDb(dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, true, createIndices);

        Utf8String dropTableSql;
        FOR_EACH(Utf8String& tableName, testTableNames)
        {
            SqlPrintfString sql("DROP TABLE %s;", tableName.c_str());
            dropTableSql.append(sql.GetUtf8CP());
        }

        Db db;
        DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

        m_stopwatch.Start();
        Savepoint savepoint(db, "SingleExecution");
        stat = db.TryExecuteSql(dropTableSql.c_str());
        ASSERT_EQ(BE_SQLITE_OK, stat) << "SQL '" << dropTableSql.c_str() << "' failed.";
        savepoint.Commit();
        m_stopwatch.Stop();
        db.CloseDb();
        if (createIndices)
        {
            PERFORMANCELOG.infov("Dropping %d tables [%d rows; one index besides PK] with a single call took %.4f seconds.", tableCount, rowCount, m_stopwatch.GetElapsedSeconds());
            LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
        }
        else
        {
            PERFORMANCELOG.infov("Dropping %d tables [%d rows; no index besides PK] with a single call took %.4f seconds.", tableCount, rowCount, m_stopwatch.GetElapsedSeconds());
            LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
        }
    }


}; // End of Test Fixture

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, PrepareStatement)
    {
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    BeSQLiteLib::Initialize (temporaryDir);

    const int tableCount = 500;
    const int rowCount = 100;
    Utf8String dbPath;
    bvector<Utf8String> testTableNames;
    CreateTestDgnDb (dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, true, true);      

    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

    const int repetitionsPerTable = 1000;

    bvector<Utf8String> sqlList;

    //SELECT
    sqlList.push_back ("select * from %s");

    //all columns explicitly
    sqlList.push_back ("select " TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s");

    //all columns ordered differently
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME ", " TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME " from %s");

    //with where clause
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s where " TESTTABLE_ID_COLUMNNAME " > 20 AND " TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%%la%%'");

    //with where clause and order by
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s where " TESTTABLE_ID_COLUMNNAME " > 20 AND " TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%la%' ORDER BY " TESTTABLE_DOUBLECOL_COLUMNNAME " DESC");

    //with join
    sqlList.push_back ("select t1." TESTTABLE_DOUBLECOL_COLUMNNAME ", t2." TESTTABLE_INTCOL_COLUMNNAME " from %s t1, testtable15 t2 where t1." TESTTABLE_ID_COLUMNNAME " = t2." TESTTABLE_ID_COLUMNNAME " + 2 AND t1." TESTTABLE_ID_COLUMNNAME " > 20 AND t2." TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%la%' ORDER BY t1." TESTTABLE_DOUBLECOL_COLUMNNAME " DESC");

    //INSERT
    sqlList.push_back ("insert into %s (" TESTTABLE_STRINGCOL_COLUMNNAME " ," TESTTABLE_DOUBLECOL_COLUMNNAME " ," TESTTABLE_INTCOL_COLUMNNAME ") VALUES (?, ?, ?)");

    //UPDATE
    sqlList.push_back ("update %s set " TESTTABLE_STRINGCOL_COLUMNNAME " = ?, " TESTTABLE_DOUBLECOL_COLUMNNAME " = -4.123 WHERE " TESTTABLE_DOUBLECOL_COLUMNNAME " > 5.12345");

    FOR_EACH (Utf8StringCR sql, sqlList)
        {
        TimePrepareStatement (db, sql.c_str (), testTableNames, repetitionsPerTable, BE_SQLITE_OK);
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sql, repetitionsPerTable);
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, PrepareStatementWithBlobColumns)
    {
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    BeSQLiteLib::Initialize (temporaryDir);

    const int tableCount = 500;
    const int rowCount = 100;
    Utf8String dbPath;
    bvector<Utf8String> testTableNames;
    CreateTestDgnDb (dbPath, testTableNames, L"SqlStatementPerformance.idgndb", tableCount, rowCount, true, true, true);      

    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

    const int repetitionsPerTable = 1000;

    bvector<Utf8String> sqlList;

    //SELECT
    sqlList.push_back ("select * from %s");

    //without blob columns 
    sqlList.push_back ("select " TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s");

    //with blob columns 
    sqlList.push_back ("select " TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_BLOBCOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s");

    //all columns ordered differently
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME ", " TESTTABLE_ID_COLUMNNAME ", " TESTTABLE_STRINGCOL_COLUMNNAME " from %s");

    //with where clause
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s where " TESTTABLE_ID_COLUMNNAME " > 20 AND " TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%%la%%'");
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s where " TESTTABLE_ID_COLUMNNAME " > 20 AND " TESTTABLE_BLOBCOL_COLUMNNAME " IS NOT NULL");

    //with where clause and order by
    sqlList.push_back ("select " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME " from %s where " TESTTABLE_ID_COLUMNNAME " > 20 AND " TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%la%' ORDER BY " TESTTABLE_DOUBLECOL_COLUMNNAME " DESC");

    //with join
    sqlList.push_back ("select t1." TESTTABLE_DOUBLECOL_COLUMNNAME ", t2." TESTTABLE_INTCOL_COLUMNNAME " from %s t1, testtable15 t2 where t1." TESTTABLE_ID_COLUMNNAME " = t2." TESTTABLE_ID_COLUMNNAME " + 2 AND t1." TESTTABLE_ID_COLUMNNAME " > 20 AND t2." TESTTABLE_STRINGCOL_COLUMNNAME " LIKE '%la%' ORDER BY t1." TESTTABLE_DOUBLECOL_COLUMNNAME " DESC");

    //INSERT
    sqlList.push_back ("insert into %s (" TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_INTCOL_COLUMNNAME ") VALUES (?, ?, ?)");
    sqlList.push_back ("insert into %s (" TESTTABLE_STRINGCOL_COLUMNNAME ", " TESTTABLE_DOUBLECOL_COLUMNNAME ", " TESTTABLE_BLOBCOL_COLUMNNAME ") VALUES (?, ?, ?)");

    //UPDATE
    sqlList.push_back ("update %s set " TESTTABLE_STRINGCOL_COLUMNNAME " = ?, " TESTTABLE_DOUBLECOL_COLUMNNAME " = -4.123 WHERE " TESTTABLE_DOUBLECOL_COLUMNNAME " > 5.12345");
    sqlList.push_back ("update %s set " TESTTABLE_STRINGCOL_COLUMNNAME " = ?, " TESTTABLE_BLOBCOL_COLUMNNAME " = ? WHERE " TESTTABLE_DOUBLECOL_COLUMNNAME " > 5.12345");

    FOR_EACH (Utf8StringCR sql, sqlList)
        {
        TimePrepareStatement (db, sql.c_str (), testTableNames, repetitionsPerTable, BE_SQLITE_OK);
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), sql, repetitionsPerTable);
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnEmptyTablesWithPK)
    {
    const int tableCount = 2000;
    const int rowCount = 0;

    RunCheckEmptiness (tableCount, rowCount, true, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnEmptyTablesWithPKAndAdditionalIndex)
    {
    const int tableCount = 2000;
    const int rowCount = 0;

    RunCheckEmptiness (tableCount, rowCount, true, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnEmptyTablesNoIndexes)
    {
    const int tableCount = 2000;
    const int rowCount = 0;

    RunCheckEmptiness (tableCount, rowCount, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnTablesWithOneRowWithPK)
    {
    const int tableCount = 2000;
    const int rowCount = 1;

    RunCheckEmptiness (tableCount, rowCount, true, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnTablesWithOneRowWithPKAndAdditionalIndex)
    {
    const int tableCount = 2000;
    const int rowCount = 1;

    RunCheckEmptiness (tableCount, rowCount, true, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnTablesWithOneRowNoIndexes)
    {
    const int tableCount = 2000;
    const int rowCount = 1;

    RunCheckEmptiness (tableCount, rowCount, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnNonEmptyTablesWithPK)
    {
    const int tableCount = 2000;
    const int rowCount = 100;

    RunCheckEmptiness (tableCount, rowCount, true, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnNonEmptyTablesWithPKAndAdditionalIndex)
    {
    const int tableCount = 2000;
    const int rowCount = 100;

    RunCheckEmptiness (tableCount, rowCount, true, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckEmptinessOnNonEmptyTablesNoIndexes)
    {
    const int tableCount = 2000;
    const int rowCount = 100;

    RunCheckEmptiness (tableCount, rowCount, false, false);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckTableExistenceOfExistingTables)
    {
    const int tableCount = 2000;
    const int rowCount = 100;
    const bool checkForExistingTables = true;

    RunCheckTableExistence (tableCount, rowCount, checkForExistingTables);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, CheckTableExistenceOfNonExistingTables)
    {
    const int tableCount = 2000;
    const int rowCount = 100;
    const bool checkForExistingTables = false;

    RunCheckTableExistence (tableCount, rowCount, checkForExistingTables);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropEmptyTablesHavingIndicesWithMultipleCalls)
    {
    const int tableCount = 5000;
    const int rowCount = 0;
    RunExecuteDropTablesWithMultipleCallsTest (tableCount, rowCount, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropTablesHavingIndicesWithMultipleCalls)
    {
    const int tableCount = 5000;
    const int rowCount = 100;
    RunExecuteDropTablesWithMultipleCallsTest (tableCount, rowCount, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropEmptyTablesNotHavingIndicesWithMultipleCalls)
    {
    const int tableCount = 5000;
    const int rowCount = 0;
    RunExecuteDropTablesWithMultipleCallsTest (tableCount, rowCount, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropTablesNotHavingIndicesWithMultipleCalls)
    {
    const int tableCount = 5000;
    const int rowCount = 100;
    RunExecuteDropTablesWithMultipleCallsTest (tableCount, rowCount, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropEmptyTablesHavingIndicesWithSingleCall)
    {
    const int tableCount = 5000;
    const int rowCount = 0;
    RunExecuteDropTablesWithSingleCallsTest (tableCount, rowCount, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropTablesHavingIndicesWithSingleCall)
    {
    const int tableCount = 5000;
    const int rowCount = 100;
    RunExecuteDropTablesWithSingleCallsTest (tableCount, rowCount, true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropEmptyTablesNotHavingIndicesWithSingleCall)
    {
    const int tableCount = 5000;
    const int rowCount = 0;
    RunExecuteDropTablesWithSingleCallsTest (tableCount, rowCount, false);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteDropTablesNotHavingIndicesWithSingleCall)
    {
    const int tableCount = 5000;
    const int rowCount = 100;
    RunExecuteDropTablesWithSingleCallsTest (tableCount, rowCount, false);
    }

//**************************** CREATE VIEW tests *************************************************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteCreateEmptyViewWithSingleCall)
    {
    const int viewCount = 5000;
    const int rowCount = 100;
    Utf8String dbPath;
    bvector<Utf8String> testTableNames;
    CreateTestDgnDb (dbPath, testTableNames, L"SqlStatementPerformance.idgndb", viewCount, rowCount, true, true);      

    Utf8String createViewSql;
    FOR_EACH (Utf8String& tableName, testTableNames)
        {
        SqlPrintfString sql ("CREATE VIEW %s_v AS SELECT NULL as id, NULL as stringcol, NULL as doublecol, NULL as intcol FROM %s LIMIT 0;", 
            tableName.c_str (), tableName.c_str ());
        createViewSql.append (sql.GetUtf8CP ());
        }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

    m_stopwatch.Start();
    Savepoint savepoint (db, "SingleExecution");
    stat = db.TryExecuteSql (createViewSql.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "SQL '" << createViewSql.c_str () << "' failed.";
    savepoint.Commit ();
    m_stopwatch.Stop ();
    db.CloseDb ();
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov("Creating %d empty views with a single call took %.4f seconds.", viewCount, m_stopwatch.GetElapsedSeconds());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSqlStatementTests, ExecuteCreateEmptyViewWithMultipleCalls)
    {
    const int viewCount = 5000;
    const int rowCount = 100;
    Utf8String dbPath;
    bvector<Utf8String> testTableNames;
    CreateTestDgnDb (dbPath, testTableNames, L"SqlStatementPerformance.idgndb", viewCount, rowCount, true, true);      

    bvector<Utf8String> sqlList;
    FOR_EACH (Utf8String& tableName, testTableNames)
        {
        SqlPrintfString sql ("CREATE VIEW %s_v AS SELECT NULL as id, NULL as stringcol, NULL as doublecol, NULL as intcol FROM %s LIMIT 0;", 
                            tableName.c_str (), tableName.c_str ());
        sqlList.push_back (Utf8String(sql.GetUtf8CP ()));
        }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test dgndb failed.";

    m_stopwatch.Start();
    Savepoint savepoint (db, "MultipleExecution");
    FOR_EACH (Utf8String& sql, sqlList)
        {
        stat = db.TryExecuteSql (sql.c_str ());
        ASSERT_EQ (BE_SQLITE_OK, stat) << "SQL '" << sql.c_str () << "' failed.";
        }

    savepoint.Commit ();
    m_stopwatch.Stop ();
    db.CloseDb ();
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov ("Creating %d empty views with multiple calls took %.4f seconds.", viewCount, m_stopwatch.GetElapsedSeconds ());
    }


//#endif//def WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS
