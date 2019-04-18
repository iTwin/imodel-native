/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#define DYNAMIC_LUMINOUS_FLUX_NAME                          "LuminousFluxProp"
#define DYNAMIC_ILLUMINANCE_NAME                            "IlluminanceProp"
#define DYNAMIC_LUMINOUS_INTENSITY_NAME                     "LuminousIntensityProp"
#define DYNAMIC_THERMAL_RESISTANCE_NAME                     "ThermalResistanceProp"
#define DYNAMIC_VELOCITY_NAME                               "VelocityProp"
#define DYNAMIC_FREQUENCY_NAME                              "FrequencyProp"
#define DYNAMIC_TEMPERATURE_NAME                            "TempProp"
#define DYNAMIC_PRESSURE_NAME                               "PressureProp"
#define DYNAMIC_FLOW_NAME                                   "FlowProp"
#define DYNAMIC_ELECTRICAL_POTENIAL_NAME                    "VoltProp"
#define DYNAMIC_PRESSURE_GRADIENT_NAME                      "PressureGradientProp"
#define DYNAMIC_ENERGY_NAME                                 "EnergyProp"
#define DYNAMIC_DYNAMIC_VISCOSITY_NAME                      "DynamicViscosityProp"
#define DYNAMIC_TIME_NAME                                   "TimeProp"
#define DYNAMIC_ACCELERATION_NAME                           "AccelerationProp"
#define DYNAMIC_LINEAR_FORCE_NAME                           "LinearForceProp"
#define DYNAMIC_MOMENT_OF_INERTIA_NAME                      "MomentOfInertiaProp"
#define DYNAMIC_DENSITY_NAME                                "DensityProp"
#define DYNAMIC_THERMAL_EXPANSION_NAME                      "ThermalExpansionProp"
#define DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION               "SpecificHeatOfVaporizationProp"
#define DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME                 "SpecificHeatCapacityProp"
#define DYNAMIC_LINEAR_DENSITY_NAME                         "LinearDensityProp"
#define DYNAMIC_FORCE_DENSITY_NAME                          "ForceDensityProp"
#define DYNAMIC_LINEAR_ROTATIONAL_SPRING_CONSTANT_NAME      "RotationalSpringConstantProp"
#define DYNAMIC_WARPING_CONSTANT_NAME                       "WarpingConstantProp"
#define DYNAMIC_ANGULAR_VELOCITY_NAME                       "AngularVelocityProp"
#define DYNAMIC_THERMAL_CONDUCTIVITY_NAME                   "ThermalConductivityProp"



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

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_THERMAL_RESISTANCE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_THERMAL_RESISTANCE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LUMINOUS_INTENSITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LUMINOUS_INTENSITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_VELOCITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_VELOCITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_FREQUENCY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_FREQUENCY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_PRESSURE_GRADIENT_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_PRESSURE_GRADIENT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ENERGY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ENERGY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_DYNAMIC_VISCOSITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_DYNAMIC_VISCOSITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_TIME_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_TIME);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ACCELERATION_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ACCELERATION);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LINEAR_FORCE_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LINEAR_FORCE);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_MOMENT_OF_INERTIA_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_MOMENT_OF_INERTIA);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_DENSITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_DENSITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_THERMAL_EXPANSION_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_THERMAL_EXPANSION_COEFFICIENT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_SPECIFIC_HEAT_OF_VAPORIZATION);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_SPECIFIC_HEAT_CAPACITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LINEAR_DENSITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LINEAR_DENSITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_FORCE_DENSITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_FORCE_DENSITY);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_LINEAR_ROTATIONAL_SPRING_CONSTANT_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_LINEAR_ROTATIONAL_SPRING_CONSTANT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_WARPING_CONSTANT_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_WARPING_CONSTANT);
    myProp->SetKindOfQuantity(locatedKOQ);

    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_ANGULAR_VELOCITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_ANGULAR_VELOCITY);
    myProp->SetKindOfQuantity(locatedKOQ);
    
    newClass->CreatePrimitiveProperty(myProp, DYNAMIC_THERMAL_CONDUCTIVITY_NAME);
    myProp->SetType(ECN::PRIMITIVETYPE_Double);
    locatedKOQ = unitsSchema->GetKindOfQuantityCP(AEC_KOQ_THERMAL_CONDUCTIVITY);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.101971621, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.101971621, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.101971621, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.101971621, 0.00001);

    // Set using KGF_PER_SQ_M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, 0.101971621));

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_PRESSURE_PROPERTY_NAME, AEC_UNIT_KGF_PER_SQ_M, testDouble);
    ASSERT_NEAR(testDouble, 0.101971621, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, testDouble);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, testDouble);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, testDouble);
    ASSERT_NEAR(testDouble, 3.412142, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_KW, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_MEGAW, testDouble);
    ASSERT_NEAR(testDouble, 0.000001, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_HP, testDouble);
    ASSERT_NEAR(testDouble, 0.00134102, 0.00000001);

    // Set using BTU/HR

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, 3.412142));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_POWER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_W, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, testDouble);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_POWER_NAME, AEC_UNIT_BTU_PER_HR, testDouble);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);


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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    // Set using Litre/Sec

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, 1000.0));

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PROCESS_PIPING_FLOW_PROPERTY_NAME, AEC_UNIT_LITRE_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

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

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_KELVIN, 1.0));

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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 0.1761102, 0.0000001);

    // Set using WATT_PER_SQUARE_METER_CELIUS

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_CELSIUS, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_HEAT_TRANSFER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, 0.1761102, 0.0000001);

    // Set using BTU_PER_SQUARE_FEET_HOUR_FAHRENHEIT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT, 0.1761102));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_HEAT_TRANSFER_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_WATT_PER_SQ_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_HEAT_TRANSFER_NAME, AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
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

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ILLUMINANCE_NAME, AEC_UNIT_LUMEN_PER_SQ_FT, testDouble);
    ASSERT_NEAR(testDouble, 0.09290304, 0.00001);

}


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LuminousIntensityUnitsTest)
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


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ThermalResistancetUnitsTest)
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

    // Set using SQUARE_METER_KELVIN_PER_WATT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_KELVIN_PER_WATT, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_RESISTANCE_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_KELVIN_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_CELSIUS_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU, testDouble);
    ASSERT_NEAR(testDouble, 5.6782633411134, 0.0000001);

    // Set using SQUARE_METER_CELIUS_PER_WATT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_CELSIUS_PER_WATT, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_RESISTANCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_KELVIN_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_CELSIUS_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU, testDouble);
    ASSERT_NEAR(testDouble, 5.6782633411134, 0.0000001);

    // Set using SQUARE_FEET_HOUR_FAHRENHEIT_PER_BTU

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU, 5.6782633411134));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_RESISTANCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_KELVIN_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_M_CELSIUS_PER_WATT, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_RESISTANCE_NAME, AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU, testDouble);
    ASSERT_NEAR(testDouble, 5.6782633411134, 0.0000001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, VelocityUnitsTest)
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

    // Set using Meter per sec

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_VELOCITY_NAME, AEC_UNIT_M_PER_SEC, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VELOCITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3.28084, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_MPH, testDouble);
    ASSERT_NEAR(testDouble, 2.23693629, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_KMPH, testDouble);
    ASSERT_NEAR(testDouble, 3.6, 0.0000001);

    // Set using ft per sec

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_FT_PER_SEC, 3.28084));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VELOCITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3.28084, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_MPH, testDouble);
    ASSERT_NEAR(testDouble, 2.23693629, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_KMPH, testDouble);
    ASSERT_NEAR(testDouble, 3.6, 0.0001);

    // Set using MPH

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_MPH, 2.23693629));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VELOCITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3.28084, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_MPH, testDouble);
    ASSERT_NEAR(testDouble, 2.23693629, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_KMPH, testDouble);
    ASSERT_NEAR(testDouble, 3.6, 0.0001);


    // Set using KMPH

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_KMPH, 3.6));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_VELOCITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_M_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3.28084, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_MPH, testDouble);
    ASSERT_NEAR(testDouble, 2.23693629, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_VELOCITY_NAME, AEC_UNIT_KMPH, testDouble);
    ASSERT_NEAR(testDouble, 3.6, 0.0001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, PressureGradientTest)
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

    // Set using HZ

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_PA_PER_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_GRADIENT_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_PA_PER_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_BAR_PER_KM, testDouble);
    ASSERT_NEAR(testDouble, 0.01, 0.00001);


    // Set using KHZ

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_BAR_PER_KM, 0.01));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_PRESSURE_GRADIENT_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_PA_PER_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_PRESSURE_GRADIENT_NAME, AEC_UNIT_BAR_PER_KM, testDouble);
    ASSERT_NEAR(testDouble, 0.01, 0.00001);

    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, frequencyUnitsTest)
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

    // Set using HZ

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_HZ, 1000.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FREQUENCY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_HZ, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_KHZ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_MHZ, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);

    // Set using KHZ

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_KHZ, 1.0));

    testElement = queriedElement->Update(&status);


    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FREQUENCY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_HZ, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_KHZ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_MHZ, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);
    
    // Set using MHZ
    
    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_MHZ, 0.001));
    
    testElement = queriedElement->Update(&status);
    
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);
    
    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());
    
    ASSERT_TRUE(queriedElement.IsValid());
    
    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FREQUENCY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_HZ, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_KHZ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);
    
    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FREQUENCY_NAME, AEC_UNIT_MHZ, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.00001);
    
    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, energyUnitsTest)
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

    // Set using Joules

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ENERGY_NAME, AEC_UNIT_JOULES, 1000.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ENERGY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_JOULES, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KILOJOULES, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_BTU, testDouble);
    ASSERT_NEAR(testDouble, 0.947817, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KW_HR, testDouble);
    ASSERT_NEAR(testDouble, 0.000277778, 0.00001);

    // Set using KJ

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KILOJOULES, 1.0));

    testElement = queriedElement->Update(&status);


    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ENERGY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_JOULES, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KILOJOULES, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_BTU, testDouble);
    ASSERT_NEAR(testDouble, 0.947817, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KW_HR, testDouble);
    ASSERT_NEAR(testDouble, 0.000277778, 0.00001);

    // Set using BTU

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_BTU, 0.947817));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ENERGY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_JOULES, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KILOJOULES, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_BTU, testDouble);
    ASSERT_NEAR(testDouble, 0.947817, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KW_HR, testDouble);
    ASSERT_NEAR(testDouble, 0.000277778, 0.00001);

    // Set using KWH

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KW_HR, 0.000277778));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ENERGY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_JOULES, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KILOJOULES, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_BTU, testDouble);
    ASSERT_NEAR(testDouble, 0.947817, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ENERGY_NAME, AEC_UNIT_KW_HR, testDouble);
    ASSERT_NEAR(testDouble, 0.000277778, 0.00001);

    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, DynamicViscosityUnitsTest)
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

    // Set using Pascals Seconds

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_PA_SEC, 1000.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DYNAMIC_VISCOSITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_PA_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_POISE, testDouble);
    ASSERT_NEAR(testDouble, 10000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_CENTIPOISE, testDouble);
    ASSERT_NEAR(testDouble, 1000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_LBM_PER_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 671.968994813, 0.0001);

    // Set using POSIE

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_POISE, 10000.0));

    testElement = queriedElement->Update(&status);


    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DYNAMIC_VISCOSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_PA_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_POISE, testDouble);
    ASSERT_NEAR(testDouble, 10000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_CENTIPOISE, testDouble);
    ASSERT_NEAR(testDouble, 1000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_LBM_PER_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 671.968994813, 0.0001);

    // Set using CENTIPOSIE

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_CENTIPOISE, 1000000.0));

    testElement = queriedElement->Update(&status);


    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DYNAMIC_VISCOSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_PA_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_POISE, testDouble);
    ASSERT_NEAR(testDouble, 10000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_CENTIPOISE, testDouble);
    ASSERT_NEAR(testDouble, 1000000.0, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_LBM_PER_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 671.968994813, 0.0001);

    // Set using POUNDMASS_PER_FEET_PER_SECOND

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_LBM_PER_FT_PER_SEC, 671.968994813));

    testElement = queriedElement->Update(&status);


    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DYNAMIC_VISCOSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_PA_SEC, testDouble);
    ASSERT_NEAR(testDouble, 1000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_POISE, testDouble);
    ASSERT_NEAR(testDouble, 10000.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_CENTIPOISE, testDouble);
    ASSERT_NEAR(testDouble, 1000000.0, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DYNAMIC_VISCOSITY_NAME, AEC_UNIT_LBM_PER_FT_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, 671.968994813, 0.0001);

    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, TimeUnitsTest)
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

    // Set using Seconds

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, 3600.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TIME_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, testDouble);
    ASSERT_NEAR(testDouble, 1.0/24.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, testDouble);
    ASSERT_NEAR(testDouble, 3600000.0, 0.001);

    // Set using Mins

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, 60.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TIME_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, testDouble);
    ASSERT_NEAR(testDouble, 1.0 / 24.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, testDouble);
    ASSERT_NEAR(testDouble, 3600000.0, 0.001);

    // Set using Hrs

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, 1.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TIME_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, testDouble);
    ASSERT_NEAR(testDouble, 1.0 / 24.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, testDouble);
    ASSERT_NEAR(testDouble, 3600000.0, 0.001);


    // Set using Days

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, 1.0/24.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TIME_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, testDouble);
    ASSERT_NEAR(testDouble, 1.0 / 24.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, testDouble);
    ASSERT_NEAR(testDouble, 3600000.0, 0.001);

    // Set using Milli-Seconds

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, 3600000.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_TIME_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_SEC, testDouble);
    ASSERT_NEAR(testDouble, 3600.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MIN, testDouble);
    ASSERT_NEAR(testDouble, 60.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_HR, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_DAY, testDouble);
    ASSERT_NEAR(testDouble, 1.0 / 24.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_TIME_NAME, AEC_UNIT_MILLISEC, testDouble);
    ASSERT_NEAR(testDouble, 3600000.0, 0.001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, AccelerationUnitsTest)
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

    // Set using M/S^2

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_M_PER_SEC_SQ, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ACCELERATION_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_M_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_CM_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_FT_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 3.2808399, 0.001);

    // Set using CM/S^2

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_CM_PER_SEC_SQ, 100.0));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ACCELERATION_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_M_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_CM_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_FT_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 3.2808399, 0.001);

    // Set using FT/SEC^2

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_FT_PER_SEC_SQ, 3.2808399));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ACCELERATION_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_M_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_CM_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 100.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ACCELERATION_NAME, AEC_UNIT_FT_PER_SEC_SQ, testDouble);
    ASSERT_NEAR(testDouble, 3.2808399, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LinearForceUnitsTest)
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

    // Set using N/M

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_M, 1.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_FORCE_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_LBF_PER_IN, testDouble);
    ASSERT_NEAR(testDouble, 0.224809 / 1000.0 * 25.4, 0.000001);

    // Set using N/MM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_MM, 0.001));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_LBF_PER_IN, testDouble);
    ASSERT_NEAR(testDouble, 0.224809 / 1000.0 * 25.4, 0.000001);

    // Set using LBF / IN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_LBF_PER_IN, 0.224809 / 1000.0 * 25.4));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_FORCE_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_M, testDouble);
    ASSERT_NEAR(testDouble, 1.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_N_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, 0.001, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_FORCE_NAME, AEC_UNIT_LBF_PER_IN, testDouble);
    ASSERT_NEAR(testDouble, 0.224809 / 1000.0 * 25.4, 0.000001);


    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, MomentOfInertiaUnitsTest)
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

