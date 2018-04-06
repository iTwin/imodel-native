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
                format.HasCompositeInputUnit() &&
                !Units::Unit::AreCompatible(format.GetCompositeInputUnit(), m_persistenceUnit))
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
            if (format.HasCompositeInputUnit() && !ECUnit::AreCompatible(&unit, format.GetCompositeInputUnit()))
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
                if (!format.HasComposite() || !format.GetCompositeSpec()->HasInputUnit())
                    {
                    LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
                    continue;
                    }
                auto hasValidName = getUnitNameFromVersion((ECUnitCP)format.GetCompositeSpec()->GetInputUnit(), presUnit);
                if(!hasValidName)
                    {
                    LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
                    continue;
                    }

                if (!first)
                    presentationUnitString += ";";
                presentationUnitString += presUnit;
                presentationUnitString += "(";
                presentationUnitString += format.GetParentFormat()->GetQualifiedName(GetSchema());
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

    ECUnitCP persUnit = nullptr;
    ECFormatCP persFormat = nullptr;
    if (ECObjectsStatus::Success != ParsePersistenceUnit(persUnit, persFormat, value.c_str(), &context, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor()))
        {
        LOG.errorv("Failed to parse persistence unit '%s'", value.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    BeAssert(nullptr != persUnit);
    if (persUnit->IsConstant())
        { 
        LOG.errorv("Persistence FormatUnitSet: '%s' on KindOfQuanity '%s' has a Constant as a persistence unit", value.c_str(), GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    m_persistenceUnit = persUnit;

    auto presAttr = GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_2) ? PRESENTATION_FORMATS_ATTRIBUTE : PRESENTATION_UNITS_ATTRIBUTE;
    // Read Presentation Formats
    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, presAttr))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presValue : presentationUnits)
            {
            ECUnitCP inputUnit = nullptr;
            ECFormatCP presFormat = nullptr;
            if (ECObjectsStatus::Success != ParsePresentationUnit(inputUnit, presFormat, presValue.c_str(), context, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor()))
                {
                LOG.errorv("Failed to parse presentation format '%s' on KindOfQuantity '%s'", presValue.c_str(), GetName().c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            
            BeAssert(nullptr != inputUnit);
            if (inputUnit->IsConstant())
                { 
                LOG.errorv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has constant as a presentation unit", presValue.c_str(), GetFullName().c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            // The input unit might need to be overridden for < ec3.2 where presentation units are required. This should be nullptr if version >= 3.2
            if (ECObjectsStatus::Success != AddPresentationFormat(*presFormat, nullptr, inputUnit))
                return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    if (nullptr != persFormat)
        m_presentationFormats.push_back(*persFormat);
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePersistenceUnit(ECUnitCP& unit, ECFormatCP& format, Utf8CP descriptor, ECSchemaReadContextP context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion)
    {
    unit = nullptr;
    format = nullptr;
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion) && (nullptr != context);

    if (xmlLessThan32)
        {
        Utf8String unitName;
        Utf8String formatName;
        Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);

        Utf8CP mappedName;
        if (Utf8String::IsNullOrEmpty(formatName.c_str()))
            mappedName = formatName.c_str();
        else
            mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(formatName.c_str());

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
                LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve format '%s'.",
                    formatsSchema->GetName().c_str(), GetSchema().GetName().c_str(), mappedName);
                if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*formatsSchema))
                    {
                    LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
                    return ECObjectsStatus::Error;
                    }
                }

            format = GetSchema().LookupFormat(mappedName);
            if (nullptr == format)
                {
                LOG.errorv("Failed to find format with name '%s' in standard formats schema", mappedName);
                return ECObjectsStatus::Error;
                }
            }

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
        }
    else
        unit = GetSchema().GetUnitsContext().LookupUnit(descriptor);

    if (nullptr == unit)
        {
        LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located",
            descriptor, GetFullName().c_str(), descriptor);
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParsePresentationUnit(ECUnitCP& unit, ECFormatCP& format, Utf8CP descriptor, ECSchemaReadContextR context, uint32_t ecXmlMajorVersion, uint32_t ecXmlMinorVersion)
    {
    format = nullptr;
    unit = nullptr;
    bool xmlLessThan32 = (3 == ecXmlMajorVersion && 2 > ecXmlMinorVersion);

    if (xmlLessThan32)
        {
        Utf8String unitName;
        Utf8String formatName;
        Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);
        SchemaKey key = SchemaKey("Formats", 1, 0, 0);
        auto formatsSchema = context.LocateSchema(key, SchemaMatchType::Latest);
        Utf8CP mappedName;
        if (Utf8String::IsNullOrEmpty(formatName.c_str()))
            mappedName = formatName.c_str();
        else
            mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(formatName.c_str());
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

        if (!Utf8String::IsNullOrEmpty(mappedName))
            {
            format = GetSchema().LookupFormat(mappedName);
            if (nullptr == format)
                {
                    LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid format, '%s'.",
                        descriptor, GetFullName().c_str(), mappedName);
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
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a unit '%s' that cannot be mapped to a Unit in the standard Units schema",
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
        unit = unitsSchema->GetUnitsContext().LookupUnit(unitName.c_str());
        if (nullptr == unit)
            {
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has a Unit '%s' that could not be located in the standard Units schema.",
                descriptor, GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }
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
ECObjectsStatus KindOfQuantity::CreateOverrideString(Utf8StringR out, ECFormatCR parent, Nullable<uint32_t> precisionOverride, ECUnitCP inputUnitOverride, Utf8CP labelOverride)
    {
    if (parent.IsOverride())
        return ECObjectsStatus::Error;

    out += parent.GetName();
    
    if (precisionOverride.IsValid())
        {
        out += "<";
        out += std::to_string(precisionOverride.Value()).c_str();
        out += ">";
        }

    if (nullptr != inputUnitOverride || nullptr != labelOverride)
        {
        out += "[";

        if (nullptr != inputUnitOverride)
            {
            out += inputUnitOverride->GetQualifiedName(GetSchema());
            }

        out += "|";

        if (nullptr != labelOverride)
            out += labelOverride;

        out += "]";
        }
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormat(NamedFormat format)
    {
    // TODO error checking. Using this for copyKindOFQuantity right now so it is guaranteed to be valid there since its
    // part of another kind of quanity already and has gone through error checking, but still should add it here.
    m_presentationFormats.emplace_back(format);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormat(ECFormatCR parent, Nullable<uint32_t> precisionOverride, ECUnitCP inputUnitOverride, Utf8CP labelOverride, bool isDefault)
    {
    // TODO logging
    if (parent.IsOverride())
        return ECObjectsStatus::Error;

    if (parent.HasCompositeInputUnit() && nullptr != m_persistenceUnit && !ECUnit::AreCompatible(parent.GetCompositeInputUnit(), m_persistenceUnit))
        return ECObjectsStatus::Error;

    if (parent.HasCompositeInputUnit() && HasPresentationFormats())
        {
        for (auto const& format : m_presentationFormats)
            {
            if (format.HasCompositeInputUnit() && !ECUnit::AreCompatible(parent.GetCompositeInputUnit(), format.GetCompositeInputUnit()))
                return ECObjectsStatus::Error;
            }
        }

    Utf8String out;
    CreateOverrideString(out, parent, precisionOverride, inputUnitOverride, labelOverride);

    if (nullptr != inputUnitOverride && parent.HasCompositeInputUnit() && !ECUnit::AreCompatible(inputUnitOverride, parent.GetCompositeInputUnit()))
        return ECObjectsStatus::Error;

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
                return ECObjectsStatus::Error;
            nfp->GetNumericSpecP()->SetFractionalPrecision(prec);
            }
        else
            {
            Formatting::DecimalPrecision prec;
            if (!Formatting::Utils::DecimalPrecisionByIndex(prec, precisionOverride.Value()))
                return ECObjectsStatus::Error;
            nfp->GetNumericSpecP()->SetDecimalPrecision(prec);
            }
        }

    if (nullptr != inputUnitOverride)
        {
        if (nfp->HasComposite())
            nfp->GetCompositeSpecP()->SetInputUnit(inputUnitOverride);
        else
            {
            auto comp = Formatting::CompositeValueSpec();
            comp.SetInputUnit(inputUnitOverride);
            nfp->SetCompositeSpec(comp);
            }
        }

    if (nullptr != labelOverride)
        { 
        if (nfp->HasComposite())
            nfp->GetCompositeSpecP()->SetInputUnitLabel(labelOverride);
        else
            {
            auto comp = Formatting::CompositeValueSpec();
            comp.SetInputUnitLabel(labelOverride);
            nfp->SetCompositeSpec(comp);
            }
        }

    return ECObjectsStatus::Success;
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
