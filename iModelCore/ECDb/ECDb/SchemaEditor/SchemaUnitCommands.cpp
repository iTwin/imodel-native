/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;
enum class Family { UnitSystem,
                    Phenomenon,
                    Unit,
                    Format };
enum class Operation { Create,
                       Alter };

Utf8CP Name(Family family) {
    switch (family) {
    case Family::UnitSystem:
        return "UnitSystem";
    case Family::Phenomenon:
        return "Phenomenon";
    case Family::Unit:
        return "Unit";
    default:
        return "Format";
    }
}

Utf8CP Key(Family family) {
    switch (family) {
    case Family::UnitSystem:
        return "unitSystem";
    case Family::Phenomenon:
        return "phenomenon";
    case Family::Unit:
        return "unit";
    default:
        return "format";
    }
}

Utf8CP Table(Family family) {
    switch (family) {
    case Family::UnitSystem:
        return "ec_UnitSystem";
    case Family::Phenomenon:
        return "ec_Phenomenon";
    case Family::Unit:
        return "ec_Unit";
    default:
        return "ec_Format";
    }
}

Result Number(SchemaCommandArgParser& parser, Utf8CP key, double& out, bool required = false) {
    if (!parser.Has(key))
        return required ? Invalid(key, "Missing required argument.") : Result();
    auto value = parser.Value(key);
    if (value.isNumeric())
        out = value.GetDouble();
    else if (value.isString()) {
        Utf8String text(value.asCString());
        text.Trim();
        char* end = nullptr;
        out = text.empty() ? 0.0 : strtod(text.c_str(), &end);
        if (text.empty() || end != text.c_str() + text.size())
            return Invalid(key, "Expected a number.");
    } else
        return Invalid(key, "Expected a number.");
    return std::isfinite(out) ? Result() : Invalid(key, "Expected a finite number.");
}

// Schemas referenced by items resolved into the working copy; persisted as ec_SchemaReference on apply.
struct ItemReferences {
    SchemaEditContext& m_ctx;
    ECSchemaR m_working;
    bvector<ECSchemaId> m_ids;
    ItemReferences(SchemaEditContext& ctx, ECSchemaR working) : m_ctx(ctx), m_working(working) {}
    Result Add(ECSchemaCR persisted) {
        if (persisted.GetName().EqualsI(m_working.GetName()))
            return {};
        auto result = Reference(m_ctx, m_working, persisted);
        if (result.IsOk() && std::find(m_ids.begin(), m_ids.end(), persisted.GetId()) == m_ids.end())
            m_ids.push_back(persisted.GetId());
        return result;
    }
};

Result AddReferences(SchemaEditContext& apply, ECSchemaId owner, bvector<ECSchemaId> const& ids) {
    for (auto id : ids) {
        auto result = apply.AddReference(owner, id);
        if (!result.IsOk())
            return result;
    }
    return {};
}

template <typename T>
Result Resolve(SchemaEditContext& ctx, ItemReferences& refs, Utf8CP arg, Utf8StringCR qualified, Utf8CP what,
               std::function<T const*(SchemaManager const&, Utf8StringCR, Utf8StringCR)> const& persistedLookup,
               std::function<T const*(ECSchemaCR, Utf8CP)> const& workingLookup, T const*& persisted, T const*& working) {
    Utf8String schemaName, item;
    if (!QualifiedName(qualified, schemaName, item))
        return Invalid(arg, "Expected a qualified schema item name (Schema:Item).");
    persisted = persistedLookup(ctx.m_db.Schemas(), schemaName, item);
    if (persisted == nullptr)
        return Missing(arg, Utf8PrintfString("%s '%s' not found.", what, qualified.c_str()));
    auto result = refs.Add(persisted->GetSchema());
    if (!result.IsOk())
        return result;
    ECSchemaCP schema = WorkingSchema(refs.m_working, persisted->GetSchema().GetName());
    working = schema == nullptr ? nullptr : workingLookup(*schema, persisted->GetName().c_str());
    return working != nullptr ? Result() : Rejected(arg, Utf8PrintfString("%s '%s' is missing from the working schema graph.", what, qualified.c_str()));
}

Result ResolveUnitSystem(SchemaEditContext& ctx, ItemReferences& refs, Utf8CP arg, Utf8StringCR name, UnitSystemCP& persisted, UnitSystemCP& working) {
    return Resolve<UnitSystem>(ctx, refs, arg, name, "UnitSystem", [](SchemaManager const& m, Utf8StringCR s, Utf8StringCR i) { return m.GetUnitSystem(s, i, SchemaLookupMode::AutoDetect); }, [](ECSchemaCR s, Utf8CP n) { return s.GetUnitSystemCP(n); }, persisted, working);
}

Result ResolvePhenomenon(SchemaEditContext& ctx, ItemReferences& refs, Utf8CP arg, Utf8StringCR name, PhenomenonCP& persisted, PhenomenonCP& working) {
    return Resolve<Phenomenon>(ctx, refs, arg, name, "Phenomenon", [](SchemaManager const& m, Utf8StringCR s, Utf8StringCR i) { return m.GetPhenomenon(s, i, SchemaLookupMode::AutoDetect); }, [](ECSchemaCR s, Utf8CP n) { return s.GetPhenomenonCP(n); }, persisted, working);
}

