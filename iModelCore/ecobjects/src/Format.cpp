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
Format::Format(ECSchemaCR schema, Utf8StringCR name) : NamedFormatSpec((schema.GetName() + ":" + name).c_str()), m_schema(&schema), m_name(name) {}

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
    if (BeXmlStatus::BEXML_Success != status)
        {
        if (BeXmlStatus::BEXML_AttributeNotFound != status)
            {
            LOG.errorv("%s node contains an invalid %s value", FORMAT_ELEMENT, FORMAT_ROUND_FACTOR_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        roundFactor = 0.0;
        }
    spec.SetRoundingFactor(roundFactor);

    Utf8String specType;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(specType, FORMAT_TYPE_ATTRIBUTE))
        {
        LOG.errorv("%s node doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, FORMAT_TYPE_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Formatting::PresentationType type;
    if (!Formatting::Utils::NameToPresentationType(specType.c_str(), type))
        {
        LOG.errorv("%s node contains an invalid %s %s", FORMAT_ELEMENT, FORMAT_TYPE_ATTRIBUTE, specType.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (Formatting::PresentationType::Scientific == type)
        {
        Utf8String scientificType;
        if (BEXML_Success != unitFormatNode.GetAttributeStringValue(scientificType, FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE))
            {
            LOG.errorv("%s node doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, FORMAT_TYPE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        Formatting::ScientificType unitsScientificType;
        if (!Formatting::Utils::NameToScientificType(scientificType, unitsScientificType))
            {
            LOG.errorv("%s node contains an invalid %s %s", FORMAT_ELEMENT, FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, scientificType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetScientificType(unitsScientificType);
        }

    if (Formatting::PresentationType::Station == type)
        {
        uint32_t stationSize;
        if (BEXML_Success != unitFormatNode.GetAttributeUInt32Value(stationSize, FORMAT_STATION_SIZE_ATTRIBUTE))
            {
            LOG.errorv("%s node doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        if (stationSize < 1)
            {
            LOG.errorv("%s node contains an invalid %s %s", FORMAT_ELEMENT, FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetStationSize(stationSize);
        }

    spec.SetPresentationType(type);

    Utf8String showSignName;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(showSignName, FORMAT_SIGN_OPTION_ATTRIBUTE))
        showSignName = "onlyNegative";

    Formatting::ShowSignOption showSign;
    if (!Formatting::Utils::NameToSignOption(showSignName.c_str(), showSign))
        {
        LOG.errorv("%s node contains an invalid %s %s", FORMAT_ELEMENT, FORMAT_SIGN_OPTION_ATTRIBUTE, specType.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetSignOption(showSign);

    Utf8String formatTraits;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(formatTraits, FORMAT_TRAITS_ATTRIBUTE))
        formatTraits = "keepDecPnt|keepSingleZero";

    if (!spec.SetFormatTraitsFromString(formatTraits))
        {
        LOG.errorv("%s node contains an invalid %s %s", FORMAT_ELEMENT, FORMAT_TRAITS_ATTRIBUTE, specType.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    uint32_t precision;
    status = unitFormatNode.GetAttributeUInt32Value(precision, FORMAT_PRECISION_ATTRIBUTE);
    if (BeXmlStatus::BEXML_Success != status)
        { 
        if (BeXmlStatus::BEXML_AttributeNotFound != status)
            {
            LOG.errorv("%s node contains an invalid %s value", FORMAT_ELEMENT, FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        precision = 6;
        }

    if(type == Formatting::PresentationType::Fractional)
        {
        Formatting::FractionalPrecision unitsFractionalPrecision;
        if (!Formatting::Utils::FractionalPrecisionByDenominator(precision, unitsFractionalPrecision))
            {
            LOG.errorv("%s node contains an invalid %s value", FORMAT_ELEMENT, FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetFractionalPrecision(unitsFractionalPrecision);
        }
    else
        {
        Formatting::DecimalPrecision unitsDecimalPrecision;
        if (!Formatting::Utils::DecimalPrecisionByIndex(precision, unitsDecimalPrecision))
            {
            LOG.errorv("%s node contains an invalid %s value", FORMAT_ELEMENT, FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetDecimalPrecision(unitsDecimalPrecision);
        }

    Utf8String fracBarTypeName;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(fracBarTypeName, FORMAT_FRACTIONAL_BAR_TYPE_ATTRIBUTE))
        fracBarTypeName = "diagonal";

    Formatting::FractionBarType fracBarType;
    if (!Formatting::Utils::NameToFractionBarType(fracBarTypeName.c_str(), fracBarType))
        {
        LOG.errorv("%s node contains an invalid %s value %s", FORMAT_ELEMENT, FORMAT_FRACTIONAL_BAR_TYPE_ATTRIBUTE, fracBarTypeName.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetFractionalBarType(fracBarType);

    Utf8String decimalSeparator;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(decimalSeparator, FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE))
        decimalSeparator = "."; //TODO: get localized separator here !//

    if (decimalSeparator.length() > 1 || decimalSeparator.length() == 0)
        {
        LOG.errorv("%s node contains an invalid %s value %s", FORMAT_ELEMENT, FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, decimalSeparator.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetDecimalSeparator(decimalSeparator.at(0));

    Utf8String thousandSeparator;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(thousandSeparator, FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE))
        thousandSeparator = "."; //TODO: get localized separator here !//   

    if (thousandSeparator.length() > 1 || thousandSeparator.length() == 0)
        {
        LOG.errorv("%s node contains an invalid %s value %s", FORMAT_ELEMENT, FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, thousandSeparator.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetThousandSeparator(thousandSeparator.at(0));
    
    Utf8String uomSeparator;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(uomSeparator, FORMAT_UOM_SEPARATOR_ATTRIBUTE))
        uomSeparator = " ";

    spec.SetUomSeparator(uomSeparator.c_str());

    Utf8String statSeparator;
    if (BeXmlStatus::BEXML_Success != unitFormatNode.GetAttributeStringValue(statSeparator, FORMAT_STAT_SEPARATOR_ATTRIBUTE))
        statSeparator = "+";

    if (statSeparator.length() > 1 || statSeparator.length() == 0)
        {
        LOG.errorv("%s node contains an invalid %s value %s", FORMAT_ELEMENT, FORMAT_STAT_SEPARATOR_ATTRIBUTE, statSeparator.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetStatSeparator(statSeparator.at(0));

    SetNumericSpec(spec);
    auto child = unitFormatNode.GetFirstChild();
    if (NULL != unitFormatNode.GetFirstChild())
        {
        if(0 != BeStringUtilities::StricmpAscii("Composite", child->GetName()))
            {
            LOG.errorv("%s node contains an invalid child %", FORMAT_ELEMENT, child->GetName());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        _ReadCompositeSpecXml(*child, context);
        }

    return SchemaReadStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus Format::_ReadCompositeSpecXml(BeXmlNodeR compositeNode, ECSchemaReadContextR context)
    {
    Utf8String spacer;
    if (BEXML_Success != compositeNode.GetAttributeStringValue(spacer, COMPOSITE_SPACER_ATTRIBUTE))
        spacer = " ";
    Utf8String inputUnit;
    if (BEXML_Success != compositeNode.GetAttributeStringValue(inputUnit, COMPOSITE_INPUT_UNIT_ATTRIBUTE))
        {
        LOG.errorv("%s node doesn't contain a valid %s attribute which is required", FORMAT_COMPOSITE_ELEMENT, COMPOSITE_INPUT_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    ECUnitCP unit = GetSchema().GetUnitsContext().LookupUnit(inputUnit.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("%s node's %s could not be found", FORMAT_COMPOSITE_ELEMENT, COMPOSITE_INPUT_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    uint32_t numChildren = 0;
    bvector<ECUnitCP> units;
    bvector<Utf8String> labels;
    for (BeXmlNodeP childNode = compositeNode.GetFirstChild(); childNode != nullptr; childNode = childNode->GetNextSibling())
        {
        if (SchemaReadStatus::Success != _ReadCompositeUnitXml(*childNode, context, units, labels))
            return SchemaReadStatus::InvalidECSchemaXml;
        if (++numChildren > 4)
            {
            LOG.errorv("%s node has too many children units", FORMAT_COMPOSITE_ELEMENT);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    BeAssert(units.size() == labels.size());
    if (units.size() == 0)
        {
        LOG.errorv("%s node has no children units", FORMAT_COMPOSITE_ELEMENT);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    bvector<std::pair<Units::UnitCP, Utf8String>> unitsAndLabels;
    for(int i = 0; i < units.size(); i++)
        {
        unitsAndLabels.push_back({units[i], labels[i]});
        }
    auto comp = Formatting::CompositeValueSpec(unitsAndLabels, unit);
    comp.SetSpacer(spacer.c_str());
    SetCompositeSpec(comp);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus Format::_ReadCompositeUnitXml(BeXmlNodeR unitNode, ECSchemaReadContextR context, bvector<ECUnitCP>& units, bvector<Utf8String>& labels)
    {
    Utf8String unitName;
    if (BEXML_Success != unitNode.GetContent(unitName))
        {
        LOG.errorv("%s node is missing a unit value", FORMAT_COMPOSITE_UNIT_ELEMENT);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    ECUnitCP unit = GetSchema().GetUnitsContext().LookupUnit(unitName.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("%s node has invalid unit %s", FORMAT_COMPOSITE_UNIT_ELEMENT, unitName.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    units.push_back(unit);

    Utf8String label;
    if (BEXML_Success == unitNode.GetAttributeStringValue(label, COMPOSITE_UNIT_LABEL_ATTRIBUTE))
        labels.push_back(label);
    else
        labels.push_back(unit->GetDisplayLabel());

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
    xmlWriter.WriteAttribute(FORMAT_ROUND_FACTOR_ATTRIBUTE, this->GetNumericSpec()->GetRoundingFactor());
    xmlWriter.WriteAttribute(FORMAT_TYPE_ATTRIBUTE, Formatting::Utils::PresentationTypeName(this->GetNumericSpec()->GetPresentationType()).c_str());
    xmlWriter.WriteAttribute(FORMAT_SIGN_OPTION_ATTRIBUTE, Formatting::Utils::SignOptionName(this->GetNumericSpec()->GetSignOption()).c_str());
    xmlWriter.WriteAttribute(FORMAT_TRAITS_ATTRIBUTE, GetNumericSpec()->GetFormatTraitsString().c_str());
    xmlWriter.WriteAttribute(FORMAT_PRECISION_ATTRIBUTE, GetPresentationType() == Formatting::PresentationType::Fractional ? 
        static_cast<uint32_t>(pow(2u, static_cast<uint32_t>(GetNumericSpec()->GetFractionalPrecision()))) : 
        static_cast<uint32_t>(GetNumericSpec()->GetDecimalPrecision()));
    xmlWriter.WriteAttribute(FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, Formatting::Utils::ScientificTypeName(GetNumericSpec()->GetScientificType()).c_str());
    xmlWriter.WriteAttribute(FORMAT_FRACTIONAL_BAR_TYPE_ATTRIBUTE, Formatting::Utils::FractionBarName(GetNumericSpec()->GetFractionalBarType()).c_str());
    xmlWriter.WriteAttribute(FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, Utf8String(1, GetNumericSpec()->GetDecimalSeparator()).c_str());
    xmlWriter.WriteAttribute(FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, Utf8String(1,GetNumericSpec()->GetThousandSeparator()).c_str());
    xmlWriter.WriteAttribute(FORMAT_UOM_SEPARATOR_ATTRIBUTE, GetNumericSpec()->GetUomSeparator());
    xmlWriter.WriteAttribute(FORMAT_STAT_SEPARATOR_ATTRIBUTE, Utf8String(1, GetNumericSpec()->GetStatSeparator()).c_str());
    if (HasComposite())
        {
        xmlWriter.WriteElementStart(FORMAT_COMPOSITE_ELEMENT);
        xmlWriter.WriteAttribute(COMPOSITE_SPACER_ATTRIBUTE, GetCompositeSpec()->GetSpacer().c_str());
        xmlWriter.WriteAttribute(COMPOSITE_INPUT_UNIT_ATTRIBUTE, ((ECUnitCP)GetCompositeSpec()->GetInputUnit())->GetQualifiedName(GetSchema()).c_str());
        auto writeUnit = [&](Utf8String label, Units::UnitCP unit)
            {
            xmlWriter.WriteElementStart(FORMAT_COMPOSITE_UNIT_ELEMENT);
            xmlWriter.WriteAttribute(COMPOSITE_UNIT_LABEL_ATTRIBUTE, label.c_str());
            xmlWriter.WriteText(((ECUnitCP)unit)->GetQualifiedName(GetSchema()).c_str());
            xmlWriter.WriteElementEnd();
            };
        auto spec = GetCompositeSpec();
        writeUnit(spec->GetMajorLabel(), spec->GetMajorUnit());
        writeUnit(spec->GetMiddleLabel(), spec->GetMiddleUnit());
        writeUnit(spec->GetMinorLabel(), spec->GetMinorUnit());
        writeUnit(spec->GetSubLabel(), spec->GetSubUnit());
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
    outValue[FORMAT_ROUND_FACTOR_ATTRIBUTE] = GetNumericSpec()->GetRoundingFactor();
    outValue[FORMAT_TYPE_ATTRIBUTE] = Formatting::Utils::PresentationTypeName(GetNumericSpec()->GetPresentationType());
    outValue[FORMAT_SIGN_OPTION_ATTRIBUTE] = Formatting::Utils::SignOptionName(GetNumericSpec()->GetSignOption());
    outValue[FORMAT_TRAITS_ATTRIBUTE] = GetNumericSpec()->GetFormatTraitsString();
    outValue[FORMAT_PRECISION_ATTRIBUTE] = GetPresentationType() == Formatting::PresentationType::Fractional ? 
        static_cast<uint32_t>(GetNumericSpec()->GetFractionalPrecision()) : 
        static_cast<uint32_t>(GetNumericSpec()->GetDecimalPrecision());
    outValue[FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE] = Formatting::Utils::ScientificTypeName(GetNumericSpec()->GetScientificType());
    outValue[FORMAT_FRACTIONAL_BAR_TYPE_ATTRIBUTE] = Formatting::Utils::FractionBarName(GetNumericSpec()->GetFractionalBarType());
    outValue[FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE] = GetNumericSpec()->GetDecimalSeparator();
    outValue[FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE] = GetNumericSpec()->GetThousandSeparator();
    outValue[FORMAT_UOM_SEPARATOR_ATTRIBUTE] = GetNumericSpec()->GetUomSeparator();
    outValue[FORMAT_STAT_SEPARATOR_ATTRIBUTE] = GetNumericSpec()->GetStatSeparator();

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