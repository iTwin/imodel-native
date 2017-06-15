/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/Tests/ArchitecturalPhysicalTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ArchitecturalPhysicalBaseFixture.h"
#include <ArchitecturalPhysical\ArchitecturalPhysicalApi.h>
//#include <BuildingPhysical\BuildingPhysicalApi.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArchitecturalPhysicalBaseFixture, EnsureDomainsAreRegistered)
    {

    Dgn::DgnDomains::RegisterDomain(BentleyApi::ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
  //  Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
  //  Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingPhysical::BuildingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);


    //This should create a DGN db with building domain.
    
    BeFileName bimFileName;
    bimFileName.assign (L"ArchitecturalPhysical.bim");
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("DomainTestFile");
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, bimFileName, createProjectParams);
    ASSERT_TRUE(db.IsValid());
    DgnDomainCP architecturalDomain = db->Domains().FindDomain(ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != architecturalDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArchitecturalPhysicalBaseFixture, CreateDoorWithSAC)
    {

    }
