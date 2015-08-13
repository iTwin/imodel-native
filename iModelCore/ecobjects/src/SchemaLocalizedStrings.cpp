/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaLocalizedStrings.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static WCharCP const LOC_SPEC           = L"LocalizationSpecification";
static WCharCP const LOCALE             = L"Locale";
static WCharCP const RESOURCE           = L"Resource";
static WCharCP const KEY                = L"Key";
static WCharCP const VALUE              = L"Value";
static WCharCP const IS_GUID            = L"IsGUID";

static WCharCP const STANDARD           = L"Standard";
static WCharCP const DISPLAYLABEL       = L"DisplayLabel";
static WCharCP const DESCRIPTION        = L"Description";
static WCharCP const SOURCE             = L"Source";
static WCharCP const TARGET             = L"Target";
static WCharCP const SOURCEROLELABEL    = L"SourceRoleLabel";
static WCharCP const TARGETROLELABEL    = L"TargetRoleLabel";
static WCharCP const COLON              = L":";
static WCharCP const DOT                = L".";
static WCharCP const GUID               = L"GUID:";
static WCharCP const AT                 = L"@";

WStringCR SchemaLocalizedStrings::GetLocalizedString(WCharCP labelKey, WStringCR invariantString) const
    {
    auto it = m_localizedStrings.find(labelKey);
    if (it != m_localizedStrings.end())
        return it->second;
    return invariantString;
    }

WString SchemaLocalizedStrings::ComputeHash(WStringCR invariantString) const
    {
    return L"";
    //Utf8String convertedString = Utf8String(invariantString);
    //
    //CharP shaHash;
    //apr_sha1_base64(convertedString.c_str(), invariantString.length(), shaHash);
    //wchar_t locHash[8];
    //wprintf(locHash, L"%02x%02x%02x%02x", shaHash[5], shaHash[6], shaHash[7], shaHash[8]);
    //return locHash;
    }

WStringCR SchemaLocalizedStrings::GetSchemaDisplayLabel(ECSchemaCP ecSchema, WStringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02.DisplayLabel:[Hash]
    WString labelKey (STANDARD);
    labelKey.append(COLON);
    labelKey.append(ecSchema->GetFullSchemaName());
    labelKey.append(DOT);
    labelKey.append(DISPLAYLABEL);
    labelKey.append(COLON);
    labelKey.append(ComputeHash(invariantDisplayLabel));
    return GetLocalizedString(labelKey.c_str(), invariantDisplayLabel);
    }

WStringCR SchemaLocalizedStrings::GetSchemaDescription(ECSchemaCP ecSchema, WStringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02.Description:[Hash]
    WString descriptionKey(STANDARD);
    descriptionKey.append(COLON);
    descriptionKey.append(ecSchema->GetFullSchemaName());
    descriptionKey.append(DOT);
    descriptionKey.append(DESCRIPTION);
    descriptionKey.append(COLON);
    descriptionKey.append(ComputeHash(invariantDescription));
    return GetLocalizedString(descriptionKey.c_str(), invariantDescription);
    }

WStringCR SchemaLocalizedStrings::GetClassDisplayLabel(ECClassCP ecClass, WStringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02:Class.DisplayLabel:[Hash]
    WString labelKey(STANDARD);
    labelKey.append(COLON);
    labelKey.append(ecClass->GetSchema().GetFullSchemaName());
    labelKey.append(COLON);
    labelKey.append(ecClass->GetName());
    labelKey.append(DOT);
    labelKey.append(DISPLAYLABEL);
    labelKey.append(COLON);
    labelKey.append(ComputeHash(invariantDisplayLabel));
    return GetLocalizedString(labelKey.c_str(), invariantDisplayLabel);
    }

WStringCR SchemaLocalizedStrings::GetClassDescription(ECClassCP ecClass, WStringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02:Class.Description:[Hash]
    WString descriptionKey(STANDARD);
    descriptionKey.append(COLON);
    descriptionKey.append(ecClass->GetSchema().GetFullSchemaName());
    descriptionKey.append(COLON);
    descriptionKey.append(ecClass->GetName());
    descriptionKey.append(DOT);
    descriptionKey.append(DESCRIPTION);
    descriptionKey.append(COLON);
    descriptionKey.append(ComputeHash(invariantDescription));

    return GetLocalizedString(descriptionKey.c_str(), invariantDescription);
    }

