/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/AecUnitsTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AecUnitsBaseFixture.h"
#include <AecUnits\AecUnitsApi.h>
#include <BeJsonCpp\BeJsonUtilities.h>
#include <Json\Json.h>

#define DOOR_CODE_VALUE       "D-001"
#define WINDOW_CODE_VALUE     "W-001"
#define WALL_CODE_VALUE       "WA-001"
#define WALL_LEAF_CODE_VALUE  "WL-001"
#define DYNAMIC_CODE_VALUE    "DYN-001"

#define DYNAMIC_SCHEMA_NAME      "UnitTest"

#define DYNAMIC_CLASS_NAME           "UnitsTestClass"
#define DYNAMIC_ASPECT_CLASS_NAME    "MyRevitAspectClass"
#define DYNAMIC_PROPERTY_NAME        "MyUnitsProp"

#define DYNAMIC_ANGLE_PROPERTY_NAME                         "AngleProp"
#define DYNAMIC_AREA_PROPERTY_NAME                          "AreaProp"
#define DYNAMIC_AREA_SMALL_PROPERTY_NAME                    "AreaSmallProp"
#define DYNAMIC_AREA_LARGE_PROPERTY_NAME                    "AreaLargeProp"
#define DYNAMIC_LENGTH_PROPERTY_NAME                        "LengthProp"
#define DYNAMIC_LENGTH_LONG_PROPERTY_NAME                   "LengthLongProp"
#define DYNAMIC_LENGTH_SHORT_PROPERTY_NAME                  "LengthShortProp"
#define DYNAMIC_VOLUME_PROPERTY_NAME                        "VolumeProp"
#define DYNAMIC_VOLUME_SMALL_PROPERTY_NAME                  "VolumeSmallProp"
#define DYNAMIC_VOLUME_LARGE_PROPERTY_NAME                  "VolumeLargeProp"
#define DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME                 "LiquidVolumeProp"
#define DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME           "LiquidVolumeSmallProp"
#define DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME           "LiquidVolumeLargeProp"
#define DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME       "PPPressureProp"
#define DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME    "PPPTemperatureProp"
#define DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME           "PPPFlowProp"
#define DYNAMIC_WEIGHT_NAME                                 "WeightProp"
#define DYNAMIC_CURRENT_NAME                                "CurrentProp"
#define DYNAMIC_FORCE_NAME                                  "ForceProp"
#define DYNAMIC_POWER_NAME                                  "PowerProp"
#define DYNAMIC_HEAT_TRANSFER_NAME                          "HeatTransferProp"
#define DYNAMIC_LUMINOUS_FLUX_NAME                          "LUMINOUS_FLUXProp"
#define DYNAMIC_ILLUMINANCE_NAME                            "ILLUMINANCEProp"
#define DYNAMIC_LUMINOUS_INTENSITY_NAME                     "LUMINOUS_INTENSITYProp"


#define DYNAMIC_TEMPERATURE_NAME                            "TempProp"
#define DYNAMIC_PRESSURE_NAME                               "PressureProp"
#define DYNAMIC_FLOW_NAME                                   "FlowProp"
#define DYNAMIC_ELECTRICAL_POTENIAL_NAME                    "VoltProp"




#define DOUBLE_TEST_VALUE            900.00
#define INT_TEST_VALUE               58
#define STRING_TEST_VALUE            "This is a test string"
#define MODEL_TEST_NAME              "SampleModel"
#define MODEL_TEST_NAME1             "SampleModelAAA"
#define MODEL_TEST_NAME2             "SampleModelBBB"
#define MODEL_TEST_NAME3             "SampleModelCCC"


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
TEST_F(AecUnitsTestFixture, EnsureDomainsAreRegistered)
    {

	DgnDbPtr db = CreateDgnDb();

	ASSERT_TRUE(db.IsValid());

	//This should create a DGN db with building domain.

    DgnDomainCP aecUnitsDomain = db->Domains().FindDomain( AecUnits::AecUnitsDomain::GetDomain().GetDomainName());
    ASSERT_TRUE(NULL != aecUnitsDomain);
	}


