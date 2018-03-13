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

static Utf8CP const ECV3_CONVERSION_ATTRIBUTES      = "ECv3ConversionAttributes";
static Utf8CP const OLD_PERSISTENCE_UNIT            = "OldPersistenceUnit";

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
    Utf8CP perUnitName = ((ECUnitCP)ecProperty->GetKindOfQuantity()->GetPersistenceUnit().GetUnit())->GetFullName().c_str();
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

    if (ecProperty->GetKindOfQuantity()->HasPresentationUnits())
        {
        Utf8CP presUnitName = ((ECUnitCP)ecProperty->GetKindOfQuantity()->GetDefaultPresentationUnit().GetUnit())->GetFullName().c_str();
        Utf8CP oldPresUnitName = Units::UnitNameMappings::TryGetOldNameFromECName(presUnitName);
        if (nullptr == oldPresUnitName)
            {
            LOG.warningv("Failed to find old  unit name for the presentation unit '%s' used on property '%s.%s.%s'",
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

    if(schema.IsSchemaReferenced(schema, *StandardUnitsHelper::GetSchema()))
        {
        for (auto ref : schema.GetReferencedSchemas())
            {
            if(3 == ref.second->GetOriginalECXmlVersionMajor() && 2 == ref.second->GetOriginalECXmlVersionMinor())
                {
                LOG.warningv("Force removing reference to schema %s even though it may be used by KoQs. They are not available in EC2", ref.first.GetFullSchemaName().c_str());
                schema.m_refSchemaList.erase(schema.m_refSchemaList.find(ref.first));
                }
            }
        }

    schema.RemoveUnusedSchemaReferences();
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