#define M4 0.0000001
#define MM4 M4  * 1000.0 * 1000.0 * 1000.0 * 1000.0
#define CM4 M4  * 100.0  * 100.0  * 100.0  * 100.0
#define IN4 MM4 /  25.4  /  25.4  /  25.4  /  25.4
#define FT4 IN4 /  12.0  /  12.0  /  12.0  /  12.0

    // Set using M^4

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, M4));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_MOMENT_OF_INERTIA_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, MM4, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, CM4, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, IN4, 0.000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, FT4, 0.00000001);

    // Set using MM^4

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, MM4));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_MOMENT_OF_INERTIA_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, MM4, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, CM4, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, IN4, 0.000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, FT4, 0.00000001);

    // Set using CM^4

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, CM4));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_MOMENT_OF_INERTIA_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, MM4, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, CM4, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, IN4, 0.000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, FT4, 0.00000001);


    // Set using IN^4

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, IN4));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_MOMENT_OF_INERTIA_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, MM4, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, CM4, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, IN4, 0.000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, FT4, 0.00000001);


    // Set using FT^4

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, FT4));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_MOMENT_OF_INERTIA_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_M_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, M4, 0.0000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_MM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, MM4, 0.1);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_CM_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, CM4, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_IN_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, IN4, 0.000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_MOMENT_OF_INERTIA_NAME, AEC_UNIT_FT_FOURTH, testDouble);
    ASSERT_NEAR(testDouble, FT4, 0.00000001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, DensityUnitsTest)
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