BE_JSON_NAME(AecUnits)

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, CreatePhysicalPartion)
    {

	DgnDbPtr db = CreateDgnDb();

    ASSERT_TRUE(db.IsValid());

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    ASSERT_TRUE(parentSubject.IsValid());

    // Create the partition and the BuildingPhysicalModel.

    Utf8String phyModelCode = MODEL_TEST_NAME;

    Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*parentSubject, phyModelCode);

    ASSERT_TRUE(partition.IsValid());

    PhysicalModelPtr physicalModel = PhysicalModel::Create(*partition);

    ASSERT_TRUE(physicalModel.IsValid());

    physicalModel->Insert();

    ECN::ECSchemaPtr dynSchema;

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == ECN::ECSchema::CreateSchema(dynSchema, DYNAMIC_SCHEMA_NAME, DYNAMIC_SCHEMA_NAME, 1, 1, 0));

    ECN::ECSchemaCP bisSchema = db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);

    ASSERT_TRUE(nullptr != bisSchema);

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*bisSchema)));

    ECN::ECSchemaCP unitSchema = db->Schemas().GetSchema(BENTLEY_AEC_UNITS_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != unitSchema);

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == dynSchema->AddReferencedSchema((ECN::ECSchemaR)(*unitSchema)));

    bvector<ECN::ECSchemaCP> schemas;

    ECN::ECSchemaCP a = &(*dynSchema);

    schemas.push_back(a);

    ASSERT_TRUE(Dgn::SchemaStatus::Success == db->ImportSchemas(schemas));

    ASSERT_TRUE( BeSQLite::DbResult::BE_SQLITE_OK == db->SaveChanges());

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, AddClassesToDynamicSchema)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    auto context = ECN::ECSchemaReadContext::CreateContext(false);

    ECN::SchemaKey k(schema->GetSchemaKey());

    ECN::ECSchemaPtr currSchema = db->GetSchemaLocater().LocateSchema(k, ECN::SchemaMatchType::Exact, *context);

    ECN::ECSchemaPtr updateableSchema;

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == currSchema->CopySchema(updateableSchema));

    updateableSchema->SetVersionMinor(currSchema->GetVersionMinor() + 1);

    ECN::ECSchemaCP unitsSchema = db->Schemas().GetSchema(BENTLEY_AEC_UNITS_SCHEMA_NAME);
    ASSERT_TRUE(nullptr != unitsSchema);

	//PhysicalModelCPtr buildingModel = GetBuildingPhyicalModel(MODEL_TEST_NAME, *db);

	//ASSERT_TRUE(buildingModel.IsValid());

	//ECN::ECSchemaPtr schema = AecUnits::AecUnitsUtilities::GetUpdateableSchema(buildingModel);

	//ASSERT_TRUE(schema.IsValid());

    ECN::ECSchemaCP bisSchema = db->Schemas().GetSchema(BIS_ECSCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    ECN::ECClassCP baseClass = bisSchema->GetClassCP(BIS_CLASS_PhysicalElement);

    ASSERT_TRUE(nullptr != baseClass);

    ECN::ECEntityClassP newClass;

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == updateableSchema->CreateEntityClass(newClass, DYNAMIC_CLASS_NAME));

    ASSERT_TRUE(ECN::ECObjectsStatus::Success == newClass->AddBaseClass(*baseClass));

	ASSERT_TRUE(nullptr != newClass);

	ECN::PrimitiveECPropertyP myProp;

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ANGLE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    ECN::KindOfQuantityCP locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ANGLE);
    myProp->SetKindOfQuantity(locatedKOQ);


	newClass->CreatePrimitiveProperty(myProp, DYNAMIC_AREA_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_AREA);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_AREA_SMALL_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_AREA_SMALL);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_AREA_LARGE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_AREA_LARGE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LENGTH_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LENGTH);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LENGTH_SHORT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LENGTH_LONG_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LENGTH_LONG);
    myProp->SetKindOfQuantity(locatedKOQ);


    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_VOLUME_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_VOLUME);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_VOLUME_SMALL);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_VOLUME_LARGE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LIQUID_VOLUME);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LIQUID_VOLUME_SMALL);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LIQUID_VOLUME_LARGE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_PROCESS_PIPING_PRESSURE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_PROCESS_PIPING_TEMPERATURE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_PROCESS_PIPING_FLOW);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_WEIGHT_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_WEIGHT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_CURRENT_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_CURRENT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_FORCE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_FORCE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_POWER_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_POWER);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_TEMPERATURE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_TEMPERATURE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PRESSURE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_PRESSURE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_FLOW_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_FLOW);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ELECTRICAL_POTENIAL_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ELECTRIC_POTENTIAL);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_HEAT_TRANSFER_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_HEAT_TRANSFER);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LUMINOUS_FLUX_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LUMINOUS_FLUX);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ILLUMINANCE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ILLUMINANCE);
    myProp->SetKindOfQuantity(locatedKOQ);


    bvector<ECN::ECSchemaCP> updatedSchemas;

    ECN::ECSchemaCP b = &(*updateableSchema);

    updatedSchemas.push_back(b);

    ASSERT_TRUE(Dgn::SchemaStatus::Success == db->ImportSchemas(updatedSchemas));

    ECN::ECSchemaCP updatedSchema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

	ASSERT_TRUE(nullptr != updatedSchema);

	ECN::ECClassCP foundClass = updatedSchema->GetClassCP(DYNAMIC_CLASS_NAME);

	ASSERT_TRUE(nullptr != foundClass);

	ASSERT_TRUE(foundClass->GetName() == DYNAMIC_CLASS_NAME);

	ECN::ECPropertyP prop = foundClass->GetPropertyP(DYNAMIC_AREA_PROPERTY_NAME);

	ASSERT_TRUE(nullptr != prop);

	ASSERT_TRUE(prop->GetName() == DYNAMIC_AREA_PROPERTY_NAME);

	ASSERT_TRUE(prop->GetTypeName() == "double");

	}



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, AngleUnitsTest)
	{
	DgnDbPtr db = OpenDgnDb();

	ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId   = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition     = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

	Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

	ASSERT_TRUE(element.IsValid());

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_DEG, 180));
    
	Dgn::DgnDbStatus status;

	Dgn::DgnElementCPtr testElement = element->Insert(&status);

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

	Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel,testElement->GetElementId());

	ASSERT_TRUE(queriedElement.IsValid());

	ECN::ECValue propVal;
	double testValue;

	ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ANGLE_PROPERTY_NAME));
	double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, PI, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_DEG, testDouble);
    ASSERT_NEAR(testDouble, 180.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_RAD, testDouble);
    ASSERT_NEAR(testDouble, PI, 0.00001);

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_RAD, PI));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ANGLE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, PI, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_DEG, testDouble);
    ASSERT_NEAR(testDouble, 180.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGLE_PROPERTY_NAME, AEC_UNIT_RAD, testDouble);
    ASSERT_NEAR(testDouble, PI, 0.00001);


	}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, AreaUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using SQ FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_FT, 100));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 9.2903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 14400.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 9.2903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 9290304.0, 0.00001);

    // Set using SQ IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_IN, 100));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.064516, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.064516, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 64516.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 0.69444, 0.00001);

    // Set using SQ M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_M, 100));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 155000.31, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 100000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1076.391, 0.001);


    // Set using SQ M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_MM, 100000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 155000.31, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 100000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1076.391, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, SmallAreaUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using SQ FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_FT, 1));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_SMALL_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    // Set using SQ IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_IN, 144));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    // Set using SQ M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_M, 0.09290304));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144., 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    // Set using SQ M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_MM, 92903.04));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144., 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_SMALL_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1., 0.001);


    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LargeAreaUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using SQ FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_FT, 1));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_LARGE_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    // Set using SQ IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_IN, 144));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    // Set using SQ M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_M, 0.09290304));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144., 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    // Set using SQ MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_MM, 92903.04));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_AREA_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_IN, testDouble);
    ASSERT_NEAR(testDouble, 144., 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.092903, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_MM, testDouble);
    ASSERT_NEAR(testDouble, 92903.04, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_AREA_LARGE_PROPERTY_NAME, AEC_UNIT_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 1., 0.001);


    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LengthUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_FT, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    // Set using IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_IN, 12));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    // Set using M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_M, 0.3048));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    // Set using  MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_MM, 304.80));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LengthLongUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_FT, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_LONG_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    // Set using IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_IN, 12));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_LONG_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    // Set using M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_M, 0.3048));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_LONG_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.30480, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    // Set using  MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_MM, 304.80));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_LONG_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_LONG_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LengthShortUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_FT, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    // Set using IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_IN, 12));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    // Set using M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_M, 0.3048));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    // Set using  MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_MM, 304.80));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_IN, testDouble);
    ASSERT_NEAR(testDouble, 12.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_M, testDouble);
    ASSERT_NEAR(testDouble, 0.3048, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_MM, testDouble);
    ASSERT_NEAR(testDouble, 304.80, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LENGTH_SHORT_PROPERTY_NAME, AEC_UNIT_FT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, VolumeUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

//    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
//    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, VolumeSmallUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, VolumeLargeUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LargeLiquidVolumeUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_LARGE_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, SmallLiquidVolumeUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_SMALL_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LiquidVolumeUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB_MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, 1000000000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using CUB IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, 61023.7441));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);



    // Set using CUB M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB FT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, 35.314666721));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using CUB YRD

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, 1.3079506193));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    // Set using Litre

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);


    // Set using Gallon

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, 264.17205236));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_MM, testDouble);
    ASSERT_NEAR(testDouble, 1000000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, 61023.7441, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, 35.31467, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_CUB_YRD, testDouble);
    ASSERT_NEAR(testDouble, 1.30795, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_LITRE, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    //    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_ML, testDouble);
    //    ASSERT_NEAR(testDouble, 1000000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LIQUID_VOLUME_PROPERTY_NAME, AEC_UNIT_GAL, testDouble);
    ASSERT_NEAR(testDouble, 264.172, 0.01);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PlantPressureUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using PA

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PA, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

    // Set using PAG

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PAG, -101324.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

    // Set using PSI

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSI, 0.000145038));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);


    // Set using PSIG

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSIG, -14.695803737775718));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PressureUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using PA

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PA, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

    // Set using PAG

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PAG, -101324.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

    // Set using PSI

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSI, 0.000145038));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);


    // Set using PSIG

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSIG, -14.695803737775718));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PAG, testDouble);
    ASSERT_NEAR(testDouble, -101324.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSI, testDouble);
    ASSERT_NEAR(testDouble, 0.000145038, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_NAME, AEC_UNIT_PSIG, testDouble);
    ASSERT_NEAR(testDouble, -14.69580374, 0.00001);

}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PlantTemperatureUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CELSIUS

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_CELSIUS, 0.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 0.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 32.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 273.15, 0.00001);


    // Set using FAHRENHEIT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_FAHRENHEIT, 32.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 0.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 32.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_TEMPERATURE_PROPERTY_NAME, AEC_UNIT_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

    }
