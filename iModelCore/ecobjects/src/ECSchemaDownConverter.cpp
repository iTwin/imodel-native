/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaDownConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static Utf8CP const UNIT_ATTRIBUTES                 = "Unit_Attributes";
static Utf8CP const UNIT_SPECIFICATION_ORIG         = "UnitSpecification";
static Utf8CP const UNIT_SPECIFICATION              = "UnitSpecificationAttr";
static Utf8CP const UNIT_NAME                       = "UnitName";
static Utf8CP const DISPLAY_UNIT_SPECIFICATION_ORIG = "DisplayUnitSpecification";
static Utf8CP const DISPLAY_UNIT_SPECIFICATION      = "DisplayUnitSpecificationAttr";
static Utf8CP const DISPLAY_UNIT_NAME               = "DisplayUnitName";
static Utf8CP const DISPLAY_FORMAT_STRING           = "DisplayFormatString";
static Utf8CP const DEFAULT_FORMATTER               = "0.######";

bool tryCreateCA(ECSchemaR schema, Utf8CP origClassName, Utf8CP className, IECInstancePtr& caInstance)
    {
    ECClassCP ecClass = nullptr;
    if (!CustomAttributeDeserializerManager::CreateAttrClassVersion(&schema, origClassName, className, ecClass))
        {
        LOG.errorv("Failed to find '%s' class in '%s' schema", className, schema.GetName().c_str());
        return false;
        }
    caInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    return true;
    }

bool addUnitSpecificationsToProperty(ECSchemaR schema, ECPropertyP ecProperty, ECSchemaR unitAttributesSchema)
    {
    schema.AddReferencedSchema(unitAttributesSchema);
    Utf8CP perUnitName = ecProperty->GetKindOfQuantity()->GetPersistenceUnit().GetUnit()->GetName();
    Utf8String oldPerUnitName;
    if(!Units::UnitRegistry::Instance().TryGetOldName(perUnitName, oldPerUnitName))
        {
        LOG.warningv("Failed to find old  unit name for the persistence unit '%s' used on property '%s.%s.%s'", 
                     perUnitName, schema.GetName().c_str(), ecProperty->GetClass().GetName().c_str(), ecProperty->GetName().c_str());
        return true;
        }
    
    IECInstancePtr unitSpecCA;
    if (!tryCreateCA(unitAttributesSchema, UNIT_SPECIFICATION_ORIG, UNIT_SPECIFICATION, unitSpecCA))
        return false;
    ECValue oldName(oldPerUnitName.c_str());
    unitSpecCA->SetValue(UNIT_NAME, oldName);
    ecProperty->SetCustomAttribute(*unitSpecCA);

    if (ecProperty->GetKindOfQuantity()->HasPresentationUnits())
        {
        Utf8CP presUnitName = ecProperty->GetKindOfQuantity()->GetDefaultPresentationUnit().GetUnit()->GetName();
        Utf8String oldPresUnitName;
        if (!Units::UnitRegistry::Instance().TryGetOldName(presUnitName, oldPresUnitName))
            {
            LOG.warningv("Failed to find old  unit name for the presentation unit '%s' used on property '%s.%s.%s'",
                         presUnitName, schema.GetName().c_str(), ecProperty->GetClass().GetName().c_str(), ecProperty->GetName().c_str());
            return true;
            }
        
        IECInstancePtr displayUnitSpecCA;
        if (!tryCreateCA(unitAttributesSchema, DISPLAY_UNIT_SPECIFICATION_ORIG, DISPLAY_UNIT_SPECIFICATION, displayUnitSpecCA))
            return false;
        ECValue oldPresName(oldPresUnitName.c_str());
        displayUnitSpecCA->SetValue(DISPLAY_UNIT_NAME, oldPresName);
        ECValue formatString(DEFAULT_FORMATTER);
        displayUnitSpecCA->SetValue(DISPLAY_FORMAT_STRING, formatString);
        ecProperty->SetCustomAttribute(*displayUnitSpecCA);
        }
    return true;
    }

ECSchemaPtr getSchema()
    {
    auto context = ECSchemaReadContext::CreateContext();
    SchemaKey schemaKey(UNIT_ATTRIBUTES, 1, 0);
    return context->LocateSchema(schemaKey, SchemaMatchType::Latest);
    }

bool ECSchemaDownConverter::Convert(ECSchemaR schema)
    {
    static ECSchemaPtr s_unitAttributesSchema = getSchema();

    if (!s_unitAttributesSchema.IsValid())
        {
        LOG.error("Could not find the Unit_Attributes schema");
        return false;
        }

    for(auto const& ecClass : schema.GetClasses())
        {
        for(auto const& ecProperty : ecClass->GetProperties())
            {
            if (ecProperty->IsKindOfQuantityDefinedLocally())
                addUnitSpecificationsToProperty(schema, ecProperty, *s_unitAttributesSchema);
            }
        }
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
