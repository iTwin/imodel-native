/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Performance/PerformanceTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnProject/BackDoor.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <PerformanceTestingHelper/PerformanceTestingHelpers.h>

#define PERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
typedef bpair<Utf8String, double> T_TimerResultPair;

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle      09/2012
//=======================================================================================    
struct PerformanceTestFixture : public testing::Test
    {
private:
    static void ImportSchema(ECN::ECSchemaReadContextR schemaContext, ECN::ECSchemaR testSchema, DgnDbTestDgnManager tdm);

protected:
    static Utf8CP const TEST_CLASS_NAME;
    static Utf8CP const TEST_SCHEMA_NAME;
    static Utf8CP const TEST_COMPLEX_SCHEMA_NAME;
    static const int TESTCLASS_INSTANCE_COUNT;

    Dgn::ScopedDgnHost m_host;

protected:
    PerformanceTestFixture() {};
    virtual ~PerformanceTestFixture () {};

    static void ImportTestSchema(ECN::ECSchemaPtr& schema, DgnDbTestDgnManager tdm, int numIntProperties, int numStringProperties);
    static void ImportComplexTestSchema (ECN::ECSchemaPtr& schema, DgnDbTestDgnManager tdm);
    static void AssignRandomECValue (ECN::ECValue& ecValue, ECN::ECPropertyCR ecProp);
    static void LogResultsToFile(bmap<Utf8String, double> results);
    };


//=======================================================================================
// @bsiclass                                                     Krischan.Eberle      06/15
//=======================================================================================
struct PerformanceElementTestFixture : public DgnDbTestFixture
    {

    protected:
        static const DgnCategoryId s_catId;
        static const int s_instanceCount = 20000;
        static Utf8CP const s_textVal;
        static const int64_t s_int64Val = 20000000LL;
        static const double s_doubleVal;
        static Utf8CP const s_testSchemaXml;

        BentleyStatus ImportTestSchema() const;
        DgnModelId InsertDgnModel() const;
        void CommitAndLogTiming(StopWatch& timer, Utf8CP scenario, Utf8String testcaseName, Utf8String testName) const;

    };

END_DGNDB_UNIT_TESTS_NAMESPACE