//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, TemperatureUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CELSIUS

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_CELSIUS, 0.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TEMPERATURE_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 0.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 32.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 273.15, 0.00001);


    // Set using FAHRENHEIT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_FAHRENHEIT, 32.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TEMPERATURE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 0.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 32.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TEMPERATURE_NAME, AEC_UNIT_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 273.15, 0.00001);

}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, CurrentUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using Amps

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_CURRENT_NAME, AEC_UNIT_AMP, 10.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_CURRENT_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 10.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_CURRENT_NAME, AEC_UNIT_AMP, testDouble);
    ASSERT_NEAR(testDouble, 10.0, 0.00001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, WeightUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using KG

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_WEIGHT_NAME, AEC_UNIT_KG, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testDouble;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_WEIGHT_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_KG, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_LBM, testDouble);
    ASSERT_NEAR(testDouble, 2.20462, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_G, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    // Set using LB Mass

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_LBM, 2.20462));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_WEIGHT_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_KG, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_LBM, testDouble);
    ASSERT_NEAR(testDouble, 2.20462, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_G, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.01);

    // Set using Grams

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_G, 1000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_WEIGHT_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_KG, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_LBM, testDouble);
    ASSERT_NEAR(testDouble, 2.20462, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WEIGHT_NAME, AEC_UNIT_G, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ForcetUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using Newtons

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_FORCE_NAME, AEC_UNIT_N, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testDouble;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_N, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_LBF, testDouble);
    ASSERT_NEAR(testDouble, 0.224808943870962, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KN, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KIP, testDouble);
    ASSERT_NEAR(testDouble, 0.000224808943870962, 0.00000001);

    // Set using LB Force

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_LBF, 0.224808943870962));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_N, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_LBF, testDouble);
    ASSERT_NEAR(testDouble, 0.224808943870962, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KN, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KIP, testDouble);
    ASSERT_NEAR(testDouble, 0.000224808943870962, 0.00000001);

    // Set using Kilo-Newtons

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KN, 0.001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_N, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_LBF, testDouble);
    ASSERT_NEAR(testDouble, 0.224808943870962, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KN, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KIP, testDouble);
    ASSERT_NEAR(testDouble, 0.000224808943870962, 0.00000001);


    // Set using Kips force

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KIP, 0.000224808943870962));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_N, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_LBF, testDouble);
    ASSERT_NEAR(testDouble, 0.224808943870962, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KN, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_NAME, AEC_UNIT_KIP, testDouble);
    ASSERT_NEAR(testDouble, 0.000224808943870962, 0.00000001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PowerUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using Watts

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_POWER_NAME, AEC_UNIT_W, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testDouble;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);


    // Set using KW

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, 0.001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);

    // Set using MegaWatts

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, 0.000001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);

    // Set using BTU/HR

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, 3.412142));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);

    // Set using HP

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, 0.00134102));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HOUR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PlantFlowUnitsTest)
    {
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB Meters / Second

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_M_PER_SEC, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testDouble;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.88, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);


    // Set using Liters per minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_MIN, 60000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);

    // Set using Cubic Feet/Minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_FT_PER_MIN, 2118.8800032893155));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);

    // Set using Gallons/Minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_GALLON_PER_MIN, 15850.323141488903));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);


    }




