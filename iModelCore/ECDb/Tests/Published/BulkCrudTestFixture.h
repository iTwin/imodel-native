/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BulkCrudTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"
#include <rapidjson/BeRapidJson.h>

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
struct BulkCrudTestFixture : public ECDbTestFixture
    {
    protected:
        static BentleyStatus CreateTestData(ECDbCR, BeFileNameCR testDataJsonFile);
        static BentleyStatus CreateTestInstance(ECDbCR ecdb, rapidjson::Value& json, ECN::ECClassCR ecClass, bool ignoreNullableProps, rapidjson::MemoryPoolAllocator<>& allocator) { return AssignPropValuePairs(ecdb, json, ecClass.GetProperties(), ignoreNullableProps, allocator); }
        static BentleyStatus AssignPropValuePairs(ECDbCR, rapidjson::Value& json, ECN::ECPropertyIterableCR, bool ignoreNullableProps, rapidjson::MemoryPoolAllocator<>&);
        static BentleyStatus AssignPropValuePair(ECDbCR, rapidjson::Value&, ECN::ECPropertyCR prop, bool ignoreNullableMemberProps, rapidjson::MemoryPoolAllocator<>&);
        static BentleyStatus AssignPrimitiveValue(rapidjson::Value&, ECN::PrimitiveType, rapidjson::MemoryPoolAllocator<>&);

        static bool IsNullableProperty(ECN::ECPropertyCR);
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
struct BulkBisDomainCrudTestFixture : public BulkCrudTestFixture
    {
    private:
        BentleyStatus CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder);
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);

    protected:
        BentleyStatus SetupDomainBimFile(Utf8CP fileName, BeFileName const& domainSchemaFolder, BeFileName const& bisSchemaFolder);
        static BeFileName GetDomainSchemaFolder(BeFileName& bisSchemaFolder);
    };
END_ECDBUNITTESTS_NAMESPACE