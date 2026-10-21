/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"
#include "SchemaImportPlanBuilder.h"
#include <ECObjects/ECJsonUtilities.h>
#include <ECObjects/SupplementalSchema.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace ECN;
using SchemaCommands::Result;
using SchemaCommands::Status;

Utf8String Lower(Utf8StringCR value) {
    Utf8String copy(value);
    copy.ToLower();
    return copy;
}
Utf8String Qualify(Utf8StringCR schema, Utf8StringCR item) {
    return schema + ":" + item;
}
template <typename T>
Utf8String QName(T const& item) {
    return Qualify(item.GetSchema().GetName(), item.GetName());
}

Utf8CP PrimitiveToken(PrimitiveType type) {
    switch (type) {
    case PRIMITIVETYPE_Integer:
        return "int";
    case PRIMITIVETYPE_Long:
        return "long";
    case PRIMITIVETYPE_Double:
        return "double";
    case PRIMITIVETYPE_Boolean:
        return "boolean";
    case PRIMITIVETYPE_DateTime:
        return "dateTime";
    case PRIMITIVETYPE_Point2d:
        return "point2d";
    case PRIMITIVETYPE_Point3d:
        return "point3d";
    case PRIMITIVETYPE_Binary:
        return "binary";
    case PRIMITIVETYPE_IGeometry:
        return "geometry";
    default:
        return "string";
    }
}

Utf8CP ModifierToken(ECClassModifier modifier) {
    return modifier == ECClassModifier::Abstract ? "Abstract" : modifier == ECClassModifier::Sealed ? "Sealed"
                                                                                                    : "None";
}

Utf8String AppliesToToken(CustomAttributeContainerType type) {
    auto has = [type](CustomAttributeContainerType flag) { return (type & flag) == flag; };
    if (has(CustomAttributeContainerType::Any))
        return "Any";
    bvector<Utf8CP> tokens;
    if (has(CustomAttributeContainerType::Schema))
        tokens.push_back("Schema");
    if (has(CustomAttributeContainerType::AnyClass))
        tokens.push_back("AnyClass");
    else
        for (auto entry : {std::make_pair(CustomAttributeContainerType::EntityClass, "EntityClass"),
                           std::make_pair(CustomAttributeContainerType::CustomAttributeClass, "CustomAttributeClass"),
                           std::make_pair(CustomAttributeContainerType::StructClass, "StructClass"),
                           std::make_pair(CustomAttributeContainerType::RelationshipClass, "RelationshipClass")})
            if (has(entry.first))
                tokens.push_back(entry.second);
    if (has(CustomAttributeContainerType::AnyProperty))
        tokens.push_back("AnyProperty");
    else
        for (auto entry : {std::make_pair(CustomAttributeContainerType::PrimitiveProperty, "PrimitiveProperty"),
                           std::make_pair(CustomAttributeContainerType::StructProperty, "StructProperty"),
                           std::make_pair(CustomAttributeContainerType::PrimitiveArrayProperty, "PrimitiveArrayProperty"),
                           std::make_pair(CustomAttributeContainerType::StructArrayProperty, "StructArrayProperty"),
                           std::make_pair(CustomAttributeContainerType::NavigationProperty, "NavigationProperty")})
            if (has(entry.first))
                tokens.push_back(entry.second);
    if (has(CustomAttributeContainerType::AnyRelationshipConstraint))
        tokens.push_back("AnyRelationshipConstraint");
    else {
        if (has(CustomAttributeContainerType::SourceRelationshipConstraint))
            tokens.push_back("SourceRelationshipConstraint");
        if (has(CustomAttributeContainerType::TargetRelationshipConstraint))
            tokens.push_back("TargetRelationshipConstraint");
    }
    Utf8String joined;
    for (auto token : tokens) {
        if (!joined.empty())
            joined.append("|");
        joined.append(token);
    }
    return joined;
}

template <typename T>
void Texts(BeJsValue out, T const& item) {
    if (item.GetIsDisplayLabelDefined())
        out["label"] = item.GetInvariantDisplayLabel();
    if (!item.GetInvariantDescription().empty())
        out["description"] = item.GetInvariantDescription();
}

//! Removes the ECJson schema-item envelope keys so only the definition remains.
void StripItemEnvelope(BeJsValue out) {
    for (auto key : {"$schema", "schema", "schemaVersion", "name", "schemaItemType"})
        out.removeMember(key);
}

void ValueToJson(BeJsValue out, ECValueCR value) {
    if (value.IsNull())
        out.SetNull();
    else if (value.IsInteger())
        out = value.GetInteger();
    else if (value.IsLong())
        out = value.GetLong();
    else if (value.IsDouble())
        out = value.GetDouble();
    else if (value.IsBoolean())
        out = value.GetBoolean();
    else
        out = value.ToString().c_str();
}

//! Local custom attribute instances in source order, as (qualified class name, ECJson object with className).
//! Source order is kept so creation commands persist the same ordinals as a native import.
struct CustomAttributes final {
    using Entry = std::pair<Utf8String, Utf8String>;
    using const_iterator = bvector<Entry>::const_iterator;
    bvector<Entry> m_entries;

    const_iterator begin() const { return m_entries.begin(); }
    const_iterator end() const { return m_entries.end(); }
    const_iterator find(Utf8StringCR name) const {
        return std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& entry) { return entry.first == name; });
    }
    size_t count(Utf8StringCR name) const { return find(name) != end() ? 1 : 0; }
    void erase(const_iterator it) { m_entries.erase(m_entries.begin() + (it - m_entries.begin())); }
    void erase(Utf8StringCR name) {
        auto it = find(name);
        if (it != end())
            erase(it);
    }
};

void CollectCustomAttributes(IECCustomAttributeContainerCR container, CustomAttributes& out) {
    for (auto const& instance : container.GetCustomAttributes(false)) {
        BeJsDocument json;
        JsonEcInstanceWriter::WriteInstanceToSchemaJson(json, *instance, nullptr);
        auto name = QName(instance->GetClass());
        json["className"] = name;
        out.m_entries.push_back({name, json.Stringify()});
    }
}

bool HasUnboundedOrValue(ArrayECPropertyCR array, BeJsValue out) {
    out["minOccurs"] = array.GetMinOccurs();
    if (array.IsStoredMaxOccursUnbounded())
        out["maxOccurs"] = "unbounded";
    else
        out["maxOccurs"] = array.GetStoredMaxOccurs();
    return true;
}

//! Describes a property as AddProperty arguments without class/name/customAttributes. Fails for unsupported values.
bool DescribeProperty(ECPropertyCR property, BeJsValue out, Utf8StringR error) {
    auto primitive = [&](ECEnumerationCP enumeration, PrimitiveType type, bool hasExtended, Utf8StringCR extended) {
        if (enumeration != nullptr)
            out["enum"] = QName(*enumeration);
        else
            out["type"] = PrimitiveToken(type);
        if (hasExtended)
            out["extendedType"] = extended;
    };
    if (auto prim = property.GetAsPrimitiveProperty()) {
        out["kind"] = "primitive";
        primitive(prim->GetEnumeration(), prim->GetType(), prim->IsExtendedTypeDefinedLocally(), prim->GetExtendedTypeName());
    } else if (auto primArray = property.GetAsPrimitiveArrayProperty()) {
        out["kind"] = "primitiveArray";
        primitive(primArray->GetEnumeration(), primArray->GetPrimitiveElementType(), primArray->IsExtendedTypeDefinedLocally(),
                  primArray->GetExtendedTypeName());
        HasUnboundedOrValue(*primArray, out);
    } else if (auto structProp = property.GetAsStructProperty()) {
        out["kind"] = "struct";
        out["struct"] = QName(structProp->GetType());
    } else if (auto structArray = property.GetAsStructArrayProperty()) {
        out["kind"] = "structArray";
        out["struct"] = QName(structArray->GetStructElementType());
        HasUnboundedOrValue(*structArray, out);
    } else if (auto nav = property.GetAsNavigationProperty()) {
        out["kind"] = "navigation";
        if (nav->GetRelationshipClass() == nullptr) {
            error = "The navigation property has no relationship class.";
            return false;
        }
        out["relationship"] = QName(*nav->GetRelationshipClass());
        out["direction"] = nav->GetDirection() == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward";
    } else {
        error = "Unsupported property kind.";
        return false;
    }
    if (property.IsReadOnlyFlagSet())
        out["readOnly"] = true;
    if (property.IsPriorityLocallyDefined() && property.GetPriority() != 0) {
        if (property.GetPriority() < 0) {
            error = "Negative property priorities are not supported by SchemaEditor.";
            return false;
        }
        out["priority"] = property.GetPriority();
    }
    if (property.IsKindOfQuantityDefinedLocally())
        out["koq"] = QName(*property.GetKindOfQuantity());
    if (property.IsCategoryDefinedLocally())
        out["category"] = QName(*property.GetCategory());
    if (property.IsMinimumLengthDefined())
        out["minLength"] = property.GetMinimumLength();
    if (property.IsMaximumLengthDefined())
        out["maxLength"] = property.GetMaximumLength();
    ECValue value;
    if (property.IsMinimumValueDefined() && ECObjectsStatus::Success == property.GetMinimumValue(value))
        ValueToJson(out["minValue"], value);
    value.Clear();
    if (property.IsMaximumValueDefined() && ECObjectsStatus::Success == property.GetMaximumValue(value))
        ValueToJson(out["maxValue"], value);
    Texts(out, property);
    return true;
}

void DescribeConstraint(ECRelationshipConstraintCR constraint, BeJsValue out) {
    auto classes = out["classes"];
    classes.SetEmptyArray();
    for (auto ecClass : constraint.GetConstraintClasses())
        classes.appendValue() = QName(*ecClass);
    out["multiplicity"] = constraint.GetMultiplicity().ToString();
    out["polymorphic"] = constraint.GetIsPolymorphic();
    out["roleLabel"] = constraint.GetRoleLabel();
    if (constraint.IsAbstractConstraintDefined())
        out["abstractConstraint"] = QName(*constraint.GetAbstractConstraint(false));
}

//! Describes a class as create arguments without schema/name/customAttributes/mapping options.
bool DescribeClass(ECClassCR ecClass, BeJsValue out, Utf8StringR error) {
    bvector<ECClassCP> bases, mixins;
    for (auto base : ecClass.GetBaseClasses()) {
        if (ecClass.IsEntityClass() && base->IsEntityClass() && base->GetEntityClassCP()->IsMixin() && !ecClass.GetEntityClassCP()->IsMixin())
            mixins.push_back(base);
        else
            bases.push_back(base);
    }
    if (bases.size() > 1) {
        error = "Multiple non-mixin base classes are not supported.";
        return false;
    }
    if (!bases.empty())
        out["baseClass"] = QName(*bases.front());
    if (!mixins.empty()) {
        auto list = out["mixins"];
        list.SetEmptyArray();
        for (auto mixin : mixins)
            list.appendValue() = QName(*mixin);
    }
    Texts(out, ecClass);
    if (ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin()) {
        auto appliesTo = ecClass.GetEntityClassCP()->GetAppliesToClass();
        if (appliesTo == nullptr) {
            error = "The mixin has no valid AppliesToEntityClass.";
            return false;
        }
        out["appliesTo"] = QName(*appliesTo);
        return true;
    }
    out["modifier"] = ModifierToken(ecClass.GetClassModifier());
    if (auto caClass = ecClass.GetCustomAttributeClassCP())
        out["appliesTo"] = AppliesToToken(caClass->GetContainerType());
    if (auto relationship = ecClass.GetRelationshipClassCP()) {
        auto strength = relationship->GetStrength();
        out["strength"] = strength == StrengthType::Holding ? "holding" : strength == StrengthType::Embedding ? "embedding"
                                                                                                              : "referencing";
        out["direction"] = relationship->GetStrengthDirection() == ECRelatedInstanceDirection::Backward ? "backward" : "forward";
        DescribeConstraint(relationship->GetSource(), out["source"]);
        DescribeConstraint(relationship->GetTarget(), out["target"]);
    }
    return true;
}

