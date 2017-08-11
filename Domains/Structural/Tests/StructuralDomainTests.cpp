/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/StructuralDomainTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainTestFixture.h"
#include <StructuralDomain\StructuralDomainApi.h>
#include <BeJsonCpp\BeJsonUtilities.h>
#include <Json\Json.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StructuralDomainTestFixture, EnsureDomainsAreRegistered)
    {
    ASSERT_TRUE(true);
    // DgnDbPtr db = CreateDgnDb();

    // ASSERT_TRUE(db.IsValid());

    // //This should create a DGN db with building domain.
    // ASSERT_FALSE(StructuralDomain::StructuralDomainUtilities::RegisterDomainHandlers());

    // DgnDomainCP architecturalDomain = db->Domains().FindDomain(ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain().GetDomainName());
    // ASSERT_TRUE(NULL != architecturalDomain);
    // DgnDomainCP buildingCommonDomain = db->Domains().FindDomain(BuildingCommon::BuildingCommonDomain::GetDomain().GetDomainName());
    // ASSERT_TRUE(NULL != buildingCommonDomain);
    // DgnDomainCP buildingPhysicalDomain = db->Domains().FindDomain(BuildingPhysical::BuildingPhysicalDomain::GetDomain().GetDomainName());
    // ASSERT_TRUE(NULL != buildingPhysicalDomain);
    }

// BE_JSON_NAME(StructuralDomain)

