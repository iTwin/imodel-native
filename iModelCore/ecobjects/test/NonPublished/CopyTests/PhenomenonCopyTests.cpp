/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PhenomenonCopyTest : ECTestFixture {};

// NOTE: A lot of testing is done through copy tests for entire schemas and schema items which reference Phenomena

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonCopyTest, BasicCopy)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    PhenomenonP phen;
    EC_ASSERT_SUCCESS(ecSchema->CreatePhenomenon(phen, "BananaLength", "BananaLength", "Banana Lengths!", "Measurements used for Banana lengths at Dole"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    PhenomenonP targetPhen;
    EC_ASSERT_SUCCESS(targetSchema->CopyPhenomenon(targetPhen, *phen, false));
    targetPhen = targetSchema->GetPhenomenonP("BananaLength");
    ASSERT_NE(nullptr, targetPhen);
    EXPECT_STREQ(phen->GetName().c_str(), targetPhen->GetName().c_str());
    EXPECT_STREQ(phen->GetDefinition().c_str(), targetPhen->GetDefinition().c_str());
    EXPECT_STREQ(phen->GetDisplayLabel().c_str(), targetPhen->GetDisplayLabel().c_str());
    EXPECT_STREQ(phen->GetDescription().c_str(), targetPhen->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PhenomenonCopyTest, NameConflict)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    PhenomenonP phen;
    EC_ASSERT_SUCCESS(ecSchema->CreatePhenomenon(phen, "BananaLength", "BananaLength", "Dole Banana Lengths!", "Measurements used for Banana lengths at Dole"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    PhenomenonP delMontePhen;
    EC_ASSERT_SUCCESS(targetSchema->CreatePhenomenon(delMontePhen, "BananaLength", "BananaLength", "Del Monte Banana Lengths!", "Measurements used for Bananas lengths at Del Monte"));
    PhenomenonP targetPhen;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, targetSchema->CopyPhenomenon(targetPhen, *phen, false));
    targetPhen = targetSchema->GetPhenomenonP("BananaLength");
    ASSERT_NE(nullptr, targetPhen);
    EXPECT_STREQ("BananaLength", targetPhen->GetName().c_str());
    EXPECT_STREQ("BananaLength", targetPhen->GetDefinition().c_str());
    EXPECT_STREQ("Del Monte Banana Lengths!", targetPhen->GetDisplayLabel().c_str());
    EXPECT_STREQ("Measurements used for Bananas lengths at Del Monte", targetPhen->GetDescription().c_str());
    }
END_BENTLEY_ECN_TEST_NAMESPACE
