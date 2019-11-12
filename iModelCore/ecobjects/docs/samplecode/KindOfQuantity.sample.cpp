/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_KindOfQuantity_Include.sampleCode
#include <ECObjects/ECObjectsAPI.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 07/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateKindOfQuantity()
    {
    //__PUBLISH_EXTRACT_START__ Overview_KindOfQuantity_CreateKindOfQuantity.sampleCode
    ECSchemaPtr schema;
    KindOfQuantityP koq;
    ECSchema::CreateSchema(schema, "KindOfQuantitySchema", "koq", 5, 0, 6);
    schema->CreateKindOfQuantity(koq, "MyKindOfQuantity");
    //__PUBLISH_EXTRACT_END__

    //__PUBLISH_EXTRACT_START__ Overview_KindOfQuantity_AddPersistenceUnit.sampleCode
    ECUnitP unit;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");
    koq->SetPersistenceUnit(*unit);
    //__PUBLISH_EXTRACT_END__
    //__PUBLISH_EXTRACT_START__ Overview_KindOfQuantity_AddToProperty.sampleCode
    ECEntityClassP entityClass;
    PrimitiveECPropertyP prop;
    schema->CreateEntityClass(entityClass, "Class");
    entityClass->CreatePrimitiveProperty(prop, "Property");  
    prop->SetKindOfQuantity(koq);
    //__PUBLISH_EXTRACT_END__

    ECFormatP format = nullptr;
    ECUnitP unit2 = nullptr;
    //__PUBLISH_EXTRACT_START__ Overview_KindOfQuantity_AddPresentationUnits.sampleCode

    // Given an ECFormat called format, a presentation format (NamedFormat) can be added to a KoQ with or without overriding
    // precision, composite and labels
    koq->AddPresentationFormat(*format); // No overrides of any kind

    // This is a convenience method to facilitate the common case of adding a single override unit
    koq->AddPresentationFormatSingleUnitOverride(*format, nullptr, unit); // Override unit no precision override
    koq->AddPresentationFormatSingleUnitOverride(*format, 7, unit); // Override unit and precision
    koq->AddPresentationFormatSingleUnitOverride(*format, 7, unit, "This is a new label"); // Override precision, label, and unit
    koq->AddPresentationFormatSingleUnitOverride(*format, 7, nullptr, "This is a new label"); // This is valid if format has a composite unit. It will override the label for the largest composite unit

    // This method also allows overriding of units and labels but requires more setup than the above methods
    KindOfQuantity::UnitAndLabelPairs pairs = KindOfQuantity::UnitAndLabelPairs();
    pairs.push_back(make_bpair(unit, "This is a new label"));
    pairs.push_back(make_bpair(unit2, "This is a new label 2"));
    koq->AddPresentationFormat(*format, nullptr, &pairs); // Override the unit and labels for multiple units;
    koq->AddPresentationFormat(*format, 7, &pairs); // Override the unit and labels for multiple units and override precision;
    koq->AddPresentationFormat(*format, 7, &pairs, true); // Same as above except add this as the default presentation format (meaning it's the first in the list)
    //__PUBLISH_EXTRACT_END__

    

    return SUCCESS;
    }