#define KG_PER_CUB_M    1.0
#define G_PER_CUB_CM    KG_PER_CUB_M  * 1000.0 / 100.0 / 100.0 / 100.0
#define LBM_PER_CUB_FT  KG_PER_CUB_M  * 2.20462  / 3.28084  / 3.28084  / 3.28084
#define LBM_PER_CUB_IN  LBM_PER_CUB_FT /  12.0  /  12.0  /  12.0
#define KIP_PER_CUB_FT  LBM_PER_CUB_FT /  1000.0  

    // Set using KG/M^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, KG_PER_CUB_M));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DENSITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, testDouble);
    ASSERT_NEAR(testDouble, G_PER_CUB_CM, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_FT, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_IN, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KIP_PER_CUB_FT, 0.0000000001);

    // Set using G/CM^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, G_PER_CUB_CM));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, testDouble);
    ASSERT_NEAR(testDouble, G_PER_CUB_CM, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_FT, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_IN, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KIP_PER_CUB_FT, 0.0000000001);

    // Set using LBM/FT^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, LBM_PER_CUB_FT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, testDouble);
    ASSERT_NEAR(testDouble, G_PER_CUB_CM, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_FT, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_IN, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KIP_PER_CUB_FT, 0.0000000001);

    // Set using LBM/IN^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, LBM_PER_CUB_IN));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, testDouble);
    ASSERT_NEAR(testDouble, G_PER_CUB_CM, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_FT, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_IN, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KIP_PER_CUB_FT, 0.0000000001);


    // Set using Kip/ft^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, KIP_PER_CUB_FT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KG_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_G_PER_CUB_CM, testDouble);
    ASSERT_NEAR(testDouble, G_PER_CUB_CM, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_FT, 0.0001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_LBM_PER_CUB_IN, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_CUB_IN, 0.00000001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_DENSITY_NAME, AEC_UNIT_KIP_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KIP_PER_CUB_FT, 0.0000000001);

    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ThermalExpansionUnitsTest)
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

