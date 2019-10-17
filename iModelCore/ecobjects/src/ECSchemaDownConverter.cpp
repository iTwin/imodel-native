/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

static Utf8CP const ECV3_CONVERSION_ATTRIBUTES      = "ECv3ConversionAttributes";
static Utf8CP const OLD_PERSISTENCE_UNIT            = "OldPersistenceUnit";

bool tryCreateCA(ECSchemaR schema, Utf8CP origClassName, Utf8CP className, IECInstancePtr& caInstance)
    {
    ECClassCP ecClass = schema.GetClassCP(className);
    if (nullptr == ecClass && !CustomAttributeDeserializerManager::CreateAttrClassVersion(&schema, origClassName, className, ecClass))
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
    Utf8CP perUnitName = ((ECUnitCP)ecProperty->GetKindOfQuantity()->GetPersistenceUnit())->GetFullName().c_str();
    Utf8CP oldPerUnitName = Units::UnitNameMappings::TryGetOldNameFromECName(perUnitName);
    if(nullptr == oldPerUnitName)
        {
        LOG.warningv("Failed to find old  unit name for the persistence unit '%s' used on property '%s.%s.%s'", 
                     perUnitName, schema.GetName().c_str(), ecProperty->GetClass().GetName().c_str(), ecProperty->GetName().c_str());
        return true;
        }
    
    IECInstancePtr unitSpecCA;
    if (!tryCreateCA(unitAttributesSchema, UNIT_SPECIFICATION_ORIG, UNIT_SPECIFICATION, unitSpecCA))
        return false;
    ECValue oldName(oldPerUnitName);
    unitSpecCA->SetValue(UNIT_NAME, oldName);
    ecProperty->SetCustomAttribute(*unitSpecCA);

    if (ecProperty->GetKindOfQuantity()->HasPresentationFormats())
        {
        ECUnitCP unit = static_cast<ECUnitCP>(ecProperty->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetCompositeMajorUnit());
        if (nullptr == unit)
            {
            LOG.warningv("Presentation format '%s' does not have a major unit used on property '%s.%s.%s'. Dropping", 
                ecProperty->GetKindOfQuantity()->GetDefaultPresentationFormat()->GetName().c_str(), schema.GetName().c_str(), 
                ecProperty->GetClass().GetName().c_str(), ecProperty->GetName().c_str());
            return true;
            }
        Utf8CP presUnitName = unit->GetFullName().c_str();
        Utf8CP oldPresUnitName = Units::UnitNameMappings::TryGetOldNameFromECName(presUnitName);
        if (nullptr == oldPresUnitName)
            {
            LOG.warningv("Failed to find old unit name for the presentation unit '%s' used on property '%s.%s.%s'",
                         presUnitName, schema.GetName().c_str(), ecProperty->GetClass().GetName().c_str(), ecProperty->GetName().c_str());
            return true;
            }
        
        IECInstancePtr displayUnitSpecCA;
        if (!tryCreateCA(unitAttributesSchema, DISPLAY_UNIT_SPECIFICATION_ORIG, DISPLAY_UNIT_SPECIFICATION, displayUnitSpecCA))
            return false;
        ECValue oldPresName(oldPresUnitName);
        displayUnitSpecCA->SetValue(DISPLAY_UNIT_NAME, oldPresName);
        ECValue formatString(DEFAULT_FORMATTER);
        displayUnitSpecCA->SetValue(DISPLAY_FORMAT_STRING, formatString);
        ecProperty->SetCustomAttribute(*displayUnitSpecCA);
        }
    return true;
    }

void removeOldPersistenceUnitCustomAttribute(ECSchemaR schema, ECPropertyP ecProperty)
    {
    ecProperty->RemoveCustomAttribute(ECV3_CONVERSION_ATTRIBUTES, OLD_PERSISTENCE_UNIT);
    ecProperty->RemoveSupplementedCustomAttribute(ECV3_CONVERSION_ATTRIBUTES, OLD_PERSISTENCE_UNIT);
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
        for(auto const& ecProperty : ecClass->GetProperties(false))
            {
            if (ecProperty->IsKindOfQuantityDefinedLocally())
                {
                if(!addUnitSpecificationsToProperty(schema, ecProperty, *s_unitAttributesSchema))
                    {
                    LOG.errorv("Failed to add UnitSpecifications to property '%s.%s.%s' stopping conversion.", 
                               schema.GetName().c_str(), ecClass->GetName().c_str(), ecProperty->GetName().c_str());
                    return false;
                    }
                removeOldPersistenceUnitCustomAttribute(schema, ecProperty);
                }
            }
        }

    static SchemaKey unitsKey = SchemaKey("Units", 1, 0, 0);
    auto unitsSchema = schema.FindSchema(unitsKey, SchemaMatchType::LatestReadCompatible);
    static SchemaKey formatsKey = SchemaKey("Formats", 1, 0, 0);
    auto formatsSchema = schema.FindSchema(formatsKey, SchemaMatchType::LatestReadCompatible);
    if(nullptr != unitsSchema && schema.IsSchemaReferenced(schema, *unitsSchema))
        {
        LOG.infov("Force removing reference to schema %s because units are not supported in EC2.  All units that could be were converted, see errors in log for failed conversions.", unitsSchema->GetFullSchemaName().c_str());
        schema.m_refSchemaList.erase(schema.m_refSchemaList.find(unitsSchema->GetSchemaKey()));
        }

    if(nullptr != formatsSchema && schema.IsSchemaReferenced(schema, *formatsSchema))
        {
        LOG.infov("Force removing reference to schema %s because format defintions are not supported in EC2. All formats that could be were converted into display units, see errors in log for failed conversions.", formatsSchema->GetFullSchemaName().c_str());
        schema.m_refSchemaList.erase(schema.m_refSchemaList.find(formatsSchema->GetSchemaKey()));
        }

    schema.RemoveUnusedSchemaReferences();
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