WStringCR SchemaLocalizedStrings::GetPropertyDisplayLabel(ECPropertyCP ecProperty, WStringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02:Class:Property.DisplayLabel:[Hash]
    WString labelKey(STANDARD);
    labelKey.append(COLON);
    labelKey.append(ecProperty->GetClass().GetSchema().GetFullSchemaName());
    labelKey.append(COLON);
    labelKey.append(ecProperty->GetClass().GetName());
    labelKey.append(COLON);
    labelKey.append(ecProperty->GetName());
    labelKey.append(DOT);
    labelKey.append(DISPLAYLABEL);
    labelKey.append(COLON);
    labelKey.append(ComputeHash(invariantDisplayLabel));

    return GetLocalizedString(labelKey.c_str(), invariantDisplayLabel);
    }

WStringCR SchemaLocalizedStrings::GetPropertyDescription(ECPropertyCP ecProperty, WStringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02:Class:Property.Description:[Hash]
    WString descriptionKey(STANDARD);
    descriptionKey.append(COLON);
    descriptionKey.append(ecProperty->GetClass().GetSchema().GetFullSchemaName());
    descriptionKey.append(COLON);
    descriptionKey.append(ecProperty->GetClass().GetName());
    descriptionKey.append(COLON);
    descriptionKey.append(ecProperty->GetName());
    descriptionKey.append(DOT);
    descriptionKey.append(DESCRIPTION);
    descriptionKey.append(COLON);
    descriptionKey.append(ComputeHash(invariantDescription));

    return GetLocalizedString(descriptionKey.c_str(), invariantDescription);
    }

WStringCR SchemaLocalizedStrings::GetRelationshipSourceRoleLabel(ECRelationshipClassCP relClass, WStringCR invariantRoleLabel) const
    {
    if (m_empty)
        return invariantRoleLabel;
    // Standard:Schema.04.02:RelClass.SourceRoleLabel:[Hash]
    WString roleLabelKey(STANDARD);
    roleLabelKey.append(COLON);
    roleLabelKey.append(relClass->GetSchema().GetFullSchemaName());
    roleLabelKey.append(COLON);
    roleLabelKey.append(relClass->GetName());
    roleLabelKey.append(DOT);
    roleLabelKey.append(SOURCEROLELABEL);
    roleLabelKey.append(COLON);
    roleLabelKey.append(ComputeHash(invariantRoleLabel));
    return GetLocalizedString(roleLabelKey.c_str(), invariantRoleLabel);
    }

WStringCR SchemaLocalizedStrings::GetRelationshipTargetRoleLabel(ECRelationshipClassCP relClass, WStringCR invariantRoleLabel) const
    {
    if (m_empty)
        return invariantRoleLabel;
    // Standard:Schema.04.02:RelClass.SourceRoleLabel:[Hash]
    WString roleLabelKey(STANDARD);
    roleLabelKey.append(COLON);
    roleLabelKey.append(relClass->GetSchema().GetFullSchemaName());
    roleLabelKey.append(COLON);
    roleLabelKey.append(relClass->GetName());
    roleLabelKey.append(DOT);
    roleLabelKey.append(TARGETROLELABEL);
    roleLabelKey.append(COLON);
    roleLabelKey.append(ComputeHash(invariantRoleLabel));
    return GetLocalizedString(roleLabelKey.c_str(), invariantRoleLabel);
    }

bool SchemaLocalizedStrings::TryGetStringValue(IECInstanceCR instance, WStringR value, WCharCP accessString)
    {
    ECValue stringValue;
    if (ECOBJECTS_STATUS_Success == instance.GetValue(stringValue, accessString) && !stringValue.IsNull())
        {
        value = stringValue.GetString();
        return true;
        }
    return false;
    }

bool SchemaLocalizedStrings::TryGetBoolValue(IECInstanceCR instance, bool & value, WCharCP accessString)
    {
    ECValue boolValue;
    if (ECOBJECTS_STATUS_Success == instance.GetValue(boolValue, accessString) && !boolValue.IsNull())
        {
        value = boolValue.GetBoolean();
        return true;
        }
    return false;
    }

