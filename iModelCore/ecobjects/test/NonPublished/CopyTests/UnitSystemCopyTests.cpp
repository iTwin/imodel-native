/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitSystemCopyTest : ECTestFixture {};

// NOTE: A lot of testing is done through copy tests for entire schemas and schema items which reference UnitSystems

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemCopyTest, BasicCopy)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    UnitSystemP system;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnitSystem(system, "Banana", "Banana!", "Measurements used for Bananas"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    UnitSystemP targetSystem;
    EC_ASSERT_SUCCESS(targetSchema->CopyUnitSystem(targetSystem, *system));
    targetSystem = targetSchema->GetUnitSystemP("Banana");
    ASSERT_NE(nullptr, targetSystem);
    EXPECT_STREQ(system->GetName().c_str(), targetSystem->GetName().c_str());
    EXPECT_STREQ(system->GetDisplayLabel().c_str(), targetSystem->GetDisplayLabel().c_str());
    EXPECT_STREQ(system->GetDescription().c_str(), targetSystem->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitSystemCopyTest, NameConflict)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    UnitSystemP doleSystem;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnitSystem(doleSystem, "Banana", "Dole Banana!", "Measurements used for Bananas at Dole"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    UnitSystemP delMonteSystem;
    EC_ASSERT_SUCCESS(targetSchema->CreateUnitSystem(delMonteSystem, "Banana", "Del Monte Banana!", "Measurements used for Bananas at Del Monte"));
    UnitSystemP targetSystem;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, targetSchema->CopyUnitSystem(targetSystem, *doleSystem));
    targetSystem = targetSchema->GetUnitSystemP("Banana");
    ASSERT_NE(nullptr, targetSystem);
    EXPECT_STREQ("Banana", targetSystem->GetName().c_str());
    EXPECT_STREQ("Del Monte Banana!", targetSystem->GetDisplayLabel().c_str());
    EXPECT_STREQ("Measurements used for Bananas at Del Monte", targetSystem->GetDescription().c_str());
    }
END_BENTLEY_ECN_TEST_NAMESPACE
