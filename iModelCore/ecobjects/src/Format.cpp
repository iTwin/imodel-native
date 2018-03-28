/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Format.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus Format::SetSchema(ECSchemaCR schema)
    {
    m_schema = &schema;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
Format::Format(ECSchemaCR schema, Utf8StringCR name) : NamedFormatSpec(name), m_schema(&schema), m_fullName(schema.GetName() + ":" + name) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR Format::GetFullName() const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();

    return m_fullName;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus Format::SetDisplayLabel(Utf8StringCR displayLabel)
    {
    T_Super::SetDisplayLabel(displayLabel.c_str());
    m_isDisplayLabelExplicitlyDefined = true;
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus Format::SetDescription(Utf8StringCR description)
    {
    T_Super::SetDescription(description.c_str());
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
Utf8String Format::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an Format name with an alias unless the schema containing the Format is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  Phenomenon: %s\n ECSchema containing Format: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus Format::ReadXml(BeXmlNodeR unitFormatNode, ECSchemaReadContextR context)
    {
    Utf8String value; // used by the macros.
    BeXmlStatus status;
    READ_OPTIONAL_XML_ATTRIBUTE(unitFormatNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(unitFormatNode, DESCRIPTION_ATTRIBUTE, this, Description)

    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();

    double roundFactor;
    status = unitFormatNode.GetAttributeDoubleValue(roundFactor, FORMAT_ROUND_FACTOR_ATTRIBUTE);
    if (BeXmlStatus::BEXML_Success == status)
        spec.SetRoundingFactor(roundFactor);
    else if (BeXmlStatus::BEXML_AttributeNotFound != status)
        {
        LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_ROUND_FACTOR_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String specType;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(specType, FORMAT_TYPE_ATTRIBUTE))
        {
        LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Formatting::PresentationType type;
    if (!Formatting::Utils::NameToPresentationType(type, specType.c_str()))
        {
        LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE, specType.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetPresentationType(type);

    if (Formatting::PresentationType::Scientific == type)
        {
        Utf8String scientificType;
        if (BEXML_Success != unitFormatNode.GetAttributeStringValue(scientificType, FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE))
            {
            LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        Formatting::ScientificType unitsScientificType;
        if (!Formatting::Utils::NameToScientificType(unitsScientificType, scientificType))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, scientificType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetScientificType(unitsScientificType);
        }

    if (Formatting::PresentationType::Station == type)
        {
        uint32_t stationSize;
        if (BEXML_Success != unitFormatNode.GetAttributeUInt32Value(stationSize, FORMAT_STATION_SIZE_ATTRIBUTE))
            {
            LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        if (stationSize < 1)
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetStationOffsetSize(stationSize);
        }

    Utf8String showSignName;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(showSignName, FORMAT_SIGN_OPTION_ATTRIBUTE))
        {        
        Formatting::ShowSignOption showSign;
        if (!Formatting::Utils::NameToSignOption(showSign, showSignName.c_str()))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_SIGN_OPTION_ATTRIBUTE, specType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetSignOption(showSign);
        }

    Utf8String formatTraits;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(formatTraits, FORMAT_TRAITS_ATTRIBUTE))
        { 
        if (!spec.SetFormatTraitsFromString(formatTraits))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TRAITS_ATTRIBUTE, specType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    uint32_t precision;
    status = unitFormatNode.GetAttributeUInt32Value(precision, FORMAT_PRECISION_ATTRIBUTE);
    if (BeXmlStatus::BEXML_Success != status && BeXmlStatus::BEXML_AttributeNotFound != status)
        { 
        LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
 
    if(type == Formatting::PresentationType::Fractional)
        {
        Formatting::FractionalPrecision unitsFractionalPrecision;
        if (!Formatting::Utils::FractionalPrecisionByDenominator(unitsFractionalPrecision, precision))
            {
            LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetFractionalPrecision(unitsFractionalPrecision);
        }
    else
        {
        Formatting::DecimalPrecision unitsDecimalPrecision;
        if (!Formatting::Utils::DecimalPrecisionByIndex(unitsDecimalPrecision, precision))
            {
            LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetDecimalPrecision(unitsDecimalPrecision);
        }

    Utf8String decimalSeparator;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(decimalSeparator, FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE))
        { 
        if (decimalSeparator.length() > 1 || decimalSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, decimalSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetDecimalSeparator(decimalSeparator.at(0));
        }

    Utf8String thousandSeparator;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(thousandSeparator, FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE))
        {
        if (thousandSeparator.length() > 1 || thousandSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, thousandSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetThousandSeparator(thousandSeparator.at(0));
        }
    
    Utf8String uomSeparator;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(uomSeparator, FORMAT_UOM_SEPARATOR_ATTRIBUTE))
        spec.SetUomSeparator(uomSeparator.c_str());

    Utf8String statSeparator;
    if (BeXmlStatus::BEXML_Success == unitFormatNode.GetAttributeStringValue(statSeparator, FORMAT_STAT_SEPARATOR_ATTRIBUTE))
        {
        if (statSeparator.length() > 1 || statSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STAT_SEPARATOR_ATTRIBUTE, statSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetStationSeparator(statSeparator.at(0));
        }   

    SetNumericSpec(spec);
    auto child = unitFormatNode.GetFirstChild();
    if (nullptr != unitFormatNode.GetFirstChild())
        {
        if(0 != BeStringUtilities::StricmpAscii("Composite", child->GetName()))
            {
            LOG.errorv("%s node '%s' contains an invalid child %", FORMAT_ELEMENT, GetFullName().c_str(), child->GetName());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        SchemaReadStatus compStatus = ReadCompositeSpecXml(*child, context);
        if (SchemaReadStatus::Success != compStatus)
            return compStatus;
        }

    return SchemaReadStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus Format::ReadCompositeSpecXml(BeXmlNodeR compositeNode, ECSchemaReadContextR context)
    {

    Utf8String inputUnit;
    if (BEXML_Success != compositeNode.GetAttributeStringValue(inputUnit, COMPOSITE_INPUT_UNIT_ATTRIBUTE))
        {
        LOG.errorv("%s node on %s doesn't contain a valid %s attribute which is required", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str(), COMPOSITE_INPUT_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    ECUnitCP unit = GetSchema().GetUnitsContext().LookupUnit(inputUnit.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("%s node's %s on %s could not be found", FORMAT_COMPOSITE_ELEMENT, COMPOSITE_INPUT_UNIT_ATTRIBUTE, GetFullName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    uint32_t numChildren = 0;
    bvector<ECUnitCP> units;
    bvector<Nullable<Utf8String>> labels;
    for (BeXmlNodeP childNode = compositeNode.GetFirstChild(); childNode != nullptr; childNode = childNode->GetNextSibling())
        {
        if (SchemaReadStatus::Success != ReadCompositeUnitXml(*childNode, context, units, labels))
            return SchemaReadStatus::InvalidECSchemaXml;
        if (++numChildren > 4)
            {
            LOG.errorv("%s node on %s has too many children units", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    BeAssert(units.size() == labels.size());
    if (units.size() == 0)
        {
        LOG.errorv("%s node on %s has no children units", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    bvector<Units::UnitCP> unitsVector(units.begin(), units.end());
    auto comp = Formatting::CompositeValueSpec(unitsVector);
    if (labels[0].IsValid())
        comp.SetMajorLabel(labels[0].Value());
    if (labels.size() > 1 && labels[1].IsValid())
        comp.SetMiddleLabel(labels[1].Value());
    if (labels.size() > 2 && labels[2].IsValid())
        comp.SetMinorLabel(labels[2].Value());
    if (labels.size() > 3 && labels[3].IsValid())
        comp.SetSubLabel(labels[3].Value());
    comp.SetInputUnit(unit);

    Utf8String spacer;
    if (BEXML_Success == compositeNode.GetAttributeStringValue(spacer, COMPOSITE_SPACER_ATTRIBUTE))
        comp.SetSpacer(spacer.c_str());

    SetCompositeSpec(comp);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus Format::ReadCompositeUnitXml(BeXmlNodeR unitNode, ECSchemaReadContextR context, bvector<ECUnitCP>& units, bvector<Nullable<Utf8String>>& labels)
    {
    Utf8String unitName;
    if (BEXML_Success != unitNode.GetContent(unitName))
        {
        LOG.errorv("%s node on %s is missing a unit value", FORMAT_COMPOSITE_UNIT_ELEMENT, GetFullName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    ECUnitCP unit = GetSchema().GetUnitsContext().LookupUnit(unitName.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("%s node on %s has invalid unit %s", FORMAT_COMPOSITE_UNIT_ELEMENT, GetFullName().c_str(), unitName.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    units.push_back(unit);

    Utf8String label;
    if (BEXML_Success == unitNode.GetAttributeStringValue(label, COMPOSITE_UNIT_LABEL_ATTRIBUTE))
        labels.push_back(label);
    else
        labels.push_back(nullptr);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus Format::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(FORMAT_ELEMENT);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    if (HasNumeric())
        { 
        auto nfs = GetNumericSpec();
        if (nfs->HasRoundingFactor())
            xmlWriter.WriteAttribute(FORMAT_ROUND_FACTOR_ATTRIBUTE, nfs->GetRoundingFactor());
        xmlWriter.WriteAttribute(FORMAT_TYPE_ATTRIBUTE, Formatting::Utils::PresentationTypeName(nfs->GetPresentationType()).c_str());
        if (nfs->HasSignOption())
            xmlWriter.WriteAttribute(FORMAT_SIGN_OPTION_ATTRIBUTE, Formatting::Utils::SignOptionName(nfs->GetSignOption()).c_str());
        if (nfs->HasFormatTraits())
            xmlWriter.WriteAttribute(FORMAT_TRAITS_ATTRIBUTE, nfs->GetFormatTraitsString().c_str());
        if (nfs->HasPrecision())
            { 
            xmlWriter.WriteAttribute(FORMAT_PRECISION_ATTRIBUTE, GetPresentationType() == Formatting::PresentationType::Fractional ? 
                static_cast<uint32_t>(pow(2u, static_cast<uint32_t>(nfs->GetFractionalPrecision()))) : 
                static_cast<uint32_t>(nfs->GetDecimalPrecision()));
            }
        if (Formatting::PresentationType::Scientific == GetPresentationType())
            xmlWriter.WriteAttribute(FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, Formatting::Utils::ScientificTypeName(nfs->GetScientificType()).c_str());
        if (nfs->HasDecimalSeparator())
            xmlWriter.WriteAttribute(FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, Utf8String(1, nfs->GetDecimalSeparator()).c_str());
        if (nfs->HasThousandsSeparator())
            xmlWriter.WriteAttribute(FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, Utf8String(1,nfs->GetThousandSeparator()).c_str());
        if (nfs->HasUomSeparator())
            xmlWriter.WriteAttribute(FORMAT_UOM_SEPARATOR_ATTRIBUTE, nfs->GetUomSeparator());
        if (nfs->HasStationSeparator())
            xmlWriter.WriteAttribute(FORMAT_STAT_SEPARATOR_ATTRIBUTE, Utf8String(1, nfs->GetStationSeparator()).c_str());
        if (Formatting::PresentationType::Station == GetPresentationType())
            xmlWriter.WriteAttribute(FORMAT_STATION_SIZE_ATTRIBUTE, nfs->GetStationOffsetSize());
        }
    if (HasComposite())
        {
        auto comp = GetCompositeSpec();
        xmlWriter.WriteElementStart(FORMAT_COMPOSITE_ELEMENT);
        if (comp->HasSpacer())
            xmlWriter.WriteAttribute(COMPOSITE_SPACER_ATTRIBUTE, comp->GetSpacer().c_str());
        if (comp->HasInputUnit())
            xmlWriter.WriteAttribute(COMPOSITE_INPUT_UNIT_ATTRIBUTE, ((ECUnitCP)comp->GetInputUnit())->GetQualifiedName(GetSchema()).c_str());
        auto writeUnit = [&](Nullable<Utf8String> label, Units::UnitCP unit)
            {
            xmlWriter.WriteElementStart(FORMAT_COMPOSITE_UNIT_ELEMENT);
            if(label.IsValid())
                xmlWriter.WriteAttribute(COMPOSITE_UNIT_LABEL_ATTRIBUTE, label.Value().c_str());
            xmlWriter.WriteText(((ECUnitCP)unit)->GetQualifiedName(GetSchema()).c_str());
            xmlWriter.WriteElementEnd();
            };
        if (HasCompositeMajorUnit())
            writeUnit(comp->HasMajorLabel() ? comp->GetMajorLabel() : nullptr, comp->GetMajorUnit());
        if (HasCompositeMiddleUnit())
            writeUnit(comp->HasMiddleLabel() ? comp->GetMiddleLabel() : nullptr, comp->GetMiddleUnit());
        if (HasCompositeMinorUnit())
            writeUnit(comp->HasMinorLabel() ? comp->GetMinorLabel() : nullptr, comp->GetMinorUnit());
        if (HasCompositeSubUnit())
            writeUnit(comp->HasSubLabel() ? comp->GetSubLabel() : nullptr, comp->GetSubUnit());
        xmlWriter.WriteElementEnd();
        }
    xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus Format::WriteJson(Json::Value & outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = FORMAT_ELEMENT;
    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (!Utf8String::IsNullOrEmpty(GetInvariantDescription().c_str()))
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();
    if (HasNumeric())
        {
        auto nfs = GetNumericSpec();
        if (nfs->HasRoundingFactor())
            outValue[FORMAT_ROUND_FACTOR_ATTRIBUTE] = nfs->GetRoundingFactor();
        outValue[FORMAT_TYPE_ATTRIBUTE] = Formatting::Utils::PresentationTypeName(nfs->GetPresentationType());
        if (nfs->HasSignOption())
            outValue[FORMAT_SIGN_OPTION_ATTRIBUTE] = Formatting::Utils::SignOptionName(nfs->GetSignOption());
        if (nfs->HasFormatTraits())
            outValue[FORMAT_TRAITS_ATTRIBUTE] = nfs->GetFormatTraitsString();
        if (nfs->HasPrecision())
            { 
            outValue[FORMAT_PRECISION_ATTRIBUTE] = GetPresentationType() == Formatting::PresentationType::Fractional ? 
                        static_cast<uint32_t>(pow(2u, static_cast<uint32_t>(nfs->GetFractionalPrecision()))) : 
                        static_cast<uint32_t>(nfs->GetDecimalPrecision());
            }
        if (Formatting::PresentationType::Scientific == GetPresentationType())
            outValue[FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE] = Formatting::Utils::ScientificTypeName(nfs->GetScientificType());
        if (nfs->HasDecimalSeparator())
            outValue[FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE] = Utf8String(1, nfs->GetDecimalSeparator()).c_str();
        if (nfs->HasThousandsSeparator())
            outValue[FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE] = Utf8String(1,nfs->GetThousandSeparator()).c_str();
        if (nfs->HasUomSeparator())
            outValue[FORMAT_UOM_SEPARATOR_ATTRIBUTE] = nfs->GetUomSeparator();
        if (nfs->HasStationSeparator())
            outValue[FORMAT_STAT_SEPARATOR_ATTRIBUTE] = Utf8String(1, nfs->GetStationSeparator()).c_str();
        if (Formatting::PresentationType::Station == GetPresentationType())
            outValue[FORMAT_STATION_SIZE_ATTRIBUTE] = nfs->GetStationOffsetSize();
        }
    
    if (HasComposite())
        {
        auto comp = GetCompositeSpec();
        auto& compElement = outValue[FORMAT_JSON_COMPOSITE_ELEMENT];
        if (comp->HasSpacer())
            compElement[COMPOSITE_SPACER_ATTRIBUTE] = comp->GetSpacer().c_str();
        if (comp->HasInputUnit())
            compElement[COMPOSITE_INPUT_UNIT_ATTRIBUTE] = ((ECUnitCP)comp->GetInputUnit())->GetQualifiedName(GetSchema()).c_str();
        auto writeUnit = [&](Nullable<Utf8String> label, Units::UnitCP unit, Json::Value& unitsArray)
            {
            auto& unitElement = unitsArray.append(Json::Value(Json::objectValue));
            if(label.IsValid())
               unitElement[COMPOSITE_UNIT_LABEL_ATTRIBUTE] = label.Value().c_str();
            unitElement[NAME_ATTRIBUTE] = ((ECUnitCP)unit)->GetQualifiedName(GetSchema()).c_str();
            };
        compElement[FORMAT_COMPOSITE_UNITS_ELEMENT] = Json::Value(Json::arrayValue);
        auto& unitArray = compElement[FORMAT_COMPOSITE_UNITS_ELEMENT];
        if (HasCompositeMajorUnit())
            writeUnit(comp->HasMajorLabel() ? comp->GetMajorLabel() : nullptr, comp->GetMajorUnit(), unitArray);
        if (HasCompositeMiddleUnit())
            writeUnit(comp->HasMiddleLabel() ? comp->GetMiddleLabel() : nullptr, comp->GetMiddleUnit(), unitArray);
        if (HasCompositeMinorUnit())
            writeUnit(comp->HasMinorLabel() ? comp->GetMinorLabel() : nullptr, comp->GetMinorUnit(), unitArray);
        if (HasCompositeSubUnit())
            writeUnit(comp->HasSubLabel() ? comp->GetSubLabel() : nullptr, comp->GetSubUnit(), unitArray);
        }
    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR Format::GetDisplayLabel() const
    {
    return GetSchema().GetLocalizedStrings().GetFormatDisplayLabel(*this, GetInvariantDisplayLabel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR Format::GetDescription() const
    {
    return GetSchema().GetLocalizedStrings().GetFormatDescription(*this, GetInvariantDescription());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus Format::WriteJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteJson(outValue, true, includeSchemaVersion);
    }

END_BENTLEY_ECOBJECT_NAMESPACE