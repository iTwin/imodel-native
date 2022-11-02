/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitCopyTest : ECTestFixture {};

// NOTE: A lot of testing is done through copy tests for entire schemas and schema items which reference units

void validateUnitAttributes(ECUnitCP expected, ECUnitCP actual)
    {
    EXPECT_STREQ(expected->GetName().c_str(), actual->GetName().c_str());
    // NOTE: This is true today but won't always be true when we properly update definition when references are not copied.
    EXPECT_STREQ(expected->GetDefinition().c_str(), actual->GetDefinition().c_str());
    EXPECT_STREQ(expected->GetDisplayLabel().c_str(), actual->GetDisplayLabel().c_str());
    EXPECT_STREQ(expected->GetDescription().c_str(), actual->GetDescription().c_str());
    EXPECT_EQ(expected->GetDenominator(), actual->GetDenominator());
    EXPECT_EQ(expected->GetNumerator(), actual->GetNumerator());
    EXPECT_EQ(expected->GetOffset(), actual->GetOffset());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitCopyTest, BasicCopy)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    PhenomenonP phen;
    EC_ASSERT_SUCCESS(ecSchema->CreatePhenomenon(phen, "BananaLength", "BananaLength", "Banana Lengths!", "Measurements used for Banana lengths at Dole"));
    PhenomenonP bananaSlope;
    EC_ASSERT_SUCCESS(ecSchema->CreatePhenomenon(bananaSlope, "BananaSlope", "BananaLength*BananaLength^-1", "Banana Slope!", "Measurements used for banana slope at Dole"));
    UnitSystemP system;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnitSystem(system, "Banana", "Banana!", "Measurements used for Bananas"));
    ECUnitP smidgen;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnit(smidgen, "Smidgen", "Smidgen", *phen, *system, "Smidge", "Base unit for measuring banana length at Dole"));
    ECUnitP superSmidgen;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnit(superSmidgen, "SuperSmidgen", "Smidgen", *phen, *system, 8.0, 5.0, 0.0, "Super Smidgen", "Like a super saiyan with smidgens at Dole."));
    ECUnitP smidgePerSuperSmidge;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnit(smidgePerSuperSmidge, "SmidgePerSuperSmidge", "Smidgen*SuperSmidgen^-1", *bananaSlope, *system, "Smidge Per SuperSmidge", "A unit of measurement for banana slope at Dole."));
    ECUnitP superSmidgePerSmidgeInv;
    EC_ASSERT_SUCCESS(ecSchema->CreateInvertedUnit(superSmidgePerSmidgeInv, *smidgePerSuperSmidge, "InverseSuperSmidgePerSmidge", *system, "Inverted SuperSmidge per Smidge", "Helpful when measuring from the perspective of the banana basket"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    ECUnitP targetSmidge;
    EC_ASSERT_SUCCESS(targetSchema->CopyUnit(targetSmidge, *smidgen, false));
    EXPECT_EQ(1, targetSchema->GetReferencedSchemas().size());
    EXPECT_EQ(1, targetSchema->GetUnitCount());
    EXPECT_EQ(0, targetSchema->GetPhenomenonCount());
    EXPECT_EQ(0, targetSchema->GetUnitSystemCount());
    validateUnitAttributes(smidgen, targetSmidge);
    EXPECT_EQ(phen, targetSmidge->GetPhenomenon());
    EXPECT_EQ(system, targetSmidge->GetUnitSystem());

    ECUnitP targetSuperSmidgePerSmidgeInv;
    // NOTE: This unit is an example of a unit whose definition is not updated when copied ... it should be updated.
    EC_ASSERT_SUCCESS(targetSchema->CopyUnit(targetSuperSmidgePerSmidgeInv, *superSmidgePerSmidgeInv, false));
    validateUnitAttributes(superSmidgePerSmidgeInv, targetSuperSmidgePerSmidgeInv);
    EXPECT_EQ(1, targetSchema->GetReferencedSchemas().size());
    EXPECT_EQ(2, targetSchema->GetUnitCount());
    EXPECT_EQ(0, targetSchema->GetPhenomenonCount());
    EXPECT_EQ(0, targetSchema->GetUnitSystemCount());
    EXPECT_EQ(bananaSlope, targetSuperSmidgePerSmidgeInv->GetPhenomenon());
    EXPECT_EQ(system, targetSuperSmidgePerSmidgeInv->GetUnitSystem());
    EXPECT_EQ(smidgePerSuperSmidge, targetSuperSmidgePerSmidgeInv->GetInvertingUnit());


    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "Dole", "d", 43, 44, 45));
    EXPECT_EQ(ECObjectsStatus::NotFound, targetSchema->CopyUnit(targetSuperSmidgePerSmidgeInv, *superSmidgePerSmidgeInv, false));
    EXPECT_EQ(nullptr, targetSchema->GetUnitCP("InverseSuperSmidgePerSmidge"));


    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "Chiquita", "c", 1, 2, 3));
    EC_ASSERT_SUCCESS(targetSchema->CopyUnit(targetSuperSmidgePerSmidgeInv, *superSmidgePerSmidgeInv, true));
    validateUnitAttributes(superSmidgePerSmidgeInv, targetSuperSmidgePerSmidgeInv);
    EXPECT_EQ(0, targetSchema->GetReferencedSchemas().size());
    EXPECT_EQ(2, targetSchema->GetUnitCount());      // NOTE: Should be 4, units referenced via the definition are not currently copied, they should be copied when copyReferences is true 
    EXPECT_EQ(1, targetSchema->GetPhenomenonCount());  // NOTE: Should be 2, phen referenced via the definition are not currently copied, they should be copied when copyReferences is true
    EXPECT_EQ(1, targetSchema->GetUnitSystemCount());
    EXPECT_NE(bananaSlope, targetSuperSmidgePerSmidgeInv->GetPhenomenon());
    EXPECT_NE(system, targetSuperSmidgePerSmidgeInv->GetUnitSystem());
    EXPECT_NE(smidgePerSuperSmidge, targetSuperSmidgePerSmidgeInv->GetInvertingUnit());

    EC_ASSERT_SUCCESS(targetSchema->CopyUnit(targetSmidge, *smidgen, true));
    EXPECT_EQ(0, targetSchema->GetReferencedSchemas().size());
    EXPECT_EQ(3, targetSchema->GetUnitCount());      // NOTE: this is a base unit so it does not need to copy any references based on it's definition
    EXPECT_EQ(2, targetSchema->GetPhenomenonCount());
    EXPECT_EQ(1, targetSchema->GetUnitSystemCount());
    validateUnitAttributes(smidgen, targetSmidge);
    EXPECT_NE(phen, targetSmidge->GetPhenomenon());
    EXPECT_NE(system, targetSmidge->GetUnitSystem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitCopyTest, NameConflict)
    {
    ECSchemaPtr ecSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(ecSchema, "Dole", "d", 42, 42, 42));
    PhenomenonP phen;
    EC_ASSERT_SUCCESS(ecSchema->CreatePhenomenon(phen, "BananaLength", "BananaLength", "Dole Banana Lengths!", "Measurements used for Banana lengths at Dole"));
    UnitSystemP system;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnitSystem(system, "Banana", "Banana!", "Measurements used for Bananas"));
    ECUnitP smidgen;
    EC_ASSERT_SUCCESS(ecSchema->CreateUnit(smidgen, "Smidgen", "Smidgen", *phen, *system, "Smidge", "Base unit for measuring banana length at Dole"));

    ECSchemaPtr targetSchema;
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(targetSchema, "DelMonte", "dm", 3, 3, 3));
    PhenomenonP delMontePhen;
    EC_ASSERT_SUCCESS(targetSchema->CreatePhenomenon(delMontePhen, "Smidgen", "Smidgen", "Smidge", "Base unit for measuring banana length at Del Monte"));
    ECUnitP targetSmidgen;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, targetSchema->CopyUnit(targetSmidgen, *smidgen, false));
    EXPECT_EQ(nullptr, targetSmidgen);
    EXPECT_EQ(0, targetSchema->GetReferencedSchemas().size());
    targetSmidgen = targetSchema->GetUnitP("Smidgen");
    ASSERT_EQ(nullptr, targetSmidgen);
    EXPECT_NE(nullptr, targetSchema->GetPhenomenonCP("Smidgen"));
    }
END_BENTLEY_ECN_TEST_NAMESPACE
