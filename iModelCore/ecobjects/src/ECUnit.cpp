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
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
// static
ECUnitP ECUnit::_Create(Units::UnitSystemCR unitSystem, Units::PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant)
    {
    // Deconstruct the name. The format should be {SchemaName}.{UnitName}
    Utf8String schemaName;
    Utf8String unitName;
    ECClass::ParseClassName(schemaName, unitName, name);
    BeAssert(!schemaName.empty());

    // Check if it's a valid name here to avoid having to construct the ECUnit Unnecessarily
    if (!ECNameValidation::IsValidName(unitName.c_str()))
        {
        LOG.errorv("A Unit cannot be created with the name '%s' because it is not a valid ECName", unitName.c_str());
        return nullptr;
        }

    auto ptrUnit = new ECUnit(unitSystem, phenomenon, name, id, definition, numerator, denominator, offset, isConstant);
    if (nullptr == ptrUnit)
        return nullptr;

    ptrUnit->SetName(unitName.c_str());
    return ptrUnit;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
// static
ECUnitP ECUnit::_Create(Units::UnitCR parentUnit, Units::UnitSystemCR system, Utf8CP name, uint32_t id)
    {
    // Deconstruct the name. The format should be {SchemaName}.{UnitName}
    Utf8String schemaName;
    Utf8String unitName;
    ECClass::ParseClassName(schemaName, unitName, name);
    BeAssert(!schemaName.empty());

    // Check if it's a valid name here to avoid having to construct the ECUnit Unnecessarily
    if (!ECNameValidation::IsValidName(unitName.c_str()))
        {
        LOG.errorv("A Unit cannot be created with the name '%s' because it is not a valid ECName", unitName.c_str());
        return nullptr;
        }

    auto ptrUnit = new ECUnit(parentUnit, system, name, id);
    if (nullptr == ptrUnit)
        return nullptr;

    ptrUnit->SetName(unitName.c_str());
    return ptrUnit;
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
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus ECUnit::SetName(Utf8StringCR name)
    {
    if(!ECNameValidation::IsValidName(name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_unqualifiedName = name.c_str();
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadXml(ECUnitP& unit, BeXmlNodeR unitNode, ECSchemaCR schema, ECSchemaReadContextR context)
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

    Utf8String name;
    Utf8String definition;
    Utf8String phenomName;
    Utf8String unitSystemName;

    checkRequiredAttribute(name, TYPE_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(definition, DEFINITION_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(phenomName, PHENOMENON_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(unitSystemName, UNIT_SYSTEM_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    
    double numerator;
    double denominator;
    double offset;
    auto xmlStatus = unitNode.GetAttributeDoubleValue(numerator, NUMERATOR_ATTRIBUTE);
    if (BEXML_Success != xmlStatus) numerator = 1.0;
    xmlStatus = unitNode.GetAttributeDoubleValue(denominator, DENOMINATOR_ATTRIBUTE);
    if (BEXML_Success != xmlStatus) denominator = 1.0;
    xmlStatus = unitNode.GetAttributeDoubleValue(offset, OFFSET_ATTRIBUTE);
    if (BEXML_Success != xmlStatus) offset = 0.0;

    Utf8String parsedPhenomSchema;
    Utf8String parsedPhenomName;
    ECClass::ParseClassName(parsedPhenomSchema, parsedPhenomName, phenomName);

    Utf8String parsedSystemSchema;
    Utf8String parsedSystemName;
    ECClass::ParseClassName(parsedSystemSchema, parsedSystemName, unitSystemName);
    const auto& alias = schema.GetAlias();
    bool phenomOk = false;
    bool systemOk = false;
    PhenomenonCP ptrPhenom = schema.GetPhenomenonCP(parsedPhenomName.c_str());
    UnitSystemCP ptrSystem = schema.GetUnitSystemCP(parsedSystemName.c_str());

    if((parsedPhenomSchema.empty() || (alias == parsedPhenomSchema)) && (nullptr != ptrPhenom))
        {
        phenomOk = true;
        phenomName = schema.GetName() + ":" + parsedPhenomName;
        }

    if((parsedSystemSchema.empty() || (alias == parsedSystemSchema)) && (nullptr != ptrSystem))
        { 
        systemOk = true;
        unitSystemName = schema.GetName() + ":" + parsedSystemName;
        }
    if(!(phenomOk && systemOk))
        { 
        for(const auto& s : schema.GetReferencedSchemas())
            {
            PhenomenonCP localPhenomPtr;
            UnitSystemCP localSystemPtr;

            if(phenomOk && systemOk)
                break;
            
            if(!phenomOk && (s.second->GetAlias() == parsedPhenomSchema))
                { 
                localPhenomPtr = s.second->GetPhenomenonCP(parsedPhenomName.c_str());
                if(nullptr != localPhenomPtr)
                    { 
                    phenomOk = true;
                    phenomName = localPhenomPtr->GetFullName();
                    }
                }
            
            if(!systemOk && (s.second->GetAlias() == parsedSystemSchema))
                {
                localSystemPtr = s.second->GetUnitSystemCP(parsedSystemName.c_str());
                if(nullptr != localSystemPtr)
                    {
                    systemOk = true;
                    unitSystemName = localSystemPtr->GetFullName();
                    }
                }
            }
        }

    if(!(phenomOk && systemOk))
        return SchemaReadStatus::InvalidECSchemaXml;
       
    Utf8String fullName = schema.GetName() + ":" + name;
    unit = Units::UnitRegistry::Instance().AddUnit<ECUnit>(phenomName.c_str(), unitSystemName.c_str(), fullName.c_str(), definition.c_str(), numerator, denominator, offset);

    if (nullptr == unit)
        return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, DESCRIPTION_ATTRIBUTE, unit, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, unit, DisplayLabel)

    unit->SetSchema(schema);
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadInvertedUnitXml(ECUnitP& invertedUnit, BeXmlNodeR unitNode, ECSchemaCR schema, ECSchemaReadContextR context)
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

    Utf8String name;
    Utf8String invertsUnitName;
    Utf8String unitSystemName;

    checkRequiredAttribute(name, TYPE_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(unitSystemName, UNIT_SYSTEM_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(invertsUnitName, INVERTS_UNIT_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String parsedUnitSchema;
    Utf8String parsedUnitName;
    ECClass::ParseClassName(parsedUnitSchema, parsedUnitName, invertsUnitName);

    Utf8String parsedSystemSchema;
    Utf8String parsedSystemName;
    ECClass::ParseClassName(parsedSystemSchema, parsedSystemName, unitSystemName);

    const auto& alias = schema.GetAlias();

    bool unitOk = false;
    bool systemOk = false;
    ECUnitCP ptrUnit = schema.GetUnitCP(invertsUnitName.c_str());
    UnitSystemCP ptrSystem = schema.GetUnitSystemCP(parsedSystemName.c_str());

    if((parsedUnitSchema.empty() || (alias == parsedUnitSchema)) && (nullptr != ptrUnit))
        {
        unitOk = true;
        invertsUnitName = schema.GetName() + ":" + parsedUnitName;
        }

    if((parsedSystemSchema.empty() || (alias == parsedSystemSchema)) && (nullptr != ptrSystem))
        { 
        systemOk = true;
        unitSystemName = schema.GetName() + ":" + parsedSystemName;
        }

    if(!(unitOk && systemOk))
        { 
        for(const auto& s : schema.GetReferencedSchemas())
            {
            ECUnitCP localUnitPtr;
            UnitSystemCP localSystemPtr;

            if(unitOk && systemOk)
                break;

            if(!unitOk && (s.second->GetAlias() == parsedUnitSchema))
                { 
                localUnitPtr = s.second->GetUnitCP(parsedUnitName.c_str());
                if(nullptr != localUnitPtr)
                    { 
                    unitOk = true;
                    invertsUnitName = localUnitPtr->GetFullName();
                    }
                }

            if(!systemOk && (s.second->GetAlias() == parsedSystemSchema))
                {
                localSystemPtr = s.second->GetUnitSystemCP(parsedSystemName.c_str());
                if(nullptr != localSystemPtr)
                    {
                    systemOk = true;
                    unitSystemName = localSystemPtr->GetFullName();
                    }
                }
            }
        }

    if(!(unitOk && systemOk))
        return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String fullName = schema.GetName() + ":" + name;
    invertedUnit = Units::UnitRegistry::Instance().AddInvertedUnit<ECUnit>(invertsUnitName.c_str(), fullName.c_str(), unitSystemName.c_str());

    if (nullptr == invertedUnit)
        return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, DESCRIPTION_ATTRIBUTE, invertedUnit, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, invertedUnit, DisplayLabel)

    invertedUnit->SetSchema(schema);
    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus ECUnit::ReadConstantXml(ECUnitP& constant, BeXmlNodeR unitNode, ECSchemaCR schema, ECSchemaReadContextR context)
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

    Utf8String name;
    Utf8String definition;
    Utf8String phenomName;
    Utf8String unitSystemName;

    checkRequiredAttribute(name, TYPE_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(definition, DEFINITION_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(phenomName, PHENOMENON_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    checkRequiredAttribute(unitSystemName, UNIT_SYSTEM_NAME_ATTRIBUTE);
    if(status != SchemaReadStatus::Success) return SchemaReadStatus::InvalidECSchemaXml;
    
    double numerator;
    double denominator;
    auto xmlStatus = unitNode.GetAttributeDoubleValue(numerator, NUMERATOR_ATTRIBUTE);
    if (BEXML_Success != xmlStatus)
        return SchemaReadStatus::InvalidECSchemaXml;
    xmlStatus = unitNode.GetAttributeDoubleValue(denominator, DENOMINATOR_ATTRIBUTE);
    if (BEXML_Success != xmlStatus) denominator = 1.0;

    Utf8String parsedPhenomSchema;
    Utf8String parsedPhenomName;
    ECClass::ParseClassName(parsedPhenomSchema, parsedPhenomName, phenomName);

    Utf8String parsedSystemSchema;
    Utf8String parsedSystemName;
    ECClass::ParseClassName(parsedSystemSchema, parsedSystemName, unitSystemName);
    const auto& alias = schema.GetAlias();
    bool phenomOk = false;
    bool systemOk = false;
    PhenomenonCP ptrPhenom = schema.GetPhenomenonCP(parsedPhenomName.c_str());
    UnitSystemCP ptrSystem = schema.GetUnitSystemCP(parsedSystemName.c_str());

    if((parsedPhenomSchema.empty() || (alias == parsedPhenomSchema)) && (nullptr != ptrPhenom))
        {
        phenomOk = true;
        phenomName = schema.GetName() + ":" + parsedPhenomName;
        }

    if((parsedSystemSchema.empty() || (alias == parsedSystemSchema)) && (nullptr != ptrSystem))
        { 
        systemOk = true;
        unitSystemName = schema.GetName() + ":" + parsedSystemName;
        }

    if(!(phenomOk && systemOk))
        { 
        for(const auto& s : schema.GetReferencedSchemas())
            {
            PhenomenonCP localPhenomPtr;
            UnitSystemCP localSystemPtr;

            if(phenomOk && systemOk)
                break;
            
            if(!phenomOk && (s.second->GetAlias() == parsedPhenomSchema))
                { 
                localPhenomPtr = s.second->GetPhenomenonCP(parsedPhenomName.c_str());
                if(nullptr != localPhenomPtr)
                    { 
                    phenomOk = true;
                    phenomName = localPhenomPtr->GetFullName();
                    }
                }

            if(!systemOk && (s.second->GetAlias() == parsedSystemSchema))
                {
                localSystemPtr = s.second->GetUnitSystemCP(parsedSystemName.c_str());
                if(nullptr != localSystemPtr)
                    {
                    systemOk = true;
                    unitSystemName = localSystemPtr->GetFullName();
                    }
                }
            }
        }

    if(!(phenomOk && systemOk))
        return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String fullName = schema.GetName() + ":" + name;
    constant = Units::UnitRegistry::Instance().AddConstant<ECUnit>(phenomName.c_str(), unitSystemName.c_str(), fullName.c_str(), definition.c_str(), numerator, denominator);

    if (nullptr == constant)
        return SchemaReadStatus::InvalidECSchemaXml;

    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, DESCRIPTION_ATTRIBUTE, constant, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, constant, DisplayLabel)

    constant->SetSchema(schema);
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

    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, ((ECN::UnitSystemCP)GetUnitSystem())->GetQualifiedName(GetSchema()).c_str());

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

    xmlWriter.WriteAttribute(PHENOMENON_NAME_ATTRIBUTE, ((ECN::PhenomenonCP)GetPhenomenon())->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, ((ECN::UnitSystemCP)GetUnitSystem())->GetQualifiedName(GetSchema()).c_str());
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

    xmlWriter.WriteAttribute(PHENOMENON_NAME_ATTRIBUTE, ((ECN::PhenomenonCP)GetPhenomenon())->GetQualifiedName(GetSchema()).c_str());
    xmlWriter.WriteAttribute(UNIT_SYSTEM_NAME_ATTRIBUTE, ((ECN::UnitSystemCP)GetUnitSystem())->GetQualifiedName(GetSchema()).c_str());
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
