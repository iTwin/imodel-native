/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceSQLite_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#ifdef WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS

#include "BeSQLitePerformanceTests.h"
#include <PerformanceTestingHelper/PerformanceTestingHelpers.h>
#ifdef WIP_PUBLISHED_API
#include <BeSQLite/SQLiteAPI.h> //only needed for test to directly work with SQLite
#endif

#include <vector>

struct PerformanceSQLiteTests : public PerformanceTestFixtureBase
{
    double totalTime;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Krischan.Eberle                     01/14
    //+---------------+---------------+---------------+---------------+---------------+------
    void RunDeleteCascadingTest(int primaryRowCount, int secondaryRowCountPerPrimaryRow, int ternaryRowCountPerPrimaryRow, bool withIndexOnSecondaryTable)
    {
        std::vector<Utf8String> primarySqlList;
        std::vector<Utf8String> secondarySqlList;
        std::vector<Utf8String> ternarySqlList;
        GenerateDeleteCascadingTestSqlLists(primarySqlList, secondarySqlList, ternarySqlList);

        if (withIndexOnSecondaryTable) // With Index test
        {
            Utf8String dbWithTriggerPath = PopulateDeleteCascadingTestDb(L"deletecascadingwithtrigger.db", withIndexOnSecondaryTable, true, primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
            ASSERT_FALSE(dbWithTriggerPath.empty()) << "Populating test db failed";
            Db dbWithTrigger;
            DbResult stat = dbWithTrigger.OpenBeSQLiteDb(dbWithTriggerPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
            ASSERT_EQ(BE_SQLITE_OK, stat);

            for (size_t i = 0; i < primarySqlList.size(); i++)
            {
                Utf8CP primarySql = primarySqlList[i].c_str();
                Utf8CP secondarySql = secondarySqlList[i].c_str();
                Utf8CP ternarySql = ternarySqlList[i].c_str();
                LOG.tracev("Executing %s [Secondary: %s Ternary: %s]", primarySql, secondarySql, ternarySql);
                Savepoint savepoint(dbWithTrigger, "");
                Statement stmt;
                ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(dbWithTrigger, primarySql)) << "Preparation of " << primarySql << " failed: " << dbWithTrigger.GetLastError();
                m_stopwatch.Start();
                auto stat = stmt.Step();
                m_stopwatch.Stop();
                ASSERT_EQ(BE_SQLITE_DONE, stat) << "Executing test delete failed";
                int rowsAffected = dbWithTrigger.GetModifiedRowCount();
                savepoint.Cancel();

                PERFORMANCELOG.tracev("With trigger: %.4f msec - deleted %d primary rows, %d secondary rows, %d ternary rows",
                    m_stopwatch.GetElapsedSeconds() * 1000,
                    rowsAffected, rowsAffected * secondaryRowCountPerPrimaryRow,
                    rowsAffected * secondaryRowCountPerPrimaryRow * ternaryRowCountPerPrimaryRow);
            }
        }
        else //Db without Index
        {
            Utf8String dbWithoutTriggerPath = PopulateDeleteCascadingTestDb(L"deletecascadingwithouttrigger.db", withIndexOnSecondaryTable, false, primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
            ASSERT_FALSE(dbWithoutTriggerPath.empty()) << "Populating test db failed";
            Db dbWithoutTrigger;
            DbResult stat = dbWithoutTrigger.OpenBeSQLiteDb(dbWithoutTriggerPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
            ASSERT_EQ(BE_SQLITE_OK, stat);

            for (size_t i = 0; i < primarySqlList.size(); i++)
            {
                Utf8CP primarySql = primarySqlList[i].c_str();
                Utf8CP secondarySql = secondarySqlList[i].c_str();
                Utf8CP ternarySql = ternarySqlList[i].c_str();
                LOG.tracev("Executing %s [Secondary: %s Ternary: %s]", primarySql, secondarySql, ternarySql);

                Savepoint savepoint(dbWithoutTrigger, "");

                //execute ternary statement first
                Statement ternaryStmt;
                ASSERT_EQ(BE_SQLITE_OK, ternaryStmt.Prepare(dbWithoutTrigger, ternarySql)) << "Preparation of " << ternarySql << " failed: " << dbWithoutTrigger.GetLastError();
                StopWatch ternaryTimer(true);
                stat = ternaryStmt.Step();
                ternaryTimer.Stop();
                ASSERT_EQ(BE_SQLITE_DONE, stat) << "Executing test delete failed";
                int ternaryRowsAffected = dbWithoutTrigger.GetModifiedRowCount();


                //execute secondary statement first
                Statement secondaryStmt;
                ASSERT_EQ(BE_SQLITE_OK, secondaryStmt.Prepare(dbWithoutTrigger, secondarySql)) << "Preparation of " << secondarySql << " failed: " << dbWithoutTrigger.GetLastError();
                StopWatch secondaryTimer(true);
                stat = secondaryStmt.Step();
                secondaryTimer.Stop();
                ASSERT_EQ(BE_SQLITE_DONE, stat) << "Executing test delete failed";
                int secondaryRowsAffected = dbWithoutTrigger.GetModifiedRowCount();

                Statement primaryStmt;
                ASSERT_EQ(BE_SQLITE_OK, primaryStmt.Prepare(dbWithoutTrigger, primarySql)) << "Preparation of " << primarySql << " failed: " << dbWithoutTrigger.GetLastError();
                StopWatch primaryTimer(true);
                auto stat = primaryStmt.Step();
                primaryTimer.Stop();
                ASSERT_EQ(BE_SQLITE_DONE, stat) << "Executing test delete failed";
                int rowsAffected = dbWithoutTrigger.GetModifiedRowCount();

                ASSERT_EQ(rowsAffected * secondaryRowCountPerPrimaryRow, secondaryRowsAffected) << primarySql << " - " << secondarySql << " - " << ternarySql;
                ASSERT_EQ(rowsAffected * secondaryRowCountPerPrimaryRow * ternaryRowCountPerPrimaryRow, ternaryRowsAffected) << primarySql << " - " << secondarySql << " - " << ternarySql;
                savepoint.Cancel();

                double primaryTiming = primaryTimer.GetElapsedSeconds() * 1000;
                double secondaryTiming = secondaryTimer.GetElapsedSeconds() * 1000;
                double ternaryTiming = ternaryTimer.GetElapsedSeconds() * 1000;
                totalTime = primaryTiming + secondaryTiming + ternaryTiming;
                PERFORMANCELOG.tracev("Without trigger: %.4f msec (Primary delete %.4f msec, secondary delete %.4f msec, ternary delete %.4f) - deleted %d primary rows, %d secondary rows, %d ternary rows",
                    primaryTiming + secondaryTiming + ternaryTiming, primaryTiming, secondaryTiming, ternaryTiming,
                    rowsAffected, secondaryRowsAffected, ternaryRowsAffected);
            }

        }




        LOG.tracev("Delete cascading test. Primary table row count: %d. Secondary rows per primary row: %d Ternary rows per secondary row: %d",
            primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow);
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Krischan.Eberle                     01/14
    //+---------------+---------------+---------------+---------------+---------------+------
    void GenerateDeleteCascadingTestSqlLists(std::vector<Utf8String>& primarySqlList, std::vector<Utf8String>& secondarySqlList, std::vector<Utf8String>& ternarySqlList)
    {
        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id = 5");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id = 5)");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN "
            "(SELECT Id FROM SecondaryTable WHERE PrimaryId IN "
            "(SELECT Id FROM PrimaryTable WHERE Id = 5)"
            ") AND "
            "PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id = 5)");

        //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id = 5");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId = 5");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE PrimaryId = 5");



        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id IN (2, 8)");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8))");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8)))"
            " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id IN (2, 8))");

        //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id IN (2, 8)");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (2, 8)");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE PrimaryId IN (2, 8)");




        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id % 2 = 0");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0)");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0))"
            " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE Id % 2 = 0)");

        //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE Id % 2 = 0");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId % 2 = 0");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE PrimaryId % 2 = 0");





        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE I IN (-2, -8)");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8))");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8)))"
            " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE I IN (-2, -8))");


        primarySqlList.push_back("DELETE FROM PrimaryTable WHERE (-1 * I) % 2 = 0");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0)");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0))"
            " AND PrimaryId IN (SELECT Id FROM PrimaryTable WHERE (-1 * I) % 2 = 0)");


        primarySqlList.push_back("DELETE FROM PrimaryTable");
        secondarySqlList.push_back("DELETE FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable)");
        ternarySqlList.push_back("DELETE FROM TernaryTable WHERE SecondaryId IN (SELECT Id FROM SecondaryTable WHERE PrimaryId IN (SELECT Id FROM PrimaryTable))"
            " AND PrimaryId IN (SELECT Id FROM PrimaryTable)");

        //this is an optimum counterpart to the previous which however is hard to come up with in a generic fashion.
        primarySqlList.push_back("DELETE FROM PrimaryTable");
        secondarySqlList.push_back("DELETE FROM SecondaryTable");
        ternarySqlList.push_back("DELETE FROM TernaryTable");
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Krischan.Eberle                     01/14
    //+---------------+---------------+---------------+---------------+---------------+------
    Utf8String PopulateDeleteCascadingTestDb(WCharCP dbFileName, bool withIndex, bool withTrigger, int primaryRowCount, int secondaryRowsPerPrimaryRow, int ternaryRowsPerSecondaryRow)
    {
        Db db;
        SetupDb(db, dbFileName);

        Utf8CP createTableSqlTemplate = "CREATE TABLE PrimaryTable (Id INTEGER PRIMARY KEY, D DOUBLE, Name TEXT, I INT);"
            "CREATE TABLE SecondaryTable (PrimaryId INTEGER, Id INTEGER, Name TEXT, L INT64 %s);"
            "CREATE TABLE TernaryTable (PrimaryId INTEGER, SecondaryId INTEGER, Id INTEGER, Name TEXT, D DOUBLE %s);";

        Utf8String createTableSql;
        if (withIndex)
            createTableSql.Sprintf(createTableSqlTemplate, ", PRIMARY KEY (PrimaryId, Id)", ", PRIMARY KEY (PrimaryId, SecondaryId, Id)");
        else
            createTableSql.Sprintf(createTableSqlTemplate, "", "");

        auto stat = db.ExecuteSql(createTableSql.c_str());
        if (stat != BE_SQLITE_OK)
        {
            LOG.error(db.GetLastError());
            return "";
        }

        if (withTrigger)
        {
            stat = db.ExecuteSql("CREATE TRIGGER DELETE_SECONDARY_ROWS AFTER DELETE ON PrimaryTable "
                "BEGIN "
                "DELETE FROM SecondaryTable WHERE PrimaryId = old.Id; "
                "END;"
                "CREATE TRIGGER DELETE_TERNARY_ROWS AFTER DELETE ON SecondaryTable "
                "BEGIN "
                "DELETE FROM TernaryTable WHERE PrimaryId = old.PrimaryId AND SecondaryId = old.Id;"
                "END;"
                );
            if (stat != BE_SQLITE_OK)
            {
                LOG.error(db.GetLastError());
                return "";
            }
        }


        Statement primaryStmt;
        stat = primaryStmt.Prepare(db, "INSERT INTO PrimaryTable (Id, D, Name, I) VALUES (?,?,?,?)");
        if (stat != BE_SQLITE_OK)
        {
            LOG.error(db.GetLastError());
            return "";
        }

        for (int i = 0; i < primaryRowCount; i++)
        {
            primaryStmt.Reset();
            primaryStmt.ClearBindings();
            primaryStmt.BindInt(1, i);
            primaryStmt.BindDouble(2, i * 3.1415);
            primaryStmt.BindText(3, "Some text", Statement::MakeCopy::Yes);
            primaryStmt.BindInt(4, -i);

            stat = primaryStmt.Step();
            if (stat != BE_SQLITE_DONE)
            {
                LOG.error(db.GetLastError());
                return "";
            }
        }

        Statement secondaryStmt;
        stat = secondaryStmt.Prepare(db, "INSERT INTO SecondaryTable (PrimaryId, Id, Name, L) VALUES (?,?,?,?)");
        if (stat != BE_SQLITE_OK)
        {
            LOG.error(db.GetLastError());
            return "";
        }

        for (int i = 0; i < primaryRowCount; i++)
        {
            for (int j = 0; j < secondaryRowsPerPrimaryRow; j++)
            {
                secondaryStmt.Reset();
                secondaryStmt.ClearBindings();
                secondaryStmt.BindInt(1, i);
                secondaryStmt.BindInt(2, j);
                secondaryStmt.BindText(3, "Some text", Statement::MakeCopy::Yes);
                secondaryStmt.BindInt(4, i*j);

                stat = secondaryStmt.Step();
                if (stat != BE_SQLITE_DONE)
                {
                    LOG.error(db.GetLastError());
                    return "";
                }
            }
        }

        Statement ternaryStmt;
        stat = ternaryStmt.Prepare(db, "INSERT INTO TernaryTable (PrimaryId, SecondaryId, Id, Name, D) VALUES (?,?,?,?,?)");
        if (stat != BE_SQLITE_OK)
        {
            LOG.error(db.GetLastError());
            return "";
        }

        for (int i = 0; i < primaryRowCount; i++)
        {
            for (int j = 0; j < secondaryRowsPerPrimaryRow; j++)
            {
                for (int k = 0; k < ternaryRowsPerSecondaryRow; k++)
                {
                    ternaryStmt.Reset();
                    ternaryStmt.ClearBindings();
                    ternaryStmt.BindInt(1, i);
                    ternaryStmt.BindInt(2, j);
                    ternaryStmt.BindInt(3, k);
                    ternaryStmt.BindText(4, "Some text", Statement::MakeCopy::Yes);
                    ternaryStmt.BindDouble(5, i*j*k*3.14);

                    stat = ternaryStmt.Step();
                    if (stat != BE_SQLITE_DONE)
                    {
                        LOG.error(db.GetLastError());
                        return "";
                    }
                }
            }
        }

        db.SaveChanges();
        return db.GetDbFileName();
    }



}; // End of Test Fixture

//---------------------------------------------------------------------------------------
//! Tests performance of a cascading delete without using foreign keys. Instead
//! SQLite triggers are compared to executing the cascading statements manually.
//! This particular test is run with indices on the column that acts as foreign key to the parent table
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSQLiteTests, DeleteCascadingWithIndex)
{
    const int primaryRowCount = 100;
    const int secondaryRowCountPerPrimaryRow = 1000;
    const int ternaryRowCountPerPrimaryRow = 100;
    RunDeleteCascadingTest(primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow, true);
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
}

//---------------------------------------------------------------------------------------
//! Tests performance of a cascading delete without using foreign keys. Instead
//! SQLite triggers are compared to executing the cascading statements manually
//! This particular test is run @p without indices on the column that acts as foreign key to the parent table
// @bsimethod                                   Krischan.Eberle                     01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceSQLiteTests, DeleteCascadingWithoutIndex)
{
    const int primaryRowCount = 10;
    const int secondaryRowCountPerPrimaryRow = 100;
    const int ternaryRowCountPerPrimaryRow = 100;
    RunDeleteCascadingTest(primaryRowCount, secondaryRowCountPerPrimaryRow, ternaryRowCountPerPrimaryRow, false);
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
}

//#endif//def WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS
