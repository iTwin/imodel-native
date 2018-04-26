/*--------------------------------------------------------------------------------------+
|
|     $Source: src/KindOfQuantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <algorithm>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus KindOfQuantity::SetName(Utf8CP name)
    {
    if (!m_validatedName.SetValidName(name, false))
        return ECObjectsStatus::InvalidName;

    m_fullName = GetSchema().GetName() + ":" + GetName();
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                03/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::Verify() const
    {
    bool isValid = true;
    if (nullptr == m_persistenceUnit)
        {
        LOG.errorv("Validation Error - KindOfQuantity '%s' does not have a persistence unit",
                   GetFullName().c_str());
        isValid = false;
        }

    if ((nullptr != m_persistenceUnit) && m_persistenceUnit->IsConstant())
        {
        LOG.errorv("Validation Error - KindOfQuantity '%s' persistence FormatUnitSet unit is a constant", GetFullName().c_str());
        isValid = false;
        }
    
    for (auto const& format : m_presentationFormats)
        {
        if (format.IsProblem())
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation FormatUnitSet has a problem: %s",
                GetFullName().c_str(), format.GetProblemDescription().c_str());
            isValid = false;
            }
        else if (nullptr != m_persistenceUnit && 
                format.HasCompositeMajorUnit() &&
                !Units::Unit::AreCompatible(format.GetCompositeMajorUnit(), m_persistenceUnit))
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation format input unit conflicts with the persistence unit %s.",
                GetFullName().c_str(), m_persistenceUnit->GetName().c_str());
            isValid = false;
            }
        }

    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR KindOfQuantity::GetFullName() const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();

    return m_fullName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String KindOfQuantity::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an KindOfQuantity name with an alias unless the schema containing the KindOfQuantity is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  KindOfQuantity: %s\n ECSchema containing KindOfQuantity: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::SetDisplayLabel(Utf8CP value) {m_validatedName.SetDisplayLabel(value); return ECObjectsStatus::Success;}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParseName(Utf8StringR alias, Utf8StringR kindOfQuantityName, Utf8StringCR stringToParse)
    {
    if (0 == stringToParse.length())
        return ECObjectsStatus::ParseError;

    Utf8String::size_type colonIndex = stringToParse.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        kindOfQuantityName = stringToParse;
        return ECObjectsStatus::Success;
        }

    if (stringToParse.length() == colonIndex + 1)
        return ECObjectsStatus::ParseError;

    if (0 == colonIndex)
        alias.clear();
    else
        alias = stringToParse.substr(0, colonIndex);

    kindOfQuantityName = stringToParse.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR KindOfQuantity::GetDisplayLabel() const
    {
    return GetSchema().GetLocalizedStrings().GetKindOfQuantityDisplayLabel(*this, GetInvariantDisplayLabel()); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR KindOfQuantity::GetDescription() const 
    { 
    return GetSchema().GetLocalizedStrings().GetKindOfQuantityDescription(*this, m_description); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::SetPersistenceUnit(ECUnitCR unit)
    {
    if (unit.IsConstant())
        {
        LOG.errorv("On KoQ '%s' cannot set unit '%s' as the persistence unit because it is a constant", GetFullName().c_str(), unit.GetFullName().c_str());
        return ECObjectsStatus::Error;
        }
    if (HasPresentationFormats())
        {
        for (auto const& format : m_presentationFormats)
            {
            if (format.HasCompositeMajorUnit() && !ECUnit::AreCompatible(&unit, format.GetCompositeMajorUnit()))
                {
                LOG.errorv("On KoQ '%s' cannot set unit '%s' as the persistence unit because it is not compatible with format '%s'", GetFullName().c_str(), unit.GetFullName().c_str(), format.GetName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        }
    m_persistenceUnit = &unit;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void KindOfQuantity::RemovePresentationFormat(NamedFormatCR presentationFormat)
    {
    m_presentationFormats.erase(std::remove_if(m_presentationFormats.begin(), m_presentationFormats.end(), [&](NamedFormatCR format) {return format.IsIdentical(presentationFormat);}));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ECFormatCP> const KindOfQuantity::GetReferencedFormats() const
    {
    bvector<ECFormatCP> formats;
    formats.reserve(m_presentationFormats.size());
    for (auto const& f : m_presentationFormats)
        {
        if (formats.end() == std::find(formats.begin(), formats.end(), f.GetParentFormat()))
            formats.push_back(f.GetParentFormat());
        }
    
    return formats;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8String KindOfQuantity::GetPresentationUnitDescriptor() const
    {
    Utf8String descriptor = "";

    bool first = true;
    for(NamedFormatCR format : m_presentationFormats)
        {
        if (!first)
            descriptor += ";";

        descriptor.append(format.GetName());
        first = false;
        }

    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus KindOfQuantity::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    //will only be serialized in 3.0 and later
    if (ecXmlVersion < ECVersion::V3_0)
        return SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(KIND_OF_QUANTITY_ELEMENT);
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    if (!GetInvariantDisplayLabel().empty())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    if (nullptr == GetPersistenceUnit())
        {
        LOG.errorv("Failed to write schema because KindOfQuantity '%s' does not have a perisistence unit", GetName().c_str());
        return SchemaWriteStatus::FailedToSaveXml;
        }

    auto getUnitNameFromVersion = [this, &ecXmlVersion](ECUnitCP unit, Utf8StringR out) -> bool 
        {
        if (ecXmlVersion > ECVersion::V3_1)
            {
            out = unit->GetQualifiedName(GetSchema());
            return true;
            }
        // EC3.0 and EC3.1
        auto ecName = Units::UnitNameMappings::TryGetNewNameFromECName(unit->GetFullName().c_str());
        if (nullptr != ecName)
            { 
            out = ecName;
            return true;
            }
        return false;
        };
    Utf8String persistenceUnitString;
    auto hasValidUnitForVersion = getUnitNameFromVersion(GetPersistenceUnit(), persistenceUnitString);
    if (!hasValidUnitForVersion)
        {
        LOG.errorv("Failed to write KindOfQuantity %s because it has a persistence unit not defined in the version it is being serialized to", GetName().c_str());
        return SchemaWriteStatus::FailedToSaveXml;
        }

    xmlWriter.WriteAttribute(PERSISTENCE_UNIT_ATTRIBUTE, persistenceUnitString.c_str());

    double relError = GetRelativeError();
    xmlWriter.WriteAttribute(ECXML_RELATIVE_ERROR_ATTRIBUTE, relError);

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
    if (presentationUnits.size() > 0)
        {
        Utf8String presentationUnitString;
        Utf8String presUnit;
        bool first = true;
        for(NamedFormatCR format : presentationUnits)
            {
            if (format.IsProblem())
                {
                LOG.errorv("Failed to write schema because presentation Format for KindOfQuantity '%s' has problem: '%s'.", GetFullName().c_str(), format.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToSaveXml;
                }

            if(ecXmlVersion < ECVersion::V3_2)
                {
                if (!format.HasComposite() || !format.GetCompositeSpec()->HasMajorUnit())
                    {
                    LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
                    continue;
                    }
                auto hasValidName = getUnitNameFromVersion((ECUnitCP)format.GetCompositeSpec()->GetMajorUnit(), presUnit);
                if(!hasValidName)
                    {
                    LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
                    continue;
                    }
                SchemaKey key("Formats", 1, 0, 0);
                if (!format.GetParentFormat()->GetSchema().GetSchemaKey().Matches(key, SchemaMatchType::Latest))
                    {
                    LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it is not a standard format", GetFullName().c_str());
                    continue;
                    }
                bvector<Utf8String> tokens;
                BeStringUtilities::Split(format.GetName().c_str(), "[", tokens);
                BeAssert(tokens.size() > 0);
                Utf8String split = tokens[0];
                Utf8CP mapped = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(("FORMATS:" + split).c_str());
                mapped = Formatting::AliasMappings::TryGetAliasFromName(mapped);
                if (nullptr == mapped)
                    {
                    LOG.warningv("Dropping presentation format '%s' for KindOfQuantity '%s' because it could not be mapped to an old format", format.GetQualifiedName(GetSchema()).c_str(), GetFullName().c_str());
                    continue;
                    }
                if (!first)
                    presentationUnitString += ";";
                presentationUnitString += presUnit;
                presentationUnitString += "(";
                presentationUnitString += mapped;
                presentationUnitString += ")";
                first = false;
                }
            else
                {
                if (!first)
                    presentationUnitString += ";";
                presentationUnitString += format.GetQualifiedName(GetSchema());
                first = false;
                }
            }
        xmlWriter.WriteAttribute(PRESENTATION_UNITS_ATTRIBUTE, presentationUnitString.c_str());
        }

    xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus KindOfQuantity::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema items
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = KIND_OF_QUANTITY_ELEMENT;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (0 != GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // KindOfQuantity Properties
    if (nullptr == GetPersistenceUnit())
        {
        LOG.errorv("Failed to write schema because KindOfQuantity '%s' does not have a persistence unit ", GetName().c_str());
        return SchemaWriteStatus::FailedToCreateJson;
        }

    outValue[PERSISTENCE_UNIT_ATTRIBUTE] = GetPersistenceUnit()->GetQualifiedName(GetSchema());

    outValue[ECJSON_RELATIVE_ERROR_ATTRIBUTE] = GetRelativeError();

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
    if (0 != presentationUnits.size())
        {
        Json::Value presentationUnitArr(Json::ValueType::arrayValue);
        for (auto const& format : presentationUnits)
            {
            if (format.IsProblem())
                {
                LOG.errorv("Failed to write schema because persistance format for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), format.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToCreateJson;
                }
            presentationUnitArr.append(format.GetQualifiedName(GetSchema()));
            }
        outValue[PRESENTATION_UNITS_ATTRIBUTE] = presentationUnitArr;
        }

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus KindOfQuantity::ReadXml(BeXmlNodeR kindOfQuantityNode, ECSchemaReadContextR context)
    {
    Utf8String value;      // used by the macros.

    if (GetName().length() == 0)
        READ_REQUIRED_XML_ATTRIBUTE(kindOfQuantityNode, TYPE_NAME_ATTRIBUTE, this, Name, kindOfQuantityNode.GetName())

    READ_OPTIONAL_XML_ATTRIBUTE(kindOfQuantityNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(kindOfQuantityNode, DESCRIPTION_ATTRIBUTE, this, Description)

    double relError;
    if (BEXML_Success != kindOfQuantityNode.GetAttributeDoubleValue(relError, ECXML_RELATIVE_ERROR_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), ECXML_RELATIVE_ERROR_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetRelativeError(relError);

    // Read Persistence Unit
    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, PERSISTENCE_UNIT_ATTRIBUTE) || Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (ECObjectsStatus::Success != ParsePersistenceUnit(value.c_str(), &context, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor()))
        return SchemaReadStatus::InvalidECSchemaXml; // Logging in ParsePersistenceUnit

    // Read Presentation Formats
    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationFormats;
        BeStringUtilities::Split(value.c_str(), ";", presentationFormats);
        bool first = true;
        for(auto const& presValue : presentationFormats)
            {
            if (ECObjectsStatus::Success != ParsePresentationUnit(presValue.c_str(), context, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor(), first))
                return SchemaReadStatus::InvalidECSchemaXml; // Logging in ParsePresentationUnit
            first = false;
            }
        }
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
static ECObjectsStatus ExtractUnitFormatAndMap(Utf8StringR unitName, Utf8StringR formatName, Utf8CP descriptor)
    {
    Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);
    unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
    if (unitName.empty())
        {
        LOG.errorv("Failed to find unit mapping for unit with name '%s' in legacy unit mappings", unitName.c_str());
        return ECObjectsStatus::InvalidUnitName;
        }
    Utf8CP mappedName = formatName.c_str();;
    if (!Utf8String::IsNullOrEmpty(formatName.c_str()))
        {
        mappedName = Formatting::AliasMappings::TryGetNameFromAlias(formatName.c_str());
        mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName((nullptr == mappedName) ? formatName.c_str() : mappedName);
        if (nullptr == mappedName)
            {
            LOG.errorv("Failed to find format mapping for format with name '%s' in legacy format mappings", mappedName);
            return ECObjectsStatus::InvalidFormat;
            }
        Utf8String alias;
        Utf8String name;
        ECClass::ParseClassName(alias, name, mappedName);
        formatName = ("f:" + name).c_str();
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParseDescriptorAndAddRefs(Utf8StringR unitName, Utf8StringR formatName, ECUnitCP& unit, Utf8CP descriptor, ECSchemaReadContextP context)
    {
    ECObjectsStatus status = ExtractUnitFormatAndMap(unitName, formatName, descriptor);
    if (ECObjectsStatus::Success != status)
        return status;

    Utf8String alias;
    ECClass::ParseClassName(alias, unitName, unitName);
    SchemaKey key("Units", 1, 0, 0);
    auto unitsSchema = context->LocateSchema(key, SchemaMatchType::Latest);

    if (!ECSchema::IsSchemaReferenced(GetSchema(), *unitsSchema))
        { 
        LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve unit '%s'.",
            unitsSchema->GetName().c_str(), GetSchema().GetName().c_str(), unitName.c_str());
        if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*unitsSchema))
            {
            LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", unitsSchema->GetName().c_str(), GetSchema().GetName().c_str());
            return ECObjectsStatus::Error;
            }
        }

    unit = unitsSchema->GetUnitsContext().LookupUnit(unitName.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located in the standard Units schema.",
            descriptor, GetFullName().c_str(), unitName.c_str());
        return ECObjectsStatus::Error;
        }

    key = SchemaKey("Formats", 1, 0, 0);
    auto formatsSchema = context->LocateSchema(key, SchemaMatchType::Latest);

    if (!ECSchema::IsSchemaReferenced(GetSchema(), *formatsSchema))
        { 
        LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve format '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str(), formatName.c_str());
        if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*formatsSchema))
            {
            LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
            return ECObjectsStatus::Error;
            }
        }

    return ECObjectsStatus::Success;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePersistenceUnit(Utf8CP descriptor, ECSchemaReadContextP context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion)
    {
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion) && (nullptr != context);
    ECUnitCP unit = nullptr;
    Nullable<int32_t> prec = nullptr;
    ECFormatCP persistenceFormat = nullptr;
    if (xmlLessThan32)
        {
        Utf8String unitName;
        Utf8String formatName;
        if (ECObjectsStatus::Success != ParseDescriptorAndAddRefs(unitName, formatName, unit, descriptor, context))
            return ECObjectsStatus::Error;
        if (!Utf8String::IsNullOrEmpty(formatName.c_str()))
            {
            bvector<Utf8String> unitNames;
            bvector<Nullable<Utf8String>> unitLabels; // TODO make these optional since we don't care about them here
            Formatting::Format::ParseFormatString(formatName, prec, unitNames, unitLabels, formatName);
            persistenceFormat = GetSchema().LookupFormat(formatName.c_str());
            if (nullptr == persistenceFormat)
                {
                LOG.errorv("Failed to find format with name '%s' in standard formats schema", formatName.c_str());
                return ECObjectsStatus::Error;
                }
            }
        }
    else
        unit = GetSchema().GetUnitsContext().LookupUnit(descriptor);

    if (nullptr == unit)
        {
        LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located",
            descriptor, GetFullName().c_str(), descriptor);
        return ECObjectsStatus::Error;
        }

    BeAssert(nullptr != unit);
    if (unit->IsConstant())
        { 
        LOG.errorv("Persistence FormatUnitSet: '%s' on KindOfQuanity '%s' has a Constant as a persistence unit", descriptor, GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (ECObjectsStatus::Success != SetPersistenceUnit(*unit))
        {
        LOG.errorv("On KOQ '%s' failed to set persistence unit with name '%s'", GetFullName().c_str(), unit->GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (nullptr != persistenceFormat && ECObjectsStatus::Success != AddPresentationFormatSingleUnitOverride(*persistenceFormat, prec, unit))
        {
        LOG.errorv("On KOQ '%s' failed to set presentation format with name '%s'", GetFullName().c_str(), persistenceFormat->GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

// Used for upgrading from ECXml 3.1.
static const Utf8CP oldDefaultFormatName = "DefaultRealU";

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePresentationUnit(Utf8CP descriptor, ECSchemaReadContextR context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion, bool shouldBeDefault)
    {
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion);

    if (xmlLessThan32)
        {
        ECUnitCP unit;
        ECFormatCP format;
        Utf8String unitName;
        Utf8String formatName;
        if (ECObjectsStatus::Success != ParseDescriptorAndAddRefs(unitName, formatName, unit, descriptor, &context))
            return ECObjectsStatus::Error;

        Nullable<int32_t> precision = nullptr;
        if (!Utf8String::IsNullOrEmpty(formatName.c_str()))
            {
            Utf8String localformatName;
            bvector<Utf8String> localUnitNames;
            bvector<Nullable<Utf8String>> localUnitLabels;
            if (SUCCESS != Formatting::Format::ParseFormatString(localformatName, precision, localUnitNames, localUnitLabels, formatName))
                {
                LOG.errorv("Failed to parse format string '%s' on KoQ '%s'", descriptor, GetFullName().c_str());
                return ECObjectsStatus::Error;
                }
            format = GetSchema().LookupFormat(localformatName.c_str());
            if (nullptr == format)
                {
                LOG.errorv("FormatString '%s' on KindOfQuantity '%s' has an invalid format, '%s'.", descriptor, GetFullName().c_str(), formatName.c_str());
                return ECObjectsStatus::Error;
                }
            }
        else
            {
            // Assuming since there was previously a format that it should contain the Unit with it.
            Utf8String alias;
            Utf8String localFormatName;
            Utf8CP mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(oldDefaultFormatName);
            BeAssert(nullptr != mappedName); // Default should always map
            ECClass::ParseClassName(alias, localFormatName, mappedName);
            Utf8String lookupFormat = "f:" + localFormatName;
            format = GetSchema().LookupFormat(lookupFormat.c_str());
            BeAssert(nullptr != format);
            LOG.warningv("Setting format to DefaultRealU for FormatUnitSet '%s' on KindOfQuantity '%s'.", descriptor, GetFullName().c_str());
            }

        if (nullptr == unit)
            {
            LOG.errorv("FormatString '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located in the standard Units schema.",
                descriptor, GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }

        if (format->HasCompositeMajorUnit() && !Units::Unit::AreEqual(unit, format->GetCompositeMajorUnit()))
            {
            LOG.errorv("On KOQ '%s' presentation unit '%s' must be compatible with parent format '%s' major unit", GetFullName().c_str(), unit->GetFullName().c_str(), format->GetParentFormat()->GetFullName().c_str());
            return ECObjectsStatus::Error;
            }

        if (!Units::Unit::AreCompatible(unit, m_persistenceUnit))
            {
            LOG.errorv("On KOQ '%s' presentation unit '%s' is incompatible with persistence unit '%s'", GetFullName().c_str(), unit->GetFullName().c_str(), m_persistenceUnit->GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        ECUnitCP unitOverride = format->HasCompositeMajorUnit() ? nullptr : unit;
        ECObjectsStatus status = AddPresentationFormatSingleUnitOverride(*format, precision, unitOverride, nullptr, shouldBeDefault);
        if (ECObjectsStatus::Success != status)
            return status;
        }
    else // >= 3.2
        {
        bvector<Utf8String> names;
        bvector<Nullable<Utf8String>> unitLabels;
        Nullable<int32_t> precision;
        Utf8String formatName;
        if (SUCCESS != Formatting::Format::ParseFormatString(formatName, precision, names, unitLabels, descriptor))
            {
            LOG.errorv("Failed to parse format string '%s' on KoQ '%s'", descriptor, GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        ECFormatCP format = GetSchema().LookupFormat(formatName.c_str());
        if (nullptr == format)
            {
            LOG.errorv("Failed to lookup format '%s' on koq '%s'", formatName.c_str(), GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        UnitAndLabelPairs unitsAndLabels;
        int i = 0;
        for (const auto& name : names)
            {
            ECUnitCP lookedUpUnit = GetSchema().GetUnitsContext().LookupUnit(name.c_str());
            if (nullptr != lookedUpUnit)
                {
                Utf8CP localLabel = nullptr;
                // Check to see if it has a label override;
                if (i < unitLabels.size() && unitLabels[i].IsValid())
                    localLabel = unitLabels[i].ValueR().c_str();
                unitsAndLabels.push_back(make_bpair(lookedUpUnit, localLabel));
                }
            else
                {
                LOG.errorv("On KOQ '%s' could not find unit '%s' being added as a presentation unit override", GetFullName().c_str(), name.c_str());
                return ECObjectsStatus::Error;
                }
            i++;
            }

        ECObjectsStatus status = AddPresentationFormat(*format, precision, &unitsAndLabels, shouldBeDefault);
        if (ECObjectsStatus::Success != status)
            return status;
        }
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::UpdateFUSDescriptors(Utf8StringR unitName, bvector<Utf8String>& formatStrings, Utf8CP persFus, bvector<Utf8CP> const& presFuses)
    {
    unitName.clear();
    formatStrings.clear();
    if (Utf8String::IsNullOrEmpty(persFus))
        return ECObjectsStatus::NullPointerValue;

    //Persistence
    Utf8String persistenceUnit;
    Utf8String persistenceFormat;
    Utf8String alias;
    Utf8String unqualifiedPers;
    ECObjectsStatus status = ExtractUnitFormatAndMap(persistenceUnit, persistenceFormat, persFus);
    if (ECObjectsStatus::Success != status )
        return status;

    if (persistenceFormat.empty())
        persistenceFormat = oldDefaultFormatName;

    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPers, persistenceUnit))
        return ECObjectsStatus::Error;


    // Presentation
    for (Utf8CP presFus : presFuses)
        {
        Utf8String presentationUnit;
        Utf8String presentationFormat;
        status = ExtractUnitFormatAndMap(presentationUnit, presentationFormat, presFus);
        if (ECObjectsStatus::Success != status)
            return status;

        if (presentationUnit.empty())
            {
            LOG.errorv("Presentation unit was not defined in this descriptor '%s'", presFus);
            return ECObjectsStatus::InvalidUnitName;
            }

        if (presentationFormat.empty())
            presentationFormat = oldDefaultFormatName;

        Utf8String unqualifiedPres;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPres, presentationUnit))
            return ECObjectsStatus::Error;

        Utf8String unqualifiedPresFormat;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPresFormat, presentationFormat))
            return ECObjectsStatus::Error;

        Utf8String formatString;
        formatString
            .append("f:")
            .append(unqualifiedPresFormat);
        bvector<Utf8String> split;
        BeStringUtilities::Split(unqualifiedPresFormat.c_str(), "(", split);
        auto format = StandardFormatsHelper::GetFormat(split[0].c_str());
        if (nullptr == format || !format->HasComposite())
            {
            formatString
                .append("[")
                .append("u:")
                .append(unqualifiedPres)
                .append("]");
            }
        formatStrings.push_back(formatString);
        }

    // If we have a persistence FUS with a specified format. Put it at the end
    if (!persistenceFormat.empty())
        {
        Utf8String unqualifiedPersFormat;
        if(ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPersFormat, persistenceFormat))
            return ECObjectsStatus::Error;

        Utf8String formatString;
        formatString
            .append("f:")
            .append(unqualifiedPersFormat);
        bvector<Utf8String> split;
        BeStringUtilities::Split(unqualifiedPersFormat.c_str(), "(", split);
        auto format = StandardFormatsHelper::GetFormat(split[0].c_str());
        if (nullptr == format || !format->HasComposite())
            {
            formatString
                .append("[")
                .append("u:")
                .append(unqualifiedPers)
                .append("]");
            }

        formatStrings.push_back(formatString);
        }

    unitName = "u:" + unqualifiedPers;
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPersistenceUnitByName(Utf8StringCR unitName, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper)
    {
    Utf8String alias;
    Utf8String name;
    ECClass::ParseClassName(alias, name, unitName);
    auto unit = nameToUnitMapper(alias, name);
    
    if (nullptr == GetSchema().GetUnitsContext().LookupUnit(unitName.c_str()))
        {
        LOG.errorv("On KoQ '%s' persistence unit with name '%s' could not be located", GetFullName().c_str(), unitName.c_str());
        return ECObjectsStatus::Error;
        }

    SetPersistenceUnit(*unit);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatsByString(Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(formatString.c_str(), ";", tokens);

    for (auto const& str : tokens) // str of the format {formatName}<{precision}>[overrides|label][...]...
        {
        ECObjectsStatus stat = AddPresentationFormatByString(str, nameToFormatMapper, nameToUnitMapper);
        if (ECObjectsStatus::Success != stat)
            return stat;
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatByString(Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper)
    {
    Utf8String formatName;
    Nullable<int32_t> prec;
    bvector<Utf8String> unitNames;
    bvector<Nullable<Utf8String>> unitLabels;
    if (BentleyStatus::SUCCESS != Formatting::Format::ParseFormatString(formatName, prec, unitNames, unitLabels, formatString))
        {
        LOG.errorv("Failed to parse Presentation FormatString '%s' on KindOfQuantity '%s'", formatString.c_str(), GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    Utf8String alias;
    Utf8String unqualifiedName;
    ECClass::ParseClassName(alias, unqualifiedName, formatName);
    if (alias.empty())
        alias = GetSchema().GetAlias();

    auto format = nameToFormatMapper(alias, unqualifiedName);

    if (nullptr == format)
        {
        LOG.errorv("Format '%s' could not be looked up on KoQ '%s'", formatName.c_str(), GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (!unitNames.empty())
        {
        UnitAndLabelPairs units;
        int i = 0;
        for (const auto& u : unitNames)
            {
            if (alias.empty())
                alias = GetSchema().GetAlias();

            ECClass::ParseClassName(alias, unqualifiedName, u);
            auto unit = nameToUnitMapper(alias, unqualifiedName);
            if (nullptr == unit)
                {
                LOG.errorv("Presentation unit with name '%s' could not be looked up on KoQ '%s'", u.c_str(), GetFullName().c_str());
                return ECObjectsStatus::Error;
                }
            units.push_back(make_bpair(unit, (i < unitLabels.size() && unitLabels[i].IsValid()) ? unitLabels[i].Value().c_str() : nullptr));
            i++;
            }

        return AddPresentationFormat(*format, prec, &units);
        }

    return AddPresentationFormat(*format, prec); // no unit overrides
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::CreateOverrideString(Utf8StringR out, ECFormatCR parent, Nullable<int32_t> precisionOverride,  UnitAndLabelPairs const* unitsAndLabels) const
    {
    if (parent.IsOverride())
        {
        LOG.errorv("On KOQ '%s' cannot create an override using another override as a parent", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    out += parent.GetName();
    
    if (precisionOverride.IsValid())
        {
        out += "(";
        out += std::to_string(precisionOverride.Value()).c_str();
        out += ")";
        }

    if (nullptr != unitsAndLabels)
        {
        auto& input = *unitsAndLabels;
        if (input.size() > 4)
            {
            LOG.errorv("On KOQ '%s' cannot have more than 4 override units specified on a presentation format", GetFullName().c_str());
            return ECObjectsStatus::Error;
            }

        for(const auto& i : input)
            { 
            auto& unit = i.first;
            if (nullptr == unit)
                return ECObjectsStatus::Error;
            out += "[";
            out += unit->GetQualifiedName(GetSchema());

            if (nullptr != i.second) // We want to override a label
                {
                out += "|";
                out += i.second;
                }

            out += "]";
            }
        }
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatInternal(NamedFormat format)
    {
    if (format.HasCompositeMajorUnit() && !Units::Unit::AreCompatible(format.GetCompositeMajorUnit(), GetPersistenceUnit()))
        {
        LOG.errorv("On KoQ '%s' cannot add presentation format '%s' because its major unit is not compatible with this KoQ's persistence unit", GetFullName().c_str(), format.GetName().c_str());
        return ECObjectsStatus::Error;
        }
    m_presentationFormats.emplace_back(format);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
NamedFormatCP KindOfQuantity::GetOrCreateCachedPersistenceFormat() const
    {
    if (nullptr == m_persistenceUnit)
        return nullptr;

    if (m_persFormatCache.IsValid())
        {
        if (Units::Unit::AreEqual(m_persFormatCache.Value().GetCompositeMajorUnit(), m_persistenceUnit))
            return &m_persFormatCache.Value();
        }
    m_persFormatCache = NamedFormat(oldDefaultFormatName);
    m_persFormatCache.ValueR().SetNumericSpec(Formatting::NumericFormatSpec::DefaultFormat());
    auto comp = Formatting::CompositeValueSpec(*m_persistenceUnit);
    comp.SetMajorLabel(m_persistenceUnit->GetDisplayLabel().c_str());
    m_persFormatCache.ValueR().SetCompositeSpec(comp);
    return &m_persFormatCache.Value();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormat(ECFormatCR parent, Nullable<int32_t> precisionOverride, UnitAndLabelPairs const* unitsAndLabels, bool isDefault)
    {
    if (parent.IsOverride())
        {
        LOG.errorv("On KOQ '%s' cannot create an override using another override as a parent", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if ((nullptr != unitsAndLabels) && unitsAndLabels->size() > 0)
        {
        const auto maj = GetPersistenceUnit();
        for (const auto& u: *unitsAndLabels)
            {
            if(nullptr == maj || nullptr == u.first || !Units::Unit::AreCompatible(maj, u.first))
                {
                LOG.errorv("On KOQ '%s' all unit overrides must be compatible with each other and must exist", GetFullName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        }

    if (parent.HasCompositeMajorUnit() && nullptr != m_persistenceUnit && !ECUnit::AreCompatible(parent.GetCompositeMajorUnit(), m_persistenceUnit))
        {
        LOG.errorv("On KOQ '%s' cannot have a format with a major unit that is incompatible with KOQ's persistence unit", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (parent.HasCompositeMajorUnit() && HasPresentationFormats())
        {
        for (auto const& format : m_presentationFormats)
            {
            if (format.HasCompositeMajorUnit() && !ECUnit::AreCompatible(parent.GetCompositeMajorUnit(), format.GetCompositeMajorUnit()))
                {
                LOG.errorv("On KOQ '%s' cannot add a format that has a major unit that is incompatible with other formats in this KOQ namely '%s'", GetFullName().c_str(), format.GetName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        }

    Utf8String out;
    CreateOverrideString(out, parent, precisionOverride, unitsAndLabels);

    NamedFormatP nfp = nullptr;
    if (isDefault)
        nfp = m_presentationFormats.emplace(m_presentationFormats.begin(), out, &parent);
    else
        {
        m_presentationFormats.emplace_back(out, &parent);
        nfp = &m_presentationFormats.back();
        }
    if (precisionOverride.IsValid())
        {
        if (Formatting::PresentationType::Fractional == nfp->GetPresentationType())
            {
            Formatting::FractionalPrecision prec;
            if (!Formatting::Utils::FractionalPrecisionByDenominator(prec, precisionOverride.Value()))
                {
                LOG.errorv("On KOQ '%s' %d is not a valid fractional precision override value", GetFullName().c_str(), precisionOverride.Value());
                return ECObjectsStatus::Error;
                }
            nfp->GetNumericSpecP()->SetPrecision(prec);
            }
        else
            {
            Formatting::DecimalPrecision prec;
            if (!Formatting::Utils::GetDecimalPrecisionByInt(prec, precisionOverride.Value()))
                {
                LOG.errorv("On KOQ '%s' %d is not a valid decimal precision override value", GetFullName().c_str(), precisionOverride.Value());
                return ECObjectsStatus::Error;
                }
            nfp->GetNumericSpecP()->SetPrecision(prec);
            }
        }
    if ((nullptr == unitsAndLabels) || (0 == unitsAndLabels->size()))
        return ECObjectsStatus::Success;

    auto& input = *unitsAndLabels;
    auto comp = nfp->GetCompositeSpecP();
    bvector<Units::UnitCP> newUnits;

    for (int i = 0; i < input.size(); ++i)
        newUnits.push_back(input[i].first);

    bvector<Units::UnitCP> compUnits;
    if (nullptr != comp)
        {
        if (comp->HasMajorUnit())
            compUnits.push_back(comp->GetMajorUnit());
        if (comp->HasMiddleUnit())
            compUnits.push_back(comp->GetMiddleUnit());
        if (comp->HasMinorUnit())
            compUnits.push_back(comp->GetMinorUnit());
        if (comp->HasSubUnit())
            compUnits.push_back(comp->GetSubUnit());
        }

    //Validate compUnits against overrides
    if(!compUnits.empty())
        {
        if (compUnits.size() != newUnits.size())
            {
            LOG.errorv("On KOQ '%s' cannot override a different number of units than already exists on a format", GetFullName().c_str());
            return ECObjectsStatus::Error;
            }

        for (int i = 0; i < input.size(); ++i)
            {
            if (!Units::Unit::AreEqual(newUnits[i], compUnits[i]))
                {
                LOG.errorv("On KOQ '%s' cannot change UOM in an override when one is already defined by the format", GetFullName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        }
    else
        {
        Formatting::CompositeValueSpec localComp;
        bool compStatus = Formatting::CompositeValueSpec::CreateCompositeSpec(localComp, newUnits);
        if (!compStatus || !nfp->SetCompositeSpec(localComp))
            {
            LOG.errorv("On KOQ '%s' failed to set composite spec", GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        comp = nfp->GetCompositeSpecP();
        }

    switch (input.size())
        {
        case 4:
            if (nullptr != input[3].second)
                comp->SetSubLabel(input[3].second);
        case 3:
            if (nullptr != input[2].second)
                comp->SetMinorLabel(input[2].second);
        case 2:
            if (nullptr != input[1].second)
                comp->SetMiddleLabel(input[1].second);
        case 1:
            if (nullptr != input[0].second)
                comp->SetMajorLabel(input[0].second);
        }
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatSingleUnitOverride(ECFormatCR parent, Nullable<int32_t> precisionOverride, ECUnitCP inputUnitOverride, Utf8CP labelOverride, bool isDefault)
    {
    UnitAndLabelPairs units;
    if (nullptr == inputUnitOverride && nullptr != labelOverride)
        {
        if (parent.HasCompositeMajorUnit())
            inputUnitOverride = (ECUnitCP)parent.GetCompositeMajorUnit();
        else
            return ECObjectsStatus::Error;
        }
    if (nullptr != inputUnitOverride)
        { 
        units = UnitAndLabelPairs();
        units.push_back(make_bpair(inputUnitOverride, labelOverride));
        }

    return AddPresentationFormat(parent, precisionOverride, &units, isDefault);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  06/2017
//---------------------------------------------------------------------------------------
Json::Value KindOfQuantity::GetPresentationsJson() const
    {
    Json::Value arrayObj(Json::arrayValue);

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
    if (presentationUnits.size() > 0)
        {
        for (NamedFormatCR format : presentationUnits)
            {
            if (format.IsProblem())
                continue;

            arrayObj.append(format.GetName());
            }
        }
    return arrayObj;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                David.Fox-Rabinovitz      08/2017
//---------------------------------------------------------------------------------------
PhenomenonCP KindOfQuantity::GetPhenomenon() const
    {
    BEU::UnitCP un = GetPersistenceUnit();
    return (nullptr == un) ? nullptr : static_cast<PhenomenonCP>(un->GetPhenomenon());
    }

END_BENTLEY_ECOBJECT_NAMESPACE
