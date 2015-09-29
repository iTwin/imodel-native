/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceBisDesignStudies_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceBisDesignTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

static const int INSTANCES_PER_CLASS_COUNT = 100000;

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Performance_BisDesign_TablePerClassScenario_TestFixture, Test)
    {
    ECDbTestProject::Initialize ();
    Utf8CP dbFileName = "performance_bisdesign_tableperclass.db";

    Context context (10, INSTANCES_PER_CLASS_COUNT);
    RunTest (dbFileName, context);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Performance_BisDesign_MasterTableScenario_TestFixture, Test)
    {
    ECDbTestProject::Initialize ();
    Utf8CP dbFileName = "performance_bisdesign_mastertable.db";

    Context context (10, INSTANCES_PER_CLASS_COUNT);
    RunTest (dbFileName, context);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Performance_BisDesign_MasterTableAndDomainTablesScenario_TestFixture, Test)
    {
    ECDbTestProject::Initialize ();
    Utf8CP dbFileName = "performance_bisdesign_mastertableplustableperclass.db";

    Context context (10, INSTANCES_PER_CLASS_COUNT);
    RunTest (dbFileName, context);
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (Performance_BisDesign, InsertPerformanceAndNumberOfIndicesTest)
    {
    ECDbTestProject::Initialize ();
    BeFileName dbPath (ECDbTestProject::BuildECDbPath ("performance_bisdesign_insertperformance_and_indexcount.db"));

    
    const int tableCount = 50;
    const int columnCount = 50;
    const int rowCount = 1000;

    auto getTableName = [] (int tableNumber)
        {
        Utf8String tableName;
        tableName.Sprintf ("t%d", tableNumber + 1);
        return tableName;
        };

    auto getColumnName = [] (int colNumber)
        {
        Utf8String columnName;
        columnName.Sprintf ("c%d", colNumber + 1);
        return columnName;
        };

        {
        if (dbPath.DoesPathExist ())
            dbPath.BeDeleteFile ();

        Db db;
        ASSERT_EQ (BE_SQLITE_OK, db.CreateNewDb (dbPath));

        for (int i = 0; i < tableCount; i++)
            {
            Utf8String tableName = getTableName (i);

            Utf8String sql ("CREATE TABLE ");
            sql.append (tableName).append ("(");

            bool isFirstItem = true;
            for (int j = 0; j < columnCount; j++)
                {
                if (!isFirstItem)
                    sql.append (", ");

                sql.append (getColumnName (j)).append (" INTEGER");
                isFirstItem = false;
                }
            sql.append (");");

            ASSERT_EQ (BE_SQLITE_OK, db.ExecuteSql (sql.c_str ())) << "CREATE TABLE failed: " << sql.c_str ();

            for (int j = 0; j < i; j++)
                {
                Utf8String colName = getColumnName (j);

                Utf8String indexName;
                indexName.Sprintf ("idx_%s_%s", tableName.c_str (), colName.c_str ());

                sql = "CREATE INDEX ";
                sql.append (indexName).append (" ON ").append (tableName).append (" (").append (colName).append (");");
                ASSERT_EQ (BE_SQLITE_OK, db.ExecuteSql (sql.c_str ())) << "CREATE INDEX failed: " << sql.c_str ();
                }
            }
        }

    Db db;
    ASSERT_EQ (BE_SQLITE_OK, db.OpenBeSQLiteDb (dbPath, Db::OpenParams (Db::OpenMode::ReadWrite)));

    PERFORMANCELOG.infov ("INSERT performance as function of index count per table - %d tables, %d columns, %d rows",
        tableCount, columnCount, rowCount);
    StopWatch logTimer(true);
    for (int i = 0; i < tableCount; i++)
        {
        Utf8String sql ("INSERT INTO ");
        sql.append (getTableName (i)).append (" (");

        Utf8String valuesSql (") VALUES (");
        bool isFirstItem = true;
        for (int j = 0; j < columnCount; j++)
            {
            if (!isFirstItem)
                {
                sql.append (", ");
                valuesSql.append (", ");
                }

            sql.append (getColumnName (j));
            valuesSql.append ("?");

            isFirstItem = false;
            }

        sql.append (valuesSql).append (");");

        Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (db, sql.c_str ()));

        StopWatch timer (true);
        for (int k = 0; k < rowCount; k++)
            {
            for (int j = 0; j < columnCount; j++)
                {
                ASSERT_EQ (BE_SQLITE_OK, stmt.BindInt (j + 1, 1000 * k + (j + 1) * 2));
                }

            ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
            stmt.Reset ();
            stmt.ClearBindings ();
            }
        timer.Stop ();

        PERFORMANCELOG.infov ("%d indices: %.4f s", i, timer.GetElapsedSeconds ());
        }
    logTimer.Stop();
    LOGTODB(TEST_DETAILS, logTimer.GetElapsedSeconds(), "PerformanceBsiDesignStudies Test run using  performance_bisdesign_insertperformance_and_indexcount.db");
    }

END_ECDBUNITTESTS_NAMESPACE
