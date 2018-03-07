/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECUnit.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8StringCR ECUnit::GetFullName() const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();

    return m_fullName;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8String ECUnit::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an Unit name with an alias unless the schema containing the Unit is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  Unit: %s\n ECSchema containing Unit: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR ECUnit::GetDisplayLabel() const 
    {
    return GetSchema().GetLocalizedStrings().GetUnitDisplayLabel(*this, GetInvariantDisplayLabel());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR ECUnit::GetInvariantDisplayLabel() const 
    {
    if(GetIsDisplayLabelDefined())
        return Units::Unit::GetInvariantLabel();
    else
        return GetName();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR ECUnit::GetDescription() const 
    {
    return GetSchema().GetLocalizedStrings().GetUnitDescription(*this, m_description);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadXml(BeXmlNodeR unitNode, ECSchemaReadContextR context)
    {
    // Read the specific properties
    Utf8CP nodeName = unitNode.GetName();
    ECSchemaElementType unitType = ECSchemaElementType::Unit;
    if (0 == strcmp(UNIT_ELEMENT, nodeName))
        unitType = ECSchemaElementType::Unit;
    else if (0 == strcmp(INVERTED_UNIT_ELEMENT, nodeName))
        unitType = ECSchemaElementType::InvertedUnit;
    else if (0 == strcmp(CONSTANT_ELEMENT, nodeName))
        unitType = ECSchemaElementType::Constant;
    else
        BeAssert(false);

    // Read the common properties shared by all Unit Types.
    // Deserialize label, description, Phenomenon, UnitSystem

    if (ECSchemaElementType::InvertedUnit != unitType)
        {
        // Read Phenomenon
        Utf8String qualifiedPhenomName;
        if(BEXML_Success != unitNode.GetAttributeStringValue(qualifiedPhenomName, PHENOMENON_NAME_ATTRIBUTE) || Utf8String::IsNullOrEmpty(qualifiedPhenomName.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), PHENOMENON_NAME_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        Utf8String phenomAlias;
        Utf8String phenomName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(phenomAlias, phenomName, qualifiedPhenomName))
            {
            LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be parsed.",
                GetName().c_str(), PHENOMENON_NAME_ATTRIBUTE, qualifiedPhenomName.c_str());

            return SchemaReadStatus::InvalidECSchemaXml;
            }

        PhenomenonP phenom;
        if (phenomAlias.empty())
            phenom = GetSchemaR().GetPhenomenonP(phenomName.c_str());
        else
            {
            ECSchemaP resolvedPhenomSchema = const_cast<ECSchemaP> (GetSchemaR().GetSchemaByAliasP(phenomAlias.c_str()));
            if (nullptr == resolvedPhenomSchema)
                {
                LOG.errorv("Invalid ECSchemaXML: The ECUnit '%s' contains a %s element with the alias '%s' that can not be resolved to a referenced schema.",
                    GetName().c_str(), PHENOMENON_NAME_ATTRIBUTE, phenomAlias.c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }

            phenom = resolvedPhenomSchema->GetPhenomenonP(phenomName.c_str());
            }

        if (nullptr == phenom)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s attribute with the value '%s' that can not be resolved to a Phenomenon named '%s' in the ECSchema '%s' or any of its references.",
                GetName().c_str(), PHENOMENON_NAME_ATTRIBUTE, qualifiedPhenomName.c_str(), phenomName.c_str(), GetSchema().GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        SetPhenomenon(*phenom);
        }

    // Read UnitSystem

    Utf8String qualifiedSystemName;
    if(BEXML_Success != unitNode.GetAttributeStringValue(qualifiedSystemName, UNIT_SYSTEM_NAME_ATTRIBUTE) || Utf8String::IsNullOrEmpty(qualifiedSystemName.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), UNIT_SYSTEM_NAME_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String systemAlias;
    Utf8String systemName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(systemAlias, systemName, qualifiedSystemName))
        {
        LOG.errorv("Invalid ECSchemaXML: The ECUnit '%s' contains a %s element with the value '%s' that can not be parsed.",
            GetName().c_str(), UNIT_SYSTEM_NAME_ATTRIBUTE, qualifiedSystemName.c_str());

        return SchemaReadStatus::InvalidECSchemaXml;
        }

    UnitSystemCP system;
    if (systemAlias.empty())
        system = GetSchema().GetUnitSystemCP(systemName.c_str());
    else
        {
        ECSchemaCP resolvedSystemSchema = GetSchema().GetSchemaByAliasP(systemAlias);
        if (nullptr == resolvedSystemSchema)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECUnit '%s' contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                GetName().c_str(), UNIT_SYSTEM_NAME_ATTRIBUTE, systemAlias.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        system = resolvedSystemSchema->GetUnitSystemCP(systemName.c_str());
        }

    if (nullptr == system)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECUnit '%s' contains a %s attribute with the value '%s' that can not be resolved to an UnitSystem named '%s' in the ECSchema '%s' or any of its references.",
            GetName().c_str(), UNIT_SYSTEM_NAME_ATTRIBUTE, qualifiedSystemName.c_str(), systemName.c_str(), GetSchema().GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetSystem(*system);

    // Read optional properties
    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    // Read the specific properties
    switch(unitType)
        {
        case ECSchemaElementType::Unit:
            return ReadStandardUnitXml(unitNode, context);
        case ECSchemaElementType::InvertedUnit:
            return ReadInvertedUnitXml(unitNode, context);
        case ECSchemaElementType::Constant:
            return ReadConstantXml(unitNode, context);
        default:
            return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::InvalidECSchemaXml;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadStandardUnitXml(BeXmlNodeR unitNode, ECSchemaReadContextR context)
    {
    Utf8String definition;
    if(BEXML_Success != unitNode.GetAttributeStringValue(definition, DEFINITION_ATTRIBUTE) || Utf8String::IsNullOrEmpty(definition.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (SUCCESS != SetDefinition(definition.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The Phenomenon %s contains an invalid %s attribute", GetName().c_str(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    double numerator;
    auto xmlStatus = unitNode.GetAttributeDoubleValue(numerator, NUMERATOR_ATTRIBUTE);
    if (BeXmlStatus::BEXML_AttributeNotFound == xmlStatus) 
        numerator = 1.0;
    else if (BeXmlStatus::BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetNumerator(numerator);

    double denominator;
    xmlStatus = unitNode.GetAttributeDoubleValue(denominator, DENOMINATOR_ATTRIBUTE);
    if (BeXmlStatus::BEXML_AttributeNotFound == xmlStatus) 
        denominator = 1.0;
    else if (BeXmlStatus::BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetDenominator(denominator);

    double offset;
    xmlStatus = unitNode.GetAttributeDoubleValue(offset, OFFSET_ATTRIBUTE);
    if (BeXmlStatus::BEXML_AttributeNotFound == xmlStatus)
        offset = 0.0;
    else if (BeXmlStatus::BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetOffset(offset);

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadInvertedUnitXml(BeXmlNodeR unitNode, ECSchemaReadContextR context)
    {
    Utf8String qualifiedInvertsUnitName;
    if(BEXML_Success != unitNode.GetAttributeStringValue(qualifiedInvertsUnitName, INVERTS_UNIT_ATTRIBUTE) || Utf8String::IsNullOrEmpty(qualifiedInvertsUnitName.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), INVERTS_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String parsedUnitAlias;
    Utf8String parsedUnitName;
    ECClass::ParseClassName(parsedUnitAlias, parsedUnitName, qualifiedInvertsUnitName);

    ECUnitCP ptrUnit;
    if (parsedUnitAlias.empty())
        ptrUnit = GetSchema().GetUnitCP(qualifiedInvertsUnitName.c_str());
    else
        {
        ECSchemaCP resolvedSchema = GetSchema().GetSchemaByAliasP(parsedUnitAlias);
        if (nullptr == resolvedSchema)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECUnit '%s' contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                GetName().c_str(), INVERTS_UNIT_ATTRIBUTE, parsedUnitAlias.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        ptrUnit = resolvedSchema->GetUnitCP(parsedUnitName.c_str());
        }

    if(nullptr == ptrUnit)
        {
        LOG.errorv("Invalid ECSchemaXML: The InvertedUnit '%s' contains a unit %s attribute with the value '%s' that can not be resolved to an unit named '%s' in the ECSchema '%s' or any of its references.",
            GetName().c_str(), INVERTS_UNIT_ATTRIBUTE, qualifiedInvertsUnitName.c_str(), parsedUnitName.c_str(), GetSchema().GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetParent(*ptrUnit);

    SetPhenomenon(const_cast<PhenomenonR> (*ptrUnit->GetPhenomenon()));

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadConstantXml(BeXmlNodeR unitNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status;
    auto checkRequiredAttribute = [&unitNode, &status](Utf8StringR value, Utf8CP attribute) -> decltype(auto)
        {
        if(BEXML_Success != unitNode.GetAttributeStringValue(value, attribute) || Utf8String::IsNullOrEmpty(value.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), attribute);
            status = SchemaReadStatus::InvalidECSchemaXml;
            }
        else
            status = SchemaReadStatus::Success;
        };

    Utf8String definition;
    if(BEXML_Success != unitNode.GetAttributeStringValue(definition, DEFINITION_ATTRIBUTE) || Utf8String::IsNullOrEmpty(definition.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", unitNode.GetName(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (SUCCESS != SetDefinition(definition.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The Phenomenon %s contains an invalid %s attribute", GetName().c_str(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    double numerator;
    auto xmlStatus = unitNode.GetAttributeDoubleValue(numerator, NUMERATOR_ATTRIBUTE);
    if (BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetNumerator(numerator);

    double denominator;
    xmlStatus = unitNode.GetAttributeDoubleValue(denominator, DENOMINATOR_ATTRIBUTE);
    if (BeXmlStatus::BEXML_AttributeNotFound == xmlStatus) 
        denominator = 1.0;
    else if (BeXmlStatus::BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetDenominator(denominator);

    SetConstant(true);

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteInvertedUnitXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const 
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = INVERTED_UNIT_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(INVERTS_UNIT_ATTRIBUTE, ((ECN::ECUnitCP)GetParent())->GetQualifiedName(GetSchema()).c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    if (GetIsDescriptionDefined())
        xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());

    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, GetUnitSystem()->GetQualifiedName(GetSchema()).c_str());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteConstantXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const 
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = CONSTANT_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    xmlWriter.WriteElementStart(elementName);
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());

    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    if (GetIsDescriptionDefined())
        xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());

    xmlWriter.WriteAttribute(PHENOMENON_NAME_ATTRIBUTE, GetPhenomenon()->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, GetUnitSystem()->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(DEFINITION_ATTRIBUTE, GetDefinition().c_str());
    xmlWriter.WriteAttribute(NUMERATOR_ATTRIBUTE, GetNumerator());
    xmlWriter.WriteAttribute(DENOMINATOR_ATTRIBUTE, GetDenominator());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = UNIT_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    xmlWriter.WriteElementStart(elementName);
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());

    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    if (GetIsDescriptionDefined())
        xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());

    xmlWriter.WriteAttribute(PHENOMENON_NAME_ATTRIBUTE, GetPhenomenon()->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, GetUnitSystem()->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(DEFINITION_ATTRIBUTE, GetDefinition().c_str());
    xmlWriter.WriteAttribute(NUMERATOR_ATTRIBUTE, GetNumerator());
    xmlWriter.WriteAttribute(DENOMINATOR_ATTRIBUTE, GetDenominator());

    if (HasOffset())
        xmlWriter.WriteAttribute(OFFSET_ATTRIBUTE, GetOffset());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteInvertedUnitJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteInvertedUnitJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteConstantJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteConstantJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = UNIT_ELEMENT;
    outValue[DEFINITION_ATTRIBUTE] = GetDefinition();
    outValue[NUMERATOR_ATTRIBUTE] = GetNumerator();
    outValue[DENOMINATOR_ATTRIBUTE] = GetDenominator();
    outValue[PHENOMENON_NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::PhenomenonCP)GetPhenomenon());
    outValue[UNIT_SYSTEM_NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::UnitSystemCP)GetUnitSystem());

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();
    if (HasOffset())
        outValue[OFFSET_ATTRIBUTE] = GetOffset();

    return SchemaWriteStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteInvertedUnitJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const 
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = INVERTED_UNIT_ELEMENT;
    outValue[INVERTS_UNIT_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::ECUnitCP)GetParent());
    outValue[UNIT_SYSTEM_NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::UnitSystemCP)GetUnitSystem());

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return SchemaWriteStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus ECUnit::WriteConstantJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const 
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = CONSTANT_ELEMENT;
    outValue[PHENOMENON_NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::PhenomenonCP)GetPhenomenon());
    outValue[UNIT_SYSTEM_NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECN::UnitSystemCP)GetUnitSystem());
    outValue[DEFINITION_ATTRIBUTE] = GetDefinition();
    outValue[NUMERATOR_ATTRIBUTE] = GetNumerator();
    outValue[DENOMINATOR_ATTRIBUTE] = GetDenominator();

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
