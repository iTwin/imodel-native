/*--------------------------------------------------------------------------------------+
|
|     $Source: src/KindOfQuantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include <regex>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus KindOfQuantity::SetName(Utf8CP name)
    {
    if (!ECNameValidation::IsValidName(name))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    return ECObjectsStatus::Success;
    }

//TODO: add string representation for FUS once we can call ToText on invalid FUS.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                03/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::Verify() const
    {
    bool isValid = true;
    if (m_persistenceFUS.HasProblem())
        {
        LOG.errorv("Validation Error - KindOfQuantity '%s' persistence FormatUnitSet has a problem: %s",
                   GetFullName().c_str(), m_persistenceFUS.GetProblemDescription().c_str());
        isValid = false;
        }
    
    for (Formatting::FormatUnitSetCR presFUS : m_presentationFUS)
        {
        if (presFUS.HasProblem())
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation FormatUnitSet has a problem: %s",
                GetFullName().c_str(), presFUS.GetProblemDescription().c_str());
            isValid = false;
            }
        else if ((!m_persistenceFUS.HasProblem() && !Units::Unit::AreCompatible(presFUS.GetUnit(), m_persistenceFUS.GetUnit())))
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation FormatUnitSet conflicts with the persistence FormatUnitSet %s.",
                GetFullName().c_str(), m_persistenceFUS.ToText(false).c_str());
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

//Following two methods need to be exported as the ValidatedName struct does not export its methods.
ECObjectsStatus KindOfQuantity::SetDisplayLabel(Utf8CP value) {m_validatedName.SetDisplayLabel(value); return ECObjectsStatus::Success;}

ECObjectsStatus KindOfQuantity::ParseName(Utf8StringR alias, Utf8StringR kindOfQuantityName, Utf8StringCR stringToParse)
    {
    if (0 == stringToParse.length())
        {
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = stringToParse.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        kindOfQuantityName = stringToParse;
        return ECObjectsStatus::Success;
        }

    if (stringToParse.length() == colonIndex + 1)
        {
        return ECObjectsStatus::ParseError;
        }

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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                08/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::SetPersistenceUnit(Formatting::FormatUnitSet persistenceFUS)
    {
    if (persistenceFUS.HasProblem() || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(persistenceFUS.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_persistenceFUS = persistenceFUS;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::AddPresentationUnit(Formatting::FormatUnitSet presentationFUS)
    {
    if (presentationFUS.HasProblem() || (!m_persistenceFUS.HasProblem() && !Units::Unit::AreCompatible(presentationFUS.GetUnit(), m_persistenceFUS.GetUnit()))
        || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(presentationFUS.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_presentationFUS.push_back(presentationFUS);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void KindOfQuantity::RemovePresentationUnit(Formatting::FormatUnitSet presentationFUS)
    {
    for (auto itor = m_presentationFUS.begin(); itor != m_presentationFUS.end(); itor++)
        if (Units::Unit::AreEqual(itor->GetUnit(), presentationFUS.GetUnit()))
            m_presentationFUS.erase(itor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus KindOfQuantity::WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_0)
        { //will only be serialized in 3.0 and later
        return SchemaWriteStatus::Success;
        }

    Utf8CP elementName = KIND_OF_QUANTITY_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    auto& displayLabel = GetInvariantDisplayLabel();
    if (!displayLabel.empty())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, displayLabel.c_str());

    if (GetPersistenceUnit().HasProblem())
        {
        LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), GetPersistenceUnit().GetProblemDescription().c_str());
        return SchemaWriteStatus::FailedToSaveXml;
        }
    Utf8String persistenceUnitString = GetPersistenceUnit().ToText(false);
    xmlWriter.WriteAttribute(PERSISTENCE_UNIT_ATTRIBUTE, persistenceUnitString.c_str());

    double relError = GetRelativeError();
    xmlWriter.WriteAttribute(RELATIVE_ERROR, relError);

    bvector<Formatting::FormatUnitSet> const& presentationUnits = GetPresentationUnitList();
    if (presentationUnits.size() > 0)
        {
        Utf8String presentationUnitString;
        bool first = true;
        for(Formatting::FormatUnitSetCR fus : presentationUnits)
            {
            if (fus.HasProblem())
                {
                LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), fus.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToSaveXml;
                }
            if (!first)
                presentationUnitString += ";";
            presentationUnitString += fus.ToText(false);
            first = false;
            }
        xmlWriter.WriteAttribute(PRESENTATION_UNITS_ATTRIBUTE, presentationUnitString.c_str());
        }

    xmlWriter.WriteElementEnd();
    return status;
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
        outValue[ECJSON_SCHEMA_ITEM_NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = KIND_OF_QUANTITY_ELEMENT;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (0 != GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // KindOfQuantity Properties
    if (GetPersistenceUnit().HasProblem())
        {
        LOG.errorv("Failed to write schema because persistance UNIT for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), GetPersistenceUnit().GetProblemDescription().c_str());
        return SchemaWriteStatus::FailedToCreateJson;
        }

    outValue[PERSISTENCE_UNIT_ATTRIBUTE] = ECJsonUtilities::FormatUnitSetToUnitFormatJson(GetPersistenceUnit());

    outValue[RELATIVE_ERROR] = GetRelativeError();

    bvector<Formatting::FormatUnitSet> const& presentationUnits = GetPresentationUnitList();
    if (0 != presentationUnits.size())
        {
        Json::Value presentationUnitArr(Json::ValueType::arrayValue);
        for (auto const& fus : presentationUnits)
            {
            if (fus.HasProblem())
                {
                LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), fus.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToCreateJson;
                }
            presentationUnitArr.append(ECJsonUtilities::FormatUnitSetToUnitFormatJson(fus));
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

    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, PERSISTENCE_UNIT_ATTRIBUTE) || Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    bool ecSchemaXmlGreaterThen31 = GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::V3_1);

    Formatting::FormatUnitSet persistenceFUS;
    bool invalidUnit = false;
    ECObjectsStatus status = ParseFUSDescriptor(persistenceFUS, invalidUnit, value.c_str(), *this, !ecSchemaXmlGreaterThen31);
    if (ECObjectsStatus::Success != status)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetPersistenceUnit(persistenceFUS);

    double relError;
    if (BEXML_Success != kindOfQuantityNode.GetAttributeDoubleValue(relError, RELATIVE_ERROR))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), RELATIVE_ERROR);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    SetRelativeError(relError);

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presValue : presentationUnits)
            {
            Formatting::FormatUnitSet presFUS;
            bool invalidUnit = false;
            if (!ecSchemaXmlGreaterThen31)
                {
                ECObjectsStatus status = ParseFUSDescriptor(presFUS, invalidUnit, presValue.c_str(), *this, true);
                if (ECObjectsStatus::Success != status || invalidUnit)
                    {
                    LOG.warningv("Presentation FormatUnitSet '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                        presValue.c_str(), GetFullName().c_str(), presFUS.GetProblemDescription().c_str());
                    }
                }
            else
                {
                Utf8String fusDesc;
                if (ECObjectsStatus::Success != FormatStringToFUSDescriptor(fusDesc, *this, presValue))
                    continue;

                if (ECObjectsStatus::Success != ParseFUSDescriptor(presFUS, invalidUnit, fusDesc.c_str(), *this, false))
                    {
                    LOG.warningv("Failed to parse FUS '%s' created from format string '%s' on KoQ '%s'", fusDesc.c_str(), presValue.c_str(), GetFullName().c_str());
                    continue;
                    }
                }
            m_presentationFUS.push_back(presFUS);
            }
        }
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::ParseFUSDescriptor(Formatting::FormatUnitSet& fus, bool& hasInvalidUnit, Utf8CP descriptor, KindOfQuantityCR koq, bool strict)
    {
    Utf8String unitName;
    Utf8String format;
    Formatting::FormatUnitSet::ParseUnitFormatDescriptor(unitName, format, descriptor);

    Formatting::NamedFormatSpecCP nfs = nullptr;
    if (Utf8String::IsNullOrEmpty(format.c_str()))
        // Need to keep the default without a Unit for backwards compatibility.
        nfs = Formatting::StdFormatSet::FindFormatSpec("DefaultReal");
    else
        {
        nfs = Formatting::StdFormatSet::FindFormatSpec(format.c_str());
        if (nullptr == nfs)
            {
            if (strict)
                {
                LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid FUS, '%s'.",
                    descriptor, koq.GetFullName().c_str(), format.c_str());
                return ECObjectsStatus::Error;
                }
            else
                {
                // Assuming since there was previously a format that it should contain the Unit with it.
                nfs = Formatting::StdFormatSet::FindFormatSpec("DefaultRealU");
                LOG.warningv("Setting format to DefaultRealU for FormatUnitSet '%s' on KindOfQuantity '%s'.",
                    descriptor, koq.GetFullName().c_str());
                }
            }
        }
    Utf8String name;
    Utf8String unqualifiedName;
    Utf8String alias;
    if(unitName.Contains(":"))
        {
        bvector<Utf8String> split;
        BeStringUtilities::Split(unitName.c_str(), ":", split);
        if(split.size() > 1)
            { 
            unqualifiedName = split[1];
            alias = split[0].EqualsI("u") ? "UNITS" : "";
            }
        }
    // HACK we dont' have the units schema yet to resolve the alias but want to support looking up qualified ec names
    // If the alias is u we just assume that it's for the Units schema.
    if(!Units::UnitRegistry::Instance().TryGetNewNameFromECName((alias + ":" + unqualifiedName).c_str(), name))
        name = unitName;
    
    Units::UnitCP unit;
    unit = Units::UnitRegistry::Instance().LookupUnit(name.c_str());
    if (nullptr == unit)
        {
        if (strict)
            {
            LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid Unit, '%s'.",
                descriptor, koq.GetFullName().c_str(), unitName.c_str());
            return ECObjectsStatus::Error;
            }

        unit = Units::UnitRegistry::Instance().AddDummyUnit(unitName.c_str());
        if (nullptr == unit)
            return ECObjectsStatus::Error;

        LOG.warningv("Adding dummy unit %s for FormatUnitSet '%s' on KindOfQuantity '%s'.",
            unitName.c_str(), descriptor, koq.GetFullName().c_str());
        hasInvalidUnit = true;
        }
    else if (!unit->IsValid())
        {
        LOG.warningv("FormatUnitSet '%s' on KindOfQuantity '%s' has a dummy unit, %s.",
            descriptor, koq.GetFullName().c_str(), unitName.c_str());
        hasInvalidUnit = true;
        }

    fus = Formatting::FormatUnitSet(nfs, unit);

    if (fus.HasProblem())
        LOG.warningv("FormatUnitSet '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
            descriptor, koq.GetFullName().c_str(), fus.GetProblemDescription().c_str());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Kyle.Abramowitz                  07/18
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus KindOfQuantity::FormatStringToFUSDescriptor(Utf8StringR fusDescriptor, KindOfQuantityCR koq, Utf8StringCR formatString)
    {
    Utf8CP mappedName = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(formatString.c_str());
                
    Utf8String formatName;
    Nullable<int32_t> prec;
    bvector<Utf8String> unitNames;
    bvector<Nullable<Utf8String>> unitLabels;
    if ((SUCCESS != ParseFormatString(formatName, prec, unitNames, unitLabels, formatString)) || formatName.empty())
        {
        LOG.warningv("Presentation Format String '%s' is invalid on KoQ '%s'", formatString.c_str(), koq.GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    if (Utf8String::IsNullOrEmpty(mappedName))
        {
        if (prec.IsValid())
            {
            Utf8String nameWithPrec;
            nameWithPrec = formatName;
            nameWithPrec
                .append("(")
                .append(std::to_string(prec.Value()).c_str())
                .append(")");
            Utf8String _alias;
            Utf8String name;
            ECClass::ParseClassName(_alias, name, nameWithPrec.c_str());     
            mappedName = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(("FORMATS:" + name).c_str());
            }
        if (nullptr == mappedName)
            {
            Utf8String _alias;
            Utf8String name;
            ECClass::ParseClassName(_alias, name, formatName.c_str());
            mappedName = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(("FORMATS:" + name).c_str());
            if (Utf8String::IsNullOrEmpty(mappedName))
                {
                LOG.warningv("Presentation Format String '%s' has a format that could not be mapped on KoQ '%s'", formatString.c_str(), koq.GetFullName().c_str());
                mappedName = "DefaultRealU";
                }
            }
        }
    Utf8String unitName = nullptr;
    if (!unitNames.empty())
        unitName = unitNames.front().c_str();
    else
        {
        auto format = Formatting::StdFormatSet::FindFormatSpec(mappedName);
        if (format != nullptr && format->HasComposite())
            {
            auto major = format->GetCompositeMajorUnit();
            if (major != nullptr)
                unitName = Utf8String("u:") + major->GetName();
            }
        }

    Utf8String presFusString;
    if (!Utf8String::IsNullOrEmpty(unitName.c_str()))
        {
        Utf8String newName;
        Utf8String alias;
        Utf8String className;
        ECClass::ParseClassName(alias, className, unitName);

        if (alias.Equals("u"))
            alias = "UNITS";

        if(!Units::UnitRegistry::Instance().TryGetNewNameFromECName((alias + ":" + className).c_str(), newName))
            {
            LOG.warningv("Failed to map unit '%s' from format string '%s' to a known unit on KoQ '%s'", unitName.c_str(), formatString.c_str(), koq.GetFullName().c_str());
            newName = koq.GetPersistenceUnit().GetUnitName();
            }
        presFusString.append(newName);
        }
    else
        presFusString.append(koq.GetPersistenceUnit().GetUnitName());

    presFusString
        .append("(")
        .append(mappedName)
        .append(")");
    fusDescriptor = presFusString;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Kyle.Abramowitz                  04/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
BentleyStatus KindOfQuantity::ParseFormatString(Utf8StringR formatName, Nullable<int32_t>& precision, bvector<Utf8String>& unitNames, bvector<Nullable<Utf8String>>& labels, Utf8StringCR formatString)
    {
    static size_t const precisionOverrideIndx = 0;
    static std::regex const rgx(R"REGEX(([\w,:]+)(\(([^\)]+)\))?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?)REGEX", std::regex::optimize);
    std::cmatch match;

    
    if (!std::regex_match(formatString.c_str(), match, rgx))
        return BentleyStatus::ERROR;

    size_t numOfRegexes = match.size();
    if (0 == numOfRegexes)
        return BentleyStatus::ERROR;

    // Handle format first to fail fast.
    if (!match[1].matched)
        {
        LOG.errorv("failed to map a format name to a Format");
        return BentleyStatus::ERROR;
        }
    // Get format name. Should always be the first match
    formatName = match[1].str().c_str();

    
    if (match[2].matched && match[3].matched)
        {
        Utf8String const overrideStr(match[2].str().c_str());
        // BeStringUtilities::Split ignores empty tokens. Since overrides are
        // position dependent, we actually need to count tokens even if they are
        // the empty string. This function does just that using ',' as a separator.
        bvector<Utf8String> overrides = [](Utf8StringCR str) -> bvector<Utf8String>
            {
            bvector<Utf8String> tokens;
            size_t prevPos = 1; // Initial position is the character directly after the opening '(' in the override string.
            size_t currPos;
            while (str.npos != (currPos = str.find_first_of(",)", prevPos)))
                {
                tokens.push_back(Utf8String(str.substr(prevPos, currPos - prevPos).c_str()).Trim());
                prevPos = currPos + 1;
                }
            return tokens;
            }(overrideStr);

        // It is considered an error to pass in a format string with empty
        // override brackets. If no overrides are needed, the user should instead
        // leave the brackets off altogether. As an example the incorrect format
        // string "SomeFormat<>" should instead be written as "SomeFormat".
        // Additionally, if a format would be specified using an override string
        // With no items actually overridden such as "SomeFormat<,,,,>" the string
        // is also erroneous.
        if (!overrideStr.empty()
            && overrides.end() == std::find_if_not(overrides.begin(), overrides.end(),
                [](Utf8StringCR ovrstr) -> bool
            {
            return std::all_of(ovrstr.begin(), ovrstr.end(), ::isspace);
            }))
            {
            LOG.errorv("override list must contain at least one override");
            return BentleyStatus::ERROR;
            }

        // The first override parameter overrides the default precision for the format.
        if (overrides.size() >= precisionOverrideIndx + 1) // Bail if the user didn't include this override.
            {
            if (!overrides[precisionOverrideIndx].empty())
                {
                uint64_t localPrecision;
                BentleyStatus status = BeStringUtilities::ParseUInt64(localPrecision, overrides[precisionOverrideIndx].c_str());
                if (BentleyStatus::SUCCESS != status)
                    {
                    LOG.errorv("Invalid FormatString: Failed to parse integer for precision override of FormatString, %s", formatString.c_str());
                    return status;
                    }
                precision = static_cast<int32_t>(localPrecision);
                }
            }
        }

    int i = 4;
    while (i < match.size())
        {
        if (!match[i].matched)
            break;
        // Unit override: required;
        if (!match[i+1].matched)
            return ERROR;
        unitNames.push_back(match[i+1].str().c_str());
        // Label override; optional
        if (match[i+2].matched) // matches a bar
            {
            labels.push_back(Nullable<Utf8String>(match[i+3].str().c_str()));
            }
        else // no label. ok
            labels.push_back(nullptr);
        i+=4;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Formatting::FormatUnitSetCP KindOfQuantity::GetPresentationFUS(size_t indx) const
    { 
    if (m_presentationFUS.size() > 0)
        {
        return (indx < m_presentationFUS.size())? &m_presentationFUS[indx] :  m_presentationFUS.begin();
        }
    else
        return &m_persistenceFUS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  12/2017
//---------------------------------------------------------------------------------------
Formatting::FormatUnitSetCP KindOfQuantity::GetPresentationFUS(Utf8CP inFusId, bool useAlias) const
    {
    if (!inFusId)
        return nullptr;
    Utf8String fusId(inFusId);

    for (Formatting::FormatUnitSetCR fus : m_presentationFUS)
        {
        if (fus.HasProblem())
            continue;

        if (fusId.Equals(fus.ToText(useAlias)))
            return &fus;
        }

    if (fusId.Equals(m_persistenceFUS.ToText(useAlias)))
        return &m_persistenceFUS;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String KindOfQuantity::GetPresentationFUSDescriptor(size_t indx, bool useAlias) const
    {
    Formatting::FormatUnitSetCP fusP;
    if (m_presentationFUS.size() > 0)
        {
        fusP = (indx < m_presentationFUS.size()) ? &m_presentationFUS[indx] : m_presentationFUS.begin();
        }
    else
        fusP = &m_persistenceFUS;
    return fusP->ToText(useAlias);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value KindOfQuantity::PresentationJson(BEU::QuantityCR qty, size_t indx, bool useAlias) const
    {
    Formatting::FormatUnitSetCP fusCP = GetPresentationFUS(indx);
    Json::Value jval = fusCP->FormatQuantityJson(qty, useAlias);
    return jval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  06/2017
//---------------------------------------------------------------------------------------
Json::Value KindOfQuantity::GetPresentationsJson(bool useAlias) const
    {
    Json::Value arrayObj(Json::arrayValue);

    bvector<Formatting::FormatUnitSet> const& presentationUnits = GetPresentationUnitList();
    if (presentationUnits.size() > 0)
        {
        for (Formatting::FormatUnitSetCR fus : presentationUnits)
            {
            if (fus.HasProblem())
                continue;

            arrayObj.append(fus.ToText(useAlias));
            }
        }
    return arrayObj;
    }
/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value KindOfQuantity::ToJson(bool useAlias) const
    {
    Json::Value jval;
    jval[Formatting::json_KOQName()] = m_fullName.c_str();
    jval[Formatting::json_schemaName()] = m_schema.GetName().c_str();
    jval[Formatting::json_persistFUS()] = m_persistenceFUS.ToJson(useAlias);
    jval[Formatting::json_relativeErr()] = GetPresentationsJson(useAlias);
    return jval;
    }

BEU::T_UnitSynonymVector* KindOfQuantity::GetSynonymVector() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? nullptr : php->GetSynonymVector();
    }

size_t KindOfQuantity::GetSynonymCount() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? 0 : php->GetSynonymCount();
    }

BEU::PhenomenonCP KindOfQuantity::GetPhenomenon() const
    {
    Formatting::FormatUnitSet fus = GetPersistenceUnit();
    BEU::UnitCP un = fus.GetUnit();
    BEU::PhenomenonCP php = (nullptr == un)? nullptr : un->GetPhenomenon();
    return php;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
