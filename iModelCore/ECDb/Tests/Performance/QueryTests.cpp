/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/QueryTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiClass                                       Maha Nasir                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PopulateDb : public ECDbTestFixture
    {
    void PopulateKitchenSink (ECDbR ecdb)
        {
        ASSERT_TRUE (ecdb.IsDbOpen ());
        ECSqlStatement stmt;
            {
            ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "INSERT INTO ks.Folder(Name, SomeNumber, UpdateTime) VALUES(?, ?, ?)"));
            Utf8String nameValue;
            for (int i = 0; i < 100; i++)
                {
                nameValue.Sprintf ("Sample-%d", i);
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (1, nameValue.c_str (), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (2, i + 100));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindDateTime (3, DateTime::GetCurrentTimeUtc ()));
                stmt.Step ();
                stmt.ClearBindings ();
                stmt.Reset ();
                }
            stmt.Finalize ();
            }
            {
            ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "INSERT INTO ks.Document(DifferentNumber,FileName,Name,Size,UpdateTime) VALUES(?, ?, ?, ?, ?)"));
            Utf8String fileName,name;
            for (int i = 0; i < 100; i++)
                {
                fileName.Sprintf ("Sample-%d", i);
                name.Sprintf ("Sample-%d", i + 100);
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (1, i + 100));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (2, fileName.c_str (), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (3, name.c_str (), IECSqlBinder::MakeCopy::No));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt64 (4, i + 1000));
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindDateTime (5, DateTime::GetCurrentTimeUtc ()));
                stmt.Step ();
                stmt.ClearBindings ();
                stmt.Reset ();
                }
                stmt.Finalize ();
            }
            {
            ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "INSERT INTO ks.Test2Class(Test2StringMember) VALUES(?)"));
            Utf8String stringVal;
            for (int i = 0; i < 100; i++)
                {
                stringVal.Sprintf ("Sample-%d", i);
                ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (1, stringVal.c_str (), IECSqlBinder::MakeCopy::No));
                stmt.Step ();
                stmt.ClearBindings ();
                stmt.Reset ();
                }
                stmt.Finalize ();
           }
           {
           ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "INSERT INTO ks.TestClass(BaseClassMember,BooleanMember,ColorStructMember,CustomFormatInt,DateArray,DateTimeMember,DoubleMember,EmbeddedStruct,EmptyIntArray,EndPoint,FormattedArray,FormattedStruct,IntArray,IntegerMember,LongMember,NegativeMember,OneMemberIntArray,PointArray,SecondEmbeddedStruct,SmallIntArray,StartPoint,StringArray,StringMember,StructArray) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
           int arraySize = 10;
           Utf8String ColorMember, StringArray, StringMember, StringVal;
           double val = 100.25;
           for (int i = 0; i < 100; i++)
               {
               ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (1, i + 100));
               ASSERT_EQ (ECSqlStatus::Success, stmt.BindBoolean (2, true));
                 {
                 ColorMember.Sprintf ("Sample-%d", i);
                 ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (3, ColorMember.c_str (), IECSqlBinder::MakeCopy::No));
                 }
               ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (4, i + 200));
                 {
                 IECSqlArrayBinder& ArrayBinder = stmt.BindArray (5, arraySize);
                 for (int j = 0; j < arraySize; j++)
                     {
                     ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindDateTime (DateTime::GetCurrentTimeUtc ()));
                     }
                 }
              ASSERT_EQ (ECSqlStatus::Success, stmt.BindDateTime (6, DateTime::GetCurrentTimeUtc ()));
              ASSERT_EQ (ECSqlStatus::Success, stmt.BindDouble (7, i / val));
                 {
                 IECSqlStructBinder& StructBinder = stmt.BindStruct (8);
                 ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct1BoolMember").BindBoolean (false));
                 ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct1IntMember").BindInt (i + 1));
                 }
                 {
                 IECSqlArrayBinder& ArrayBinder = stmt.BindArray (9, arraySize);
                 for (int j = 0; j < arraySize; j++)
                     {
                     ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindInt (i + 10));
                     }
                 }
             ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (10, DPoint3d::From ((i + 2) / val, (i + 3) / val, (i + 4) / val)));
                {
                IECSqlArrayBinder& ArrayBinder = stmt.BindArray (11, arraySize);
                for (int j = 0; j < arraySize; j++)
                    {
                    ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindInt (i + 300));
                    }
                }
                {
                IECSqlStructBinder& StructBinder = stmt.BindStruct (12);
                ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct3DoubleMember").BindDouble (i + 10 / val));
                ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct3IntMember").BindInt (i + 400));
                ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct3BoolMember").BindBoolean (true));
                }
                {
                IECSqlArrayBinder& ArrayBinder = stmt.BindArray (13, arraySize);
                for (int j = 0; j < arraySize; j++)
                    {
                    ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindInt (i + 500));
                    }
                }
            ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (14, i + 600));
            ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt64 (15, i + 100000));
            ASSERT_EQ (ECSqlStatus::Success, stmt.BindInt (16, i + 700));
               {
               IECSqlArrayBinder& ArrayBinder = stmt.BindArray (17, arraySize);
               for (int j = 0; j < arraySize; j++)
                   {
                   ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindInt (i + 800));
                   }
               }
               {
               IECSqlArrayBinder& ArrayBinder = stmt.BindArray (18, arraySize);
               for (int j = 0; j < arraySize; j++)
                   {
                   ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindPoint3D (DPoint3d::From (i, i + 1, i + 2)));
                   }
               }
               {
               IECSqlStructBinder& StructBinder = stmt.BindStruct (19);
               ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct1BoolMember").BindBoolean (true));
               ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct1IntMember").BindInt (i + 1000));
               }
               {
               IECSqlArrayBinder& ArrayBinder = stmt.BindArray (20, arraySize);
               for (int j = 0; j < arraySize; j++)
                   {
                   ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindInt (i + 2000));
                   }
               }
               ASSERT_EQ (ECSqlStatus::Success, stmt.BindPoint3D (21, DPoint3d::From ((i + 10) / val, (i + 100) / val, (i + 1000) / val)));
               {
               StringArray.Sprintf ("Sample-%d", i);
               IECSqlArrayBinder& ArrayBinder = stmt.BindArray (22, arraySize);
               ASSERT_EQ (ECSqlStatus::Success, ArrayBinder.AddArrayElement ().BindText (StringArray.c_str (), IECSqlBinder::MakeCopy::No));
               }
               {
               StringMember.Sprintf ("Sample-%d", i + 100);
               ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (23, StringMember.c_str (), IECSqlBinder::MakeCopy::No));
               }
               {
               StringVal.Sprintf ("Sample-%d", i + 1000);
               IECSqlArrayBinder& ArrayBinder = stmt.BindArray (24, arraySize);
               IECSqlStructBinder& StructBinder = ArrayBinder.AddArrayElement ().BindStruct ();
               ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct2StringMember").BindText (StringVal.c_str (), IECSqlBinder::MakeCopy::No));
               ASSERT_EQ (ECSqlStatus::Success, StructBinder.GetMember ("Struct2DoubleMember").BindDouble (i + 10000 / val));
               }
               stmt.Step ();
               stmt.ClearBindings ();
               stmt.Reset ();
             }
           stmt.Finalize ();
         }
       }
    };