#define KELVIN          200.0
#define CELSIUS         KELVIN
#define FAHRENHEIT      (CELSIUS / (9.0 / 5.0)) 

    // Set using Strain/Kelvin

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_KELVIN, KELVIN));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_EXPANSION_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, FAHRENHEIT, 0.001);

    // Set using Strain/Celsius

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_CELSIUS, CELSIUS));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_EXPANSION_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, FAHRENHEIT, 0.001);

    // Set using Strain/Fahrenheit

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_FAHRENHEIT, FAHRENHEIT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_EXPANSION_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_EXPANSION_NAME, AEC_UNIT_STRAIN_PER_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, FAHRENHEIT, 0.001);


    }


    


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, SpecificHeatOfVaporizationUnitsTest)
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

#define J_PER_KG        1000.0
#define KJ_PER_KG       J_PER_KG / 1000
#define BTU_PER_LBM     J_PER_KG * 0.0004299226139295 

    // Set using Strain/Kelvin

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_J_PER_KG, J_PER_KG));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_J_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_KJ_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, KJ_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_BTU_PER_LBM, testDouble);
    ASSERT_NEAR(testDouble, BTU_PER_LBM, 0.001);

    // Set using Strain/Celsius

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_KJ_PER_KG, KJ_PER_KG));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_J_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_KJ_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, KJ_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_BTU_PER_LBM, testDouble);
    ASSERT_NEAR(testDouble, BTU_PER_LBM, 0.001);

    // Set using Strain/Fahrenheit

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_BTU_PER_LBM, BTU_PER_LBM));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_J_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, J_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_KJ_PER_KG, testDouble);
    ASSERT_NEAR(testDouble, KJ_PER_KG, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_OF_VAPORIZATION, AEC_UNIT_BTU_PER_LBM, testDouble);
    ASSERT_NEAR(testDouble, BTU_PER_LBM, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, SpecificHeatCapacityUnitsTest)
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

    // Set using Strain/Kelvin

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_J_PER_KG_KELVIN, 10.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 10.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_J_PER_KG_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 10.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_BTU_PER_LBM_RANKINE, testDouble);
    ASSERT_NEAR(testDouble, 0.00238845896627, 0.000001);

    // Set using Strain/Celsius

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_BTU_PER_LBM_RANKINE, 0.00238845896627));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 10.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_J_PER_KG_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, 10.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_SPECIFIC_HEAT_CAPACITY_NAME, AEC_UNIT_BTU_PER_LBM_RANKINE, testDouble);
    ASSERT_NEAR(testDouble, 0.00238845896627, 0.000001);

    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, LinearDensityUnitsTest)
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