Result ResolveUnit(SchemaEditContext& ctx, ItemReferences& refs, Utf8CP arg, Utf8StringCR name, ECUnitCP& persisted, ECUnitCP& working) {
    return Resolve<ECUnit>(ctx, refs, arg, name, "Unit", [](SchemaManager const& m, Utf8StringCR s, Utf8StringCR i) { return m.GetUnit(s, i, SchemaLookupMode::AutoDetect); }, [](ECSchemaCR s, Utf8CP n) { return s.GetUnitCP(n); }, persisted, working);
}

ECSchemaCP FindItemSchema(SchemaEditContext& ctx, Family family, Utf8StringCR qualified, BeInt64Id& id, ECN::ECUnitCP& unit, ECFormatCP& format,
                          UnitSystemCP& system, PhenomenonCP& phenomenon) {
    Utf8String schemaName, item;
    if (!QualifiedName(qualified, schemaName, item))
        return nullptr;
    auto& schemas = ctx.m_db.Schemas();
    switch (family) {
    case Family::UnitSystem:
        system = schemas.GetUnitSystem(schemaName, item, SchemaLookupMode::AutoDetect);
        if (system == nullptr)
            return nullptr;
        id = system->GetId();
        return &system->GetSchema();
    case Family::Phenomenon:
        phenomenon = schemas.GetPhenomenon(schemaName, item, SchemaLookupMode::AutoDetect);
        if (phenomenon == nullptr)
            return nullptr;
        id = phenomenon->GetId();
        return &phenomenon->GetSchema();
    case Family::Unit:
        unit = schemas.GetUnit(schemaName, item, SchemaLookupMode::AutoDetect);
        if (unit == nullptr)
            return nullptr;
        id = unit->GetId();
        return &unit->GetSchema();
    default:
        format = schemas.GetFormat(schemaName, item, SchemaLookupMode::AutoDetect);
        if (format == nullptr)
            return nullptr;
        id = format->GetId();
        return &format->GetSchema();
    }
}

template <typename T>
void CreatedTexts(T const& item, SchemaMetaRow& row) {
    // SchemaWriter persists the label only when explicitly defined and the description only when non-empty.
    if (item.GetIsDisplayLabelDefined())
        row.push_back({"DisplayLabel", item.GetInvariantDisplayLabel()});
    if (item.GetIsDescriptionDefined())
        row.push_back({"Description", item.GetInvariantDescription()});
}

// An empty value removes the label/description, which SchemaWriter persists as NULL.
Result AlteredTexts(SchemaCommandArgParser& parser, Result& parsed, SchemaMetaRow& row) {
    for (auto key : {"label", "description"}) {
        if (!parser.Has(key))
            continue;
        Utf8String value;
        if (!parser.String(key, value))
            return parsed;
        row.push_back({strcmp(key, "label") == 0 ? "DisplayLabel" : "Description",
                       value.empty() ? SchemaMetaValue(nullptr) : SchemaMetaValue(value)});
    }
    return {};
}

Result Separator(SchemaCommandArgParser& parser, Result& parsed, Utf8CP key, std::function<void(Utf8Char)> const& set) {
    if (!parser.Has(key))
        return {};
    Utf8String value;
    if (!parser.String(key, value))
        return parsed;
    if (value.length() != 1)
        return Invalid(key, "A separator must be exactly one character.");
    set(value[0]);
    return {};
}

constexpr Utf8CP s_numericKeys[] = {"type", "precision", "roundFactor", "minWidth", "showSignOption", "formatTraits",
                                    "decimalSeparator", "thousandSeparator", "uomSeparator", "scientificType", "stationOffsetSize", "stationSeparator"};

