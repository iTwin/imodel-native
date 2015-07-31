/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECSqlStatementTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include "TestSchemaHelper.h"

#include "PerformanceECSqlStatementTestFixture.h"

#include "ProfileBreak.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
//************ GetIntegerValueAsserter **************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual
void PerformanceECSqlStatementTestFixture::GetIntegerValueAsserter::AssertGetValue 
(
ECSqlStatement const& statement
) const
    {
    int value = statement.GetValueInt (GetPropertyIndex ());
    //just some dummy code doing something with value to avoid that it gets optimized away.
    EXPECT_TRUE (value > 0 || value <= 0);
    }

//************ GetStringValueAsserter **************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual
void PerformanceECSqlStatementTestFixture::GetStringValueAsserter::AssertGetValue 
(
ECSqlStatement const& statement
) const
    {
    Utf8CP value = statement.GetValueText (GetPropertyIndex());
    EXPECT_TRUE (value != nullptr);
    }

//************ GetPoint3DValueAsserter **************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//virtual
void PerformanceECSqlStatementTestFixture::GetPoint3DValueAsserter::AssertGetValue 
(
ECSqlStatement const& statement
) const
    {
    DPoint3d value = statement.GetValuePoint3D (GetPropertyIndex ());
    //just some dummy code doing something with value to avoid that it gets optimized away.
    EXPECT_TRUE (value.x > 0.0 || value.x <= 0.0);
    }


