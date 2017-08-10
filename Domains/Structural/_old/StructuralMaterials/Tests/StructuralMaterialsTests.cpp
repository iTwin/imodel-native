/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/StructuralMaterials/Tests/StructuralMaterialsTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralMaterialsBaseFixture.h"
#include <StructuralMaterials\StructuralMaterialsApi.h>
//#include <BuildingPhysical\BuildingPhysicalApi.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StructuralMaterialsBaseFixture, EnsureDomainsAreRegistered)
    {

    Dgn::DgnDomains::RegisterDomain(BentleyApi::StructuralMaterials::StructuralMaterialsDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
  //  Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingCommon::BuildingCommonDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);
  //  Dgn::DgnDomains::RegisterDomain(BentleyApi::BuildingPhysical::BuildingPhysicalDomain::GetDomain(), Dgn::DgnDomain::Required::Yes, Dgn::DgnDomain::Readonly::No);


    //This should create a DGN db with building domain.
    
    BeFileName bimFileName;
    bimFileName.assign (L"StructuralMaterials.bim");
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetRootSubjectName("DomainTestFile");
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, bimFileName, createProjectParams);
    ASSERT_TRUE(db.IsValid());
    DgnDomainCP StructuralMaterialsDomain = db->Domains().FindDomain(StructuralMaterials::StructuralMaterialsDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != StructuralMaterialsDomain);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StructuralMaterialsBaseFixture, CreateDoorWithSAC)
    {

    }
