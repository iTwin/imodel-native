/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormat::NamedFormat(Utf8StringCR name, ECFormatCP format) : Formatting::Format(), m_nameOrFormatString(name), m_ecFormat(format)
    {
    if (nullptr != format)
        {
        if (format->HasNumeric())
            SetNumericSpec(*format->GetNumericSpec());
        if (format->HasComposite())
            SetCompositeSpec(*format->GetCompositeSpec());
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String NamedFormat::GetQualifiedFormatString(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(m_ecFormat->GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify a NamedFormat name with an alias unless the schema containing the ECFormat is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  NamedFormat: %s\n ECSchema containing the parent ECFormat: %s", primarySchema.GetName().c_str(), name.c_str(), m_ecFormat->GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool NamedFormat::_ToJson(BeJsValue out, bool verbose) const
    {
    if(HasNumeric())
        {
        if (!GetNumericSpec()->ToJson(out, false))
            return false;
        }
    // TODO: This is duplicated from Formatting::CompositeValueSpec::_ToJson. Make sure to update both
    if (HasComposite())
        {
        auto comp = GetCompositeSpec();
        auto compElement = out[FORMAT_JSON_COMPOSITE_ELEMENT];
        if (!comp->ToJson(compElement, verbose, true))
            return false;

        auto writeUnit = [&](Nullable<Utf8String> label, Units::UnitCP unit, BeJsValue unitsArray)
            {
            auto unitElement = unitsArray.appendValue();
            if(label.IsValid())
               unitElement[COMPOSITE_UNIT_LABEL_ATTRIBUTE] = label.Value().c_str();
            unitElement[NAME_ATTRIBUTE] = ECJsonUtilities::ECNameToJsonName(*(ECUnitCP)unit);
            };

        auto unitArray = compElement[FORMAT_COMPOSITE_UNITS_ELEMENT];
        unitArray.toArray();
        if (HasCompositeMajorUnit())
            writeUnit((comp->HasMajorLabel() || verbose) ? comp->GetMajorLabel() : nullptr, comp->GetMajorUnit(), unitArray);
        if (HasCompositeMiddleUnit())
            writeUnit((comp->HasMiddleLabel() || verbose) ? comp->GetMiddleLabel() : nullptr, comp->GetMiddleUnit(), unitArray);
        if (HasCompositeMinorUnit())
            writeUnit((comp->HasMinorLabel() || verbose) ? comp->GetMinorLabel() : nullptr, comp->GetMinorUnit(), unitArray);
        if (HasCompositeSubUnit())
            writeUnit((comp->HasSubLabel() || verbose) ? comp->GetSubLabel() : nullptr, comp->GetSubUnit(), unitArray);
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECFormat::ECFormat(ECSchemaCR schema, Utf8StringCR name) : NamedFormat(name.c_str(), this), m_isDisplayLabelExplicitlyDefined(false), m_schema(&schema)
    {
    GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECFormat::SetSchema(ECSchemaCR schema)
    {
    m_schema = &schema;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECFormat::GetFullName() const
    {
    return m_fullName.GetName(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECFormat::GetDisplayLabel() const
    {
    return GetInvariantDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECFormat::GetDescription() const
    {
    return GetInvariantDescription();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String ECFormat::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<ECFormat>(primarySchema, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECFormat::ReadXml(pugi::xml_node unitFormatNode, ECSchemaReadContextR context)
    {
    Utf8String value; // used by the macros.
    BePugiXmlValueResult status;
    READ_OPTIONAL_XML_ATTRIBUTE(unitFormatNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(unitFormatNode, DESCRIPTION_ATTRIBUTE, this, Description)

    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();

    double roundFactor;
    status = BePugiXmlHelper::GetAttributeDoubleValue(unitFormatNode, roundFactor, FORMAT_ROUND_FACTOR_ATTRIBUTE);
    if (status == BePugiXmlValueResult::Success)
        spec.SetRoundingFactor(roundFactor);
    else if (status == BePugiXmlValueResult::TypeMismatch)
        {
        LOG.errorv("%s node '%s' contains an invalid %s value: %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_ROUND_FACTOR_ATTRIBUTE, unitFormatNode.attribute(FORMAT_ROUND_FACTOR_ATTRIBUTE).as_string());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    auto specTypeAttr = unitFormatNode.attribute(FORMAT_TYPE_ATTRIBUTE);
    if (!specTypeAttr)
        {
        LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Utf8String specType = specTypeAttr.as_string();

    Formatting::PresentationType type;
    if (!Formatting::Utils::ParsePresentationType(type, specType.c_str()))
        {
        LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE, specType.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    spec.SetPresentationType(type);

    if (Formatting::PresentationType::Scientific == type)
        {
        auto scientificTypeAttr = unitFormatNode.attribute(FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE);
        
        if (!scientificTypeAttr)
            {
            LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TYPE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        Utf8String scientificType(scientificTypeAttr.as_string());

        Formatting::ScientificType unitsScientificType;
        if (!Formatting::Utils::ParseScientificType(unitsScientificType, scientificType))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, scientificType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetScientificType(unitsScientificType);
        }

    if (Formatting::PresentationType::Station == type)
        {
        uint32_t stationSize;
        if (BePugiXmlHelper::GetAttributeUInt32Value(unitFormatNode, stationSize, FORMAT_STATION_SIZE_ATTRIBUTE) != BePugiXmlValueResult::Success)
            {
            LOG.errorv("%s node '%s' doesn't contain a valid %s attribute which is required", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (stationSize < 1 || stationSize > INT32_MAX) //If negative values are passed they underflow but GetAttributeUint32 still returns success. Underflow is undefined behaviour according to the standard TODO
            {
            LOG.errorv("%s node '%s' contains an invalid %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STATION_SIZE_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetStationOffsetSize(stationSize);
        }

    
    auto showSignAttr = unitFormatNode.attribute(FORMAT_SIGN_OPTION_ATTRIBUTE);
    if (showSignAttr)
        {
        Formatting::SignOption showSign;
        if (!Formatting::Utils::ParseSignOption(showSign, showSignAttr.as_string()))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_SIGN_OPTION_ATTRIBUTE, specType.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetSignOption(showSign);
        }

    auto formatTraitsAttr = unitFormatNode.attribute(FORMAT_TRAITS_ATTRIBUTE);
    if (formatTraitsAttr)
        {
        if (!spec.SetFormatTraits(formatTraitsAttr.as_string()))
            {
            LOG.errorv("%s node '%s' contains an invalid %s %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_TRAITS_ATTRIBUTE, formatTraitsAttr.as_string());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    uint32_t precision;
    status = BePugiXmlHelper::GetAttributeUInt32Value(unitFormatNode, precision, FORMAT_PRECISION_ATTRIBUTE);
    if (status != BePugiXmlValueResult::Success)
        {
        if (status == BePugiXmlValueResult::Null)
            LOG.errorv("%s node '%s' has a missing '%s' attribute ", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
        else
            LOG.errorv("%s node '%s' has an invalid '%s' attribute ", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
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
        spec.SetPrecision(unitsFractionalPrecision);
        }
    else
        {
        Formatting::DecimalPrecision unitsDecimalPrecision;
        if (!Formatting::Utils::GetDecimalPrecisionByInt(unitsDecimalPrecision, precision))
            {
            LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_PRECISION_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetPrecision(unitsDecimalPrecision);
        }

    int32_t minWidth;
    status = BePugiXmlHelper::GetAttributeInt32Value(unitFormatNode, minWidth, FORMAT_MIN_WIDTH_ATTRIBUTE);
    if (status == BePugiXmlValueResult::Success && minWidth > 0)
        spec.SetMinWidth(minWidth);
    else if (status != BePugiXmlValueResult::Null)
        {
        LOG.errorv("%s node '%s' contains an invalid %s value", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_MIN_WIDTH_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    auto decimalSeparatorAttr = unitFormatNode.attribute(FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE);
    if (decimalSeparatorAttr)
        {
        Utf8String decimalSeparator(decimalSeparatorAttr.as_string());
        if (decimalSeparator.length() > 1 || decimalSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, decimalSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetDecimalSeparator(decimalSeparator.at(0));
        }

    auto thousandSeparatorAttr = unitFormatNode.attribute(FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE);
    if (thousandSeparatorAttr)
        {
        Utf8String thousandSeparator(thousandSeparatorAttr.as_string());
        if (thousandSeparator.length() > 1 || thousandSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, thousandSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetThousandSeparator(thousandSeparator.at(0));
        }

    auto uomSeparatorAttr = unitFormatNode.attribute(FORMAT_UOM_SEPARATOR_ATTRIBUTE);;
    if (uomSeparatorAttr)
        spec.SetUomSeparator(uomSeparatorAttr.as_string());

    auto statSeparatorAttr = unitFormatNode.attribute(FORMAT_STAT_SEPARATOR_ATTRIBUTE);
    if (statSeparatorAttr)
        {
        Utf8String statSeparator(statSeparatorAttr.as_string());
        if (statSeparator.length() > 1 || statSeparator.length() == 0)
            {
            LOG.errorv("%s node '%s' contains an invalid %s value %s", FORMAT_ELEMENT, GetFullName().c_str(), FORMAT_STAT_SEPARATOR_ATTRIBUTE, statSeparator.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        spec.SetStationSeparator(statSeparator.at(0));
        }

    SetNumericSpec(spec);
    pugi::xml_node child = unitFormatNode.first_child();
    if (child)
        {
        if(0 != BeStringUtilities::StricmpAscii("Composite", child.name()))
            {
            LOG.errorv("%s node '%s' contains an invalid child %", FORMAT_ELEMENT, GetFullName().c_str(), child.name());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        SchemaReadStatus compStatus = ReadCompositeSpecXml(child, context);
        if (SchemaReadStatus::Success != compStatus)
            return compStatus;
        }

    if (IsProblem())
        {
        LOG.errorv("%s node '%s' has problem '%s'", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str(), GetProblemDescription().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECFormat::ReadCompositeSpecXml(pugi::xml_node compositeNode, ECSchemaReadContextR context)
    {
    uint32_t numChildren = 0;
    bvector<ECUnitCP> units;
    bvector<Nullable<Utf8String>> labels;
    for (pugi::xml_node childNode : compositeNode.children())
        {
        if(childNode.type() != pugi::xml_node_type::node_element)
            continue;
        
        if (SchemaReadStatus::Success != ReadCompositeUnitXml(childNode, context, units, labels))
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
    Formatting::CompositeValueSpec comp;
    if (!Formatting::CompositeValueSpec::CreateCompositeSpec(comp, unitsVector))
        {
        LOG.errorv("%s node on %s failed to create composite value spec", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    if (labels[0].IsValid())
        comp.SetMajorLabel(labels[0].Value());
    if (labels.size() > 1 && labels[1].IsValid())
        comp.SetMiddleLabel(labels[1].Value());
    if (labels.size() > 2 && labels[2].IsValid())
        comp.SetMinorLabel(labels[2].Value());
    if (labels.size() > 3 && labels[3].IsValid())
        comp.SetSubLabel(labels[3].Value());

    auto spacerAttr = compositeNode.attribute(COMPOSITE_SPACER_ATTRIBUTE);
    if (spacerAttr)
        comp.SetSpacer(spacerAttr.as_string());

    auto includeZeroAttr = compositeNode.attribute(COMPOSITE_INCLUDEZERO_ATTRIBUTE);
    if (includeZeroAttr)
        comp.SetIncludeZero(includeZeroAttr.as_bool(true));

    if (comp.IsProblem())
        {
        LOG.errorv("%s node on %s has problem %s", FORMAT_COMPOSITE_ELEMENT, GetFullName().c_str(), comp.GetProblemDescription().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetCompositeSpec(comp);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECFormat::ReadCompositeUnitXml(pugi::xml_node unitNode, ECSchemaReadContextR context, bvector<ECUnitCP>& units, bvector<Nullable<Utf8String>>& labels)
    {
    if (unitNode.text().empty())
        {
        LOG.errorv("%s node on %s is missing a unit value", FORMAT_COMPOSITE_UNIT_ELEMENT, GetFullName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Utf8String unitName(unitNode.text().as_string());

    ECUnitCP unit = GetSchema().GetUnitsContext().LookupUnit(unitName.c_str());
    if (nullptr == unit)
        {
        LOG.errorv("%s node on %s has invalid unit %s", FORMAT_COMPOSITE_UNIT_ELEMENT, GetFullName().c_str(), unitName.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    units.push_back(unit);

    auto labelAttr = unitNode.attribute(COMPOSITE_UNIT_LABEL_ATTRIBUTE);
    if (labelAttr)
        {
        Utf8String label(labelAttr.as_string());
        labels.push_back(label);
        }
    else
        labels.push_back(nullptr);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECFormat::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
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
        xmlWriter.WriteAttribute(FORMAT_TYPE_ATTRIBUTE, Formatting::Utils::GetPresentationTypeString(nfs->GetPresentationType()).c_str());
        if (nfs->HasSignOption())
            xmlWriter.WriteAttribute(FORMAT_SIGN_OPTION_ATTRIBUTE, Formatting::Utils::GetSignOptionString(nfs->GetSignOption()).c_str());
        if (nfs->HasFormatTraits())
            xmlWriter.WriteAttribute(FORMAT_TRAITS_ATTRIBUTE, nfs->GetFormatTraitsString().c_str());
        xmlWriter.WriteAttribute(FORMAT_PRECISION_ATTRIBUTE, GetPresentationType() == Formatting::PresentationType::Fractional ?
            static_cast<uint32_t>(pow(2u, static_cast<uint32_t>(nfs->GetFractionalPrecision()))) :
            static_cast<uint32_t>(nfs->GetDecimalPrecision()));
        if (nfs->HasMinWidth())
            xmlWriter.WriteAttribute(FORMAT_MIN_WIDTH_ATTRIBUTE, nfs->GetMinWidth());
        if (Formatting::PresentationType::Scientific == GetPresentationType())
            xmlWriter.WriteAttribute(FORMAT_SCIENTIFIC_TYPE_ATTRIBUTE, Formatting::Utils::GetScientificTypeString(nfs->GetScientificType()).c_str());
        if (nfs->HasDecimalSeparator())
            xmlWriter.WriteAttribute(FORMAT_DECIMAL_SEPARATOR_ATTRIBUTE, Utf8String(1, nfs->GetDecimalSeparator()).c_str());
        if (nfs->HasThousandsSeparator())
            xmlWriter.WriteAttribute(FORMAT_THOUSANDS_SEPARATOR_ATTRIBUTE, Utf8String(1,nfs->GetThousandSeparator()).c_str());
        if (nfs->HasUomSeparator())
			{
            Utf8String uomSeparator = nfs->GetUomSeparator();
            if (Utf8String::IsNullOrEmpty(uomSeparator.c_str()))
                xmlWriter.WriteEmptyAttribute(FORMAT_UOM_SEPARATOR_ATTRIBUTE);
            else
                xmlWriter.WriteAttribute(FORMAT_UOM_SEPARATOR_ATTRIBUTE, uomSeparator.c_str());
            }
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
            {
            Utf8String spacerString = comp->GetSpacer();
            if (Utf8String::IsNullOrEmpty(spacerString.c_str()))
                xmlWriter.WriteEmptyAttribute(COMPOSITE_SPACER_ATTRIBUTE);
            else
                xmlWriter.WriteAttribute(COMPOSITE_SPACER_ATTRIBUTE, spacerString.c_str());
            }
        xmlWriter.WriteAttribute(COMPOSITE_INCLUDEZERO_ATTRIBUTE, comp->IsIncludeZero());
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECFormat::ToJsonInternal(BeJsValue outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema children
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = FORMAT_ELEMENT;
    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (!Utf8String::IsNullOrEmpty(GetInvariantDescription().c_str()))
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // The includeSchemaVersion flag is acting as the verbose flag here. This isn't ideal...
    if (!T_Super::_ToJson(outValue, includeSchemaVersion))
        return false;

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