SchemaLocalizedStrings::SchemaLocalizedStrings(ECSchemaCP localizationSupplemental, ECSchemaR primarySchema)
    {
    bmap<WString, bpair<size_t, WString> > caStrings;
    if (!TryConstructStringMaps(caStrings, localizationSupplemental))
        return;
    
    m_empty = false;

    // Length of the string Standard:[SchemaFullName]:  10 is for the chars 'Standard::'
    size_t prefixLength = 10 + primarySchema.GetFullSchemaName().length();
    WString lastContainerAccessor = L""; // This is the container accessor for the primary schema
    WString lastCaClassName;
    IECCustomAttributeContainerP caContainer = &primarySchema;
    IECInstancePtr caInstance;
    for (auto const& it : caStrings)
        {
        WString containerAccessor;
        WString caClassName;
        WString caPropertyAccessor;
        if(ECOBJECTS_STATUS_Success != ParseCaKeyString(containerAccessor, caClassName, caPropertyAccessor, it.first, prefixLength, it.second.first))
            {
            LOG.errorv(L"Invalid key '%ls' for localized string '%ls' for schema '%ls'", it.first, it.second.second, primarySchema.GetFullSchemaName());
            continue;
            }
        
        if (!containerAccessor.Equals(lastContainerAccessor))
            {
            lastContainerAccessor = WString(containerAccessor);
            lastCaClassName = L"";
            caContainer = GetContainer(containerAccessor, primarySchema);
            }
        
        if (!caClassName.Equals(lastCaClassName))
            {
            lastCaClassName = WString(caClassName);
            caInstance = caContainer->GetLocalAttributeAsConsolidated(caClassName);
            }
        
        if (!caInstance.IsValid())
            {
            LOG.errorv(L"Cannot apply the localized string '%ls' because the custom attribute or container cannot be found given the key '%ls'", it.second.second, it.first);
            continue;
            }

        // have ECValue hold the copy because we're not going to hold onto CA strings
        ECValueAccessor accessor;
        if (ECOBJECTS_STATUS_Success != ECValueAccessor::PopulateValueAccessor(accessor, *caInstance, caPropertyAccessor.c_str()) ||
            ECOBJECTS_STATUS_Success != caInstance->SetValueUsingAccessor(accessor, ECValue(it.second.second.c_str())))
                LOG.errorv(L"Cannot apply the localized string '%ls' using the property accessor '%ls' on the custom attribute class '%ls' on the container '%ls'",
                    it.second.second, caPropertyAccessor, caClassName, containerAccessor);
        }
    }

bool SchemaLocalizedStrings::TryConstructStringMaps(bmap<WString, bpair<size_t, WString> >& caStrings, ECSchemaCP localizationSupplemental)
    {
    IECInstancePtr localizationSpec = localizationSupplemental->GetCustomAttribute(LOC_SPEC);
    if (!localizationSpec.IsValid())
        {
        LOG.errorv(L"Unable to load schema localizations from '%ls' because it does not have a '%ls' Custom Attribute", localizationSupplemental->GetFullSchemaName(), LOC_SPEC);
        return false;
        }

    ECValue resource;
    if (ECOBJECTS_STATUS_Success != localizationSpec->GetValue(resource, RESOURCE))
        {
        LOG.errorv(L"Unable to load schema localizations from '%ls' because the Custom Attribute '%ls' does not have a value for '%ls'",
            localizationSupplemental->GetFullSchemaName(), LOC_SPEC, RESOURCE);
        return false;
        }

    bmap<WString, WString> guidLookUp;
    bmap<WString, bpair<size_t, WString> > entriesReffingGuid;

    ArrayInfo resourcesInfo = resource.GetArrayInfo();
    for (uint32_t i = 0; i < resourcesInfo.GetCount(); ++i)
        {
        ECValue stringResource;
        if (ECOBJECTS_STATUS_Success == localizationSpec->GetValue(stringResource, RESOURCE, i) && !stringResource.IsNull())
            {
            IECInstancePtr resourceEntry = stringResource.GetStruct();
            if (!resourceEntry.IsValid())
                continue;
            
            WString key;
            WString value;
            if (!TryGetStringValue(*resourceEntry, key, KEY) || !TryGetStringValue(*resourceEntry, value, VALUE))
                continue;

            bool isGUID;
            if (!TryGetBoolValue(*resourceEntry, isGUID, IS_GUID))
                isGUID = false;

            // Strip off hash ... TODO stop doing this once we can generate hash correctly
            if (!key.StartsWithI(GUID))
                {
                size_t lastColon = key.rfind(COLON);
                key = key.substr(0, lastColon + 1);
                }

            size_t atIndex = key.find(AT);

            if (isGUID)
                entriesReffingGuid[key] = bpair<size_t, WString>(atIndex, GUID + value);
            else if (key.StartsWithI(GUID))
                guidLookUp[key] = value;
            else if (atIndex != std::string::npos)
                caStrings[key] = bpair<size_t, WString>(atIndex, value);
            else
                m_localizedStrings[WString(key)] = WString(value);
            }
        }

    for (auto const& it : entriesReffingGuid)
        {
        if(it.second.first != std::string::npos)
            caStrings[it.first] = bpair<size_t, WString>(it.second.first, guidLookUp[it.second.second]);
        else
            m_localizedStrings[WString(it.first)] = WString(guidLookUp[it.second.second]);
        }
    
    return true;
    }


