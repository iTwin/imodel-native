/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceOverflowTablesResearchTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
struct PerformanceOverflowTablesResearchTestFixture : ECDbTestFixture
    {
    
protected:
    struct Scenario final
        {
        int m_primaryTableCount;
        int m_joinedTablesPerPrimaryTableCount;
        int m_overflowTablesPerJoinedTableCount;

        Scenario(int primaryTableCount, int joinedTablesPerPrimaryTableCount, int overflowTablesPerJoinedTableCount):
            m_primaryTableCount(primaryTableCount), m_joinedTablesPerPrimaryTableCount(joinedTablesPerPrimaryTableCount), m_overflowTablesPerJoinedTableCount(overflowTablesPerJoinedTableCount) {}
        };

    void Setup(Db& db, BeFileName const& fileName, Scenario const& scenario)
        {
        BeFileName filePath = ECDbTestUtility::BuildECDbPath(fileName.GetNameUtf8().c_str());
        ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(filePath));

        for (int i = 0; i < scenario.m_primaryTableCount; i++)
            {
            Utf8String ddl;
            ddl.Sprintf("CREATE TABLE prim%d(Id INTEGER PRIMARY KEY, c1 INTEGER, c2 INTEGER)", i + 1);
            ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(ddl.c_str())) << ddl.c_str();

            for (int j = 0; j < scenario.m_joinedTablesPerPrimaryTableCount; j++)
                {
                Utf8String ddl;
                ddl.Sprintf("CREATE TABLE joined%d_%d(Id INTEGER PRIMARY KEY, c1 INTEGER, c2 INTEGER)", i + 1, j + 1);
                ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql(ddl.c_str())) << ddl.c_str();

                }
            }
        }
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                                  Affan.Khan     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearchTestFixture, ViewsWithTriggers)
    {
    ECDbR db = SetupECDb("performanceoverflowtables_viewswithtriggers.ecdb");
    ASSERT_TRUE(db.IsDbOpen());

    db.ExecuteSql("create table test1(id integer primary key, sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)");
    db.ExecuteSql("create view test1_view as select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1");
    db.ExecuteSql("create trigger update_test1 instead of update on test1_view begin update test1 set sc1=new.sc1,sc2=new.sc2,sc3=new.sc3,sc4=new.sc4,sc5=new.sc5,sc6=new.sc6,sc7=new.sc7,sc8=new.sc8,sc9=new.sc9,sc10=new.sc10 where id =new.id; end");
    db.ExecuteSql("create trigger insert_test1 instead of insert on test1_view begin insert into test1(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(new.id,new.sc1,new.sc2,new.sc3,new.sc4,new.sc5,new.sc6,new.sc7,new.sc8,new.sc9,new.sc10); end");
    db.ExecuteSql("create trigger delete_test1 instead of delete on test1_view begin delete from test1 where id=old.id; end");

    Statement insert_test1, update_test1, delete_test1, select_test1;
    insert_test1.Prepare(db, "insert into test1(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(?,?,?,?,?,?,?,?,?,?,?)");
    update_test1.Prepare(db, "update test1 set sc1=?,sc2=?,sc3=?,sc4=?,sc5=?,sc6=?,sc7=?,sc8=?,sc9=?,sc10=? where id=?");
    delete_test1.Prepare(db, "delete from test1 where id=?");
    select_test1.Prepare(db, "select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1 where id=?");

    Statement insert_test1_view, update_test1_view, delete_test1_view, select_test1_view;
    insert_test1_view.Prepare(db, "insert into test1_view(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(?,?,?,?,?,?,?,?,?,?,?)");
    update_test1_view.Prepare(db, "update test1_view set sc1=?,sc2=?,sc3=?,sc4=?,sc5=?,sc6=?,sc7=?,sc8=?,sc9=?,sc10=? where id=?");
    delete_test1_view.Prepare(db, "delete from test1_view where id=?");
    select_test1_view.Prepare(db, "select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1_view where id=?");

    const int start = 1;
    const int end = 500000;
    std::function<void(std::function<void()>, Utf8CP)> exec = [] (std::function<void()> callback, Utf8CP msg)
        {
        StopWatch timer;
        timer.Start();
        callback();
        timer.Stop();
        printf("%s -> %.5f secs.\r\n", msg, timer.GetElapsedSeconds());
        };

    //insert
    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            insert_test1.Reset();
            insert_test1.ClearBindings();
            insert_test1.BindInt(1, i);
            for (int sc = 1; sc <= 10; sc++) insert_test1.BindInt(sc + 1, rand());
            insert_test1.Step();
            }
        }, "Insert using table");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            update_test1.Reset();
            update_test1.ClearBindings();
            for (int sc = 1; sc <= 10; sc++) update_test1.BindInt(sc, rand());
            update_test1.BindInt(11, i);
            update_test1.Step();
            }
        }, "Update using table");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            select_test1.Reset();
            select_test1.ClearBindings();
            select_test1.BindInt(1, i);
            select_test1.Step();
            }
        }, "Select using table");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            delete_test1.Reset();
            delete_test1.ClearBindings();
            delete_test1.BindInt(1, i);
            delete_test1.Step();
            }
        }, "Delete using table");

    db.SaveChanges();
    //insert
    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            insert_test1_view.Reset();
            insert_test1_view.ClearBindings();
            insert_test1_view.BindInt(1, i);
            for (int sc = 1; sc <= 10; sc++) insert_test1_view.BindInt(sc + 1, rand());
            insert_test1_view.Step();
            }
        }, "Insert using view");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            update_test1_view.Reset();
            update_test1_view.ClearBindings();
            for (int sc = 1; sc <= 10; sc++) update_test1_view.BindInt(sc, rand());
            update_test1_view.BindInt(11, i);
            update_test1_view.Step();
            }
        }, "Update using view");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            select_test1_view.Reset();
            select_test1_view.ClearBindings();
            select_test1_view.BindInt(1, i);
            select_test1_view.Step();
            }
        }, "Select using view");

    exec([&] ()
        {
        for (int i = start; i <= end; i++)
            {
            delete_test1_view.Reset();
            delete_test1_view.ClearBindings();
            delete_test1_view.BindInt(1, i);
            delete_test1_view.Step();
            }
        }, "Delete using view");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Affan.Khan     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearchTestFixture, TestCachedStatement)
    {
    ECDbR db = SetupECDb("performanceoverflowtables_cachedstatement.ecdb");
    ASSERT_TRUE(db.IsDbOpen());

    ASSERT_EQ(db.ExecuteSql("CREATE TABLE Foo(Id INTEGER PRIMARY KEY, Str TEXT NOT NULL)"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(1,'Test1')"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(2,'Test2')"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(3,'Test3')"), BE_SQLITE_OK);

    bvector<CachedStatementPtr> recursionStack(100);
    for (CachedStatementPtr& ptr : recursionStack)
        {
        ptr = db.GetCachedStatement("SELECT Id, Str FROM Foo ORDER BY Id");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 1);
        ASSERT_STREQ(ptr->GetValueText(1), "Test1");
        }

    for (CachedStatementPtr& ptr : recursionStack)
        {
        ASSERT_EQ(ptr->GetValueInt64(0), 1);
        ASSERT_STREQ(ptr->GetValueText(1), "Test1");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 2);
        ASSERT_STREQ(ptr->GetValueText(1), "Test2");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 3);
        ASSERT_STREQ(ptr->GetValueText(1), "Test3");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_DONE);
        }
    }
END_ECDBUNITTESTS_NAMESPACE