// Mirrors ECFormat::ReadXml so that accepted values and failures match a native import.
Result NumericSpec(SchemaCommandArgParser& parser, Result& parsed, Formatting::NumericFormatSpec& spec) {
    if (parser.Has("roundFactor")) {
        double roundFactor;
        auto result = Number(parser, "roundFactor", roundFactor);
        if (!result.IsOk())
            return result;
        spec.SetRoundingFactor(roundFactor);
    }
    Utf8String text;
    if (!parser.String("type", text, true))
        return parsed;
    Formatting::PresentationType type;
    if (!Formatting::Utils::ParsePresentationType(type, text.c_str()))
        return Invalid("type", "Invalid format type.");
    spec.SetPresentationType(type);
    if (Formatting::PresentationType::Scientific == type) {
        if (!parser.String("scientificType", text, true))
            return parsed;
        Formatting::ScientificType scientificType;
        if (!Formatting::Utils::ParseScientificType(scientificType, text))
            return Invalid("scientificType", "Invalid scientific type.");
        spec.SetScientificType(scientificType);
    }
    if (Formatting::PresentationType::Station == type) {
        uint32_t size;
        if (!parser.UInt32("stationOffsetSize", size, true))
            return parsed;
        if (size < 1 || size > INT32_MAX)
            return Invalid("stationOffsetSize", "The station offset size must be between 1 and 2147483647.");
        spec.SetStationOffsetSize(size);
    }
    if (parser.Has("showSignOption")) {
        if (!parser.String("showSignOption", text))
            return parsed;
        Formatting::SignOption sign;
        if (!Formatting::Utils::ParseSignOption(sign, text.c_str()))
            return Invalid("showSignOption", "Invalid sign option.");
        spec.SetSignOption(sign);
    }
    if (parser.Has("formatTraits")) {
        auto value = parser.Value("formatTraits");
        Utf8String traits;
        if (value.isString())
            traits = value.asCString();
        else if (value.isArray()) {
            for (uint32_t i = 0; i < value.size(); ++i) {
                if (!value[i].isString())
                    return Invalid(Utf8PrintfString("formatTraits[%u]", i).c_str(), "Expected a string.");
                if (!traits.empty())
                    traits.append("|");
                traits.append(value[i].asCString());
            }
        } else
            return Invalid("formatTraits", "Expected a string or an array of strings.");
        if (!spec.SetFormatTraits(traits.c_str()))
            return Invalid("formatTraits", "Invalid format traits.");
    }
    uint32_t precision;
    if (!parser.UInt32("precision", precision, true))
        return parsed;
    if (precision > INT32_MAX)
        return Invalid("precision", "Invalid precision.");
    if (Formatting::PresentationType::Fractional == type) {
        Formatting::FractionalPrecision fractional;
        if (!Formatting::Utils::FractionalPrecisionByDenominator(fractional, static_cast<int32_t>(precision)))
            return Invalid("precision", "Invalid fractional precision.");
        spec.SetPrecision(fractional);
    } else {
        Formatting::DecimalPrecision decimal;
        if (!Formatting::Utils::GetDecimalPrecisionByInt(decimal, static_cast<int32_t>(precision)))
            return Invalid("precision", "Invalid decimal precision.");
        spec.SetPrecision(decimal);
    }
    if (parser.Has("minWidth")) {
        uint32_t minWidth;
        if (!parser.UInt32("minWidth", minWidth))
            return parsed;
        if (minWidth == 0 || minWidth > INT32_MAX)
            return Invalid("minWidth", "The minimum width must be a positive int32.");
        spec.SetMinWidth(minWidth);
    }
    auto result = Separator(parser, parsed, "decimalSeparator", [&](Utf8Char c) { spec.SetDecimalSeparator(c); });
    if (!result.IsOk())
        return result;
    result = Separator(parser, parsed, "thousandSeparator", [&](Utf8Char c) { spec.SetThousandSeparator(c); });
    if (!result.IsOk())
        return result;
    if (parser.Has("uomSeparator")) {
        if (!parser.String("uomSeparator", text))
            return parsed;
        spec.SetUomSeparator(text.c_str());
    }
    return Separator(parser, parsed, "stationSeparator", [&](Utf8Char c) { spec.SetStationSeparator(c); });
}

struct CompositeUnit {
    Utf8String m_name;
    Nullable<Utf8String> m_label;
};

Result CompositeUnits(SchemaCommandArgParser& parser, Result& parsed, bvector<CompositeUnit>& units) {
    auto value = parser.Value("composite.units");
    if (value.isArray()) {
        for (uint32_t i = 0; i < value.size(); ++i) {
            auto member = value[i];
            CompositeUnit unit;
            if (member.isString())
                unit.m_name = member.asCString();
            else {
                Result result;
                SchemaCommandArgParser unitParser(member, result);
                if (!member.isObject() || !unitParser.CheckKeys({"name", "label"}) || !unitParser.String("name", unit.m_name, true))
                    return result.IsOk() ? Invalid(Utf8PrintfString("composite.units[%u]", i).c_str(), "Expected a unit name or {name, label}.") : result;
                if (unitParser.Has("label")) {
                    Utf8String label;
                    if (!unitParser.String("label", label))
                        return result;
                    unit.m_label = label;
                }
            }
            units.push_back(unit);
        }
    } else {
        bvector<Utf8String> names;
        if (!parser.StringList("composite.units", names, true))
            return parsed;
        for (auto const& name : names)
            units.push_back({name, nullptr});
    }
    if (units.empty() || units.size() > 4)
        return Invalid("composite.units", "A composite format requires between one and four units.");
    return {};
}

Nullable<Utf8String> CompositeLabel(Formatting::CompositeValueSpecCR spec, size_t index) {
    switch (index) {
    case 0:
        return spec.HasMajorLabel() ? Nullable<Utf8String>(spec.GetMajorLabel()) : nullptr;
    case 1:
        return spec.HasMiddleLabel() ? Nullable<Utf8String>(spec.GetMiddleLabel()) : nullptr;
    case 2:
        return spec.HasMinorLabel() ? Nullable<Utf8String>(spec.GetMinorLabel()) : nullptr;
    default:
        return spec.HasSubLabel() ? Nullable<Utf8String>(spec.GetSubLabel()) : nullptr;
    }
}

