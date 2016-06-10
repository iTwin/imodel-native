/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaLocalizedStrings.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static Utf8CP const SCHEMANAME         = "SchemaLocalizationCustomAttributes";
static Utf8CP const LOC_SPEC           = "LocalizationSpecification";
static Utf8CP const LOCALE             = "Locale";
static Utf8CP const RESOURCE           = "Resource";
static Utf8CP const KEY                = "Key";
static Utf8CP const VALUE              = "Value";

static Utf8CP const SOURCE             = "Source";
static Utf8CP const COLON              = ":";
static Utf8CP const DOT                = ".";
static Utf8CP const AT                 = "@";

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetLocalizedString(Utf8CP labelKey, Utf8StringCR invariantString) const
    {
    auto it = m_localizedStrings.find(labelKey);
    if (it != m_localizedStrings.end())
        return it->second;
    return invariantString;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetSchemaDisplayLabel(ECSchemaCP ecSchema, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty || ecSchema == nullptr)
        return invariantDisplayLabel;

    Utf8String labelKey = SchemaResourceKeyHelper::GetSchemaDisplayLabelKey(*ecSchema);
    return GetLocalizedString(labelKey.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetSchemaDescription(ECSchemaCP ecSchema, Utf8StringCR invariantDescription) const
    {
    if (m_empty || ecSchema == nullptr)
        return invariantDescription;
    
    Utf8String key = SchemaResourceKeyHelper::GetSchemaDescriptionKey(*ecSchema);
    return GetLocalizedString(key.c_str(), invariantDescription);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetClassDisplayLabel(ECClassCP ecClass, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty || ecClass == nullptr)
        return invariantDisplayLabel;
    
    Utf8String key = SchemaResourceKeyHelper::GetTypeDisplayLabelKey(*ecClass);
    
    return GetLocalizedString(key.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Robert.Schili                      01/2016
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetEnumerationDisplayLabel(ECEnumerationCR ecEnumeration, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;

    Utf8String key = SchemaResourceKeyHelper::GetTypeDisplayLabelKey(ecEnumeration);
    return GetLocalizedString(key.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetClassDescription(ECClassCP ecClass, Utf8StringCR invariantDescription) const
    {
    if (m_empty || ecClass == nullptr)
        return invariantDescription;

    Utf8String key = SchemaResourceKeyHelper::GetTypeDescriptionKey(*ecClass);

    return GetLocalizedString(key.c_str(), invariantDescription);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Robert.Schili                      01/2016
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetEnumerationDescription(ECEnumerationCR ecEnumeration, Utf8StringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;

    Utf8String key = SchemaResourceKeyHelper::GetTypeDescriptionKey(ecEnumeration);

    return GetLocalizedString(key.c_str(), invariantDescription);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetPropertyDisplayLabel(ECPropertyCP ecProperty, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty || ecProperty == nullptr)
        return invariantDisplayLabel;

    Utf8String key = SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(*ecProperty);

    return GetLocalizedString(key.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetPropertyDescription(ECPropertyCP ecProperty, Utf8StringCR invariantDescription) const
    {
    if (m_empty || ecProperty == nullptr)
        return invariantDescription;

    Utf8String key = SchemaResourceKeyHelper::GetTypeChildDescriptionKey(*ecProperty);

    return GetLocalizedString(key.c_str(), invariantDescription);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetEnumeratorDisplayLabel(ECEnumeratorCR ecEnumerator, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;

    Utf8String key = SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(ecEnumerator);
    return GetLocalizedString(key.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetRelationshipSourceRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const
    {
    if (m_empty || relClass == nullptr)
        return invariantRoleLabel;
    
    Utf8String key = SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(*relClass, invariantRoleLabel.c_str());
    return GetLocalizedString(key.c_str(), invariantRoleLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetRelationshipTargetRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const
    {
    if (m_empty || relClass == nullptr)
        return invariantRoleLabel;

    Utf8String key = SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(
        relClass->GetSchema().GetLegacyFullSchemaName().c_str(), relClass->GetName().c_str(), invariantRoleLabel.c_str());

    return GetLocalizedString(key.c_str(), invariantRoleLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
bool SchemaLocalizedStrings::TryGetStringValue(IECInstanceCR instance, Utf8StringR value, Utf8CP accessString)
    {
    ECValue stringValue;
    if (ECObjectsStatus::Success == instance.GetValue(stringValue, accessString) && !stringValue.IsNull())
        {
        value = stringValue.GetUtf8CP();
        return true;
        }
    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
bool SchemaLocalizedStrings::TryGetBoolValue(IECInstanceCR instance, bool & value, Utf8CP accessString)
    {
    ECValue boolValue;
    if (ECObjectsStatus::Success == instance.GetValue(boolValue, accessString) && !boolValue.IsNull())
        {
        value = boolValue.GetBoolean();
        return true;
        }
    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
SchemaLocalizedStrings::SchemaLocalizedStrings(ECSchemaCP localizationSupplemental, ECSchemaR primarySchema)
    {
    bmap<Utf8String, bpair<size_t, Utf8String> > caStrings;
    if (!TryConstructStringMaps(caStrings, localizationSupplemental))
        return;
    
    m_empty = false;

    size_t prefixLength = primarySchema.GetName().length() + 1;
    Utf8String lastContainerAccessor = ""; // This is the container accessor for the primary schema
    Utf8String lastCaClassName;
    IECCustomAttributeContainerP caContainer = &primarySchema;
    IECInstancePtr caInstance;
    for (auto const& it : caStrings)
        {
        Utf8String containerAccessor;
        Utf8String caSchemaName;
        Utf8String caClassName;
        Utf8String caPropertyAccessor;
        if(ECObjectsStatus::Success != ParseCaKeyString(containerAccessor, caSchemaName, caClassName, caPropertyAccessor, it.first, prefixLength, it.second.first))
            {
            LOG.errorv("Invalid key '%s' for localized string '%s' for schema '%s'", it.first.c_str(), it.second.second.c_str(), primarySchema.GetFullSchemaName().c_str());
            continue;
            }
        
        if (!containerAccessor.Equals(lastContainerAccessor))
            {
            lastContainerAccessor = containerAccessor;
            lastCaClassName = "";
            caContainer = GetContainer(containerAccessor, primarySchema);
            }
        
        if (!caClassName.Equals(lastCaClassName))
            {
            lastCaClassName = caClassName;
            caInstance = caContainer->GetLocalAttributeAsSupplemented(caSchemaName, caClassName); // TODO: Check!
            }
        
        if (!caInstance.IsValid())
            {
            LOG.errorv("Cannot apply the localized string '%s' because the custom attribute '%s' or container cannot be found given the key '%s' in schema '%s'", it.second.second.c_str(), caClassName.c_str(), it.first.c_str(), caSchemaName.c_str());
            continue;
            }

        // have ECValue hold the copy because we're not going to hold onto CA strings
        ECValueAccessor accessor;
        if (ECObjectsStatus::Success != ECValueAccessor::PopulateValueAccessor(accessor, *caInstance, caPropertyAccessor.c_str()) ||
            ECObjectsStatus::Success != caInstance->SetValueUsingAccessor(accessor, ECValue(it.second.second.c_str())))
                LOG.errorv("Cannot apply the localized string '%s' using the property accessor '%s' on the custom attribute class '%s' on the container '%s'",
                    it.second.second.c_str(), caPropertyAccessor.c_str(), caClassName.c_str(), containerAccessor.c_str());
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
bool SchemaLocalizedStrings::TryConstructStringMaps(bmap<Utf8String, bpair<size_t, Utf8String> >& caStrings, ECSchemaCP localizationSupplemental)
    {
    IECInstancePtr localizationSpec = localizationSupplemental->GetCustomAttribute(SCHEMANAME, LOC_SPEC);
    if (!localizationSpec.IsValid())
        {
        LOG.errorv("Unable to load schema localizations from '%s' because it does not have a '%s' Custom Attribute", localizationSupplemental->GetFullSchemaName().c_str(), LOC_SPEC);
        return false;
        }

    ECValue resource;
    if (ECObjectsStatus::Success != localizationSpec->GetValue(resource, RESOURCE))
        {
        LOG.errorv("Unable to load schema localizations from '%s' because the Custom Attribute '%s' does not have a value for '%s'",
            localizationSupplemental->GetFullSchemaName().c_str(), LOC_SPEC, RESOURCE);
        return false;
        }

    ArrayInfo resourcesInfo = resource.GetArrayInfo();
    for (uint32_t i = 0; i < resourcesInfo.GetCount(); ++i)
        {
        ECValue stringResource;
        if (ECObjectsStatus::Success == localizationSpec->GetValue(stringResource, RESOURCE, i) && !stringResource.IsNull())
            {
            IECInstancePtr resourceEntry = stringResource.GetStruct();
            if (!resourceEntry.IsValid())
                continue;
            
            Utf8String key;
            Utf8String value;
            if (!TryGetStringValue(*resourceEntry, key, KEY) || !TryGetStringValue(*resourceEntry, value, VALUE))
                continue;

            size_t atIndex = key.find(AT);
            if (atIndex != std::string::npos)
                caStrings[key] = bpair<size_t, Utf8String>(atIndex, value);
            else
                m_localizedStrings[Utf8String(key)] = Utf8String(value);
            }
        }

    return true;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
IECCustomAttributeContainerP SchemaLocalizedStrings::GetContainer(Utf8StringCR containerAccessor, ECSchemaR primarySchema)
    {
    Utf8String className;
    Utf8String relEndPoint;
    Utf8String propertyName;
    if (ECObjectsStatus::Success != ParseContainerAccessor(className, relEndPoint, propertyName, containerAccessor))
        {
        LOG.errorv("Unable to parse '%s' to set a localized string for schema '%s'", containerAccessor.c_str(), primarySchema.GetFullSchemaName().c_str());
        return nullptr;
        }
    
    IECCustomAttributeContainerP caContainer = nullptr;
    if (Utf8String::IsNullOrEmpty(className.c_str()))
        {
        caContainer = &primarySchema;
        }
    else if (Utf8String::IsNullOrEmpty(propertyName.c_str()))
        {
        caContainer = GetClassContainer(className, relEndPoint, primarySchema);
        }
    else
        {
        caContainer = GetPropertyContainer(className, propertyName, primarySchema);
        }
    
    return caContainer;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
IECCustomAttributeContainerP SchemaLocalizedStrings::GetClassContainer(Utf8StringCR className, Utf8StringCR relEndPoint, ECSchemaR primarySchema)
    {
    ECClassP ecClass = primarySchema.GetClassP(className.c_str());
    if (nullptr == ecClass)
        {
        LOG.errorv("Cannot find the class '%s' in schema '%s'", className.c_str(), primarySchema.GetFullSchemaName().c_str());
        return nullptr;
        }

    if (Utf8String::IsNullOrEmpty(relEndPoint.c_str()))
        return ecClass;
    else
        {
        ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
        if (nullptr == relClass)
            {
            LOG.errorv("The class '%s' is not a relationship class, cannot apply localized '%s'", className.c_str(), relEndPoint.c_str());
            return nullptr;
            }
        if (relEndPoint.Equals(SOURCE))
            return &relClass->GetSource();
        else
            return &relClass->GetTarget();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
IECCustomAttributeContainerP SchemaLocalizedStrings::GetPropertyContainer(Utf8StringCR className, Utf8StringCR propertyName, ECSchemaR primarySchema)
    {
    ECClassP ecClass = primarySchema.GetClassP(className.c_str());
    if (nullptr == ecClass)
        {
        LOG.errorv("Cannot find the class '%s' in schema '%s'", className.c_str(), primarySchema.GetFullSchemaName().c_str());
        return nullptr;
        }
    ECPropertyP ecProperty = ecClass->GetPropertyP(propertyName.c_str());
    if (nullptr == ecProperty)
        {
        LOG.errorv("Cannot find the property '%s' in the class '%s' from the schema '%s'", propertyName.c_str(), className.c_str(), primarySchema.GetFullSchemaName().c_str());
        return nullptr;
        }
    
    return ecProperty;
    }

/*---------------------------------------------------------------------------------**//**
* Parses a string like: 
* Standard:[PrimarySchemaFullName]:<ClassName>:<PropertyName>@Standard:[CaClassSchemaFullName]:[CaClassName]:[PropertyAccessor]:[Hash]
* @bsimethod                                    Colin.Kerr                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaLocalizedStrings::ParseCaKeyString(Utf8StringR containerAccessor, Utf8StringR caSchemaName, Utf8StringR caClassName, Utf8StringR propertyAccessor, Utf8StringCR keyString, size_t prefixLength, size_t atIndex)
    {
    if (atIndex > prefixLength)
        containerAccessor = keyString.substr(prefixLength, atIndex - prefixLength); // <ClassName>:<PropertyName>
    size_t beginCaKeyIndex = atIndex + 1; // index after @
    size_t caSchemaClassSepIndex = keyString.find(COLON, beginCaKeyIndex); // index of ':' between CaClassSchemaName and CaClassName
    if (WString::npos == caSchemaClassSepIndex)
        return ECObjectsStatus::ParseError;

    size_t propertyAccessorIndex = keyString.find(COLON, caSchemaClassSepIndex + 1); // index of ':' between CaClassName and PropertyAccessor
    if (WString::npos == propertyAccessorIndex)
        return ECObjectsStatus::ParseError;

    size_t accessorHashSepIndex = keyString.rfind(COLON); // index of ':' between PropertyAccessor and Hash
    if (WString::npos == accessorHashSepIndex)
        return ECObjectsStatus::ParseError;
				
    caSchemaName = keyString.substr(beginCaKeyIndex, caSchemaClassSepIndex - beginCaKeyIndex);
    caClassName = keyString.substr(caSchemaClassSepIndex + 1, propertyAccessorIndex - caSchemaClassSepIndex - 1);
    propertyAccessor = keyString.substr(propertyAccessorIndex + 1, accessorHashSepIndex - propertyAccessorIndex - 1);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Parses a string like: 
* <ClassName>:<PropertyName>
* @bsimethod                                    Colin.Kerr                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaLocalizedStrings::ParseContainerAccessor(Utf8StringR className, Utf8StringR relEndPoint, Utf8StringR propertyName, Utf8StringCR containerAccessor)
    {
    // Is CA applied to schema
    if(Utf8String::IsNullOrEmpty(containerAccessor.c_str()))
        return ECObjectsStatus::Success;
    
    size_t sepIndex = containerAccessor.find(COLON);
    // Is CA applied to a class or rel class endpoint
    if (Utf8String::npos == sepIndex)
        {
        size_t dotIndex = containerAccessor.find(DOT);
        // Is CA applied to class
        if (WString::npos == dotIndex)
            {
            className = Utf8String(containerAccessor);
            return ECObjectsStatus::Success;
            }
        
        // Is CA applied to rel class endpoint
        className = containerAccessor.substr(0, dotIndex);
        relEndPoint = containerAccessor.substr(dotIndex + 1);
        return ECObjectsStatus::Success;
        }
    
    // Has ':' at end of accessor
    if (containerAccessor.length() == sepIndex + 1)
        return ECObjectsStatus::ParseError;

    // CA is applied to a property
    className = containerAccessor.substr(0, sepIndex);
    propertyName = containerAccessor.substr(sepIndex + 1);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaLocalizedStrings::IsLocalizationSupplementalSchema(ECSchemaCP schema)
    {
    return schema->IsDefined (SCHEMANAME, LOC_SPEC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaLocalizedStrings::GetLocaleFromSupplementalSchema(ECSchemaCP schema)
    {
    IECInstancePtr caInstance = schema->GetCustomAttribute(SCHEMANAME, LOC_SPEC);
    if (!caInstance.IsValid())
        return "";
    
    ECValue localeValue;
    caInstance->GetValue(localeValue, LOCALE);
    if (localeValue.IsNull())
        return "";

    return localeValue.GetUtf8CP();
    }


END_BENTLEY_ECOBJECT_NAMESPACE