//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, FlowUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using CUB Meters / Second

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_M_PER_SEC, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testDouble;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FLOW_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.88, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);


    // Set using Liters per minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_LITRE_PER_MIN, 60000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FLOW_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);

    // Set using Cubic Feet/Minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_FT_PER_MIN, 2118.8800032893155));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FLOW_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);

    // Set using Gallons/Minute

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_GALLON_PER_MIN, 15850.323141488903));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FLOW_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_LITRE_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60000.0, 0.01);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_CUB_FT_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 2118.8800032893155, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FLOW_NAME, AEC_UNIT_GALLON_PER_MIN, testDouble);
    ASSERT_NEAR(testDouble, 15850.323141488903, 0.00000001);


}



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, VoltUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using Volt

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_VOLT, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ELECTRICAL_POTENIAL_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_VOLT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_KILOVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_MEGAVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.0000001);

    // Set using KILOVolt

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_KILOVOLT, 0.001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ELECTRICAL_POTENIAL_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_VOLT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_KILOVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_MEGAVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.0000001);

    // Set using MegaVolt

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_MEGAVOLT, 0.000001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ELECTRICAL_POTENIAL_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_VOLT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_KILOVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ELECTRICAL_POTENIAL_NAME, AEC_UNIT_MEGAVOLT, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.0000001);


}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, HeatTransfertUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using WATT_PER_SQUARE_METER_KELVIN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_KELVIN, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_HEAT_TRANSFER_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_CELIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 0.1761102, 0.0000001);

    // Set using WATT_PER_SQUARE_METER_CELIUS

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_CELIUS, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_HEAT_TRANSFER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_CELIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 0.1761102, 0.0000001);

    // Set using BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT, 0.1761102));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_HEAT_TRANSFER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQUARE_METER_CELIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 0.1761102, 0.0000001);

}

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LuminousFluxUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using WATT_PER_SQUARE_METER_KELVIN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LUMINOUS_FLUX_NAME, AEC_UNIT_LUMEN, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LUMINOUS_FLUX_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LUMINOUS_FLUX_NAME, AEC_UNIT_LUMEN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);


}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ILLUMINANCEUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using WATT_PER_SQUARE_METER_KELVIN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ILLUMINANCE_NAME, AEC_UNIT_LUX, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ILLUMINANCE_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ILLUMINANCE_NAME, AEC_UNIT_LUX, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ILLUMINANCE_NAME, AEC_UNIT_LUMEN_PER_SQUARE_FEET, testDouble);
    ASSERT_NEAR(testDouble, 0.09290304, 0.00001);

}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LUMINOUS_INTENSITYUnitsTest)
{
    DgnDbPtr db = OpenDgnDb();

    ASSERT_TRUE(db.IsValid());

    ECN::ECSchemaCP schema = db->Schemas().GetSchema(DYNAMIC_SCHEMA_NAME);

    ASSERT_TRUE(nullptr != schema);

    Dgn::SubjectCPtr parentSubject = db->Elements().GetRootSubject();

    Dgn::DgnCode               partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, MODEL_TEST_NAME);
    Dgn::DgnElementId          partitionId = db->Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db->Elements().Get<Dgn::PhysicalPartition>(partitionId);
    ASSERT_TRUE(partition.IsValid());

    PhysicalModelCPtr physModel = dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    ASSERT_TRUE(physModel.IsValid());

    Dgn::PhysicalElementPtr element = CreatePhysicalElement(DYNAMIC_SCHEMA_NAME, DYNAMIC_CLASS_NAME, *physModel);

    ASSERT_TRUE(element.IsValid());

    // Set using WATT_PER_SQUARE_METER_KELVIN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LUMINOUS_INTENSITY_NAME, AEC_UNIT_CANDELA, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LUMINOUS_INTENSITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LUMINOUS_INTENSITY_NAME, AEC_UNIT_CANDELA, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

}
