/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbFileInfoTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle         11/15
//=======================================================================================
struct PerformanceECDbFileInfoTests : ECDbTestFixture
    {
protected:
    static const int s_initialInstanceCountPerClass = 1000;

    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbFileInfoTests, CreateSeedFile)
    {
    //separate out code that creates and populates the seed files, so that multiple runs of the actual
    //perf timings can be done without influence of the heavy work to create the seed file.
    ECDbR ecdb = SetupECDb("performanceecdbfileinfo.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"), s_initialInstanceCountPerClass);
    ASSERT_TRUE(ecdb.IsDbOpen());
    }

END_ECDBUNITTESTS_NAMESPACE