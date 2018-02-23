/*--------------------------------------------------------------------------------------+
|
|     $Source: src/KindOfQuantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

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

    if ((nullptr != m_persistenceFUS.GetUnit()) && m_persistenceFUS.GetUnit()->IsConstant())
        {
        LOG.errorv("Validation Error - KindOfQuantity '%s' persistence FormatUnitSet unit is a constant", GetFullName().c_str());
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

        if (nullptr != presFUS.GetUnit() && presFUS.GetUnit()->IsConstant())
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation FormatUnitSet unit is a constant", GetFullName().c_str());
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
    if (persistenceFUS.HasProblem() || ((nullptr != persistenceFUS.GetUnit()) && persistenceFUS.GetUnit()->IsConstant()) || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(persistenceFUS.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_persistenceFUS = persistenceFUS;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::AddPresentationUnit(Formatting::FormatUnitSet presentationFUS)
    {
    if (presentationFUS.HasProblem() ||((nullptr != presentationFUS.GetUnit()) && presentationFUS.GetUnit()->IsConstant()) || (!m_persistenceFUS.HasProblem() && !Units::Unit::AreCompatible(presentationFUS.GetUnit(), m_persistenceFUS.GetUnit()))
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
    xmlWriter.WriteAttribute(ECXML_RELATIVE_ERROR_ATTRIBUTE, relError);

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
    
    //WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus KindOfQuantity::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema children
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_CHILD_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = KIND_OF_QUANTITY_ELEMENT;

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

    outValue[ECJSON_RELATIVE_ERROR_ATTRIBUTE] = GetRelativeError();

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
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", kindOfQuantityNode.GetName(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    auto concatenateName = [](Utf8StringCR schemaName, Utf8StringCR unit, Utf8StringCR format)
        {
        auto totalLen = schemaName.length() + unit.length() + format.length() + 4;
        auto buf = Utf8String();
        buf.resize(totalLen);
        BeStringUtilities::Snprintf((Utf8P)buf.c_str(), totalLen, "%s:%s(%s)", schemaName.c_str(), unit.c_str(), format.c_str());
        return buf;
        };

    auto getResolvedName = [&concatenateName](ECSchemaCR schema, Utf8StringCR value)
        {
        Utf8String resolvedName;
        Utf8String unitName;
        Utf8String format;
        Formatting::FormatUnitSet::ParseUnitFormatDescriptor(unitName, format, value.c_str());
        if(schema.GetOriginalECXmlVersionMajor() <= 3 && schema.GetOriginalECXmlVersionMinor() < 2)
            resolvedName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
        else
            { 
            Utf8String alias;
            Utf8String name;
            ECClass::ParseClassName(alias, name, unitName);
            if(alias.empty() || alias == schema.GetAlias())
                {
                if(schema.GetUnitCP(name.c_str()))
                    resolvedName = concatenateName(schema.GetName(), name, format);
                }
            else
                {
                for (const auto& s : schema.GetReferencedSchemas())
                    {
                    auto const& refSchema = s.second;
                    if((refSchema->GetAlias() == alias) && (nullptr != refSchema->GetUnitCP(name.c_str())))
                        {
                        resolvedName = concatenateName(refSchema->GetName(), name, format);
                        break;
                        }
                    }
                }
            }
        if(resolvedName.empty())
            {
            // HACK because we don't have base units in a schema yet, can't fully qualify them
            if(Units::UnitRegistry::Instance().HasUnit(unitName.c_str()))
                resolvedName = value.c_str();
            }
        return resolvedName;
        };

    auto resolvedName = getResolvedName(GetSchema(), value);
    if(resolvedName.empty())
        return SchemaReadStatus::InvalidECSchemaXml;

    Formatting::FormatUnitSet persistenceFUS(resolvedName.c_str());

    if (persistenceFUS.HasProblem())
        LOG.warningv("Persistence FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                     value.c_str(), kindOfQuantityNode.GetName(), persistenceFUS.GetProblemDescription().c_str());

    if ((nullptr != persistenceFUS.GetUnit()) && persistenceFUS.GetUnit()->IsConstant())
        { 
        LOG.errorv("Persistence FormatUnitSet: '%s' on KindOfQuanity '%s' has a Constant as a persistence unit", value.c_str(), kindOfQuantityNode.GetName());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetPersistenceUnit(persistenceFUS);

    double relError;
    if (BEXML_Success != kindOfQuantityNode.GetAttributeDoubleValue(relError, ECXML_RELATIVE_ERROR_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", kindOfQuantityNode.GetName(), ECXML_RELATIVE_ERROR_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetRelativeError(relError);

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presUnit : presentationUnits)
            {
            auto resolvedName = getResolvedName(GetSchema(), presUnit);
            if(resolvedName.empty())
                return SchemaReadStatus::InvalidECSchemaXml;

            Formatting::FormatUnitSet presFUS(resolvedName.c_str());
            if (presFUS.HasProblem())
                LOG.warningv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                             presUnit.c_str(), kindOfQuantityNode.GetName(), presFUS.GetProblemDescription().c_str());

            if (nullptr != presFUS.GetUnit() && presFUS.GetUnit()->IsConstant())
                { 
                LOG.errorv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has constant as a presentation unit", presUnit.c_str(), kindOfQuantityNode.GetName());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            m_presentationFUS.push_back(presFUS);
            }
        }
    return SchemaReadStatus::Success;
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
