/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"
#include <BeRapidJson/BeRapidJson.h>

BEGIN_ECDBUNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
struct BulkCrudTestFixture : public ECDbTestFixture
    {
    protected:
        //! Represents test instances on which the tests will operate
        //! the test instances are persisted into a plain SQLite file where the prop value pairs of an instance
        //! are stored in a JSON format understood by the JsonInserter
        struct TestDataset
            {
            private:
                Db m_dataDb;

                BentleyStatus Setup(ECDbCR);

                BentleyStatus InsertTestInstance(ECDbCR, ECN::ECClassCR, bool ignoreNullableProps);
                static BentleyStatus GeneratePropValuePairs(ECDbCR, rapidjson::Value& json, ECN::ECPropertyIterableCR, bool ignoreNullableProps, rapidjson::MemoryPoolAllocator<>&);
                static BentleyStatus GeneratePropValuePair(ECDbCR, rapidjson::Value&, ECN::ECPropertyCR prop, bool ignoreNullableMemberProps, rapidjson::MemoryPoolAllocator<>&);
                static BentleyStatus GeneratePrimitiveValue(rapidjson::Value&, ECN::PrimitiveType, rapidjson::MemoryPoolAllocator<>&);

                static bool IsNullableProperty(ECN::ECPropertyCR);

            public:
                TestDataset() {}

                BentleyStatus Populate(ECDbCR);

                Db& GetDb() { return m_dataDb; }

                static BentleyStatus ParseJson(rapidjson::Document& json, Utf8CP jsonStr);
            };

        void AssertInsert(TestDataset&);
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