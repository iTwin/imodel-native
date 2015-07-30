/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaLocalizedStrings.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static Utf8CP const LOC_SPEC           = "LocalizationSpecification";
static Utf8CP const LOCALE             = "Locale";
static Utf8CP const RESOURCE           = "Resource";
static Utf8CP const KEY                = "Key";
static Utf8CP const VALUE              = "Value";
static Utf8CP const IS_GUID            = "IsGUID";

static Utf8CP const STANDARD           = "Standard";
static Utf8CP const DISPLAYLABEL       = "DisplayLabel";
static Utf8CP const DESCRIPTION        = "Description";
static Utf8CP const SOURCE             = "Source";
static Utf8CP const SOURCEROLELABEL    = "SourceRoleLabel";
static Utf8CP const TARGETROLELABEL    = "TargetRoleLabel";
static Utf8CP const COLON              = ":";
static Utf8CP const DOT                = ".";
static Utf8CP const GUID               = "GUID:";
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
Utf8String SchemaLocalizedStrings::ComputeHash(Utf8StringCR invariantString) const
    {
    return "";
    //Utf8String convertedString = Utf8String(invariantString);
    //
    //CharP shaHash;
    //apr_sha1_base64(convertedString.c_str(), invariantString.length(), shaHash);
    //wchar_t locHash[8];
    //wprintf(locHash, L"%02x%02x%02x%02x", shaHash[5], shaHash[6], shaHash[7], shaHash[8]);
    //return locHash;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetSchemaDisplayLabel(ECSchemaCP ecSchema, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02.DisplayLabel:[Hash]
    Utf8String labelKey (STANDARD);
    labelKey.append(COLON);
    labelKey.append(ecSchema->GetFullSchemaName());
    labelKey.append(DOT);
    labelKey.append(DISPLAYLABEL);
    labelKey.append(COLON);
    labelKey.append(ComputeHash(invariantDisplayLabel));
    return GetLocalizedString(labelKey.c_str(), invariantDisplayLabel);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetSchemaDescription(ECSchemaCP ecSchema, Utf8StringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02.Description:[Hash]
    Utf8String descriptionKey(STANDARD);
    descriptionKey.append(COLON);
    descriptionKey.append(ecSchema->GetFullSchemaName());
    descriptionKey.append(DOT);
    descriptionKey.append(DESCRIPTION);
    descriptionKey.append(COLON);
    descriptionKey.append(ComputeHash(invariantDescription));
    return GetLocalizedString(descriptionKey.c_str(), invariantDescription);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetClassDisplayLabel(ECClassCP ecClass, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02:Class.DisplayLabel:[Hash]
    Utf8String labelKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetClassDescription(ECClassCP ecClass, Utf8StringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02:Class.Description:[Hash]
    Utf8String descriptionKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetPropertyDisplayLabel(ECPropertyCP ecProperty, Utf8StringCR invariantDisplayLabel) const
    {
    if (m_empty)
        return invariantDisplayLabel;
    // Standard:Schema.04.02:Class:Property.DisplayLabel:[Hash]
    Utf8String labelKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetPropertyDescription(ECPropertyCP ecProperty, Utf8StringCR invariantDescription) const
    {
    if (m_empty)
        return invariantDescription;
    // Standard:Schema.04.02:Class:Property.Description:[Hash]
    Utf8String descriptionKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetRelationshipSourceRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const
    {
    if (m_empty)
        return invariantRoleLabel;
    // Standard:Schema.04.02:RelClass.SourceRoleLabel:[Hash]
    Utf8String roleLabelKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
Utf8StringCR SchemaLocalizedStrings::GetRelationshipTargetRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const
    {
    if (m_empty)
        return invariantRoleLabel;
    // Standard:Schema.04.02:RelClass.SourceRoleLabel:[Hash]
    Utf8String roleLabelKey(STANDARD);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
bool SchemaLocalizedStrings::TryGetStringValue(IECInstanceCR instance, Utf8StringR value, Utf8CP accessString)
    {
    ECValue stringValue;
    if (ECOBJECTS_STATUS_Success == instance.GetValue(stringValue, accessString) && !stringValue.IsNull())
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
    if (ECOBJECTS_STATUS_Success == instance.GetValue(boolValue, accessString) && !boolValue.IsNull())
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

    // Length of the string Standard:[SchemaFullName]:  10 is for the chars 'Standard::'
    size_t prefixLength = 10 + primarySchema.GetFullSchemaName().length();
    Utf8String lastContainerAccessor = ""; // This is the container accessor for the primary schema
    Utf8String lastCaClassName;
    IECCustomAttributeContainerP caContainer = &primarySchema;
    IECInstancePtr caInstance;
    for (auto const& it : caStrings)
        {
        Utf8String containerAccessor;
        Utf8String caClassName;
        Utf8String caPropertyAccessor;
        if(ECOBJECTS_STATUS_Success != ParseCaKeyString(containerAccessor, caClassName, caPropertyAccessor, it.first, prefixLength, it.second.first))
            {
            LOG.errorv("Invalid key '%s' for localized string '%s' for schema '%s'", it.first.c_str(), it.second.second.c_str(), primarySchema.GetFullSchemaName().c_str());
            continue;
            }
        
        if (!containerAccessor.Equals(lastContainerAccessor))
            {
            lastContainerAccessor = Utf8String(containerAccessor);
            lastCaClassName = "";
            caContainer = GetContainer(containerAccessor, primarySchema);
            }
        
        if (!caClassName.Equals(lastCaClassName))
            {
            lastCaClassName = Utf8String(caClassName);
            caInstance = caContainer->GetLocalAttributeAsSupplemented(caClassName);
            if (!caInstance.IsValid())
                {
                LOG.errorv("Cannot apply the localized string '%s' because the custom attribute or container cannot be found given the key '%s'", it.second.second.c_str(), it.first.c_str());
                continue;
                }
            }
        
        // have ECValue hold the copy because we're not going to hold onto CA strings
        ECValueAccessor accessor;
        if (ECOBJECTS_STATUS_Success != ECValueAccessor::PopulateValueAccessor(accessor, *caInstance, caPropertyAccessor.c_str()) ||
            ECOBJECTS_STATUS_Success != caInstance->SetValueUsingAccessor(accessor, ECValue(it.second.second.c_str())))
                LOG.errorv("Cannot apply the localized string '%s' using the property accessor '%s' on the custom attribute class '%s' on the container '%s'",
                    it.second.second.c_str(), caPropertyAccessor.c_str(), caClassName.c_str(), containerAccessor.c_str());
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                      04/2015
//--------------------------------------------------------------------------------------
bool SchemaLocalizedStrings::TryConstructStringMaps(bmap<Utf8String, bpair<size_t, Utf8String> >& caStrings, ECSchemaCP localizationSupplemental)
    {
    IECInstancePtr localizationSpec = localizationSupplemental->GetCustomAttribute(LOC_SPEC);
    if (!localizationSpec.IsValid())
        {
        LOG.errorv("Unable to load schema localizations from '%s' because it does not have a '%s' Custom Attribute", localizationSupplemental->GetFullSchemaName().c_str(), LOC_SPEC);
        return false;
        }

    ECValue resource;
    if (ECOBJECTS_STATUS_Success != localizationSpec->GetValue(resource, RESOURCE))
        {
        LOG.errorv("Unable to load schema localizations from '%s' because the Custom Attribute '%s' does not have a value for '%s'",
            localizationSupplemental->GetFullSchemaName().c_str(), LOC_SPEC, RESOURCE);
        return false;
        }

    bmap<Utf8String, Utf8String> guidLookUp;
    bmap<Utf8String, bpair<size_t, Utf8String> > entriesReffingGuid;

    ArrayInfo resourcesInfo = resource.GetArrayInfo();
    for (uint32_t i = 0; i < resourcesInfo.GetCount(); ++i)
        {
        ECValue stringResource;
        if (ECOBJECTS_STATUS_Success == localizationSpec->GetValue(stringResource, RESOURCE, i) && !stringResource.IsNull())
            {
            IECInstancePtr resourceEntry = stringResource.GetStruct();
            if (!resourceEntry.IsValid())
                continue;
            
            Utf8String key;
            Utf8String value;
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
                entriesReffingGuid[key] = bpair<size_t, Utf8String>(atIndex, GUID + value);
            else if (key.StartsWithI(GUID))
                guidLookUp[key] = value;
            else if (atIndex != std::string::npos)
                caStrings[key] = bpair<size_t, Utf8String>(atIndex, value);
            else
                m_localizedStrings[Utf8String(key)] = Utf8String(value);
            }
        }

    for (auto const& it : entriesReffingGuid)
        {
        if(it.second.first != std::string::npos)
            caStrings[it.first] = bpair<size_t, Utf8String>(it.second.first, guidLookUp[it.second.second]);
        else
            m_localizedStrings[Utf8String(it.first)] = Utf8String(guidLookUp[it.second.second]);
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
    if (ECOBJECTS_STATUS_Success != ParseContainerAccessor(className, relEndPoint, propertyName, containerAccessor))
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
ECObjectsStatus SchemaLocalizedStrings::ParseCaKeyString(Utf8StringR containerAccessor, Utf8StringR caClassName, Utf8StringR propertyAccessor, Utf8StringCR keyString, size_t prefixLength, size_t atIndex)
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
ECObjectsStatus SchemaLocalizedStrings::ParseContainerAccessor(Utf8StringR className, Utf8StringR relEndPoint, Utf8StringR propertyName, Utf8StringCR containerAccessor)
    {
    // Is CA applied to schema
    if(Utf8String::IsNullOrEmpty(containerAccessor.c_str()))
        return ECObjectsStatus::ECOBJECTS_STATUS_Success;
    
    size_t sepIndex = containerAccessor.find(COLON);
    // Is CA applied to a class or rel class endpoint
    if (Utf8String::npos == sepIndex)
        {
        size_t dotIndex = containerAccessor.find(DOT);
        // Is CA applied to class
        if (WString::npos == dotIndex)
            {
            className = Utf8String(containerAccessor);
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
Utf8String SchemaLocalizedStrings::GetLocaleFromSupplementalSchema(ECSchemaCP schema)
    {
    IECInstancePtr caInstance = schema->GetCustomAttribute(LOC_SPEC);
    if (!caInstance.IsValid())
        return "";
    
    ECValue localeValue;
    caInstance->GetValue(localeValue, LOCALE);
    if (localeValue.IsNull())
        return "";

    return localeValue.GetUtf8CP();
    }


END_BENTLEY_ECOBJECT_NAMESPACE

