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

#define PERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
typedef bpair<Utf8String, double> T_TimerResultPair;

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE
//=======================================================================================    
//! @bsiclass                                                 Krischan.Eberle      09/2012
//=======================================================================================    
struct PerformanceTestFixture : public testing::Test
    {
private:
    static void ImportSchema(ECN::ECSchemaReadContextR schemaContext, ECN::ECSchemaR testSchema, DgnDbTestDgnManager tdm);

protected:
    static WCharCP const TEST_CLASS_NAME;
    static WCharCP const TEST_SCHEMA_NAME;
    static WCharCP const TEST_COMPLEX_SCHEMA_NAME;
    static const int TESTCLASS_INSTANCE_COUNT;

    DgnPlatform::ScopedDgnHost       m_host;

protected:
    PerformanceTestFixture() {};
    virtual ~PerformanceTestFixture () {};

    static void ImportTestSchema(ECN::ECSchemaPtr& schema, DgnDbTestDgnManager tdm, int numIntProperties, int numStringProperties);
    static void ImportComplexTestSchema (ECN::ECSchemaPtr& schema, DgnDbTestDgnManager tdm);
    static void AssignRandomECValue (ECN::ECValue& ecValue, ECN::ECPropertyCR ecProp);
    static void LogResultsToFile(bmap<Utf8String, double> results);
    };

END_DGNDB_UNIT_TESTS_NAMESPACE