Units::UnitCP CompositeUnitAt(Formatting::CompositeValueSpecCR spec, size_t index) {
    switch (index) {
    case 0:
        return spec.GetMajorUnit();
    case 1:
        return spec.GetMiddleUnit();
    case 2:
        return spec.GetMinorUnit();
    default:
        return spec.GetSubUnit();
    }
}

bool HasNumericKey(SchemaCommandArgParser const& parser) {
    for (auto key : s_numericKeys)
        if (parser.Has(key))
            return true;
    return false;
}

Result Ec32(SchemaEditContext& ctx, Family family) {
    if (FeatureManager::IsEC32Available(ctx.m_db))
        return {};
    return Rejected(Key(family), Utf8PrintfString("%s items are not supported in a file that does not support EC3.2.", Name(family)));
}

Result NotSupported(Family family, Utf8CP arg) {
    return Unsupported(arg, Utf8PrintfString("Changing properties of %s is not supported except for DisplayLabel and Description.", Name(family)));
}

Result PrepareCreateUnit(SchemaEditContext& ctx, SchemaCommandArgParser& parser, Result& parsed, ECSchemaCR schema, ECSchemaR working,
                         Utf8StringCR name, ItemReferences& refs, SchemaMetaRow& row) {
    bool isConstant = false;
    if (!parser.Boolean("isConstant", isConstant))
        return parsed;
    bool inverted = parser.Has("invertsUnit");
    Utf8CP mode = inverted ? "an inverted unit" : isConstant ? "a constant"
                                                             : "a unit";
    if (inverted && isConstant)
        return Invalid("isConstant", "An inverted unit cannot be a constant.");
    for (auto key : {"phenomenon", "definition", "numerator", "denominator", "offset"})
        if (inverted && parser.Has(key))
            return Invalid(key, Utf8PrintfString("'%s' is not valid for %s.", key, mode));
    for (auto key : {"unitSystem", "offset"})
        if (isConstant && parser.Has(key))
            return Invalid(key, Utf8PrintfString("'%s' is not valid for %s.", key, mode));
    Utf8String label, description, definition, phenomenonName, systemName, parentName;
    if (!parser.String("label", label) || !parser.String("description", description))
        return parsed;
    Utf8CP labelArg = parser.Has("label") ? label.c_str() : nullptr;
    Utf8CP descriptionArg = parser.Has("description") ? description.c_str() : nullptr;
    PhenomenonCP persistedPhenomenon = nullptr, workingPhenomenon = nullptr;
    UnitSystemCP persistedSystem = nullptr, workingSystem = nullptr;
    ECUnitCP persistedParent = nullptr, workingParent = nullptr;
    Result result;
    if (!inverted) {
        if (!parser.String("phenomenon", phenomenonName, true) || !parser.String("definition", definition, true))
            return parsed;
        if (definition.empty())
            return Invalid("definition", "The definition must not be empty.");
        result = ResolvePhenomenon(ctx, refs, "phenomenon", phenomenonName, persistedPhenomenon, workingPhenomenon);
        if (!result.IsOk())
            return result;
    } else {
        if (!parser.String("invertsUnit", parentName, true))
            return parsed;
        result = ResolveUnit(ctx, refs, "invertsUnit", parentName, persistedParent, workingParent);
        if (!result.IsOk())
            return result;
        if (persistedParent->IsInvertedUnit() || persistedParent->IsConstant())
            return Rejected("invertsUnit", "An inverted unit must invert a regular unit.");
    }
    if (!isConstant) {
        if (!parser.String("unitSystem", systemName, true))
            return parsed;
        result = ResolveUnitSystem(ctx, refs, "unitSystem", systemName, persistedSystem, workingSystem);
        if (!result.IsOk())
            return result;
    }
    Nullable<double> factors[3];
    Utf8CP factorKeys[3] = {"numerator", "denominator", "offset"};
    for (size_t i = 0; i < 3; ++i) {
        if (!parser.Has(factorKeys[i]))
            continue;
        double value;
        result = Number(parser, factorKeys[i], value);
        if (!result.IsOk())
            return result;
        if (i < 2 && value == 0.0)
            return Rejected(factorKeys[i], Utf8PrintfString("The %s must not be zero.", factorKeys[i]));
        factors[i] = value;
    }
    ECUnitP unit = nullptr;
    ECObjectsStatus status;
    if (inverted)
        status = working.CreateInvertedUnit(unit, *workingParent, name.c_str(), *workingSystem, labelArg, descriptionArg);
    else if (isConstant) {
        // ECUnit::ReadConstantXml requires the numerator.
        if (factors[0].IsNull())
            return Invalid("numerator", "A constant requires a numerator.");
        status = working.CreateConstant(unit, name.c_str(), definition.c_str(), *workingPhenomenon, factors[0].Value(), factors[1], labelArg, descriptionArg);
    } else
        status = working.CreateUnit(unit, name.c_str(), definition.c_str(), *workingPhenomenon, *workingSystem, factors[0], factors[1], factors[2], labelArg, descriptionArg);
    if (ECObjectsStatus::Success != status || unit == nullptr)
        return Rejected("name", Utf8PrintfString("ECObjects rejected %s with this name.", mode));
    CreatedTexts(*unit, row);
    row.push_back({"PhenomenonId", int64_t(inverted ? persistedParent->GetPhenomenon()->GetId().GetValue() : persistedPhenomenon->GetId().GetValue())});
    if (unit->HasUnitSystem())
        row.push_back({"UnitSystemId", int64_t(persistedSystem->GetId().GetValue())});
    if (unit->HasDefinition())
        row.push_back({"Definition", unit->GetDefinition()});
    if (unit->HasNumerator())
        row.push_back({"Numerator", unit->GetNumerator()});
    if (unit->HasDenominator())
        row.push_back({"Denominator", unit->GetDenominator()});
    if (unit->HasOffset())
        row.push_back({"Offset", unit->GetOffset()});
    row.push_back({"IsConstant", int64_t(unit->IsConstant() ? 1 : 0)});
    if (inverted)
        row.push_back({"InvertingUnitId", int64_t(persistedParent->GetId().GetValue())});
    return {};
}

