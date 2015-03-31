/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceAdapterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include "TestSchemaHelper.h"
#include "PerformanceTestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE

//struct PerformanceAdapterTestFixture : BeSQlite::ECDbPerformanceTests::PerformanceTestFixture {};
struct PerformanceAdapterTestFixture : PerformanceTestFixture 
    {
    private:
        ECDbTestProject m_testProject;

        void DoSelect(bool oneAtATime, ECDb& ecdb, ECClassCP classP, Utf8String propertyName, bmap<Utf8String, double>& results, StopWatch &LogTimer, Utf8String TestDetails)
            {
            ECSqlSelectBuilder selectBuilder;
            selectBuilder.From(*classP).Select(propertyName.c_str());
            ECSqlStatement ecStatement;        
            ecStatement.Prepare(ecdb, selectBuilder.ToString ().c_str ()); 

            IECInstancePtr instance = classP->GetDefaultStandaloneEnabler()->CreateInstance();
            ECSqlStepStatus result;
            while ((result = ecStatement.Step ()) == ECSqlStepStatus::HasRow)
                {
                for (int i = 0; i < ecStatement.GetColumnCount(); i++)
                    {
                    auto prop = ecStatement.GetColumnInfo(i).GetProperty();
                    Utf8String columnName(prop->GetName());
                    if (!columnName.Equals(propertyName))
                        continue;

                    IECSqlArrayValue const& arrayValue = ecStatement.GetValueArray (i);
                    
                    wchar_t timerName[256];
                    BeStringUtilities::Snwprintf(timerName, L"Adding 10000 array elements %ls to %ls", oneAtATime ? L"one at a time" : L"at once", prop->GetName().c_str());
                    StopWatch timer(timerName);

                    timer.Start();
                    LogTimer.Start();
                    if (oneAtATime)
                        {
                        for (auto arrayIt = arrayValue.begin (); arrayIt != arrayValue.end (); ++arrayIt)
                            {
                            instance->AddArrayElements(prop->GetName().c_str(), 1);
                            }
                        }
                    else
                        {
                        int arrayLength = arrayValue.GetArrayLength ();
                        if (0 >= arrayLength)
                            continue;
                        instance->AddArrayElements(prop->GetName().c_str(), arrayLength);
                        }
                    timer.Stop();
                    LogTimer.Stop();
                    results[Utf8String(timerName)] = timer.GetElapsedSeconds();
                    LOG.infov(L"%ls took %lf seconds.", timerName, timer.GetElapsedSeconds());
                    PerformanceTestingFrameWork performanceObj;
                    EXPECT_TRUE(performanceObj.writeTodb(L"PerformanceTest.ecdb", LogTimer, TestDetails, " Adding 10000 array elements for property: " + propertyName));
                    }
                }
            }

    public:
        virtual void InitializeTestDb () override {}

        void RunArrayTests(bool oneAtATime, StopWatch &Logtimer, Utf8String TestDetails)
            {
            BeFileName seedPath;
            BeTest::GetHost().GetDocumentsRoot (seedPath);
            seedPath.AppendToPath(L"DgnDb");
            seedPath.AppendToPath(L"ECDb");
            seedPath.AppendToPath(L"performanceEC.ecdb");

            auto stat = m_testProject.Open(seedPath.GetNameUtf8().c_str());
            EXPECT_EQ (BE_SQLITE_OK, stat);
            auto& ecdb = m_testProject.GetECDb();
            Utf8String schemaName("PerformanceSchema");
            Utf8String className("ClassOfArrays");
            auto classP = ecdb. Schemas ().GetECClass (schemaName.c_str (), className.c_str ());
            EXPECT_TRUE (classP != nullptr);
            bmap<Utf8String, double> results;
            DoSelect(oneAtATime, ecdb, classP, "IntArray", results, Logtimer,TestDetails);
            DoSelect(oneAtATime, ecdb, classP, "StringArray", results, Logtimer, TestDetails);
            DoSelect(oneAtATime, ecdb, classP, "StructArray", results, Logtimer, TestDetails);
            DoSelect(oneAtATime, ecdb, classP, "DateTimeArray", results, Logtimer, TestDetails);
            LogResultsToFile(results);
            }
    };

TEST_F(PerformanceAdapterTestFixture, AddArrayElementsOneAtATime)
    {
    StopWatch Logtimer("", false);
    Utf8String TestDetails = "PerformanceAdapterTestFixture.AddArrayElementsOneAtATime";
    RunArrayTests(true,Logtimer,TestDetails);
    }

TEST_F(PerformanceAdapterTestFixture, AddArrayElementsOnce)
    {
    StopWatch Logtimer("", false);
    Utf8String TestDetails = "PerformanceAdapterTestFixture.AddArrayElementsOnce";
    RunArrayTests(false,Logtimer, TestDetails);
    }

END_ECDBUNITTESTS_NAMESPACE
