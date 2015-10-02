/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceTestingFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once
#include <Bentley/BeTest.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_SQLITE


//=======================================================================================
// @bsiclass                                                Majd.Uddin     08/2015
//=======================================================================================
struct PerformanceTestFixtureBase : public ::testing::Test
{
private:
    BeFileName getDbFilePath(WCharCP dbName);

public:
    StopWatch                   m_stopwatch;
    DbResult SetupDb(Db& db, WCharCP dbName);

};
