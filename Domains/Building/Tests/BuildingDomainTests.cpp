/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BuildingDomainTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainBaseFixture.h"
#include <BuildingDomain\BuildingDomainApi.h>
#include <BeJsonCpp\BeJsonUtilities.h>
#include <Json\Json.h>
//#include <BuildingPhysical\BuildingPhysicalApi.h>

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuildingDomainTestFixture, EnsureDomainsAreRegistered)
    {

	//This should create a DGN db with building domain.
	
	DgnDbPtr db = CreateDgnDb();

	ASSERT_TRUE(db.IsValid());

    DgnDomainCP architecturalDomain = db->Domains().FindDomain(ArchitecturalPhysical::ArchitecturalPhysicalDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != architecturalDomain);
	DgnDomainCP buildingCommonDomain = db->Domains().FindDomain(BuildingCommon::BuildingCommonDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != buildingCommonDomain);
	DgnDomainCP buildingPhysicalDomain = db->Domains().FindDomain(BuildingPhysical::BuildingPhysicalDomain::GetDomain().GetDomainName());
	ASSERT_TRUE(NULL != buildingPhysicalDomain);
	}


BE_JSON_NAME(BuildingDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vern.Francisco                 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuildingDomainTestFixture, CreatePhysicalPartion)
    {

	DgnDbPtr db = CreateDgnDb();

	ASSERT_TRUE(db.IsValid());

	Dgn::SubjectCPtr rootSubject = db->Elements().GetRootSubject();

	// Create the partition and the BuildingPhysicalModel.   

	Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*rootSubject, "BuildingPhysicalModel");
	ASSERT_TRUE(partition.IsValid());

	BuildingPhysical::BuildingPhysicalModelPtr physicalModel = BuildingPhysical::BuildingPhysicalModel::Create(*partition);
	ASSERT_TRUE(physicalModel.IsValid());

	Json::Value val;

	val["TEST"] = "TestString";

	physicalModel->SetJsonProperties(json_BuildingDomain(), val);

	physicalModel->Update();

	Json::Value readVal;

	readVal = physicalModel->GetJsonProperties(json_BuildingDomain());

	Utf8String string = readVal["TEST"].asString();

	Dgn::DefinitionPartitionCPtr defPartition = Dgn::DefinitionPartition::CreateAndInsert(*rootSubject, "BuildingTypeDefinitionModel");
	ASSERT_TRUE(defPartition.IsValid());

	BuildingPhysical::BuildingTypeDefinitionModelPtr typeDefinitionModel = BuildingPhysical::BuildingTypeDefinitionModel::Create(*defPartition);

	ASSERT_TRUE(typeDefinitionModel.IsValid());

	typeDefinitionModel->SetJsonProperties(json_BuildingDomain(), val);

	typeDefinitionModel->Update();

    }


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuildingDomainTestFixture, ReadPhysicalPartion)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());


	}
