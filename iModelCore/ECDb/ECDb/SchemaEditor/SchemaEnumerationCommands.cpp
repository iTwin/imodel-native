/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;
enum class Operation { Create,
                       Alter,
                       Drop,
                       AddEnumerator,
                       DropEnumerator,
                       AlterEnumerator };

Result IntegerValue(Utf8StringCR text, int32_t& value) {
    if (text.empty())
        return Invalid("value", "Expected an int32 enumerator value.");
    size_t offset = text[0] == '-' ? 1 : 0;
    bool negative = offset != 0;
    if (offset == text.size())
        return Invalid("value", "Expected an int32 enumerator value.");
    uint64_t magnitude = 0;
    for (; offset < text.size(); ++offset) {
        if (text[offset] < '0' || text[offset] > '9')
            return Invalid("value", "Expected a complete int32 enumerator value.");
        magnitude = magnitude * 10 + text[offset] - '0';
        if (magnitude > uint64_t(std::numeric_limits<int32_t>::max()) + (negative ? 1 : 0))
            return Invalid("value", "Enumerator value exceeds int32.");
    }
    value = negative ? static_cast<int32_t>(-static_cast<int64_t>(magnitude)) : static_cast<int32_t>(magnitude);
    return {};
}

Result Enumerator(ECEnumerationR enumeration, Utf8StringCR name, Utf8StringCR value, Utf8StringCR label, Utf8StringCR description) {
    ECEnumeratorP enumerator = nullptr;
    ECObjectsStatus created;
    if (enumeration.GetType() == PRIMITIVETYPE_Integer) {
        int32_t integer;
        auto result = IntegerValue(value, integer);
        if (!result.IsOk())
            return result;
        created = enumeration.CreateEnumerator(enumerator, name, integer);
    } else
        created = enumeration.CreateEnumerator(enumerator, name, value.c_str());
    if (created != ECObjectsStatus::Success || enumerator == nullptr)
        return Rejected("enumerator", "Invalid enumerator name or duplicate name/value.");
    if ((!label.empty() && ECObjectsStatus::Success != enumerator->SetDisplayLabel(label)) ||
        (!description.empty() && ECObjectsStatus::Success != enumerator->SetDescription(description)))
        return Rejected("enumerator", "Invalid enumerator label or description.");
    return {};
}

Result Enumerators(ECEnumerationR enumeration, BeJsConst values) {
    if (values.isArray()) {
        for (uint32_t i = 0; i < values.size(); ++i) {
            Result result;
            SchemaCommandArgParser parser(values[i], result);
            Utf8String name, value, label, description;
            if (!parser.CheckKeys({"name", "value", "label", "description"}) || !parser.String("name", name, true) ||
                !parser.String("label", label) || !parser.String("description", description))
                return result;
            if (parser.Value("value").isNumeric()) {
                if (enumeration.GetType() != PRIMITIVETYPE_Integer)
                    return Invalid("enumerators.value", "A string-backed enumeration requires string values.");
                value = parser.Value("value").Stringify();
            } else if (!parser.String("value", value, true))
                return result;
            result = Enumerator(enumeration, name, value, label, description);
            if (!result.IsOk())
                return result;
        }
        return {};
    }
    if (!values.isString())
        return Invalid("enumerators", "Expected an enumerator array or Name=Value[:Label];... string.");
    Utf8String list(values.asCString());
    if (list.empty())
        return {};
    size_t offset = 0;
    while (offset < list.size()) {
        auto end = list.find(';', offset);
        if (end == Utf8String::npos)
            end = list.size();
        auto entry = list.substr(offset, end - offset);
        auto equal = entry.find('=');
        if (equal == Utf8String::npos || equal == 0)
            return Invalid("enumerators", "Expected Name=Value[:Label].");
        auto colon = entry.find(':', equal + 1);
        auto result = Enumerator(enumeration, entry.substr(0, equal),
                                 entry.substr(equal + 1, colon == Utf8String::npos ? Utf8String::npos : colon - equal - 1),
                                 colon == Utf8String::npos ? Utf8String() : entry.substr(colon + 1), "");
        if (!result.IsOk())
            return result;
        offset = end + 1;
        if (offset == list.size())
            return Invalid("enumerators", "A trailing enumerator separator is not allowed.");
    }
    return {};
}