IECCustomAttributeContainerP SchemaLocalizedStrings::GetContainer(WStringCR containerAccessor, ECSchemaR primarySchema)
    {
    WString className;
    WString relEndPoint;
    WString propertyName;
    if (ECOBJECTS_STATUS_Success != ParseContainerAccessor(className, relEndPoint, propertyName, containerAccessor))
        {
        LOG.errorv(L"Unable to parse '%ls' to set a localized string for schema '%ls'", containerAccessor, primarySchema.GetFullSchemaName());
        return nullptr;
        }
    
    IECCustomAttributeContainerP caContainer = nullptr;
    if (WString::IsNullOrEmpty(className.c_str()))
        {
        caContainer = &primarySchema;
        }
    else if (WString::IsNullOrEmpty(propertyName.c_str()))
        {
        caContainer = GetClassContainer(className, relEndPoint, primarySchema);
        }
    else
        {
        caContainer = GetPropertyContainer(className, propertyName, primarySchema);
        }
    
    return caContainer;
    }

IECCustomAttributeContainerP SchemaLocalizedStrings::GetClassContainer(WStringCR className, WStringCR relEndPoint, ECSchemaR primarySchema)
    {
    ECClassP ecClass = primarySchema.GetClassP(className.c_str());
    if (nullptr == ecClass)
        {
        LOG.errorv(L"Cannot find the class '%ls' in schema '%ls'", className, primarySchema.GetFullSchemaName());
        return nullptr;
        }

    if (WString::IsNullOrEmpty(relEndPoint.c_str()))
        return ecClass;
    else
        {
        ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
        if (nullptr == relClass)
            {
            LOG.errorv(L"The class '%ls' is not a relationship class, cannot apply localized '%ls'", className, relEndPoint);
            return nullptr;
            }
        if (relEndPoint.Equals(SOURCE))
            return &relClass->GetSource();
        else
            return &relClass->GetTarget();
        }
    return nullptr;
    }

IECCustomAttributeContainerP SchemaLocalizedStrings::GetPropertyContainer(WStringCR className, WStringCR propertyName, ECSchemaR primarySchema)
    {
    ECClassP ecClass = primarySchema.GetClassP(className.c_str());
    if (nullptr == ecClass)
        {
        LOG.errorv(L"Cannot find the class '%ls' in schema '%ls'", className, primarySchema.GetFullSchemaName());
        return nullptr;
        }
    ECPropertyP ecProperty = ecClass->GetPropertyP(propertyName.c_str());
    if (nullptr == ecProperty)
        {
        LOG.errorv(L"Cannot find the property '%ls' in the class '%ls' from the schema '%ls'", propertyName, className, primarySchema.GetFullSchemaName());
        return nullptr;
        }
    
    return ecProperty;
    }

