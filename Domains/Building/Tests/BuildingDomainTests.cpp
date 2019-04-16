/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "BuildingDomainBaseFixture.h"
#include <BuildingDomain\BuildingDomainApi.h>
#include <BeJsonCpp\BeJsonUtilities.h>
#include <Json\Json.h>
#include <Grids\gridsApi.h>

#define DOOR_CODE_VALUE       "D-001"
#define WINDOW_CODE_VALUE     "W-001"
#define WALL_CODE_VALUE       "WA-001"
#define WALL_LEAF_CODE_VALUE  "WL-001"
#define DYNAMIC_CODE_VALUE    "DYN-001"

#define DYNAMIC_CLASS_NAME           "MyRevitClass"
#define DYNAMIC_ASPECT_CLASS_NAME    "MyRevitAspectClass"
#define DYNAMIC_PROPERTY_NAME        "MyStringProp"
#define DOUBLE_TEST_VALUE            900.00
#define INT_TEST_VALUE               58
#define STRING_TEST_VALUE            "This is a test string"
#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"
#define DYNAMIC_SCHEMA_NAME          "SampleDynamic"


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
//	DgnDomainCP buildingCommonDomain = db->Domains().FindDomain(BuildingCommon::BuildingCommonDomain::GetDomain().GetDomainName());
//	ASSERT_TRUE(NULL != buildingCommonDomain);
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
	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME, *db ));

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	// Testing the minimal arguments for CreateBuildingModels with supplied subject

	Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

	Dgn::SubjectPtr subject = Dgn::Subject::Create(*parentSubject, MODEL_TEST_NAME1);

	ASSERT_TRUE(subject.IsValid());

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = subject->Insert(&status);

	ASSERT_TRUE(element.IsValid() && Dgn::DgnDbStatus::Success == status);

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME1, *db, subject));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME1, *db);

	// Testing the minimal arguments for CreateBuildingModels where we are supressing the creation of the dynamic schema

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME2, *db, nullptr, false));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME2, *db);

	ECN::ECSchemaCP schema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(schema == nullptr);

	// Test the creation with a supplied schema

	ECN::ECSchemaPtr dynSchema;

	ASSERT_TRUE(ECN::ECObjectsStatus::Success == ECN::ECSchema::CreateSchema(dynSchema, DYNAMIC_SCHEMA_NAME, "DYN", 1, 1, 0));

	ECN::ECSchemaCP bisSchema = db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);

	ASSERT_FALSE(nullptr == bisSchema);

	ASSERT_TRUE(ECN::ECObjectsStatus::Success == dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*bisSchema)));

	ASSERT_FALSE(BuildingDomain::BuildingDomainUtilities::CreateBuildingModels(MODEL_TEST_NAME3, *db, nullptr, false, dynSchema));

	buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, *db);

	schema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(schema != nullptr);

	ASSERT_TRUE(schema->GetName() == DYNAMIC_SCHEMA_NAME);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, CreateFunctionalPartion)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	Dgn::FunctionalModelPtr functionModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingFunctionalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(functionModel.IsValid());


	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, CreateDocumentListPartion)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	Dgn::DocumentListModelPtr model = BuildingDomain::BuildingDomainUtilities::CreateBuildingDocumentListModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(model.IsValid());

	for (int i = 0; i < 100; i++)
		{
		Utf8String drawingName;
		drawingName.Sprintf("PID-%d", i);
		Dgn::DrawingModelPtr drawingModel = BuildingDomain::BuildingDomainUtilities::CreateBuildingDrawingModel(drawingName, *db, *model);
		ASSERT_TRUE(drawingModel.IsValid());
		}



	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, GetBuildingModel)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelPtr       buildingModel     = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(buildingModel.IsValid());
	BuildingPhysical::BuildingTypeDefinitionModelPtr buildingTypeModel = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel(MODEL_TEST_NAME, *db);
	ASSERT_TRUE(buildingTypeModel.IsValid());

	Dgn::DgnCode subjectCode = Dgn::Subject::CreateCode(*(db->Elements().GetRootSubject()), MODEL_TEST_NAME1);

	ASSERT_TRUE(subjectCode.IsValid());

	Dgn::DgnElementId          subjectId = db->Elements().QueryElementIdByCode(subjectCode);
	Dgn::SubjectCPtr           subject   = db->Elements().Get<Dgn::Subject>(subjectId);

	BuildingPhysical::BuildingPhysicalModelPtr       buildingModel1     = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME1, *db, subject);
	ASSERT_TRUE(buildingModel1.IsValid());
	BuildingPhysical::BuildingTypeDefinitionModelPtr buildingTypeModel1 = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel(MODEL_TEST_NAME1, *db, subject);
	ASSERT_TRUE(buildingTypeModel1.IsValid());

	BuildingPhysical::BuildingPhysicalModelPtr       buildingModel2     = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME2, *db);
	ASSERT_TRUE(buildingModel2.IsValid());
	BuildingPhysical::BuildingTypeDefinitionModelPtr buildingTypeModel2 = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel(MODEL_TEST_NAME2, *db);
	ASSERT_TRUE(buildingTypeModel2.IsValid());

	BuildingPhysical::BuildingPhysicalModelPtr       buildingModel3     = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, *db);
	ASSERT_TRUE(buildingModel3.IsValid());
	BuildingPhysical::BuildingTypeDefinitionModelPtr buildingTypeModel3 = BuildingDomain::BuildingDomainUtilities::GetBuildingTypeDefinitionModel(MODEL_TEST_NAME3, *db);
	ASSERT_TRUE(buildingTypeModel2.IsValid());

	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, AddClassesToDynamicSchema)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	ECN::ECSchemaPtr schema = BuildingDomain::BuildingDomainUtilities::GetUpdateableSchema(buildingModel);

	ASSERT_TRUE(schema.IsValid());

	ECN::ECEntityClassP newClass = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElementEntityClass(db, schema, DYNAMIC_CLASS_NAME);

	ASSERT_TRUE(nullptr != newClass);

	ECN::PrimitiveECPropertyP myProp;

	newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(Dgn::SchemaStatus::Success == BuildingDomain::BuildingDomainUtilities::UpdateSchemaInDb(*db, *schema));

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

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME3, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	ECN::ECSchemaPtr schema = BuildingDomain::BuildingDomainUtilities::GetUpdateableSchema(buildingModel);

	ASSERT_TRUE(schema.IsValid());

	ECN::ECEntityClassP newClass = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElementEntityClass(db, schema, DYNAMIC_CLASS_NAME);

	ASSERT_TRUE(nullptr != newClass);

	ECN::PrimitiveECPropertyP myProp;

	newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(Dgn::SchemaStatus::Success == BuildingDomain::BuildingDomainUtilities::UpdateSchemaInDb(*db, *schema));

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

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

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

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

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

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr wall = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_NAME, AP_CLASS_Wall, *buildingModel);

	ASSERT_TRUE(wall.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, WALL_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == wall->SetCode(code));

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, GROSS_FOOTPRINT_AREA, "SQ_IN", DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, NET_FOOTPRINT_AREA,   "SQ_FT", DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, GROSS_SIDE_AREA,      "SQ_M",  DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, NET_SIDE_AREA,        "SQ_KM", DOUBLE_TEST_VALUE));


    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, GROSS_VOLUME, "CUB_M",  DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, NET_VOLUME,   "CUB_FT", DOUBLE_TEST_VALUE));

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, HEIGHT, "FT", DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, LENGTH, "M",  DOUBLE_TEST_VALUE));
    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::SetDoublePropertyUsingUnitString(*wall, WIDTH,  "IN", DOUBLE_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr element = wall->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedWall = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, WALL_CODE_VALUE);

	ASSERT_TRUE(queriedWall.IsValid());

	ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, GROSS_FOOTPRINT_AREA, "SQ_IN", testValue));
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, NET_FOOTPRINT_AREA, "SQ_FT", testValue));
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, GROSS_SIDE_AREA, "SQ_M", testValue));
    ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, NET_SIDE_AREA, "SQ_KM", testValue));
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, GROSS_VOLUME, "CUB_M", testValue));
    ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, NET_VOLUME, "CUB_FT", testValue));
    ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, HEIGHT, "FT", testValue));
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, LENGTH, "M", testValue));
    ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == BuildingDomain::BuildingDomainUtilities::GetDoublePropertyUsingUnitString(*queriedWall, WIDTH, "IN", testValue));
	ASSERT_NEAR(testValue, DOUBLE_TEST_VALUE, 0.00001);

	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, WallLeafClassTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

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

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

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


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, AddAspectClassesToDynamicSchema)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	ECN::ECSchemaPtr schema = BuildingDomain::BuildingDomainUtilities::GetUpdateableSchema(buildingModel);

	ASSERT_TRUE(schema.IsValid());

	ECN::ECEntityClassP newClass = BuildingDomain::BuildingDomainUtilities::CreateUniqueAspetClass(db, schema, DYNAMIC_ASPECT_CLASS_NAME);

	ASSERT_TRUE(nullptr != newClass);

	ECN::PrimitiveECPropertyP myProp;

	newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(Dgn::SchemaStatus::Success == BuildingDomain::BuildingDomainUtilities::UpdateSchemaInDb(*db, *schema));

	ECN::ECSchemaCP updatedSchema = BuildingDomain::BuildingDomainUtilities::GetBuildingDynamicSchema(buildingModel);

	ASSERT_TRUE(nullptr != updatedSchema);

	ECN::ECClassCP foundClass = updatedSchema->GetClassCP(DYNAMIC_ASPECT_CLASS_NAME);

	ASSERT_TRUE(nullptr != foundClass);

	ASSERT_TRUE(foundClass->GetName() == DYNAMIC_ASPECT_CLASS_NAME);

	ECN::ECPropertyP prop = foundClass->GetPropertyP(DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(nullptr != prop);

	ASSERT_TRUE(prop->GetName() == DYNAMIC_PROPERTY_NAME);

	ASSERT_TRUE(prop->GetTypeName() == "string");

	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, DynamicClassInstancing)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	Utf8String schemaName = BuildingDomain::BuildingDomainUtilities::GetSchemaNameFromModel(buildingModel);

	Dgn::PhysicalElementPtr element = BuildingDomain::BuildingDomainUtilities::CreatePhysicalElement(schemaName, DYNAMIC_CLASS_NAME, *buildingModel);

	ASSERT_TRUE(element.IsValid());

	Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, *buildingModel, DYNAMIC_CODE_VALUE);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == element->SetCode(code));

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == element->SetPropertyValue(DYNAMIC_PROPERTY_NAME, STRING_TEST_VALUE));

	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr testElement = element->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedElement = BuildingDomain::BuildingDomainUtilities::QueryById<Dgn::PhysicalElement>(*buildingModel,testElement->GetElementId());

	ASSERT_TRUE(queriedElement.IsValid());

	ECN::ECValue propVal;
	double testValue;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROPERTY_NAME));
	Utf8String testString = propVal.GetUtf8CP();
	ASSERT_TRUE(testString == STRING_TEST_VALUE);

	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, AddingAspectsTests)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

	BuildingPhysical::BuildingPhysicalModelCPtr buildingModel = BuildingDomain::BuildingDomainUtilities::GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	ASSERT_TRUE(buildingModel.IsValid());

	Dgn::PhysicalElementPtr dynamicInstance = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, DYNAMIC_CODE_VALUE);

	ASSERT_TRUE(dynamicInstance.IsValid());

	Utf8String schemaName = BuildingDomain::BuildingDomainUtilities::GetSchemaNameFromModel(buildingModel);

	ECN::IECInstancePtr aspect = BuildingDomain::BuildingDomainUtilities::AddAspect(*buildingModel, dynamicInstance, schemaName.c_str(), DYNAMIC_ASPECT_CLASS_NAME);

	ASSERT_TRUE(aspect.IsValid());

	ECN::ECValue stringValue;

	stringValue.SetUtf8CP(STRING_TEST_VALUE);
	ASSERT_TRUE(ECN::ECObjectsStatus::Success == aspect->SetValue(DYNAMIC_PROPERTY_NAME, stringValue));

	Dgn::DgnElementCPtr element = dynamicInstance->Update();

	ASSERT_TRUE(element.IsValid());

	// Read back the data to make sure it was persisted correctly.

	Dgn::PhysicalElementPtr queriedInstance = BuildingDomain::BuildingDomainUtilities::QueryByCodeValue<Dgn::PhysicalElement>(*buildingModel, DYNAMIC_CODE_VALUE);

	ASSERT_TRUE(queriedInstance.IsValid());

	ECN::ECClassCP aspectClassP = buildingModel->GetDgnDb().GetClassLocater().LocateClass(schemaName.c_str(), DYNAMIC_ASPECT_CLASS_NAME);

	ASSERT_TRUE(nullptr != aspectClassP);

	ECN::IECInstanceCP asp = Dgn::DgnElement::GenericUniqueAspect::GetAspect(*queriedInstance, *aspectClassP);

	ASSERT_TRUE(nullptr != asp);

	ECN::ECValue value;

	ASSERT_TRUE ( ECN::ECObjectsStatus::Success == asp->GetValue(value, DYNAMIC_PROPERTY_NAME) );

	Utf8String testString = value.GetUtf8CP();

	ASSERT_TRUE(testString == STRING_TEST_VALUE);


	}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(BuildingDomainTestFixture, RadialTests)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    Dgn::SpatialLocationModelCPtr spatialModel =  BuildingDomain::BuildingDomainUtilities::CreateBuildingSpatialLocationModel(MODEL_TEST_NAME, *db);
    ASSERT_TRUE(spatialModel.IsValid());

#ifdef NOT_NOW
    Grids::RadialGridPortion::CreateParams params (&(*spatialModel), 2, 2, 10, 10, 20, 20, true);

    auto status = Grids::RadialGridPortion::CreateAndInsert(params);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status.IsValid());

    Grids::OrthogonalGridPortion::CreateParams params1 ( (&(*spatialModel), 2, 2, 10, 15, 20, 20, DVec3d::From(0, 0, 10), DVec3d::From(10, 0, 0));

    Grids::GridAxisMap grid2;

    status = Grids::OrthogonalGridPortion::CreateAndInsert(grid2, params1, DVec3d::From(0, 0, 1));
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);
#endif



    }


