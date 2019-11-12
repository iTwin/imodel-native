/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include "../TestFixture/DgnDbTestFixtures.h"

typedef bpair<Utf8String, double> T_TimerResultPair;

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle      09/2012
//=======================================================================================    
struct PerformanceTestFixture : public DgnDbTestFixture
    {
private:
    static void ImportSchema(ECN::ECSchemaReadContextR schemaContext, ECN::ECSchemaR testSchema, DgnDbR project);

protected:
    static Utf8CP const TEST_CLASS_NAME;
    static Utf8CP const TEST_SCHEMA_NAME;
    static Utf8CP const TEST_COMPLEX_SCHEMA_NAME;
    static const int TESTCLASS_INSTANCE_COUNT;

protected:
    PerformanceTestFixture() {};
    virtual ~PerformanceTestFixture () {};

    static void ImportTestSchema(ECN::ECSchemaPtr& schema, DgnDbR project, int numIntProperties, int numStringProperties);
    static void ImportComplexTestSchema(ECN::ECSchemaPtr& schema, DgnDbR project);
    static void AssignRandomECValue (ECN::ECValue& ecValue, ECN::ECPropertyCR ecProp);
    static void LogResultsToFile(bmap<Utf8String, double> results);
    };

END_DGNDB_UNIT_TESTS_NAMESPACE
