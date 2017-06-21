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

#define DYNAMIC_CLASS_NAME    "MyRevitClass"
#define DYNAMIC_PROPERTY_NAME "MyStringProp"
#define DOUBLE_TEST_VALUE     900.00
#define INT_TEST_VALUE        58
#define STRING_TEST_VALUE     "This is a test string"
#define MODEL_TEST_NAME       "SampleModel"
#define OVERALL_WIDTH         "OverallWidth"
#define OVERALL_HEIGHT        "OverallHeight"
#define DESCRIPTION           "Description"

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuildingDomainTestFixture, EnsureDomainsAreRegistered)
    {

	DgnDbPtr db = CreateDgnDb();

	ASSERT_TRUE(db.IsValid());

	//This should create a DGN db with building domain.
	
	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::RegisterDomainHandlers());

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

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME, db ));

    }


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuildingDomainTestFixture, GetBuildingModel)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/


TEST_F(BuildingDomainTestFixture, AddClassesToDynamicSchema)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	ECN::ECSchemaPtr schema = BuildingDomain::BuildingDomainUtilities::GetUpdateableSchema(buildingModel);

	ASSERT_TRUE(schema.IsValid());

	ECN::ECEntityClassP newClass = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElementEntityClass(db, schema, DYNAMIC_CLASS_NAME);

	ASSERT_TRUE(nullptr != newClass);

	ECN::PrimitiveECPropertyP myProp;

	newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(Dgn::SchemaStatus::Success == BuildingDomain::BuildingDomainUtilities::UpdateSchemaInDb(db, schema));

	ECN::ECSchemaCP updatedSchema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(nullptr != updatedSchema);

	ECN::ECClassCP foundClass = updatedSchema->GetClassCP(DYNAMIC_CLASS_NAME);

	ASSERT_TRUE(nullptr != foundClass);

	ASSERT_TRUE(foundClass->GetName() == DYNAMIC_CLASS_NAME);

	ECN::ECPropertyP prop = foundClass->GetPropertyP(DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(nullptr != prop);

	ASSERT_TRUE(prop->GetName() == DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(prop->GetTypeName() == "string");

	}

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Vern.Francisco                 06/2017
 +---------------+---------------+---------------+---------------+---------------+------*/

TEST_F(BuildingDomainTestFixture, CreateDomainClasses)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr door = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Door, *buildingModel);

	ASSERT_TRUE(door.IsValid());

	char doorID[100];
	sprintf_s(doorID, "D%.3d", 1);

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, doorID);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(OVERALL_WIDTH, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(OVERALL_HEIGHT, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(DESCRIPTION, STRING_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = door->Insert( &status );
	        
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedDoor = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, doorID);

	ASSERT_TRUE(queriedDoor.IsValid());

	ECN::ECValue propVal;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedDoor->GetPropertyValue(propVal, OVERALL_WIDTH));

	double testValue = propVal.GetDouble();

	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedDoor->GetPropertyValue(propVal, OVERALL_HEIGHT));

	testValue = propVal.GetDouble();

	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedDoor->GetPropertyValue(propVal, DESCRIPTION));

	Utf8String testString = propVal.GetUtf8CP();

	ASSERT_TRUE(testString == STRING_TEST_VALUE);

	}


