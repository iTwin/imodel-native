/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECInstanceECSqlSelectAdpaterTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald                 04/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(Performance_ECInstanceECSqlSelectAdapterTests, SelectFromComplexClass)
    {
    BeFileName seedPath;
    BeTest::GetHost().GetDocumentsRoot (seedPath);
    seedPath.AppendToPath(L"DgnDb");
    seedPath.AppendToPath(L"ECDb");
    seedPath.AppendToPath(L"KitchenSinkWithInstances.ecdb");

    ECDbTestProject testProject;

    auto stat = testProject.Open(seedPath.GetNameUtf8().c_str());
    EXPECT_EQ (BE_SQLITE_OK, stat);

    //printf ("Please attach to profiler and press any key...\r\n"); getchar ();

    ECSqlStatement ecStatement;        
    ECSqlStatus prepareStatus = ecStatement.Prepare(testProject.GetECDb(), "SELECT c0.* FROM ONLY [KitchenSink].[TestClass] c0");
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
TEST (Performance_ECInstanceECSqlSelectAdapterTests, SelectFromComplexClass_WithoutAdapter)
    {
    BeFileName seedPath;
    BeTest::GetHost ().GetDocumentsRoot (seedPath);
    seedPath.AppendToPath (L"DgnDb");
    seedPath.AppendToPath (L"ECDb");
    seedPath.AppendToPath (L"KitchenSinkWithInstances.ecdb");

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

    ECDbTestProject testProject;
    auto stat = testProject.Open (seedPath.GetNameUtf8 ().c_str ());
    EXPECT_EQ (BE_SQLITE_OK, stat);


    //printf ("Please attach to profiler and press any key...\r\n"); getchar ();

    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare (testProject.GetECDb (), "SELECT c0.* FROM ONLY [KitchenSink].[TestClass] c0");
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

END_ECDBUNITTESTS_NAMESPACE