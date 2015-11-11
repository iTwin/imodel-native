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
struct PopulateKitchenSinkDb : public ::testing::Test
    {
    protected:
     void PopulateDb (ECDbR ecdb)
        {
        ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk (L"KitchenSink.01.00.ecschema.xml", nullptr);
        ASSERT_TRUE (schemaPtr != NULL);

        ECSchemaCachePtr schemacache = ECSchemaCache::Create ();
        ASSERT_EQ (ECObjectsStatus::Success, schemacache->AddSchema (*schemaPtr)) << "couldn't add schema to the cache" << schemaPtr->GetName ().c_str ();
        ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (*schemacache, ECDbSchemaManager::ImportOptions ())) << "Couldn't import test ecschema.";
        ecdb.SaveChanges ();

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

struct PerformanceQueryTests : public PopulateKitchenSinkDb
    {
    private:
        void ExecuteSql(Utf8StringR ecSql, Utf8StringR timerName, int expectedNumberOfResults, double &overAllTime)
            {
            ECSqlStatement ecStatement;        
            ecStatement.Prepare (m_testDb, ecSql.c_str ());

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
        ECDb m_testDb;
        void ConnectToDb()
            {
            ECDbTestFixture::Initialize ();
            ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (m_testDb, nullptr, L"KitchenSinkDbWithInstances.ecdb"));
            PopulateKitchenSinkDb::PopulateDb (m_testDb);
            ASSERT_TRUE(m_testDb.IsDbOpen ());
            }

        void LoadByInstanceId()
            {
            ConnectToDb();
            LoadByInstanceId("401", "TestClass", 1);
            LoadByInstanceId("251", "Test2Class", 1);
            LoadByInstanceId("99", "Folder", 1);
            LoadByInstanceId("199", "Document", 1);
            m_testDb.CloseDb ();
            }

        void QueryAllInstances()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClass("TestClass", results, 100);
            QueryAllInstancesByClass("Test2Class", results, 100);
            QueryAllInstancesByClass("Folder", results, 100);
            QueryAllInstancesByClass("Document", results, 100);
            m_testDb.CloseDb ();
            }

        void QueryAllInstancesWithOrderBy()
            {
            ConnectToDb();
            bmap<Utf8String, double> results;
            QueryAllInstancesByClassWithOrderBy("TestClass", "BaseClassMember", results, 100);
            QueryAllInstancesByClassWithOrderBy("Folder", "Name", results, 100);
            QueryAllInstancesByClassWithOrderBy("Document", "Name", results, 100);
            m_testDb.CloseDb ();
            }
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceECInstanceECSqlSelectAdapterTests : public PopulateKitchenSinkDb
    {
    public:
        ECDb m_testDb;
        void SetUpDb ()
            {
            ECDbTestFixture::Initialize ();
            ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (m_testDb, nullptr, L"KitchenSinkDbWithInstances.ecdb"));
            PopulateKitchenSinkDb::PopulateDb (m_testDb);
            ASSERT_TRUE (m_testDb.IsDbOpen ());
            }
    };

TEST_F (PerformanceECInstanceECSqlSelectAdapterTests, SelectFromComplexClass)
    {
    SetUpDb ();
    //printf ("Please attach to profiler and press any key...\r\n"); getchar ();
    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare (m_testDb, "SELECT c0.* FROM ONLY [KitchenSink].[TestClass] c0");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

    const int expectedRowCount = 100;
    const int repetitionCount = 100;

    StopWatch timer (true);
    for (int j = 0; j < repetitionCount; j++)
        {
        ecStatement.Reset ();
        ECInstanceECSqlSelectAdapter dataAdapter (ecStatement);
        ECValue value;
        int rowCount = 0;
        while (ecStatement.Step () == BE_SQLITE_ROW)
            {
            ++rowCount;
            IECInstancePtr instance = dataAdapter.GetInstance ();

            }
        ASSERT_EQ (expectedRowCount, rowCount) << L"Should have found " << expectedRowCount << " instances of TestClass";
        }
    timer.Stop ();
    LOG.infov ("Iterating over %d rows (with %d repetitions) took %.4f msecs.", expectedRowCount, repetitionCount, timer.GetElapsedSeconds () * 1000.0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceECInstanceECSqlSelectAdapterTests, SelectFromComplexClass_WithoutAdapter)
    {
    std::function<void (IECSqlValue const&)> processECSqlValue;
    processECSqlValue = [&processECSqlValue] (IECSqlValue const& ecsqlValue)
        {
        auto const& dataType = ecsqlValue.GetColumnInfo ().GetDataType ();
        if (dataType.IsPrimitive ())
            {
            switch (dataType.GetPrimitiveType ())
                {
                    case ECN::PRIMITIVETYPE_Binary:
                        {
                        int size = 0;
                        ecsqlValue.GetBinary (&size);
                        break;
                        }
                    case ECN::PRIMITIVETYPE_Boolean:
                        ecsqlValue.GetBoolean ();
                        break;
                    case ECN::PRIMITIVETYPE_DateTime:
                        ecsqlValue.GetDateTime ();
                        break;
                    case ECN::PRIMITIVETYPE_Double:
                        ecsqlValue.GetDouble ();
                        break;
                    case ECN::PRIMITIVETYPE_Integer:
                        ecsqlValue.GetInt ();
                        break;
                    case ECN::PRIMITIVETYPE_Long:
                        ecsqlValue.GetInt64 ();
                        break;
                    case ECN::PRIMITIVETYPE_Point2D:
                        ecsqlValue.GetPoint2D ();
                        break;
                    case ECN::PRIMITIVETYPE_Point3D:
                        ecsqlValue.GetPoint3D ();
                        break;
                    case ECN::PRIMITIVETYPE_String:
                        ecsqlValue.GetText ();
                        break;
                    default:
                        break;
                }
            }
        else if (dataType.IsStruct ())
            {
            auto const& structValue = ecsqlValue.GetStruct ();
            const int memberCount = structValue.GetMemberCount ();
            for (int j = 0; j < memberCount; j++)
                processECSqlValue (structValue.GetValue (j));
            }
        else
            {
            auto const& arrayValue = ecsqlValue.GetArray ();
            for (auto arrayElementVal : arrayValue)
                {
                processECSqlValue (*arrayElementVal);
                }
            }
        };

    SetUpDb ();
    //printf ("Please attach to profiler and press any key...\r\n"); getchar ();

    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare (m_testDb, "SELECT c0.* FROM ONLY [KitchenSink].[TestClass] c0");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    const int expectedRowCount = 100;
    const int repetitionCount = 100;

    StopWatch timer (true);
    for (int j = 0; j < repetitionCount; j++)
        {
        ecStatement.Reset ();
        const int columnCount = ecStatement.GetColumnCount ();
        int rowCount = 0;
        while (ecStatement.Step () == BE_SQLITE_ROW)
            {
            ++rowCount;
            for (int i = 0; i < columnCount; i++)
                {
                auto const& ecsqlValue = ecStatement.GetValue (i);
                processECSqlValue (ecsqlValue);
                }

            }
        ASSERT_EQ (expectedRowCount, rowCount) << L"Should have found " << expectedRowCount << " instances of TestClass";
        }
    timer.Stop ();
    LOG.infov ("Iterating over %d rows (with %d repetitions) took %.4f msecs.", expectedRowCount, repetitionCount, timer.GetElapsedSeconds () * 1000.0);
    }


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
