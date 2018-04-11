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
        return ECObjectsStatus::Error;
    if (HasPresentationFormats())
        {
        for (auto const& format : m_presentationFormats)
            {
            if (format.HasCompositeMajorUnit() && !ECUnit::AreCompatible(&unit, format.GetCompositeMajorUnit()))
                return ECObjectsStatus::Error;
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

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormatList();
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
                if (!format.HasComposite() || !format.GetCompositeSpec()->HasMajorLabel())
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
                Utf8String split = tokens[0]; // Need to drop unit and label overrides // TODO: Should drop or throw error when serializing to old format with overrides?
                Utf8CP mapped = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(split.c_str());
                mapped = Formatting::AliasMappings::TryGetAliasFromName(mapped);
                if (nullptr == mapped)
                    {
                    LOG.warningv("Dropping presentation format '%s' for KindOfQuantity '%s' because it could not be mapped to an old format", format.GetName().c_str(), GetFullName().c_str());
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
                presentationUnitString += format.GetName();
                first = false;
                }
            }
        auto presAttr = ecXmlVersion >= ECVersion::V3_2 ? PRESENTATION_FORMATS_ATTRIBUTE : PRESENTATION_UNITS_ATTRIBUTE;
        xmlWriter.WriteAttribute(presAttr, presentationUnitString.c_str());
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

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormatList();
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
            presentationUnitArr.append(format.GetParentFormat()->GetQualifiedName(GetSchema()));
            }
        outValue[PRESENTATION_FORMATS_ATTRIBUTE] = presentationUnitArr;
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
        return SchemaReadStatus::InvalidECSchemaXml;

    // Read Presentation Formats
    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_2) ? PRESENTATION_FORMATS_ATTRIBUTE : PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presValue : presentationUnits)
            {
            if (ECObjectsStatus::Success != ParsePresentationUnit(presValue.c_str(), context, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor()))
                return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePersistenceUnit(Utf8CP descriptor, ECSchemaReadContextP context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion)
    {
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion) && (nullptr != context);
    ECUnitCP unit = nullptr;
    Nullable<unsigned> prec = nullptr;
    ECFormatCP persistenceFormat = nullptr;
    if (xmlLessThan32)
        {
        Utf8String unitName;
        Utf8String formatName;
        Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);
        unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
        if (unitName.empty())
            {
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a unit '%s' that cannot be mapped to a Unit in the standard Units schema",
                descriptor, GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }
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

        Utf8CP mappedName;
        if (Utf8String::IsNullOrEmpty(formatName.c_str()))
            mappedName = formatName.c_str();
        else
            {
            mappedName = Formatting::AliasMappings::TryGetNameFromAlias(formatName.c_str());
            mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(mappedName);
            }

        if (nullptr == mappedName)
            {
            LOG.errorv("Failed to find format mapping with name '%s' in legacy format mappings", mappedName);
            return ECObjectsStatus::Error;
            }

        if (!Utf8String::IsNullOrEmpty(mappedName))
            {
            SchemaKey key = SchemaKey("Formats", 1, 0, 0);
            auto formatsSchema = context->LocateSchema(key, SchemaMatchType::Latest);

            if (!ECSchema::IsSchemaReferenced(GetSchema(), *formatsSchema))
                { 
                LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve format '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str(), mappedName);
                if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*formatsSchema))
                    {
                    LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
                    return ECObjectsStatus::Error;
                    }
                }

            bvector<Utf8String> unitNames;
            bvector<Nullable<Utf8String>> unitLabels; // TODO make these optional since we don't care about them here
            Formatting::Format::ParseFormatString(formatName, prec, unitNames, unitLabels, mappedName);
            persistenceFormat = GetSchema().LookupFormat(formatName.c_str());
            if (nullptr == persistenceFormat)
                {
                LOG.errorv("Failed to find format with name '%s' in standard formats schema", mappedName);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePresentationUnit(Utf8CP descriptor, ECSchemaReadContextR context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion)
    {
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion);

    if (xmlLessThan32)
        {
        ECFormatCP format;
        Utf8String unitName;
        Utf8String formatName;
        Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);
        SchemaKey key = SchemaKey("Formats", 1, 0, 0);
        auto formatsSchema = context.LocateSchema(key, SchemaMatchType::Latest);
        Utf8CP mappedName;
        if (Utf8String::IsNullOrEmpty(formatName.c_str()))
            mappedName = formatName.c_str();
        else
            {
            mappedName = Formatting::AliasMappings::TryGetNameFromAlias(formatName.c_str());
            mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(mappedName);
            }
        if (!ECSchema::IsSchemaReferenced(GetSchema(), *formatsSchema))
            { 
            LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve format '%s'.",
                formatsSchema->GetName().c_str(), GetSchema().GetName().c_str(), mappedName);
            if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*formatsSchema))
                {
                LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        Nullable<unsigned> precision = nullptr;
        if (!Utf8String::IsNullOrEmpty(mappedName))
            {
            Utf8String localformatName;

            bvector<Utf8String> localUnitNames;
            bvector<Nullable<Utf8String>> localUnitLabels;
            Formatting::Format::ParseFormatString(localformatName, precision, localUnitNames, localUnitLabels, mappedName);
            format = GetSchema().LookupFormat(localformatName.c_str());
            if (nullptr == format)
                {
                LOG.errorv("FormatString '%s' on KindOfQuantity '%s' has an invalid format, '%s'.", descriptor, GetFullName().c_str(), mappedName);
                return ECObjectsStatus::Error;
                }
            }
        else
            {
            // Assuming since there was previously a format that it should contain the Unit with it.
            format = formatsSchema->LookupFormat(Formatting::FormatConstant::DefaultFormatName());
            BeAssert(nullptr != format);
            LOG.warningv("Setting format to DefaultRealU for FormatUnitSet '%s' on KindOfQuantity '%s'.", descriptor, GetFullName().c_str());
            }

        unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());

        if (unitName.empty())
            {
            LOG.errorv("FormatString '%s' on KindOfQuantity '%s' has a unit '%s' that cannot be mapped to a Unit in the standard Units schema",
                descriptor, GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }

        Utf8String alias;
        ECClass::ParseClassName(alias, unitName, unitName);
        SchemaKey unitsKey("Units", 1, 0, 0);
        auto unitsSchema = context.LocateSchema(unitsKey, SchemaMatchType::Latest);

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

        ECUnitCP unit = unitsSchema->GetUnitsContext().LookupUnit(unitName.c_str());

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

        AddPresentationFormatSingleUnitOverride(*format, precision, format->HasCompositeMajorUnit() ? nullptr : unit);
        }
    else // >= 3.2
        {
        bvector<Utf8String> names;
        bvector<Nullable<Utf8String>> unitLabels;
        Nullable<unsigned> precision;
        Utf8String formatName;
        Formatting::Format::ParseFormatString(formatName, precision, names, unitLabels, descriptor);
        ECFormatCP format = GetSchema().LookupFormat(formatName.c_str());
        if (nullptr == format)
            {
            LOG.errorv("Failed to lookup format '%s' on koq '%s'", formatName.c_str(), GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        bvector<std::pair<ECUnitCP, Utf8CP>> unitsAndLabels;
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
                unitsAndLabels.push_back(std::make_pair(lookedUpUnit, localLabel));
                }
            else
                {
                LOG.errorv("On KOQ '%s' could not find unit '%s' being added as a presentation unit override", GetFullName().c_str(), name.c_str());
                return ECObjectsStatus::Error;
                }
            }
        AddPresentationFormat(*format, precision, unitsAndLabels);
        }
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::ParseFUSDescriptor(ECUnitCP& unit, Formatting::FormatCP& nfs, Utf8CP descriptor, KindOfQuantityR koq, ECSchemaReadContextP context, Nullable<uint32_t> ecXmlMajorVersion, Nullable<uint32_t> ecXmlMinorVersion)
    {
    if (ecXmlMajorVersion.IsNull())
        ecXmlMajorVersion = 3;

    if (ecXmlMinorVersion.IsNull())
        ecXmlMinorVersion = 2;

    bool xmlLessThan32 = (3 == ecXmlMajorVersion.Value() && 2 > ecXmlMinorVersion.Value()) && (nullptr != context);
    bool xmlLessThanOrEqual32 = (3 == ecXmlMajorVersion.Value() && 2 <= ecXmlMinorVersion.Value());

    Utf8String unitName;
    Utf8String format;
    Formatting::Format::ParseUnitFormatDescriptor(unitName, format, descriptor);

    // TODO: Needs work...

    nfs = nullptr;
    if (Utf8String::IsNullOrEmpty(format.c_str()))
        // Need to keep the default without a Unit for backwards compatibility.
        // nfs = formatSchema->GetFormatCP("DefaultRealU");
        return ECObjectsStatus::Error;
    else
        {
        // nfs = s_stdFmtSet.FindFormat(format.c_str());
        if (nullptr == nfs)
            {
            if (xmlLessThanOrEqual32)
                {
                LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid format, '%s'.",
                    descriptor, koq.GetFullName().c_str(), format.c_str());
                return ECObjectsStatus::Error;
                }
            
            // Assuming since there was previously a format that it should contain the Unit with it.
            // nfs = s_stdFmtSet.FindFormat("DefaultRealU");
            LOG.warningv("Setting format to DefaultRealU for FormatUnitSet '%s' on KindOfQuantity '%s'.",
                descriptor, koq.GetFullName().c_str());
            }
        }

    unit = nullptr;
    if (xmlLessThan32)
        {
        // unitName should be a newName at this point. Update it to ecName.
        unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
        if (unitName.empty())
            {
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a unit '%s' that cannot be mapped to a Unit in the standard Units schema",
                descriptor, koq.GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }

        Utf8String alias;
        Utf8String name;
        ECClass::ParseClassName(alias, name, unitName);

        SchemaKey key("Units", 1, 0, 0);
        auto unitsSchema = context->LocateSchema(key, SchemaMatchType::Latest);
        // The alias should be the Units schema name from the ECName Mappings
        BeAssert(unitsSchema->GetName().EqualsI(alias));

        unit = unitsSchema->GetUnitCP(name.c_str());
        if (nullptr == unit)
            {
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located in the standard Units schema.",
                descriptor, koq.GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }

        if (ECSchema::IsSchemaReferenced(koq.GetSchema(), *unitsSchema))
            return ECObjectsStatus::Success;

        LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve unit '%s'.",
            unitsSchema->GetName().c_str(), koq.GetSchema().GetName().c_str(), unitName.c_str());
        
        if (ECObjectsStatus::Success != koq.GetSchemaR().AddReferencedSchema(*unitsSchema))
            {
            LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", unitsSchema->GetName().c_str(), koq.GetSchema().GetName().c_str());
            return ECObjectsStatus::Error;
            }

        return ECObjectsStatus::Success;
        }

    unit = koq.GetSchema().GetUnitsContext().LookupUnit(unitName.c_str());

    if (nullptr == unit)
        {
        LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid Unit, '%s'.",
            descriptor, koq.GetFullName().c_str(), unitName.c_str());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::UpdateFUSDescriptor(Utf8String& updatedDescriptor, Utf8CP descriptor)
    {
    updatedDescriptor.clear();

    if (nullptr == descriptor)
        return ECObjectsStatus::NullPointerValue;

    Utf8String unitName;
    Utf8String format;
    Formatting::Format::ParseUnitFormatDescriptor(unitName, format, descriptor);

    if (unitName.size() <= 0)
        return ECObjectsStatus::InvalidUnitName;

    unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
    if (unitName.empty())
        return ECObjectsStatus::InvalidUnitName;

    Utf8String alias;
    Utf8String name;
    ECClass::ParseClassName(alias, name, unitName);

    ECUnitCP unit = StandardUnitsHelper::GetSchema()->GetUnitCP(name.c_str());
    if (nullptr == unit)
        return ECObjectsStatus::InvalidUnitName;

    updatedDescriptor
        .append("u:")
        .append(unit->GetName().c_str());

    if (format.size() > 0)
        {
        updatedDescriptor.append("(")
        .append(format.c_str())
        .append(")");
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::CreateOverrideString(Utf8StringR out, ECFormatCR parent, Nullable<uint32_t> precisionOverride,  Nullable<bvector<std::pair<ECUnitCP, Utf8CP>>> unitsAndLabels)
    {
    if (parent.IsOverride())
        {
        LOG.errorv("On KOQ '%s' cannot create an override using another override as a parent", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    out += parent.GetQualifiedName(GetSchema());
    
    if (precisionOverride.IsValid())
        {
        out += "<";
        out += std::to_string(precisionOverride.Value()).c_str();
        out += ">";
        }

    if (unitsAndLabels.IsValid())
        {
        auto& input = unitsAndLabels.ValueR();
        if (unitsAndLabels.Value().size() > 4)
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
            out += "|";

            if (nullptr != i.second)
                out += i.second;

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
    // TODO error checking. Using this for copyKindOFQuantity right now so it is guaranteed to be valid there since its
    // part of another kind of quanity already and has gone through error checking, but still should add it here.
    m_presentationFormats.emplace_back(format);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormat(ECFormatCR parent, Nullable<uint32_t> precisionOverride, Nullable<bvector<std::pair<ECUnitCP, Utf8CP>>> unitsAndLabels, bool isDefault)
    {
    if (parent.IsOverride())
        {
        LOG.errorv("On KOQ '%s' cannot create an override using another override as a parent", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (unitsAndLabels.IsValid() && unitsAndLabels.ValueR().size() > 0)
        {
        const auto maj = GetPersistenceUnit();
        for (const auto& u: unitsAndLabels.ValueR())
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
                LOG.errorv("On KOQ '%s' cannot add a format that has a major unit that is incompatible with other formats in this KOQ namely '%s'", GetFullName().c_str(), format.GetName());
                return ECObjectsStatus::Error;
                }
            }
        }

    Utf8String out;
    CreateOverrideString(out, parent, precisionOverride, unitsAndLabels);

    if (isDefault)
        m_presentationFormats.emplace(m_presentationFormats.begin(), out, &parent);
    else
        m_presentationFormats.emplace_back(out, &parent);

    auto nfp = &m_presentationFormats.back();
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
            nfp->GetNumericSpecP()->SetFractionalPrecision(prec);
            }
        else
            {
            Formatting::DecimalPrecision prec;
            if (!Formatting::Utils::DecimalPrecisionByIndex(prec, precisionOverride.Value()))
                {
                LOG.errorv("On KOQ '%s' %d is not a valid decimal precision override value", GetFullName().c_str(), precisionOverride.Value());
                return ECObjectsStatus::Error;
                }
            nfp->GetNumericSpecP()->SetDecimalPrecision(prec);
            }
        }
    if (!unitsAndLabels.IsValid() || (0 == unitsAndLabels.Value().size()))
        return ECObjectsStatus::Success;

    auto& input = unitsAndLabels.ValueR();
    auto comp = nfp->GetCompositeSpecP();
    bvector<Units::UnitCP> newUnits;

    for (int i = 0; i < input.size(); ++i)
        newUnits.push_back(input[i].first);

    bvector<Units::UnitCP> compUnits; // C++11 initializer list doesn't work?
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
        if (!nfp->SetCompositeSpec(Formatting::CompositeValueSpec(newUnits)))
            {
            LOG.errorv("On KOQ '%s' failed to set composite spec", GetFullName().c_str());
            return ECObjectsStatus::Error;
            }
        comp = nfp->GetCompositeSpecP();
        }

    for (int i = 0; i < input.size(); ++i)
        {
        if (nullptr != input[i].second)
            comp->SetUnitLabel(input[i].second, i);
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatSingleUnitOverride(ECFormatCR parent, Nullable<uint32_t> precisionOverride, ECUnitCP inputUnitOverride, Utf8CP labelOverride, bool isDefault)
    {
    Nullable<bvector<std::pair<ECUnitCP, Utf8CP>>> units = nullptr;
    if (nullptr != inputUnitOverride)
        { 
        units= bvector<std::pair<ECUnitCP, Utf8CP>>();
        units.ValueR().push_back(std::make_pair(inputUnitOverride, labelOverride));
        }

    return AddPresentationFormat(parent, precisionOverride, units, isDefault);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  06/2017
//---------------------------------------------------------------------------------------
Json::Value KindOfQuantity::GetPresentationsJson() const
    {
    Json::Value arrayObj(Json::arrayValue);

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormatList();
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
