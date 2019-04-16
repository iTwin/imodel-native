/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_Units_Include.sampleCode
#include <ECObjects/ECObjectsAPI.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 07/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateUnit()
	{
	//__PUBLISH_EXTRACT_START__ Overview_Units_CreateUnit.sampleCode
    ECSchemaPtr schema;
    ECUnitP unit;
    UnitSystemP system;
    PhenomenonP phenom;
    ECSchema::CreateSchema(schema, "UnitsSchema", "schema", 5, 0, 6);
    schema->CreateUnitSystem(system, "TestUnitSystem", "SystemLabel", "SystemDescription");
    schema->CreatePhenomenon(phenom, "TestPhenom", "TestDefinition", "PhenomLabel", "PhenomDescription");
    double numerator = 10.0;
    double denominator = 1.0;
    double offset = 0.0;
    schema->CreateUnit(unit, "TestUnit", "TestDefinition", *phenom, *system, numerator, denominator, offset, "UnitLabel", "UnitDescription");
    
    //__PUBLISH_EXTRACT_END__
    auto parent = unit;
    //__PUBLISH_EXTRACT_START__ Overview_Units_CreateInvertedUnitAndConstant.sampleCode
    ECUnitP invUnit;
    schema->CreateInvertedUnit(invUnit, *parent, "InvertedUnit", *system, "InvertedUnitLabel", "InvertedUnitDescription");
    ECUnitP constant;
    schema->CreateConstant(constant, "Constant", "M", *phenom, numerator, denominator, "ConstantLabel", "ConstantDescription");
    
    //__PUBLISH_EXTRACT_END__
    return BentleyStatus::SUCCESS;
	}