Result PrepareAlterUnit(SchemaEditContext& ctx, SchemaCommandArgParser& parser, Result& parsed, ECUnitCR old, SchemaMetaRow& row) {
    Utf8String text;
    if (parser.Has("definition")) {
        if (!parser.String("definition", text))
            return parsed;
        if (!old.HasDefinition() || !text.Equals(old.GetDefinition()))
            return NotSupported(Family::Unit, "definition");
    }
    if (parser.Has("isConstant")) {
        bool isConstant;
        if (!parser.Boolean("isConstant", isConstant))
            return parsed;
        if (isConstant != old.IsConstant())
            return NotSupported(Family::Unit, "isConstant");
    }
    Utf8String schemaName, item;
    if (parser.Has("phenomenon")) {
        if (!parser.String("phenomenon", text))
            return parsed;
        auto value = QualifiedName(text, schemaName, item) ? ctx.m_db.Schemas().GetPhenomenon(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
        if (value == nullptr)
            return Missing("phenomenon", "Phenomenon not found.");
        if (!value->GetFullName().EqualsIAscii(old.GetPhenomenon()->GetFullName()))
            return NotSupported(Family::Unit, "phenomenon");
    }
    if (parser.Has("unitSystem")) {
        if (!parser.String("unitSystem", text))
            return parsed;
        auto value = QualifiedName(text, schemaName, item) ? ctx.m_db.Schemas().GetUnitSystem(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
        if (value == nullptr)
            return Missing("unitSystem", "Unit system not found.");
        if (!old.HasUnitSystem() || !value->GetFullName().EqualsIAscii(old.GetUnitSystem()->GetFullName()))
            return NotSupported(Family::Unit, "unitSystem");
    }
    if (parser.Has("invertsUnit")) {
        if (!parser.String("invertsUnit", text))
            return parsed;
        auto value = QualifiedName(text, schemaName, item) ? ctx.m_db.Schemas().GetUnit(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
        if (value == nullptr)
            return Missing("invertsUnit", "Unit not found.");
        if (!old.IsInvertedUnit() || !value->GetFullName().EqualsIAscii(old.GetInvertingUnit()->GetFullName()))
            return NotSupported(Family::Unit, "invertsUnit");
    }
    // SchemaWriter::UpdateUnit accepts a numerator/denominator/offset change only when the old value
    // was not explicitly defined and the new value equals the implicit default.
    struct Factor {
        Utf8CP m_key;
        Utf8CP m_column;
        bool m_has;
        double m_old;
        double m_default;
    };
    for (auto const& factor : {Factor{"numerator", "Numerator", old.HasNumerator(), old.GetNumerator(), 1.0},
                               Factor{"denominator", "Denominator", old.HasDenominator(), old.GetDenominator(), 1.0},
                               Factor{"offset", "Offset", old.HasOffset(), old.GetOffset(), 0.0}}) {
        if (!parser.Has(factor.m_key))
            continue;
        double value;
        auto result = Number(parser, factor.m_key, value);
        if (!result.IsOk())
            return result;
        if (factor.m_has && value == factor.m_old)
            continue;
        if (factor.m_has || value != factor.m_default)
            return Unsupported(factor.m_key, Utf8PrintfString("Changing the %s of a Unit is only supported when it was not defined and the new value is the default %g.",
                                                              factor.m_key, factor.m_default));
        if (old.IsInvertedUnit())
            return NotSupported(Family::Unit, factor.m_key);
        row.push_back({factor.m_column, value});
    }
    return {};
}

Result PrepareFormat(SchemaEditContext& ctx, SchemaCommandArgParser& parser, Result& parsed, bool create, ECSchemaR working,
                     Utf8StringCR name, ECFormatCP old, ItemReferences& refs, SchemaMetaRow& row,
                     bvector<std::pair<UnitId, Nullable<Utf8String>>>& compositeUnits, bvector<std::pair<BeInt64Id, Nullable<Utf8String>>>& labelChanges) {
    ECFormatP format = nullptr;
    if (create) {
        Formatting::NumericFormatSpec spec;
        auto result = NumericSpec(parser, parsed, spec);
        if (!result.IsOk())
            return result;
        Formatting::CompositeValueSpec composite;
        bool hasComposite = parser.Has("composite");
        if (hasComposite) {
            bvector<CompositeUnit> units;
            result = CompositeUnits(parser, parsed, units);
            if (!result.IsOk())
                return result;
            bvector<Units::UnitCP> workingUnits;
            for (size_t i = 0; i < units.size(); ++i) {
                ECUnitCP persisted = nullptr, unit = nullptr;
                result = ResolveUnit(ctx, refs, "composite.units", units[i].m_name, persisted, unit);
                if (!result.IsOk())
                    return result;
                workingUnits.push_back(unit);
                compositeUnits.push_back({persisted->GetId(), units[i].m_label});
            }
            if (!Formatting::CompositeValueSpec::CreateCompositeSpec(composite, workingUnits))
                return Rejected("composite.units", "Cannot create a composite value spec from these units.");
            for (size_t i = 0; i < units.size(); ++i) {
                if (units[i].m_label.IsNull())
                    continue;
                auto const& label = units[i].m_label.Value();
                switch (i) {
                case 0:
                    composite.SetMajorLabel(label);
                    break;
                case 1:
                    composite.SetMiddleLabel(label);
                    break;
                case 2:
                    composite.SetMinorLabel(label);
                    break;
                default:
                    composite.SetSubLabel(label);
                    break;
                }
            }
            Utf8String spacer;
            if (parser.Has("composite.spacer")) {
                if (!parser.String("composite.spacer", spacer))
                    return parsed;
                composite.SetSpacer(spacer.c_str());
            }
            bool includeZero;
            if (parser.Has("composite.includeZero")) {
                if (!parser.Boolean("composite.includeZero", includeZero))
                    return parsed;
                composite.SetIncludeZero(includeZero);
            }
            if (composite.IsProblem())
                return Rejected("composite", Utf8PrintfString("Invalid composite: %s", composite.GetProblemDescription().c_str()));
        }
        Utf8String label, description;
        if (!parser.String("label", label) || !parser.String("description", description))
            return parsed;
        if (ECObjectsStatus::Success != working.CreateFormat(format, name.c_str(), parser.Has("label") ? label.c_str() : nullptr,
                                                             parser.Has("description") ? description.c_str() : nullptr, &spec, hasComposite ? &composite : nullptr) ||
            format == nullptr)
            return Rejected("name", "Invalid or duplicate format name.");
        if (format->IsProblem())
            return Rejected("format", Utf8PrintfString("Invalid format: %s", format->GetProblemDescription().c_str()));
        CreatedTexts(*format, row);
        if (format->HasNumeric())
            row.push_back({"NumericSpec", SchemaPersistenceHelper::SerializeNumericSpec(*format->GetNumericSpec())});
        if (format->HasComposite()) {
            auto serialized = SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(*format->GetCompositeSpec());
            if (!serialized.empty())
                row.push_back({"CompositeSpec", serialized});
        }
        return {};
    }

    format = working.GetFormatP(name.c_str());
    if (format == nullptr)
        return Rejected("format", "Format missing from working schema.");
    auto result = AlteredTexts(parser, parsed, row);
    if (!result.IsOk())
        return result;
    if (HasNumericKey(parser)) {
        // The numeric spec is replaced as a whole, as in an ECJSON/ECXML upgrade.
        Formatting::NumericFormatSpec spec;
        result = NumericSpec(parser, parsed, spec);
        if (!result.IsOk())
            return result;
        auto serialized = SchemaPersistenceHelper::SerializeNumericSpec(spec);
        if (!old->HasNumeric() || !serialized.Equals(SchemaPersistenceHelper::SerializeNumericSpec(*old->GetNumericSpec())))
            row.push_back({"NumericSpec", serialized});
        format->SetNumericSpec(spec);
    }
    if (parser.Has("composite")) {
        if (!old->HasComposite())
            return Unsupported("composite", "Changing the composite units of Format is not supported.");
        Formatting::CompositeValueSpec composite(*format->GetCompositeSpec());
        auto const& oldComposite = *old->GetCompositeSpec();
        if (parser.Has("composite.units")) {
            bvector<CompositeUnit> units;
            result = CompositeUnits(parser, parsed, units);
            if (!result.IsOk())
                return result;
            if (units.size() != oldComposite.GetUnitCount())
                return Unsupported("composite.units", "Changing the composite units of Format is not supported.");
            for (size_t i = 0; i < units.size(); ++i) {
                Utf8String schemaName, item;
                auto unit = QualifiedName(units[i].m_name, schemaName, item) ? ctx.m_db.Schemas().GetUnit(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
                if (unit == nullptr)
                    return Missing("composite.units", Utf8PrintfString("Unit '%s' not found.", units[i].m_name.c_str()));
                auto oldUnit = static_cast<ECUnitCP>(CompositeUnitAt(oldComposite, i));
                if (oldUnit == nullptr || !oldUnit->GetFullName().EqualsIAscii(unit->GetFullName()))
                    return Unsupported("composite.units", "Changing the composite units of Format is not supported.");
                auto oldLabel = CompositeLabel(oldComposite, i);
                if (oldLabel.IsNull() == units[i].m_label.IsNull() && (oldLabel.IsNull() || oldLabel.Value().Equals(units[i].m_label.Value())))
                    continue;
                Statement statement;
                if (BE_SQLITE_OK != statement.Prepare(ctx.m_db, "SELECT Id FROM main.ec_FormatCompositeUnit WHERE FormatId=? AND Ordinal=?") ||
                    BE_SQLITE_OK != statement.BindId(1, old->GetId()) || BE_SQLITE_OK != statement.BindInt(2, (int)i) ||
                    BE_SQLITE_ROW != statement.Step())
                    return DatabaseError("Reading the composite unit of the format failed.");
                labelChanges.push_back({statement.GetValueId<BeInt64Id>(0), units[i].m_label});
            }
        }
        if (parser.Has("composite.spacer")) {
            Utf8String spacer;
            if (!parser.String("composite.spacer", spacer))
                return parsed;
            composite.SetSpacer(spacer.c_str());
        }
        if (parser.Has("composite.includeZero")) {
            bool includeZero;
            if (!parser.Boolean("composite.includeZero", includeZero))
                return parsed;
            composite.SetIncludeZero(includeZero);
        }
        auto serialized = SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(composite);
        if (!serialized.Equals(SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(oldComposite)))
            row.push_back({"CompositeSpec", serialized});
        format->SetCompositeSpec(composite);
    }
    if (format->IsProblem())
        return Rejected("format", Utf8PrintfString("Invalid format: %s", format->GetProblemDescription().c_str()));
    return {};
}

Result PrepareItem(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, Family family, Operation operation) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    bool create = operation == Operation::Create;
    Utf8CP key = Key(family);
    bool keysOk;
    switch (family) {
    case Family::UnitSystem:
        keysOk = create ? parser.CheckKeys({"schema", "name", "label", "description"}) : parser.CheckKeys({"unitSystem", "label", "description"});
        break;
    case Family::Phenomenon:
        keysOk = create ? parser.CheckKeys({"schema", "name", "definition", "label", "description"})
                        : parser.CheckKeys({"phenomenon", "definition", "label", "description"});
        break;
    case Family::Unit:
        keysOk = create ? parser.CheckKeys({"schema", "name", "phenomenon", "unitSystem", "definition", "numerator", "denominator", "offset",
                                            "isConstant", "invertsUnit", "label", "description"})
                        : parser.CheckKeys({"unit", "phenomenon", "unitSystem", "definition", "numerator", "denominator", "offset",
                                            "isConstant", "invertsUnit", "label", "description"});
        break;
    default:
        keysOk = create ? parser.CheckKeys({"schema", "name", "label", "description", "type", "precision", "roundFactor", "minWidth", "showSignOption",
                                            "formatTraits", "decimalSeparator", "thousandSeparator", "uomSeparator", "scientificType",
                                            "stationOffsetSize", "stationSeparator", "composite.spacer", "composite.includeZero", "composite.units"})
                        : parser.CheckKeys({"format", "label", "description", "type", "precision", "roundFactor", "minWidth", "showSignOption",
                                            "formatTraits", "decimalSeparator", "thousandSeparator", "uomSeparator", "scientificType",
                                            "stationOffsetSize", "stationSeparator", "composite.spacer", "composite.includeZero", "composite.units"});
        break;
    }
    if (!keysOk)
        return result;
    Utf8String owner, name;
    if (!parser.String(create ? "schema" : key, owner, true) || (create && !parser.String("name", name, true)))
        return result;
    ECSchemaCP schema;
    BeInt64Id itemId;
    ECUnitCP oldUnit = nullptr;
    ECFormatCP oldFormat = nullptr;
    UnitSystemCP oldSystem = nullptr;
    PhenomenonCP oldPhenomenon = nullptr;
    if (create) {
        schema = FindSchema(ctx, owner);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
        if (!ECNameValidation::IsValidName(name.c_str()))
            return Invalid("name", "Invalid ECName.");
    } else {
        schema = FindItemSchema(ctx, family, owner, itemId, oldUnit, oldFormat, oldSystem, oldPhenomenon);
        if (schema == nullptr)
            return Missing(key, Utf8PrintfString("%s not found; use Schema:%s.", Name(family), Name(family)));
    }
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    result = Ec32(ctx, family);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    SchemaMetaRow row;
    bvector<ECSchemaId> references;
    bvector<std::pair<UnitId, Nullable<Utf8String>>> compositeUnits;
    bvector<std::pair<BeInt64Id, Nullable<Utf8String>>> labelChanges;
    if (create || family == Family::Format) {
        ECSchemaPtr working;
        result = Copy(ctx, working, *schema);
        if (!result.IsOk())
            return result;
        ItemReferences refs(ctx, *working);
        if (create)
            row = {{"SchemaId", int64_t(schemaId.GetValue())}, {"Name", name}};
        else
            name = oldFormat->GetName();
        Utf8String label, description;
        if (!parser.String("label", label) || !parser.String("description", description))
            return result;
        Utf8CP labelArg = parser.Has("label") ? label.c_str() : nullptr;
        Utf8CP descriptionArg = parser.Has("description") ? description.c_str() : nullptr;
        if (family == Family::UnitSystem) {
            UnitSystemP system = nullptr;
            if (ECObjectsStatus::Success != working->CreateUnitSystem(system, name.c_str(), labelArg, descriptionArg) || system == nullptr)
                return Rejected("name", "Invalid or duplicate unit system name.");
            CreatedTexts(*system, row);
        } else if (family == Family::Phenomenon) {
            Utf8String definition;
            if (!parser.String("definition", definition, true))
                return result;
            if (definition.empty())
                return Invalid("definition", "The definition must not be empty.");
            PhenomenonP phenomenon = nullptr;
            if (ECObjectsStatus::Success != working->CreatePhenomenon(phenomenon, name.c_str(), definition.c_str(), labelArg, descriptionArg) || phenomenon == nullptr)
                return Rejected("name", "Invalid or duplicate phenomenon name.");
            CreatedTexts(*phenomenon, row);
            row.push_back({"Definition", phenomenon->GetDefinition()});
        } else if (family == Family::Unit) {
            result = PrepareCreateUnit(ctx, parser, result, *schema, *working, name, refs, row);
            if (!result.IsOk())
                return result;
        } else {
            result = PrepareFormat(ctx, parser, result, create, *working, name, oldFormat, refs, row, compositeUnits, labelChanges);
            if (!result.IsOk())
                return result;
        }
        result = ValidateWorking(ctx, *working);
        if (!result.IsOk())
            return result;
        references = refs.m_ids;
    } else {
        // Unit systems, phenomena and units only allow DisplayLabel/Description changes
        // (plus the narrow unit factor rule); there is nothing to validate in a working copy.
        if (family == Family::Phenomenon && parser.Has("definition")) {
            Utf8String definition;
            if (!parser.String("definition", definition))
                return result;
            if (!definition.Equals(oldPhenomenon->GetDefinition()))
                return NotSupported(family, "definition");
        }
        if (family == Family::Unit) {
            result = PrepareAlterUnit(ctx, parser, result, *oldUnit, row);
            if (!result.IsOk())
                return result;
        }
        result = AlteredTexts(parser, result, row);
        if (!result.IsOk())
            return result;
    }
    if (!create && row.empty() && labelChanges.empty())
        return Invalid(key, Utf8PrintfString("At least one %s attribute must change.", Name(family)));
    out.m_apply = [create, family, schemaId, itemId, row, references, compositeUnits, labelChanges](SchemaEditContext& apply) mutable {
        auto written = AddReferences(apply, schemaId, references);
        if (!written.IsOk())
            return written;
        auto& ids = apply.m_db.GetImpl().GetIdFactory();
        BeInt64Id id = itemId;
        if (create) {
            switch (family) {
            case Family::UnitSystem:
                id = ids.UnitSystem().NextId();
                break;
            case Family::Phenomenon:
                id = ids.Phenomenon().NextId();
                break;
            case Family::Unit:
                id = ids.Unit().NextId();
                break;
            default:
                id = ids.Format().NextId();
                break;
            }
            row.push_back({"Id", int64_t(id.GetValue())});
        }
        if (create || !row.empty()) {
            written = create ? apply.m_writer.Insert(Table(family), row) : apply.m_writer.Update(Table(family), id, row);
            if (!written.IsOk())
                return written;
        }
        for (size_t ordinal = 0; ordinal < compositeUnits.size(); ++ordinal) {
            SchemaMetaRow unitRow{{"FormatId", int64_t(id.GetValue())}};
            if (compositeUnits[ordinal].second.IsValid())
                unitRow.push_back({"Label", compositeUnits[ordinal].second.Value()});
            unitRow.push_back({"UnitId", int64_t(compositeUnits[ordinal].first.GetValue())});
            unitRow.push_back({"Ordinal", int64_t(ordinal)});
            unitRow.push_back({"Id", int64_t(ids.FormatCompositeUnit().NextId().GetValue())});
            written = apply.m_writer.Insert("ec_FormatCompositeUnit", unitRow);
            if (!written.IsOk())
                return written;
        }
        for (auto const& change : labelChanges) {
            written = apply.m_writer.Update("ec_FormatCompositeUnit", change.first,
                                            {{"Label", change.second.IsValid() ? SchemaMetaValue(change.second.Value()) : SchemaMetaValue(nullptr)}});
            if (!written.IsOk())
                return written;
        }
        Affected(apply, schemaId);
        return written;
    };
    return {};
}
}

void RegisterSchemaUnitCommands(SchemaCommandRegistry& registry) {
    for (auto family : {Family::UnitSystem, Family::Phenomenon, Family::Unit, Family::Format})
        for (auto operation : {Operation::Create, Operation::Alter})
            Register(registry, Utf8PrintfString("%s%s", operation == Operation::Create ? "Create" : "Alter", Name(family)).c_str(),
                     [family, operation](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
                         out.m_mapsClasses = false;
                         return PrepareItem(ctx, args, out, family, operation);
                     });
}
END_BENTLEY_SQLITE_EC_NAMESPACE