struct PerformanceQueryTests : public ::testing::Test
    {
    private:
        ECDbTestProject m_testProject;
                void ExecuteSql(Utf8StringR ecSql, Utf8StringR timerName, int expectedNumberOfResults,double &overAllTime)
            {
            ECSqlStatement ecStatement;        
            ecStatement.Prepare(m_testProject.GetECDb(), ecSql.c_str()); 

            ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);

            DbResult result;
            Utf8String overallTimerName;
            overallTimerName.Sprintf("%s (overall time)", timerName.c_str());
            StopWatch overallTimer(overallTimerName.c_str(), false);

            Utf8String instanceTimerName;
            instanceTimerName.Sprintf("%s (instance creation time)", timerName.c_str());
            StopWatch instanceTimer(instanceTimerName.c_str(), false);
            double elapsedSeconds = 0;
            int counter = 0;
            overallTimer.Start();
            while ((result = ecStatement.Step ()) == BE_SQLITE_ROW)
                {
                instanceTimer.Start();
                IECInstancePtr instance = dataAdapter.GetInstance ();
                instanceTimer.Stop();
                elapsedSeconds += instanceTimer.GetElapsedSeconds();
                counter++;
                }
            overallTimer.Stop();
            overAllTime = overallTimer.GetElapsedSeconds();
            ASSERT_EQ(expectedNumberOfResults, counter);
            }

        void LoadByInstanceId(Utf8String instanceId, Utf8String className, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0 WHERE c0.ECInstanceId =('%s' )", className.c_str(), instanceId.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query One Instance By Id Of Class '%s'", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "LoadByInstanceId");
            }

        void QueryAllInstancesByClass(Utf8String className, bmap<Utf8String, double>& results, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0", className.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query All Instances Of Class '%s'", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "QueryAllInstances");
            }

        void QueryAllInstancesByClassWithOrderBy(Utf8String className, Utf8String propertyName, bmap<Utf8String, double>& results, int expectedNumberOfResults)
            {
            Utf8String ecSql;
            ecSql.Sprintf("SELECT c0.* FROM ONLY [KitchenSink].[%s] c0 ORDER BY %s ASC", className.c_str(), propertyName.c_str());
            Utf8String timerName;
            timerName.Sprintf("Query All Instances Of Class '%s' With OrderBy", className.c_str());
            double ellapsedSecond;
            ExecuteSql(ecSql, timerName, expectedNumberOfResults, ellapsedSecond);
            LOGTODB(TEST_DETAILS, ellapsedSecond * 1000.0, "QueryAllInstancesByClassWithOrderBy");
            }

    public:
        void ConnectToDb()
            {
            BeFileName seedPath;
            BeTest::GetHost().GetDocumentsRoot (seedPath);
            seedPath.AppendToPath(L"ECDb");
            seedPath.AppendToPath(L"performanceQueryTests.ecdb");

            auto stat = m_testProject.Open(seedPath.GetNameUtf8().c_str());
            EXPECT_EQ (BE_SQLITE_OK, stat);
            }

        void LoadByInstanceId()
            {
            ConnectToDb();
            LoadByInstanceId("10000", "TestClass", 1);
            LoadByInstanceId("30000", "Test2Class", 1);
            LoadByInstanceId("110001", "Folder", 1);
            LoadByInstanceId("130001", "Document", 1);
            m_testProject.GetECDb().CloseDb();
            }

        void QueryAllInstances()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClass("TestClass", results, 20000);
            QueryAllInstancesByClass("Test2Class", results, 20000);
            QueryAllInstancesByClass("Folder", results, 20000);
            QueryAllInstancesByClass("Document", results, 20000);
            m_testProject.GetECDb().CloseDb();
            }

        void QueryAllInstancesWithOrderBy()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClassWithOrderBy("TestClass", "BaseClassMember", results, 20000);
            QueryAllInstancesByClassWithOrderBy("Folder", "Name", results, 20000);
            QueryAllInstancesByClassWithOrderBy("Document", "Name", results, 20000);
            m_testProject.GetECDb().CloseDb();
            }
    };

TEST_F(PerformanceQueryTests, LoadByInstanceIDForAllClasses)
    {
    LoadByInstanceId();
    }

TEST_F(PerformanceQueryTests, QueryAllInstancesOfEachClass)
    {
    QueryAllInstances();
    }

TEST_F(PerformanceQueryTests, QueryAllInstancesOfEachClassWithOrderBy)
    {
    QueryAllInstancesWithOrderBy();
    }

END_ECDBUNITTESTS_NAMESPACE
