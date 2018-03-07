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
bool KindOfQuantity::SetPersistenceUnit(Utf8StringCR fusDescriptor)
    {
    ECUnitCP unit;
    Formatting::NamedFormatSpecCP nfs;
    auto status = ParseFUSDescriptor(unit, nfs, fusDescriptor.c_str(), *this);
    if (ECObjectsStatus::Success != status)
        return false;

    Formatting::FormatUnitSet persFUS(nfs, unit);

    if (persFUS.HasProblem() || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(persFUS.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_persistenceFUS = persFUS;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                03/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::SetPersistenceUnit(ECUnitCR unit, Utf8CP format)
    {
    if (unit.IsConstant())
        return false;

    auto fus = Formatting::FormatUnitSet(&unit, format);
    if (fus.HasProblem() || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(fus.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;
    
    m_persistenceFUS = fus;
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool KindOfQuantity::SetPersistenceUnit(ECUnitCR unit, Formatting::NamedFormatSpecCP format)
    {
    if (unit.IsConstant())
        return false;

    auto fus = Formatting::FormatUnitSet(format, &unit);
    if (fus.HasProblem() || (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(fus.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;
    
    m_persistenceFUS = fus;
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool KindOfQuantity::SetDefaultPresentationUnit(Utf8StringCR fusDescriptor)
    {
    ECUnitCP unit;
    Formatting::NamedFormatSpecCP nfs;
    auto status = ParseFUSDescriptor(unit, nfs, fusDescriptor.c_str(), *this);
    if (ECObjectsStatus::Success != status)
        return false;

    Formatting::FormatUnitSet presFUS(nfs, unit);

    if (presFUS.HasProblem() ||
        ((nullptr != presFUS.GetUnit()) && presFUS.GetUnit()->IsConstant()))
        return false;
    m_presentationFUS.insert(m_presentationFUS.begin(), presFUS);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::AddPresentationUnit(Utf8StringCR fusDescriptor)
    {
    ECUnitCP unit;
    Formatting::NamedFormatSpecCP nfs;
    auto status = ParseFUSDescriptor(unit, nfs, fusDescriptor.c_str(), *this);
    if (ECObjectsStatus::Success != status)
        return false;

    Formatting::FormatUnitSet presFUS(nfs, unit);

    if (presFUS.HasProblem() ||
        ((nullptr != presFUS.GetUnit()) && presFUS.GetUnit()->IsConstant()) ||
        (!m_persistenceFUS.HasProblem() && !Units::Unit::AreCompatible(presFUS.GetUnit(), m_persistenceFUS.GetUnit())) ||
        (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(presFUS.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_presentationFUS.push_back(presFUS);
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
bool KindOfQuantity::AddPresentationUnit(ECUnitCR unit, Utf8CP format)
    {
    if (unit.IsConstant())
        return false;

    Formatting::FormatUnitSet fus(&unit, format);
    if (fus.HasProblem() ||
        (!m_persistenceFUS.HasProblem() && !Units::Unit::AreCompatible(fus.GetUnit(), m_persistenceFUS.GetUnit())) ||
        (!GetDefaultPresentationUnit().HasProblem() && !Units::Unit::AreCompatible(fus.GetUnit(), GetDefaultPresentationUnit().GetUnit())))
        return false;

    m_presentationFUS.push_back(fus);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void KindOfQuantity::RemovePresentationUnit(Formatting::FormatUnitSetCR presentationFUS)
    {
    for (auto itor = m_presentationFUS.begin(); itor != m_presentationFUS.end(); itor++)
        if (Units::Unit::AreEqual(itor->GetUnit(), presentationFUS.GetUnit()))
            m_presentationFUS.erase(itor);
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

    if (GetPersistenceUnit().HasProblem())
        {
        LOG.errorv("Failed to write schema because persistance FUS for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), GetPersistenceUnit().GetProblemDescription().c_str());
        return SchemaWriteStatus::FailedToSaveXml;
        }

    auto getUnitNameFromVersion = [this, &ecXmlVersion](ECUnitCP unit) -> Utf8String {
        if (GetSchema().GetECVersion() == ecXmlVersion)
            return unit->GetQualifiedName(GetSchema());

        // EC3.0 and EC3.1
        return Units::UnitNameMappings::TryGetNewNameFromECName(unit->GetFullName().c_str());
    };

    Utf8String persistenceUnitString = getUnitNameFromVersion((ECUnitCP)GetPersistenceUnit().GetUnit());
    if (persistenceUnitString.empty())
        {
        // LOG.errorv("", );
        return SchemaWriteStatus::FailedToSaveXml;
        }

    xmlWriter.WriteAttribute(PERSISTENCE_UNIT_ATTRIBUTE, persistenceUnitString.c_str());

    double relError = GetRelativeError();
    xmlWriter.WriteAttribute(ECXML_RELATIVE_ERROR_ATTRIBUTE, relError);

    bvector<Formatting::FormatUnitSet> const& presentationUnits = GetPresentationUnitList();
    if (presentationUnits.size() > 0)
        {
        Utf8String presentationUnitString;
        Utf8String presUnit;
        bool first = true;
        for(Formatting::FormatUnitSetCR fus : presentationUnits)
            {
            if (fus.HasProblem())
                {
                LOG.errorv("Failed to write schema because presentation FUS for KindOfQuantity '%s' has problem: '%s'.", GetName().c_str(), fus.GetProblemDescription().c_str());
                return SchemaWriteStatus::FailedToSaveXml;
                }

            presUnit = getUnitNameFromVersion((ECUnitCP)fus.GetUnit());
            if (presUnit.empty())
                {
                // LOG.errorv("Failed to write schema becasue presentation FUS for KindOfQuantity '%s' has problem: '%s'.", );
                return SchemaWriteStatus::FailedToSaveXml;
                }

            if (!first)
                presentationUnitString += ";";
            presentationUnitString += presUnit;
            presentationUnitString += "(";
            presentationUnitString += fus.GetNamedFormatSpec()->GetName();
            presentationUnitString += ")";
            first = false;
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

    outValue[PERSISTENCE_UNIT_ATTRIBUTE] = ECJsonUtilities::FormatUnitSetToUnitFormatJson(GetPersistenceUnit(), *this);

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
            presentationUnitArr.append(ECJsonUtilities::FormatUnitSetToUnitFormatJson(fus, *this));
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

    // Read Persistence FUS

    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(value, PERSISTENCE_UNIT_ATTRIBUTE) || Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    ECUnitCP persUnit;
    Formatting::NamedFormatSpecCP persNFS;
    ECObjectsStatus status = ParseFUSDescriptor(persUnit, persNFS, value.c_str(), *this, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor());
    if (ECObjectsStatus::Success != status)
        return SchemaReadStatus::InvalidECSchemaXml; // log messages are in the ParseFUSDescriptor

    if ((nullptr != persUnit) && persUnit->IsConstant())
        { 
        LOG.errorv("Persistence FormatUnitSet: '%s' on KindOfQuanity '%s' has a Constant as a persistence unit", value.c_str(), GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    m_persistenceFUS = Formatting::FormatUnitSet(persNFS, persUnit);

    if (m_persistenceFUS.HasProblem())
        LOG.warningv("Persistence FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                     value.c_str(), GetName(), m_persistenceFUS.GetProblemDescription().c_str());

    // Read Presentation FUS'

    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        {
        bvector<Utf8String> presentationUnits;
        BeStringUtilities::Split(value.c_str(), ";", presentationUnits);
        for(auto const& presValue : presentationUnits)
            {
            ECUnitCP presUnit;
            Formatting::NamedFormatSpecCP presNFS;
            status = ParseFUSDescriptor(presUnit, presNFS, presValue.c_str(), *this, GetSchema().GetOriginalECXmlVersionMajor(), GetSchema().GetOriginalECXmlVersionMinor());
            
            if (ECObjectsStatus::Success != status)
                return SchemaReadStatus::InvalidECSchemaXml;

            if (nullptr != presUnit && presUnit->IsConstant())
                { 
                LOG.errorv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has constant as a presentation unit", presValue.c_str(), GetFullName().c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }

            Formatting::FormatUnitSet presFUS(presNFS, presUnit);

            if (presFUS.HasProblem())
                LOG.warningv("Presentation FormatUnitSet: '%s' on KindOfQuantity '%s' has problem '%s'.  Continuing to load but schema will not pass validation.",
                    value.c_str(), GetName(), presFUS.GetProblemDescription().c_str());

            m_presentationFUS.push_back(presFUS);
            }
        }
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::ParseFUSDescriptor(ECUnitCP& unit, Formatting::NamedFormatSpecCP& nfs, Utf8CP descriptor, KindOfQuantityR koq, Nullable<uint32_t> ecXmlMajorVersion, Nullable<uint32_t> ecXmlMinorVersion)
    {
    if (ecXmlMajorVersion.IsNull())
        ecXmlMajorVersion = 3;

    if (ecXmlMinorVersion.IsNull())
        ecXmlMinorVersion = 2;

    bool xmlLessThan32 = (3 == ecXmlMajorVersion.Value() && 2 > ecXmlMinorVersion.Value());
    bool xmlLessThanOrEqual32 = (3 == ecXmlMajorVersion.Value() && 2 <= ecXmlMinorVersion.Value());

    Utf8String unitName;
    Utf8String format;
    Formatting::FormatUnitSet::ParseUnitFormatDescriptor(unitName, format, descriptor);

    nfs = nullptr;
    if (Utf8String::IsNullOrEmpty(format.c_str()))
        // Need to keep the default without a Unit for backwards compatibility.
        nfs = Formatting::StdFormatSet::FindFormatSpec("DefaultRealU");
    else
        {
        nfs = Formatting::StdFormatSet::FindFormatSpec(format.c_str());
        if (nullptr == nfs)
            {
            if (xmlLessThanOrEqual32)
                {
                LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid format, '%s'.",
                    descriptor, koq.GetFullName().c_str(), format.c_str());
                return ECObjectsStatus::Error;
                }
            
            // Assuming since there was previously a format that it should contain the Unit with it.
            nfs = Formatting::StdFormatSet::FindFormatSpec("DefaultRealU");
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

        auto unitsSchema = StandardUnitsHelper::GetSchema();
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

    unit = koq.GetSchema().LookupUnit(unitName.c_str());

    if (nullptr == unit)
        {
        LOG.errorv("FormatUnitSet '%s' on KindOfQuantity '%s' has an invalid Unit, '%s'.",
            descriptor, koq.GetFullName().c_str(), unitName.c_str());
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                David.Fox-Rabinovitz      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Formatting::FormatUnitSetCP KindOfQuantity::GetPresentationFUS(size_t indx) const
    {
    BeAssert(indx >= m_presentationFUS.size());
    if (indx >= m_presentationFUS.size())
        return nullptr;

    return &m_presentationFUS[indx];
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

//---------------------------------------------------------------------------------------
// @bsimethod                                David.Fox-Rabinovitz      08/2017
//---------------------------------------------------------------------------------------
BEU::T_UnitSynonymVector* KindOfQuantity::GetSynonymVector() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? nullptr : php->GetSynonymVector();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                David.Fox-Rabinovitz      08/2017
//---------------------------------------------------------------------------------------
size_t KindOfQuantity::GetSynonymCount() const
    {
    BEU::PhenomenonCP php = GetPhenomenon();
    return (nullptr == php) ? 0 : php->GetSynonymCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                David.Fox-Rabinovitz      08/2017
//---------------------------------------------------------------------------------------
BEU::PhenomenonCP KindOfQuantity::GetPhenomenon() const
    {
    BEU::UnitCP un = GetPersistenceUnit().GetUnit();
    BEU::PhenomenonCP php = (nullptr == un)? nullptr : un->GetPhenomenon();
    return php;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