//************ PerformanceECSqlStatementTestFixture ***************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static member initialization
WCharCP const PerformanceECSqlStatementTestFixture::TEST_DB_NAME = L"ecdbperformancetest.ecdb";
const int PerformanceECSqlStatementTestFixture::TESTCLASS_INSTANCE_COUNT = 100000;
Utf8PrintfString TESTCLASS_INSTANCE_COUNT_String("%d", 100000);
Utf8CP const PerformanceECSqlStatementTestFixture::s_selectClause = "TAG, IntegerMember, StringMember, DateTimeMember, StartPoint";
bool PerformanceECSqlStatementTestFixture::s_testDbExists = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          11/12
+---------------+---------------+---------------+---------------+---------------+------*/
PerformanceECSqlStatementTestFixture::PerformanceECSqlStatementTestFixture
(
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle         11/12
+---------------+---------------+---------------+---------------+---------------+------*/
//override
void PerformanceECSqlStatementTestFixture::InitializeTestDb
(
)
    {
    //cache the db path in the test fixture
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot (outputDir);
    BeFileName dbPath = BeFileName (nullptr, outputDir.GetName(), TEST_DB_NAME, nullptr);
    SetTestDbPath (dbPath);

    if (TestDbExists ())
        {
        LOG.infov (L"Test DgnDb file already exists. Reusing it.");
        return;
        }

    ECDb db;
    CreateEmptyDb (db, dbPath);

    //now populate the db
    PopulateTestDb (db);

    db.CloseDb ();
    s_testDbExists = true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceECSqlStatementTestFixture::RunGetValueWithIsNullTest 
(
GetValueAsserter const& asserter
,StopWatch &totalStopWatch, Utf8String testDetails) const
    {
    ECDb db;
    OpenTestDb (db);

    ECClassCP testClass = GetTestClass (db);
    ASSERT_TRUE (testClass != nullptr);

    ECSqlSelectBuilder ecsqlBuilder;
    CreateECSQL (ecsqlBuilder, *testClass, s_selectClause);

    ECSqlStatement statement;
    auto stat = statement.Prepare (db, ecsqlBuilder.ToString ().c_str ());
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

    int propertyIndex = asserter.GetPropertyIndex ();
    int rowCount = 0;

    //PROFILEBREAK (L"Please attach a profiler to the test runner now, if you want. Then press any key to continue.");

   // StopWatch totalStopWatch ("", true);
    totalStopWatch.Start();
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        ++rowCount;
        bool isNull = statement.IsValueNull (propertyIndex);
        ASSERT_FALSE (isNull);

        if (!isNull)
            {
            asserter.AssertGetValue (statement);
            }
        }

    totalStopWatch.Stop ();
    EXPECT_EQ (TESTCLASS_INSTANCE_COUNT, rowCount);

    LOG.infov ("Total Time: %.4lf ms for %d instances. Test Property: %s::%s.", 
            totalStopWatch.GetElapsedSeconds () * 1000, 
            TESTCLASS_INSTANCE_COUNT,
            TestSchemaHelper::TESTCLASS_NAME,
            asserter.GetPropertyName ());
    PerformanceTestingFrameWork performanceObj;
    performanceObj.writeTodb(totalStopWatch, testDetails, "RunGetValueWithIsNullTest for the ECSQLStatement Preformance for TESTCLASS_INSTANCE_COUNT = " + TESTCLASS_INSTANCE_COUNT_String);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceECSqlStatementTestFixture::RunGetValueWithoutIsNullTest 
(
GetValueAsserter const& asserter
, StopWatch &totalStopWatch,Utf8String testDetails) const
    {
    ECDb db;
    OpenTestDb (db);
    
    ECClassCP testClass = GetTestClass (db);
    EXPECT_TRUE (testClass != nullptr);

    ECSqlSelectBuilder ecsqlBuilder;
    CreateECSQL (ecsqlBuilder, *testClass, s_selectClause);

    ECSqlStatement statement;
    auto stat = statement.Prepare (db, ecsqlBuilder.ToString ().c_str ());
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

    int rowCount = 0;
   // StopWatch totalStopWatch ("", true);
    totalStopWatch.Start();
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        ++rowCount;
        asserter.AssertGetValue (statement);
        }

    totalStopWatch.Stop ();
    EXPECT_EQ (TESTCLASS_INSTANCE_COUNT, rowCount);

    LOG.infov ("Total Time: %.4lf ms for %d instances. Test Property: %s::%s.", 
            totalStopWatch.GetElapsedSeconds () * 1000, 
            TESTCLASS_INSTANCE_COUNT,
            TestSchemaHelper::TESTCLASS_NAME,
            asserter.GetPropertyName ());
    PerformanceTestingFrameWork performanceObj;
    performanceObj.writeTodb(totalStopWatch, testDetails, "RunGetValueWithoutIsNullTest for the ECSQLStatement Preformance  TESTCLASS_INSTANCE_COUNT = " + TESTCLASS_INSTANCE_COUNT_String);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceECSqlStatementTestFixture::PopulateTestDb
(
ECDbR testDb
)
    {
    ECSchemaReadContextPtr schemaReadContext = nullptr;
    ECSchemaPtr testSchema = TestSchemaHelper::CreateComplexTestSchema (schemaReadContext);
    ASSERT_TRUE (schemaReadContext.IsValid ());

    ImportSchema (testDb, testSchema, schemaReadContext);

    ECClassCP testClass = testSchema->GetClassP (TestSchemaHelper::TESTCLASS_NAME);
    InsertTestData (testDb, testClass, TESTCLASS_INSTANCE_COUNT);

    testDb.SaveChanges ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
ECClassCP PerformanceECSqlStatementTestFixture::GetTestClass 
(
ECDbR testDb
)
    {
    ECSchemaCP schema = testDb.Schemas().GetECSchema(TestSchemaHelper::TESTSCHEMA_NAME);

    ECClassCP testClass = schema->GetClassCP (TestSchemaHelper::TESTCLASS_NAME);
    POSTCONDITION (testClass != nullptr, nullptr);

    return testClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceECSqlStatementTestFixture::CreateECSQL 
(
ECSqlSelectBuilder& ecsql, 
ECClassCR searchClass, 
Utf8CP selectClause
)
    {
    ecsql.From (searchClass).Select (selectClause);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          11/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool PerformanceECSqlStatementTestFixture::TestDbExists 
(
)
    {
    return s_testDbExists;
    }
END_ECDBUNITTESTS_NAMESPACE