void DescribeEnumeration(ECEnumerationCR enumeration, BeJsValue out) {
    out["backingType"] = enumeration.GetType() == PRIMITIVETYPE_Integer ? "int" : "string";
    out["isStrict"] = enumeration.GetIsStrict();
    Texts(out, enumeration);
}

void DescribeEnumerator(ECEnumeratorCR enumerator, BeJsValue out) {
    if (enumerator.IsInteger())
        out["value"] = enumerator.GetInteger();
    else
        out["value"] = enumerator.GetString();
    Texts(out, enumerator);
}

void DescribeUnit(ECUnitCR unit, BeJsValue out) {
    unit.ToJson(out, false);
    bool constant = Utf8String(out["schemaItemType"].asString()).EqualsI("Constant");
    StripItemEnvelope(out);
    if (constant) {
        out["isConstant"] = true;
        out.removeMember("unitSystem");
    }
}

void DescribeFormat(ECFormatCR format, BeJsValue out) {
    format.ToJson(out, false);
    StripItemEnvelope(out);
    if (!out.isMember("precision") && format.HasNumeric()) {
        BeJsDocument verbose;
        format.GetNumericSpec()->ToJson(verbose, true);
        if (verbose.isMember("precision"))
            out["precision"].From(verbose["precision"]);
    }
    if (format.HasComposite() && out.isMember("composite") && out["composite"].isMember("units")) {
        auto units = format.GetCompositeSpec()->GetUnits();
        auto list = out["composite"]["units"];
        for (uint32_t i = 0; i < list.size() && i < units.size(); ++i)
            if (auto unit = dynamic_cast<ECUnitCP>(units[i]))
                list[i]["name"] = unit->GetFullName();
    }
}

template <typename T>
void DescribeItem(T const& item, BeJsValue out) {
    item.ToJson(out, false);
    StripItemEnvelope(out);
}

//! Native transitions clear an optional value by sending these sentinels.
bool ClearValue(Utf8CP key, BeJsValue out) {
    for (auto text : {"label", "description", "koq", "category", "extendedType", "baseClass", "roleLabel"})
        if (0 == strcmp(key, text)) {
            out = "";
            return true;
        }
    for (auto bound : {"minLength", "maxLength", "minValue", "maxValue"})
        if (0 == strcmp(key, bound)) {
            out.SetNull();
            return true;
        }
    if (0 == strcmp(key, "priority")) {
        out = 0;
        return true;
    }
    if (0 == strcmp(key, "readOnly")) {
        out = false;
        return true;
    }
    if (0 == strcmp(key, "presentationUnits")) {
        out.SetEmptyArray();
        return true;
    }
    return false;
}

bool Contains(std::initializer_list<Utf8CP> keys, Utf8CP key) {
    for (auto candidate : keys)
        if (0 == strcmp(candidate, key))
            return true;
    return false;
}
} // namespace

namespace {
struct PlannedStep final {
    Utf8String m_schema, m_item, m_facet;
    bool m_group = false;
    bvector<std::pair<Utf8String, Utf8String>> m_commands;
};

struct DeferredAttribute final {
    Utf8String m_container, m_caClass, m_instance;
};

//! Translates normalized incoming schemas into SchemaEditor command steps.
struct Planner final {
    ECDbR m_db;
    bvector<PlannedStep>& m_steps;
    bvector<Utf8String>& m_warnings;
    Result m_result;
    ECSchemaReadContextPtr m_context = ECSchemaReadContext::CreateContext();
    bvector<ECSchemaP> m_ordered;             //!< Primary schema copies to plan, in dependency order.
    bmap<Utf8String, ECSchemaCP> m_persisted; //!< Lower-case name -> persisted counterpart (if any).
    std::set<Utf8String> m_available;         //!< Lower-case qualified names of CA classes usable now.
    bvector<DeferredAttribute> m_deferred;
    bvector<DeferredAttribute> m_indexes;
    std::set<Utf8String> m_createdNavigations; //!< Lower-case "Schema:Class.Prop" created with their relationship.
    //! Container -> CA classes appended, in order, by the container's own creation step.
    bmap<Utf8String, bvector<Utf8String>> m_creationAttributes;

    Planner(ECDbR db, bvector<PlannedStep>& steps, bvector<Utf8String>& warnings) : m_db(db), m_steps(steps), m_warnings(warnings) {}

    bool Fail(Utf8StringCR item, Utf8StringCR message) {
        if (m_result.IsOk()) {
            LOG.errorv("SchemaImportAdapter plan [%s]: %s", item.c_str(), message.c_str());
            m_result = Result::Failure(Status::ERROR_VALIDATION, "SchemaImport.Unsupported", item.c_str(), message);
        }
        return false;
    }
    void Warn(Utf8StringCR item, Utf8StringCR message) {
        Utf8PrintfString text("%s: %s", item.c_str(), message.c_str());
        LOG.warningv("SchemaImportAdapter plan: %s", text.c_str());
        m_warnings.push_back(text);
    }

    PlannedStep& NewStep(Utf8StringCR schema, Utf8StringCR item, Utf8CP facet, bool group) {
        m_steps.push_back(PlannedStep());
        auto& step = m_steps.back();
        step.m_schema = schema;
        step.m_item = item;
        step.m_facet = facet;
        step.m_group = group;
        return step;
    }
    static void Add(PlannedStep& step, Utf8CP command, BeJsConst args) { step.m_commands.push_back({command, args.Stringify()}); }
    void Emit(Utf8StringCR schema, Utf8StringCR item, Utf8CP facet, Utf8CP command, BeJsConst args) { Add(NewStep(schema, item, facet, false), command, args); }

    bool IsAvailable(Utf8StringCR caClass) const {
        if (m_available.count(Lower(caClass)) != 0)
            return true;
        Utf8String schema, item;
        return SchemaCommands::QualifiedName(caClass, schema, item) && m_db.Schemas().GetClass(schema, item, SchemaLookupMode::ByName) != nullptr;
    }
    void MarkAvailable(Utf8StringCR qualified) { m_available.insert(Lower(qualified)); }

    //! Generic leaf diff. Changed alterable keys are copied, removed alterable keys are cleared; anything else fails.
    bool Diff(Utf8StringCR item, BeJsConst oldArgs, BeJsConst newArgs, std::initializer_list<Utf8CP> alterable, BeJsValue alter) {
        bool ok = true;
        newArgs.ForEachProperty([&](Utf8CP key, BeJsConst value) {
            if (oldArgs.isMember(key) && oldArgs[key].isExactEqual(value))
                return false;
            if (!Contains(alterable, key))
                ok = Fail(item, Utf8PrintfString("Changing '%s' is not supported: native schema upgrade rules forbid it or SchemaEditor cannot represent it.", key));
            else
                alter[key].From(value);
            return !ok;
        });
        if (!ok)
            return false;
        oldArgs.ForEachProperty([&](Utf8CP key, BeJsConst) {
            if (newArgs.isMember(key))
                return false;
            // A primitive type <-> enumeration swap replaces one key with the other.
            if ((0 == strcmp(key, "type") && newArgs.isMember("enum")) || (0 == strcmp(key, "enum") && newArgs.isMember("type")))
                return false;
            if (!Contains(alterable, key) || !ClearValue(key, alter[key]))
                ok = Fail(item, Utf8PrintfString("Removing '%s' is not supported.", key));
            return !ok;
        });
        return ok;
    }

    //! Emits Set/Remove custom attribute commands that turn the old set into the new set.
    void DiffAttributes(PlannedStep* step, Utf8StringCR schema, Utf8StringCR container, CustomAttributes const& oldSet,
                        CustomAttributes const& newSet, std::function<bool(Utf8StringCR)> const& include, bool sets, bool removes) {
        for (auto const& entry : newSet) {
            if (!sets || !include(entry.first))
                continue;
            auto old = oldSet.find(entry.first);
            if (old != oldSet.end() && BeJsDocument(old->second.c_str()).isExactEqual(BeJsDocument(entry.second.c_str())))
                continue;
            SetAttribute(step, schema, container, entry.first, entry.second);
        }
        if (!removes)
            return;
        for (auto const& entry : oldSet) {
            if (newSet.count(entry.first) != 0 || !include(entry.first))
                continue;
            BeJsDocument args;
            args["container"] = container;
            args["caClass"] = entry.first;
            if (step != nullptr)
                Add(*step, "RemoveCustomAttribute", args);
            else
                Emit(schema, container, "CustomAttribute", "RemoveCustomAttribute", args);
        }
    }
    void SetAttribute(PlannedStep* step, Utf8StringCR schema, Utf8StringCR container, Utf8StringCR caClass, Utf8StringCR instance) {
        if (Lower(caClass) == "ecdbmap:dbindexlist") {
            m_indexes.push_back({container, caClass, instance});
            return;
        }
        BeJsDocument args;
        args["container"] = container;
        args["caClass"] = caClass;
        args["instance"].From(BeJsDocument(instance.c_str()));
        if (step != nullptr) {
            m_creationAttributes[container].push_back(caClass);
            Add(*step, "SetCustomAttribute", args);
        }
        else
            Emit(schema, container, "CustomAttribute", "SetCustomAttribute", args);
    }

    //! Appends inline creation CAs; unusable or excluded ones are deferred or routed to `later`.
    void InlineAttributes(BeJsValue args, Utf8StringCR container, CustomAttributes const& attributes,
                          std::function<bool(Utf8StringCR)> const& inlineAllowed, bvector<std::pair<Utf8String, Utf8String>>* later) {
        for (auto const& entry : attributes) {
            if (Lower(entry.first) == "ecdbmap:dbindexlist") {
                m_indexes.push_back({container, entry.first, entry.second});
                continue;
            }
            if (!inlineAllowed(entry.first)) {
                if (later != nullptr)
                    later->push_back(entry);
                continue;
            }
            if (!IsAvailable(entry.first)) {
                m_deferred.push_back({container, entry.first, entry.second});
                continue;
            }
            if (!args.isMember("customAttributes"))
                args["customAttributes"].SetEmptyArray();
            m_creationAttributes[container].push_back(entry.first);
            args["customAttributes"].appendValue().From(BeJsDocument(entry.second.c_str()));
        }
    }
    void FlushDeferred(Utf8StringCR schema) {
        for (auto const& entry : m_deferred)
            SetAttribute(nullptr, schema, entry.m_container, entry.m_caClass, entry.m_instance);
        m_deferred.clear();
    }

