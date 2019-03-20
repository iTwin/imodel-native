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

    m_fullName.RecomputeName(*this);
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
        LOG.errorv("Validation Error - KindOfQuantity '%s' persistence unit is a constant", GetFullName().c_str());
        isValid = false;
        }
    
    for (auto const& format : m_presentationFormats)
        {
        if (format.IsProblem())
            {
            LOG.errorv("Validation Error - KindOfQuantity '%s' presentation format has a problem: %s",
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
    return m_fullName.GetName(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2018
//--------------------------------------------------------------------------------------
Utf8String KindOfQuantity::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<KindOfQuantity>(primarySchema, *this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::SetDisplayLabel(Utf8CP value) {m_validatedName.SetDisplayLabel(value); return ECObjectsStatus::Success;}

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
        {
        LOG.errorv("On KoQ '%s' cannot set unit '%s' as the persistence unit because it is a constant", GetFullName().c_str(), unit.GetFullName().c_str());
        return ECObjectsStatus::Error;
        }
    if (HasPresentationFormats())
        {
        for (auto const& format : m_presentationFormats)
            {
            if (format.HasCompositeMajorUnit() && !ECUnit::AreCompatible(&unit, format.GetCompositeMajorUnit()))
                {
                LOG.errorv("On KoQ '%s' cannot set unit '%s' as the persistence unit because it is not compatible with format '%s'", GetFullName().c_str(), unit.GetFullName().c_str(), format.GetName().c_str());
                return ECObjectsStatus::Error;
                }
            }
        }
    m_persistenceUnit = &unit;
    m_descriptorCacheValid = false;

    return ECObjectsStatus::Success;
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
    xmlWriter.WriteAttribute(RELATIVE_ERROR, relError);

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
    if (presentationUnits.size() > 0)
        {
        Utf8String presentationUnitString;
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
                Utf8String fusDescriptor;
                if(ECObjectsStatus::Success != FormatToFUSDescriptor(fusDescriptor, format))
                    continue;
                if (!first)
                    presentationUnitString += ";";
                presentationUnitString += fusDescriptor;
                first = false;
                }
            else
                {
                if (!first)
                    presentationUnitString += ";";
                presentationUnitString += format.GetQualifiedFormatString(GetSchema());
                first = false;
                }
            }
        xmlWriter.WriteAttribute(PRESENTATION_UNITS_ATTRIBUTE, presentationUnitString.c_str());
        }

    xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPersistenceUnitByName(Utf8StringCR unitName, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper)
    {
    Utf8String alias;
    Utf8String name;
    ECClass::ParseClassName(alias, name, unitName);
    if (alias.empty())
        alias.assign(GetSchema().GetAlias());

    auto unit = nameToUnitMapper(alias, name);
    if (nullptr == unit)
        {
        LOG.errorv("Unable to add unit name '%s' ", unitName.c_str());
        return ECObjectsStatus::InvalidUnitName;
        }

    if (nullptr == GetSchema().GetUnitsContext().LookupUnit(unitName.c_str()))
        {
        LOG.errorv("Could not locate persistence unit with name '%s' for KoQ '%s' could not be located", unitName.c_str(), GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    SetPersistenceUnit(*unit);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
NamedFormatCP KindOfQuantity::GetCachedPersistenceFormat() const
    {
    if (nullptr == m_persistenceUnit)
        return nullptr;

    if (m_persFormatCache.IsValid())
        {
        if (Units::Unit::AreEqual(m_persFormatCache.Value().GetCompositeMajorUnit(), m_persistenceUnit))
            return &m_persFormatCache.Value();
        }

    Utf8String name = "DefaultRealU";
    name += "[";
    name += m_persistenceUnit->GetQualifiedName(GetSchema());
    name += "]";
    m_persFormatCache = NamedFormat(name);
    m_persFormatCache.ValueR().SetNumericSpec(Formatting::NumericFormatSpec::DefaultFormat());
    auto comp = Formatting::CompositeValueSpec(*m_persistenceUnit);
    comp.SetMajorLabel(m_persistenceUnit->GetDisplayLabel().c_str());
    m_persFormatCache.ValueR().SetCompositeSpec(comp);
    return &m_persFormatCache.Value();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatSingleUnitOverride(ECFormatCR parent, Nullable<int32_t> precisionOverride, ECUnitCP inputUnitOverride, Utf8CP labelOverride, bool isDefault)
    {
    UnitAndLabelPairs units;
    if (nullptr == inputUnitOverride && nullptr != labelOverride)
        {
        if (parent.HasCompositeMajorUnit())
            inputUnitOverride = (ECUnitCP)parent.GetCompositeMajorUnit();
        else
            return ECObjectsStatus::Error;
        }
    if (nullptr != inputUnitOverride)
        { 
        units = UnitAndLabelPairs();
        units.push_back(make_bpair(inputUnitOverride, (nullptr != labelOverride) ? Nullable<Utf8String>(labelOverride) : Nullable<Utf8String>(nullptr)));
        }

    return AddPresentationFormat(parent, precisionOverride, &units, isDefault);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  07/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::TransformFormatString(ECFormatCP& outFormat, Nullable<int32_t>& outPrec, UnitAndLabelPairs& outPairs, Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper, ECSchemaCP koqSchema)
    {
    Utf8String formatName;
    Nullable<int32_t> prec;
    bvector<Utf8String> unitNames;
    bvector<Nullable<Utf8String>> unitLabels;
    if (BentleyStatus::SUCCESS != Formatting::Format::ParseFormatString(formatName, prec, unitNames, unitLabels, formatString))
        {
        LOG.errorv("Failed to parse Presentation FormatString '%s'", formatString.c_str());
        return ECObjectsStatus::Error;
        }

    Utf8String alias;
    Utf8String unqualifiedName;
    ECClass::ParseClassName(alias, unqualifiedName, formatName);

    if (alias.empty() && (nullptr != koqSchema))
        alias = koqSchema->GetAlias();

    auto format = nameToFormatMapper(alias, unqualifiedName);

    if (nullptr == format)
        {
        LOG.errorv("Format '%s' could not be looked up", formatName.c_str());
        return ECObjectsStatus::Error;
        }

    if (!unitNames.empty())
        {
        int i = 0;
        for (const auto& u : unitNames)
            {
            ECClass::ParseClassName(alias, unqualifiedName, u);
            if (alias.empty() && (nullptr != koqSchema))
                alias = koqSchema->GetAlias();

            auto unit = nameToUnitMapper(alias, unqualifiedName);
            if (nullptr == unit)
                {
                LOG.errorv("Presentation unit with name '%s' could not be looked up", u.c_str());
                return ECObjectsStatus::Error;
                }
            outPairs.push_back(make_bpair(unit, (i < unitLabels.size()) ? unitLabels[i] : nullptr));
            i++;
            }
        }

    outFormat = format;
    outPrec = prec;
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatByString(Utf8StringCR formatString, std::function<ECFormatCP(Utf8StringCR, Utf8StringCR)> const& nameToFormatMapper, std::function<ECUnitCP(Utf8StringCR, Utf8StringCR)> const& nameToUnitMapper)
    {
    ECFormatCP format = nullptr;
    Nullable<int32_t> prec = nullptr;
    UnitAndLabelPairs units;
    if (ECObjectsStatus::Success != TransformFormatString(format, prec, units, formatString, nameToFormatMapper, nameToUnitMapper, &GetSchema()))
        return ECObjectsStatus::Error;

    if (!units.empty())
        return AddPresentationFormat(*format, prec, &units);
    return AddPresentationFormat(*format, prec); // no unit overrides
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  07/2018
//--------------------------------------------------------------------------------------
// static
bool KindOfQuantity::ValidatePresentationFormat(ECFormatCR parent, ECUnitCP persistenceUnit, Nullable<int32_t> precisionOverride, KindOfQuantity::UnitAndLabelPairs const* unitsAndLabels)
    {
    // Check for null persistence unit. Cannot add presentation units if there is no persistence unit
    if (nullptr == persistenceUnit)
        {
        LOG.error("Missing a persistence unit. Cannot add presentation formats.");
        return false;
        }

    if (parent.IsOverride())
        {
        LOG.error("KOQ cannot create an override using another override as a parent");
        return false;
        }

    // Parent has no units and we don't provide any overrides
    if (!parent.HasCompositeMajorUnit() && (nullptr == unitsAndLabels || unitsAndLabels->empty()))
        {
        LOG.error("KOQ cannot have a format without composite units and no unit overrides.");
        return false;
        }

    // Parent has unit and verify they are compatible
    if (parent.HasCompositeMajorUnit() && nullptr != persistenceUnit && !Units::Unit::AreCompatible(parent.GetCompositeMajorUnit(), persistenceUnit))
        {
        LOG.error("KOQ cannot have a format with a major unit that is incompatible with KOQ's persistence unit");
        return false;
        }

    if (precisionOverride.IsValid())
        {
        if (Formatting::PresentationType::Fractional == parent.GetPresentationType())
            {
            Formatting::FractionalPrecision prec;
            if (!Formatting::Utils::FractionalPrecisionByDenominator(prec, precisionOverride.Value()))
                {
                LOG.errorv("%d is not a valid fractional precision override value", precisionOverride.Value());
                return false;
                }
            }
        else
            {
            Formatting::DecimalPrecision prec;
            if (!Formatting::Utils::GetDecimalPrecisionByInt(prec, precisionOverride.Value()))
                {
                LOG.errorv("%d is not a valid decimal precision override value", precisionOverride.Value());
                return false;
                }
            }
        }

    if (nullptr == unitsAndLabels || unitsAndLabels->empty())
        return true;

    auto& overrides = *unitsAndLabels;
    if(parent.HasComposite() && 0 < parent.GetCompositeSpec()->GetUnitCount())
        {
        auto composite = parent.GetCompositeSpec();
        if (composite->GetUnitCount() != overrides.size())
            {
            LOG.error("Cannot override a different number of units than already exists on a format");
            return false;
            }

        auto compUnits = composite->GetUnits();
        for (int i = 0; i < overrides.size(); ++i)
            {
            if (!Units::Unit::AreEqual(overrides[i].first, compUnits[i]))
                {
                LOG.error("KOQ cannot change UOM in an override when one is already defined by the format");
                return false;
                }
            }
        }
    else
        {
        bvector<Units::UnitCP> newUnits;
        for (auto pair : overrides)
            {
            // Validate that unit is compatible with the KOQ.
            if(nullptr == persistenceUnit || nullptr == pair.first || !Units::Unit::AreCompatible(persistenceUnit, pair.first))
                {
                LOG.errorv("All unit overrides must be compatible with each other and must exist");
                return false;
                }
            newUnits.push_back(pair.first);
            }

        Formatting::CompositeValueSpec localComp;
        bool compStatus = Formatting::CompositeValueSpec::CreateCompositeSpec(localComp, newUnits);

        if (!compStatus) // CompStatus will be false if: too many units, null units or the spec has a problem
            {
            LOG.error("Failed to set composite spec");
            if (localComp.IsProblem())
                LOG.errorv("Composite spec has problem '%s'", localComp.GetProblemDescription().c_str());

            return false;
            }
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  07/2018
//--------------------------------------------------------------------------------------
// static
bool KindOfQuantity::ValidatePresentationFormat(Utf8StringCR formatString, ECUnitCP persistenceUnit, ECSchemaCR formats, ECSchemaCR units)
    {
    ECFormatCP format = nullptr;
    Nullable<int32_t> prec = nullptr;
    UnitAndLabelPairs pairs;

    // lookup units on this KoQs schema. All references must be already added.
    const auto unitLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        return units.LookupUnit((alias + ":" + name).c_str());
        };

    // lookup units on this KoQs schema. All references must be already added.
    const auto formatLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        return formats.LookupFormat((alias + ":" + name).c_str());
        };

    if (ECObjectsStatus::Success != TransformFormatString(format, prec, pairs, formatString, formatLookerUpper, unitLookerUpper))
        return false;

    return ValidatePresentationFormat(*format, persistenceUnit, prec, &pairs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormat(ECFormatCR parent, Nullable<int32_t> precisionOverride, UnitAndLabelPairs const* unitsAndLabels, bool isDefault)
    {
    // All validation done here. No need to check error codes in this function
    if (!ValidatePresentationFormat(parent, GetPersistenceUnit(), precisionOverride, unitsAndLabels))
        return ECObjectsStatus::Error;

    Utf8String out;
    CreateOverrideString(out, parent, precisionOverride, unitsAndLabels);
    NamedFormat nfp = NamedFormat(out, &parent);

    if (precisionOverride.IsValid())
        {
        if (Formatting::PresentationType::Fractional == nfp.GetPresentationType())
            {
            Formatting::FractionalPrecision prec;
            Formatting::Utils::FractionalPrecisionByDenominator(prec, precisionOverride.Value());
            nfp.GetNumericSpecP()->SetPrecision(prec);
            }
        else
            {
            Formatting::DecimalPrecision prec;
            Formatting::Utils::GetDecimalPrecisionByInt(prec, precisionOverride.Value());
            nfp.GetNumericSpecP()->SetPrecision(prec);
            }
        }

    // If there are no unit overrides
    if (nullptr == unitsAndLabels || 0 == unitsAndLabels->size())
        {
        if (isDefault)
            m_presentationFormats.insert(m_presentationFormats.begin(), nfp);
        else
            m_presentationFormats.push_back(nfp);
        return ECObjectsStatus::Success;
        }

    auto& overrides = *unitsAndLabels;
    bvector<Units::UnitCP> newUnits;
    for (auto pair : overrides)
        newUnits.push_back(pair.first);

    Formatting::CompositeValueSpec localComp;
    Formatting::CompositeValueSpec::CreateCompositeSpec(localComp, newUnits);
    nfp.SetCompositeSpec(localComp);

    if (!overrides.empty())
        {
        // If we override a numeric spec then set spacer to match UomSeparator. If we don't there will always be a space
        // between the value and its unit label since the default spacer is a blank character.
        if (!nfp.GetCompositeSpecP()->HasSpacer())
            {
            Formatting::NumericFormatSpecCP nsP = parent.GetNumericSpec();
            if (nsP && nsP->GetUomSeparator())
                nfp.GetCompositeSpecP()->SetSpacer(nsP->GetUomSeparator());
            }
        }

    switch (overrides.size()) // Fallthroughs intentional to add all necessary labels
        {
        case 4:
            if (overrides[3].second.IsValid())
                nfp.GetCompositeSpecP()->SetSubLabel(overrides[3].second.Value());
        case 3:
            if (overrides[2].second.IsValid())
                nfp.GetCompositeSpecP()->SetMinorLabel(overrides[2].second.Value());
        case 2:
            if (overrides[1].second.IsValid())
                nfp.GetCompositeSpecP()->SetMiddleLabel(overrides[1].second.Value());
        case 1:
            if (overrides[0].second.IsValid())
                nfp.GetCompositeSpecP()->SetMajorLabel(overrides[0].second.Value());
        }

    if (isDefault)
        m_presentationFormats.insert(m_presentationFormats.begin(), nfp);
    else
        m_presentationFormats.push_back(nfp);

    m_descriptorCacheValid = false;

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::AddPresentationFormatInternal(NamedFormat format)
    {
    if (format.HasCompositeMajorUnit() && !Units::Unit::AreCompatible(format.GetCompositeMajorUnit(), GetPersistenceUnit()))
        {
        LOG.errorv("KOQ '%s' cannot add presentation format '%s' because its major unit is not compatible with this KoQ's persistence unit", GetFullName().c_str(), format.GetName().c_str());
        return ECObjectsStatus::Error;
        }

    m_presentationFormats.emplace_back(format);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::CreateOverrideString(Utf8StringR out, ECFormatCR parent, Nullable<int32_t> precisionOverride,  UnitAndLabelPairs const* unitsAndLabels) const
    {
    if (parent.IsOverride())
        {
        LOG.errorv("KOQ '%s' cannot create an override using another override as a parent", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    out += parent.GetName();
    
    if (precisionOverride.IsValid())
        {
        out += "(";
        out += std::to_string(precisionOverride.Value()).c_str();
        out += ")";
        }

    if (nullptr != unitsAndLabels)
        {
        auto& input = *unitsAndLabels;
        if (input.size() > 4)
            {
            LOG.errorv("KOQ '%s' cannot have more than 4 override units specified on a presentation format", GetFullName().c_str());
            return ECObjectsStatus::Error;
            }

        for(const auto& i : input)
            { 
            auto& unit = i.first;
            if (nullptr == unit)
                return ECObjectsStatus::Error;
            out += "[";
            out += unit->GetQualifiedName(GetSchema());

            if (i.second.IsValid()) // We want to override a label
                {
                out += "|";
                out += i.second.Value();
                }

            out += "]";
            }
        }
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  06/2017
//---------------------------------------------------------------------------------------
Json::Value KindOfQuantity::GetPresentationFormatsJson() const
    {
    Json::Value arrayObj(Json::arrayValue);

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
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
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool KindOfQuantity::ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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
        return false;
        }

    outValue[PERSISTENCE_UNIT_ATTRIBUTE] =  ECJsonUtilities::ECNameToJsonName(*GetPersistenceUnit());

    outValue[RELATIVE_ERROR] = GetRelativeError();

    bvector<NamedFormat> const& presentationUnits = GetPresentationFormats();
    if (0 != presentationUnits.size())
        {
        Json::Value presentationUnitArr(Json::ValueType::arrayValue);
        for (auto const& format : presentationUnits)
            {
            if (format.IsProblem())
                {
                LOG.errorv("Failed to write schema because persistance format for KindOfQuantity '%s' has problem: '%s'", GetName().c_str(), format.GetProblemDescription().c_str());
                return false;
                }
            presentationUnitArr.append(SchemaParseUtils::GetJsonFormatString(format, GetSchema()));
            }
        outValue[PRESENTATION_UNITS_ATTRIBUTE] = presentationUnitArr;
        }

    return true;
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
    if (BEXML_Success != kindOfQuantityNode.GetAttributeDoubleValue(relError, RELATIVE_ERROR))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), RELATIVE_ERROR);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetRelativeError(relError);

    // Read Persistence Unit
    Utf8String persUnit;
    if (BEXML_Success != kindOfQuantityNode.GetAttributeStringValue(persUnit, PERSISTENCE_UNIT_ATTRIBUTE) || Utf8String::IsNullOrEmpty(persUnit.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: KindOfQuantity %s must contain a %s attribute", GetFullName().c_str(), PERSISTENCE_UNIT_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    // Read Presentation Formats
    bvector<Utf8String> presentationFormats;
    if (BEXML_Success == kindOfQuantityNode.GetAttributeStringValue(value, PRESENTATION_UNITS_ATTRIBUTE))
        BeStringUtilities::Split(value.c_str(), ";", presentationFormats);

    // If version < 3.2. We have to upgrade our desriptors before we parse them.
    if (GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_2))
        {
        // Add schema references. Always need units for the persistence unit.
        SchemaKey key("Units", 1, 0, 0);
        auto unitsSchema = context.LocateSchema(key, SchemaMatchType::Latest);
        if (!unitsSchema.IsValid())
            {
            LOG.errorv("Failed to find '%s' schema to add as a reference schema for '%s'.", key.GetName().c_str(), GetSchema().GetName().c_str());
            return SchemaReadStatus::ReferencedSchemaNotFound;
            }
        if (!ECSchema::IsSchemaReferenced(GetSchema(), *unitsSchema))
            { 
            LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve old units.",
                unitsSchema->GetName().c_str(), GetSchema().GetName().c_str());
            if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*unitsSchema))
                {
                LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", unitsSchema->GetName().c_str(), GetSchema().GetName().c_str());
                return SchemaReadStatus::ReferencedSchemaNotFound;
                }
            }

        key = SchemaKey("Formats", 1, 0, 0);
        auto formatsSchema = context.LocateSchema(key, SchemaMatchType::Latest);
        if (!formatsSchema.IsValid())
            {
            LOG.errorv("Failed to find '%s' schema to add as a reference schema for '%s'.", key.GetName().c_str(), GetSchema().GetName().c_str());
            return SchemaReadStatus::ReferencedSchemaNotFound;
            }
        if (!ECSchema::IsSchemaReferenced(GetSchema(), *formatsSchema))
            { 
            LOG.warningv("Adding '%s' as a reference schema to '%s', in order to resolve old formats.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
            if (ECObjectsStatus::Success != GetSchemaR().AddReferencedSchema(*formatsSchema))
                {
                LOG.errorv("Failed to add '%s' as a reference schema of '%s'.", formatsSchema->GetName().c_str(), GetSchema().GetName().c_str());
                return SchemaReadStatus::ReferencedSchemaNotFound;
                }
            }

        bvector<Utf8CP> fusDescriptors;
        fusDescriptors.reserve(presentationFormats.size());
        for (const auto& str : presentationFormats) // Load pointers to match api for FromFUSDescriptors
            fusDescriptors.push_back(str.c_str());

        if (ECObjectsStatus::Success != FromFUSDescriptors(persUnit.c_str(), fusDescriptors, *formatsSchema, *unitsSchema))
            return SchemaReadStatus::InvalidECSchemaXml;

        if (GetPresentationFormats().empty())
            {
            GetSchemaR().RemoveReferencedSchema(*formatsSchema);
            }

        return SchemaReadStatus::Success;
        }

    // lookup units on this KoQs schema. All references must be already added.
    const auto unitLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        if (alias == GetSchema().GetAlias())
            return GetSchema().LookupUnit(name.c_str());

        return GetSchema().LookupUnit((alias + ":" + name).c_str());
        };

    // lookup units on this KoQs schema. All references must be already added.
    const auto formatLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        if (alias == GetSchema().GetAlias())
            return GetSchema().LookupFormat(name.c_str());

        return GetSchema().LookupFormat((alias + ":" + name).c_str());
        };

    ECUnitCP persUnitPointer = GetSchema().LookupUnit(persUnit.c_str());
    if (nullptr == persUnitPointer)
        return SchemaReadStatus::InvalidECSchemaXml;

    SetPersistenceUnit(*persUnitPointer);

    for(auto const& presValue : presentationFormats)
        {
        if (ECObjectsStatus::Success != AddPresentationFormatByString(presValue, formatLookerUpper, unitLookerUpper))
            return SchemaReadStatus::InvalidECSchemaXml; // Logging in AddPresentationUnit
        }

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
static ECObjectsStatus ExtractUnitFormatAndMap(Utf8StringR unitName, Utf8StringR formatName, Utf8CP descriptor)
    {
    Formatting::Format::ParseUnitFormatDescriptor(unitName, formatName, descriptor);
    unitName = Units::UnitNameMappings::TryGetECNameFromNewName(unitName.c_str());
    if (unitName.empty())
        {
        LOG.errorv("Failed to find unit mapping for unit with name '%s' in legacy unit mappings", unitName.c_str());
        return ECObjectsStatus::InvalidUnitName;
        }
    Utf8CP mappedName = formatName.c_str();;
    if (!Utf8String::IsNullOrEmpty(formatName.c_str()))
        {
        mappedName = Formatting::AliasMappings::TryGetNameFromAlias(formatName.c_str());
        mappedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName((nullptr == mappedName) ? formatName.c_str() : mappedName);
        if (nullptr == mappedName)
            {
            LOG.warningv("EC3.2 upgrade: Failed to find a mapping for the legacy name/alias '%s'. Dropping the format.", formatName.c_str());
            formatName = ""; // Resetting this so there isn't a valid formatName in the return.
            return ECObjectsStatus::InvalidFormat;
            }
        Utf8String alias;
        Utf8String name;
        SchemaParseUtils::ParseName(alias, name, mappedName);
        formatName = ("f:" + name).c_str();
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::FromFUSDescriptors(Utf8CP persFUS, const bvector<Utf8CP>& presFuses, ECSchemaCR formatSchema, ECSchemaCR unitsSchema)
    {
    Utf8String persUnit;
    bvector<Utf8String> formatStrings;
    auto status = KindOfQuantity::UpdateFUSDescriptors(persUnit, formatStrings, persFUS, presFuses, formatSchema, unitsSchema);
    
    if (ECObjectsStatus::Success != status)
        return status;

    auto persUnitCP = GetSchema().LookupUnit(persUnit.c_str());
    SetPersistenceUnit(*persUnitCP);
    m_descriptorCacheValid = false;

    // lookup units on this KoQs schema. All references must be already added.
    const auto unitLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        if (alias == GetSchema().GetAlias())
            return GetSchema().LookupUnit(name.c_str());

        return GetSchema().LookupUnit((alias + ":" + name).c_str());
        };

    // lookup units on this KoQs schema. All references must be already added.
    const auto formatLookerUpper = [&](Utf8StringCR alias, Utf8StringCR name) 
        {
        if (alias == GetSchema().GetAlias())
            return GetSchema().LookupFormat(name.c_str());

        return GetSchema().LookupFormat((alias + ":" + name).c_str());
        };

    RemoveAllPresentationFormats();
    for (const auto& pres : formatStrings)
        {
        if (ECObjectsStatus::Success != AddPresentationFormatByString(pres, formatLookerUpper, unitLookerUpper))
            return ECObjectsStatus::Error;
        }

    // Cache old descriptors
    m_descriptorCache.first = persFUS;
    m_descriptorCache.second = {};
    for (const auto& pres : presFuses)
        {
        m_descriptorCache.second.push_back(pres);
        }
    m_descriptorCacheValid = true;

    return ECObjectsStatus::Success;
    }

// Used for upgrading from ECXml 3.1.
static const Utf8String oldDefaultFormatName = "DefaultReal";

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
ECObjectsStatus KindOfQuantity::UpdateFUSDescriptors(Utf8StringR unitName, bvector<Utf8String>& formatStrings, Utf8CP persFus, bvector<Utf8CP> const& presFuses, ECSchemaCR formatSchema, ECSchemaCR unitsSchema)
    {
    unitName.clear();
    formatStrings.clear();
    if (Utf8String::IsNullOrEmpty(persFus))
        return ECObjectsStatus::NullPointerValue;

    //Persistence
    Utf8String persistenceUnit;
    Utf8String persistenceFormat;
    Utf8String alias;
    Utf8String unqualifiedPers;
    ECObjectsStatus status = ExtractUnitFormatAndMap(persistenceUnit, persistenceFormat, persFus);
    if (ECObjectsStatus::Success != status && ECObjectsStatus::InvalidFormat != status)
        return status;

    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPers, persistenceUnit))
        return ECObjectsStatus::Error;
    
    auto persistenceUnitCP = unitsSchema.GetUnitCP(unqualifiedPers.c_str());
    // Presentation
    for (Utf8CP presFus : presFuses)
        {
        Utf8String presentationUnit;
        Utf8String presentationFormat;
        status = ExtractUnitFormatAndMap(presentationUnit, presentationFormat, presFus);
        if (ECObjectsStatus::Success != status)
            {
            if (ECObjectsStatus::InvalidFormat == status)
                continue; // Dropping the presentation FUS
            return status;
            }

        if (presentationUnit.empty())
            {
            LOG.errorv("Presentation unit was not defined in this descriptor '%s'", presFus);
            return ECObjectsStatus::InvalidUnitName;
            }

        if (presentationFormat.empty())
            presentationFormat = oldDefaultFormatName;

        Utf8String unqualifiedPres;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPres, presentationUnit))
            return ECObjectsStatus::Error;

        Utf8String unqualifiedPresFormat;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPresFormat, presentationFormat))
            return ECObjectsStatus::Error;

        Utf8String formatString;
        formatString
            .append("f:")
            .append(unqualifiedPresFormat);
        bvector<Utf8String> split;
        BeStringUtilities::Split(unqualifiedPresFormat.c_str(), "(", split);
        Utf8String qualifiedPres = "u:" + unqualifiedPres;
        auto format = formatSchema.GetFormatCP(split[0].c_str());
        if (nullptr != format && !format->HasComposite() && !unqualifiedPresFormat.Contains("["))
            {
            formatString
                .append("[")
                .append(qualifiedPres)
                .append("]");
            }

        if (!ValidatePresentationFormat(formatString, persistenceUnitCP, formatSchema, unitsSchema))
            {
            LOG.warningv("Dropping presentation FUS '%s' because it mapped to format string '%s' which is not valid", presFus, formatString.c_str());
            continue;
            }

        formatStrings.push_back(formatString);
        }

    // If there are no presentation units, create on using the persistence fus. Use the default format if none is provided
    if (formatStrings.size() == 0 && !persistenceFormat.empty())
        {
        Utf8String unqualifiedPersFormat;
        if(ECObjectsStatus::Success != ECClass::ParseClassName(alias, unqualifiedPersFormat, persistenceFormat))
            return ECObjectsStatus::Error;

        Utf8String formatString;
        formatString
            .append("f:")
            .append(unqualifiedPersFormat);
        bvector<Utf8String> split;
        BeStringUtilities::Split(unqualifiedPersFormat.c_str(), "(", split);
        auto format = formatSchema.GetFormatCP(split[0].c_str());
        if (nullptr != format && !format->HasComposite() && !unqualifiedPersFormat.Contains("["))
            {
            formatString
                .append("[")
                .append("u:")
                .append(unqualifiedPers)
                .append("]");
            }

        if (ValidatePresentationFormat(formatString, persistenceUnitCP, formatSchema, unitsSchema))
            formatStrings.push_back(formatString);
        else
            LOG.warningv("Dropping presentation FUS '%s' because it mapped to format string '%s' which is not valid", persFus, formatString.c_str());
        }

    unitName = "u:" + unqualifiedPers;
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Joseph.Urbano                   09/2018
//--------------------------------------------------------------------------------------
ECObjectsStatus KindOfQuantity::FormatToFUSDescriptor(Utf8StringR outFUSDescriptor, NamedFormatCR format) const
    {
    outFUSDescriptor = "";

    if (!format.HasComposite() || !format.GetCompositeSpec()->HasMajorUnit())
        {
        LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }
    ECUnitCP unit = (ECUnitCP)format.GetCompositeSpec()->GetMajorUnit();
    auto ecName = Units::UnitNameMappings::TryGetNewNameFromECName(unit->GetFullName().c_str());
    if (nullptr == ecName)
        {
        LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it does not have an input unit which is required to serialize to version < v3_2.", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }
    Utf8String presUnit = ecName;

    SchemaKey key("Formats", 1, 0, 0);
    if (!format.GetParentFormat()->GetSchema().GetSchemaKey().Matches(key, SchemaMatchType::Latest))
        {
        LOG.warningv("Dropping presentation format for KindOfQuantity '%s because it is not a standard format", GetFullName().c_str());
        return ECObjectsStatus::Error;
        }
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(format.GetName().c_str(), "[", tokens);
    BeAssert(tokens.size() > 0);
    Utf8String split = tokens[0];
    Utf8CP mapped = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(("FORMATS:" + split).c_str());
    mapped = Formatting::AliasMappings::TryGetAliasFromName(mapped);
    if (nullptr == mapped)
        {
        LOG.warningv("Dropping presentation format '%s' for KindOfQuantity '%s' because it could not be mapped to an old format", format.GetQualifiedFormatString(GetSchema()).c_str(), GetFullName().c_str());
        return ECObjectsStatus::Error;
        }

    Utf8String presentationUnitString;
    outFUSDescriptor += presUnit;
    outFUSDescriptor += "(";
    outFUSDescriptor += mapped;
    outFUSDescriptor += ")";
    
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Joseph.Urbano                   09/2018
//--------------------------------------------------------------------------------------
void KindOfQuantity::ValidateDescriptorCache() const
    {
    if (m_descriptorCacheValid)
        return;

    m_descriptorCache.first.clear();
    m_descriptorCache.second.clear();

    ECUnitCP unit = GetPersistenceUnit();
    if(nullptr == unit)
        {
        LOG.errorv("Persistence unit not found on KindOfQuantity '%s' while validating the FUS descriptor cache", GetFullName().c_str());
        return;
        } 

    auto newName = Units::UnitNameMappings::TryGetNewNameFromECName(unit->GetFullName().c_str());
    if (nullptr != newName)
        m_descriptorCache.first = newName;

    for (NamedFormatCR format : GetPresentationFormats())
        {
        Utf8String fusDescriptor;
        if(ECObjectsStatus::Success != FormatToFUSDescriptor(fusDescriptor, format))
            continue;
        m_descriptorCache.second.push_back(fusDescriptor);
        }

    m_descriptorCacheValid = true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
