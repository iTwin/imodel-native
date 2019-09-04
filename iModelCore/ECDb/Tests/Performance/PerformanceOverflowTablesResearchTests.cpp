/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceOverflowTablesResearchTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
struct PerformanceOverflowTables_PhysicalElementTests : PerformanceOverflowTablesResearchTestFixture
    {
    protected:
        static std::vector<Scenario> CreateScenarios()
            {
            //col counts don't include the id col
            const int primaryTableColCount = 11; //bis_Element
            const int secondaryTableUnsharedColCount = 19; // bis_GeometricElement3d (in reality only 18, but it is easier to compute with 19)
            std::vector<int> eightyPercentClassColCounts {40, 70};
            std::vector<int> maxClassColCounts {50, 75, 100, 125, 150, 200};
            std::vector<int> sharedColCounts {10, 20, 30, 40, 70, 110};

            std::vector<Scenario> scenarios;
            for (int maxClassColCount : maxClassColCounts)
                {
                for (int eightyPercentClassColCount : eightyPercentClassColCounts)
                    {
                    if (eightyPercentClassColCount > maxClassColCount)
                        continue;

                    for (int sharedColCount : sharedColCounts)
                        {
                        Scenario scenario("PhysicalElement", primaryTableColCount, secondaryTableUnsharedColCount, sharedColCount, maxClassColCount, eightyPercentClassColCount);
                        if (!scenario.IsValid())
                            {
                            BeAssert(false);
                            return std::vector<Scenario>();
                            }


                        if (!scenarios.empty())
                            {
                            Scenario const& prevScenario = scenarios.back();
                            if (prevScenario.PrimaryTableColCount() == scenario.PrimaryTableColCount() &&
                                prevScenario.SecondaryTableColCount() == scenario.SecondaryTableColCount() &&
                                prevScenario.TernaryTableColCount() == scenario.TernaryTableColCount() &&
                                prevScenario.EightyPercentClassColCount() == scenario.EightyPercentClassColCount())
                                continue; //no duplicate scenarios wanted

                            }
                        scenarios.push_back(scenario);
                        }
                    }
                }
            return scenarios;
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTables_PhysicalElementTests, InsertAll)
    {
    std::vector<Scenario> scenarios = CreateScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunInsertAllCols(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTables_PhysicalElementTests, InsertSingleColumn)
    {
    std::vector<Scenario> scenarios = CreateScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunInsertSingleCol(scenario, ColumnMode::First);
        RunInsertSingleCol(scenario, ColumnMode::Middle);
        RunInsertSingleCol(scenario, ColumnMode::Last);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTables_PhysicalElementTests, UpdateAllColumns)
    {
    std::vector<Scenario> scenarios = CreateScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunUpdateAllCols(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTables_PhysicalElementTests, SelectAllColumns)
    {
    std::vector<Scenario> scenarios = CreateScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunSelectAllCols(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTables_PhysicalElementTests, Delete)
    {
    std::vector<Scenario> scenarios = CreateScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunDelete(scenario);
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearch_NullColumnsTestFixture, Insert)
    {
    std::vector<Scenario> scenarios = GetTestScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunInsert(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearch_NullColumnsTestFixture, Update)
    {
    std::vector<Scenario> scenarios = GetTestScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunUpdate(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle 03/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearch_NullColumnsTestFixture, Delete)
    {
    std::vector<Scenario> scenarios = GetTestScenarios();
    for (Scenario const& scenario : scenarios)
        {
        RunDelete(scenario);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Affan.Khan     01/2017
//---------------------------------------------------------------------------------------
TEST_F(PerformanceOverflowTablesResearchTestFixture, ViewsWithTriggers)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("performanceoverflowtables_viewswithtriggers.ecdb"));

    m_ecdb.ExecuteSql("create table test1(id integer primary key, sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)");
    m_ecdb.ExecuteSql("create view test1_view as select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1");
    m_ecdb.ExecuteSql("create trigger update_test1 instead of update on test1_view begin update test1 set sc1=new.sc1,sc2=new.sc2,sc3=new.sc3,sc4=new.sc4,sc5=new.sc5,sc6=new.sc6,sc7=new.sc7,sc8=new.sc8,sc9=new.sc9,sc10=new.sc10 where id =new.id; end");
    m_ecdb.ExecuteSql("create trigger insert_test1 instead of insert on test1_view begin insert into test1(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(new.id,new.sc1,new.sc2,new.sc3,new.sc4,new.sc5,new.sc6,new.sc7,new.sc8,new.sc9,new.sc10); end");
    m_ecdb.ExecuteSql("create trigger delete_test1 instead of delete on test1_view begin delete from test1 where id=old.id; end");

    Statement insert_test1, update_test1, delete_test1, select_test1;
    insert_test1.Prepare(m_ecdb, "insert into test1(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(?,?,?,?,?,?,?,?,?,?,?)");
    update_test1.Prepare(m_ecdb, "update test1 set sc1=?,sc2=?,sc3=?,sc4=?,sc5=?,sc6=?,sc7=?,sc8=?,sc9=?,sc10=? where id=?");
    delete_test1.Prepare(m_ecdb, "delete from test1 where id=?");
    select_test1.Prepare(m_ecdb, "select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1 where id=?");

    Statement insert_test1_view, update_test1_view, delete_test1_view, select_test1_view;
    insert_test1_view.Prepare(m_ecdb, "insert into test1_view(id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10)values(?,?,?,?,?,?,?,?,?,?,?)");
    update_test1_view.Prepare(m_ecdb, "update test1_view set sc1=?,sc2=?,sc3=?,sc4=?,sc5=?,sc6=?,sc7=?,sc8=?,sc9=?,sc10=? where id=?");
    delete_test1_view.Prepare(m_ecdb, "delete from test1_view where id=?");
    select_test1_view.Prepare(m_ecdb, "select id,sc1,sc2,sc3,sc4,sc5,sc6,sc7,sc8,sc9,sc10 from test1_view where id=?");

    const int start = 1;
    const int end = 5000;
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

    m_ecdb.SaveChanges();
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


END_ECDBUNITTESTS_NAMESPACE