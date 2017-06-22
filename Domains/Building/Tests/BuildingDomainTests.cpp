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

#define DOOR_CODE_VALUE       "D-001"
#define WINDOW_CODE_VALUE     "W-001"
#define WALL_CODE_VALUE       "WA-001"
#define WALL_LEAF_CODE_VALUE  "WL-001"

#define DYNAMIC_CLASS_NAME    "MyRevitClass"
#define DYNAMIC_PROPERTY_NAME "MyStringProp"
#define DOUBLE_TEST_VALUE     900.00
#define INT_TEST_VALUE        58
#define STRING_TEST_VALUE     "This is a test string"
#define MODEL_TEST_NAME       "SampleModel"
#define MODEL_TEST_NAME1      "SampleModelAAA"
#define MODEL_TEST_NAME2      "SampleModelBBB"
#define MODEL_TEST_NAME3      "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME   "SampleDynamic"


#define OVERALL_WIDTH         "OverallWidth"
#define OVERALL_HEIGHT        "OverallHeight"
#define DESCRIPTION           "Description"
#define GROSS_FOOTPRINT_AREA  "GrossFootprintArea"
#define NET_FOOTPRINT_AREA    "NetFootprintArea"
#define GROSS_SIDE_AREA       "GrossSideArea"
#define NET_SIDE_AREA         "NetSideArea"
#define GROSS_VOLUME          "GrossVolume"
#define NET_VOLUME            "NetVolume"
#define HEIGHT                "Height"
#define LENGTH                "Length"
#define WIDTH                 "Width"

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------
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

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, CreatePhysicalPartion)
    {

	DgnDbPtr db = CreateDgnDb();

	ASSERT_TRUE(db.IsValid());

	// Testing the minimal arguments for CreateBuildingModels
	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME, db ));

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	// Testing the minimal arguments for CreateBuildingModels with supplied subject

	Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

	Dgn::SubjectPtr subject = Dgn::Subject::Create(*parentSubject, MODEL_TEST_NAME1);

	ASSERT_TRUE(subject.IsValid());

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = subject->Insert(&status);

	ASSERT_TRUE(element.IsValid() && Dgn::DgnDbStatus::Success == status);

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME1, db, subject));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME1, db);

	// Testing the minimal arguments for CreateBuildingModels where we are supressing the creation of the dynamic schema

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME2, db, nullptr, false));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME2, db);

	ECN::ECSchemaCP schema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(schema == nullptr);

	// Test the creation with a supplied schema

	ECN::ECSchemaPtr dynSchema;

	ASSERT_TRUE(ECN::ECObjectsStatus::Success == ECN::ECSchema::CreateSchema(dynSchema, DYNAMIC_SCHEMA_NAME, "DYN", 1, 1, 0));

	ECN::ECSchemaCP bisSchema = db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);

	ASSERT_FALSE(nullptr == bisSchema);

	ASSERT_TRUE(ECN::ECObjectsStatus::Success == dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*bisSchema)));

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME3, db, nullptr, false, dynSchema));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, db);

	schema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(schema != nullptr);

	ASSERT_TRUE(schema->GetName() == DYNAMIC_SCHEMA_NAME);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, GetBuildingModel)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel  = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel1 = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME1, db);

	ASSERT_TRUE(buildingModel1.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel2 = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME2, db);

	ASSERT_TRUE(buildingModel2.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel3 = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, db);

	ASSERT_TRUE(buildingModel3.IsValid());
	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

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


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, AddClassesToSuppliedSchema)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, db);

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

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, DoorClassTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr door = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Door, *buildingModel);

	ASSERT_TRUE(door.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, DOOR_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(OVERALL_WIDTH, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(OVERALL_HEIGHT, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == door->SetPropertyValue(DESCRIPTION, STRING_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = door->Insert( &status );
	        
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedDoor = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, DOOR_CODE_VALUE);

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

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, WindowClassTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr window = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Window, *buildingModel);

	ASSERT_TRUE(window.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, WINDOW_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == window->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == window->SetPropertyValue(OVERALL_WIDTH, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == window->SetPropertyValue(OVERALL_HEIGHT, DOUBLE_TEST_VALUE));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == window->SetPropertyValue(DESCRIPTION, STRING_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = window->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedWindow = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WINDOW_CODE_VALUE);

	ASSERT_TRUE(queriedWindow.IsValid());

	ECN::ECValue propVal;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWindow->GetPropertyValue(propVal, OVERALL_WIDTH));

	double testValue = propVal.GetDouble();

	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWindow->GetPropertyValue(propVal, OVERALL_HEIGHT));

	testValue = propVal.GetDouble();

	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWindow->GetPropertyValue(propVal, DESCRIPTION));

	Utf8String testString = propVal.GetUtf8CP();

	ASSERT_TRUE(testString == STRING_TEST_VALUE);

	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, WallClassTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr wall = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Wall, *buildingModel);

	ASSERT_TRUE(wall.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, WALL_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(GROSS_FOOTPRINT_AREA, DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(NET_FOOTPRINT_AREA,   DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(GROSS_SIDE_AREA,      DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(NET_SIDE_AREA,        DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(GROSS_VOLUME,         DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(NET_VOLUME,           DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(HEIGHT,               DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(LENGTH,               DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetPropertyValue(WIDTH,                DOUBLE_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = wall->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedWall = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_CODE_VALUE);

	ASSERT_TRUE(queriedWall.IsValid());

	ECN::ECValue propVal;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, GROSS_FOOTPRINT_AREA));
	double testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, NET_FOOTPRINT_AREA));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, GROSS_SIDE_AREA));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);
	
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, NET_SIDE_AREA));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);
	
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, GROSS_VOLUME));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);
	
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, NET_VOLUME));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, HEIGHT));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, LENGTH));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWall->GetPropertyValue(propVal, WIDTH));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, WallLeafClassTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr wallLeaf = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_WallLeaf, *buildingModel);

	ASSERT_TRUE(wallLeaf.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, WALL_LEAF_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wallLeaf->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wallLeaf->SetPropertyValue(HEIGHT, DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wallLeaf->SetPropertyValue(LENGTH, DOUBLE_TEST_VALUE));
	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wallLeaf->SetPropertyValue(WIDTH, DOUBLE_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = wallLeaf->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedWallLeaf = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_LEAF_CODE_VALUE);

	ASSERT_TRUE(queriedWallLeaf.IsValid());

	ECN::ECValue propVal;
	double testValue;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWallLeaf->GetPropertyValue(propVal, HEIGHT));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWallLeaf->GetPropertyValue(propVal, LENGTH));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedWallLeaf->GetPropertyValue(propVal, WIDTH));
	testValue = propVal.GetDouble();
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, WallOwnsWallLeafsTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr queriedWall = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_CODE_VALUE);

	ASSERT_TRUE(queriedWall.IsValid());

	Dgn::PhysicalElementPtr queriedWallLeaf = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_LEAF_CODE_VALUE);

	ASSERT_TRUE(queriedWallLeaf.IsValid());

	ECN::ECClassCP relationshipClass = db->GetClassLocater().LocateClass(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_Class_WallOwnsWallLeafs);

	ASSERT_TRUE(nullptr != relationshipClass);

	Dgn::DgnDbStatus status = queriedWallLeaf->SetParentId(queriedWall->GetElementId(), relationshipClass->GetId());

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::DgnElementCPtr element = queriedWallLeaf->Update();

	ASSERT_TRUE(element.IsValid());

	queriedWallLeaf = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_LEAF_CODE_VALUE);

	ASSERT_TRUE(queriedWallLeaf.IsValid());

	ASSERT_TRUE(queriedWallLeaf->GetParentId() == queriedWall->GetElementId());

	Dgn::DgnElementIdSet children = queriedWall->QueryChildren();

	ASSERT_TRUE(children.size() == 1);

	ASSERT_TRUE(children.Contains(queriedWallLeaf->GetElementId()));


	}