#define KG_PER_M        1000.0
#define KG_PER_MM       KG_PER_M / 1000.0
#define LBM_PER_FT      KG_PER_MM * 2.204622621849 * 25.4 * 12.0 

    // Set using Strain/Kelvin

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_M, KG_PER_M));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_DENSITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_MM, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_LBM_PER_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_FT, 0.001);

    // Set using Strain/Celsius

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_MM, KG_PER_MM));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_MM, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_LBM_PER_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_FT, 0.001);

    // Set using Strain/Fahrenheit

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_LBM_PER_FT, LBM_PER_FT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_M, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_KG_PER_MM, testDouble);
    ASSERT_NEAR(testDouble, KG_PER_MM, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_DENSITY_NAME, AEC_UNIT_LBM_PER_FT, testDouble);
    ASSERT_NEAR(testDouble, LBM_PER_FT, 0.001);


    }




//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ForceDensityUnitsTest)
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

#define N_PER_CUB_M    1000.0
#define KN_PER_CUB_M   N_PER_CUB_M / 1000.0
#define N_PER_CUB_FT   N_PER_CUB_M / (1000.0 / (25.4 * 12.0)) / (1000.0 / (25.4 * 12.0)) / (1000.0 / (25.4 * 12.0))
#define KN_PER_CUB_FT  N_PER_CUB_FT / 1000.0

    // Set using N/M^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_M, N_PER_CUB_M));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_DENSITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_FT, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_FT, 0.000001);

    // Set using KN/M^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_M, KN_PER_CUB_M));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_FT, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_FT, 0.000001);

    // Set using N/FT^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_FT, N_PER_CUB_FT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_FT, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_FT, 0.000001);

    // Set using KN/FT^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_FT, KN_PER_CUB_FT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_FORCE_DENSITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_M, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_M, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_N_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, N_PER_CUB_FT, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_FORCE_DENSITY_NAME, AEC_UNIT_KN_PER_CUB_FT, testDouble);
    ASSERT_NEAR(testDouble, KN_PER_CUB_FT, 0.000001);

    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, RotationalSpringConstantUnitsTest)
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

    // Set using N/M^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_LINEAR_ROTATIONAL_SPRING_CONSTANT_NAME, AEC_UNIT_N_PER_RAD, 10.0));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_LINEAR_ROTATIONAL_SPRING_CONSTANT_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, 10.0, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_LINEAR_ROTATIONAL_SPRING_CONSTANT_NAME, AEC_UNIT_N_PER_RAD, testDouble);
    ASSERT_NEAR(testDouble, 10.0, 0.001);


    }