    // ---------------------------------------------------------------- normalization
    bool CollectClosure(ECSchemaCP schema, bmap<Utf8String, ECSchemaCP>& byName, bvector<ECSchemaCP>& postOrder, std::set<Utf8String>& visiting) {
        auto key = Lower(schema->GetName());
        auto found = byName.find(key);
        if (found != byName.end()) {
            if (found->second != schema && found->second->GetSchemaKey().CompareByVersion(schema->GetSchemaKey()) != 0)
                return Fail(schema->GetName(), Utf8PrintfString("The input references conflicting versions %s and %s.",
                                                                found->second->GetFullSchemaName().c_str(), schema->GetFullSchemaName().c_str()));
            return true;
        }
        if (!visiting.insert(key).second)
            return Fail(schema->GetName(), "The schema reference graph contains a cycle.");
        bvector<ECSchemaCP> references;
        for (auto const& entry : schema->GetReferencedSchemas())
            references.push_back(entry.second.get());
        std::sort(references.begin(), references.end(), [](ECSchemaCP a, ECSchemaCP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
        for (auto reference : references)
            if (!CollectClosure(reference, byName, postOrder, visiting))
                return false;
        visiting.erase(key);
        byName[key] = schema;
        postOrder.push_back(schema);
        return true;
    }

    bool Validate(ECSchemaCR schema) {
        if (schema.GetECVersion() != ECVersion::Latest)
            return Fail(schema.GetName(), Utf8PrintfString("The in-memory version of the ECSchema must be %s.", ECSchema::GetECVersionString(ECVersion::Latest)));
        if (schema.OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            return Fail(schema.GetName(), "The ECSchema has a higher ECXml version than this ECDb supports.");
        if (schema.HasId()) {
            auto id = SchemaPersistenceHelper::GetSchemaId(m_db, DbTableSpace::Main(), schema.GetName().c_str(), SchemaLookupMode::ByName);
            if (!id.IsValid() || id != schema.GetId())
                return Fail(schema.GetName(), "The ECSchema is owned by some other ECDb file.");
        }
        return true;
    }

    bool Normalize(bvector<ECSchemaCP> const& input) {
        bmap<Utf8String, ECSchemaCP> byName;
        bvector<ECSchemaCP> postOrder;
        bvector<ECSchemaCP> sortedInput(input);
        for (auto schema : sortedInput)
            if (schema == nullptr)
                return Fail("", "The input contains a null schema.");
        std::sort(sortedInput.begin(), sortedInput.end(), [](ECSchemaCP a, ECSchemaCP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
        for (auto schema : sortedInput) {
            std::set<Utf8String> visiting;
            if (!CollectClosure(schema, byName, postOrder, visiting))
                return false;
        }
        std::set<Utf8String> wasSupplemented;
        bvector<ECSchemaP> primaries, supplementals;
        for (auto source : postOrder) {
            if (!Validate(*source))
                return false;
            ECSchemaPtr copy;
            if (ECObjectsStatus::Success != source->CopySchema(copy, m_context.get()) || copy.IsNull() ||
                ECObjectsStatus::Success != copy->SetOriginalECXmlVersion(source->GetOriginalECXmlVersionMajor(), source->GetOriginalECXmlVersionMinor()) ||
                ECObjectsStatus::Success != m_context->AddSchema(*copy))
                return Fail(source->GetName(), "Cannot create an owned copy of the incoming schema.");
            if (source->IsSupplemented())
                wasSupplemented.insert(Lower(source->GetName()));
            (copy->IsSupplementalSchema() ? supplementals : primaries).push_back(copy.get());
        }
        if (!supplementals.empty())
            for (auto primary : primaries) {
                if (wasSupplemented.count(Lower(primary->GetName())) != 0)
                    continue;
                if (SupplementedSchemaStatus::Success != SupplementedSchemaBuilder().UpdateSchema(*primary, supplementals, *m_context, false))
                    return Fail(primary->GetName(), "Failed to supplement the ECSchema.");
                auto provenance = primary->GetCustomAttribute("SupplementalProvenance");
                if (provenance.IsValid() && !ECSchema::IsSchemaReferenced(*primary, provenance->GetClass().GetSchema()))
                    primary->AddReferencedSchema(const_cast<ECSchemaR>(provenance->GetClass().GetSchema()));
            }
        // Supplementation can add references, so order the primaries again.
        std::set<Utf8String> done;
        bmap<Utf8String, ECSchemaP> primaryByName;
        for (auto primary : primaries)
            primaryByName[Lower(primary->GetName())] = primary;
        std::function<void(ECSchemaP)> visit = [&](ECSchemaP schema) {
            if (!done.insert(Lower(schema->GetName())).second)
                return;
            bvector<ECSchemaP> references;
            for (auto const& entry : schema->GetReferencedSchemas()) {
                auto found = primaryByName.find(Lower(entry.second->GetName()));
                if (found != primaryByName.end())
                    references.push_back(found->second);
            }
            std::sort(references.begin(), references.end(), [](ECSchemaP a, ECSchemaP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
            for (auto reference : references)
                visit(reference);
            m_ordered.push_back(schema);
        };
        for (auto primary : primaries)
            visit(primary);
        // ECDb profile schemas are owned by the profile; never plan them.
        bvector<ECSchemaP> planned;
        for (auto schema : m_ordered) {
            auto persisted = m_db.Schemas().GetSchema(schema->GetName(), true, SchemaLookupMode::ByName);
            if (SchemaCommands::IsECDbProfileSchema(schema->GetName())) {
                if (persisted == nullptr || persisted->GetSchemaKey().CompareByVersion(schema->GetSchemaKey()) != 0)
                    Warn(schema->GetName(), "ECDb profile schemas are managed by the ECDb profile and are not imported.");
                continue;
            }
            if (persisted != nullptr)
                m_persisted[Lower(schema->GetName())] = persisted;
            planned.push_back(schema);
        }
        m_ordered = planned;
        return true;
    }

    ECSchemaCP Persisted(ECSchemaCR schema) const {
        auto found = m_persisted.find(Lower(schema.GetName()));
        return found == m_persisted.end() ? nullptr : found->second;
    }
};
} // namespace

namespace {
//! Local item names referenced by a unit/phenomenon definition expression.
bset<Utf8String> DefinitionTokens(Utf8StringCR definition, ECSchemaCR schema) {
    bset<Utf8String> tokens;
    Utf8String token;
    auto flush = [&]() {
        if (token.empty())
            return;
        auto colon = token.find(':');
        if (colon == Utf8String::npos)
            tokens.insert(Lower(token));
        else if (Utf8String(token.substr(0, colon)).EqualsI(schema.GetAlias()) || Utf8String(token.substr(0, colon)).EqualsI(schema.GetName()))
            tokens.insert(Lower(token.substr(colon + 1)));
        token.clear();
    };
    for (auto ch : definition) {
        if (isalnum((unsigned char)ch) || ch == '_' || ch == ':')
            token.push_back(ch);
        else
            flush();
    }
    flush();
    return tokens;
}

//! Deterministic topological order: items become ready once all local dependencies are emitted.
template <typename T>
bool TopologicalOrder(bvector<T const*> items, std::function<Utf8String(T const&)> const& keyOf,
                      std::function<bset<Utf8String>(T const&)> const& dependencies, bvector<T const*>& ordered) {
    std::sort(items.begin(), items.end(), [&keyOf](T const* a, T const* b) { return keyOf(*a) < keyOf(*b); });
    bset<Utf8String> pending, done;
    for (auto item : items)
        pending.insert(keyOf(*item));
    while (ordered.size() < items.size()) {
        bool progressed = false;
        for (auto item : items) {
            auto key = keyOf(*item);
            if (done.count(key) != 0)
                continue;
            bool ready = true;
            for (auto const& dependency : dependencies(*item))
                if (dependency != key && pending.count(dependency) != 0 && done.count(dependency) == 0) {
                    ready = false;
                    break;
                }
            if (!ready)
                continue;
            done.insert(key);
            ordered.push_back(item);
            progressed = true;
        }
        if (!progressed)
            return false;
    }
    return true;
}
} // namespace

namespace {
struct SchemaPhase final {
    Planner& m_planner;
    ECSchemaCR m_schema;
    ECSchemaCP m_old;
    Utf8String m_name;

    SchemaPhase(Planner& planner, ECSchemaCR schema) : m_planner(planner), m_schema(schema), m_old(planner.Persisted(schema)), m_name(schema.GetName()) {}

    bool Fail(Utf8StringCR item, Utf8StringCR message) { return m_planner.Fail(item, message); }
    void Emit(Utf8StringCR item, Utf8CP facet, Utf8CP command, BeJsConst args) { m_planner.Emit(m_name, item, facet, command, args); }

    //! Shared new/alter emission for items described by a generator.
    bool CreateOrAlter(Utf8CP facet, Utf8StringCR itemName, Utf8CP createCommand, Utf8CP alterCommand, Utf8CP alterKey,
                       BeJsConst newArgs, BeJsConst const* oldArgs, std::initializer_list<Utf8CP> alterable) {
        auto qualified = Qualify(m_name, itemName);
        if (oldArgs == nullptr) {
            BeJsDocument args;
            args["schema"] = m_name;
            args["name"] = itemName;
            newArgs.ForEachProperty([&](Utf8CP key, BeJsConst value) { args[key].From(value); return false; });
            Emit(qualified, facet, createCommand, args);
            return true;
        }
        BeJsDocument alter;
        alter.SetEmptyObject();
        if (!m_planner.Diff(qualified, *oldArgs, newArgs, alterable, alter))
            return false;
        if (alter.size() == 0)
            return true;
        alter[alterKey] = qualified;
        Emit(qualified, facet, alterCommand, alter);
        return true;
    }

    bool PlanSchemaHeader(bool& skip) {
        skip = false;
        CustomAttributes attributes;
        CollectCustomAttributes(m_schema, attributes);
        auto const& key = m_schema.GetSchemaKey();
        Utf8PrintfString version("%02" PRIu32 ".%02" PRIu32 ".%02" PRIu32, key.GetVersionRead(), key.GetVersionWrite(), key.GetVersionMinor());
        if (m_old == nullptr) {
            BeJsDocument args;
            args["name"] = m_name;
            args["alias"] = m_schema.GetAlias();
            args["version"] = version;
            Texts(args, m_schema);
            if (m_schema.GetOriginalECXmlVersionMajor() > 0) {
                args["originalECXmlVersionMajor"] = m_schema.GetOriginalECXmlVersionMajor();
                args["originalECXmlVersionMinor"] = m_schema.GetOriginalECXmlVersionMinor();
            }
            m_planner.InlineAttributes(args, m_name, attributes, [](Utf8StringCR) { return true; }, nullptr);
            Emit(m_name, "Schema", "CreateSchema", args);
            bvector<ECSchemaCP> references;
            for (auto const& entry : m_schema.GetReferencedSchemas())
                references.push_back(entry.second.get());
            std::sort(references.begin(), references.end(), [](ECSchemaCP a, ECSchemaCP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
            for (auto reference : references)
                AddReference(*reference);
            return true;
        }
        auto const& oldKey = m_old->GetSchemaKey();
        if (oldKey.GetVersionRead() == key.GetVersionRead() && oldKey.GetVersionWrite() == key.GetVersionWrite() && key.GetVersionMinor() < oldKey.GetVersionMinor()) {
            LOG.infov("SchemaImportAdapter: schema %s is skipped because a newer compatible version already exists.", m_name.c_str());
            skip = true;
            return true;
        }
        BeJsDocument oldArgs, newArgs;
        auto describe = [](ECSchemaCR schema, BeJsValue out) {
            auto const& k = schema.GetSchemaKey();
            out["alias"] = schema.GetAlias();
            out["version"] = Utf8PrintfString("%02" PRIu32 ".%02" PRIu32 ".%02" PRIu32, k.GetVersionRead(), k.GetVersionWrite(), k.GetVersionMinor());
            Texts(out, schema);
            if (schema.GetOriginalECXmlVersionMajor() > 0) {
                out["originalECXmlVersionMajor"] = schema.GetOriginalECXmlVersionMajor();
                out["originalECXmlVersionMinor"] = schema.GetOriginalECXmlVersionMinor();
            }
        };
        describe(*m_old, oldArgs);
        describe(m_schema, newArgs);
        BeJsDocument alter;
        alter.SetEmptyObject();
        if (!m_planner.Diff(m_name, oldArgs, newArgs, {"alias", "version", "label", "description", "originalECXmlVersionMajor", "originalECXmlVersionMinor"}, alter))
            return false;
        // Removing the original ECXml version is not representable; native keeps the stored value as well.
        if (alter.isMember("originalECXmlVersionMajor") && !alter.isMember("originalECXmlVersionMinor"))
            alter["originalECXmlVersionMinor"] = m_schema.GetOriginalECXmlVersionMinor();
        if (alter.isMember("originalECXmlVersionMinor") && !alter.isMember("originalECXmlVersionMajor"))
            alter["originalECXmlVersionMajor"] = m_schema.GetOriginalECXmlVersionMajor();
        if (alter.size() != 0) {
            alter["schema"] = m_name;
            Emit(m_name, "Schema", "AlterSchema", alter);
        }
        for (auto const& entry : m_schema.GetReferencedSchemas()) {
            bool exists = false;
            for (auto const& oldEntry : m_old->GetReferencedSchemas())
                if (oldEntry.second->GetName().EqualsI(entry.second->GetName())) {
                    exists = true;
                    break;
                }
            if (!exists)
                AddReference(*entry.second);
        }
        return true;
    }

    void AddReference(ECSchemaCR reference) {
        BeJsDocument args;
        args["schema"] = m_name;
        args["reference"] = reference.GetName();
        auto const& k = reference.GetSchemaKey();
        args["version"] = Utf8PrintfString("%02" PRIu32 ".%02" PRIu32 ".%02" PRIu32, k.GetVersionRead(), k.GetVersionWrite(), k.GetVersionMinor());
        Emit(m_name, "SchemaReference", "AddSchemaReference", args);
    }

    bool PlanEnumerations() {
        bvector<ECEnumerationCP> enumerations;
        for (auto enumeration : m_schema.GetEnumerations())
            enumerations.push_back(enumeration);
        std::sort(enumerations.begin(), enumerations.end(), [](ECEnumerationCP a, ECEnumerationCP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
        for (auto enumeration : enumerations) {
            auto qualified = QName(*enumeration);
            auto oldEnum = m_old != nullptr ? m_old->GetEnumerationCP(enumeration->GetName().c_str()) : nullptr;
            BeJsDocument newArgs;
            DescribeEnumeration(*enumeration, newArgs);
            if (oldEnum == nullptr) {
                BeJsDocument args;
                args["schema"] = m_name;
                args["name"] = enumeration->GetName();
                newArgs.ForEachProperty([&](Utf8CP key, BeJsConst value) { args[key].From(value); return false; });
                auto list = args["enumerators"];
                list.SetEmptyArray();
                for (auto enumerator : enumeration->GetEnumerators()) {
                    auto entry = list.appendObject();
                    entry["name"] = enumerator->GetName();
                    DescribeEnumerator(*enumerator, entry);
                }
                Emit(qualified, "Enumeration", "CreateEnumeration", args);
                continue;
            }
            BeJsDocument oldArgs, alter;
            DescribeEnumeration(*oldEnum, oldArgs);
            alter.SetEmptyObject();
            if (!m_planner.Diff(qualified, oldArgs, newArgs, {"isStrict", "label", "description"}, alter))
                return false;
            if (alter.size() != 0) {
                alter["enum"] = qualified;
                Emit(qualified, "Enumeration", "AlterEnumeration", alter);
            }
            if (!PlanEnumerators(*oldEnum, *enumeration))
                return false;
        }
        return true;
    }

    bool PlanEnumerators(ECEnumerationCR oldEnum, ECEnumerationCR newEnum) {
        auto qualified = QName(newEnum);
        auto valueOf = [](ECEnumeratorCR e) { BeJsDocument json; DescribeEnumerator(e, json); return json["value"].Stringify(); };
        std::set<Utf8String> renamedFrom;
        for (auto enumerator : newEnum.GetEnumerators()) {
            auto item = qualified + "." + enumerator->GetName();
            BeJsDocument newArgs;
            DescribeEnumerator(*enumerator, newArgs);
            auto old = oldEnum.FindEnumeratorByName(enumerator->GetName().c_str());
            Utf8String renameFrom;
            if (old == nullptr) {
                // Native only recognizes renames that keep the value of an enumerator missing from the new definition.
                for (auto candidate : oldEnum.GetEnumerators())
                    if (newEnum.FindEnumeratorByName(candidate->GetName().c_str()) == nullptr && valueOf(*candidate) == valueOf(*enumerator)) {
                        old = candidate;
                        renameFrom = candidate->GetName();
                        break;
                    }
                if (old == nullptr) {
                    BeJsDocument args;
                    args["enum"] = qualified;
                    args["enumerator"] = enumerator->GetName();
                    newArgs.ForEachProperty([&](Utf8CP key, BeJsConst value) { args[key].From(value); return false; });
                    Emit(item, "Enumerator", "AddEnumerator", args);
                    continue;
                }
                renamedFrom.insert(Lower(renameFrom));
            }
            BeJsDocument oldArgs, alter;
            DescribeEnumerator(*old, oldArgs);
            alter.SetEmptyObject();
            if (!m_planner.Diff(item, oldArgs, newArgs, {"label", "description"}, alter))
                return false;
            if (!renameFrom.empty())
                alter["name"] = enumerator->GetName();
            if (alter.size() == 0)
                continue;
            alter["enum"] = qualified;
            alter["enumerator"] = old->GetName();
            Emit(item, "Enumerator", "AlterEnumerator", alter);
        }
        for (auto old : oldEnum.GetEnumerators()) {
            if (newEnum.FindEnumeratorByName(old->GetName().c_str()) != nullptr || renamedFrom.count(Lower(old->GetName())) != 0)
                continue;
            BeJsDocument args;
            args["enum"] = qualified;
            args["enumerator"] = old->GetName();
            Emit(qualified + "." + old->GetName(), "Enumerator", "DropEnumerator", args);
        }
        return true;
    }

    bool PlanUnitsFamily() {
        for (auto system : m_schema.GetUnitSystems()) {
            BeJsDocument newArgs, oldArgs;
            DescribeItem(*system, newArgs);
            auto old = m_old != nullptr ? m_old->GetUnitSystemCP(system->GetName().c_str()) : nullptr;
            if (old != nullptr)
                DescribeItem(*old, oldArgs);
            if (!CreateOrAlter("UnitSystem", system->GetName(), "CreateUnitSystem", "AlterUnitSystem", "unitSystem", newArgs,
                               old != nullptr ? &static_cast<BeJsConst const&>(oldArgs) : nullptr, {"label", "description"}))
                return false;
        }
        bvector<PhenomenonCP> phenomena, orderedPhenomena;
        for (auto phenomenon : m_schema.GetPhenomena())
            phenomena.push_back(phenomenon);
        if (!TopologicalOrder<Phenomenon>(phenomena, [](Phenomenon const& p) { return Lower(p.GetName()); }, [this](Phenomenon const& p) { return DefinitionTokens(p.GetDefinition(), m_schema); }, orderedPhenomena))
            return Fail(m_name, "Phenomenon definitions contain a cycle.");
        for (auto phenomenon : orderedPhenomena) {
            BeJsDocument newArgs, oldArgs;
            DescribeItem(*phenomenon, newArgs);
            auto old = m_old != nullptr ? m_old->GetPhenomenonCP(phenomenon->GetName().c_str()) : nullptr;
            if (old != nullptr)
                DescribeItem(*old, oldArgs);
            if (!CreateOrAlter("Phenomenon", phenomenon->GetName(), "CreatePhenomenon", "AlterPhenomenon", "phenomenon", newArgs,
                               old != nullptr ? &static_cast<BeJsConst const&>(oldArgs) : nullptr, {"definition", "label", "description"}))
                return false;
        }
        bvector<ECUnitCP> regular, inverted, ordered;
        for (auto unit : m_schema.GetUnits())
            (unit->IsInvertedUnit() ? inverted : regular).push_back(unit);
        if (!TopologicalOrder<ECUnit>(regular, [](ECUnit const& u) { return Lower(u.GetName()); }, [this](ECUnit const& u) { return u.HasDefinition() ? DefinitionTokens(u.GetDefinition(), m_schema) : bset<Utf8String>(); }, ordered))
            return Fail(m_name, "Unit definitions contain a cycle.");
        std::sort(inverted.begin(), inverted.end(), [](ECUnitCP a, ECUnitCP b) { return a->GetName().CompareToI(b->GetName()) < 0; });
        ordered.insert(ordered.end(), inverted.begin(), inverted.end());
        for (auto unit : ordered) {
            BeJsDocument newArgs, oldArgs;
            DescribeUnit(*unit, newArgs);
            auto old = m_old != nullptr ? m_old->GetUnitCP(unit->GetName().c_str()) : nullptr;
            if (old != nullptr)
                DescribeUnit(*old, oldArgs);
            if (!CreateOrAlter("Unit", unit->GetName(), "CreateUnit", "AlterUnit", "unit", newArgs,
                               old != nullptr ? &static_cast<BeJsConst const&>(oldArgs) : nullptr, {"label", "description"}))
                return false;
        }
        for (auto format : m_schema.GetFormats()) {
            BeJsDocument newArgs, oldArgs;
            DescribeFormat(*format, newArgs);
            auto old = m_old != nullptr ? m_old->GetFormatCP(format->GetName().c_str()) : nullptr;
            if (old == nullptr) {
                if (!CreateOrAlter("Format", format->GetName(), "CreateFormat", "AlterFormat", "format", newArgs, nullptr, {}))
                    return false;
                continue;
            }
            DescribeFormat(*old, oldArgs);
            if (!PlanFormatAlter(*format, oldArgs, newArgs))
                return false;
        }
        return true;
    }

    //! Numeric and composite specs are replaced as a whole so the editor validates a coherent definition.
    bool PlanFormatAlter(ECFormatCR format, BeJsConst oldArgs, BeJsConst newArgs) {
        auto qualified = QName(format);
        BeJsDocument alter;
        alter.SetEmptyObject();
        bool numericChanged = false;
        auto isText = [](Utf8CP key) { return 0 == strcmp(key, "label") || 0 == strcmp(key, "description"); };
        auto check = [&](BeJsConst from, BeJsConst to) {
            to.ForEachProperty([&](Utf8CP key, BeJsConst value) {
                if (isText(key) || 0 == strcmp(key, "composite"))
                    return false;
                if (!from.isMember(key) || !from[key].isExactEqual(value))
                    numericChanged = true;
                return false;
            });
        };
        check(oldArgs, newArgs);
        check(newArgs, oldArgs);
        if (numericChanged)
            newArgs.ForEachProperty([&](Utf8CP key, BeJsConst value) { if (!isText(key) && 0 != strcmp(key, "composite")) alter[key].From(value); return false; });
        if (newArgs.isMember("composite") != oldArgs.isMember("composite") ||
            (newArgs.isMember("composite") && !newArgs["composite"].isExactEqual(oldArgs["composite"]))) {
            if (!newArgs.isMember("composite"))
                return Fail(qualified, "Removing a composite specification from a format is not supported.");
            alter["composite"].From(newArgs["composite"]);
        }
        for (auto key : {"label", "description"}) {
            if (newArgs.isMember(key) && (!oldArgs.isMember(key) || !oldArgs[key].isExactEqual(newArgs[key])))
                alter[key].From(newArgs[key]);
            else if (!newArgs.isMember(key) && oldArgs.isMember(key))
                alter[key] = "";
        }
        if (alter.size() == 0)
            return true;
        alter["format"] = qualified;
        Emit(qualified, "Format", "AlterFormat", alter);
        return true;
    }

    bool PlanQuantitiesAndCategories() {
        for (auto koq : m_schema.GetKindOfQuantities()) {
            BeJsDocument newArgs, oldArgs;
            DescribeItem(*koq, newArgs);
            auto old = m_old != nullptr ? m_old->GetKindOfQuantityCP(koq->GetName().c_str()) : nullptr;
            if (old != nullptr)
                DescribeItem(*old, oldArgs);
            if (!CreateOrAlter("KindOfQuantity", koq->GetName(), "CreateKindOfQuantity", "AlterKindOfQuantity", "koq", newArgs,
                               old != nullptr ? &static_cast<BeJsConst const&>(oldArgs) : nullptr,
                               {"persistenceUnit", "relativeError", "presentationUnits", "label", "description"}))
                return false;
        }
        for (auto category : m_schema.GetPropertyCategories()) {
            BeJsDocument newArgs, oldArgs;
            DescribeItem(*category, newArgs);
            auto old = m_old != nullptr ? m_old->GetPropertyCategoryCP(category->GetName().c_str()) : nullptr;
            if (old != nullptr)
                DescribeItem(*old, oldArgs);
            if (!CreateOrAlter("PropertyCategory", category->GetName(), "CreatePropertyCategory", "AlterPropertyCategory", "category", newArgs,
                               old != nullptr ? &static_cast<BeJsConst const&>(oldArgs) : nullptr, {"priority", "label", "description"}))
                return false;
        }
        return true;
    }
};
} // namespace

namespace {
Utf8String ClassKey(ECClassCR ecClass) {
    return Lower(QName(ecClass));
}
bool IsMixinClass(ECClassCR ecClass) {
    return ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin();
}
Utf8CP KindName(ECClassCR ecClass) {
    if (ecClass.IsRelationshipClass())
        return "RelationshipClass";
    if (ecClass.IsStructClass())
        return "StructClass";
    if (ecClass.IsCustomAttributeClass())
        return "CustomAttributeClass";
    return IsMixinClass(ecClass) ? "Mixin" : "EntityClass";
}
bool IsCreationOnlyAttribute(Utf8StringCR caClass) {
    auto lower = Lower(caClass);
    return lower == "corecustomattributes:classhascurrenttimestampproperty" || lower == "ecdbmap:queryview";
}

//! Plans class-level items. Shared by per-schema (struct/CA class) and global (entity/relationship) phases.
struct ClassPlanner final {
    Planner& m_planner;
    explicit ClassPlanner(Planner& planner) : m_planner(planner) {}

    bool Fail(Utf8StringCR item, Utf8StringCR message) { return m_planner.Fail(item, message); }

    ECClassCP OldClass(ECClassCR ecClass) const {
        auto old = m_planner.Persisted(ecClass.GetSchema());
        return old != nullptr ? old->GetClassCP(ecClass.GetName().c_str()) : nullptr;
    }

    //! Converts an incoming ForeignKeyConstraint into fkOnDelete and removes it from the attribute set.
    bool TakeForeignKey(Utf8StringCR item, CustomAttributes& attributes, BeJsValue args) {
        auto found = attributes.find("ECDbMap:ForeignKeyConstraint");
        if (found == attributes.end())
            return true;
        BeJsDocument instance(found->second.c_str());
        if (instance.isMember("OnUpdateAction") && !instance["OnUpdateAction"].isNull())
            return Fail(item, "ForeignKeyConstraint.OnUpdateAction is not supported by SchemaEditor.");
        args["fkOnDelete"] = instance.isMember("OnDeleteAction") && instance["OnDeleteAction"].isString() ? instance["OnDeleteAction"].asString().c_str() : "Default";
        attributes.erase(found);
        return true;
    }

    bool AddPropertyCommand(PlannedStep& step, ECClassCR owner, ECPropertyCR property) {
        auto item = QName(owner) + "." + property.GetName();
        BeJsDocument args;
        args["class"] = QName(owner);
        args["name"] = property.GetName();
        Utf8String error;
        if (!DescribeProperty(property, args, error))
            return Fail(item, error);
        CustomAttributes attributes;
        CollectCustomAttributes(property, attributes);
        if (property.GetIsNavigation() && !TakeForeignKey(item, attributes, args))
            return false;
        m_planner.InlineAttributes(args, item, attributes, [](Utf8StringCR) { return true; }, nullptr);
        Planner::Add(step, "AddProperty", args);
        return true;
    }

    using NavigationHandler = std::function<bool(PlannedStep&, NavigationECPropertyCR)>;

    //! Builds a create group: the class, its local properties in declared order and creation-only attributes.
    //! Navigation properties are delegated to @p navigations (skipped if null, i.e. planned later).
    bool CreateClass(ECClassCR ecClass, NavigationHandler const* navigations) {
        auto qualified = QName(ecClass);
        BeJsDocument args;
        Utf8String error;
        if (!DescribeClass(ecClass, args, error))
            return Fail(qualified, error);
        args["schema"] = ecClass.GetSchema().GetName();
        args["name"] = ecClass.GetName();
        CustomAttributes attributes;
        CollectCustomAttributes(ecClass, attributes);
        bvector<std::pair<Utf8String, Utf8String>> later;
        Utf8CP command = "CreateEntityClass";
        auto& step = m_planner.NewStep(ecClass.GetSchema().GetName(), qualified, KindName(ecClass), true);
        if (IsMixinClass(ecClass)) {
            command = "CreateMixin";
            attributes.erase("CoreCustomAttributes:IsMixin");
            for (auto const& entry : attributes)
                later.push_back(entry);
        } else {
            if (ecClass.IsStructClass())
                command = "CreateStructClass";
            else if (ecClass.IsCustomAttributeClass())
                command = "CreateCustomAttributeClass";
            m_planner.InlineAttributes(args, qualified, attributes, [](Utf8StringCR name) { return !IsCreationOnlyAttribute(name); }, &later);
        }
        Planner::Add(step, command, args);
        for (auto property : ecClass.GetProperties(false)) {
            if (auto navigation = property->GetAsNavigationProperty()) {
                if (navigations != nullptr && !(*navigations)(step, *navigation))
                    return false;
                continue;
            }
            if (!AddPropertyCommand(step, ecClass, *property))
                return false;
        }
        for (auto const& entry : later) {
            if (!m_planner.IsAvailable(entry.first)) {
                m_planner.m_deferred.push_back({qualified, entry.first, entry.second});
                continue;
            }
            m_planner.SetAttribute(&step, ecClass.GetSchema().GetName(), qualified, entry.first, entry.second);
        }
        if (ecClass.IsCustomAttributeClass())
            m_planner.MarkAvailable(qualified);
        return true;
    }

    bool AlterProperties(ECClassCR newClass, ECClassCR oldClass, bool navigationsLater) {
        auto qualified = QName(newClass);
        auto schema = newClass.GetSchema().GetName();
        for (auto property : newClass.GetProperties(false)) {
            auto item = qualified + "." + property->GetName();
            auto old = oldClass.GetPropertyP(property->GetName().c_str(), false);
            if (old == nullptr) {
                if (property->GetIsNavigation() && navigationsLater)
                    continue;
                if (!AddPropertyCommand(m_planner.NewStep(schema, item, "Property", false), newClass, *property))
                    return false;
                continue;
            }
            BeJsDocument oldArgs, newArgs, alter;
            Utf8String error;
            if (!DescribeProperty(*old, oldArgs, error) || !DescribeProperty(*property, newArgs, error))
                return Fail(item, error);
            alter.SetEmptyObject();
            if (!m_planner.Diff(item, oldArgs, newArgs, {"type", "enum", "minOccurs", "maxOccurs", "readOnly", "label", "description", "priority", "extendedType", "koq", "category", "minLength", "maxLength", "minValue", "maxValue"}, alter))
                return false;
            if (alter.size() != 0) {
                alter["class"] = qualified;
                alter["property"] = property->GetName();
                m_planner.Emit(schema, item, "Property", "AlterProperty", alter);
            }
            CustomAttributes oldAttributes, newAttributes;
            CollectCustomAttributes(*old, oldAttributes);
            CollectCustomAttributes(*property, newAttributes);
            m_planner.DiffAttributes(nullptr, schema, item, oldAttributes, newAttributes, [](Utf8StringCR) { return true; }, true, true);
        }
        return true;
    }

    bool AlterClass(ECClassCR newClass, ECClassCR oldClass, bool navigationsLater) {
        auto qualified = QName(newClass);
        auto schema = newClass.GetSchema().GetName();
        if (0 != strcmp(KindName(newClass), KindName(oldClass)))
            return Fail(qualified, Utf8PrintfString("Changing a %s into a %s is not supported.", KindName(oldClass), KindName(newClass)));
        if (newClass.IsRelationshipClass())
            return AlterRelationship(*newClass.GetRelationshipClassCP(), *oldClass.GetRelationshipClassCP(), navigationsLater);
        CustomAttributes oldAttributes, newAttributes;
        CollectCustomAttributes(oldClass, oldAttributes);
        CollectCustomAttributes(newClass, newAttributes);
        auto all = [](Utf8StringCR) { return true; };
        m_planner.DiffAttributes(nullptr, schema, qualified, oldAttributes, newAttributes, all, true, false);

        BeJsDocument oldArgs, newArgs, alter;
        Utf8String error;
        if (!DescribeClass(oldClass, oldArgs, error) || !DescribeClass(newClass, newArgs, error))
            return Fail(qualified, error);
        bvector<Utf8String> oldMixins, newMixins;
        auto takeMixins = [](BeJsValue args, bvector<Utf8String>& out) {
            if (args.isMember("mixins"))
                for (uint32_t i = 0; i < args["mixins"].size(); ++i)
                    out.push_back(args["mixins"][i].asString());
            args.removeMember("mixins");
        };
        takeMixins(oldArgs, oldMixins);
        takeMixins(newArgs, newMixins);
        Utf8CP command = "AlterEntityClass";
        std::initializer_list<Utf8CP> entityKeys = {"modifier", "baseClass", "label", "description"};
        std::initializer_list<Utf8CP> mixinKeys = {"label", "description"};
        std::initializer_list<Utf8CP> caKeys = {"modifier", "appliesTo", "label", "description"};
        auto keys = entityKeys;
        if (IsMixinClass(newClass)) {
            // The mixin's AppliesToEntityClass is carried by the IsMixin attribute diff.
            oldArgs.removeMember("appliesTo");
            newArgs.removeMember("appliesTo");
            keys = mixinKeys;
        } else if (newClass.IsStructClass())
            command = "AlterStructClass";
        else if (newClass.IsCustomAttributeClass()) {
            command = "AlterCustomAttributeClass";
            keys = caKeys;
        }
        alter.SetEmptyObject();
        if (!m_planner.Diff(qualified, oldArgs, newArgs, keys, alter))
            return false;
        if (alter.size() != 0) {
            alter["class"] = qualified;
            m_planner.Emit(schema, qualified, KindName(newClass), command, alter);
        }
        for (auto const& mixin : newMixins)
            if (std::find_if(oldMixins.begin(), oldMixins.end(), [&](Utf8StringCR m) { return m.EqualsI(mixin); }) == oldMixins.end()) {
                BeJsDocument args;
                args["class"] = qualified;
                args["mixin"] = mixin;
                m_planner.Emit(schema, qualified, "Mixin", "AddMixin", args);
            }
        for (auto const& mixin : oldMixins)
            if (std::find_if(newMixins.begin(), newMixins.end(), [&](Utf8StringCR m) { return m.EqualsI(mixin); }) == newMixins.end()) {
                BeJsDocument args;
                args["class"] = qualified;
                args["mixin"] = mixin;
                m_planner.Emit(schema, qualified, "Mixin", "RemoveMixin", args);
            }
        if (!AlterProperties(newClass, oldClass, navigationsLater))
            return false;
        m_planner.DiffAttributes(nullptr, schema, qualified, oldAttributes, newAttributes, all, false, true);
        return true;
    }

    bool AlterRelationship(ECRelationshipClassCR newClass, ECRelationshipClassCR oldClass, bool navigationsLater) {
        auto qualified = QName(newClass);
        auto schema = newClass.GetSchema().GetName();
        auto all = [](Utf8StringCR) { return true; };
        CustomAttributes oldAttributes, newAttributes;
        CollectCustomAttributes(oldClass, oldAttributes);
        CollectCustomAttributes(newClass, newAttributes);
        m_planner.DiffAttributes(nullptr, schema, qualified, oldAttributes, newAttributes, all, true, false);
        BeJsDocument oldArgs, newArgs, alter;
        Utf8String error;
        if (!DescribeClass(oldClass, oldArgs, error) || !DescribeClass(newClass, newArgs, error))
            return Fail(qualified, error);
        alter.SetEmptyObject();
        for (auto end : {"source", "target"}) {
            BeJsDocument endAlter;
            endAlter.SetEmptyObject();
            if (!m_planner.Diff(qualified + "." + end, oldArgs[end], newArgs[end], {"classes", "multiplicity", "polymorphic", "roleLabel", "abstractConstraint"}, endAlter))
                return false;
            if (endAlter.size() != 0)
                alter[end].From(endAlter);
            oldArgs.removeMember(end);
            newArgs.removeMember(end);
        }
        if (!m_planner.Diff(qualified, oldArgs, newArgs, {"label", "description", "strength", "direction"}, alter))
            return false;
        if (alter.size() != 0) {
            alter["class"] = qualified;
            m_planner.Emit(schema, qualified, "RelationshipClass", "AlterRelationshipClass", alter);
        }
        for (auto end : {"source", "target"}) {
            CustomAttributes oldEnd, newEnd;
            bool isSource = 0 == strcmp(end, "source");
            CollectCustomAttributes(isSource ? oldClass.GetSource() : oldClass.GetTarget(), oldEnd);
            CollectCustomAttributes(isSource ? newClass.GetSource() : newClass.GetTarget(), newEnd);
            m_planner.DiffAttributes(nullptr, schema, qualified + "." + end, oldEnd, newEnd, all, true, true);
        }
        if (!AlterProperties(newClass, oldClass, navigationsLater))
            return false;
        m_planner.DiffAttributes(nullptr, schema, qualified, oldAttributes, newAttributes, all, false, true);
        return true;
    }
};
} // namespace

namespace {
//! Orchestrates the phases over the normalized schema graph.
struct PlanPhases final {
    Planner& m_planner;
    ClassPlanner m_classes;
    bvector<ECSchemaCP> m_schemas;                                                 //!< Non-skipped schemas in dependency order.
    bmap<Utf8String, bvector<NavigationECPropertyCP>> m_navigationsByRelationship; //!< New navigation properties per relationship key.
    bmap<Utf8String, NavigationECPropertyCP> m_relationshipNavigation;             //!< Navigation created together with a new FK root.
    bmap<Utf8String, ECClassCP> m_nodes;                                           //!< Entity and relationship classes of the import by key.
    bmap<Utf8String, bset<Utf8String>> m_navigationDependencies;                   //!< Extra ordering edges introduced by inline navigations.
    bmap<Utf8String, Utf8String> m_absorbedInto;                                   //!< New FK root relationship key -> owning class key.
    bmap<Utf8String, ECRelationshipClassCP> m_inlineNavigations;                   //!< Navigation key -> relationship created with it (null: AddProperty).

    explicit PlanPhases(Planner& planner) : m_planner(planner), m_classes(planner) {}

    bool Fail(Utf8StringCR item, Utf8StringCR message) { return m_planner.Fail(item, message); }
    Utf8String PropertyKey(ECPropertyCR property) const { return Lower(QName(property.GetClass()) + "." + property.GetName()); }

    template <typename Predicate>
    bvector<ECClassCP> Classes(Predicate const& predicate) const {
        bvector<ECClassCP> classes;
        for (auto schema : m_schemas)
            for (auto ecClass : schema->GetClasses())
                if (predicate(*ecClass))
                    classes.push_back(ecClass);
        return classes;
    }

    bool Order(bvector<ECClassCP> const& classes, std::function<bset<Utf8String>(ECClassCR)> const& extra, bvector<ECClassCP>& ordered, Utf8CP what) {
        if (TopologicalOrder<ECClass>(classes, [](ECClassCR c) { return ClassKey(c); }, [&extra](ECClassCR c) {
                bset<Utf8String> dependencies;
                for (auto base : c.GetBaseClasses()) dependencies.insert(ClassKey(*base));
                if (extra) for (auto const& key : extra(c)) dependencies.insert(key);
                return dependencies; }, ordered))
            return true;
        return Fail(what, "The class dependency graph contains a cycle.");
    }

    bool PlanSchemas() {
        for (auto schema : m_planner.m_ordered) {
            SchemaPhase phase(m_planner, *schema);
            bool skip = false;
            size_t firstStep = m_planner.m_steps.size();
            if (!phase.PlanSchemaHeader(skip))
                return false;
            if (skip)
                continue;
            m_schemas.push_back(schema);
            if (!phase.PlanEnumerations() || !phase.PlanUnitsFamily() || !phase.PlanQuantitiesAndCategories())
                return false;
            if (!PlanStructsAndAttributeClasses(*schema))
                return false;
            m_planner.FlushDeferred(schema->GetName());
            if (phase.m_old != nullptr) {
                CustomAttributes oldAttributes, newAttributes;
                CollectCustomAttributes(*phase.m_old, oldAttributes);
                CollectCustomAttributes(*schema, newAttributes);
                m_planner.DiffAttributes(nullptr, schema->GetName(), schema->GetName(), oldAttributes, newAttributes, [](Utf8StringCR) { return true; }, true, true);
            }
            WarnUnchangedVersion(*schema, phase.m_old, firstStep);
        }
        return true;
    }

    void WarnUnchangedVersion(ECSchemaCR schema, ECSchemaCP old, size_t firstStep) {
        if (old == nullptr || old->GetSchemaKey().CompareByVersion(schema.GetSchemaKey()) != 0)
            return;
        if (schema.IsDynamicSchema())
            return;
        for (size_t i = firstStep; i < m_planner.m_steps.size(); ++i)
            if (!m_planner.m_steps[i].m_facet.Equals("SchemaReference")) {
                m_planner.Warn(schema.GetName(), "The schema has changes but its version was not incremented.");
                return;
            }
    }

    bool PlanStructsAndAttributeClasses(ECSchemaCR schema) {
        auto local = [&schema](ECClassCR c) { return &c.GetSchema() == &schema; };
        bvector<ECClassCP> structs, ordered;
        for (auto ecClass : schema.GetClasses())
            if (ecClass->IsStructClass() && local(*ecClass))
                structs.push_back(ecClass);
        if (!Order(structs, [](ECClassCR c) {
                bset<Utf8String> dependencies;
                for (auto property : c.GetProperties(false))
                    {
                    if (auto structProp = property->GetAsStructProperty()) dependencies.insert(ClassKey(structProp->GetType()));
                    if (auto structArray = property->GetAsStructArrayProperty()) dependencies.insert(ClassKey(structArray->GetStructElementType()));
                    }
                return dependencies; }, ordered, schema.GetName().c_str()))
            return false;
        bvector<ECClassCP> caClasses;
        for (auto ecClass : schema.GetClasses())
            if (ecClass->IsCustomAttributeClass())
                caClasses.push_back(ecClass);
        bvector<ECClassCP> orderedCA;
        if (!Order(caClasses, nullptr, orderedCA, schema.GetName().c_str()))
            return false;
        ordered.insert(ordered.end(), orderedCA.begin(), orderedCA.end());
        for (auto ecClass : ordered) {
            auto old = m_classes.OldClass(*ecClass);
            if (!(old == nullptr ? m_classes.CreateClass(*ecClass, nullptr) : m_classes.AlterClass(*ecClass, *old, false)))
                return false;
        }
        return true;
    }

    bool IsNewProperty(ECPropertyCR property) const {
        auto old = m_classes.OldClass(property.GetClass());
        return old == nullptr || old->GetPropertyP(property.GetName().c_str(), false) == nullptr;
    }

    //! Mirrors native relationship mapping classification for roots.
    Utf8CP ClassifyRoot(ECRelationshipClassCR relationship) {
        if (relationship.GetCustomAttributeLocal("ECDbMap", "LinkTableRelationshipMap").IsValid())
            return "linkTable";
        if (relationship.GetPropertyCount(false) > 0)
            return "linkTable";
        if (relationship.GetSource().GetMultiplicity().GetUpperLimit() > 1 && relationship.GetTarget().GetMultiplicity().GetUpperLimit() > 1)
            return "linkTable";
        return m_navigationsByRelationship.count(ClassKey(relationship)) != 0 ? "foreignKey" : "linkTable";
    }

    bool PrepareNavigations() {
        for (auto ecClass : Classes([](ECClassCR c) { return c.IsEntityClass() || c.IsRelationshipClass(); })) {
            m_nodes[ClassKey(*ecClass)] = ecClass;
            for (auto property : ecClass->GetProperties(false))
                if (auto nav = property->GetAsNavigationProperty())
                    if (IsNewProperty(*nav) && nav->GetRelationshipClass() != nullptr)
                        m_navigationsByRelationship[ClassKey(*nav->GetRelationshipClass())].push_back(nav);
        }
        for (auto& entry : m_navigationsByRelationship)
            std::sort(entry.second.begin(), entry.second.end(), [this](NavigationECPropertyCP a, NavigationECPropertyCP b) { return PropertyKey(*a) < PropertyKey(*b); });
        for (auto ecClass : Classes([](ECClassCR c) { return c.IsRelationshipClass(); })) {
            if (m_classes.OldClass(*ecClass) != nullptr || ecClass->HasBaseClasses())
                continue;
            auto found = m_navigationsByRelationship.find(ClassKey(*ecClass));
            if (found == m_navigationsByRelationship.end() || 0 != strcmp(ClassifyRoot(*ecClass->GetRelationshipClassCP()), "foreignKey"))
                continue;
            m_relationshipNavigation[ClassKey(*ecClass)] = found->second.front();
            m_planner.m_createdNavigations.insert(PropertyKey(*found->second.front()));
        }
        PlaceInlineNavigations();
        return true;
    }

    //! Ordering edges that hold regardless of navigation placement.
    bset<Utf8String> StructuralDependencies(ECClassCR ecClass) const {
        bset<Utf8String> dependencies;
        for (auto base : ecClass.GetBaseClasses())
            dependencies.insert(ClassKey(*base));
        if (IsMixinClass(ecClass) && ecClass.GetEntityClassCP()->GetAppliesToClass() != nullptr)
            dependencies.insert(ClassKey(*ecClass.GetEntityClassCP()->GetAppliesToClass()));
        if (auto relationship = ecClass.GetRelationshipClassCP())
            for (auto constraint : {&relationship->GetSource(), &relationship->GetTarget()})
                for (auto constraintClass : constraint->GetConstraintClasses()) {
                    dependencies.insert(ClassKey(*constraintClass));
                    // Mixins have no physical table; polymorphic constraints resolve their consumers' tables.
                    if (constraint->GetIsPolymorphic() && IsMixinClass(*constraintClass))
                        for (auto const& node : m_nodes)
                            if (!IsMixinClass(*node.second) && node.second->Is(constraintClass))
                                dependencies.insert(node.first);
                }
        return dependencies;
    }

    bset<Utf8String> Dependencies(Utf8StringCR key) const {
        bset<Utf8String> dependencies;
        auto absorbed = m_absorbedInto.find(key);
        if (absorbed != m_absorbedInto.end()) {
            dependencies.insert(absorbed->second);
            return dependencies;
        }
        auto node = m_nodes.find(key);
        if (node == m_nodes.end())
            return dependencies;
        for (auto const& dependency : StructuralDependencies(*node->second))
            if (dependency != key && m_nodes.count(dependency) != 0)
                dependencies.insert(dependency);
        auto extra = m_navigationDependencies.find(key);
        if (extra != m_navigationDependencies.end())
            dependencies.insert(extra->second.begin(), extra->second.end());
        return dependencies;
    }

    bool Reaches(Utf8StringCR from, Utf8StringCR to) const {
        bset<Utf8String> visited;
        bvector<Utf8String> stack{from};
        while (!stack.empty()) {
            auto key = stack.back();
            stack.pop_back();
            if (key == to)
                return true;
            if (!visited.insert(key).second)
                continue;
            for (auto const& next : Dependencies(key))
                stack.push_back(next);
        }
        return false;
    }

    //! Creates navigation properties of new classes inside the class's group, in declared order, so property
    //! ordinals and shared-column allocation match a native import. A new FK root relationship is created in the
    //! group of the class owning its chosen navigation. Placements that would form a dependency cycle are left
    //! to PlanNavigations/CreateRelationship (same metadata, later column allocation).
    void PlaceInlineNavigations() {
        bvector<NavigationECPropertyCP> candidates;
        for (auto const& entry : m_navigationsByRelationship)
            for (auto nav : entry.second)
                if (m_classes.OldClass(nav->GetClass()) == nullptr && nav->GetClass().IsEntityClass())
                    candidates.push_back(nav);
        std::sort(candidates.begin(), candidates.end(), [this](NavigationECPropertyCP a, NavigationECPropertyCP b) { return PropertyKey(*a) < PropertyKey(*b); });
        for (auto nav : candidates) {
            auto owner = ClassKey(nav->GetClass());
            auto relationship = nav->GetRelationshipClass();
            auto relationshipKey = ClassKey(*relationship);
            bool exists = m_nodes.count(relationshipKey) == 0 || m_classes.OldClass(*relationship) != nullptr;
            auto chosen = m_relationshipNavigation.find(relationshipKey);
            bool absorb = !exists && chosen != m_relationshipNavigation.end() && chosen->second == nav;
            bset<Utf8String> targets;
            if (absorb) {
                for (auto const& dependency : StructuralDependencies(*relationship))
                    if (dependency != owner && m_nodes.count(dependency) != 0)
                        targets.insert(dependency);
            } else if (!exists)
                targets.insert(relationshipKey);
            bool cyclic = false;
            for (auto const& target : targets)
                if (Reaches(target, owner)) {
                    cyclic = true;
                    break;
                }
            if (cyclic)
                continue;
            m_navigationDependencies[owner].insert(targets.begin(), targets.end());
            if (absorb)
                m_absorbedInto[relationshipKey] = owner;
            m_inlineNavigations[PropertyKey(*nav)] = absorb ? relationship->GetRelationshipClassCP() : nullptr;
            m_planner.m_createdNavigations.insert(PropertyKey(*nav));
        }
    }

    //! Entities and relationships in one dependency order: bases, mixin targets, relationship constraints and
    //! inline navigation prerequisites before their consumers.
    bool PlanClasses() {
        bvector<ECClassCP> classes, ordered;
        for (auto const& entry : m_nodes)
            if (m_absorbedInto.count(entry.first) == 0)
                classes.push_back(entry.second);
        auto dependencies = [this](ECClassCR c) {
            bset<Utf8String> resolved;
            for (auto key : Dependencies(ClassKey(c))) {
                auto absorbed = m_absorbedInto.find(key);
                resolved.insert(absorbed != m_absorbedInto.end() ? absorbed->second : key);
            }
            return resolved;
        };
        if (!TopologicalOrder<ECClass>(classes, [](ECClassCR c) { return ClassKey(c); }, dependencies, ordered))
            return Fail("Classes", "The class dependency graph contains a cycle.");
        ClassPlanner::NavigationHandler navigations = [this](PlannedStep& step, NavigationECPropertyCR nav) {
            auto found = m_inlineNavigations.find(PropertyKey(nav));
            if (found == m_inlineNavigations.end())
                return true;
            if (found->second != nullptr)
                return CreateRelationship(*found->second, &step);
            return m_classes.AddPropertyCommand(step, nav.GetClass(), nav);
        };
        for (auto ecClass : ordered) {
            auto old = m_classes.OldClass(*ecClass);
            if (old != nullptr) {
                if (!m_classes.AlterClass(*ecClass, *old, true))
                    return false;
            } else if (auto relationship = ecClass->GetRelationshipClassCP()) {
                if (!CreateRelationship(*relationship, nullptr))
                    return false;
            } else if (!m_classes.CreateClass(*ecClass, &navigations))
                return false;
        }
        return true;
    }

    bool CreateRelationship(ECRelationshipClassCR relationship, PlannedStep* into) {
        auto qualified = QName(relationship);
        auto schema = relationship.GetSchema().GetName();
        auto classMap = relationship.GetCustomAttributeLocal("ECDbMap", "ClassMap");
        if (classMap.IsValid()) {
            ECValue strategy;
            if (ECObjectsStatus::Success == classMap->GetValue(strategy, "MapStrategy") && !strategy.IsNull() && Utf8String(strategy.GetUtf8CP()).EqualsI("NotMapped"))
                return Fail(qualified, "NotMapped relationship classes are not supported by SchemaEditor.");
        }
        BeJsDocument args;
        Utf8String error;
        if (!DescribeClass(relationship, args, error))
            return Fail(qualified, error);
        args["schema"] = schema;
        args["name"] = relationship.GetName();
        if (!relationship.HasBaseClasses())
            args["mapAs"] = ClassifyRoot(relationship);
        auto& step = into != nullptr ? *into : m_planner.NewStep(schema, qualified, "RelationshipClass", true);
        CustomAttributes attributes;
        CollectCustomAttributes(relationship, attributes);
        bvector<std::pair<Utf8String, Utf8String>> later;
        m_planner.InlineAttributes(args, qualified, attributes, [](Utf8StringCR name) { return !IsCreationOnlyAttribute(name); }, &later);
        NavigationECPropertyCP navigation = nullptr;
        CustomAttributes navAttributes;
        auto chosen = m_relationshipNavigation.find(ClassKey(relationship));
        if (chosen != m_relationshipNavigation.end()) {
            navigation = chosen->second;
            args["navProperty"]["class"] = QName(navigation->GetClass());
            args["navProperty"]["name"] = navigation->GetName();
            args["navProperty"]["direction"] = navigation->GetDirection() == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward";
            CollectCustomAttributes(*navigation, navAttributes);
            if (!m_classes.TakeForeignKey(QName(navigation->GetClass()) + "." + navigation->GetName(), navAttributes, args))
                return false;
        }
        Planner::Add(step, "CreateRelationshipClass", args);
        for (auto property : relationship.GetProperties(false))
            if (!m_classes.AddPropertyCommand(step, relationship, *property))
                return false;
        if (navigation != nullptr) {
            auto item = QName(navigation->GetClass()) + "." + navigation->GetName();
            BeJsDocument described, alter;
            if (!DescribeProperty(*navigation, described, error))
                return Fail(item, error);
            alter.SetEmptyObject();
            described.ForEachProperty([&](Utf8CP key, BeJsConst value) {
                if (!Contains({"kind", "relationship", "direction"}, key))
                    alter[key].From(value);
                return false;
            });
            if (alter.size() != 0) {
                alter["class"] = QName(navigation->GetClass());
                alter["property"] = navigation->GetName();
                Planner::Add(step, "AlterProperty", alter);
            }
            for (auto const& entry : navAttributes)
                m_planner.SetAttribute(&step, schema, item, entry.first, entry.second);
        }
        for (auto end : {"source", "target"}) {
            CustomAttributes endAttributes;
            CollectCustomAttributes(0 == strcmp(end, "source") ? relationship.GetSource() : relationship.GetTarget(), endAttributes);
            for (auto const& entry : endAttributes)
                m_planner.SetAttribute(&step, schema, qualified + "." + end, entry.first, entry.second);
        }
        for (auto const& entry : later)
            m_planner.SetAttribute(&step, schema, qualified, entry.first, entry.second);
        return true;
    }

    bool PlanNavigations() {
        bvector<NavigationECPropertyCP> navigations;
        for (auto const& entry : m_navigationsByRelationship)
            for (auto nav : entry.second)
                if (m_planner.m_createdNavigations.count(PropertyKey(*nav)) == 0)
                    navigations.push_back(nav);
        std::sort(navigations.begin(), navigations.end(), [this](NavigationECPropertyCP a, NavigationECPropertyCP b) { return PropertyKey(*a) < PropertyKey(*b); });
        for (auto nav : navigations) {
            auto& owner = nav->GetClass();
            if (!m_classes.AddPropertyCommand(m_planner.NewStep(owner.GetSchema().GetName(), QName(owner) + "." + nav->GetName(), "NavigationProperty", false), owner, *nav))
                return false;
        }
        return true;
    }

    //! Collapse actual navigation/constraint cycles, not entire schema graphs, into mapping units.
    //! Class shells precede members so cyclic relationships can reference every endpoint.
    bool GroupClassDependencies(size_t firstStep) {
        bmap<Utf8String, bset<Utf8String>> graph;
        for (auto const& node : m_nodes) {
            auto& edges = graph[node.first];
            for (auto const& dependency : StructuralDependencies(*node.second))
                if (m_nodes.count(dependency) != 0)
                    edges.insert(dependency);
        }
        for (auto const& entry : m_navigationsByRelationship)
            for (auto nav : entry.second)
                if (m_nodes.count(entry.first) != 0)
                    graph[ClassKey(nav->GetClass())].insert(entry.first);
        for (auto const& entry : m_relationshipNavigation)
            graph[entry.first].insert(ClassKey(entry.second->GetClass()));

        bmap<Utf8String, size_t> index, low, component;
        bset<Utf8String> active;
        bvector<Utf8String> stack;
        bvector<bvector<Utf8String>> components;
        size_t next = 0;
        std::function<void(Utf8StringCR)> visit = [&](Utf8StringCR key) {
            index[key] = low[key] = next++;
            stack.push_back(key);
            active.insert(key);
            for (auto const& dependency : graph[key]) {
                if (index.count(dependency) == 0) {
                    visit(dependency);
                    low[key] = std::min(low[key], low[dependency]);
                } else if (active.count(dependency) != 0)
                    low[key] = std::min(low[key], index[dependency]);
            }
            if (low[key] != index[key])
                return;
            components.push_back({});
            Utf8String member;
            do {
                member = stack.back();
                stack.pop_back();
                active.erase(member);
                component[member] = components.size() - 1;
                components.back().push_back(member);
            } while (member != key);
        };
        for (auto const& node : m_nodes)
            if (index.count(node.first) == 0)
                visit(node.first);

        auto& steps = m_planner.m_steps;
        bmap<Utf8String, size_t> owners;
        for (size_t i = firstStep; i < steps.size(); ++i)
            for (auto const& command : steps[i].m_commands)
                if (command.first == "CreateEntityClass" || command.first == "CreateMixin") {
                    BeJsDocument args(command.second.c_str());
                    owners[Lower(Utf8String(args["schema"].asCString()) + ":" + args["name"].asCString())] = i;
                }

        // Reinsert deferred navigations at their declared position before the grouped mapper runs.
        for (auto const& entry : m_navigationsByRelationship)
            for (auto nav : entry.second) {
                auto ownerKey = ClassKey(nav->GetClass());
                auto owner = owners.find(ownerKey);
                if (owner == owners.end() || component.count(entry.first) == 0 ||
                    component[ownerKey] != component[entry.first])
                    continue;
                auto chosen = m_relationshipNavigation.find(entry.first);
                bool createsRelationship = chosen != m_relationshipNavigation.end() && chosen->second == nav;
                size_t source = SIZE_MAX;
                for (size_t i = firstStep; i < steps.size(); ++i) {
                    if (i == owner->second)
                        continue;
                    if ((createsRelationship && Lower(steps[i].m_item) == entry.first) ||
                        (!createsRelationship && Lower(steps[i].m_item) == PropertyKey(*nav))) {
                        source = i;
                        break;
                    }
                }
                if (source == SIZE_MAX)
                    continue;
                auto& commands = steps[owner->second].m_commands;
                bset<Utf8String> following;
                bool found = false;
                for (auto property : nav->GetClass().GetProperties(false)) {
                    if (found)
                        following.insert(Lower(property->GetName()));
                    if (property == nav)
                        found = true;
                }
                auto position = commands.end();
                for (auto it = commands.begin(); it != commands.end(); ++it) {
                    BeJsDocument args(it->second.c_str());
                    Utf8String name;
                    if (it->first == "AddProperty")
                        name = args["name"].asCString();
                    else if (it->first == "CreateRelationshipClass" && args.isMember("navProperty"))
                        name = args["navProperty"]["name"].asCString();
                    if (!name.empty() && following.count(Lower(name)) != 0) {
                        position = it;
                        break;
                    }
                }
                commands.insert(position, steps[source].m_commands.begin(), steps[source].m_commands.end());
                steps[source].m_commands.clear();
            }

        bvector<bvector<PlannedStep>> byComponent(components.size());
        for (size_t i = firstStep; i < steps.size(); ++i) {
            if (steps[i].m_commands.empty())
                continue;
            auto key = Lower(steps[i].m_item);
            auto dot = key.find('.');
            if (dot != Utf8String::npos)
                key.resize(dot);
            auto found = component.find(key);
            if (found == component.end())
                return Fail(steps[i].m_item, "Cannot resolve a class plan dependency.");
            byComponent[found->second].push_back(std::move(steps[i]));
        }
        steps.resize(firstStep);
        // Tarjan emits prerequisite components first because edges point from consumers to prerequisites.
        for (size_t i = 0; i < components.size(); ++i) {
            auto& members = byComponent[i];
            if (components[i].size() == 1) {
                for (auto& step : members)
                    steps.push_back(std::move(step));
                continue;
            }
            if (members.empty())
                continue;
            PlannedStep group;
            group.m_schema = members.front().m_schema;
            group.m_item = members.front().m_item;
            group.m_facet = "ClassDependencyGroup";
            group.m_group = true;
            for (auto& step : members)
                for (auto& command : step.m_commands)
                    if (command.first == "CreateEntityClass" || command.first == "CreateMixin")
                        group.m_commands.push_back(command);
            for (auto& step : members)
                for (auto& command : step.m_commands)
                    if (command.first != "CreateEntityClass" && command.first != "CreateMixin")
                        group.m_commands.push_back(std::move(command));
            steps.push_back(std::move(group));
        }
        return true;
    }

    void PlanIndexes() {
        for (auto const& entry : m_planner.m_indexes) {
            BeJsDocument args;
            args["container"] = entry.m_container;
            args["caClass"] = entry.m_caClass;
            args["instance"].From(BeJsDocument(entry.m_instance.c_str()));
            Utf8String schema, item;
            SchemaCommands::QualifiedName(entry.m_container, schema, item);
            m_planner.Emit(schema, entry.m_container, "DbIndexList", "SetCustomAttribute", args);
        }
        m_planner.m_indexes.clear();
    }

    void PlanAttributeOrder() {
        auto reorder = [&](Utf8StringCR schema, Utf8StringCR name, IECCustomAttributeContainerCR container) {
            // Creation persists inline and in-step attributes at consecutive ordinals. Reordering is only
            // needed when some were applied elsewhere (deferred, indexes, standalone) or out of source order.
            bvector<Utf8String> source;
            for (auto const& instance : container.GetCustomAttributes(false))
                source.push_back(QName(instance->GetClass()));
            if (source.size() < 2)
                return;
            auto created = m_planner.m_creationAttributes.find(name);
            if (created != m_planner.m_creationAttributes.end() && created->second == source)
                return;
            uint32_t ordinal = 0;
            for (auto const& instance : container.GetCustomAttributes(false)) {
                BeJsDocument args;
                args["container"] = name;
                args["caClass"] = QName(instance->GetClass());
                JsonEcInstanceWriter::WriteInstanceToSchemaJson(args["instance"], *instance, nullptr);
                args["ordinal"] = ordinal++;
                m_planner.Emit(schema, name, "CustomAttributeOrder", "SetCustomAttribute", args);
            }
        };
        for (auto schema : m_planner.m_ordered) {
            if (m_planner.Persisted(*schema) == nullptr)
                reorder(schema->GetName(), schema->GetName(), *schema);
            for (auto ecClass : schema->GetClasses()) {
                if (m_classes.OldClass(*ecClass) == nullptr) {
                    reorder(schema->GetName(), QName(*ecClass), *ecClass);
                    if (auto relationship = ecClass->GetRelationshipClassCP()) {
                        reorder(schema->GetName(), QName(*ecClass) + ".source", relationship->GetSource());
                        reorder(schema->GetName(), QName(*ecClass) + ".target", relationship->GetTarget());
                    }
                }
                for (auto property : ecClass->GetProperties(false))
                    if (IsNewProperty(*property))
                        reorder(schema->GetName(), QName(*ecClass) + "." + property->GetName(), *property);
            }
        }
    }

    void Drop(Utf8StringCR schema, Utf8StringCR item, Utf8CP facet, Utf8CP command, Utf8CP key) {
        BeJsDocument args;
        args[key] = item;
        m_planner.Emit(schema, item, facet, command, args);
    }

    bool PlanDrops() {
        for (auto it = m_schemas.rbegin(); it != m_schemas.rend(); ++it) {
            auto const& schema = **it;
            auto old = m_planner.Persisted(schema);
            if (old == nullptr)
                continue;
            auto const& name = schema.GetName();
            bvector<ECClassCP> removed;
            for (auto oldClass : old->GetClasses()) {
                auto survivor = schema.GetClassCP(oldClass->GetName().c_str());
                if (survivor == nullptr) {
                    removed.push_back(oldClass);
                    continue;
                }
                for (auto property : oldClass->GetProperties(false))
                    if (survivor->GetPropertyP(property->GetName().c_str(), false) == nullptr) {
                        BeJsDocument args;
                        args["class"] = QName(*survivor);
                        args["property"] = property->GetName();
                        m_planner.Emit(name, QName(*survivor) + "." + property->GetName(), "Property", "DropProperty", args);
                    }
            }
            bvector<ECClassCP> ordered;
            if (!Order(removed, nullptr, ordered, name.c_str()))
                return false;
            std::reverse(ordered.begin(), ordered.end());
            for (auto pass : {0, 1, 2, 3})
                for (auto ecClass : ordered) {
                    bool matches = pass == 0 ? ecClass->IsRelationshipClass() : pass == 1 ? ecClass->IsEntityClass()
                                                                            : pass == 2   ? ecClass->IsCustomAttributeClass()
                                                                                          : ecClass->IsStructClass();
                    if (!matches)
                        continue;
                    if (IsMixinClass(*ecClass))
                        return Fail(QName(*ecClass), "Deleting a mixin is not supported by SchemaEditor.");
                    Utf8CP command = pass == 0 ? "DropRelationshipClass" : pass == 1 ? "DropEntityClass"
                                                                       : pass == 2   ? "DropCustomAttributeClass"
                                                                                     : "DropStructClass";
                    Drop(name, QName(*ecClass), KindName(*ecClass), command, "class");
                }
            for (auto item : old->GetEnumerations())
                if (schema.GetEnumerationCP(item->GetName().c_str()) == nullptr)
                    Drop(name, QName(*item), "Enumeration", "DropEnumeration", "enum");
            for (auto item : old->GetKindOfQuantities())
                if (schema.GetKindOfQuantityCP(item->GetName().c_str()) == nullptr)
                    Drop(name, QName(*item), "KindOfQuantity", "DropKindOfQuantity", "koq");
            for (auto item : old->GetPropertyCategories())
                if (schema.GetPropertyCategoryCP(item->GetName().c_str()) == nullptr)
                    Drop(name, QName(*item), "PropertyCategory", "DropPropertyCategory", "category");
            for (auto item : old->GetUnits())
                if (schema.GetUnitCP(item->GetName().c_str()) == nullptr)
                    return Fail(QName(*item), "Deleting a unit is not supported (native schema upgrade rule).");
            for (auto item : old->GetFormats())
                if (schema.GetFormatCP(item->GetName().c_str()) == nullptr)
                    return Fail(QName(*item), "Deleting a format is not supported (native schema upgrade rule).");
            for (auto item : old->GetPhenomena())
                if (schema.GetPhenomenonCP(item->GetName().c_str()) == nullptr)
                    return Fail(QName(*item), "Deleting a phenomenon is not supported (native schema upgrade rule).");
            for (auto item : old->GetUnitSystems())
                if (schema.GetUnitSystemCP(item->GetName().c_str()) == nullptr)
                    return Fail(QName(*item), "Deleting a unit system is not supported (native schema upgrade rule).");
            for (auto const& entry : old->GetReferencedSchemas()) {
                bool kept = false;
                for (auto const& newEntry : schema.GetReferencedSchemas())
                    if (newEntry.second->GetName().EqualsI(entry.second->GetName())) {
                        kept = true;
                        break;
                    }
                if (kept)
                    continue;
                BeJsDocument args;
                args["schema"] = name;
                args["reference"] = entry.second->GetName();
                m_planner.Emit(name, name, "SchemaReference", "RemoveSchemaReference", args);
            }
        }
        return true;
    }

    bool Run() {
        if (!PlanSchemas() || !PrepareNavigations())
            return false;
        auto firstClassStep = m_planner.m_steps.size();
        return PlanClasses() && PlanNavigations() && GroupClassDependencies(firstClassStep) &&
               (PlanIndexes(), PlanAttributeOrder(), true) && PlanDrops();
    }
};
//! Moves AddProperty commands that directly follow their class's creation in the same step into the
//! creation's "properties" array, so the class and its properties are prepared and validated once.
//! Execution order is unchanged. A navigation property moves only if its relationship is already
//! persisted, because preparing it requires the relationship's class map.
void InlineCreatedClassProperties(ECDbCR db, PlannedStep& step) {
    bvector<std::pair<Utf8String, Utf8String>> commands;
    Utf8String owner; // lower-case qualified name of the class created by commands.back(), while properties can follow
    BeJsDocument properties;
    auto flush = [&]() {
        if (!owner.empty() && properties.size() != 0) {
            BeJsDocument args(commands.back().second.c_str());
            args["properties"].From(properties);
            commands.back().second = args.Stringify();
        }
        owner.clear();
    };
    for (auto& command : step.m_commands) {
        if (!owner.empty() && command.first == "AddProperty") {
            BeJsDocument args(command.second.c_str());
            Utf8String schema, item;
            bool inlinable = Lower(args["class"].asString()) == owner &&
                             (!args["kind"].asString().EqualsIAscii("navigation") ||
                              (SchemaCommands::QualifiedName(args["relationship"].asString(), schema, item) &&
                               db.Schemas().GetClass(schema, item, SchemaLookupMode::AutoDetect) != nullptr));
            if (inlinable) {
                args.removeMember("class");
                properties.appendValue().From(args);
                continue;
            }
        }
        flush();
        commands.push_back(std::move(command));
        if (commands.back().first == "CreateEntityClass" || commands.back().first == "CreateMixin" ||
            commands.back().first == "CreateStructClass" || commands.back().first == "CreateCustomAttributeClass") {
            BeJsDocument args(commands.back().second.c_str());
            owner = Lower(args["schema"].asString() + ":" + args["name"].asString());
            properties.SetEmptyArray();
        }
    }
    flush();
    step.m_commands = std::move(commands);
}
} // namespace

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SchemaCommandResult SchemaImportPlanBuilder::Build(ECDbR db, bvector<ECSchemaCP> const& schemas, bvector<SchemaImportPlanStep>& steps, bvector<Utf8String>& warnings) {
    steps.clear();
    warnings.clear();
    if (schemas.empty())
        return Result::Failure(Status::ERROR_INVALID_ARGS, "SchemaImport.NoSchemas", "schemas", "At least one schema is required.");
    bvector<PlannedStep> planned;
    Planner planner(db, planned, warnings);
    if (!planner.Normalize(schemas))
        return planner.m_result;
    PlanPhases phases(planner);
    if (!phases.Run())
        return planner.m_result;
    if (!planner.m_deferred.empty())
        return Result::Failure(Status::ERROR_VALIDATION, "SchemaImport.Unsupported", planner.m_deferred.front().m_container.c_str(),
                               "A custom attribute refers to a class that is neither persisted nor part of the import.");
    for (auto& step : planned) {
        InlineCreatedClassProperties(db, step);
        SchemaImportPlanStep out;
        out.m_schema = step.m_schema;
        out.m_item = step.m_item;
        out.m_facet = step.m_facet;
        out.m_group = step.m_group;
        BeJsDocument commands;
        commands.SetEmptyArray();
        for (auto const& command : step.m_commands) {
            auto envelope = commands.appendObject();
            envelope["command"] = command.first;
            envelope["args"].From(BeJsDocument(command.second.c_str()));
        }
        out.m_commands = commands.Stringify();
        steps.push_back(out);
    }
    return planner.m_result;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String SchemaImportPlanBuilder::ComputeBaseline(ECDbR db) {
    Utf8String baseline = db.GetDbGuid().ToString();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db,
                                     "SELECT (SELECT group_concat(Entry, ',') FROM (SELECT Name || ':' || VersionDigit1 || '.' || VersionDigit2 || '.' || VersionDigit3 AS Entry FROM main.ec_Schema ORDER BY Name)),"
                                     "(SELECT count(*) || '/' || ifnull(max(Id), 0) FROM main.ec_Class),"
                                     "(SELECT count(*) || '/' || ifnull(max(Id), 0) FROM main.ec_Property),"
                                     "(SELECT count(*) || '/' || ifnull(max(Id), 0) FROM main.ec_CustomAttribute),"
                                     "(SELECT count(*) || '/' || ifnull(max(Id), 0) FROM main.ec_Enumeration),"
                                     "(SELECT count(*) || '/' || ifnull(max(Id), 0) FROM main.ec_KindOfQuantity)") ||
        BE_SQLITE_ROW != stmt.Step())
        return baseline;
    for (int i = 0; i < stmt.GetColumnCount(); ++i) {
        baseline.append("|");
        if (auto text = stmt.GetValueText(i))
            baseline.append(text);
    }
    return baseline;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t SchemaImportPlan::GetCommandCount() const {
    size_t count = 0;
    for (auto const& step : m_steps)
        count += BeJsDocument(step.m_commands.c_str()).size();
    return count;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaImportPlan::ToJson(BeJsValue out) const {
    out.SetEmptyObject();
    out["baseline"] = m_baseline;
    auto warnings = out["warnings"];
    warnings.SetEmptyArray();
    for (auto const& warning : m_warnings)
        warnings.appendValue() = warning;
    auto steps = out["steps"];
    steps.SetEmptyArray();
    for (auto const& step : m_steps) {
        auto entry = steps.appendObject();
        entry["schema"] = step.m_schema;
        entry["item"] = step.m_item;
        entry["facet"] = step.m_facet;
        entry["group"] = step.m_group;
        entry["commands"].From(BeJsDocument(step.m_commands.c_str()));
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