/*---------------------------------------------------------------------------------**//**
* Parses a string like: 
* Standard:[PrimarySchemaFullName]:<ClassName>:<PropertyName>@Standard:[CaClassSchemaFullName]:[CaClassName]:[PropertyAccessor]:[Hash]
* @bsimethod                                    Colin.Kerr                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaLocalizedStrings::ParseCaKeyString(WStringR containerAccessor, WStringR caClassName, WStringR propertyAccessor, WStringCR keyString, size_t prefixLength, size_t atIndex)
    {
    if (atIndex > prefixLength)
        containerAccessor = keyString.substr(prefixLength, atIndex - prefixLength); // <ClassName>:<PropertyName>
    size_t beginCaKeyIndex = atIndex + 9; // index after @Standard:
    size_t caSchemaClassSepIndex = keyString.find(COLON, beginCaKeyIndex + 1); // index of ':' between CaClassSchemaFullName and CaClassName
    if (WString::npos == caSchemaClassSepIndex)
        return ECObjectsStatus::ECOBJECTS_STATUS_ParseError;

    size_t propertyAccessorIndex = keyString.find(COLON, caSchemaClassSepIndex + 1); // index of ':' between CaClassName and PropertyAccessor
    if (WString::npos == propertyAccessorIndex)
        return ECObjectsStatus::ECOBJECTS_STATUS_ParseError;

    size_t accessorHashSepIndex = keyString.rfind(COLON); // index of ':' between PropertyAccessor and Hash
    if (WString::npos == accessorHashSepIndex)
        return ECObjectsStatus::ECOBJECTS_STATUS_ParseError;

    caClassName = keyString.substr(caSchemaClassSepIndex + 1, propertyAccessorIndex - caSchemaClassSepIndex - 1);
    propertyAccessor = keyString.substr(propertyAccessorIndex + 1, accessorHashSepIndex - propertyAccessorIndex - 1);

    return ECObjectsStatus::ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* Parses a string like: 
* <ClassName>:<PropertyName>
* @bsimethod                                    Colin.Kerr                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaLocalizedStrings::ParseContainerAccessor(WStringR className, WStringR relEndPoint, WStringR propertyName, WStringCR containerAccessor)
    {
    // Is CA applied to schema
    if(WString::IsNullOrEmpty(containerAccessor.c_str()))
        return ECObjectsStatus::ECOBJECTS_STATUS_Success;
    
    size_t sepIndex = containerAccessor.find(COLON);
    // Is CA applied to a class or rel class endpoint
    if (WString::npos == sepIndex)
        {
        size_t dotIndex = containerAccessor.find(DOT);
        // Is CA applied to class
        if (WString::npos == dotIndex)
            {
            className = WString(containerAccessor);
            return ECObjectsStatus::ECOBJECTS_STATUS_Success;
            }
        
        // Is CA applied to rel class endpoint
        className = containerAccessor.substr(0, dotIndex);
        relEndPoint = containerAccessor.substr(dotIndex + 1);
        return ECObjectsStatus::ECOBJECTS_STATUS_Success;
        }
    
    // Has ':' at end of accessor
    if (containerAccessor.length() == sepIndex + 1)
        return ECObjectsStatus::ECOBJECTS_STATUS_ParseError;

    // CA is applied to a property
    className = containerAccessor.substr(0, sepIndex);
    propertyName = containerAccessor.substr(sepIndex + 1);

    return ECObjectsStatus::ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaLocalizedStrings::IsLocalizationSupplementalSchema(ECSchemaCP schema)
    {
    return schema->IsDefined (LOC_SPEC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Colin.Kerr                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WString SchemaLocalizedStrings::GetLocaleFromSupplementalSchema(ECSchemaCP schema)
    {
    IECInstancePtr caInstance = schema->GetCustomAttribute(LOC_SPEC);
    if (!caInstance.IsValid())
        return L"";
    
    ECValue localeValue;
    caInstance->GetValue(localeValue, LOCALE);
    if (localeValue.IsNull())
        return L"";

    return localeValue.GetString();
    }


END_BENTLEY_ECOBJECT_NAMESPACE