//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, WarpingConstantUnitsTest)
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

#define M_SIXTH    0.001
#define FT_SIXTH   M_SIXTH * (1000.0 / 25.4 /12.0) * (1000.0 / 25.4 /12.0) * (1000.0 / 25.4 /12.0) * (1000.0 / 25.4 /12.0) * (1000.0 / 25.4 /12.0) * (1000.0 / 25.4 /12.0)

    // Set using M^6

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_M_SIXTH, M_SIXTH));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_WARPING_CONSTANT_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M_SIXTH, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_M_SIXTH, testDouble);
    ASSERT_NEAR(testDouble, M_SIXTH, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_FT_SIXTH, testDouble);
    ASSERT_NEAR(testDouble, FT_SIXTH, 0.001);

    // Set using KN/M^3

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_FT_SIXTH, FT_SIXTH));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_WARPING_CONSTANT_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, M_SIXTH, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_M_SIXTH, testDouble);
    ASSERT_NEAR(testDouble, M_SIXTH, 0.00001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_WARPING_CONSTANT_NAME, AEC_UNIT_FT_SIXTH, testDouble);
    ASSERT_NEAR(testDouble, FT_SIXTH, 0.001);

    }

//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, AngularVelocityUnitsTest)
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

#define RAD_PER_SEC    10.0
#define DEG_PER_SEC    RAD_PER_SEC * 180.0 / PI
#define RPM            DEG_PER_SEC / 360.0 * 60

    // Set using RAD_PER_SEC

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RAD_PER_SEC, RAD_PER_SEC));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ANGULAR_VELOCITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RAD_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_DEG_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, DEG_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RPM, testDouble);
    ASSERT_NEAR(testDouble, RPM, 0.001);

    // Set using DEG_PER_SEC

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_DEG_PER_SEC, DEG_PER_SEC));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ANGULAR_VELOCITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RAD_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_DEG_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, DEG_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RPM, testDouble);
    ASSERT_NEAR(testDouble, RPM, 0.001);

    // Set using RPM

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RPM, RPM));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_ANGULAR_VELOCITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RAD_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, RAD_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_DEG_PER_SEC, testDouble);
    ASSERT_NEAR(testDouble, DEG_PER_SEC, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_ANGULAR_VELOCITY_NAME, AEC_UNIT_RPM, testDouble);
    ASSERT_NEAR(testDouble, RPM, 0.001);


    }