Result PrepareEnumeration(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, Operation operation) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    bool create = operation == Operation::Create;
    if (create) {
        if (!parser.CheckKeys({"schema", "name", "backingType", "isStrict", "enumerators", "label", "description"}))
            return result;
    } else if (operation == Operation::Alter) {
        if (!parser.CheckKeys({"enum", "isStrict", "label", "description"}))
            return result;
    } else if (operation == Operation::AddEnumerator) {
        if (!parser.CheckKeys({"enum", "enumerator", "value", "label", "description"}))
            return result;
    } else if (operation == Operation::AlterEnumerator) {
        if (!parser.CheckKeys({"enum", "enumerator", "name", "label", "description"}))
            return result;
    } else if (!parser.CheckKeys(operation == Operation::Drop ? std::initializer_list<Utf8CP>{"enum"} : std::initializer_list<Utf8CP>{"enum", "enumerator"}))
        return result;
    Utf8String owner, name, backingType;
    if (!parser.String(create ? "schema" : "enum", owner, true) || (create && !parser.String("name", name, true)))
        return result;
    ECSchemaCP schema;
    ECEnumerationCP persisted = nullptr;
    if (create) {
        schema = FindSchema(ctx, owner);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
    } else {
        persisted = FindEnumeration(ctx, owner);
        if (persisted == nullptr)
            return Missing("enum", "Enumeration not found; use Schema:Enumeration.");
        schema = &persisted->GetSchema();
        name = persisted->GetName();
    }
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    auto enumId = persisted == nullptr ? ECEnumerationId() : persisted->GetId();
    if (operation == Operation::Drop) {
        result = ctx.RequireMajorUpgrade(*schema);
        if (!result.IsOk())
            return result;
        result = NoReferences(ctx, "SELECT Id FROM main.ec_Property WHERE EnumerationId=? LIMIT 1",
                              enumId, "enum", "The enumeration is referenced by a property.");
        if (!result.IsOk())
            return result;
        out.m_apply = [enumId, schemaId](SchemaEditContext& apply) {
            auto written = apply.m_writer.Delete("ec_Enumeration", enumId);
            if (written.IsOk())
                Affected(apply, schemaId);
            return written;
        };
        return {};
    }
    ECSchemaPtr working;
    result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    ECEnumerationP enumeration = nullptr;
    SchemaMetaRow row;
    if (create) {
        if (!parser.String("backingType", backingType, true))
            return result;
        PrimitiveType type;
        if (!Primitive(backingType, type) || (type != PRIMITIVETYPE_String && type != PRIMITIVETYPE_Integer))
            return Invalid("backingType", "Enumerations require string or int backing type.");
        if (ECObjectsStatus::Success != working->CreateEnumeration(enumeration, name.c_str(), type))
            return Rejected("name", "Invalid or duplicate enumeration name.");
        row = {{"SchemaId", int64_t(schemaId.GetValue())}, {"Name", name}, {"UnderlyingPrimitiveType", int64_t(type)}};
    } else
        enumeration = working->GetEnumerationP(name.c_str());
    if (enumeration == nullptr)
        return Rejected("enum", "Enumeration missing from working schema.");
    if (create || parser.Has("isStrict")) {
        bool strict = true;
        if (!parser.Boolean("isStrict", strict))
            return result;
        if (!create && strict && !persisted->GetIsStrict()) {
            result = ctx.RequireMajorUpgrade(*schema);
            if (!result.IsOk())
                return result;
            // Existing values are not constrained by SQLite. A version bump alone is insufficient.
            result = NoReferences(ctx, "SELECT Id FROM main.ec_Property WHERE EnumerationId=? LIMIT 1",
                                  enumId, "isStrict", "Making a used enumeration strict requires validation of every stored value.");
            if (!result.IsOk())
                return result;
        }
        if (ECObjectsStatus::Success != enumeration->SetIsStrict(strict))
            return Rejected("isStrict", "Cannot change enumeration strictness.");
        row.push_back({"IsStrict", int64_t(strict)});
    }
    bool changedValues = create || operation == Operation::AddEnumerator || operation == Operation::DropEnumerator || operation == Operation::AlterEnumerator;
    if (operation == Operation::AlterEnumerator) {
        Utf8String enumeratorName, newName, label, description;
        if (!parser.String("enumerator", enumeratorName, true) || !parser.String("name", newName) ||
            !parser.String("label", label) || !parser.String("description", description))
            return result;
        if (!parser.Has("name") && !parser.Has("label") && !parser.Has("description"))
            return Invalid("enumerator", "At least one of name, label, or description must be supplied.");
        auto enumerator = enumeration->FindEnumeratorByName(enumeratorName.c_str());
        if (enumerator == nullptr)
            return Missing("enumerator", "Enumerator not found.");
        if (parser.Has("name") && !newName.Equals(enumerator->GetName())) {
            auto major = persisted->GetSchema().GetOriginalECXmlVersionMajor();
            auto minor = persisted->GetSchema().GetOriginalECXmlVersionMinor();
            auto original = ctx.m_session.m_originalECXmlVersions.find(schemaId);
            if (original != ctx.m_session.m_originalECXmlVersions.end()) {
                major = original->second.first;
                minor = original->second.second;
            }
            if (!(major < 3 || (major == 3 && minor < 2)))
                return Rejected("name", "Enumerator names can only change for schemas originally serialized before ECXML 3.2 (ECDb_0402).");
            if (enumeration->FindEnumeratorByName(newName.c_str()) != nullptr)
                return Rejected("name", "An enumerator with this name already exists.");
            if (ECObjectsStatus::Success != enumerator->SetName(newName))
                return Rejected("name", "Invalid enumerator name.");
        }
        if (parser.Has("label") && ECObjectsStatus::Success != enumerator->SetDisplayLabel(label))
            return Rejected("label", "Invalid enumerator label.");
        if (parser.Has("description") && ECObjectsStatus::Success != enumerator->SetDescription(description))
            return Rejected("description", "Invalid enumerator description.");
    }
    if (create && parser.Has("enumerators")) {
        result = Enumerators(*enumeration, parser.Value("enumerators"));
        if (!result.IsOk())
            return result;
    }
    if (operation == Operation::AddEnumerator || operation == Operation::DropEnumerator) {
        Utf8String enumeratorName;
        if (!parser.String("enumerator", enumeratorName, true))
            return result;
        if (operation == Operation::AddEnumerator) {
            Utf8String value, label, description;
            if (!parser.String("label", label) || !parser.String("description", description))
                return result;
            if (parser.Value("value").isNumeric()) {
                if (enumeration->GetType() != PRIMITIVETYPE_Integer)
                    return Invalid("value", "String-backed enumerations require string values.");
                value = parser.Value("value").Stringify();
            } else if (!parser.String("value", value, true))
                return result;
            result = Enumerator(*enumeration, enumeratorName, value, label, description);
            if (!result.IsOk())
                return result;
        } else {
            auto enumerator = enumeration->FindEnumeratorByName(enumeratorName.c_str());
            if (enumerator == nullptr)
                return Missing("enumerator", "Enumerator not found.");
            if (ctx.m_originalReadVersions.find(schemaId) != ctx.m_originalReadVersions.end()) {
                // VerifyEnumeratorChanges rejects every unmatched deletion, without
                // a major-version exception. Its ignore option retains the value;
                // a standalone deletion cannot use the legacy same-value rename rule.
                return Rejected("enumerator", "Native enumerator-upgrade rules prohibit unmatched persisted enumerator deletion at every schema version. DoNotFailForDeletionsOrModifications retains the old value, so it cannot make DropEnumerator succeed.");
            }
            result = ctx.RequireMajorUpgrade(*schema);
            if (!result.IsOk())
                return result;
            if (enumeration->GetIsStrict()) {
                result = NoReferences(ctx, "SELECT Id FROM main.ec_Property WHERE EnumerationId=? LIMIT 1",
                                      enumId, "enumerator", "Removing a value from a used strict enum requires stored-value validation.");
                if (!result.IsOk())
                    return result;
            }
            if (ECObjectsStatus::Success != enumeration->DeleteEnumerator(*enumerator))
                return Rejected("enumerator", "ECObjects rejected deleting the enumerator.");
        }
    }
    if (create || operation == Operation::Alter) {
        result = Labels(parser, *enumeration, row);
        if (!result.IsOk())
            return result;
    }
    if (changedValues) {
        Utf8String serialized;
        if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(serialized, *enumeration, FeatureManager::IsEC32Available(ctx.m_db)))
            return Rejected("enumerators", "Cannot serialize the enumerators for this ECDb profile.");
        row.push_back({"EnumValues", serialized});
    }
    if (row.empty())
        return Invalid("enum", "At least one enumeration attribute must change.");
    result = ValidateWorking(ctx, *working);
    if (!result.IsOk())
        return result;
    out.m_apply = [create, enumId, schemaId, row](SchemaEditContext& apply) mutable {
        ECEnumerationId id = enumId;
        if (create) {
            id = ECEnumerationId(apply.m_db.GetImpl().GetIdFactory().Enumeration().NextId().GetValue());
            row.push_back({"Id", int64_t(id.GetValue())});
        }
        auto written = create ? apply.m_writer.Insert("ec_Enumeration", row) : apply.m_writer.Update("ec_Enumeration", id, row);
        if (written.IsOk())
            Affected(apply, schemaId);
        return written;
    };
    return {};
}
}

void RegisterSchemaEnumerationCommands(SchemaCommandRegistry& registry) {
    for (auto entry : {std::make_pair("CreateEnumeration", Operation::Create), std::make_pair("AlterEnumeration", Operation::Alter),
                       std::make_pair("DropEnumeration", Operation::Drop), std::make_pair("AddEnumerator", Operation::AddEnumerator),
                       std::make_pair("DropEnumerator", Operation::DropEnumerator),
                       std::make_pair("AlterEnumerator", Operation::AlterEnumerator)})
        Register(registry, entry.first, [operation = entry.second](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
            // No property can use an enumeration before it exists, so creating one maps nothing.
            out.m_mapsClasses = operation != Operation::Create;
            return PrepareEnumeration(ctx, args, out, operation);
        });
}
END_BENTLEY_SQLITE_EC_NAMESPACE