//---------------------------------------------------------------------------------------
//  @bsimethod                                                    06/2017
//+---------------+---------------+---------------+---------------+---------------+-------

TEST_F(AecUnitsTestFixture, ThermalConductivityUnitsTest)
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

#define W_PER_M_KELVIN                    10.0
#define W_PER_M_CELSIUS                   W_PER_M_KELVIN
#define BTU_IN_PER_SQ_FT_HR_FAHRENHEIT    69.334717985159784

    // Set using W_PER_M_KELVIN

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*element, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_KELVIN, W_PER_M_KELVIN));

    Dgn::DgnDbStatus status;

    Dgn::DgnElementCPtr testElement = element->Insert(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    Dgn::PhysicalElementPtr queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ECN::ECValue propVal;
    double testValue;


    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_CONDUCTIVITY_NAME));
    double testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, 0.001);

    // Set using W_PER_M_CELSIUS

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_CELSIUS, W_PER_M_CELSIUS));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_CONDUCTIVITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, 0.001);

    // Set using BTU_IN_PER_SQ_FT_HR_FAHRENHEIT

    ASSERT_TRUE(BentleyStatus::SUCCESS == AecUnits::AecUnitsUtilities::SetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, BTU_IN_PER_SQ_FT_HR_FAHRENHEIT));

    testElement = queriedElement->Update(&status);

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == status);

    queriedElement = QueryById<Dgn::PhysicalElement>(*physModel, testElement->GetElementId());

    ASSERT_TRUE(queriedElement.IsValid());

    ASSERT_TRUE(Dgn::DgnDbStatus::Success == queriedElement->GetPropertyValue(propVal, DYNAMIC_THERMAL_CONDUCTIVITY_NAME));
    testDouble = propVal.GetDouble();
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_KELVIN, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_KELVIN, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_W_PER_M_CELSIUS, testDouble);
    ASSERT_NEAR(testDouble, W_PER_M_CELSIUS, 0.001);

    AecUnits::AecUnitsUtilities::GetDoublePropertyUsingUnitString(*queriedElement, DYNAMIC_THERMAL_CONDUCTIVITY_NAME, AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, testDouble);
    ASSERT_NEAR(testDouble, BTU_IN_PER_SQ_FT_HR_FAHRENHEIT, 0.001);


    }
